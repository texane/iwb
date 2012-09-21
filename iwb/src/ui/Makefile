# Makefile for iwb_ui

CFLAGS=-O -Wall -Wextra
LDLIBS=-lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_calib3d

iwb_ui:	main.o
	gcc $^ $(LDLIBS) -o $@

clean:
	rm -f main.o iwb_ui

%.o:	%.c
	gcc -c $(CFLAGS) $<
