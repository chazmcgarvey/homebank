/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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

#ifndef __GTK_CHART_H__
#define __GTK_CHART_H__

#include "gtk-chart-colors.h"

G_BEGIN_DECLS
#define GTK_TYPE_CHART            (gtk_chart_get_type ())
#define GTK_CHART(obj)			  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHART, GtkChart))
#define GTK_CHART_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHART, GtkChartClass)
#define GTK_IS_CHART(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHART))
#define GTK_IS_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHART))
#define GTK_CHART_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CHART, GtkChartClass))

typedef struct _GtkChart		GtkChart;
typedef struct _GtkChartClass	GtkChartClass;
//typedef struct _GtkChartPrivate	GtkChartPrivate;

typedef struct _ChartItem	    ChartItem;
typedef gchar (* GtkChartPrintIntFunc)    (gint value, gboolean minor);
typedef gchar (* GtkChartPrintDoubleFunc) (gdouble value, gboolean minor);

/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* phi value */
#define PHI 1.61803399


/* default zoomx for charts */
#define GTK_CHART_BARW 			24

#define GTK_CHART_MINBARW 		 4
#define GTK_CHART_MAXBARW 		128

#define GTK_CHART_MINRADIUS 	64


#define CHART_BUFFER_LENGTH 128

// for cairo pie
#define PIE_LINE_SLICE 0
#define SOFT_LIGHT  0
#define GRADIENT	0
#define CHART_PIE_DONUT	 1


/* new stuff */
#define CHART_MARGIN	18 //standard a4 margin
#define CHART_SPACING   6

//#define PROP_SHOW_MINOR		6
//#define PROP_SHOW_LEGEND	7

enum
{
	CHART_TYPE_NONE,
	CHART_TYPE_COL,
	CHART_TYPE_PIE,
	CHART_TYPE_LINE,
	CHART_TYPE_MAX
};


enum
{
	LST_LEGEND_FAKE,
	LST_LEGEND_COLOR,
	LST_LEGEND_TITLE,
	LST_LEGEND_AMOUNT,
	LST_LEGEND_RATE,
	NUM_LST_LEGEND
};


struct _ChartItem
{
	/* data part */
	gchar	 *label;
	gdouble  serie1;
	gdouble	 serie2;
	gdouble	 rate;

	/* draw stuffs */
	gchar    *legend;
	double	 angle2;	  /* rate for pie */
	double	 height;   /* for column */ 
};


struct _GtkChart
{
	//own widget here

	/*< private >*/
	//GtkChartPrivate *priv;


	/* all below should be in priv normally */
	GtkBox			hbox;

	GtkWidget		*drawarea;
	GtkAdjustment	*adjustment;
	GtkWidget		*scrollbar;

	GtkWidget		*scrollwin;
	GtkWidget		*treeview;
	GtkTreeModel	*legend;

	/* data storage */
	gint		nb_items;
	GArray		*items;

	gchar		*title;
	gchar		*subtitle;

	/* chart properties */
	gint		type;
	gboolean	dual;
	gboolean	abs;
	gboolean	show_over;
	gboolean	show_xval;
	gboolean	show_mono;
	gint		every_xval;
	guint32		kcur;
	gboolean	minor;
	gdouble		minor_rate;
	gchar		*minor_symbol;

	/* color datas */
	GtkColorScheme color_scheme;
	
	/* cairo default value */
	PangoFontDescription *pfd;
	gint				pfd_size;

	/* buffer surface */
	cairo_surface_t	 *surface;
	
	/* draw area coordinates */
	double	l, t, b, r, w, h;
	/* our drawing rectangle with margin */
	double		label_w;
	double		legend_w;

	/* zones height */
	double		title_zh;
	double  subtitle_zh, subtitle_y;


	double		ox, oy;
	gint		lastx, lasty, lastactive;
	gint		lastpress_x, lastpress_y;
	gint		active;
	guint		timer_tag;

	/* pie specifics */
	gdouble		total;
	gint		rayon, left, top;

	/* bar specifics */
	double	rawmin, rawmax, range, min, max, unit, minimum;
	gint	div;
	gint	visible;

	double font_h;

	double scale_x, scale_y, scale_w, scale_h;
	double graph_x, graph_y, graph_width, graph_height;	//graph dimension
	double barw, blkw, posbarh, negbarh;

	gchar			buffer1[CHART_BUFFER_LENGTH];
	gchar			buffer2[CHART_BUFFER_LENGTH];
};

struct _GtkChartClass
{
	GtkBoxClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GType gtk_chart_get_type (void) G_GNUC_CONST;

/* public function */
GtkWidget *gtk_chart_new(gint type);

void gtk_chart_set_type(GtkChart *chart, gint type);
void gtk_chart_set_color_scheme(GtkChart * chart, gint colorscheme);

void gtk_chart_queue_redraw(GtkChart *chart);

void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column, gchar *title, gchar *subtitle);
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2, gchar *title, gchar *subtitle);

void gtk_chart_set_minor_prefs(GtkChart * chart, gdouble rate, gchar *symbol);
void gtk_chart_set_currency(GtkChart * chart, guint32 kcur);

void gtk_chart_set_overdrawn(GtkChart * chart, gdouble minimum);
void gtk_chart_set_every_xval(GtkChart * chart, gint decay);
void gtk_chart_set_barw(GtkChart * chart, gdouble barw);
void gtk_chart_set_showmono(GtkChart * chart, gboolean mono);

void gtk_chart_show_legend(GtkChart * chart, gboolean visible, gboolean showextracol);
void gtk_chart_show_overdrawn(GtkChart * chart, gboolean visible);
void gtk_chart_show_xval(GtkChart * chart, gboolean visible);
void gtk_chart_show_minor(GtkChart * chart, gboolean minor);
void gtk_chart_set_absolute(GtkChart * chart, gboolean abs);

G_END_DECLS
#endif /* __GTK_CHART_H__ */
