# ----------------------------------------------------------
# Makefile for pfind
# 
#
#

GCC = gcc -Wall -Wextra -g

pfind: pfind.o
	$(GCC) -o pfind pfind.o

pfind.o: pfind.c
	$(GCC) -c pfind.c

clean:
	rm -f *.o pfind
