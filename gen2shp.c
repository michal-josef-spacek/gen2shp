/* Jan-Oliver Wagner	$Date: 1999/04/21 16:01:31 $
 * $Id: gen2shp.c,v 1.1 1999/04/21 16:01:31 jwagner Exp $
 *
 * Copyright (C) 1999 by Jan-Oliver Wagner
 * 
 *    This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Log: gen2shp.c,v $
 * Revision 1.1  1999/04/21 16:01:31  jwagner
 * Initial revision
 *
 */

#include <shapefil.h>

#include <utils.h>

#define VERSION "0.1.0 (RCS-$Revision: 1.1 $)"

#define	ERR_USAGE	1
#define ERR_TYPE	2
#define ERR_FORMAT	3

#define ERR_DBFCREATE	10
#define ERR_DBFADDFIELD	11
#define ERR_DBFOPEN	12
#define	ERR_DBFWRITEINTEGERATTRIBUTE	13

#define ERR_SHPOPEN	20

void print_version(FILE *file)
{
             fprintf(file,"gen2shp version " VERSION "\n"); 
             fprintf(file,"Copyright (C) 1999 by Jan-Oliver Wagner.\n"
             	    "The GNU GENERAL PUBLIC LICENSE applies."
             	    	"Absolutly No Warranty!\n");
#ifdef DEBUG
             fprintf(file,"compiled with option: DEBUG\n"); 
#endif
}

static DBFHandle LaunchDbf (	const char *fname ) {
	DBFHandle	hDBF;
	char		dbffname[255];
	char		fieldname[255];

	sprintf(dbffname, "%s.dbf", fname);
	sprintf(fieldname, "%s-id", fname);

	hDBF = DBFCreate( dbffname );
	if( hDBF == NULL ) {
		fprintf(stderr, "DBFCreate(%s) failed.\n", fname );
		exit(ERR_DBFCREATE);
	}

	if (DBFAddField( hDBF, fieldname, FTInteger, 11, 0 ) == -1) {
		fprintf(stderr, "DBFAddField(hDBF,%s,FTInteger,11,0) failed.\n", fieldname);
		exit(ERR_DBFADDFIELD);
	}

	DBFClose( hDBF );

	hDBF = DBFOpen( dbffname, "r+b" );
	if( hDBF == NULL ) {
		fprintf(stderr, "DBFOpen(%s,\"r+b\") failed.\n", dbffname );
		exit(ERR_DBFOPEN);
	}

	return hDBF;
}

static SHPHandle LaunchShp(	const char *fname ) {
	SHPHandle	hSHP;
	SHPObject	*psShape;
	char		shpfname[255];

	sprintf(shpfname, "%s.shp", fname);

	hSHP = SHPCreate( shpfname, SHPT_POINT );

	if( hSHP == NULL ) {
		fprintf(stderr, "SHPOpen(%s,\"SHPT_POINT\") failed.\n", shpfname );
		exit(ERR_SHPOPEN);
	}

	return hSHP;
}

static void WriteDbf (	DBFHandle hDBF,
			int rec,
			int id ) {
	if (! DBFWriteIntegerAttribute(hDBF, rec, 0, id)) {
		fprintf(stderr, "DBFWriteIntegerAttribute(hDBFs,%d,1,%d) failed.\n", rec, id );
		exit(ERR_DBFWRITEINTEGERATTRIBUTE);
	}
}

static void WritePoint(	SHPHandle hSHP,
			int rec,
			double x,
			double y ) {
	SHPObject	*psShape;

	psShape = SHPCreateObject( SHPT_POINT, rec, 0, NULL, NULL,
                               1, &x, &y, NULL, NULL );
	SHPWriteObject( hSHP, -1, psShape );
	SHPDestroyObject( psShape );
}

int main(	int argc,
		char ** argv ) {
	int id;
	double x, y;
	int result;
	int rec = 0;
	char linebuf[255];
	char * str;
	DBFHandle hDBF;
	SHPHandle hSHP;

	if (argc != 3) {
		print_version(stderr);
		fprintf(stderr, "usage: %s outfile type < infile\n", argv[0]);
		fprintf(stderr, "\treads stdin and creates outfile.shp, "
			"outfile.shx and outfile.dbf\n"
			"\ttype must one of these: points\n"
			"\tinfile must be in 'generate' format\n");
		exit(ERR_USAGE);
	}

	if (strcmp(argv[2], "points") != 0) {
		fprintf(stderr, "type '%s' unknown, use one these: points.", argv[2]);
		exit(ERR_TYPE);
	}

#ifdef DEBUG
	fprintf(stderr, "debug output: outfile=%s\n", argv[1]);
#endif

	hDBF = LaunchDbf(argv[1]);
	hSHP = LaunchShp(argv[1]);

	while (getline(stdin, linebuf) != EOF) {
		if (strcmp(linebuf, "end") == 0) {
#ifdef DEBUG
			fprintf(stderr, "debug output: 'end' detected\n");
#endif
			break;
		}
		if ((str = dtok(linebuf, ',')) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		id = atoi((const char *)str);

		if ((str = dtok(NULL, ',')) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		x = atof((const char *)str);

		if ((str = dtok(NULL, ',')) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		y = atof((const char *)str);

#ifdef DEBUG
		fprintf(stderr, "debug output: id=%d, x=%f, y=%f\n", id, x, y);
#endif

		WriteDbf(hDBF, rec, id);
		WritePoint(hSHP, rec, x, y);
		rec ++;
	}

	DBFClose( hDBF );
	SHPClose( hSHP );

	exit(0);
}
