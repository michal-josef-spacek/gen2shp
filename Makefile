# $Log: Makefile,v $
# Revision 1.2  1999/04/22 15:33:31  jwagner
# Added further tests and utils module.
#
# Revision 1.1  1999/04/21  16:01:52  jwagner
# Initial revision
#

SHAPELIBPATH=/home/fkoorman/jfb/src/shapelib-1.2.4

CC=gcc
CFLAGS=-I$(SHAPELIBPATH) # -DDEBUG

all: gen2shp

utils.o: utils.c utils.h
	$(CC) -c utils.c

gen2shp: gen2shp.c utils.o
	$(CC) $(CFLAGS) -o $@ gen2shp.c  $(SHAPELIBPATH)/shpopen.o\
		$(SHAPELIBPATH)/dbfopen.o utils.o

test: gen2shp
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	gen2shp pnttest points < pnttest.gen
	rm -f arctest.dbf arctest.shp arctest.shx
	gen2shp arctest lines < arctest.gen
	rm -f plytest.dbf plytest.shp plytest.shx
	gen2shp plytest polygons < plytest.gen

clean:
	rm -f gen2shp *.o
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	rm -f arctest.dbf arctest.shp arctest.shx
	rm -f plytest.dbf plytest.shp plytest.shx
