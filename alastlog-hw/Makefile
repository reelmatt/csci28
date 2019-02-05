# ----------------------------------------------------------
# This is a makefile.  A makefile contains rules that tell
# how to build a program, often from separate source files
#
# This sample makefile shows how to build dumputmp from
# the two source files dumputmp.c and utmplib.c
#
# You must modify this file so it contains rules to make
# your solution to the project
# (note: the indented lines MUST start with a single tab
#

CC = gcc -Wall -Wextra -g

alastlog: alastlog.o lllib.o
	$(CC) -o alastlog alastlog.o llib.o

alastlog.o: alastlog.c
	$(CC) -c alastlog.c

lllib.o: llib.c
	$(CC) -c llib.c

clean:
	rm -f *.o core alastlog

