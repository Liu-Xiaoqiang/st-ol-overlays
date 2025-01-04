all: hpdet receive receive_show send

hpdet:
	$(CC) -o hpdet hpdet.c -lgpiod

send:
	$(CC) -o send send.cpp

receive:
	$(CC) -o receive receive.cpp

receive_show:
	$(CC) -o receive_show receive_show.cpp

clean:
	rm -f *.o hpdet receive receive_show send

PHONY += FORCE
FORCE:
