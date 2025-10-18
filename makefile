CC=gcc
CFLAGS=-Wall -Wextra -I. -g

OBJS=file_helpers.o http_helpers.o server_handlers.o server.o

all: server

server: $(OBJS)
	gcc -o $@ $^

file_helpers.o: file_helpers.c file_helpers.h

server_handlers.o: server_handlers.c server_handlers.h

http_helpers.o: http_helpers.c http_helpers.h

server.o: server.c server_handlers.h

clean:
	rm -f *.o
	rm -f server

.PHONY: clean