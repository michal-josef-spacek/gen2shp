# $Id: Makefile,v 1.1 2002/01/24 17:16:38 jan Exp $

all:
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	../gen2shp pnttest points < pnttest.gen
	rm -f arctest.dbf arctest.shp arctest.shx
	../gen2shp arctest lines < arctest.gen
	rm -f plytest.dbf plytest.shp plytest.shx
	../gen2shp plytest polygons < plytest.gen

clean:
	rm -f pnttest.dbf pnttest.shp pnttest.shx
	rm -f arctest.dbf arctest.shp arctest.shx
	rm -f plytest.dbf plytest.shp plytest.shx
