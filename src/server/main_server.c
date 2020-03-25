#include "uchat.h"


int mx_set_demon(const char *log_file) {
    int fd;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    if((pid = fork()) < 0) {
        perror("error fork");
        exit(1);
    }
    if (pid > 0)
        exit(0);
    umask(0);  // Сбросить маску режима создания файла.

//     Обеспечить невозможность обретения управляющего терминала в будущем.
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        mx_printerr("невозможно игнорировать сигнал SIGHUP");

     // Закрыть все открытые файловые дескрипторы.
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1025;
    for (rlim_t i = 0; i < rl.rlim_max; i++)
        close(i);
//    if (chdir("/") < 0)
//        mx_printerr("%s: невозможно сделать текущим рабочим каталогом /\n");

    if ((fd = open(log_file, O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, S_IRWXU)) == -1) {
        printf("open error\n");
        return -1;
    }
    printf("log_file fd  %d\n", fd);
    int rc = dup2(fd, STDOUT_FILENO);
    rc = dup2(fd, STDERR_FILENO);
    close(fd);
    if (rc == -1) {
        printf("dup error\n");
        return -1;
    }
    close(STDIN_FILENO);
    close(STDERR_FILENO);
    return setsid();
}

int main(int argc, char **argv) {
    main2(argc, argv);  // test info
    SSL_CTX *ctx;

   uint16_t port = atoi(argv[1]);
    int server;
    if (argc < 2) {
        mx_printerr("usage: chat_server [port]\n");
        _exit(1);
    }
    if (mx_set_demon("logfile") == -1) {
        printf("error = %s\n", strerror(errno));
        return 0;
    }

    ctx = mx_init_server_ctx();  // initialize SSL
    mx_load_certificates(ctx, "newreq.pem", "newreq.pem"); // load cert

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, argv[1], &hints, &bind_address);


char abuf[INET_ADDRSTRLEN];
const char *addr;
struct sockaddr_in *sinp;

sinp = (struct sockaddr_in *)bind_address->ai_addr;
addr = inet_ntop(AF_INET, &sinp->sin_addr, abuf, INET_ADDRSTRLEN);

    printf("bind_adress %s\n", addr);

    printf("Creating socket...\n");
    server = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (server == -1) {
        printf("socket error = %s\n", strerror(errno));
        return -1;
    }
    //////
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton("193.168.1.124", &serv_addr.sin_addr);
    if (bind(server, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        printf("bind error = %s\n", strerror(errno));
        return -1;
    }

    // if (bind(server, bind_address->ai_addr, bind_address->ai_addrlen)) {
        // printf("bind error = %s\n", strerror(errno));
        // return 0;
    // }
    freeaddrinfo(bind_address);

    if (listen(server, SOMAXCONN) == -1) {
        printf("listen error = %s\n", strerror(errno));
        return -1;
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    while (1) {
        pthread_t worker_thread;

        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        int client_sock;

        client_sock = accept(server, (struct sockaddr*) &client_address,
                             &client_len);
        printf("server new client socket %d\n", client_sock);
        if (client_sock == -1) {
            printf("error = %s\n", strerror(errno));
            continue;
        }
        char address_buffer[101];
        getnameinfo((struct sockaddr*)&client_address, client_len,
                    address_buffer, sizeof(address_buffer), 0, 0,
                    NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);

        SSL *ssl = SSL_new(ctx);
        if (!ctx) {
            fprintf(stderr, "SSL_new() failed.\n");
            return 1;
        }
        if (!SSL_set_tlsext_host_name(ssl, "193.168.1.124")) {
            fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
            ERR_print_errors_fp(stderr);
            return 1;
        }

        SSL_set_fd(ssl, client_sock);
        if (SSL_connect(ssl) == -1) {
            fprintf(stderr, "SSL_connect() failed.\n");
            ERR_print_errors_fp(stderr);
            return 1;
        }

        if ( SSL_accept(ssl) == 0 )					/* do SSL-protocol accept */
            ERR_print_errors_fp(stderr);
        else {
            mx_show_certs(ssl);
        }

        int rc = pthread_create(&worker_thread, &attr, mx_worker, &client_sock);
        if (rc != 0) {
            printf("error pthread_create = %s\n", strerror(rc));
            close(client_sock);
        }
    }
    pthread_attr_destroy(&attr);
    close(server);
    SSL_CTX_free(ctx);
    return 0;
}


SSL_CTX* mx_init_server_ctx(void) {
//    SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();		// load & register all cryptos, etc.
    SSL_load_error_strings();			// load all error messages
//    method = SSLv23_server_method();

    // create new server-method instance
    ctx = SSL_CTX_new(SSLv23_server_method());			// create new context from method
    if ( ctx == NULL ) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}


void mx_load_certificates(SSL_CTX* ctx, char* cert_file, char* key_file) {
    // set the local certificate from CertFile
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    // set the private key from KeyFile (may be the same as CertFile)
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    // verify private key
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

/*
        // Конвертирует имя хоста в IP адрес
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // Указаваем адрес IP сокета
    bcopy((char *)server->h_addr,
          (char *)&_addr.sin_addr.s_addr,
          server->h_length);
    */

    /*
    struct hostent *server;
     int n;
//     char *host;
     if ((n = sysconf(_SC_HOST_NAME_MAX)) < 1)
        n = HOST_NAME_MAX; // лучшее, что можно сделать
    if ((host = malloc(n)) == NULL)
        err_sys("ошибка вызова функции malloc");
    if (gethostname(host, n) < 1)
        err_sys("ошибка вызова функции gethostname");

*/
