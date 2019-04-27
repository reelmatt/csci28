#
#
# makefile for webserver
#

CC = gcc -Wall

wsng: wsng.o socklib.o web-time.o
	$(CC) -o wsng wsng.o socklib.o web-time.o
	

ws: ws.o socklib.o
	$(CC) -o ws ws.o socklib.o


clean:
	rm -f ws.o socklib.o core
