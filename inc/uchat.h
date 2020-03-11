#ifndef UCHAT_H
#define UCHAT_H

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>


#include "libmx/inc/libmx.h"

/*
 * Colors.
 */
#define MX_BLK   "\x1B[30m"
#define MX_RED   "\x1B[31m"
#define MX_GRN   "\x1B[32m"
#define MX_YEL   "\x1B[33m"
#define MX_BLU   "\x1B[34m"
#define MX_MAG   "\x1B[35m"
#define MX_CYN   "\x1B[36m"
#define MX_WHT   "\x1B[37m"
#define MX_RESET "\x1B[0m"
#define MX_RED_B "\x1B[1;31m"
#define MX_RESET_B "\x1B[1;31m"
#define MX_BLK_F_RED_B "\x1B[0;30;41m"
#define MX_BLK_F_CYAN_B "\x1B[0;30;46m"
#define MX_BLOCK "\x1B[0;34;46m"
#define MX_CHR "\x1B[0;34;43m"
#define MX_DIR_T "\x1B[0;30;42m"
#define MX_DIR_X "\033[0;30;43m"
#define MX_BOLD_MAGENTA "\x1B[1;35m"
#define MX_BOLD_CYAN "\x1B[1;36m"
#define MX_BOLD_RED "\x1B[[1;31m"
#define MX_BOLD_BLUE "\x1B[1;34m"

int mx_set_demon(const char *log_file);
void *mx_worker(void *arg);


#endif
