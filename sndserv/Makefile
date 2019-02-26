##########################################################
#
# $Id:$
#
# $Log:$
#
#

CC=gcc
#CFLAGS=-O -DNORMALUNIX -DLINUX
SDL_CFLAGS=`sdl-config --cflags`
CFLAGS=-m32 -g -Wall -O -DNORMALUNIX -DLINUX $(SDL_CFLAGS)
#LDFLAGS=
LDFLAGS=-L/usr/lib/i386-linux-gnu
SDL_LIBS=`sdl-config --libs` -lSDL_mixer
LIBS=-lm $(SDL_LIBS)

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
	$(O)/sdl.o
	$(CC) $(CFLAGS) $(LDFLAGS) \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/wadread.o \
	$(O)/sdl.o -o $(O)/sndserver $(LIBS)
	echo make complete.

# Rule
$(O)/%.o: %.c
	mkdir -pv ${O}
	$(CC) $(CFLAGS) -c $< -o $@


