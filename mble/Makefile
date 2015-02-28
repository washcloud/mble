CC?=gcc
CFLAGS+=-Os
CFLAGS+=-Iincludes/
CFLAGS+=-Wall -Wextra -pedantic -std=gnu99
LDLIBS+=-levent -lbluetooth

OBJS=main.c mble.c

.PHONY: all clean install

all: mble

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

mble: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mble $+ $(LDLIBS)

clean:
	rm -f mble *.o
	rm -rf dist