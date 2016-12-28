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


#define HELPDRAW 0
#define DYNAMICS 1


static void gtk_chart_class_intern_init (gpointer);
static void gtk_chart_class_init (GtkChartClass *klass);
static void gtk_chart_init (GtkChart *chart);
static void gtk_chart_dispose (GObject * object);
static void gtk_chart_finalize (GObject * object);

static gboolean drawarea_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
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

static GdkPixbuf *create_color_pixbuf (struct rgbcol *color);
static GtkWidget *legend_list_new(GtkChart *chart);


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
gtk_chart_class_init (GtkChartClass * klass)
{
GObjectClass *gobject_class;

	DB( g_print("\n[gtkchart] class init\n") );

	gobject_class = G_OBJECT_CLASS (klass);

	//gobject_class->get_property = gtk_chart_get_property;
	//gobject_class->set_property = gtk_chart_set_property;
	gobject_class->dispose = gtk_chart_dispose;
	gobject_class->finalize = gtk_chart_finalize;
}


static void
gtk_chart_init (GtkChart * chart)
{
GtkWidget *widget, *vbox;
GtkWidget *scrollwin, *treeview;

	DB( g_print("\n[gtkchart] init\n") );


	chart->surface = NULL;
 	chart->nb_items = 0;
	chart->items = NULL;
 	chart->title = NULL;
 	
  	chart->pfd = NULL;
	chart->abs = FALSE;
	chart->dual = FALSE;
	chart->barw = GTK_CHART_BARW;
	chart->show_mono = FALSE;
	//chart->drawmode = CHART_DRAW_FULL;
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
	scrollwin = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(scrollwin), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	//scrollwin = gtk_scrolled_window_new(NULL,NULL);
    //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    //gtk_box_pack_start (GTK_BOX (vbox), scrollwin, TRUE, TRUE, 0);

	chart->drawarea = gtk_drawing_area_new();
	//gtk_widget_set_double_buffered (GTK_WIDGET(widget), FALSE);
	
	gtk_container_add( GTK_CONTAINER(scrollwin), chart->drawarea );
	gtk_widget_set_size_request(chart->drawarea, 100, 100 );
	gtk_widget_set_has_tooltip(chart->drawarea, TRUE);
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

	/* legend treeview */
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	chart->scrollwin = scrollwin;
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	treeview = legend_list_new(chart);
	chart->treeview = treeview;
	chart->legend = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_box_pack_start (GTK_BOX (widget), scrollwin, FALSE, FALSE, 0);

	gtk_widget_add_events(GTK_WIDGET(chart->drawarea),
		GDK_EXPOSURE_MASK |
		//GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK
		//GDK_BUTTON_PRESS_MASK |
		//GDK_BUTTON_RELEASE_MASK
		);

	g_signal_connect( G_OBJECT(chart->drawarea), "configure-event", G_CALLBACK (drawarea_configure_event), chart);
	g_signal_connect( G_OBJECT(chart->drawarea), "realize", G_CALLBACK(drawarea_realize_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "draw", G_CALLBACK(drawarea_draw_callback), chart ) ;
#if DYNAMICS == 1
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
GtkTreeIter iter, l_iter;
gint color;

	DB( g_print("\n[chart] setup with model\n") );

	chart_clear(chart);
	
	if( GTK_IS_LIST_STORE(chart->legend) )
		gtk_list_store_clear (GTK_LIST_STORE(chart->legend));
	
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
	GdkPixbuf *pixbuf;
		
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

		/* populate our legend list */
		color = i % chart->color_scheme.nb_cols;
		//color = id % chart->nb_cols;

		//DB( g_print ("Row %d: (%s, %2.f) color %d\n", id, title, value, color) );

		pixbuf = create_color_pixbuf (&chart->color_scheme.colors[color]);

        gtk_list_store_append (GTK_LIST_STORE(chart->legend), &l_iter);
        gtk_list_store_set (GTK_LIST_STORE(chart->legend), &l_iter,
                            LST_LEGEND_COLOR, pixbuf,
                            LST_LEGEND_TITLE, label,
                            LST_LEGEND_AMOUNT, value1,
                            -1);

		/* pie chart total sum */
		chart->total += ABS(value1);
		
		valid = gtk_tree_model_iter_next (list_store, &iter);
		i++;
	}

	// compute rate for legend for bar/pie
	for(i=0;i<chart->nb_items;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);

		item->rate = ABS(item->serie1*100/chart->total);
		item->legend = g_markup_printf_escaped("%s (%.2f%%)", item->label, item->rate);
	}

	if( chart->type != CHART_TYPE_LINE )
	{

		valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(chart->legend), &iter);
		while (valid)
		{
		gdouble amount, rate;
		
			gtk_tree_model_get(GTK_TREE_MODEL(chart->legend), &iter,
				LST_LEGEND_AMOUNT, &amount,
			-1);

			rate = ABS( amount*100/chart->total);

			gtk_list_store_set(GTK_LIST_STORE(chart->legend), &iter,
				LST_LEGEND_RATE, rate,
			-1);
	
			valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(chart->legend), &iter);
		}

	}


	//g_print("total is %.2f\n", total);
	//ensure the widget is mapped
	//gtk_widget_map(chart);

}


static void chart_set_font_size(GtkChart *chart, gint font_size)
{
gint size = 10;

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
	}

	//DB( g_print(" size=%d\n", size) );

	pango_font_description_set_size(chart->pfd, size * PANGO_SCALE);

}


/*
** recompute according to type
*/
static void chart_recompute(GtkChart *chart)
{

	DB( g_print("\n[gtkchart] recompute\n") );

	chart_calculation (chart);
	
	switch(chart->type)
	{
		case CHART_TYPE_LINE:
		case CHART_TYPE_COL:
			colchart_compute_range(chart);
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
	gint maxticks = MIN(10,floor(chart->graph_height / (chart->font_h * 2)));

	DB( g_print(" raw :: [%.2f - %.2f] range=%.2f\n", chart->rawmin, chart->rawmax, chart->range) );
	DB( g_print(" raw :: maxticks=%d (%g / (%g*2))\n", maxticks, chart->graph_height, chart->font_h) );

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
		chart_set_font_size(chart, CHART_FONT_SIZE_TITLE);
		pango_layout_set_font_description (layout, chart->pfd);

		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->title_zh = (th / PANGO_SCALE);
		DB( g_print(" - title: %s w=%d h=%d\n", chart->title, tw, th) );
	}

	// compute subtitle
	chart->subtitle_zh = 0;
	if(chart->subtitle != NULL)
	{
		chart_set_font_size(chart, CHART_FONT_SIZE_SUBTITLE);
		pango_layout_set_font_description (layout, chart->pfd);

		pango_layout_set_text (layout, chart->subtitle, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->subtitle_zh = (th / PANGO_SCALE);
		DB( g_print(" - title: %s w=%d h=%d\n", chart->subtitle, tw, th) );
	}

	chart->subtitle_y = chart->t + chart->title_zh;

	// todo: compute maxwidth of item labels
	double label_w = 0;
	for(i=0;i<chart->nb_items;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
	
		// category width
		pango_layout_set_text (layout, item->label, -1);
		pango_layout_get_size (layout, &tw, &th);
		label_w = MAX(label_w, (tw / PANGO_SCALE));
	}

	chart->label_w = label_w + CHART_SPACING;

	DB( g_print(" - label_w:%g\n", chart->label_w) );


	// compute other text
	chart_set_font_size(chart, CHART_FONT_SIZE_NORMAL);
	pango_layout_set_font_description (layout, chart->pfd);

	// compute amount scale
	valstr = chart_print_int(chart, (gint)chart->min);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->scale_w = (tw / PANGO_SCALE);

	valstr = chart_print_int(chart, (gint)chart->max);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->scale_w = MAX(chart->scale_w, (tw / PANGO_SCALE));
	DB( g_print(" - scale: %g,%g %g,%g\n", chart->l, 0.0, chart->scale_w, 0.0) );

	// compute font height
	chart->font_h = (th / PANGO_SCALE);

	// compute graph region
	switch(chart->type)
	{
		case CHART_TYPE_LINE:
		case CHART_TYPE_COL:
			chart->graph_x = chart->l + chart->scale_w + 2;
			chart->graph_y = chart->t + chart->title_zh + chart->subtitle_zh;
			chart->graph_width  = chart->w - chart->scale_w - 2;
			chart->graph_height = chart->h - chart->title_zh - chart->subtitle_zh;
			break;
		case CHART_TYPE_PIE:
			chart->graph_x = chart->l;
			chart->graph_y = chart->t + chart->title_zh + chart->subtitle_zh;
			chart->graph_width  = chart->w;
			chart->graph_height = chart->h - chart->title_zh - chart->subtitle_zh;
			break;	
	}

	if(chart->title_zh > 0 || chart->subtitle_zh > 0)
	{
		chart->graph_y += CHART_MARGIN;
		chart->graph_height -= CHART_MARGIN;
	}

	if(chart->type != CHART_TYPE_PIE && chart->show_xval)
		chart->graph_height -= (chart->font_h + CHART_SPACING);

	g_object_unref (layout);

	cairo_destroy(cr);
	cairo_surface_destroy(surf);

}


static void colchart_calculation(GtkChart *chart)
{
gint blkw;

	DB( g_print("\n[column]  calculation\n") );


	//if expand : we compute available space
	//blkw = floor(MAX(8, (chart->graph_width)/chart->nb_items));

	// if fixed
	blkw = chart->barw + 3;
	if( chart->dual )
		blkw = (chart->barw * 2) + 3;

	chart->blkw = blkw;
	chart->visible = chart->graph_width / blkw;
	chart->visible = MIN(chart->visible, chart->nb_items);

	chart->ox = chart->l;
	chart->oy = chart->b;
	if(chart->range > 0)
		chart->oy = floor(chart->graph_y + (chart->max/chart->range) * chart->graph_height);

	DB( g_print(" + ox=%f oy=%f\n", chart->ox, chart->oy) );

	/* todo: hack on every xval */
	if(chart->label_w > 0)
	{
		blkw = floor(MIN(chart->nb_items*chart->blkw, chart->graph_width) / chart->label_w);
		if(blkw > 0 )
			blkw = chart->visible / blkw;
	
		chart->every_xval = MAX(1,blkw);
	}
	//chart->every_xval = chart->visible - floor(chart->graph_width / chart->label_w);

	DB( g_print("vis=%d, width=%g, lbl_w=%g :: %d\n", chart->visible, chart->graph_width, chart->label_w, blkw) );

}


/*
** draw the scale
*/
static void colchart_draw_scale(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
double x, y;
gdouble curxval;
gint i, first;

	DB( g_print("\n[column] draw scale\n") );

cairo_t *cr;


	//gdkwindow = gtk_widget_get_window(widget);
	//cr = gdk_cairo_create (gdkwindow);
	//cr = gdk_cairo_create (widget->window);
	cr = cairo_create (chart->surface);
	
	cairo_set_line_width(cr, 1);

	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);

	/* draw vertical lines + legend */
	if(chart->show_xval)
	{
		x = chart->graph_x + 1.5 + (chart->barw/2);
		y = chart->oy;
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

		cairo_set_dash(cr, dashed3, 1, 0);
		for(i=first; i<(first+chart->visible) ;i++)
		{
			if( !(i % chart->every_xval) )
			{
				//cairo_user_set_rgbcol(cr, &global_colors[GREY1]);
				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.1);
				
				//cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); //blue

				cairo_move_to(cr, x, chart->graph_y);
				cairo_line_to(cr, x, chart->b - chart->font_h);
				cairo_stroke(cr);
			}

			x += chart->blkw;
		}
	}

	/* horizontal lines */

	curxval = chart->max;
	cairo_set_dash(cr, 0, 0, 0);
	for(i=0;i<=chart->div;i++)
	{

		//if(i == 0 || i == chart->div) 	/* top/bottom line */
		//{
			//cairo_set_dash(cr, 0, 0, 0);
			//cairo_user_set_rgbcol(cr, &global_colors[GREY1]);
			cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.1);
		//}
		//else /* intermediate line (dotted) */
		//{
			//cairo_set_dash(cr, dashed3, 1, 0);
			//cairo_user_set_rgbcol(cr, &global_colors[GREY1]);
		//}

		/* x axis ? */
		if( curxval == 0.0 )
		{
			//cairo_set_dash(cr, 0, 0, 0);
			cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.8);
		}

		y = 0.5 + floor(chart->graph_y + ((i * chart->unit) / chart->range) * chart->graph_height);

		DB( g_print(" + i=%d :: y=%f (%f / %f) * %f\n", i, y, i*chart->unit, chart->range, chart->graph_height) );

		cairo_move_to(cr, chart->graph_x, y);
		cairo_line_to(cr, chart->graph_x + chart->graph_width, y);
		cairo_stroke(cr);

		curxval -= chart->unit;
	}

	cairo_destroy(cr);

}


static void colchart_draw_scale_text(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
double x, y;
gdouble curxval;
gchar *valstr;
gint i, first;
cairo_t *cr;
PangoLayout *layout;
int tw, th;


	DB( g_print("\n([column] draw scale text\n") );

	//GdkWindow *gdkwindow;
	//gdkwindow = gtk_widget_get_window(widget);

	//cr = gdk_cairo_create (gdkwindow);
	//cr = gdk_cairo_create (widget->window);
	cr = cairo_create (chart->surface);

	layout = pango_cairo_create_layout (cr);

	cairo_set_line_width(cr, 1);

	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);

	//cairo_set_operator(cr, CAIRO_OPERATOR_SATURATE);

	chart_set_font_size(chart, CHART_FONT_SIZE_NORMAL);
	pango_layout_set_font_description (layout, chart->pfd);



	/* draw x-legend (items) */
	if(chart->show_xval)
	{
		x = chart->graph_x + 1.5 + (chart->barw/2);
		y = chart->b - chart->font_h;
		first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

		for(i=first; i<(first+chart->visible) ;i++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, i);

			if( !(i % chart->every_xval) )
			{
				valstr = item->label;
				pango_layout_set_text (layout, valstr, -1);
				pango_layout_get_size (layout, &tw, &th);

				DB( g_print("%s w=%d h=%d\n", valstr, tw, th) );

				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.78);
				cairo_move_to(cr, x - ((tw / PANGO_SCALE)/2), y);
				//cairo_move_to(cr, x, y);
				pango_cairo_show_layout (cr, layout);

				/*cairo_user_set_rgbcol(cr, &global_colors[TEXT]);
				cairo_move_to(cr, x, y);
				cairo_line_to(cr, x, y + te.height);
				cairo_stroke(cr);*/
			}

			x += chart->blkw;
		}
	}

	/* draw y-legend (amount) */

	curxval = chart->max;
	for(i=0;i<=chart->div;i++)
	{
		y = 0.5 + floor(chart->graph_y + ((i * chart->unit) / chart->range) * chart->graph_height);

		DB( g_print(" + i=%d :: y=%f (%f / %f) * %f\n", i, y, i*chart->unit, chart->range, chart->graph_height) );

		if( curxval != 0.0 )
		{
			valstr = chart_print_int(chart, (gint)curxval);
			pango_layout_set_text (layout, valstr, -1);
			pango_layout_get_size (layout, &tw, &th);

			//DB( g_print("'%s', %f %f %f %f %f %f\n", valstr, te.x_bearing, te.y_bearing, te.width, te.height, te.x_advance, te.y_advance) );

			// draw texts
			cairo_move_to(cr, chart->graph_x - (tw / PANGO_SCALE) - 2, y - ((th / PANGO_SCALE)*0.8) );
			cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
			pango_cairo_show_layout (cr, layout);

		}

		curxval -= chart->unit;
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

	x = chart->graph_x;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	cr = gdk_cairo_create (gtk_widget_get_window(widget));
	//cr = cairo_create (chart->surface);

	#if HELPDRAW == 1
	x2 = x + 0.5;
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // violet
	for(i=first; i<=(first+chart->visible) ;i++)
	{
		cairo_move_to(cr, x2, chart->graph_y);
		cairo_line_to(cr, x2, chart->graph_x + chart->graph_height);
		x2 += chart->blkw;
	}
	cairo_stroke(cr);
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
		
		if(item->serie1)
		{
			x2 = x;
			h = floor((item->serie1 / chart->range) * chart->graph_height);
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
			//DB( g_print(" + i=%d :: y2=%f h=%f (%f / %f) * %f\n", i, y2, h, chart->datas1[i], chart->range, chart->graph_height ) );

			cairo_rectangle(cr, x2+2, y2, barw, h);
			cairo_fill(cr);

		}

		if( chart->dual && item->serie2)
		{

			x2 = x + barw + 1;
			h = floor((item->serie2 / chart->range) * chart->graph_height);
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

	if( x <= chart->r && x >= chart->graph_x && y >= chart->graph_y && y <= chart->b )
	{
		px = (x - chart->graph_x);
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

	x = chart->graph_x;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	cr = gdk_cairo_create (gtk_widget_get_window(widget));
	//cr = cairo_create (chart->surface);
	
	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);


	#if HELPDRAW == 1
	x2 = x + 0.5;
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // violet
	for(i=first; i<=(first+chart->visible) ;i++)
	{
		cairo_move_to(cr, x2, chart->graph_y);
		cairo_line_to(cr, x2, chart->graph_x + chart->graph_height);
		x2 += chart->blkw;
	}
	cairo_stroke(cr);

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
		y2 = chart->oy - (item->serie1 / chart->range) * chart->graph_height;
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

	x = chart->graph_x;
	y = chart->oy;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	// draw plots
	for(i=first; i<(first+chart->visible) ;i++)
	{
	ChartItem *item = &g_array_index(chart->items, ChartItem, i);
		
		x2 = x + (chart->blkw)/2;
		y2 = chart->oy - (item->serie1 / chart->range) * chart->graph_height;
		linechart_draw_plot(cr,  x2, y2, i == chart->active ? linew+1 : linew, chart);
		x += chart->blkw;
	}

/* overdrawn */

	DB( g_print(" min=%.2f range=%.2f\n", chart->min, chart->range) );


	if( chart->show_over )
	{
		if(chart->minimum != 0 && chart->minimum >= chart->min)
		{
			y  = 0.5 + chart->oy + (ABS(chart->minimum)/chart->range) * chart->graph_height;
			y2 = (ABS(chart->min)/chart->range) * chart->graph_height - (y - chart->oy) + 1;
			cairo_set_source_rgba(cr, COLTOCAIRO(255), COLTOCAIRO(0), COLTOCAIRO(0), AREA_ALPHA / 2);

			DB( g_print(" draw over: x%f, y%f, w%f, h%f\n", chart->l, y, chart->w, y2) );

			cairo_rectangle(cr, chart->graph_x, y, chart->graph_width, y2 );
			cairo_fill(cr);
			
			cairo_set_line_width(cr, 1.0);
			cairo_set_source_rgb(cr, COLTOCAIRO(255), COLTOCAIRO(0), COLTOCAIRO(0));

			cairo_set_dash (cr, dashed3, 1, 0);
			cairo_move_to(cr, chart->graph_x, y);
			cairo_line_to (cr, chart->graph_x + chart->graph_width, y);
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

	if( x <= chart->r && x >= chart->graph_x && y >= chart->graph_y && y <= chart->b )
	{
		px = (x - chart->graph_x);
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
GtkWidget *drawarea = chart->drawarea;
GtkAllocation allocation;
gint w, h;

	DB( g_print("\n[pie] calculation\n") );


	w = chart->graph_width;
	h = chart->graph_height;

	chart->rayon = MIN(w, h);

	gtk_widget_get_allocation(drawarea, &allocation);
	
	chart->ox    = chart->graph_x + (chart->graph_width / 2);
	chart->oy    = chart->graph_y + (chart->rayon / 2);

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

		for(i=0; i< chart->nb_items ;i++)
		{
		ChartItem *item = &g_array_index(chart->items, ChartItem, i);

			a1 = ((360 * (sum / chart->total)) - 90) * (M_PI / 180);
			sum += ABS(item->serie1);
			a2 = ((360 * (sum / chart->total)) - 90) * (M_PI / 180);
			if(i < chart->nb_items-1) a2 += 0.0175;
			
			dx = cx;
			dy = cy;

			cairo_move_to(cr, dx, dy);
			cairo_arc(cr, dx, dy, radius, a1, a2);

			#if PIE_LINE_SLICE == 1
				cairo_set_line_width(cr, 1.0);
				cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
				cairo_line_to(cr, cx, cy);
				cairo_stroke_preserve(cr);
			#endif

			DB( g_print("%d: %.2f%% %.2f %.2f\n", i, sum / chart->total, a1, a2) );

			//g_print("color : %f %f %f\n", COLTOCAIRO(colors[i].r), COLTOCAIRO(colors[i].g), COLTOCAIRO(colors[i].b));

			color = i % chart->color_scheme.nb_cols;
			cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[color], i == chart->active);
			cairo_fill(cr);
		}

#if SOFT_LIGHT == 1
	cairo_pattern_t *pat1;

	a1 = 0;
	a2 = 2 * M_PI;

	pat1 = cairo_pattern_create_radial( cx, cy, 0, cx, cy, radius);
	cairo_pattern_add_color_stop_rgba(pat1, 0.0, 1.0, 1.0, 1.0, .50);
	cairo_pattern_add_color_stop_rgba(pat1, 0.9, 1.0, 1.0, 1.0, 0.1);

	cairo_arc(cr, cx, cy, radius, a1, a2);
	cairo_set_source(cr, pat1);
	cairo_fill(cr);
#endif

#if GRADIENT == 1
	cairo_pattern_t *pat1;

	a1 = 0;
	a2 = 2 * M_PI;
	double gradius = radius - 8;

	// start point, end point
	pat1 = cairo_pattern_create_linear(cx, cy-gradius, cx, cy+gradius);

	cairo_pattern_add_color_stop_rgba(pat1, 0.0, 1.0, 1.0, 1.0, .15);
	cairo_pattern_add_color_stop_rgba(pat1, 1.0, 1.0, 1.0, 1.0, 0.0);

	//debug
	//cairo_rectangle(cr, 	cx-radius, cy-radius, radius*2, radius*2);

	cairo_arc(cr, cx, cy, gradius, a1, a2);
	cairo_set_source(cr, pat1);
	cairo_fill(cr);

#endif

#if CHART_PIE_DONUT == 1
	a1 = 0;
	a2 = 2 * M_PI;

	//original
	//radius = (gint)((chart->rayon/3) * (1 / PHI));
	//5.1
	//radius = (gint)((chart->rayon/2) * 2 / 3);
	//ynab
	radius = (gint)(chart->rayon/2) * 0.5;

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
double h;

	DB( g_print("\n[pie] get active\n") );

	px = x - chart->ox;
	py = y - chart->oy;
	h  = sqrt( pow(px,2) + pow(py,2) );
	retval = -1;

	if(h < (chart->rayon/2))
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

	DB( g_print("\n[gtkchart] drawarea full redraw\n") );

	cr = cairo_create (chart->surface);

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
#if HELPDRAW == 1
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); //green
	cairo_rectangle(cr, chart->l+0.5, chart->t+0.5, chart->w, chart->h);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, 1.0, 0.5, 0.0); //orange
	cairo_rectangle(cr, chart->graph_x+0.5, chart->graph_y+0.5, chart->graph_width, chart->graph_height);
	cairo_stroke(cr);
#endif


	// draw title
	if(chart->title)
	{
		layout = pango_cairo_create_layout (cr);
	
		chart_set_font_size(chart, CHART_FONT_SIZE_TITLE);
		pango_layout_set_font_description (layout, chart->pfd);
		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);

		cairo_user_set_rgbcol(cr, &global_colors[THTEXT]);
		cairo_move_to(cr, chart->l, chart->t);
		pango_cairo_show_layout (cr, layout);

		#if HELPDRAW == 1
			double dashlength;
			cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
			dashlength = 3;
			cairo_set_dash (cr, &dashlength, 1, 0);
			cairo_move_to(cr, chart->l, chart->t);
			cairo_rectangle(cr, chart->l, chart->t, (tw / PANGO_SCALE), (th / PANGO_SCALE));
			cairo_stroke(cr);
		#endif

		g_object_unref (layout);
	}

	switch(chart->type)
	{
		case CHART_TYPE_COL:
			colchart_draw_scale(widget, chart);
			//colchart_draw_bars(widget, chart);
			colchart_draw_scale_text(widget, chart);
			break;
		case CHART_TYPE_LINE:
			colchart_draw_scale(widget, chart);
			//linechart_draw_lines(widget, chart);
			colchart_draw_scale_text(widget, chart);
			break;
		case CHART_TYPE_PIE:
			//piechart_draw_slices(widget, chart);
			break;
	}

	cairo_destroy(cr);

	return TRUE;
}



static gboolean
drawarea_configure_event (GtkWidget         *widget,
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
	colfound = gtk_style_context_lookup_color(context, "theme_fg_color", &color);
	if(!colfound)
		gtk_style_context_lookup_color(context, "fg_color", &color);

	if( colfound )
	{
		struct rgbcol *tcol = &global_colors[THTEXT];
		tcol->r = color.red * 255;
		tcol->g = color.green * 255;
		tcol->b = color.blue * 255;
		DB( g_print(" - theme text (bg) col: %x %x %x\n", tcol->r, tcol->g, tcol->b) );
	}


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
			update_rect.x = chart->graph_x;
			update_rect.y = chart->graph_y;
			update_rect.width = chart->graph_width;
			update_rect.height = chart->graph_height;

			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
		                          &update_rect,
		                          FALSE);
		}
	
		if(chart->lastactive != -1)
		{
			/* column/line : invalidate rollover */
			if( chart->type != CHART_TYPE_PIE )
			{
				update_rect.x = chart->graph_x + (chart->lastactive - first) * chart->blkw;
				update_rect.y = chart->graph_y - 6;
				update_rect.width = chart->blkw;
				update_rect.height = chart->graph_height + 12;
			}
		
			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
		                          &update_rect,
		                          FALSE);
		}

		/* column/line : invalidate current item */
		if(chart->type != CHART_TYPE_PIE)
		{
			update_rect.x = chart->graph_x + (chart->active - first) * chart->blkw;
			update_rect.y = chart->graph_y - 6;
			update_rect.width = chart->blkw;
			update_rect.height = chart->graph_height + 12;
	
			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
			                      &update_rect,
			                      FALSE);
		}
		
		//gtk_widget_queue_draw( widget );
		//retval = FALSE;
	}

	DB( g_print(" x=%d, y=%d, time=%d\n", x, y, event->time) );
	DB( g_print(" trigger tooltip query\n") );

	gtk_tooltip_trigger_tooltip_query(gtk_widget_get_display(chart->drawarea));

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
		gtk_widget_queue_draw( chart->drawarea );
	}
}


/*
** change the model and/or column
*/
void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column, gchar *title, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHART (chart));

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
		if( GTK_IS_LIST_STORE(chart->legend) )
			gtk_list_store_clear (GTK_LIST_STORE(chart->legend));

	}
}

/*
** change the model and/or column
*/
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2, gchar *title, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHART (chart));

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
		if( GTK_IS_LIST_STORE(chart->legend) )
			gtk_list_store_clear (GTK_LIST_STORE(chart->legend));
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

	chart->minor_rate   = rate;
	chart->minor_symbol = symbol;
}


void gtk_chart_set_absolute(GtkChart * chart, gboolean abs)
{
	g_return_if_fail (GTK_IS_CHART (chart));

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

	chart->minimum = minimum;

	//if(chart->type == CHART_TYPE_LINE)
	//	chart_recompute(chart);
}

/*
** set the every_xval
*/
void gtk_chart_set_every_xval(GtkChart * chart, gint gap)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->every_xval = gap;

	//if(chart->type != CHART_TYPE_PIE)
	//	chart_recompute(chart);
}


/*
** set the barw
*/
void gtk_chart_set_barw(GtkChart * chart, gdouble barw)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->barw = barw;

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
GtkTreeViewColumn *column;

	g_return_if_fail (GTK_IS_CHART (chart));

	if(visible == TRUE)
		gtk_widget_show(chart->scrollwin);
	else
		gtk_widget_hide(chart->scrollwin);

	/* manage column visibility */
	column = gtk_tree_view_get_column (GTK_TREE_VIEW(chart->treeview), 1);	 //amount
	gtk_tree_view_column_set_visible (column, showextracol);
	
	column = gtk_tree_view_get_column (GTK_TREE_VIEW(chart->treeview), 2);	 //percent
	gtk_tree_view_column_set_visible (column, showextracol);
	
}

/*
** change the x-value visibility
*/
void gtk_chart_show_xval(GtkChart * chart, gboolean visible)
{
	g_return_if_fail (GTK_IS_CHART (chart));

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

	chart->minor = minor;

	if(chart->type != CHART_TYPE_PIE)
		gtk_chart_queue_redraw(chart);

	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(chart->treeview));
	gtk_widget_queue_draw (chart->treeview);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* legend list */

#define LEG_SQUARE_SIZE 12

static GdkPixbuf *create_color_pixbuf (struct rgbcol *color)
{
GdkPixbuf *pixbuf;
guint32 pixel;

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, LEG_SQUARE_SIZE, LEG_SQUARE_SIZE);

	pixel  = 0xFF;
	pixel |= (color->r << 24);
	pixel |= (color->g << 16);
	pixel |= (color->b <<  8);

	/*g_print("%08x\n", (0xFF & col->red) << 24 );
	g_print("%08x\n", (0xFF & col->green) << 16 );
	g_print("%08x\n", (0xFF & col->blue) << 8 );
	g_print("%x %x %x => %08x\n", col->red, col->green, col->blue, pixel);*/

	gdk_pixbuf_fill(pixbuf, pixel);

	return pixbuf;
}


static void legend_list_cell_data_function(GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
GdkPixbuf *pixbuf;
gchar *title;

	gtk_tree_model_get(model, iter, 
		LST_LEGEND_COLOR, &pixbuf,
		LST_LEGEND_TITLE, &title,
		-1);

	switch(GPOINTER_TO_INT(user_data))
	{
		case LST_LEGEND_COLOR:
			g_object_set(renderer, "pixbuf", pixbuf, NULL);
			break;
		case LST_LEGEND_TITLE:
			g_object_set(renderer, "text", title, NULL);
			break;
	}

}

static void
legend_list_float_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
GtkChart *chart = user_data;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble amount;

	gtk_tree_model_get(model, iter,
		LST_LEGEND_AMOUNT, &amount,
		-1);

	hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, chart->kcur, chart->minor);

	g_object_set(renderer, 
		"text", buf,
		NULL);

}

static void legend_list_rate_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble rate;
gchar buf[8];

	gtk_tree_model_get(model, iter,
		LST_LEGEND_RATE, &rate,
		-1);

	g_snprintf(buf, sizeof(buf), "%.02f %%", rate);
	g_object_set(renderer, "text", buf, NULL);

}


static GtkWidget *legend_list_new(GtkChart *chart)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	store = gtk_list_store_new(NUM_LST_LEGEND,
		G_TYPE_POINTER,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

#if MYDEBUG == 1
/*	GtkStyle *style;
	PangoFontDescription *font_desc;

	g_print("legend_list_new font\n");
	
	style = gtk_widget_get_style(GTK_WIDGET(view));
	font_desc = style->font_desc;

	g_print("family: %s\n", pango_font_description_get_family(font_desc) );
	g_print("size: %d (%d)\n", pango_font_description_get_size (font_desc), pango_font_description_get_size (font_desc )/PANGO_SCALE );
*/
#endif

	// change the font size to a smaller one
	/*PangoFontDescription *font = pango_font_description_new();
	pango_font_description_set_size (font, 8 * PANGO_SCALE);
	gtk_widget_modify_font(GTK_WIDGET(view), font);
	pango_font_description_free( font );*/
	GtkStyleContext *context;
	PangoFontDescription *desc, *nfd;
	gint nfsize;
	
	context = gtk_widget_get_style_context (chart->drawarea);
	gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL, "font", &desc, NULL);
	nfd = pango_font_description_copy(desc);
	nfsize = pango_font_description_get_size (desc) / PANGO_SCALE;
	pango_font_description_set_size(nfd, MAX(8, nfsize-2) * PANGO_SCALE);
	gtk_widget_override_font(GTK_WIDGET(view), nfd);
	pango_font_description_free (nfd);
	

	// column 1
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, legend_list_cell_data_function, GINT_TO_POINTER(LST_LEGEND_COLOR), NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, legend_list_cell_data_function, GINT_TO_POINTER(LST_LEGEND_TITLE), NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	// column 2
	column = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title(column, name);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, legend_list_float_cell_data_function, chart, NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_spacing( column, 16 );

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_visible (column, FALSE);
	
	// column 3
	column = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title(column, "%");
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", id);
	gtk_tree_view_column_set_cell_data_func(column, renderer, legend_list_rate_cell_data_function, GINT_TO_POINTER(3), NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	//gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	gtk_tree_view_column_set_visible (column, FALSE);


	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_NONE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(view), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);

/*
	GValue value = { 0, };
	g_value_init (&value, G_TYPE_INT);
	g_value_set_int (&value, 20);
	g_object_set_property(view, "vertical-separator", &value);
	g_value_unset (&value);
*/

	return(view);
}

