
# Comment/uncomment the following line to enable/disable debugging
DEBUG = y

# This Makefile has been simplified as much as possible, by putting all
# generic material, independent of this specific directory, into
# ../Rules.make. Read that file for details

TOPDIR  := $(shell cd ..; pwd)
#include ../Rules.make

ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

CFLAGS += $(DEBFLAGS) -I. -I.. -I../drvr


test : test.o
	gcc test.o -o test

regrw : regrw.o
	gcc regrw.o -o regrw

test.o : test.c
	gcc -c $(CFLAGS) test.c -o test.o

regrw.o : regrw.c
	gcc -c $(CFLAGS) regrw.c -o regrw.o

clean:
	rm -f *.o *~ core .depend test

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > $@


ifeq (.depend,$(wildcard .depend))
include .depend
endif
