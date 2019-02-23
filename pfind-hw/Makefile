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

GCC = gcc -Wall -Wextra -g

pfind: pfind.o
	$(GCC) -o pfind pfind.o

pfind.o: pfind.c
	$(GCC) -c pfind.c

clean:
	rm -f *.o pfind
