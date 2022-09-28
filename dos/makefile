cc=wcc
!ifdef NDEBUG
debug_flags=-DNDEBUG
link_debug=
!else
debug_flags=-d3
link_debug=debug all
!endif
cflags=-oneatx -0 -ml -zp1 $(debug_flags)

all: rai.exe

rai.exe: main.obj pc.obj
	wlink name rai.exe system dos $(link_debug) file { main.obj pc.obj } option {stack=32k quiet}
!ifdef NDEBUG
	wstrip -q -n rai.exe
	vac2 rai.exe
	ren rai.exe rai.bin
	sauce -preset rai -autodate -i rai.bin -o rai.exe
	%erase rai.bin
!endif

release: .symbolic
	@wmake -h clean
	@wmake -h NDEBUG=1 rai.exe


clean: .symbolic
	if exist *.obj del *.obj
	if exist rai.exe del rai.exe

.c.obj:
	*$(CC) -q $(CFLAGS) $< -fo$@
