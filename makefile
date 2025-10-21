CC=gcc
CFLAGS=-Wall -Wextra -I. -g
SRCDIR=src
OBJDIR=bin
OBJS=file_helpers.o other_helpers.o request_handlers.o response_handlers.o http_helpers.o server_handlers.o server.o

all: server

server: $(OBJS)
	gcc -o $@ $^

other_helpers.o: other_helpers.c other_helpers.h

file_helpers.o: file_helpers.c file_helpers.h

http_helpers.o: http_helpers.c http_helpers.h

request_handlers.o: request_handlers.c request_handlers.h file_helpers.h

response_handlers.o: response_handlers.c response_handlers.h

server_handlers.o: server_handlers.c server_handlers.h http_helpers.h

server.o: server.c

clean:
	rm -f *.o
	rm -f server

.PHONY: clean