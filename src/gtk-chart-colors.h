/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2014 Maxime DOYEN
 *
 *  This file is part of HomeBank.
 *
 *  HomeBank is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  HomeBank is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GTK_COLORS_H__
#define __GTK_COLORS_H__

#include <cairo.h>

#define MASKCOL 255
#define COLTO16(col8) ( (col8 | col8<<8 ) )
#define COLTOOVER(col8) ( (col8 + MASKCOL) / 2 )


#define OVER_ALPHA .15
#define OVER_COLOR (MASKCOL * OVER_ALPHA)
#define COLTOCAIRO(col8) 	 ( (col8 / 255.0) )
#define COLTOCAIROOVER(col8) ( ((col8 * (1 - OVER_ALPHA)) + OVER_COLOR ) / 255.0 )

/* new stuff */
#define CHART_FONT_SIZE_NORMAL  10
#define CHART_FONT_SIZE_TITLE   18
#define CHART_FONT_SIZE_PERIOD  11


//typedef struct _rgbcol RgbCol;
	
struct rgbcol
{
	guint8	r, g, b;
};



enum {
	BLACK,
	WHITE,
	GREY1,
	TEXT,
	XYLINES,
	THBASE,
	THTEXT
};

enum colmap
{
	CHART_COLMAP_HOMEBANK,
	CHART_COLMAP_MSMONEY,
	CHART_COLMAP_SAP,
	CHART_COLMAP_QUICKEN,
	CHART_COLMAP_OFFICE2010,
	CHART_COLMAP_OFFICE2013,
	CHART_COLMAP_ANALYTICS,
};

extern char *chart_colors[];

extern struct rgbcol global_colors[];
extern struct rgbcol money_colors[];
extern struct rgbcol quicken_colors[];
extern struct rgbcol analytics_colors[];
extern struct rgbcol office2010_colors[];
extern struct rgbcol office2013_colors[];
extern struct rgbcol sap_colors[];
extern struct rgbcol homebank_colors[];

extern int money_nbcolors;
extern int quicken_nbcolors;
extern int analytics_nbcolors;
extern int office2010_nbcolors;
extern int office2013_nbcolors;
extern int sap_nbcolors;
extern int homebank_nbcolors;

void cairo_user_set_rgbcol(cairo_t *cr, struct rgbcol *col);
void cairo_user_set_rgbacol(cairo_t *cr, struct rgbcol *col, double alpha);
void cairo_user_set_rgbcol_over(cairo_t *cr, struct rgbcol *col, gboolean over);

#endif /* __GTK_COLORS_H__ */