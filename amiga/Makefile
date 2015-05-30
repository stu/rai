AOPTS=+aos68k39
CC=vc

all: rai 

rai: main.c ini.c dlist.c list.c queue.c amiga_missing.c
	$(CC) $(AOPTS) amiga_missing.c main.c ini.c dlist.c list.c queue.c  -o rai -lauto -lamiga

clean:
	rm rai 

