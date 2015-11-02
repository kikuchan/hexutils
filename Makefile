PREFIX=/usr/local

all: hd hexdec

hd: hd.c
	$(CC) $(CFLAGS) -o hd hd.c

hexdec: hexdec.c
	$(CC) $(CFLAGS) -o hexdec hexdec.c

install: hd hexdec
	cp hd hexdec $(PREFIX)/bin

clean:
	rm -f hd hexdec
