# $Log: Makefile,v $
# Revision 1.1  1999/04/21 16:01:52  jwagner
# Initial revision
#

SHAPELIBPATH=/home/fkoorman/jfb/src/shapelib-1.2.4
TXT2DBFPATH=/home/jwagner/project/g/src/analysis/txt2dbf

CC=gcc
CFLAGS=-I$(SHAPELIBPATH) -I$(TXT2DBFPATH)/include # -DDEBUG

all: gen2shp

gen2shp: gen2shp.c
	$(CC) $(CFLAGS) -o $@ $? $(SHAPELIBPATH)/shpopen.o\
		$(SHAPELIBPATH)/dbfopen.o $(TXT2DBFPATH)/lib/utils.o

test: gen2shp
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	gen2shp pnttest points < pnttest.gen

clean:
	rm -f gen2shp pnttest.dbf pnttest.shp pnttest.shx
