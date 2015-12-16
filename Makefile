CFLAGS=-Wall
CFLAGS=-std=c99

all: clean
	gcc $(CFLAGS) utilities.c atp_part3.c -o atp_part3 -lpthread
	gcc $(CFLAGS) utilities.c publisher.c -o publisher
	gcc $(CFLAGS) utilities.c subscriber.c -o subscriber
clean:
	-rm -r atp_part3 publisher subscriber