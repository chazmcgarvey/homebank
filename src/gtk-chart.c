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

#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#include "homebank.h"
#include "gtk-chart-colors.h"
#include "gtk-chart.h"


#define HELPDRAW 0

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif


static void gtk_chart_class_intern_init (gpointer);
static void gtk_chart_class_init      (GtkChartClass *klass);
static void gtk_chart_init            (GtkChart      *chart);
static void gtk_chart_destroy         (GtkObject     *chart);

static gboolean
drawarea_configure_event (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           user_data);
static void drawarea_sizeallocate_callback(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);
static void drawarea_realize_callback(GtkWidget *widget, gpointer user_data);
static gboolean drawarea_draw_callback( GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static gboolean drawarea_motionnotifyevent_callback(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static gboolean drawarea_querytooltip_callback(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data);

static gboolean drawarea_full_redraw(GtkWidget *widget, gpointer user_data);

static void chart_calculation(GtkChart *chart);
static void chart_clear(GtkChart *chart, gboolean store);

static void colchart_first_changed( GtkAdjustment *adj, gpointer user_data);
static void colchart_compute_range(GtkChart *chart);
static void colchart_calculation(GtkChart *chart);
static void colchart_scrollbar_setvalues(GtkChart *chart);

static void piechart_calculation(GtkChart *chart);

static GdkPixbuf *create_color_pixbuf (GdkColor *col);
static GtkWidget *legend_list_new(GtkChart *chart);


static GtkHBoxClass *gtk_chart_parent_class = NULL;


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

		chart_type = g_type_register_static (GTK_TYPE_HBOX, "GtkChart",
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
//GObjectClass *gobject_class;
GtkObjectClass *object_class;
//GtkWidgetClass *widget_class;

	//gobject_class = (GObjectClass*) klass;
	object_class = (GtkObjectClass*) klass;
	//widget_class = (GtkWidgetClass*) klass;

	gtk_chart_parent_class = g_type_class_peek_parent (klass);

	DB( g_print("\n[gtkchart] class_init\n") );

	object_class->destroy = gtk_chart_destroy;

}

static void
gtk_chart_init (GtkChart * chart)
{
GtkWidget *widget, *vbox, *frame;
GtkWidget *scrollwin, *treeview;

	chart->surface = NULL;
 	chart->nb_items = 0;
	chart->items = NULL;
 	chart->title = NULL;
	chart->abs = FALSE;
	chart->dual = FALSE;
	chart->barw = GTK_CHART_BARW;

	chart->active = -1;
	chart->lastactive = -1;

 	chart->minor_rate = 1.0;
	chart->timer_tag = 0;

	gtk_chart_set_color_scheme(chart, CHART_COLMAP_HOMEBANK);
	
	widget=GTK_WIDGET(chart);

	gtk_box_set_homogeneous(GTK_BOX(widget), FALSE);

	vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (widget), vbox, TRUE, TRUE, 0);

	/* drawing area */
	frame = gtk_frame_new(NULL);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);

	chart->drawarea = gtk_drawing_area_new();
	//gtk_widget_set_double_buffered (GTK_WIDGET(widget), FALSE);
	
	gtk_container_add( GTK_CONTAINER(frame), chart->drawarea );
	gtk_widget_set_size_request(chart->drawarea, 150, 150 );
	gtk_widget_set_has_tooltip(chart->drawarea, TRUE);
	gtk_widget_show(chart->drawarea);

#if MYDEBUG == 1
	GtkStyle *style;
	PangoFontDescription *font_desc;

	g_print("draw_area font\n");
	
	style = gtk_widget_get_style(GTK_WIDGET(chart->drawarea));
	font_desc = style->font_desc;

	g_print("family: %s\n", pango_font_description_get_family(font_desc) );
	g_print("size: %d (%d)\n", pango_font_description_get_size (font_desc), pango_font_description_get_size (font_desc )/PANGO_SCALE );

#endif
	
	/* scrollbar */
    chart->adjustment = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
    chart->scrollbar = gtk_hscrollbar_new (GTK_ADJUSTMENT (chart->adjustment));
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

	g_signal_connect( G_OBJECT(chart->drawarea),"configure-event", G_CALLBACK (drawarea_configure_event), chart);
	g_signal_connect( G_OBJECT(chart->drawarea), "size-allocate", G_CALLBACK(drawarea_sizeallocate_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "realize", G_CALLBACK(drawarea_realize_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "expose-event", G_CALLBACK(drawarea_draw_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "query-tooltip", G_CALLBACK(drawarea_querytooltip_callback), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "motion-notify-event", G_CALLBACK(drawarea_motionnotifyevent_callback), chart );

	g_signal_connect (G_OBJECT(chart->adjustment), "value_changed", G_CALLBACK (colchart_first_changed), chart);

	//g_signal_connect( G_OBJECT(chart->drawarea), "map-event", G_CALLBACK(chart_map), chart ) ;
	//g_signal_connect( G_OBJECT(chart->drawarea), "button-press-event", G_CALLBACK(chart_button_press), chart );
	//g_signal_connect( G_OBJECT(chart->drawarea), "button-release-event", G_CALLBACK(chart_button_release), chart );
}


/* --- */

GtkWidget *
gtk_chart_new (gint type)
{
GtkChart *chart;

	DB( g_print("\n[gtkchart] new\n") );

	chart = g_object_new (GTK_TYPE_CHART, NULL);
	chart->type = type;

	return GTK_WIDGET(chart);
}


static void
gtk_chart_destroy (GtkObject * object)
{
GtkChart *chart;

	DB( g_print("\n[gtkchart] destroy\n") );

	g_return_if_fail (GTK_IS_CHART (object));

	chart = GTK_CHART (object);

	chart_clear(chart, FALSE);

	if (chart->surface)
	{
		cairo_surface_destroy (chart->surface);
		chart->surface = NULL;
	}
	GTK_OBJECT_CLASS (gtk_chart_parent_class)->destroy (object);
}




/*
** print a integer number
*/
static gchar *chart_print_int(GtkChart *chart, gint value)
{

	//mystrfmon(chart->buffer, CHART_BUFFER_LENGTH-1, (gdouble)value, chart->minor);
	mystrfmon_int(chart->buffer1, CHART_BUFFER_LENGTH-1, (gdouble)value, chart->minor);

	/*
	if(chart->minor)
	{
		value *= chart->minor_rate;
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%!.0n ", (gdouble)value);
		strcat(chart->buffer, chart->minor_symbol);
	}
	else
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%.0n", (gdouble)value);
	*/

	return chart->buffer1;
}

/*
** print a double number
*/
static gchar *chart_print_double(GtkChart *chart, gchar *buffer, gdouble value)
{

	mystrfmon(buffer, CHART_BUFFER_LENGTH-1, value, chart->minor);

	/*
	if(chart->minor)
	{
		value *= chart->minor_rate;
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%!n ", (gdouble)value);
		strcat(chart->buffer, chart->minor_symbol);
	}
	else
		strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, "%n", (gdouble)value);
	*/

	return buffer;
}


/*
** clear any allocated memory
*/
static void chart_clear(GtkChart *chart, gboolean store)
{
gint i;

	DB( g_print("\n[gtkchart] clear\n") );

	//free & clear any previous allocated datas
	if(chart->title != NULL)
	{
		g_free(chart->title);
		chart->title = NULL;
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

	if(store == TRUE)
	{
		gtk_list_store_clear (GTK_LIST_STORE(chart->legend));
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
GdkColor colour;

	DB( g_print("\n[gtkchart] setup with model\n") );

	chart_clear(chart, TRUE);
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

		/* populate our legend list */

		color = i % chart->nb_cols;
		//color = id % chart->nb_cols;

		//DB( g_print ("Row %d: (%s, %2.f) color %d\n", id, title, value, color) );

		colour.red   = COLTO16(chart->colors[color].r);
		colour.green = COLTO16(chart->colors[color].g);
		colour.blue  = COLTO16(chart->colors[color].b);

        gtk_list_store_append (GTK_LIST_STORE(chart->legend), &l_iter);
        gtk_list_store_set (GTK_LIST_STORE(chart->legend), &l_iter,
                            LST_LEGEND_COLOR, create_color_pixbuf (&colour),
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

	DB( g_print("\n[gtkchart] bar compute range\n") );

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
cairo_text_extents_t te;
cairo_font_extents_t fe;
GtkAllocation allocation;
gchar *valstr;

	
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

	// compute title
	chart->title_zh = 0;
	if(chart->title != NULL)
	{
		cairo_set_font_size(cr, CHART_FONT_SIZE_TITLE);
		cairo_font_extents(cr, &fe);
		chart->title_zh = fe.height;
	}

	// compute subtitle
	chart->subtitle_zh = 0;
	if(chart->subtitle != NULL)
	{
		cairo_set_font_size(cr, CHART_FONT_SIZE_PERIOD);
		cairo_font_extents(cr, &fe);
		chart->subtitle_zh = fe.height;
	}

	chart->subtitle_y = chart->t + chart->title_zh;

	cairo_set_font_size(cr, CHART_FONT_SIZE_NORMAL);

	// compute amount scale
	valstr = chart_print_int(chart, (gint)chart->min);
	cairo_text_extents(cr, valstr, &te);
	chart->scale_w = te.width;
	valstr = chart_print_int(chart, (gint)chart->max);
	cairo_text_extents(cr, valstr, &te);
	chart->scale_w = MAX(chart->scale_w, te.width);
	DB( g_print(" - scale: %g,%g %g,%g\n", chart->l, 0.0, chart->scale_w, 0.0) );

	// compute font height
	cairo_font_extents(cr, &fe);
	chart->font_h = fe.height;

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

	cairo_destroy(cr);
	cairo_surface_destroy(surf);

}


static void colchart_calculation(GtkChart *chart)
{
gint blkw;

	DB( g_print("\n[gtkchart] bar calculation\n") );


	//if expand : we compute available space
	//chart->barw = MAX(32, (chart->graph_width)/chart->nb_items);
	//chart->barw = 32; // usr setted or defaut to BARW

	// if fixed
	blkw = chart->barw + 3;
	if( chart->dual )
		blkw = (chart->barw * 2) + 3;

	chart->blkw = blkw;
	chart->visible = chart->graph_width / blkw;
	chart->visible = MIN(chart->visible, chart->nb_items);

	chart->ox = chart->l;
	chart->oy = floor(chart->graph_y + (chart->max/chart->range) * chart->graph_height);

	DB( g_print(" + ox=%f oy=%f\n", chart->ox, chart->oy) );

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

	DB( g_print("\n(gtkline) draw scale\n") );

cairo_t *cr;
//static const double dashed3[] = {2.0};

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

		for(i=first; i<(first+chart->visible) ;i++)
		{
			if( !(i % chart->every_xval) )
			{
				//cairo_user_set_rgbcol(cr, &global_colors[GREY1]);
				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.05);

				cairo_move_to(cr, x, chart->graph_y);
				cairo_line_to(cr, x, chart->b);
				cairo_stroke(cr);
			}

			x += chart->blkw;
		}
	}

	/* horizontal lines */

	curxval = chart->max;
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

	DB( g_print("----------------------\n(gtkline) draw scale text\n") );

cairo_t *cr;
cairo_text_extents_t te;

	//GdkWindow *gdkwindow;
	//gdkwindow = gtk_widget_get_window(widget);

	//cr = gdk_cairo_create (gdkwindow);
	//cr = gdk_cairo_create (widget->window);
	cr = cairo_create (chart->surface);
	
	cairo_set_line_width(cr, 1);

	/* clip */
	//cairo_rectangle(cr, CHART_MARGIN, 0, chart->w, chart->h + CHART_MARGIN);
	//cairo_clip(cr);

	//cairo_set_operator(cr, CAIRO_OPERATOR_SATURATE);

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
				cairo_text_extents(cr, valstr, &te);

				DB( g_print("%s w=%f h=%f\n", valstr, te.width, te.height) );

				cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.78);
				//cairo_move_to(cr, x - (te.width/2), y  - te.y_bearing);
				cairo_move_to(cr, x, y  - te.y_bearing);
				cairo_show_text(cr, valstr);

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
			cairo_text_extents(cr, valstr, &te);

			//DB( g_print("'%s', %f %f %f %f %f %f\n", valstr, te.x_bearing, te.y_bearing, te.width, te.height, te.x_advance, te.y_advance) );

			// draw texts
			cairo_move_to(cr, chart->graph_x - te.x_bearing - te.width - 2, y + (( te.height)/2));
			cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
			cairo_show_text(cr, valstr);
			
		}

		curxval -= chart->unit;
	}

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

	DB( g_print("\n[gtkchart] bar draw bars\n") );

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

		color = i % chart->nb_cols;

		cairo_user_set_rgbcol_over(cr, &chart->colors[color], i == chart->active);
		
		if(item->serie1)
		{
			x2 = x;
			h = floor((item->serie1 / chart->range) * chart->graph_height);
			y2 = chart->oy - h;
			if(item->serie1 < 0.0)
				y2 += 1;

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

	DB( g_print("\n[gtkchart] bar first changed\n") );

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

	DB( g_print("\n[gtkchart] sb_set_values\n") );

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
	cairo_user_set_rgbcol(cr, &chart->colors[chart->cs_blue]);
	cairo_fill(cr);
}


static void linechart_draw_lines(GtkWidget *widget, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;
double x, y, x2, y2, firstx, lastx, linew;
gint first, i;


	DB( g_print("\n(gtkline) line draw lines\n") );

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
		cairo_stroke(cr);
		x2 += chart->blkw;
	}
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

	cairo_user_set_rgbcol(cr, &chart->colors[chart->cs_blue]);
	cairo_stroke_preserve(cr);

	cairo_line_to(cr, lastx, y);
	cairo_line_to(cr, firstx, y);
	cairo_close_path(cr);

	cairo_user_set_rgbacol(cr, &chart->colors[chart->cs_blue], 0.15);
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
	if( chart->show_over )
	{
		if(chart->minimum != 0 && chart->minimum >= chart->min)
		{
			y = chart->oy - (chart->minimum/chart->range) * chart->graph_height;
			cairo_set_source_rgba(cr, COLTOCAIRO(255), COLTOCAIRO(0), COLTOCAIRO(0), .15);

			DB( g_print(" draw over: %f, %f, %f, %f\n", chart->l, y, chart->w, chart->b - y) );

			cairo_rectangle(cr, chart->graph_x, y, chart->graph_width, chart->b - y);
			cairo_fill(cr);
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

	retval = -1;

	if( x <= chart->r && x >= chart->l )
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

			color = i % chart->nb_cols;
			cairo_user_set_rgbcol_over(cr, &chart->colors[color], i == chart->active);
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
	radius = (gint)((chart->rayon/3) * (1 / PHI));

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
cairo_text_extents_t te;
cairo_font_extents_t fe;
cairo_t *cr;

	DB( g_print("\n[gtkchart] drawarea full redraw\n") );


	
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

	/* taken from scrolled window
    gtk_paint_shadow (widget->style, widget->window,
			GTK_STATE_NORMAL, scrolled_window->shadow_type,
			area, widget, "scrolled_window",
			widget->allocation.x + relative_allocation.x,
			widget->allocation.y + relative_allocation.y,
			relative_allocation.width,
			relative_allocation.height);
	*/
	

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
	cairo_set_font_size(cr, CHART_FONT_SIZE_TITLE);
	cairo_font_extents(cr, &fe);
	cairo_text_extents(cr, chart->title, &te);

#if HELPDRAW == 1
	double dashlength;
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
	dashlength = 3;
	cairo_set_dash (cr, &dashlength, 1, 0);
	cairo_move_to(cr, chart->l, chart->t);
	cairo_rectangle(cr, chart->l + te.x_bearing, chart->t, te.width, fe.height);
	cairo_stroke(cr);
#endif

	//center title
	//cairo_move_to(cr, chart->l + (chart->w/2) - ((te.width - te.x_bearing) / 2), chart->t - te.y_bearing);
	cairo_move_to(cr, chart->l, chart->t - te.y_bearing);
	//cairo_user_set_rgbcol(cr, &global_colors[BLACK]);
	cairo_user_set_rgbcol(cr, &global_colors[THTEXT]);
	cairo_show_text(cr, chart->title);

	cairo_destroy(cr);

	if(chart->nb_items == 0)
		return FALSE;

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

	return TRUE;
}


static gboolean
drawarea_configure_event (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           user_data)
{
GtkChart *chart = GTK_CHART(user_data);
GtkAllocation allocation;
GtkStyle *style;
GdkColor *color;

	DB( g_print("\n[gtkchart] drawarea configure \n") );

	DB( g_print("w=%d h=%d\n", allocation.width, allocation.height) );

	gtk_widget_get_allocation (widget, &allocation);

	chart_recompute(chart);   
	
	if (chart->surface)
		cairo_surface_destroy (chart->surface);

	chart->surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               allocation.width,
                                               allocation.height);


		// get theme color
		style = gtk_widget_get_style (widget);
		//style = gtk_widget_get_style (chart->treeview);
		//style = gtk_widget_get_default_style();

		//get text color
		color = &style->text[GTK_STATE_NORMAL];
		struct rgbcol *tcol = &global_colors[THTEXT];
		tcol->r = color->red;
		tcol->g = color->green;
		tcol->b = color->blue;
		DB( g_print(" - theme text col: %x %x %x\n", tcol->r, tcol->g, tcol->b) );

		// get base color
		color = &style->base[GTK_STATE_NORMAL];
		tcol = &global_colors[THBASE];
		tcol->r = color->red;
		tcol->g = color->green;
		tcol->b = color->blue;
		DB( g_print(" - theme base col: %x %x %x\n", tcol->r, tcol->g, tcol->b) );

	
	drawarea_full_redraw(widget, user_data);

	
  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}


static void drawarea_sizeallocate_callback(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
	
	DB( g_print("\n[gtkchart] drawarea sizeallocate\n") );
	DB( g_print("w=%d h=%d\n", allocation->width, allocation->height) );

	 //g_print("\n[gtkchart] drawarea sizeallocate\n") ;
	 //g_print("w=%d h=%d\n", allocation->width, allocation->height) ;


	if( gtk_widget_get_realized(widget))
	{
		chart_recompute(chart);
	}

}


static void drawarea_realize_callback(GtkWidget *widget, gpointer   user_data)
{
//GtkChart *chart = GTK_CHART(user_data);
	
	DB( g_print("\n[gtkchart] drawarea realize\n") );

	//chart_recompute(chart);

}



static gboolean drawarea_draw_callback( GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
cairo_t *cr;

	DB( g_print("\n[gtkchart] drawarea expose\n") );

	DB( g_print(" type=%d regionempty=%d\n", event->type, cairo_region_is_empty(event->region)) );

	
	
	cr = gdk_cairo_create (gtk_widget_get_window (widget));

	cairo_set_source_surface (cr, chart->surface, 0, 0);
	//gdk_cairo_rectangle (cr, &event->area);
	cairo_paint (cr);

	/* here draw only line, bar, slices */
	if(chart->nb_items == 0)
		return FALSE;

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
	

	
	cairo_destroy (cr);

	return FALSE;
}


static gboolean drawarea_querytooltip_callback(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data)
{
GtkChart *chart = GTK_CHART(user_data);
gchar *strval, *strval2, *buffer;
gboolean retval = FALSE;
	
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

			if( chart->type == CHART_TYPE_PIE )
				buffer = g_markup_printf_escaped("%s\n%s\n%.2f%%", item->label, strval, item->rate);
			else
				buffer = g_markup_printf_escaped("%s\n%s", item->label, strval);

		}
		else
		{
			strval2 = chart_print_double(chart, chart->buffer2, item->serie2);
			buffer = g_markup_printf_escaped("%s\n+%s\n%s", item->label, strval2, strval);
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

	if(chart->nb_items == 0)
		return FALSE;

	DB( g_print("\n[gtkchart] drawarea motion\n") );
	x = event->x;
	y = event->y;

	//todo see this
	if(event->is_hint)
	{
		DB( g_print(" is hint\n") );

		//gdk_window_get_device_position(event->window, event->device, &x, &y, NULL);
		gdk_window_get_pointer(event->window, &x, &y, NULL);
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
		DB( g_print(" rollover redraw :: active=%d\n", chart->active) );
		//chart->drawmode = CHART_DRAW_OVERCHANGE;
		//gtk_widget_queue_draw_area(widget, chart->graph_x, chart->graph_y, chart->graph_width, chart->graph_height);

		gtk_widget_queue_draw( widget );
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
	chart_recompute(chart);
	drawarea_full_redraw(chart->drawarea, chart);
	gtk_widget_queue_draw( chart->drawarea );
}


/*
** change the model and/or column
*/
void gtk_chart_set_datas(GtkChart *chart, GtkTreeModel *model, guint column, gchar *title)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column, column);
		if(title != NULL)
			chart->title = g_strdup(title);



		gtk_chart_queue_redraw(chart);
	}
	else
	{
		chart_clear(chart, TRUE);
	}
}

/*
** change the model and/or column
*/
void gtk_chart_set_dualdatas(GtkChart *chart, GtkTreeModel *model, guint column1, guint column2, gchar *title)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	if( GTK_IS_TREE_MODEL(model) )
	{
		chart_setup_with_model(chart, model, column1, column2 );
		if(title != NULL)
			chart->title = g_strdup(title);



		gtk_chart_queue_redraw(chart);
	}
	else
	{
		chart_clear(chart, TRUE);
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

void gtk_chart_set_color_scheme(GtkChart * chart, gint colorscheme)
{
	chart->cs_blue = 0;

	switch(colorscheme)
	{
		default:
		case CHART_COLMAP_HOMEBANK:
			chart->colors = homebank_colors;
			chart->nb_cols = homebank_nbcolors;
			break;
		case CHART_COLMAP_MSMONEY:
			chart->colors = money_colors;
			chart->nb_cols = money_nbcolors;
			chart->cs_blue = 1;
			break;
		case CHART_COLMAP_SAP:
			chart->colors = sap_colors;
			chart->nb_cols = sap_nbcolors;
			break;
		case CHART_COLMAP_QUICKEN:
			chart->colors = quicken_colors;
			chart->nb_cols = quicken_nbcolors;
			chart->cs_blue = 3;
			break;
		case CHART_COLMAP_OFFICE2010:
			chart->colors = office2010_colors;
			chart->nb_cols = office2010_nbcolors;
			break;
		case CHART_COLMAP_OFFICE2013:
			chart->colors = office2013_colors;
			chart->nb_cols = office2013_nbcolors;
			break;
		case CHART_COLMAP_ANALYTICS:
			chart->colors = analytics_colors;
			chart->nb_cols = analytics_nbcolors;
			break;
	}
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

/*
void gtk_chart_set_currency(GtkChart * chart, guint32 kcur)
{
	g_return_if_fail (GTK_IS_CHART (chart));

	chart->kcur = kcur;
}
*/

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
}




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* legend list */

static GdkPixbuf *
//create_color_pixbuf (const char *color)
create_color_pixbuf (GdkColor *col)
{
        GdkPixbuf *pixbuf;
        //GdkColor col = color;

        int x;
        int num;
        guchar *p;

/*
        if (!gdk_color_parse (color, &col))
                return NULL;
                */

#define squaredim 12

        pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                 FALSE, 8, //bits
                                 squaredim, squaredim);		//width,height

        //rowstride = gdk_pixbuf_get_rowstride (pixbuf);
        p = gdk_pixbuf_get_pixels (pixbuf);

        num = gdk_pixbuf_get_width (pixbuf) *
                gdk_pixbuf_get_height (pixbuf);

        for (x = 0; x < num; x++) {

                p[0] = col->red;
                p[1] = col->green;
                p[2] = col->blue;

				/*
                p[0] = col->red / 65535 * 255;
                p[1] = col->green / 65535 * 255;
                p[2] = col->blue / 65535 * 255;
                */
                p += 3;
        }

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

	//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur);
	//todo: manage GLOBALS->minor eq
	mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, chart->minor);

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
	PangoFontDescription *font = pango_font_description_new();
	pango_font_description_set_size (font, 8 * PANGO_SCALE);
	gtk_widget_modify_font(GTK_WIDGET(view), font);
	pango_font_description_free( font );

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

