# $Id: Makefile,v 1.5 2000/06/12 13:24:08 jan Exp $

# add -DWITH_STRICMP if you are compiling i.e. with MS VC
# add -DDEBUG if you want lots of debugging infos
CFLAGS=

all: gen2shp

utils.o: utils.c utils.h
	$(CC) -c utils.c

gen2shp: gen2shp.c utils.o
	$(CC) $(CFLAGS) -o $@ -lshp gen2shp.c utils.o

test: gen2shp
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	./gen2shp pnttest points < pnttest.gen
	rm -f arctest.dbf arctest.shp arctest.shx
	./gen2shp arctest lines < arctest.gen
	rm -f plytest.dbf plytest.shp plytest.shx
	./gen2shp plytest polygons < plytest.gen

clean:
	rm -f gen2shp *.o
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	rm -f arctest.dbf arctest.shp arctest.shx
	rm -f plytest.dbf plytest.shp plytest.shx
