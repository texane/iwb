# Makefile for iwb

CFLAGS=-std=c99 -O -Wall -Wextra
OBJECTS=iwb.o main.o

iwb:	$(OBJECTS)
	gcc $^ -o $@

clean:
	rm -f iwb $(OBJECTS)

%.o:	%.c
	gcc -c $(CFLAGS) $<

main.o:	iwb.h
iwb.o:	iwb.h
