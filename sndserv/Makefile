##########################################################
#
# $Id:$
#
# $Log:$
#
#

CC=gcc
#CFLAGS=-O -DNORMALUNIX -DLINUX
CFLAGS=-m32 -g -Wall -O -DNORMALUNIX -DLINUX
#LDFLAGS=
LDFLAGS=-L/usr/lib/i386-linux-gnu
LIBS=-lm

O=linux

all:	 $(O)/sndserver

clean:
	rm -f *.o *~ *.flc
	rm -f linux/*

# Target
$(O)/sndserver: \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/wadread.o \
	$(O)/linux.o
	$(CC) $(CFLAGS) $(LDFLAGS) \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/wadread.o \
	$(O)/linux.o -o $(O)/sndserver $(LIBS)
	echo make complete.

# Rule
$(O)/%.o: %.c
	mkdir -pv ${O}
	$(CC) $(CFLAGS) -c $< -o $@


