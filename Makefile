CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra -Wno-unused

.PHONY: all
all: clean nyush

nyush: nyush.o sh.o

nyush.o: nyush.c sh.h

sh.o: sh.c sh.h

.PHONY: clean
clean:
	rm -f *.o nyush

