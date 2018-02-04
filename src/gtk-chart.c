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


#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#include "homebank.h"
#include "gtk-chart-colors.h"
#include "gtk-chart.h"

#define MYDEBUG 0


#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif


#define DYNAMICS 1

#define DBGDRAW_RECT 0
#define DBGDRAW_TEXT 0
#define DBGDRAW_ITEM 0


static void gtk_chart_class_intern_init (gpointer);
static void gtk_chart_class_init (GtkChartClass *klass);
static void gtk_chart_init (GtkChart *chart);
static void gtk_chart_dispose (GObject * object);
static void gtk_chart_finalize (GObject * object);

static gboolean drawarea_configure_event_callback(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
static void drawarea_realize_callback(GtkWidget *widget, gpointer user_data);
static gboolean drawarea_draw_callback(GtkWidget *widget, cairo_t *wcr, gpointer user_data);
static gboolean drawarea_motionnotifyevent_callback(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static gboolean drawarea_querytooltip_callback(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data);

static gboolean drawarea_full_redraw(GtkWidget *widget, gpointer user_data);

static void chart_calculation(GtkChart *chart);
static void chart_clear(GtkChart *chart);

static void colchart_first_changed( GtkAdjustment *adj, gpointer user_data);
static void colchart_compute_range(GtkChart *chart);
static void colchart_calculation(GtkChart *chart);
static void colchart_scrollbar_setvalues(GtkChart *chart);

static void piechart_calculation(GtkChart *chart);




static GtkBoxClass *gtk_chart_parent_class = NULL;

static const double dashed3[] = {3.0};

GType
gtk_chart_get_type ()
{
static GType chart_type = 0;

	if (!chart_type)
    {
		static const GTypeInfo chart_info =
		{
		sizeof (GtkChartClass),
		NULL,		/* base_init */
		NULL,		/* base_finalize */
		(GClassInitFunc) gtk_chart_class_intern_init,
		NULL,		/* class_finalize */
		NULL,		/* class_data */
		sizeof (GtkChart),
		0,		/* n_preallocs */
		(GInstanceInitFunc) gtk_chart_init,
		NULL
		};

		chart_type = g_type_register_static (GTK_TYPE_BOX, "GtkChart",
							 &chart_info, 0);

	}
	return chart_type;
}

static void
gtk_chart_class_intern_init (gpointer klass)
{
	gtk_chart_parent_class = g_type_class_peek_parent (klass);
	gtk_chart_class_init ((GtkChartClass *) klass);
}

static void
gtk_chart_class_init (GtkChartClass * class)
{
GObjectClass *gobject_class = G_OBJECT_CLASS (class);
//GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
 
	DB( g_print("\n[gtkchart] class init\n") );

	//gobject_class->get_property = gtk_chart_get_property;
	//gobject_class->set_property = gtk_chart_set_property;
	gobject_class->dispose = gtk_chart_dispose;
	gobject_class->finalize = gtk_chart_finalize;
	
	//widget_class->size_allocate = gtk_chart_size_allocate;
	
}


static void
gtk_chart_init (GtkChart * chart)
{
GtkWidget *widget, *vbox, *frame;

	DB( g_print("\n[gtkchart] init\n") );


	chart->surface = NULL;
 	chart->nb_items = 0;
	chart->items = NULL;
 	chart->title = NULL;
 	
  	chart->pfd = NULL;
	chart->abs = FALSE;
	chart->dual = FALSE;
	chart->usrbarw = 0.0;
	chart->barw = GTK_CHART_BARW;
	
	chart->show_legend = TRUE;
	chart->show_legend_wide = FALSE;
	chart->show_mono = FALSE;
	chart->active = -1;
	chart->lastactive = -1;

 	chart->minor_rate = 1.0;
	chart->timer_tag = 0;

	gtk_chart_set_color_scheme(chart, CHART_COLMAP_HOMEBANK);
	
	widget=GTK_WIDGET(chart);

	gtk_box_set_homogeneous(GTK_BOX(widget), FALSE);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (widget), vbox, TRUE, TRUE, 0);

	/* drawing area */
	frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	chart->drawarea = gtk_drawing_area_new();
	//gtk_widget_set_double_buffered (GTK_WIDGET(widget), FALSE);
	
	gtk_container_add( GTK_CONTAINER(frame), chart->drawarea );
	gtk_widget_set_size_request(chart->drawarea, 100, 100 );
#if DYNAMICS == 1
	gtk_widget_set_has_tooltip(chart->drawarea, TRUE);
#endif
	gtk_widget_show(chart->drawarea);

#if MYDEBUG == 1
	/*GtkStyle *style;
	PangoFontDescription *font_desc;

	g_print("draw_area font\n");
	
	style = gtk_widget_get_style(GTK_WIDGET(chart->drawarea));
	font_desc = style->font_desc;

	g_print("family: %s\n", pango_font_description_get_family(font_desc) );
	g_print("size: %d (%d)\n", pango_font_description_get_size (font_desc), pango_font_description_get_size (font_desc )/PANGO_SCALE );
	*/
#endif
	
	/* scrollbar */
    chart->adjustment = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
    chart->scrollbar = gtk_scrollbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (chart->adjustment));
    gtk_box_pack_start (GTK_BOX (vbox), chart->scrollbar, FALSE, TRUE, 0);
	gtk_widget_show(chart->scrollbar);

	g_signal_connect( G_OBJECT(chart->drawarea), "configure-event", G_CALLBACK (drawarea_configure_event_callback), chart);
	g_signal_connect( G_OBJECT(chart->drawarea), "realize", G_CALLBACK(drawarea_realize_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "draw", G_CALLBACK(drawarea_draw_callback), chart ) ;

#if DYNAMICS == 1
	gtk_widget_add_events(GTK_WIDGET(chart->drawarea),
		GDK_EXPOSURE_MASK |
		//GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK
		//GDK_BUTTON_PRESS_MASK |
		//GDK_BUTTON_RELEASE_MASK
		);

	g_signal_connect( G_OBJECT(chart->drawarea), "query-tooltip", G_CALLBACK(drawarea_querytooltip_callback), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "motion-notify-event", G_CALLBACK(drawarea_motionnotifyevent_callback), chart );
#endif
	g_signal_connect (G_OBJECT(chart->adjustment), "value-changed", G_CALLBACK (colchart_first_changed), chart);

	//g_signal_connect( G_OBJECT(chart->drawarea), "map-event", G_CALLBACK(chart_map), chart ) ;
	//g_signal_connect( G_OBJECT(chart->drawarea), "button-press-event", G_CALLBACK(chart_button_press), chart );
	//g_signal_connect( G_OBJECT(chart->drawarea), "button-release-event", G_CALLBACK(chart_button_release), chart );
}


/* --- */

GtkWidget *
gtk_chart_new (gint type)
{
GtkChart *chart;

	DB( g_print("\n======================================================\n") );
	DB( g_print("\n[gtkchart] new\n") );

	chart = g_object_new (GTK_TYPE_CHART, NULL);
	chart->type = type;

	return GTK_WIDGET(chart);
}


static void
gtk_chart_dispose (GObject *gobject)
{
//GtkChart *chart = GTK_CHART (object);

	DB( g_print("\n[gtkchart] dispose\n") );

  /* In dispose(), you are supposed to free all types referenced from this
   * object which might themselves hold a reference to self. Generally,
   * the most simple solution is to unref all members on which you own a 
   * reference.
   */

  /* dispose() might be called multiple times, so we must guard against
   * calling g_object_unref() on an invalid GObject by setting the member
   * NULL; g_clear_object() does this for us, atomically.
   */
  //g_clear_object (&self->priv->an_object);

  /* Always chain up to the parent class; there is no need to check if
   * the parent class implements the dispose() virtual function: it is
   * always guaranteed to do so
   */
  G_OBJECT_CLASS (gtk_chart_parent_class)->dispose (gobject);
}


static void
gtk_chart_finalize (GObject * object)
{
GtkChart *chart = GTK_CHART (object);

	DB( g_print("\n[gtkchart] finalize\n") );

	chart_clear(chart);

	if(chart->pfd)
	{
		pango_font_description_free (chart->pfd);
		chart->pfd = NULL;
	}
	
	if (chart->surface)
	{
		cairo_surface_destroy (chart->surface);
		chart->surface = NULL;
	}

	G_OBJECT_CLASS (gtk_chart_parent_class)->finalize (object);
}


/*
** print a integer number
*/
static gchar *chart_print_int(GtkChart *chart, gint value)
{
	hb_strfmon_int(chart->buffer1, CHART_BUFFER_LENGTH-1, (gdouble)value, chart->kcur, chart->minor);
	return chart->buffer1;
}


/*
** print a rate number
*/
static gchar *chart_print_rate(GtkChart *chart, gdouble value)
{
	g_snprintf (chart->buffer1, CHART_BUFFER_LENGTH-1, "%.2f%%", value);
	return chart->buffer1;
}


/*
** print a double number
*/
static gchar *chart_print_double(GtkChart *chart, gchar *buffer, gdouble value)
{
	hb_strfmon(buffer, CHART_BUFFER_LENGTH-1, value, chart->kcur, chart->minor);
	return buffer;
}


/*
** clear any allocated memory
*/
static void chart_clear(GtkChart *chart)
{
gint i;

	DB( g_print("\n[gtkchart] clear\n") );

	//free & clear any previous allocated datas
	if(chart->title != NULL)
	{
		g_free(chart->title);
		chart->title = NULL;
	}

	if(chart->subtitle != NULL)
	{
		g_free(chart->subtitle);
		chart->subtitle = NULL;
	}

	if(chart->items != NULL)
	{
		for(i=0;i<chart->nb_items;i++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, i);

			g_free(item->label);	//we free label as it comes from a model_get into setup_with_model
			g_free(item->legend);
		}		
		g_array_free(chart->items, TRUE);
		chart->items =  NULL;
	}

	chart->nb_items = 0;

	chart->total = 0;
	chart->range = 0;
	chart->rawmin = 0;
	chart->rawmax = 0;
	chart->every_xval = 7;

	chart->active = -1;
	chart->lastactive = -1;

}


/*
** setup our chart with a model and column
*/
static void chart_setup_with_model(GtkChart *chart, GtkTreeModel *list_store, guint column1, guint column2)
{
gint i;
gboolean valid;
GtkTreeIter iter;

	DB( g_print("\n[chart] setup with model\n") );

	chart_clear(chart);
	
	chart->nb_items = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_store), NULL);

	chart->items = g_array_sized_new(FALSE, FALSE, sizeof(ChartItem), chart->nb_items);

	DB( g_print(" nb=%d, struct=%d\n", chart->nb_items, sizeof(ChartItem)) );

	chart->dual = (column1 == column2) ? FALSE : TRUE;

	/* Get the first iter in the list */
	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store), &iter);
	i = 0;
	while (valid)
    {
	gint id;
	gchar *label;
	gdouble value1, value2;
	ChartItem item;
		
		/* column 0: pos (gint) */
		/* column 1: key (gint) */
		/* column 2: label (gchar) */
		/* column x: values (double) */

		gtk_tree_model_get (GTK_TREE_MODEL(list_store), &iter,
			0, &id,
			2, &label,
			column1, &value1,
			column2, &value2,
			-1);

		if(chart->dual || chart->abs)
		{
			value1 = ABS(value1);
			value2 = ABS(value2);
		}
		
		DB( g_print("%d: '%s' %.2f %2f\n", i, label, value1, value2) );

		/* data1 value storage & min, max compute */
		chart->rawmin = MIN(chart->rawmin, value1);
		chart->rawmax = MAX(chart->rawmax, value1);

		if( chart->dual )
		{
			/* data2 value storage & min, max compute */
			chart->rawmin = MIN(chart->rawmin, value2);
			chart->rawmax = MAX(chart->rawmax, value2);
		}

		item.label = label;
		item.serie1 = value1;
		item.serie2 = value2;
		g_array_append_vals(chart->items, &item, 1);

		/* ensure rawmin rawmax not equal */
		if(chart->rawmin == chart->rawmax)
		{
			chart->rawmin = 0;
			chart->rawmax = 100;
		}

		/* pie chart total sum */
		chart->total += ABS(value1);
		
		//leak
		//don't g_free(label); here done into chart_clear
		
		valid = gtk_tree_model_iter_next (list_store, &iter);
		i++;
	}

	// compute rate for legend for bar/pie
	for(i=0;i<chart->nb_items;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
	gchar *strval;

		strval = chart_print_double(chart, chart->buffer1, item->serie1);
		item->rate = ABS(item->serie1*100/chart->total);
		item->legend = g_markup_printf_escaped("%s\n%s (%.f%%)", item->label, strval, item->rate);
	}

	//g_print("total is %.2f\n", total);
	//ensure the widget is mapped
	//gtk_widget_map(chart);

}


static void chart_set_font_size(GtkChart *chart, PangoLayout *layout, gint font_size)
{
gint size = 10;

	//TODO: use PANGO_SCALE INSTEAD

	//DB( g_print("\n[chart] set font size\n") );

	switch(font_size)
	{
		case CHART_FONT_SIZE_TITLE:
			size = chart->pfd_size + 3;
			break;
		case CHART_FONT_SIZE_SUBTITLE:
			size = chart->pfd_size + 1;
			break;
		case CHART_FONT_SIZE_NORMAL:
			size = chart->pfd_size - 1;
			break;
		case CHART_FONT_SIZE_SMALL:
			size = chart->pfd_size - 2;
			break;
	}

	//DB( g_print(" size=%d\n", size) );

	pango_font_description_set_size(chart->pfd, size * PANGO_SCALE);

	pango_layout_set_font_description (layout, chart->pfd);

}


/*
** recompute according to type
*/
static void chart_recompute(GtkChart *chart)
{

	DB( g_print("\n[gtkchart] recompute\n") );


	if( !gtk_widget_get_realized(chart->drawarea) || chart->surface == NULL )
		return;

	chart_calculation (chart);
	
	switch(chart->type)
	{
		case CHART_TYPE_LINE:
		case CHART_TYPE_COL:
			colchart_calculation(chart);
			gtk_adjustment_set_value(chart->adjustment, 0);
			colchart_scrollbar_setvalues(chart);
			gtk_widget_show(chart->scrollbar);
			break;
		case CHART_TYPE_PIE:
			piechart_calculation(chart);
			gtk_widget_hide(chart->scrollbar);
			break;
	}
}




/* bar section */


static float CalculateStepSize(float range, float targetSteps)
{
    // calculate an initial guess at step size
    float tempStep = range/targetSteps;

    // get the magnitude of the step size
    float mag = (float)floor(log10(tempStep));
    float magPow = (float)pow(10, mag);

    // calculate most significant digit of the new step size
    float magMsd = (int)(tempStep/magPow + 0.5);

    // promote the MSD to either 1, 2, or 5
    if (magMsd > 5.0)
        magMsd = 10.0f;
    else if (magMsd > 2.0)
        magMsd = 5.0f;
    else if (magMsd >= 1.0)
        magMsd = 2.0f;

    return magMsd*magPow;
}


static void colchart_compute_range(GtkChart *chart)
{
double lobound=chart->rawmin, hibound=chart->rawmax;

	DB( g_print("\n[column] compute range\n") );

	/* comptute max ticks */
	chart->range = chart->rawmax - chart->rawmin;
	gint maxticks = MIN(10,floor(chart->graph.height / (chart->font_h * 2)));

	DB( g_print(" raw :: [%.2f - %.2f] range=%.2f\n", chart->rawmin, chart->rawmax, chart->range) );
	DB( g_print(" raw :: maxticks=%d (%g / (%g*2))\n", maxticks, chart->graph.height, chart->font_h) );

	DB( g_print("\n") );
	chart->unit  = CalculateStepSize((hibound-lobound), maxticks);
	chart->min   = -chart->unit * ceil(-lobound/chart->unit);
	chart->max   = chart->unit * ceil(hibound/chart->unit);
	chart->range = chart->max - chart->min;
	chart->div   = chart->range / chart->unit;
	
	DB( g_print(" end :: interval=%.2f, ticks=%d\n", chart->unit, chart->div) );
	DB( g_print(" end :: [%.2f - %.2f], range=%.2f\n", chart->min, chart->max, chart->range) );

}


static void chart_calculation(GtkChart *chart)
{
GtkWidget *drawarea = chart->drawarea;
GdkWindow *gdkwindow;
cairo_surface_t *surf = NULL;
cairo_t *cr;
int tw, th;
GtkAllocation allocation;
PangoLayout *layout;
gchar *valstr;
gint i;

	
	DB( g_print("\n[gtkchart] calculation\n") );

	gtk_widget_get_allocation(drawarea, &allocation);
	
	chart->l = CHART_MARGIN;
	chart->t = CHART_MARGIN;
	chart->r = allocation.width - CHART_MARGIN;
	chart->b = allocation.height - CHART_MARGIN;
	chart->w = allocation.width - (CHART_MARGIN*2);
	chart->h = allocation.height - (CHART_MARGIN*2);

	gdkwindow = gtk_widget_get_window(chart->drawarea);
	if(!gdkwindow)
	{
		surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
		cr = cairo_create (surf);
	}	
	else
		cr = gdk_cairo_create (gdkwindow);

	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (cr);


	// compute title
	chart->title_zh = 0;
	if(chart->title != NULL)
	{
		chart_set_font_size(chart, layout, CHART_FONT_SIZE_TITLE);
		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->title_zh = (th / PANGO_SCALE);
		DB( g_print(" - title: %s w=%d h=%d\n", chart->title, tw, th) );
	}

	// compute subtitle
	chart->subtitle_zh = 0;
	if(chart->subtitle != NULL)
	{
		chart_set_font_size(chart, layout, CHART_FONT_SIZE_SUBTITLE);
		pango_layout_set_text (layout, chart->subtitle, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->subtitle_zh = (th / PANGO_SCALE);
		DB( g_print(" - title: %s w=%d h=%d\n", chart->subtitle, tw, th) );
	}
	chart->subtitle_y = chart->t + chart->title_zh;


	chart->graph.y = chart->t + chart->title_zh + chart->subtitle_zh;
	chart->graph.height = chart->h - chart->title_zh - chart->subtitle_zh;

	if(chart->title_zh > 0 || chart->subtitle_zh > 0)
	{
		chart->graph.y += CHART_MARGIN;
		chart->graph.height -= CHART_MARGIN;
	}


	// compute other text
	chart_set_font_size(chart, layout, CHART_FONT_SIZE_NORMAL);

	// y-axis labels (amounts)
	chart->scale_w = 0;
	colchart_compute_range(chart);

	valstr = chart_print_int(chart, (gint)chart->min);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->scale_w = (tw / PANGO_SCALE);

	valstr = chart_print_int(chart, (gint)chart->max);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->scale_w = MAX(chart->scale_w, (tw / PANGO_SCALE));

	DB( g_print(" - scale: %d,%d %g,%g\n", chart->l, 0, chart->scale_w, 0.0) );

	// todo: compute maxwidth of item labels
	double label_w = 0;
	for(i=0;i<chart->nb_items;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
	
		// label width
		pango_layout_set_text (layout, item->label, -1);
		pango_layout_get_size (layout, &tw, &th);
		label_w = MAX(label_w, (tw / PANGO_SCALE));
	}
	chart->label_w = label_w;

	// compute font height
	chart->font_h = (th / PANGO_SCALE);

	// compute graph region
	switch(chart->type)
	{
		case CHART_TYPE_LINE:
		case CHART_TYPE_COL:
			chart->graph.x = chart->l + chart->scale_w + 2;
			chart->graph.width  = chart->w - chart->scale_w - 2;
			break;
		case CHART_TYPE_PIE:
			chart->graph.x = chart->l;
			chart->graph.width  = chart->w;
			break;	
	}

	DB( g_print(" - graph : %g,%g %g,%g\n", chart->graph.x, chart->graph.y, chart->graph.width, chart->graph.height) );


	if( ((chart->type == CHART_TYPE_LINE) || (chart->type == CHART_TYPE_COL)) && chart->show_xval)
		chart->graph.height -= (chart->font_h + CHART_SPACING);

	// compute: each legend column width, and legend width
	if(chart->show_legend)
	{
		chart_set_font_size(chart, layout, CHART_FONT_SIZE_SMALL);

		//we compute the rate text here to get the font height
		pango_layout_set_text (layout, "00.00 %", -1);
		pango_layout_get_size (layout, &tw, &th);

		chart->legend_font_h = (th / PANGO_SCALE);
		
		// labels not much than 1/4 of graph
		gdouble lw = floor(chart->graph.width / 4);
		chart->legend_label_w = MIN(chart->label_w, lw);

		chart->legend.width = chart->legend_font_h + CHART_SPACING + chart->legend_label_w;
		chart->legend.height = MIN(floor(chart->nb_items * chart->legend_font_h * CHART_LINE_SPACING), chart->graph.height);

		if(chart->show_legend_wide )
		{
			chart->legend_value_w = chart->scale_w;
			chart->legend_rate_w = (tw / PANGO_SCALE);
			chart->legend.width += CHART_SPACING + chart->legend_value_w + CHART_SPACING + chart->legend_rate_w;
		}

		//if legend visible, substract
		chart->graph.width -= (chart->legend.width + CHART_MARGIN);

		chart->legend.x = chart->graph.x + chart->graph.width + CHART_MARGIN;	
		chart->legend.y = chart->graph.y;
	
		DB( g_print(" - graph : %g %g %g %g\n", chart->graph.x, chart->graph.y, chart->graph.width, chart->graph.height ) );
		DB( g_print(" - legend: %g %g %g %g\n", chart->legend.x, chart->legend.y, chart->legend.width, chart->legend.height ) );
	}
	
	g_object_unref (layout);

	cairo_destroy(cr);
	cairo_surface_destroy(surf);
}


static void colchart_calculation(GtkChart *chart)
{
gint blkw, maxvisi;

	DB( g_print("\n[column] calculation\n") );

	/* from fusionchart
	the bar has a default width of 41
	min space is 3 and min barw is 8
	*/

	// new computing
	if( chart->usrbarw > 0.0 )
	{
		blkw = chart->usrbarw;
		chart->barw = blkw - 3;
	}
	else
	{
		//minvisi = floor(chart->graph.width / (GTK_CHART_MINBARW+3) );
		maxvisi = floor(chart->graph.width / (GTK_CHART_MAXBARW+3) );

		DB( g_print(" width=%.2f, nb=%d, minvisi=%d, maxvisi=%d\n", chart->graph.width, chart->nb_items, minvisi, maxvisi) );
		
		if( chart->nb_items <= maxvisi )
		{
			chart->barw = GTK_CHART_MAXBARW;
			blkw = GTK_CHART_MAXBARW + ( chart->graph.width - (chart->nb_items*GTK_CHART_MAXBARW) ) / chart->nb_items;
		}
		else
		{
			blkw = MAX(GTK_CHART_MINBARW, floor(chart->graph.width / chart->nb_items));
			chart->barw = blkw - 3;
		}
	}

	if(chart->dual)
		chart->barw = chart->barw / 2;

	DB( g_print(" blkw=%d, barw=%2.f\n", blkw, chart->barw) );


	chart->blkw = blkw;
	chart->visible = chart->graph.width / blkw;
	chart->visible = MIN(chart->visible, chart->nb_items);

	chart->ox = chart->l;
	chart->oy = chart->b;
	if(chart->range > 0)
		chart->oy = floor(chart->graph.y + (chart->max/chart->range) * chart->graph.height);

	DB( g_print(" + ox=%f oy=%f\n", chart->ox, chart->oy) );

	/* todo: hack on every xval */
	if(chart->label_w > 0 && chart->visible > 0)
	{
		if( chart->label_w <= chart->blkw )
			chart->every_xval = 1;
		else
			chart->every_xval = floor( 0.5 + (chart->label_w + CHART_SPACING) / chart->blkw);

		DB( g_print(" vis=%3d/%3d, xlabel_w=%g, blk_w=%g :: everyxval=%d\n", chart->visible, chart->nb_items, chart->label_w, chart->blkw, chart->every_xval) );
	}
}


/*
** draw the scale
*/
static void colchart_draw_scale(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;
double x, y;
gdouble curxval;
gint i, first;
PangoLayout *layout;
gchar *valstr;
int tw, th;


	DB( g_print("\n[column] draw scale\n") );

	cr = cairo_create (chart->surface);
	layout = pango_cairo_create_layout (cr);
	chart_set_font_size(chart, layout, CHART_FONT_SIZE_NORMAL);
	cairo_set_line_width (cr, 1);
	
	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);

	// Y-scale lines + labels (amounts)
	curxval = chart->max;
	cairo_set_dash(cr, 0, 0, 0);
	for(i=0 ; i<=chart->div ; i++)
	{
		y = 0.5 + floor (chart->graph.y + ((i * chart->unit) / chart->range) * chart->graph.height);
		//DB( g_print(" + i=%d :: y=%f (%f / %f) * %f\n", i, y, i*chart->unit, chart->range, chart->graph.height) );

		// curxval = 0.0 is x-axis
		cairo_user_set_rgbacol (cr, &global_colors[THTEXT], ( curxval == 0.0 ) ? 0.8 : 0.1);
		cairo_move_to (cr, chart->graph.x, y);
		cairo_line_to (cr, chart->graph.x + chart->graph.width, y);
		cairo_stroke (cr);

		cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
		valstr = chart_print_int(chart, (gint)curxval);
		pango_layout_set_text (layout, valstr, -1);
		pango_layout_get_size (layout, &tw, &th);
		cairo_move_to (cr, chart->graph.x - (tw / PANGO_SCALE) - 2, y - ((th / PANGO_SCALE)*0.8) );
		pango_cairo_show_layout (cr, layout);

		curxval -= chart->unit;
	}

	// X-scale lines + labels (items)
	if(chart->show_xval && chart->every_xval > 0 )
	{
		x = chart->graph.x + (chart->blkw/2);
		y = chart->b - chart->font_h;
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

		cairo_set_dash(cr, dashed3, 1, 0);

		for(i=first ; i<(first+chart->visible) ; i++)
		{
			if( !(i % chart->every_xval) )
			{
			ChartItem *item = &g_array_index(chart->items, ChartItem, i);
			
				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.1);
				cairo_move_to(cr, x, chart->graph.y);
				cairo_line_to(cr, x, chart->b - chart->font_h);
				cairo_stroke(cr);
				
				valstr = item->label;
				pango_layout_set_text (layout, valstr, -1);
				pango_layout_get_size (layout, &tw, &th);
				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.78);
				cairo_move_to(cr, x - ((tw / PANGO_SCALE)/2), y);
				pango_cairo_show_layout (cr, layout);
			}
			x += chart->blkw;
		}
	}

	g_object_unref (layout);
	cairo_destroy(cr);
}


/*
** draw all visible bars
*/
static void colchart_draw_bars(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;
double x, x2, y2, h;
gint i, first;

	DB( g_print("\n[column] draw bars\n") );

	x = chart->graph.x;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	cr = gdk_cairo_create (gtk_widget_get_window(widget));
	//cr = cairo_create (chart->surface);

	DB( g_print(" x=%.2f first=%d, blkw=%.2f, barw=%.2f\n", x, first, chart->blkw, chart->barw ) );
		
	#if DBGDRAW_ITEM == 1
	x2 = x + 0.5;
	cairo_set_line_width(cr, 1.0);
	double dashlength;
	dashlength = 4;
	cairo_set_dash (cr, &dashlength, 1, 0);
	cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // violet
	for(i=first; i<=(first+chart->visible) ;i++)
	{
		cairo_move_to(cr, x2, chart->graph.y);
		cairo_line_to(cr, x2, chart->graph.x + chart->graph.height);
		x2 += chart->blkw;
	}
	cairo_stroke(cr);
	cairo_set_dash (cr, &dashlength, 0, 0);
	#endif

	for(i=first; i<(first+chart->visible) ;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
	gint color;
	gint barw = chart->barw;

		//if(!chart->datas1[i]) goto nextbar;

		if(!chart->show_mono)
			color = i % chart->color_scheme.nb_cols;
		else
			color = chart->color_scheme.cs_green;

		cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[color], i == chart->active);
		
		x2 = x + (chart->blkw/2) - 1;
		x2 = !chart->dual ? x2 - (barw/2) : x2 - barw - 1;

		if(item->serie1)
		{
			h = floor((item->serie1 / chart->range) * chart->graph.height);
			y2 = chart->oy - h;
			if(item->serie1 < 0.0)
			{
				y2 += 1;
				if(chart->show_mono)
				{
					color = chart->color_scheme.cs_red;
					cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[color], i == chart->active);
				}
			}
			//DB( g_print(" + i=%d :: y2=%f h=%f (%f / %f) * %f\n", i, y2, h, chart->datas1[i], chart->range, chart->graph.height ) );

			cairo_rectangle(cr, x2+2, y2, barw, h);
			cairo_fill(cr);

		}

		if( chart->dual && item->serie2)
		{
			x2 = x2 + barw + 1;
			h = floor((item->serie2 / chart->range) * chart->graph.height);
			y2 = chart->oy - h;

			cairo_rectangle(cr, x2+2, y2, barw, h);
			cairo_fill(cr);

		}

		x += chart->blkw;

		//debug
		//gdk_draw_line (widget->window, widget->style->fg_gc[widget->state], x, chart->oy-chart->posbarh, x, chart->oy+chart->negbarh);

	}

	cairo_destroy(cr);

}

/*
** get the bar under the mouse pointer
*/
static gint colchart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval;
gint index, first, px;
		
	retval = -1;

	if( x <= chart->r && x >= chart->graph.x && y >= chart->graph.y && y <= chart->b )
	{
		px = (x - chart->graph.x);
		//py = (y - chart->oy);
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
		index = first + (px / chart->blkw);

		if(index < chart->nb_items)
			retval = index;
	}

	return(retval);
}

static void colchart_first_changed( GtkAdjustment *adj, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
//gint first;

	DB( g_print("\n[column] first changed\n") );

	//first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	//DB( g_print(" first=%d\n", first) );

/*
	DB( g_print("scrollbar\n adj=%8x, low=%.2f upp=%.2f val=%.2f step=%.2f page=%.2f size=%.2f\n", adj,
		adj->lower, adj->upper, adj->value, adj->step_increment, adj->page_increment, adj->page_size) );
 */
    /* Set the number of decimal places to which adj->value is rounded */
    //gtk_scale_set_digits (GTK_SCALE (hscale), (gint) adj->value);
    //gtk_scale_set_digits (GTK_SCALE (vscale), (gint) adj->value);

	drawarea_full_redraw (chart->drawarea, chart);
	
	DB( g_print("gtk_widget_queue_draw\n") );
	gtk_widget_queue_draw(chart->drawarea);

}

/*
** scrollbar set values for upper, page size, and also show/hide
*/
static void colchart_scrollbar_setvalues(GtkChart *chart)
{
GtkAdjustment *adj = chart->adjustment;
gint first;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	DB( g_print("\n[column] sb_set_values\n") );

	first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	DB( g_print(" entries=%d, visible=%d\n", chart->nb_items, chart->visible) );
	DB( g_print(" first=%d, upper=%d, pagesize=%d\n", first, chart->nb_items, chart->visible) );

	gtk_adjustment_set_upper(adj, (gdouble)chart->nb_items);
	gtk_adjustment_set_page_size(adj, (gdouble)chart->visible);
	gtk_adjustment_set_page_increment(adj, (gdouble)chart->visible);

	if(first+chart->visible > chart->nb_items)
	{
		gtk_adjustment_set_value(adj, (gdouble)chart->nb_items - chart->visible);
	}
	gtk_adjustment_changed (adj);

	if( chart->visible < chart->nb_items )
		gtk_widget_hide(GTK_WIDGET(chart->scrollbar));
	else
		gtk_widget_show(GTK_WIDGET(chart->scrollbar));

}

/* line section */

/*
** draw all visible lines
*/
static void linechart_draw_plot(cairo_t *cr, double x, double y, double r, GtkChart *chart)
{
	cairo_set_line_width(cr, r / 2);

	cairo_user_set_rgbcol(cr, &global_colors[THBASE]);
	cairo_arc(cr, x, y, r, 0, 2*M_PI);
	cairo_stroke_preserve(cr);

	//cairo_set_source_rgb(cr, COLTOCAIRO(0), COLTOCAIRO(119), COLTOCAIRO(204));
	cairo_user_set_rgbcol(cr, &chart->color_scheme.colors[chart->color_scheme.cs_blue]);
	cairo_fill(cr);
}


static void linechart_draw_lines(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;
double x, y, x2, y2, firstx, lastx, linew;
gint first, i;


	DB( g_print("\n[line] draw lines\n") );

	x = chart->graph.x;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	cr = gdk_cairo_create (gtk_widget_get_window(widget));
	//cr = cairo_create (chart->surface);
	
	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);


	#if DBGDRAW_ITEM == 1
	x2 = x + 0.5;
	cairo_set_line_width(cr, 1.0);
	double dashlength = 4;
	cairo_set_dash (cr, &dashlength, 1, 0);
	cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // violet
	for(i=first; i<=(first+chart->visible) ;i++)
	{
		cairo_move_to(cr, x2, chart->graph.y);
		cairo_line_to(cr, x2, chart->graph.x + chart->graph.height);
		x2 += chart->blkw;
	}
	cairo_stroke(cr);

	cairo_set_dash (cr, &dashlength, 0, 0);
	#endif

	//todo: it should be possible to draw line & plot together using surface and composite fill, or sub path ??
	lastx = x;
	firstx = x;
	linew = 4.0;
	if(chart->barw < 24)
	{
		linew = 1 + (chart->barw / 8.0);
	}

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
	cairo_set_line_width(cr, linew);

	for(i=first; i<(first+chart->visible) ;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
		
		x2 = x + (chart->blkw)/2;
		y2 = chart->oy - (item->serie1 / chart->range) * chart->graph.height;
		if( i == first)
		{
			firstx = x2;
			cairo_move_to(cr, x2, y2);
		}
		else
		{
			if( i < (chart->nb_items) )
			{
				cairo_line_to(cr, x2, y2);
				lastx = x2;
			}
			else
				lastx = x2 - chart->barw;
		}

		x += chart->blkw;
	}

	cairo_user_set_rgbcol(cr, &chart->color_scheme.colors[chart->color_scheme.cs_blue]);
	cairo_stroke_preserve(cr);

	cairo_line_to(cr, lastx, y);
	cairo_line_to(cr, firstx, y);
	cairo_close_path(cr);

	cairo_user_set_rgbacol(cr, &chart->color_scheme.colors[chart->color_scheme.cs_blue], AREA_ALPHA);
	cairo_fill(cr);

	x = chart->graph.x;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	// draw plots
	for(i=first; i<(first+chart->visible) ;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
		
		x2 = x + (chart->blkw)/2;
		y2 = chart->oy - (item->serie1 / chart->range) * chart->graph.height;

		//test draw vertical selection line		
		if( i == chart->active )
		{
			cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.1);
			
			//cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); //blue
			cairo_set_line_width(cr, 1.0);
			cairo_move_to(cr, x2, chart->graph.y);
			cairo_line_to(cr, x2, chart->b - chart->font_h);
			cairo_stroke(cr);	
		}


		linechart_draw_plot(cr,  x2, y2, i == chart->active ? linew+1 : linew, chart);

		
		x += chart->blkw;
	}

/* overdrawn */

	DB( g_print(" min=%.2f range=%.2f\n", chart->min, chart->range) );


	if( chart->show_over )
	{
		//if(chart->minimum != 0 && chart->minimum >= chart->min)
		if(chart->minimum >= chart->min)
		{
			if( chart->minimum < 0 )
			{
				y  = 0.5 + chart->oy + (ABS(chart->minimum)/chart->range) * chart->graph.height;
			}
			else
			{
				y  = 0.5 + chart->oy - (ABS(chart->minimum)/chart->range) * chart->graph.height;
			}

			y2 = (ABS(chart->min)/chart->range) * chart->graph.height - (y - chart->oy) + 1;

			cairo_set_source_rgba(cr, COLTOCAIRO(255), COLTOCAIRO(0), COLTOCAIRO(0), AREA_ALPHA / 2);

			DB( g_print(" draw over: x%d, y%f, w%d, h%f\n", chart->l, y, chart->w, y2) );

			cairo_rectangle(cr, chart->graph.x, y, chart->graph.width, y2 );
			cairo_fill(cr);
			
			cairo_set_line_width(cr, 1.0);
			cairo_set_source_rgb(cr, COLTOCAIRO(255), COLTOCAIRO(0), COLTOCAIRO(0));

			cairo_set_dash (cr, dashed3, 1, 0);
			cairo_move_to(cr, chart->graph.x, y);
			cairo_line_to (cr, chart->graph.x + chart->graph.width, y);
			cairo_stroke(cr);
		}
	}

	cairo_destroy(cr);


}




/*
** get the point under the mouse pointer
*/
static gint linechart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval;
gint first, index, px;

	DB( g_print("\n[line] get active\n") );

	retval = -1;

	if( x <= chart->r && x >= chart->graph.x && y >= chart->graph.y && y <= chart->b )
	{
		px = (x - chart->graph.x);
		//py = (y - chart->oy);
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
		index = first + (px / (chart->blkw));

		if(index < chart->nb_items)
			retval = index;
	}

	return(retval);
}


/* pie section */

static void piechart_calculation(GtkChart *chart)
{
//GtkWidget *drawarea = chart->drawarea;
gint w, h;

	DB( g_print("\n[pie] calculation\n") );

	w = chart->graph.width;
	h = chart->graph.height;

	chart->rayon = MIN(w, h);
	chart->mark = 0;

	#if CHART_PARAM_PIE_MARK == TRUE
	gint m = floor(chart->rayon / 100);
	m = MAX(2, m);
	chart->rayon -= (m * 2);
	chart->mark = m;
	#endif

	chart->ox = chart->graph.x + (w / 2);
	chart->oy = chart->graph.y + (chart->rayon / 2);
	
	DB( g_print(" center: %g, %g - R=%d, mark=%d\n", chart->ox, chart->oy, chart->rayon, chart->mark) );

}


static void piechart_draw_slices(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;

	if(chart->nb_items <= 0)
		return;

	DB( g_print("\n[pie] draw slices\n") );


		//cairo drawing

		double a1 = 0 * (M_PI / 180);
		double a2 = 360 * (M_PI / 180);

		//g_print("angle1=%.2f angle2=%.2f\n", a1, a2);

		double cx = chart->ox;
		double cy = chart->oy;
		double radius = chart->rayon/2;
		gint i;
		double dx, dy;
		double sum = 0.0;
		gint color;

		cr = gdk_cairo_create (gtk_widget_get_window(widget));
		//cr = cairo_create (chart->surface);

		DB( g_print("rayon=%d\n", chart->rayon) );
		
		for(i=0; i< chart->nb_items ;i++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, i);

			a1 = ((360 * (sum / chart->total)) - 90) * (M_PI / 180);
			sum += ABS(item->serie1);
			a2 = ((360 * (sum / chart->total)) - 90) * (M_PI / 180);
			//if(i < chart->nb_items-1) a2 += 0.0175;
			
			dx = cx;
			dy = cy;

			cairo_move_to(cr, dx, dy);
			cairo_arc(cr, dx, dy, radius, a1, a2);

			#if CHART_PARAM_PIE_LINE == TRUE
				cairo_set_line_width(cr, 2.0);
				cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
				cairo_line_to(cr, cx, cy);
				cairo_stroke_preserve(cr);
			#endif

			DB( g_print("- s%2d: %.2f%% a1=%.2f a2=%.2f\n", i, sum / chart->total, a1, a2) );

			//g_print("color : %f %f %f\n", COLTOCAIRO(colors[i].r), COLTOCAIRO(colors[i].g), COLTOCAIRO(colors[i].b));

			color = i % chart->color_scheme.nb_cols;
			cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[color], i == chart->active);
			cairo_fill(cr);
		}



#if CHART_PARAM_PIE_DONUT == TRUE
	a1 = 0;
	a2 = 2 * M_PI;

	//original
	//radius = (gint)((chart->rayon/3) * (1 / PHI));
	//5.1
	//radius = (gint)((chart->rayon/2) * 2 / 3);
	//ynab
	//piehole value from 0.4 to 0.6 will look best on most charts
	radius = (gint)(chart->rayon/2) * CHART_PARAM_PIE_HOLEVALUE;

	cairo_arc(cr, cx, cy, radius, a1, a2);
	cairo_user_set_rgbcol(cr, &global_colors[THBASE]);
	cairo_fill(cr);
#endif

	cairo_destroy(cr);

}


static gint piechart_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gint retval, px, py;
gint index;
double radius, h;

	DB( g_print("\n[pie] get active\n") );

	retval = -1;

	px = x - chart->ox;
	py = y - chart->oy;
	h  = sqrt( pow(px,2) + pow(py,2) );
	radius = chart->rayon / 2;

	if(h <= radius && h >= (radius * CHART_PARAM_PIE_HOLEVALUE) )
	{
	double angle, b;

		b     = (acos(px / h) * 180) / M_PI;
		angle = py > 0 ? b : 360 - b;
		angle += 90;
		if(angle > 360) angle -= 360;
		//angle = 360 - angle;

		//todo optimize
		gdouble cumul = 0;
		for(index=0; index< chart->nb_items ;index++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, index);

			cumul += ABS(item->serie1/chart->total)*360;
			if( cumul > angle )
			{
				retval = index;
				break;
			}
		}

		//DB( g_print(" inside: x=%d, y=%d\n", x, y) );
		//DB( g_print(" inside: b=%f angle=%f, slice is %d\n", b, angle, index) );
	}
	return(retval);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static gboolean drawarea_full_redraw(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;
PangoLayout *layout;
int tw, th;

	DB( g_print("------\n[gtkchart] drawarea full redraw\n") );

	cr = cairo_create (chart->surface);

#if MYDEBUG == 1
cairo_font_face_t *ff;
cairo_scaled_font_t *sf;

	ff = cairo_get_font_face(cr);
	sf = cairo_get_scaled_font(cr);
	
	g_print("cairo ff = '%s'\n", cairo_toy_font_face_get_family(ff) );

	ff = cairo_scaled_font_get_font_face(sf);
	g_print("cairo sf = '%s'\n", cairo_toy_font_face_get_family(ff) );

	//cairo_set_font_face(cr, ff);
#endif

	/* fillin the back in white */
	//cairo_user_set_rgbcol(cr, &global_colors[WHITE]);
	cairo_user_set_rgbcol(cr, &global_colors[THBASE]);

	cairo_paint(cr);

	if(chart->nb_items == 0)
	{
		cairo_destroy(cr);
		return FALSE;
	}

	/*debug help draws */
	#if DBGDRAW_RECT == 1
		//clip area
		cairo_set_line_width(cr, 1.0);
		cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); //green
		cairo_rectangle(cr, chart->l+0.5, chart->t+0.5, chart->w, chart->h);
		cairo_stroke(cr);

		//graph area
		cairo_set_source_rgb(cr, 1.0, 0.5, 0.0); //orange
		cairo_rectangle(cr, chart->graph.x+0.5, chart->graph.y+0.5, chart->graph.width, chart->graph.height);
		cairo_stroke(cr);
	#endif

	// draw title
	if(chart->title)
	{
		layout = pango_cairo_create_layout (cr);
	
		chart_set_font_size(chart, layout, CHART_FONT_SIZE_TITLE);
		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);

		cairo_user_set_rgbcol(cr, &global_colors[THTEXT]);
		cairo_move_to(cr, chart->l, chart->t);
		pango_cairo_show_layout (cr, layout);

		#if DBGDRAW_TEXT == 1
			double dashlength;
			cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
			dashlength = 3;
			cairo_set_dash (cr, &dashlength, 1, 0);
			//cairo_move_to(cr, chart->l, chart->t);
			cairo_rectangle(cr, chart->l+0.5, chart->t+0.5, (tw / PANGO_SCALE), (th / PANGO_SCALE));
			cairo_stroke(cr);
		#endif

		g_object_unref (layout);
	}

	switch(chart->type)
	{
		case CHART_TYPE_COL:
			colchart_draw_scale(widget, chart);
			//colchart_draw_bars(widget, chart);
			break;
		case CHART_TYPE_LINE:
			colchart_draw_scale(widget, chart);
			//linechart_draw_lines(widget, chart);
			break;
		case CHART_TYPE_PIE:
			//piechart_draw_slices(widget, chart);
			break;
	}


	//test legend
	if(chart->show_legend)
	{
	gint i;
	gchar *valstr;
	gint x, y;
	gint radius;
	gint color;

		DB( g_print("\n[chart] draw legend\n") );

		layout = pango_cairo_create_layout (cr);
		chart_set_font_size(chart, layout, CHART_FONT_SIZE_SMALL);
		pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

		x = chart->legend.x;
		y = chart->legend.y;
		radius = chart->legend_font_h;

		#if DBGDRAW_RECT == 1
			double dashlength;
			cairo_set_source_rgb(cr, 1.0, 0.5, 0.0);	//orange
			dashlength = 3;
			cairo_set_dash (cr, &dashlength, 1, 0);
			//cairo_move_to(cr, x, y);
			cairo_rectangle(cr, chart->legend.x+0.5, chart->legend.y+0.5, chart->legend.width, chart->legend.height);
			cairo_stroke(cr);
		#endif

		for(i=0; i< chart->nb_items ;i++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, i);

			if(item)
			{
				//DB( g_print(" draw %2d '%s' y=%g\n", i, item->label, y) );

				#if DBGDRAW_TEXT == 1
					double dashlength;
					cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
					dashlength = 3;
					cairo_set_dash (cr, &dashlength, 1, 0);
					//cairo_move_to(cr, x, y);
					cairo_rectangle(cr, x+0.5, y+0.5, chart->legend_font_h + CHART_SPACING + chart->legend_label_w, chart->legend_font_h);
					cairo_stroke(cr);
				#endif

				// check if enought height to draw
				if( chart->nb_items - i > 1 )
				{
					if( (y + floor(2 * radius * CHART_LINE_SPACING)) > chart->b )
					{
						DB( g_print(" print ...\n\n") );
						pango_layout_set_text (layout, "...", -1);
						cairo_move_to(cr, x + radius + CHART_SPACING, y);
						pango_cairo_show_layout (cr, layout);
						break;
					}
				}

				// 1: palette
				cairo_arc(cr, x + (radius/2), y + (radius/2), (radius/2), 0, 2 * M_PI);
				color = i % chart->color_scheme.nb_cols;
				cairo_user_set_rgbcol(cr, &chart->color_scheme.colors[color]);
				cairo_fill(cr);

				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.78);

				// 2: label
				valstr = item->label;
				pango_layout_set_text (layout, valstr, -1);
				pango_layout_set_width(layout, chart->legend_label_w * PANGO_SCALE);
				cairo_move_to(cr, x + chart->legend_font_h + CHART_SPACING, y);
				pango_cairo_show_layout (cr, layout);

				if( chart->show_legend_wide )
				{
					pango_layout_set_width(layout, -1);

					// 3: value
					valstr = chart_print_double(chart, chart->buffer1, item->serie1);
					pango_layout_set_text (layout, valstr, -1);
					pango_layout_get_size (layout, &tw, &th);
					cairo_move_to(cr, x + chart->legend_font_h + chart->legend_label_w + (CHART_SPACING*3) + chart->legend_value_w - (tw/PANGO_SCALE), y);
					pango_cairo_show_layout (cr, layout);

					// 4: rate
					valstr = chart_print_rate(chart, item->rate);
					pango_layout_set_text (layout, valstr, -1);
					pango_layout_get_size (layout, &tw, &th);
					cairo_move_to(cr, x + chart->legend_font_h + chart->legend_label_w + chart->legend_value_w + chart->legend_rate_w + (CHART_SPACING*3) - (tw/PANGO_SCALE), y);
					pango_cairo_show_layout (cr, layout);
				}

				//the radius contains the font height here
				//y += floor(chart->font_h * CHART_LINE_SPACING);
				y += floor(radius * CHART_LINE_SPACING);
			}
		}
		
		g_object_unref (layout);
		
	}

	cairo_destroy(cr);

	return TRUE;
}


static gboolean
drawarea_configure_event_callback (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           user_data)
{
GtkChart *chart = GTK_CHART(user_data);
GtkAllocation allocation;
GtkStyleContext *context;
PangoFontDescription *desc;
gboolean colfound;
GdkRGBA color;

	DB( g_print("\n[gtkchart] drawarea configure \n") );

	gtk_widget_get_allocation (widget, &allocation);

	DB( g_print("w=%d h=%d\n", allocation.width, allocation.height) );

	
	if (chart->surface)
		cairo_surface_destroy (chart->surface);

	chart->surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               allocation.width,
                                               allocation.height);

	context = gtk_widget_get_style_context (widget);

	chart_color_global_default();

	// get base color
	colfound = gtk_style_context_lookup_color(context, "theme_base_color", &color);
	if(!colfound)
		colfound = gtk_style_context_lookup_color(context, "base_color", &color);

	if( colfound )
	{
		struct rgbcol *tcol = &global_colors[THBASE];
		tcol->r = color.red * 255;
		tcol->g = color.green * 255;
		tcol->b = color.blue * 255;
		DB( g_print(" - theme base col: %x %x %x\n", tcol->r, tcol->g, tcol->b) );
	}

	//get text color
	colfound = gtk_style_context_lookup_color(context, "theme_text_color", &color);
	if(!colfound)
		gtk_style_context_lookup_color(context, "text_color", &color);


	if( colfound )
	{
		struct rgbcol *tcol = &global_colors[THTEXT];
		tcol->r = color.red * 255;
		tcol->g = color.green * 255;
		tcol->b = color.blue * 255;
		DB( g_print(" - theme text (bg) col: %x %x %x\n", tcol->r, tcol->g, tcol->b) );
	}

	//commented 5.1.5
	//drawarea_full_redraw(widget, user_data);

	/* get and copy the font */
	gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL, "font", &desc, NULL);
	if(chart->pfd)
	{
		pango_font_description_free (chart->pfd);
		chart->pfd = NULL;
	}
	chart->pfd = pango_font_description_copy(desc);
	chart->pfd_size = pango_font_description_get_size (desc) / PANGO_SCALE;
	//chart->barw = (6 + chart->pfd_size) * PHI;

	//leak: we should free desc here ?
	//or no need to copy above ?
	//pango_font_description_free(desc);

	DB( g_print("family: %s\n", pango_font_description_get_family(chart->pfd) ) );
	DB( g_print("size  : %d (%d)\n", chart->pfd_size, chart->pfd_size/PANGO_SCALE ) );
	DB( g_print("isabs : %d\n", pango_font_description_get_size_is_absolute (chart->pfd) ) );

	
	if( gtk_widget_get_realized(widget))
	{
		chart_recompute(chart);   
		drawarea_full_redraw(widget, chart);
	}
	
	/* We've handled the configure event, no need for further processing. */
	return TRUE;
}


static void drawarea_realize_callback(GtkWidget *widget, gpointer   user_data)
{
//GtkChart *chart = GTK_CHART(user_data);
	
	DB( g_print("\n[gtkchart] drawarea realize\n") );

	//chart_recompute(chart);

}


static gboolean drawarea_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);

	if( !gtk_widget_get_realized(widget) || chart->surface == NULL )
		return FALSE;

	DB( g_print("\n[gtkchart] drawarea draw callback\n") );

	cairo_set_source_surface (cr, chart->surface, 0, 0);
	cairo_paint (cr);

	switch(chart->type)
	{
		case CHART_TYPE_COL:
			colchart_draw_bars(widget, chart);
			break;
		case CHART_TYPE_LINE:
			linechart_draw_lines(widget, chart);
			break;
		case CHART_TYPE_PIE:
			piechart_draw_slices(widget, chart);
			break;
	}
	
	return FALSE;
}


static gboolean drawarea_querytooltip_callback(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gchar *strval, *strval2, *buffer;
gboolean retval = FALSE;
	
	if(chart->surface == NULL)
		return FALSE;
	
	DB( g_print("\n[gtkchart] drawarea querytooltip\n") );

	DB( g_print(" x=%d, y=%d kbm=%d\n", x, y, keyboard_mode) );
	if(chart->lastactive != chart->active)
	{
		goto end;
	}
	
	if(chart->active >= 0)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, chart->active);

		strval = chart_print_double(chart, chart->buffer1, item->serie1);
		if( !chart->dual )
		{
			//#1420495 don't use g_markup_printf_escaped
			if( chart->type == CHART_TYPE_PIE )
				buffer = g_strdup_printf("%s\n%s\n%.2f%%", item->label, strval, item->rate);
			else
				buffer = g_strdup_printf("%s\n%s", item->label, strval);

		}
		else
		{
			strval2 = chart_print_double(chart, chart->buffer2, item->serie2);
			buffer = g_strdup_printf("%s\n+%s\n%s", item->label, strval2, strval);
		}
		
		gtk_tooltip_set_text(tooltip, buffer);
		//gtk_label_set_markup(GTK_LABEL(chart->ttlabel), buffer);
		g_free(buffer);
		retval = TRUE;
	}
end:
	chart->lastactive = chart->active;
	
	return retval;
}


static gboolean drawarea_motionnotifyevent_callback(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gboolean retval = TRUE;
gint x, y;

	if(chart->surface == NULL || chart->nb_items == 0)
		return FALSE;

	DB( g_print("\n[gtkchart] drawarea motion\n") );
	x = event->x;
	y = event->y;

	//todo see this
	if(event->is_hint)
	{
		DB( g_print(" is hint\n") );

		gdk_window_get_device_position(event->window, event->device, &x, &y, NULL);
		//gdk_window_get_pointer(event->window, &x, &y, NULL);
		//return FALSE;
	}

	switch(chart->type)
	{
		case CHART_TYPE_COL:
			chart->active = colchart_get_active(widget, x, y, chart);
			break;
		case CHART_TYPE_LINE:
			chart->active = linechart_get_active(widget, x, y, chart);
			break;
		case CHART_TYPE_PIE:
			chart->active = piechart_get_active(widget, x, y, chart);
			break;
	}

	//test: eval legend
	if( chart->show_legend && chart->active == - 1)
	{
		if( x >= chart->legend.x && (x <= (chart->legend.x+chart->legend.width )) 
		&&  y >= chart->legend.y && (y <= (chart->legend.y+chart->legend.height ))
		)
		{
			//use the radius a font height here
			chart->active = (y - chart->legend.y) / floor(chart->legend_font_h * CHART_LINE_SPACING);
		}

		if( chart->active > chart->nb_items - 1)
		{
			chart->active = -1;
		}
	}

	// rollover redraw ?
	DB( g_print(" active: last=%d, curr=%d\n", chart->lastactive, chart->active) );

	if(chart->lastactive != chart->active)
	{
	GdkRectangle update_rect;
	gint first;

		DB( g_print(" motion rollover redraw :: active=%d\n", chart->active) );

		first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
	
		/* pie : always invalidate all graph area */
		if( chart->type == CHART_TYPE_PIE )
		{
			update_rect.x = chart->graph.x;
			update_rect.y = chart->graph.y;
			update_rect.width = chart->graph.width;
			update_rect.height = chart->graph.height;

			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
		                          &update_rect,
		                          FALSE);
		}
	
		if(chart->lastactive != -1)
		{
			/* column/line : invalidate rollover */
			if( chart->type == CHART_TYPE_COL || chart->type == CHART_TYPE_LINE )
			{
				update_rect.x = chart->graph.x + (chart->lastactive - first) * chart->blkw;
				update_rect.y = chart->graph.y - 6;
				update_rect.width = chart->blkw;
				update_rect.height = chart->graph.height + 12;
			}
		
			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
		                          &update_rect,
		                          FALSE);
		}

		/* column/line : invalidate current item */
		if( chart->type == CHART_TYPE_COL || chart->type == CHART_TYPE_LINE )
		{
			update_rect.x = chart->graph.x + (chart->active - first) * chart->blkw;
			update_rect.y = chart->graph.y - 6;
			update_rect.width = chart->blkw;
			update_rect.height = chart->graph.height + 12;
	
			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
			                      &update_rect,
			                      FALSE);
		}
		
		//gtk_widget_queue_draw( widget );
		//retval = FALSE;
	}

	DB( g_print(" x=%d, y=%d, time=%d\n", x, y, event->time) );

	//if(inlegend != TRUE)
	//	gtk_tooltip_trigger_tooltip_query(gtk_widget_get_display(chart->drawarea));

	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* public functions */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void gtk_chart_queue_redraw(GtkChart *chart)
{
	DB( g_print("\n[gtkchart] queue redraw\n") );

	if( gtk_widget_get_realized(chart->drawarea) )
	{
		chart_recompute(chart);
		drawarea_full_redraw(chart->drawarea, chart);
		DB( g_print("gtk_widget_queue_draw\n") );
		gtk_widget_queue_draw( chart->drawarea );
	}
}


/*
** change the model and/or column
*/
void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column, gchar *title, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set datas\n") );

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column, column);
		if(title != NULL)
			chart->title = g_strdup(title);
		if(subtitle != NULL)
			chart->subtitle = g_strdup(subtitle);

		gtk_chart_queue_redraw(chart);
	}
	else
	{
		chart_clear(chart);
		if( GTK_IS_LIST_STORE(chart->model) )
			gtk_list_store_clear (GTK_LIST_STORE(chart->model));

	}
}

/*
** change the model and/or column
*/
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2, gchar *title, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set dualdatas\n") );

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column1, column2 );
		if(title != NULL)
			chart->title = g_strdup(title);
		if(subtitle != NULL)
			chart->subtitle = g_strdup(subtitle);
		
		gtk_chart_queue_redraw(chart);
	}
	else
	{
		chart_clear(chart);
		if( GTK_IS_LIST_STORE(chart->model) )
			gtk_list_store_clear (GTK_LIST_STORE(chart->model));
	}
}


/*
** change the type dynamically
*/
void gtk_chart_set_type(GtkChart * chart, gint type)
{
	g_return_if_fail (GTK_IS_CHART (chart));
	//g_return_if_fail (type < CHART_TYPE_MAX);

	DB( g_print("\n[gtkchart] set type %d\n", type) );

	chart->type = type;
	chart->dual = FALSE;

	gtk_chart_queue_redraw(chart);
}

/* = = = = = = = = = = parameters = = = = = = = = = = */

void gtk_chart_set_color_scheme(GtkChart * chart, gint index)
{
	colorscheme_init(&chart->color_scheme, index);
}



/*
** set the minor parameters
*/
void gtk_chart_set_minor_prefs(GtkChart * chart, gdouble rate, gchar *symbol)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set minor prefs\n") );

	chart->minor_rate   = rate;
	chart->minor_symbol = symbol;
}


void gtk_chart_set_absolute(GtkChart * chart, gboolean abs)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set absolute\n") );

	chart->abs = abs;
}


void gtk_chart_set_currency(GtkChart * chart, guint32 kcur)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->kcur = kcur;
}


/*
** set the overdrawn minimum
*/
void gtk_chart_set_overdrawn(GtkChart * chart, gdouble minimum)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set overdrawn\n") );

	chart->minimum = minimum;

	//if(chart->type == CHART_TYPE_LINE)
	//	chart_recompute(chart);
}


/*
** obsolete: set the every_xval
*/
/*void gtk_chart_set_every_xval(GtkChart * chart, gint gap)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set every_xval\n") );

	chart->every_xval = gap;

	//if(chart->type != CHART_TYPE_PIE)
	//	chart_recompute(chart);
}*/


/*
** set the barw
*/
void gtk_chart_set_barw(GtkChart * chart, gdouble barw)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set barw\n") );

	if( barw >= GTK_CHART_MINBARW && barw <= GTK_CHART_MAXBARW )
		chart->usrbarw = barw;
	else
		chart->usrbarw = 0;


	if(chart->type != CHART_TYPE_PIE)
		gtk_chart_queue_redraw(chart);
}

/*
** set the show mono (colors)
*/
void gtk_chart_set_showmono(GtkChart * chart, gboolean mono)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->show_mono = mono;

	if(chart->type != CHART_TYPE_PIE)
		gtk_chart_queue_redraw(chart);
}


/* = = = = = = = = = = visibility = = = = = = = = = = */

/*
** change the legend visibility
*/
void gtk_chart_show_legend(GtkChart * chart, gboolean visible, gboolean showextracol)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->show_legend = visible;
	chart->show_legend_wide = showextracol;
	gtk_chart_queue_redraw(chart);
}

/*
** change the x-value visibility
*/
void gtk_chart_show_xval(GtkChart * chart, gboolean visible)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set show xval\n") );

	chart->show_xval = visible;

	//if(chart->type != CHART_TYPE_PIE)
	//	chart_recompute(chart);
}

/*
** chnage the overdrawn visibility
*/
void gtk_chart_show_overdrawn(GtkChart * chart, gboolean visible)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set show overdrawn\n") );

	chart->show_over = visible;

	//if(chart->type == CHART_TYPE_LINE)
	//	chart_recompute(chart);
}


/*
** change the minor visibility
*/
void gtk_chart_show_minor(GtkChart * chart, gboolean minor)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	DB( g_print("\n[gtkchart] set show minor\n") );

	chart->minor = minor;

	if(chart->type != CHART_TYPE_PIE)
		gtk_chart_queue_redraw(chart);

}

