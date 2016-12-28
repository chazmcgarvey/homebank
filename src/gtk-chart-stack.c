/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2013 Maxime DOYEN
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
#include "gtk-chart-stack.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif


#define HELPDRAW 0



/* --- prototypes --- */
static void ui_chart_stack_class_init      (ChartStackClass *klass);
static void ui_chart_stack_init            (ChartStack      *chart);
static void ui_chart_stack_destroy         (GtkWidget     *chart);
/*static void	ui_chart_stack_set_property		 (GObject           *object,
						  guint              prop_id,
						  const GValue      *value,
						  GParamSpec        *pspec);*/

static gboolean drawarea_configure_event_callback (GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
static gboolean drawarea_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean drawarea_scroll_event_callback( GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
static gboolean drawarea_motionnotifyevent_callback(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
static void ui_chart_stack_first_changed( GtkAdjustment *adj, gpointer user_data);

static void ui_chart_stack_clear(ChartStack *chart, gboolean store);

static void ui_chart_stack_queue_redraw(ChartStack *chart);

/* --- variables --- */
static GtkBoxClass *parent_class = NULL;


/* --- functions --- */
GType ui_chart_stack_get_type ()
{
static GType ui_chart_stack_type = 0;

	if (G_UNLIKELY(ui_chart_stack_type == 0))
    {
		const GTypeInfo ui_chart_stack_info =
		{
		sizeof (ChartStackClass),
		NULL,	/* base_init */
		NULL,	/* base_finalize */
		(GClassInitFunc) ui_chart_stack_class_init,
		NULL,	/* class_finalize */
		NULL,	/* class_init */
		sizeof (ChartStack),
		0,		/* n_preallocs */
		(GInstanceInitFunc) ui_chart_stack_init,
		NULL	/* value_table */
		};

		ui_chart_stack_type = g_type_register_static (GTK_TYPE_BOX, "ChartStack",
							 &ui_chart_stack_info, 0);

	}
	return ui_chart_stack_type;
}


static void ui_chart_stack_class_init (ChartStackClass * class)
{
//GObjectClass *gobject_class;
GtkWidgetClass *widget_class;

	DB( g_print("\n[chartstack] class_init\n") );

	//gobject_class = (GObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = g_type_class_peek_parent (class);

	//gobject_class->dispose = ui_chart_stack_dispose;
	//gobject_class->finalize = ui_chart_stack_finalize;
	//gobject_class->set_property = ui_chart_stack_set_property;
	//gobject_class->get_property = ui_chart_stack_get_property;

	widget_class->destroy = ui_chart_stack_destroy;

	
}

/* get/set properties goes here */


static void
ui_chart_stack_init (ChartStack * chart)
{
GtkWidget *widget, *hbox, *scrollwin;


	DB( g_print("\n[chartstack] init\n") );

	chart->surface = NULL;
 	chart->nb_items = 0;
	chart->active = -1;
 	chart->title = NULL;
	chart->subtitle = NULL;
 	chart->pfd = NULL;
 	
 	chart->budget_title = "Budget";
 	chart->result_title = "Result";
 
	chart->barw = GTK_CHARTSTACK_BARW;
	ui_chart_stack_set_color_scheme(chart, CHART_COLMAP_HOMEBANK);

	widget=GTK_WIDGET(chart);

	gtk_box_set_homogeneous(GTK_BOX(widget), FALSE);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (widget), hbox, TRUE, TRUE, 0);

	/* drawing area */
	scrollwin = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(scrollwin), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	//scrollwin = gtk_scrolled_window_new(NULL,NULL);
    //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    //gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	chart->drawarea = gtk_drawing_area_new();
	//gtk_widget_set_double_buffered (GTK_WIDGET(widget), FALSE);
	
	gtk_container_add( GTK_CONTAINER(scrollwin), chart->drawarea );
	gtk_widget_set_size_request(chart->drawarea, 150, 150 );
	gtk_widget_set_has_tooltip(chart->drawarea, FALSE);
	gtk_widget_show(chart->drawarea);
	
	/* scrollbar */
    chart->adjustment = GTK_ADJUSTMENT(gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
    chart->scrollbar = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL,GTK_ADJUSTMENT (chart->adjustment));
    gtk_box_pack_start (GTK_BOX (hbox), chart->scrollbar, FALSE, TRUE, 0);

	gtk_widget_set_events(GTK_WIDGET(chart->drawarea),
		GDK_EXPOSURE_MASK |
		//GDK_POINTER_MOTION_MASK |
		//GDK_POINTER_MOTION_HINT_MASK |
		GDK_SCROLL_MASK
		);

	g_signal_connect( G_OBJECT(chart->drawarea), "configure-event", G_CALLBACK (drawarea_configure_event_callback), chart);
	//g_signal_connect( G_OBJECT(chart->drawarea), "realize", G_CALLBACK(drawarea_realize_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "draw", G_CALLBACK(drawarea_draw_callback), chart );
	//g_signal_connect( G_OBJECT(chart->drawarea), "query-tooltip", G_CALLBACK(drawarea_querytooltip_callback), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "scroll-event", G_CALLBACK(drawarea_scroll_event_callback), chart ) ;
	g_signal_connect( G_OBJECT(chart->drawarea), "motion-notify-event", G_CALLBACK(drawarea_motionnotifyevent_callback), chart );

	g_signal_connect (G_OBJECT(chart->adjustment), "value-changed", G_CALLBACK (ui_chart_stack_first_changed), chart);

	/*
	g_signal_connect( G_OBJECT(chart->drawarea), "leave-notify-event", G_CALLBACK(ui_chart_stack_leave), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "enter-notify-event", G_CALLBACK(ui_chart_stack_enter), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "button-press-event", G_CALLBACK(ui_chart_stack_button_press), chart );
	g_signal_connect( G_OBJECT(chart->drawarea), "button-release-event", G_CALLBACK(ui_chart_stack_button_release), chart );
	*/


}


GtkWidget *
ui_chart_stack_new (void)
{
GtkWidget *chart;

	chart = (GtkWidget *)g_object_new (GTK_TYPE_CHARTSTACK, NULL);

	return chart;
}


void
ui_chart_stack_destroy (GtkWidget * object)
{
ChartStack *chart = GTK_CHARTSTACK(object);

	g_return_if_fail (GTK_IS_CHARTSTACK (object));

	ui_chart_stack_clear(GTK_CHARTSTACK (object), FALSE);

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

	GTK_WIDGET_CLASS (parent_class)->destroy (object);
}




/*
** print a integer number
*/
static gchar *ui_chart_stack_print_int(ChartStack *chart, gdouble value)
{

	hb_strfmon(chart->buffer, CHART_BUFFER_LENGTH-1, value, chart->kcur, chart->minor);
	return chart->buffer;
}


static void ui_chart_stack_clear(ChartStack *chart, gboolean store)
{

	DB( g_print("\n[chartstack] clear\n") );

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
		/*for(i=0;i<chart->nb_items;i++)
		{
		StackItem *item = &g_array_index(chart->items, StackItem, i);

			//g_free(item->legend);
		}*/
		g_array_free(chart->items, TRUE);
		chart->items =  NULL;
	}

	chart->nb_items = 0;

}


static void ui_chart_stack_setup_with_model(ChartStack *chart, GtkTreeModel *list_store, gchar *coltitle1, gchar *coltitle2)
{
guint i;
gboolean valid;
GtkTreeIter iter;

	DB( g_print("\n[chartstack] setup with model\n") );

	ui_chart_stack_clear(chart, TRUE);

	chart->nb_items = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_store), NULL);

	chart->items = g_array_sized_new(FALSE, FALSE, sizeof(StackItem), chart->nb_items);

	DB( g_print(" nb=%d\n", chart->nb_items) );

	if(coltitle1)
		chart->budget_title = coltitle1;
	if(coltitle2)
		chart->result_title = coltitle2;

	/* Get the first iter in the list */
	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store), &iter);
	i = 0;
	while (valid)
    {
	gint id;
	gchar *label, *status;
	gdouble	value1, value2;
	StackItem item;

		gtk_tree_model_get (GTK_TREE_MODEL(list_store), &iter,
			0, &id,
			//1, &pos,
			2, &label,
			3, &value1,   //spent
			4, &value2,   //budget
			//5, &result,
			6, &status,
			-1);

		item.label = label;
		item.spent = value1;
		item.budget = value2;
		item.status = status;

		/* additional pre-compute */	
		item.result = item.spent - item.budget;
		item.rawrate = 0;
		if(ABS(item.budget) > 0)
		{
			item.rawrate = item.spent / item.budget;
		}

		item.warn = item.result < 0.0 ? TRUE : FALSE;

		item.rate = CLAMP(item.rawrate, 0, 1.0);
		
		g_array_append_vals(chart->items, &item, 1);

		i++;
		valid = gtk_tree_model_iter_next (list_store, &iter);
	}

}


static void ui_chart_stack_set_font_size(ChartStack *chart, gint font_size)
{
gint size = 10;

	DB( g_print("\n[chartstack] set font size\n") );

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

	DB( g_print(" size=%d\n", size) );

	pango_font_description_set_size(chart->pfd, size * PANGO_SCALE);

}



static void ui_chart_stack_calculation(ChartStack *chart)
{
GtkWidget *drawarea = chart->drawarea;
cairo_surface_t *surf;
cairo_t *cr;
gint blkw;
gint i;
int tw, th;
gchar *valstr;
GtkAllocation allocation;
PangoLayout *layout;

	DB( g_print("\n[chartstack] bar calculation\n") );

	gtk_widget_get_allocation(drawarea, &allocation);

	chart->l = CHART_MARGIN;
	chart->t = CHART_MARGIN;
	chart->r = allocation.width  - CHART_MARGIN;
	chart->b = allocation.height - CHART_MARGIN;
	chart->w = allocation.width  - (CHART_MARGIN*2);
	chart->h = allocation.height - (CHART_MARGIN*2);

	//todo: seems not working well...
	surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
	cr = cairo_create (surf);

	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (cr);

	
	// compute title
	chart->title_zh = 0;
	if(chart->title)
	{
		//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_TITLE * PANGO_SCALE);
		ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_TITLE);
		pango_layout_set_font_description (layout, chart->pfd);

		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->title_zh = (th / PANGO_SCALE) + CHART_SPACING;
		DB( g_print(" - title: %s w=%d h=%d\n", chart->title, tw, th) );
	}
		
	// compute period
	chart->subtitle_zh = 0;
	if(chart->subtitle)
	{
		//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_PERIOD * PANGO_SCALE);
		ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_SUBTITLE);
		pango_layout_set_font_description (layout, chart->pfd);

		pango_layout_set_text (layout, chart->subtitle, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->subtitle_zh = (th / PANGO_SCALE) + CHART_SPACING;
		DB( g_print(" - period: %s w=%d h=%d\n", chart->subtitle, tw, th) );
	}
	
	// compute other text
	//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_NORMAL * PANGO_SCALE);
	ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_NORMAL);
	pango_layout_set_font_description (layout, chart->pfd);

	double title_w = 0;
	chart->bud_col_w = 0;
	chart->rel_col_w = 0;

	gdouble maxbudget = 0;
	gdouble maxresult = 0;
	gdouble result;
	for(i=0;i<chart->nb_items;i++)
	{
	StackItem *item = &g_array_index(chart->items, StackItem, i);
	
		// category width
		pango_layout_set_text (layout, item->label, -1);
		pango_layout_get_size (layout, &tw, &th);
		title_w = MAX(title_w, (tw / PANGO_SCALE));

		DB( g_print(" - calc '%s' title_w=%f (w=%d)\n", item->label, title_w, tw) );

		//result = ABS(chart->spent[i]) - ABS(chart->budget[i]);
		result = ABS(item->spent - item->budget);
		
		maxbudget = MAX(maxbudget, ABS(item->budget) );
		maxresult = MAX(maxresult, result);

		DB( g_print(" - maxbudget maxbudget=%f (w=%d)\n", maxbudget, tw) );

		pango_layout_set_text (layout, item->status, -1);
		pango_layout_get_size (layout, &tw, &th);
		chart->rel_col_w = MAX(chart->rel_col_w, (tw / PANGO_SCALE));
	}
	
	chart->rel_col_w += CHART_SPACING;

	// compute budget/result width
	valstr = ui_chart_stack_print_int(chart, -maxbudget);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->bud_col_w = (tw / PANGO_SCALE);
	pango_layout_set_text (layout, chart->budget_title, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->bud_col_w = MAX(chart->bud_col_w, (tw / PANGO_SCALE));
	DB( g_print(" - budget-col: w=%f, %.2f, '%s'\n", chart->bud_col_w, maxbudget, valstr) );

	
	valstr = ui_chart_stack_print_int(chart, -maxresult);
	pango_layout_set_text (layout, valstr, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->res_col_w = (tw / PANGO_SCALE);
	pango_layout_set_text (layout, chart->result_title, -1);
	pango_layout_get_size (layout, &tw, &th);
	chart->res_col_w = MAX(chart->res_col_w, (tw / PANGO_SCALE));
	DB( g_print(" - result-col: w=%f, %.2f, '%s'\n", chart->res_col_w, maxresult, valstr) );

	
	// collect other width, add margins
	chart->header_zh = (th / PANGO_SCALE) + CHART_SPACING;
	chart->cat_col_w = title_w + CHART_SPACING;

	//chart->title_y = chart->t;
	chart->subtitle_y = chart->t + chart->title_zh;
	chart->header_y = chart->subtitle_y + chart->subtitle_zh;

	
	chart->graph_width  = chart->w - chart->cat_col_w - chart->bud_col_w - chart->res_col_w - chart->rel_col_w - (double)(CHART_SPACING*4);
	chart->graph_height = chart->h - chart->title_zh - chart->subtitle_zh -chart->header_zh;


	DB( g_print("gfx_w = %.2f - %.2f - %.2f  - %.2f - %.2f \n",
		            chart->w , chart->cat_col_w , chart->bud_col_w , chart->res_col_w , (double)(CHART_SPACING*4)) );
	
	DB( g_print("gfx_w = %.2f\n", chart->graph_width) );

	//if expand : we compute available space
	//chart->barw = MAX(32, (chart->graph_width)/chart->nb_items);
	//chart->barw = 32; // usr setted or defaut to BARW

	blkw = chart->barw + floor(chart->barw * 0.2);
	chart->blkw = blkw;
	
	chart->visible = (chart->graph_height - chart->t) / blkw;
	chart->visible = MIN(chart->visible, chart->nb_items);

	g_object_unref (layout);
	
	cairo_destroy(cr);
	cairo_surface_destroy(surf);

}


#if HELPDRAW == 1
static void ui_chart_stack_draw_help(GtkWidget *widget, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
cairo_t *cr;
double x, y, y2;
gint first = 0;
gint i;

	//cr = gdk_cairo_create (gtk_widget_get_window(widget));
	cr = cairo_create (chart->surface);

	cairo_set_line_width (cr, 1);


	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); //green
	cairo_rectangle(cr, chart->l+0.5, chart->t+0.5, chart->w, chart->h);
	cairo_stroke(cr);

	
	//title rect
	cairo_set_source_rgb(cr, .0, .0, 1.0);
	cairo_rectangle(cr, chart->l+0.5, chart->t+0.5, chart->w, chart->title_zh);
	cairo_stroke(cr);

	//period rect
	cairo_set_source_rgb(cr, .0, 0, 1.0);
	cairo_rectangle(cr, chart->l+0.5, chart->subtitle_y+0.5, chart->w, chart->subtitle_zh);
	cairo_stroke(cr);
	
	//header rect
	cairo_set_source_rgb(cr, .0, 1.0, 1.0);
	cairo_rectangle(cr, chart->l+0.5, chart->header_y+0.5, chart->w, chart->header_zh);
	cairo_stroke(cr);

	//category column
	y = chart->t + chart->title_zh + chart->header_zh + chart->subtitle_zh;
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
	cairo_rectangle(cr, chart->l+0.5, y+0.5, chart->cat_col_w, chart->h - y);
	cairo_stroke(cr);

	//budget column
	x = chart->l + chart->cat_col_w + chart->graph_width + CHART_SPACING ;
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
	cairo_rectangle(cr, x+0.5, y+0.5, chart->bud_col_w, chart->h - y);
	cairo_stroke(cr);

	//result column
	x = chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + (CHART_SPACING*3);
	cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); //blue
	cairo_rectangle(cr, x+0.5, y+0.5, chart->res_col_w, chart->h - y);
	cairo_stroke(cr);

	
	// draw item lines
	y = chart->header_y + chart->header_zh;

	cairo_set_source_rgb(cr, 1.0, .0, .0);
	cairo_rectangle(cr, chart->l+chart->cat_col_w+0.5, y+0.5, chart->graph_width+0.5, chart->graph_height+0.5);
	cairo_stroke(cr);



	y2 = y+0.5;
	cairo_set_line_width(cr, 1.0);
	double dashlength;
	dashlength = 4;
	cairo_set_dash (cr, &dashlength, 1, 0);
	cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); // violet
	for(i=first; i<=(first+chart->visible) ;i++)
	{
		cairo_move_to(cr, chart->l, y2);
		cairo_line_to(cr, chart->r, y2);
		
		y2 += chart->blkw;
	}
	cairo_stroke(cr);

	cairo_destroy(cr);

}
#endif


/*
** draw all visible bars
*/
static void ui_chart_stack_draw_bars(ChartStack *chart, cairo_t *cr)
{
double x, y, x2, y2, h;
gint first;
gint i, idx;
gchar *valstr;
PangoLayout *layout;
int tw, th;

	DB( g_print("\n[chartstack] bar draw bars\n") );

	layout = pango_cairo_create_layout (cr);
	
	x = chart->l + chart->cat_col_w;
	y = chart->t + chart->title_zh + chart->header_zh + chart->subtitle_zh;
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));

	// draw title
	if(chart->title)
	{
		//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_TITLE * PANGO_SCALE);
		ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_TITLE);
		pango_layout_set_font_description (layout, chart->pfd);
		pango_layout_set_text (layout, chart->title, -1);
		pango_layout_get_size (layout, &tw, &th);

		cairo_user_set_rgbcol(cr, &global_colors[THTEXT]);
		cairo_move_to(cr, chart->l, chart->t);
		pango_cairo_show_layout (cr, layout);
	}
	
	// draw period
	if(chart->subtitle)
	{
		//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_PERIOD * PANGO_SCALE);
		ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_SUBTITLE);
		pango_layout_set_font_description (layout, chart->pfd);
		pango_layout_set_text (layout, chart->subtitle, -1);
		pango_layout_get_size (layout, &tw, &th);

		cairo_user_set_rgbcol(cr, &global_colors[THTEXT]);
		cairo_move_to(cr, chart->l, chart->subtitle_y);
		pango_cairo_show_layout (cr, layout);
	}
	
	// draw column title
	//cairo_user_set_rgbcol(cr, &global_colors[BLACK]);
	cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
	//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_NORMAL * PANGO_SCALE);
	ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_NORMAL);
	pango_layout_set_font_description (layout, chart->pfd);

	pango_layout_set_text (layout, chart->budget_title, -1);
	pango_layout_get_size (layout, &tw, &th);
	cairo_move_to(cr, chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + CHART_SPACING - (tw /PANGO_SCALE), chart->header_y);
	pango_cairo_show_layout (cr, layout);

	pango_layout_set_text (layout, chart->result_title, -1);
	pango_layout_get_size (layout, &tw, &th);
	cairo_move_to(cr, chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + chart->res_col_w - (tw /PANGO_SCALE) + (CHART_SPACING*3), chart->header_y);
	pango_cairo_show_layout (cr, layout);


	// draw items
	//pango_font_description_set_size(chart->pfd, CHART_FONT_SIZE_NORMAL * PANGO_SCALE);
	ui_chart_stack_set_font_size(chart, CHART_FONT_SIZE_NORMAL);
	pango_layout_set_font_description (layout, chart->pfd);

	for(i=0; i<chart->visible ;i++)
	{
	StackItem *item;
	gint barw = chart->barw;
	gint blkw = chart->blkw;

		idx = i + first;

		item = &g_array_index(chart->items, StackItem, idx);

		x2 = x;
		y2 = y + (CHART_SPACING/2) + (blkw * i);
		
		DB( g_print("'%-32s' wrn=%d %.2f%% (%.2f%%) :: r=% 4.2f s=% 4.2f b=% 4.2f\n", 
		item->label, item->warn, item->rawrate, item->rate, item->result, item->spent, item->budget) );
		
		valstr = item->label;
		pango_layout_set_text (layout, valstr, -1);
		pango_layout_get_size (layout, &tw, &th);

		double ytext = y2 + ((barw - (th / PANGO_SCALE))/2);

		//cairo_user_set_rgbcol(cr, &global_colors[BLACK]);
		cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
		cairo_move_to(cr, chart->l + chart->cat_col_w - (tw / PANGO_SCALE) - CHART_SPACING, ytext);
		pango_cairo_show_layout (cr, layout);

		// bar background
		cairo_user_set_rgbacol(cr, &global_colors[THTEXT], 0.15);
		cairo_rectangle(cr, x2, y2, chart->graph_width, barw);
		cairo_fill(cr);

		//bar with color :: todo migrate this
		h = floor(item->rate * chart->graph_width);	
		if(item->warn)
		{
			cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[chart->color_scheme.cs_red], idx == chart->active);
		}   
		else
		{
			if(item->rate > 0.8 && item->rate < 1.0)
				cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[chart->color_scheme.cs_orange], idx == chart->active);
			else
				cairo_user_set_rgbcol_over(cr, &chart->color_scheme.colors[chart->color_scheme.cs_green], idx == chart->active);
		}

		
		cairo_rectangle(cr, x2, y2, h, barw);
		cairo_fill(cr);

		// spent value
		if( item->result != 0)
		{
			valstr = ui_chart_stack_print_int(chart, item->spent);
			pango_layout_set_text (layout, valstr, -1);
			pango_layout_get_size (layout, &tw, &th);
	
			if( h  >= ( (tw / PANGO_SCALE) + (CHART_SPACING*2)) )
			{
				// draw inside
				cairo_user_set_rgbcol(cr, &global_colors[WHITE]);
				//cairo_user_set_rgbcol (cr, &global_colors[THBASE]);
				cairo_move_to(cr, x2 + h - (tw / PANGO_SCALE) - CHART_SPACING, ytext);
			}
			else
			{
				// draw outside
				//cairo_user_set_rgbcol(cr, &global_colors[TEXT]);
				cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
				cairo_move_to(cr, x2 + h + CHART_SPACING, ytext);			
			}
		
			pango_cairo_show_layout (cr, layout);
		}

		// budget value
		valstr = ui_chart_stack_print_int(chart, item->budget);
		pango_layout_set_text (layout, valstr, -1);
		pango_layout_get_size (layout, &tw, &th);
		//cairo_user_set_rgbcol(cr, &global_colors[BLACK]);
		cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
		cairo_move_to(cr, chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + CHART_SPACING - (tw / PANGO_SCALE), ytext);
		pango_cairo_show_layout (cr, layout);

		// result value

		if( item->result != 0)
		{
			valstr = ui_chart_stack_print_int(chart, item->result);

			if(item->warn)
				//cairo_set_source_rgb(cr, COLTOCAIRO(164), COLTOCAIRO(0), COLTOCAIRO(0));
				cairo_user_set_rgbcol(cr, &chart->color_scheme.colors[chart->color_scheme.cs_red]);
			else
				//cairo_user_set_rgbcol(cr, &global_colors[TEXT]);
				cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);

			pango_layout_set_text (layout, valstr, -1);
			pango_layout_get_size (layout, &tw, &th);
			cairo_move_to(cr, chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + chart->res_col_w - (tw / PANGO_SCALE) + (CHART_SPACING*3), ytext);
			pango_cairo_show_layout (cr, layout);

			// status
			pango_layout_set_text (layout, item->status, -1);
			pango_layout_get_size (layout, &tw, &th);
			cairo_move_to(cr, chart->l + chart->cat_col_w + chart->graph_width + chart->bud_col_w + chart->res_col_w  + (CHART_SPACING*4), ytext);
			pango_cairo_show_layout (cr, layout);
		}
		
		//y += blkw;

	}

	g_object_unref (layout);
	
}


/*
** get the bar under the mouse pointer
*/
static gint ui_chart_stack_get_active(GtkWidget *widget, gint x, gint y, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
gint retval, first, index, py;
gint blkw = chart->blkw;
double oy;
		
	retval = -1;

	oy = chart->t + chart->title_zh + chart->header_zh + chart->subtitle_zh;

	//DB( g_print(" y=%d, oy=%f, cb=%f\n", y, oy, chart->b) );


	if( (y <= chart->b && y >= oy) && (x >= chart->l && x <= chart->r) )
	{
		first = gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
		py = (y - oy);
		index = first + (py / blkw);


		if(index < chart->nb_items)
			retval = index;

		//DB( g_print(" hover=%d\n", retval) );
	}

	return(retval);
}


static void ui_chart_stack_first_changed( GtkAdjustment *adj, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
//gint first;

	DB( g_print("\n[chartstack] bar first changed\n") );

	//first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	//DB( g_print(" first=%d\n", first) );

/*
	DB( g_print("scrollbar\n adj=%8x, low=%.2f upp=%.2f val=%.2f step=%.2f page=%.2f size=%.2f\n", adj,
		adj->lower, adj->upper, adj->value, adj->step_increment, adj->page_increment, adj->page_size) );
 */
    /* Set the number of decimal places to which adj->value is rounded */
    //gtk_scale_set_digits (GTK_SCALE (hscale), (gint) adj->value);
    //gtk_scale_set_digits (GTK_SCALE (vscale), (gint) adj->value);
	gtk_widget_queue_draw(chart->drawarea);

}

/*
** scrollbar set values for upper, page size, and also show/hide
*/
static void ui_chart_stack_scrollbar_setvalues(ChartStack *chart)
{
GtkAdjustment *adj = chart->adjustment;
gint first;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	DB( g_print("\n[chartstack] sb_set_values\n") );

	//if(visible < entries)
	//{
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

		//gtk_widget_show(GTK_WIDGET(scrollbar));
	//}
	//else
		//gtk_widget_hide(GTK_WIDGET(scrollbar));


}


static void ui_chart_stack_recompute(ChartStack *chart)
{

	DB( g_print("\n[chartstack] recompute\n") );

	ui_chart_stack_calculation(chart);
	gtk_adjustment_set_value(chart->adjustment, 0);
	ui_chart_stack_scrollbar_setvalues(chart);
	gtk_widget_show(chart->scrollbar);

	gtk_widget_queue_draw( chart->drawarea );
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static gboolean drawarea_full_redraw(GtkWidget *widget, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
cairo_t *cr;

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

#if HELPDRAW == 1
	ui_chart_stack_draw_help(widget, user_data);
#endif

	ui_chart_stack_draw_bars(chart, cr);

	cairo_destroy(cr);

	return TRUE;
}


static gboolean
drawarea_configure_event_callback (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
GtkAllocation allocation;
GtkStyleContext *context;
PangoFontDescription *desc;
gboolean colfound;
GdkRGBA color;

	DB( g_print("\n[chartstack] drawarea configure \n") );

	DB( g_print("w=%d h=%d\n", allocation.width, allocation.height) );

	gtk_widget_get_allocation (widget, &allocation);

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
	chart->barw = (6 + chart->pfd_size) * PHI;

	DB( g_print("family: %s\n", pango_font_description_get_family(chart->pfd) ) );
	DB( g_print("size  : %d (%d)\n", chart->pfd_size, chart->pfd_size/PANGO_SCALE ) );
	DB( g_print("isabs : %d\n", pango_font_description_get_size_is_absolute (chart->pfd) ) );

	if( gtk_widget_get_realized(widget) )
	{
		ui_chart_stack_recompute(chart);   
		drawarea_full_redraw(widget, user_data);
	}
	
	/* We've handled the configure event, no need for further processing. */
	return TRUE;
}


static gboolean drawarea_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);

	if( !gtk_widget_get_realized(widget) || chart->surface == NULL )
		return FALSE;

	//DB( g_print("\n[chartstack] drawarea draw cb\n") );

	cairo_set_source_surface (cr, chart->surface, 0, 0);
	cairo_paint (cr);
	
	/* always redraw directly the active block */
	gint first;
	double ox, oy;
	
	first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
	ox = chart->l + chart->cat_col_w;
	oy = chart->t + chart->title_zh + chart->header_zh + chart->subtitle_zh;


	if(chart->active != -1)
	{
		DB( g_print(" draw active\n") );

		oy += CHART_SPACING/2 + (chart->active - first) * chart->blkw;
		//cairo_user_set_rgbacol (cr, &global_colors[THTEXT], 0.78);
		cairo_user_set_rgbacol(cr, &global_colors[WHITE], OVER_ALPHA);
		//cairo_move_to(cr, chart->l, chart->t);
		cairo_rectangle(cr, ox, oy, chart->graph_width, chart->barw);
		cairo_fill(cr);
	}
	
	return FALSE;
}


static gboolean drawarea_motionnotifyevent_callback(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
gint x, y;

	if(chart->nb_items == 0)
		return FALSE;

	DB( g_print("\n[chartstack] drawarea motion cb\n") );
	x = event->x;
	y = event->y;

	//todo see this
	if(event->is_hint)
	{
		//DB( g_print(" is hint\n") );

		gdk_window_get_device_position(event->window, event->device, &x, &y, NULL);
		//gdk_window_get_pointer(event->window, &x, &y, NULL);
		//return FALSE;
	}

	chart->active = ui_chart_stack_get_active(widget, x, y, chart);

	
	// rollover redraw ?
	DB( g_print(" %d, %d :: active: last=%d, curr=%d\n", x, y, chart->lastactive, chart->active) );
	
	if(chart->lastactive != chart->active)
	{
	GdkRectangle update_rect;
	gint first;
	double oy;
	
		DB( g_print(" motion rollover redraw :: active=%d\n", chart->active) );

		first = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(chart->adjustment));
		oy = chart->t + chart->title_zh + chart->header_zh + chart->subtitle_zh;

		if(chart->lastactive != -1)
		{
			update_rect.x = chart->l;
			update_rect.y = oy + (chart->lastactive - first) * chart->blkw;
			update_rect.width = chart->r;
			update_rect.height = chart->blkw;
		
			/* Now invalidate the affected region of the drawing area. */
			gdk_window_invalidate_rect (gtk_widget_get_window (widget),
		                          &update_rect,
		                          FALSE);
		}
		
		update_rect.x = chart->l;
		update_rect.y = oy + (chart->active - first) * chart->blkw;
		update_rect.width = chart->r;
		update_rect.height = chart->blkw;
	
		/* Now invalidate the affected region of the drawing area. */
		gdk_window_invalidate_rect (gtk_widget_get_window (widget),
	                          &update_rect,
	                          FALSE);
		
		//gtk_widget_queue_draw( widget );		
		//retval = FALSE;
	}
	
	chart->lastactive = chart->active;

	return TRUE;
}



static gboolean drawarea_scroll_event_callback( GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
ChartStack *chart = GTK_CHARTSTACK(user_data);
GtkAdjustment *adj = chart->adjustment;
gdouble first, upper, pagesize;
	
	DB( g_print("\n[chartstack] scroll\n") );

	first = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));
	//lower = gtk_adjustment_get_lower(GTK_ADJUSTMENT(adj));
	upper = gtk_adjustment_get_upper(GTK_ADJUSTMENT(adj));
	pagesize = gtk_adjustment_get_page_size(GTK_ADJUSTMENT(adj));

	DB( g_print("- pos is %.2f, [%.2f - %.2f]\n", first, 0.0, upper) );

	switch(event->direction)
	{
		case GDK_SCROLL_UP:
			gtk_adjustment_set_value(adj, first - 1);
			break;
		case GDK_SCROLL_DOWN:
			gtk_adjustment_set_value(adj, CLAMP(first + 1, 0, upper - pagesize) );
			break;
		default:
			break;
	}

	drawarea_full_redraw(widget, user_data);

	return TRUE;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* public functions */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void ui_chart_stack_queue_redraw(ChartStack *chart)
{

	if( gtk_widget_get_realized(GTK_WIDGET(chart)) )
	{
		ui_chart_stack_recompute(chart);
		drawarea_full_redraw(chart->drawarea, chart);
		//gtk_widget_queue_draw( chart->drawarea );
	}
}

/*
** change the model and/or column
*/
void ui_chart_stack_set_dualdatas(ChartStack *chart, GtkTreeModel *model, gchar *coltitle1, gchar *coltitle2, gchar *title, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	if( GTK_IS_TREE_MODEL(model) )
	{
		ui_chart_stack_setup_with_model(chart, model, coltitle1, coltitle2 );
		if(title != NULL)
			chart->title = g_strdup(title);
		if(subtitle != NULL)
			chart->subtitle = g_strdup(subtitle);

		ui_chart_stack_queue_redraw(chart);
	}
	else
	{
		ui_chart_stack_clear(chart, TRUE);
	}
}

/*
** change the tooltip title
*/
void ui_chart_stack_set_title(ChartStack * chart, gchar *title)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	chart->title = g_strdup(title);

	DB( g_print("\n[chartstack] set title = %s\n", chart->title) );

	ui_chart_stack_recompute(chart);

}

void ui_chart_stack_set_subtitle(ChartStack * chart, gchar *subtitle)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	chart->subtitle = g_strdup(subtitle);

	DB( g_print("\n[chartstack] set period = %s\n", chart->subtitle) );

	ui_chart_stack_recompute(chart);

}


/*
** change the minor visibility
*/
void ui_chart_stack_show_minor(ChartStack * chart, gboolean minor)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	chart->minor = minor;

	ui_chart_stack_queue_redraw(chart);

}

void ui_chart_stack_set_color_scheme(ChartStack * chart, gint index)
{
	colorscheme_init(&chart->color_scheme, index);
}


/*
** set the minor parameters
*/
/*void ui_chart_stack_set_minor_prefs(ChartStack * chart, gdouble rate, gchar *symbol)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	chart->minor_rate   = rate;
	chart->minor_symbol = symbol;
}*/

void ui_chart_stack_set_currency(ChartStack * chart, guint32 kcur)
{
	g_return_if_fail (GTK_IS_CHARTSTACK (chart));

	chart->kcur = kcur;
}

