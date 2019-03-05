# ------------------------------------------------------------
# Makefile for sttyl
# ------------------------------------------------------------
# Compiles with messages about warnings and produces debugging
# information. Only file is pfind.c.
#

GCC = gcc -Wall -g

sttyl: sttyl.o tty_tables.o termfuncs.o
	$(GCC) -o sttyl sttyl.o tty_tables.o termfuncs.o

sttyl.o: sttyl.c
	$(GCC) -c sttyl.c
	
tty_tables.o: tty_tables.c
	$(GCC) -c tty_tables.c

termfuncs.o: termfuncs.c
	$(GCC) -c termfuncs.c

clean:
	rm -f *.o sttyl
