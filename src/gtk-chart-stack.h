/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2018 Maxime DOYEN
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

#ifndef __CHARTSTACK_H__
#define __CHARTSTACK_H__

#include "gtk-chart-colors.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	
/* Macro for casting a pointer to a GtkWidget or GtkWidgetClass pointer.
 * Macros for testing whether `widget' or `klass' are of type GTK_TYPE_WIDGET.
 */
#define GTK_TYPE_CHARTSTACK            (ui_chart_stack_get_type ())
#define GTK_CHARTSTACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHARTSTACK, ChartStack))
#define GTK_CHARTSTACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHARTSTACK, ChartStackClass)
#define GTK_IS_CHARTSTACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHARTSTACK))
#define GTK_IS_CHARTSTACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHARTSTACK))
#define GTK_CHARTSTACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CHARTSTACK, ChartStackClass))

typedef struct _ChartStack		ChartStack;
typedef struct _ChartStackClass	ChartStackClass;

typedef struct _StackItem	    StackItem;

typedef gchar (* ChartStackPrintIntFunc)    (gint value, gboolean minor);
typedef gchar (* ChartStackPrintDoubleFunc) (gdouble value, gboolean minor);

/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#define GTK_CHARTSTACK_BARW 		32
#define CHART_BUFFER_LENGTH 128


#define DEFAULT_DELAY 500           /* Default delay in ms */


#define CHART_MARGIN	18 //standard a4 margin
#define CHART_SPACING   6



struct _StackItem
{
	/* data part */
	gchar	 *label;
	gdouble  spent;
	gdouble	 budget;
	gdouble  result;
	gchar    *status;
	
	/* draw stuffs */
	gdouble  rate;
	gboolean warn;
		
	/* tmp datas */
	gdouble	 rawrate;
	
};



/* you should access only the entry and list fields directly */
struct _ChartStack
{
	/*< private >*/
	GtkBox			hbox;

	GtkWidget		*drawarea;
	GtkAdjustment	*adjustment;
	GtkWidget		*scrollbar;

	/* data storage */
	gint		nb_items;
	GArray		*items;

	/*gchar		**titles;
	gdouble		*spent;
	gdouble		*budget;*/

	gchar		*title;
	gchar		*subtitle;

	gchar	    *budget_title;
	gchar	    *result_title;

	gboolean	minor;
	guint32		kcur;
	gdouble		minor_rate;
	gchar		*minor_symbol;



	/* color datas */
	GtkColorScheme color_scheme;

	/* cairo default value */
	PangoFontDescription *pfd;
	gint				pfd_size;

	/* buffer surface */
	cairo_surface_t	 *surface;

	
	double barw, blkw;

	/* draw area coordinates */
	double	l, t, b, r, w, h;

	/* zones height */
	double  title_zh;
	double  subtitle_zh, subtitle_y;
	double  header_zh, header_y;
	double  item_zh;

	/* column width */
	double	cat_col_w;
	double  bud_col_w;
	double	res_col_w;
	double	rel_col_w;

	
	double  graph_width, graph_height;	//graph dimension
	gint	visible;

	gint	active, lastactive;

	gchar   buffer[CHART_BUFFER_LENGTH];

};


struct _ChartStackClass {
	GtkBoxClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GType      ui_chart_stack_get_type (void);


/* public function */
GtkWidget *ui_chart_stack_new(void);

void ui_chart_stack_set_color_scheme(ChartStack * chart, gint colorscheme);
void ui_chart_stack_set_dualdatas(ChartStack *chart, GtkTreeModel *model, gchar *coltitle1, gchar *coltitle2, gchar *title, gchar *subtitle);
void ui_chart_stack_set_title(ChartStack * chart, gchar *title);
void ui_chart_stack_set_subtitle(ChartStack * chart, gchar *subtitle);
void ui_chart_stack_set_barw(ChartStack * chart, gdouble barw);
void ui_chart_stack_show_minor(ChartStack * chart, gboolean minor);

void ui_chart_stack_set_minor_prefs(ChartStack * chart, gdouble rate, gchar *symbol);
void ui_chart_stack_set_currency(ChartStack * chart, guint32 kcur);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CHARTSTACK_H__ */
