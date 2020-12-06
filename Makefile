CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build: s-talk.c list.o
	gcc $(CFLAGS) $^ -o s-talk -lpthread

clean:
	rm -f s-talk

valgrind: build
	valgrind --leak-check=full ./s-talk



