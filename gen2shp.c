/* 
 * $Id: gen2shp.c,v 1.11 2002/07/22 10:44:49 jan Exp $
 *
 * Copyright (C) 1999-2002 by Jan-Oliver Wagner <jan@intevation.de>
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
 */

#include <string.h>

#include <libshp/shapefil.h>

#include "utils.h"

#define VERSION "0.3.1-cvs"

#ifdef DEBUG
#define DEBUG_OUT(str) fprintf(stderr, "gen2shp debug: " str)
#define DEBUG_OUT1(str,v) fprintf(stderr, "gen2shp debug: " str, v)
#define DEBUG_OUT2(str,v,w) fprintf(stderr, "gen2shp debug: " str, v, w)
#define DEBUG_OUT3(str,v,w,x) fprintf(stderr, "gen2shp debug: " str, v, w, x)
#else
#define DEBUG_OUT(str) 
#define DEBUG_OUT1(str,v) 
#define DEBUG_OUT2(str,v,w) 
#define DEBUG_OUT3(str,v,w,x) 
#endif

/* Error codes for exit() routine: */
#define	ERR_USAGE	1
#define ERR_TYPE	2
#define ERR_FORMAT	3
#define ERR_OBJECTTYPE	4
#define ERR_ALLOC	5

#define ERR_DBFCREATE	10
#define ERR_DBFADDFIELD	11
#define ERR_DBFOPEN	12
#define	ERR_DBFWRITEINTEGERATTRIBUTE	13

#define ERR_SHPOPEN	20

/* Object Type codes used in main(): */
#define OBJECTTYPE_NONE		0
#define OBJECTTYPE_POINT	1
#define OBJECTTYPE_LINE		2
#define OBJECTTYPE_POLYGON	3

/* minimum number of coordinates allocated blockwise */
#define COORDS_BLOCKSIZE	100

/* maximum length for read strings,
 * if input lines with more characters appear,
 * errors are likely to occur */
#define STR_BUFFER_SIZE		300

#ifdef USE_STRICMP
#define CASE_INSENSITIVE_STR_CMP	stricmp
#else
#define CASE_INSENSITIVE_STR_CMP	strcasecmp
#endif

void print_version(FILE *file)
{
	fprintf(file,"gen2shp version " VERSION "\n"); 
	fprintf(file,"Copyright (C) 1999-2002 by Jan-Oliver Wagner.\n"
		"The GNU GENERAL PUBLIC LICENSE applies. "
		"Absolutly No Warranty!\n");
#ifdef DEBUG
	fprintf(file,"compiled with option: DEBUG\n"); 
#endif
}

static DBFHandle LaunchDbf (	const char *fname ) {
	DBFHandle	hDBF;
	char		dbffname[STR_BUFFER_SIZE];
	char		fieldname[STR_BUFFER_SIZE];

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

static SHPHandle LaunchShp(	const char *fname,
				int ObjectType ) {
	SHPHandle	hSHP;
	SHPObject	*psShape;
	char		shpfname[STR_BUFFER_SIZE];

	sprintf(shpfname, "%s.shp", fname);

	switch (ObjectType) {
		case OBJECTTYPE_POINT:
			hSHP = SHPCreate( shpfname, SHPT_POINT );
			break;
		case OBJECTTYPE_LINE:
			hSHP = SHPCreate( shpfname, SHPT_ARC );
			break;
		case OBJECTTYPE_POLYGON:
			hSHP = SHPCreate( shpfname, SHPT_POLYGON );
			break;
		default:
			fprintf(stderr, "internal error: "
				"unknown ObjectType=%d\n", ObjectType);
			exit(ERR_OBJECTTYPE);
	}

	if( hSHP == NULL ) {
		fprintf(stderr, "SHPOpen(%s, shape_type) failed.\n", shpfname );
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

static void WriteLine(	SHPHandle hSHP,
			int rec,
			int coords,
			double * x,
			double * y ) {
	SHPObject	*psShape;

	psShape = SHPCreateObject( SHPT_ARC, rec, 0, NULL, NULL,
		coords, x, y, NULL, NULL );
	SHPWriteObject( hSHP, -1, psShape );
	SHPDestroyObject( psShape );
}

static void WritePolygon(	SHPHandle hSHP,
				int rec,
				int coords,
				double * x,
				double * y,
				int nparts,
				int * partstarts) {
	SHPObject	*psShape;

DEBUG_OUT1("WritePolygon: rec = %d\n", rec);
DEBUG_OUT1("WritePolygon: nparts = %d\n", nparts);
DEBUG_OUT1("WritePolygon: coords = %d\n", coords);

	psShape = SHPCreateObject( SHPT_POLYGON, rec, nparts, partstarts, NULL,
		coords, x, y, NULL, NULL );
	SHPWriteObject( hSHP, -1, psShape );
	SHPDestroyObject( psShape );
}

/* read from fp and generate point shapefile to hDBF/hSHP */
static void GeneratePoints (	FILE *fp,
				DBFHandle hDBF,
				SHPHandle hSHP ) {
	char linebuf[STR_BUFFER_SIZE];	/* buffer for line-wise reading from file */
	int id;			/* ID of point */
	double x, y;		/* coordinates of point */
	char * str;		/* tmp variable needed for assertions */
	char * dstr;		/* tmp variable needed to find out substrings */
	int rec = 0;		/* Counter for records */

	while (getline(fp, linebuf) != EOF) {
		if (CASE_INSENSITIVE_STR_CMP(linebuf, "end") == 0) {
			DEBUG_OUT("'end' detected\n");
			break;
		}
		if ((str = strtok(linebuf, " ,")) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		id = atoi((const char *)str);

		if ((str = strtok(NULL, " ,")) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		dstr = (char *)strchr((const char *)str, (char)'D');
		if (dstr) *dstr = 'E';
		x = atof((const char *)str);

		if ((str = strtok(NULL, " ,")) == NULL) {
			fprintf(stderr, "format error in line %d\n", rec + 1);
			exit(ERR_FORMAT);
		}
		dstr = (char *)strchr((const char *)str, (char)'D');
		if (dstr) *dstr = 'E';
		y = atof((const char *)str);

		DEBUG_OUT3("id=%d, x=%f, y=%f\n", id, x, y);

		WriteDbf(hDBF, rec, id);
		WritePoint(hSHP, rec, x, y);
		rec ++;
	}
}

/* parse a coordinate pair from a string */
static void GetCoordinatePair(
	int id,			/* ID of object */
	char * linebuf,	/* should contain the coordinate pair */
	double * x,		/* output x */
	double * y		/* output y */
	) {
	char * str;		/* tmp variable needed for assertions */
	char * dstr;	/* tmp variable needed to find out substrings */

	if ((str = strtok(linebuf, " ,")) == NULL) {
		fprintf(stderr, "format error for object with "
			"id=%d\n", id);
		exit(ERR_FORMAT);
	}
	dstr = (char *)strchr((const char *)str, (char)'D');
	if (dstr) *dstr = 'E';
		*x = atof((const char *)str);

	if ((str = strtok(NULL, " ,")) == NULL) {
		fprintf(stderr, "format error for object with "
			"id=%d\n", id);
		exit(ERR_FORMAT);
	}
	dstr = (char *)strchr((const char *)str, (char)'D');
	if (dstr) *dstr = 'E';
		*y = atof((const char *)str);

	DEBUG_OUT2("x=%f, y=%f\n", *x, *y);
}

/* read from fp and generate line/arc shapefile to hDBF/hSHP */
static void GenerateLines (	FILE *fp,
				DBFHandle hDBF,
				SHPHandle hSHP ) {
	char linebuf[STR_BUFFER_SIZE];	/* buffer for line-wise reading from file */
	int id;			/* ID of point */
	double	* x = NULL,
		* y = NULL;	/* coordinates arrays */
	int vector_size = 0;	/* current size of the vectors x and y */
	char * str;		/* tmp variable needed for assertions */
	char * dstr;		/* tmp variable needed to find out substrings */
	int rec = 0;		/* Counter for records */
	int coord = 0;		/* Counter for coordinates */

	/* loop lines */
	while (getline(fp, linebuf) != EOF) {
		if (CASE_INSENSITIVE_STR_CMP(linebuf, "end") == 0) {
			DEBUG_OUT("final 'end' detected\n");
			break;
		}

		/* IDs are in single lines */
		id = atoi((const char *)linebuf);

		DEBUG_OUT1("id=%d\n", id);

		coord = 0;

		/* loop coordinates of line 'id' */
		while (getline(fp, linebuf) != EOF) {
			if (CASE_INSENSITIVE_STR_CMP(linebuf, "end") == 0) {
				DEBUG_OUT("a lines 'end' detected\n");
				break;
			}

			/* allocate coordinate vectors if too small */
			if (vector_size <= coord) {
				vector_size += COORDS_BLOCKSIZE;
				x = realloc(x, vector_size * sizeof(double));
				y = realloc(y, vector_size * sizeof(double));
				if (x == NULL || y == NULL) {
					fprintf(stderr, "memory allocation failed\n");
					exit(ERR_ALLOC);
				}
			}

			GetCoordinatePair(id, linebuf, &x[coord], &y[coord]);

			coord ++;
		}
		WriteDbf(hDBF, rec, id);
		WriteLine(hSHP, rec, coord, x, y);
		rec ++;
	}

	free(x);
	free(y);
}

/* read from fp and generate polgon shapefile to hDBF/hSHP */
static void GeneratePolygons (	FILE *fp,
				DBFHandle hDBF,
				SHPHandle hSHP ) {
	char linebuf[STR_BUFFER_SIZE];	/* buffer for line-wise reading from file */
	int id = -1;			/* ID of polygon */
	int new_id;		/* tmp for checking id */
	double	* x = NULL,
		* y = NULL;	/* coordinates arrays */
	int vector_size = 0;	/* current size of the vectors x and y */
	int nparts = 0; /* number of parts */
	int * partstarts = NULL; /* indices where new parts start in x[],y[] */
	int rec = 0;		/* Counter for records */
	int coord = 0;		/* Counter for coordinates */

	/* loop polygons */
	while (getline(fp, linebuf) != EOF) {
		if (CASE_INSENSITIVE_STR_CMP(linebuf, "end") == 0) {
			DEBUG_OUT("final 'end' detected\n");
			break;
		}

		if (strchr(linebuf,',') == NULL) {
			/* we assume we found an id */
			if (id != -1) {
				/* now its time to create the last read object */
				WriteDbf(hDBF, rec, id);
				if (partstarts) partstarts[0] = 0;
				WritePolygon(hSHP, rec, coord, x, y,
					(nparts > 0 ? nparts+1 : 0), partstarts);
				free(partstarts); partstarts = NULL;
				rec ++;
			}
			/* IDs are on a single line */
			new_id = atoi((const char *)linebuf);
			if (new_id == -99999) {
				/* this ID is a special one.
				 * it introduces a hole in the previous
				 * polygon.
				 * FIXME: we need to handle this situation
				 */
				DEBUG_OUT1("special id=%d found\n", new_id);
			} else {
				id = new_id;
				coord = 0;
				nparts = 0;
				DEBUG_OUT1("id=%d\n", id);
			}
		} else {
				/* assume we found a coordinate pair.
				 * This basically means we are supposed to
				 * add this one and the following coordinates
				 * the the previous polygon as an additional 'part'. */
				if (coord == 0) {
						DEBUG_OUT("no id for coordinates!");
						exit(ERR_FORMAT);
				}
				nparts ++;	/* a new part starts */
				partstarts = realloc(partstarts, sizeof(int) * (nparts+1));
				if (partstarts == NULL) {
					fprintf(stderr, "memory allocation failed\n");
					exit(ERR_ALLOC);
				}
				partstarts[nparts] = coord;
				DEBUG_OUT1("newpart at %d\n", coord);

			/* the following block is just a copy from the while
			 * construct below. Should find a more elegant solution!
			 * ---------
			 */
			/* allocate coordinate vectors if too small */
			if (vector_size <= coord) {
				vector_size += COORDS_BLOCKSIZE;
				x = realloc(x, vector_size * sizeof(double));
				y = realloc(y, vector_size * sizeof(double));
				if (x == NULL || y == NULL) {
					fprintf(stderr, "memory allocation failed\n");
					exit(ERR_ALLOC);
				}
			}

			GetCoordinatePair(id, linebuf, &x[coord], &y[coord]);

			coord ++;
			/* ---------- end of copy */
		}

		/* loop coordinates of polygon 'id' */
		while (getline(fp, linebuf) != EOF) {
			if (CASE_INSENSITIVE_STR_CMP(linebuf, "end") == 0) {
				DEBUG_OUT("an 'end' detected\n");
				break;
			}

			/* allocate coordinate vectors if too small */
			if (vector_size <= coord) {
				vector_size += COORDS_BLOCKSIZE;
				x = realloc(x, vector_size * sizeof(double));
				y = realloc(y, vector_size * sizeof(double));
				if (x == NULL || y == NULL) {
					fprintf(stderr, "memory allocation failed\n");
					exit(ERR_ALLOC);
				}
			}

			GetCoordinatePair(id, linebuf, &x[coord], &y[coord]);

			coord ++;
		}
	}

	if (id != -1) {
		/* now its time to create the last object */
		WriteDbf(hDBF, rec, id);
		if (partstarts) partstarts[0] = 0;
		WritePolygon(hSHP, rec, coord, x, y, (nparts > 0 ? nparts+1 : 0), partstarts);
	}

	free(partstarts);
	free(x);
	free(y);
}

int main(	int argc,
		char ** argv ) {
	DBFHandle hDBF;		/* handle for dBase file */
	SHPHandle hSHP;		/* handle for shape files .shx and .shp */
	int ObjectType = OBJECTTYPE_NONE;

	if (argc != 3) {
		print_version(stderr);
		fprintf(stderr, "usage: %s outfile type < infile\n", argv[0]);
		fprintf(stderr, "\treads stdin and creates outfile.shp, "
			"outfile.shx and outfile.dbf\n"
			"\ttype must be one of these: points lines polygons\n"
			"\tinfile must be in 'generate' format\n");
		exit(ERR_USAGE);
	}

	/* determine Object Type: */
	if (strcmp(argv[2], "points") == 0) ObjectType = OBJECTTYPE_POINT;
	if (strcmp(argv[2], "lines") == 0) ObjectType = OBJECTTYPE_LINE;
	if (strcmp(argv[2], "polygons") == 0) ObjectType = OBJECTTYPE_POLYGON;
	if (ObjectType == OBJECTTYPE_NONE) {
		fprintf(stderr, "type '%s' unknown, use one of these: "
			"points lines polygons.", argv[2]);
		exit(ERR_TYPE);
	}

	DEBUG_OUT1("outfile=%s\n", argv[1]);
	DEBUG_OUT1("type=%s\n", argv[2]);

	/* Open and prepare output files */
	hDBF = LaunchDbf(argv[1]);
	hSHP = LaunchShp(argv[1], ObjectType);

	/* Call generate function */
	switch (ObjectType) {
		case OBJECTTYPE_POINT:
			GeneratePoints(stdin, hDBF, hSHP);
			break;
		case OBJECTTYPE_LINE:
			GenerateLines(stdin, hDBF, hSHP);
			break;
		case OBJECTTYPE_POLYGON:
			GeneratePolygons(stdin, hDBF, hSHP);
			break;
		default:
			fprintf(stderr, "internal error: "
				"unknown ObjectType=%d\n", ObjectType);
			exit(ERR_OBJECTTYPE);
	}

	/* Finish output files */
	DBFClose( hDBF );
	SHPClose( hSHP );

	/* success */
	exit(0);
}
