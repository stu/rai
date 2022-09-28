# unixy makefile.

# This one comes with VICE
C1541 = /usr/local/bin/c1541

CC = cl65
AS = ca65
AFLAGS= -t c64
CFLAGS= -t c64 -O
LFLAGS= -t c64 -O

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(AFLAGS) $< -o $@

OBJS = main.o c64_reu.o c64_ram.o

all: clean pucrunch/pucrunch r64 disk c128 diskall
#plus4

pucrunch/pucrunch:
	cd pucrunch ; $(MAKE) -f Makefile all

c64_reu.o:
	co65 --code-label _x_reu $(CC65_HOME)/target/c64/drv/emd/c64-reu.emd -o c64_reu.s
	ca65 c64_reu.s

c64_ram.o:
	co65 --code-label _x_ram $(CC65_HOME)/target/c64/drv/emd/c64-ram.emd -o c64_ram.s
	ca65 c64_ram.s


r64: $(OBJS)
	$(CC) $(LFLAGS) $^ -m $@.map -o $@

sv:
	cd ../games ; ruby ./compiler.rb -v -x dutch.gs
	cd ../games ; ruby ./compiler.rb -v -x mystery.gs
	cd ../games ; ruby ./compiler.rb -v -x adv1.gs
	cd ../games ; ruby ./compiler.rb -v -x al.gs
	cd ../games ; ruby ./compiler.rb -v -x pod.gs


rai64: r64
	pucrunch/pucrunch -fshort -m6 -f -c64 r64 rai64
	#cp r64 rai64

disk: rai64
	$(C1541) -format game,po  d64 $@.d64 > /dev/null
	$(C1541) -attach $@.d64 -write rai64 > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/dutch.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/mystery.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/adv1.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/al.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/pod.rai > /dev/null || exit $$?;\

diskall: rai64 c128 #plus4
	$(C1541) -format game,po d64 $@.d64 > /dev/null
	$(C1541) -attach $@.d64 -write rai64 > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write rai128 > /dev/null || exit $$?;\
	#$(C1541) -attach $@.d64 -write raip4 > /dev/null || exit $$?;
	$(C1541) -attach $@.d64 -write ../games/dutch.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/mystery.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/adv1.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/al.rai > /dev/null || exit $$?;\
	$(C1541) -attach $@.d64 -write ../games/pod.rai > /dev/null || exit $$?;\

plus4:
	rm *.o
	$(MAKE) -f MakefileP4 all

c128:
	rm *.o
	$(MAKE) -f Makefile128 all

clean:
	rm -rf *.o r64 rai64 r64.map disk.d64 c64_reu.s c64_ram.s diskall.d64
	cd pucrunch ; $(MAKE) -f Makefile clean
	$(MAKE) -f Makefile128 clean
	$(MAKE) -f MakefileP4 clean
