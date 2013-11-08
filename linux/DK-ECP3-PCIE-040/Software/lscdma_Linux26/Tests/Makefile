
# Comment/uncomment the following line to enable/disable debugging
DEBUG = y

TEST_BINS = Open

# "-O" is needed to expand inlines
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG
else
  DEBFLAGS = -O2
endif

CFLAGS += $(DEBFLAGS) -I. -I../drvr


Open : Open.o
	gcc Open.o -o Open


Open.o : Open.c
	gcc -c $(CFLAGS) Open.c -o Open.o

ISR : ISR.o
	gcc ISR.o -o ISR

ISR.o : ISR.c
	gcc -c $(CFLAGS) ISR.c -o ISR.o

clean:
	rm -f *.o *~ core .depend $(TEST_BINS)

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > $@


ifeq (.depend,$(wildcard .depend))
include .depend
endif
