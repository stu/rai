# RAI : Retro Adventure Interpreter
### By Stu George


This is the amiga port of my little adventure game interpreter.
It started life as an idea to make something playable on the C64/C128.
It was prototyped using a GLK library on my 64bit linux box and once
the main engine was working, moved to the c128 then to the c64.

After hacking in REU code and using ROM space, I moved it to the amiga.

By the time it got to the Amiga it had all kinds of idiosyncrasies.

The amiga port seems to work, but I'm done spending time on it.

I built the cross platform games using a ruby script as my compiler,
and the game scripts will run on the pc/amiga/c64/c126/c+4. Save games
should be portable as well.

Any way....

Games
=====
Most of the games are tests or shells. There are only two full games
in there, dutch (Lost Dutchmans Mine) and mystery (Mystery Island).

The PC version has a decompiler as a separate app for debugging etc.


PC
==
To compile the PC version you need GLK libraries for output
(winglk, xglk, etc) and premake to create the build files.
run from command line
rai GAMENAME

C64
===
For the C64 and other 8bit commodores, you need CC65 installed.
The c64 usees pucrunch to pack its files, the c128 does not pack
as doing so will kill the command line parameters.
once exec is loaded type
run:rem GAMENAME

Amiga
=====
The Amiga uses vbcc to compile.
There is an .ini file that holds the font config for the amiga interpreter,
so if you have a large screen set topaz to 9, if you have a small screen
set it to 8 etc.
Need to run it from the shell so you can specify the game to load
rai GAMENAME


Technicals
==========
Designed to fit and run on a c64, which needed the interpreter and data to fit into ram,
so the bytecode is small, very much oriented on Scott Adams style codes.

Text compression is done in the compiler, dupe strings are removed, and all words are
placed into a dictionary. All messages are done as a list of pointers directly into
the dictionary pointing to a word and encoding the length to print, this way the dictionary
only contains unique words, and a 16bit pointer is less in size than most words... so..
crappy but works and speed is good. Word length is encoded as 4 bits, so 16 letters,
which means the top 12 bits are pointer into the dictionary, meaning dictionary can not
exceed offset 0x0FFF.

This is pointed to by the message/sentence dictionary, so an opcode will be encoded
with a 16bit message number.


