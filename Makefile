# $Id: Makefile,v 1.6 2002/03/11 13:34:24 jan Exp $

# add -DWITH_STRICMP if you are compiling i.e. with MS VC
# add -DDEBUG if you want lots of debugging infos
CFLAGS=

all: gen2shp

utils.o: utils.c utils.h
	$(CC) -c utils.c

gen2shp: gen2shp.c utils.o
	$(CC) $(CFLAGS) -o $@ gen2shp.c -lshp utils.o

check: gen2shp
	make -C tests all

clean:
	rm -f gen2shp *.o
	make -C tests clean
