NAME_S = chat_server
NAME_C = uchat


INC = uchat.h

SRC_SERVER = server.c

SRC_CLIENT = client.c

OBJ_SERVER = server.o

OBJ_CLIENT = client.o

CFLAGS = -std=c11 -Wall -Wextra -Werror -Wpedantic

all: install clean

install:
	#@make install -C libmx
	@cp $(addprefix inc/, $(INC)) .
	@cp $(addprefix src/server/, $(SRC_SERVER)) .
	@cp $(addprefix src/client/, $(SRC_CLIENT)) .
	@clang $(CFLAGS) -c $(SRC_SERVER) -I $(INC)
	@clang $(CFLAGS) -c $(SRC_CLIENT) -I $(INC)
	@clang $(CFLAGS) libmx/libmx.a $(OBJ_SERVER) -o $(NAME_S)
	@clang $(CFLAGS) libmx/libmx.a $(OBJ_CLIENT) -o $(NAME_C)
	@mkdir -p obj
	@mv $(OBJ_SERVER) $(OBJ_CLIENT) ./obj

uninstall: clean
	#@make uninstall -C libmx
	@rm -rf $(NAME_S) $(NAME_C)

clean:
	#@make clean -C libmx
	@rm -rf $(INC)
	@rm -rf $(SRC) $(SRC_SERVER) $(SRC_CLIENT)
	@rm -rf $(OBJ) $(OBJ_SERVER) $(OBJ_CLIENT)
	@rm -rf ./obj

reinstall: uninstall install
