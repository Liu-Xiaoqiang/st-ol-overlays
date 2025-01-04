all: hpdet receive receive_show send

hpdet:
	$(CC) -o hpdet hpdet.c -lgpiod

send:
	$(CC) -o send send.c

receive:
	$(CC) -o receive receive.c

receive_show:
	$(CC) -o receive_show receive_show.c

clean:
	rm -f *.o hpdet receive receive_show send

PHONY += FORCE
FORCE:
