# unixy makefile.

CC=gcc
CFLAGS=-m32 -Wall -funsigned-char -O -Dstricmp=strcasecmp
LFLAGS=-m32

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

OBJS = pucrunch.o

all: pucrunch

pucrunch: $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm -rf *.o pucrunch

