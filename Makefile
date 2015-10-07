all: hd hexdec

hd: hd.c
	$(CC) $(CFLAGS) -o hd hd.c

hexdec: hexdec.c
	$(CC) $(CFLAGS) -o hexdec hexdec.c

install: hd
	cp hd hexdec $(HOME)/bin

clean:
	rm -f hd hexdec
