
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic

SRCS = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJS = $(SRCS:.c=.o)


EXEC = ifj24_compiler

.PHONY: all clean zip

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

zip:
	zip xstepa77.zip $(SRCS) $(HEADERS) Makefile dokumentace.pdf rozdeleni
