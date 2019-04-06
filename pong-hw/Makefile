# ------------------------------------------------------------
# Makefile for sttyl
# ------------------------------------------------------------
# Compiles with messages about warnings and produces debugging
# information. Only file is sttyl.c.
#

CC = gcc
CFLAGS = -Wall -g

pong: pong.o alarmlib.o ball.o clock.o court.o paddle.o 
	$(CC) -o pong pong.o alarmlib.o ball.o clock.o court.o paddle.o -lcurses

pong.o: pong.c
	$(CC) $(CFLAGS) -c pong.c

alarmlib.o: alarmlib.c
	$(CC) $(CFLAGS) -c alarmlib.c

ball.o: ball.c
	$(CC) $(CFLAGS) -c ball.c

clock.o: clock.c
	$(CC) $(CFLAGS) -c clock.c

court.o: court.c
	$(CC) $(CFLAGS) -c court.c

paddle.o: paddle.c
	$(CC) $(CFLAGS) -c paddle.c

clean:
	rm -f *.o pong
