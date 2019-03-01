# ----------------------------------------------------------
# Makefile for pfind
# 
#
#

GCC = gcc -Wall -Wextra -g

pfind: pfind.o pfind-errors.o
	$(GCC) -o pfind pfind.o pfind-errors.o

pfind.o: pfind.c
	$(GCC) -c pfind.c

pfind-errors.o: pfind-errors.c
	$(GCC) -c pfind-errors.c

clean:
	rm -f *.o pfind
