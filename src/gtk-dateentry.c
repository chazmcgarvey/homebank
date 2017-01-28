/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2017 Maxime DOYEN
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

#include <stdlib.h>	
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gtk-dateentry.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

enum {
  CHANGED,
  LAST_SIGNAL
};

enum {
	PROPERTY_DATE = 5,
};

static void gtk_date_entry_dispose (GObject *gobject);
static void gtk_date_entry_finalize (GObject *gobject);
static void gtk_date_entry_destroy         (GtkWidget     *dateentry);

static void gtk_date_entry_entry_activate(GtkWidget * calendar, gpointer user_data);
static gint gtk_date_entry_entry_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void	gtk_date_entry_button_clicked     (GtkWidget * widget, GtkDateEntry * dateentry);

static void
gtk_date_entry_popup(GtkDateEntry * dateentry, GdkEvent *event);

static gint gtk_date_entry_popup_key_event (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gint gtk_date_entry_popup_button_press (GtkWidget * widget, GdkEvent * event, gpointer data);

static void gtk_date_entry_calendar_year(GtkWidget * calendar, GtkDateEntry * dateentry);
static void gtk_date_entry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry);
static gint gtk_date_entry_calendar_select(GtkWidget * calendar, gpointer user_data);


static void gtk_date_entry_entry_set_text(GtkDateEntry * dateentry);
static void gtk_date_entry_popdown(GtkDateEntry *dateentry);



static guint dateentry_signals[LAST_SIGNAL] = {0,};


// todo:finish this
// this is to be able to seizure d or d/m or m/d in the gtkdateentry

/* order of these in the current locale */
static GDateDMY dmy_order[3] = 
{
   G_DATE_DAY, G_DATE_MONTH, G_DATE_YEAR
};

struct _GDateParseTokens {
  gint num_ints;
  gint n[3];
  guint month;
};

typedef struct _GDateParseTokens GDateParseTokens;

#define NUM_LEN 10

static void
hb_date_fill_parse_tokens (const gchar *str, GDateParseTokens *pt)
{
  gchar num[4][NUM_LEN+1];
  gint i;
  const guchar *s;
  
  DB( g_print("\n[dateentry] fill parse token\n") );
  
  /* We count 4, but store 3; so we can give an error
   * if there are 4.
   */
  num[0][0] = num[1][0] = num[2][0] = num[3][0] = '\0';
  
  s = (const guchar *) str;
  pt->num_ints = 0;
  while (*s && pt->num_ints < 4) 
    {
      
      i = 0;
      while (*s && g_ascii_isdigit (*s) && i < NUM_LEN)
        {
          num[pt->num_ints][i] = *s;
          ++s; 
          ++i;
        }
      
      if (i > 0) 
        {
          num[pt->num_ints][i] = '\0';
          ++(pt->num_ints);
        }
      
      if (*s == '\0') break;
      
      ++s;
    }
  
  pt->n[0] = pt->num_ints > 0 ? atoi (num[0]) : 0;
  pt->n[1] = pt->num_ints > 1 ? atoi (num[1]) : 0;
  pt->n[2] = pt->num_ints > 2 ? atoi (num[2]) : 0;

}


/*
static void hb_date_determine_dmy(void)
{
GDate d;
gchar buf[128];
GDateParseTokens testpt;
gint i;

	DB( g_print("\n[dateentry] determine dmy\n") );

	g_date_clear (&d, 1);              // clear for scratch use
    
	// had to pick a random day - don't change this, some strftimes
	// are broken on some days, and this one is good so far.
	g_date_set_dmy (&d, 4, 7, 1976);
	g_date_strftime (buf, 127, "%x", &d);

	hb_date_fill_parse_tokens (buf, &testpt);

	i = 0;
	while (i < testpt.num_ints)
	{
		switch (testpt.n[i])
		{
		case 7:
			dmy_order[i] = G_DATE_MONTH;
			break;
		case 4:
			dmy_order[i] = G_DATE_DAY;
			break;
		case 1976:
			dmy_order[2] = G_DATE_YEAR;
			break;
		}
		++i;
	}

	DB( g_print(" dmy legend: 0=day, 1=month, 2=year\n") );   
	DB( g_print(" dmy is: %d %d %d\n", dmy_order[0], dmy_order[1], dmy_order[2]) );   
}*/    


static void hb_date_parse_tokens(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GtkDateEntryPrivate *priv = dateentry->priv;
const gchar *str;
GDateParseTokens pt;

 	str = gtk_entry_get_text (GTK_ENTRY (priv->entry));

	hb_date_fill_parse_tokens(str, &pt);
	DB( g_print(" -> parsetoken return is %d values :%d %d %d\n", pt.num_ints, pt.n[0], pt.n[1], pt.n[2]) );

	// initialize with today's date
	g_date_set_time_t(priv->date, time(NULL));
	
	switch( pt.num_ints )
	{
		case 1:
			DB( g_print(" -> seizured 1 number\n") );
			if(g_date_valid_day(pt.n[0]))
				g_date_set_day(priv->date, pt.n[0]);
			break;
		case 2:
			DB( g_print(" -> seizured 2 numbers\n") );
			if( dmy_order[0] != G_DATE_YEAR )
			{
				if( dmy_order[0] == G_DATE_DAY )
				{
					if(g_date_valid_day(pt.n[0]))
					    g_date_set_day(priv->date, pt.n[0]);
					if(g_date_valid_month(pt.n[1]))
						g_date_set_month(priv->date, pt.n[1]);
				}
				else
				{
					if(g_date_valid_day(pt.n[1]))
					    g_date_set_day(priv->date, pt.n[1]);
					if(g_date_valid_month(pt.n[0]))
						g_date_set_month(priv->date, pt.n[0]);
				}
			}
			break;
	}


	
}





//end 


G_DEFINE_TYPE(GtkDateEntry, gtk_date_entry, GTK_TYPE_BOX)



static void
gtk_date_entry_class_init (GtkDateEntryClass *class)
{
GObjectClass *object_class;
GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	DB( g_print("\n[dateentry] class_init\n") );

	//object_class->constructor = gtk_date_entry_constructor;
	//object_class->set_property = gtk_date_entry_set_property;
	//object_class->get_property = gtk_date_entry_get_property;
	object_class->dispose  = gtk_date_entry_dispose;
	object_class->finalize = gtk_date_entry_finalize;

	widget_class->destroy  = gtk_date_entry_destroy;
	
	dateentry_signals[CHANGED] =
		g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkDateEntryClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

	g_type_class_add_private (object_class, sizeof (GtkDateEntryPrivate));
	
}


static gboolean 
gtk_date_entry_entry_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] entry focus-out-event %d\n", gtk_widget_is_focus(GTK_WIDGET(dateentry))) );

	gtk_date_entry_entry_activate(GTK_WIDGET(dateentry), dateentry);

	return FALSE;
}

static void
gtk_date_entry_init (GtkDateEntry *dateentry)
{
GtkDateEntryPrivate *priv;

	DB( g_print("\n[dateentry] init\n") );

	/* yes, also priv, need to keep the code readable */
	dateentry->priv = G_TYPE_INSTANCE_GET_PRIVATE (dateentry,
                                                  GTK_TYPE_DATE_ENTRY,
                                                  GtkDateEntryPrivate);
	priv = dateentry->priv;

	
	/* initialize datas */
	priv->date = g_date_new();
	priv->device = NULL;
	priv->popup_in_progress = FALSE;
	priv->has_grab = FALSE;

	g_date_set_time_t(priv->date, time(NULL));

	g_date_set_dmy(&priv->mindate, 1, 1, 1900);
	g_date_set_dmy(&priv->maxdate, 31, 12, 2200);


	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(dateentry)), GTK_STYLE_CLASS_LINKED);

	//widget=GTK_WIDGET(dateentry);
	
	priv->entry = gtk_entry_new ();
	
	//gtk_entry_set_width_chars(GTK_ENTRY(priv->entry), 10);
	//gtk_entry_set_max_width_chars(GTK_ENTRY(priv->entry), 4);
	
	
	gtk_box_pack_start (GTK_BOX (dateentry), priv->entry, TRUE, TRUE, 0);

	g_signal_connect (priv->entry, "key-press-event",
				G_CALLBACK (gtk_date_entry_entry_key_pressed), dateentry);

	g_signal_connect_after (priv->entry, "focus-out-event",
				G_CALLBACK (gtk_date_entry_entry_focus_out), dateentry);

	g_signal_connect (priv->entry, "activate",
				G_CALLBACK (gtk_date_entry_entry_activate), dateentry);

	
	priv->button = gtk_button_new ();
	priv->arrow = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_container_add (GTK_CONTAINER (priv->button), priv->arrow);
	gtk_box_pack_end (GTK_BOX (dateentry), priv->button, FALSE, FALSE, 0);
	gtk_widget_show_all (priv->button);

	g_signal_connect (priv->button, "clicked",
				G_CALLBACK (gtk_date_entry_button_clicked), dateentry);


	priv->popup_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_type_hint (
		GTK_WINDOW (priv->popup_window), GDK_WINDOW_TYPE_HINT_COMBO);
	gtk_widget_set_events (priv->popup_window,
		gtk_widget_get_events(priv->popup_window) | GDK_KEY_PRESS_MASK);

	priv->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (priv->popup_window), priv->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (priv->frame), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show (priv->frame);

	g_signal_connect (priv->popup_window, "key-press-event",
				G_CALLBACK (gtk_date_entry_popup_key_event), dateentry);

	g_signal_connect (priv->popup_window, "button-press-event",
				G_CALLBACK (gtk_date_entry_popup_button_press), dateentry);

	
	priv->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (priv->frame), priv->calendar);
	gtk_widget_show (priv->calendar);

	g_signal_connect (priv->calendar, "prev-year",
				G_CALLBACK (gtk_date_entry_calendar_year), dateentry);
	g_signal_connect (priv->calendar, "next-year",
				G_CALLBACK (gtk_date_entry_calendar_year), dateentry);
	g_signal_connect (priv->calendar, "prev-month",
				G_CALLBACK (gtk_date_entry_calendar_year), dateentry);
	g_signal_connect (priv->calendar, "next-month",
				G_CALLBACK (gtk_date_entry_calendar_year), dateentry);

	g_signal_connect (priv->calendar, "day-selected",
				G_CALLBACK (gtk_date_entry_calendar_getfrom), dateentry);

	g_signal_connect (priv->calendar, "day-selected-double-click",
				G_CALLBACK (gtk_date_entry_calendar_select), dateentry);

}


GtkWidget *
gtk_date_entry_new ()
{
GtkDateEntry *dateentry;

	DB( g_print("\n[dateentry] new\n") );

	dateentry = g_object_new (GTK_TYPE_DATE_ENTRY, NULL);

	return GTK_WIDGET(dateentry);
}


static void 
gtk_date_entry_destroy (GtkWidget *object)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);
GtkDateEntryPrivate *priv = dateentry->priv;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_DATE_ENTRY(object));

	DB( g_print(" \n[dateentry] destroy\n") );

	DB( g_print(" free gtkentry: %p\n", priv->entry) );
	DB( g_print(" free arrow: %p\n", priv->button) );
	DB( g_print(" free popup_window: %p\n", priv->popup_window) );

	DB( g_print(" free dateentry: %p\n", dateentry) );

	if(priv->popup_window)
		gtk_widget_destroy (priv->popup_window);
	priv->popup_window = NULL;

	if(priv->date)
		g_date_free(priv->date);
	priv->date = NULL;

	GTK_WIDGET_CLASS (gtk_date_entry_parent_class)->destroy (object);
}



static void
gtk_date_entry_dispose (GObject *gobject)
{
//GtkDateEntry *self = GTK_DATE_ENTRY (gobject);

	DB( g_print(" \n[dateentry] dispose\n") );

	
  //g_clear_object (&self->priv->an_object);

  G_OBJECT_CLASS (gtk_date_entry_parent_class)->dispose (gobject);
}




static void
gtk_date_entry_finalize (GObject *gobject)
{
//GtkDateEntry *self = GTK_DATE_ENTRY (gobject);

	DB( g_print(" \n[dateentry] finalize\n") );

	
	//g_date_free(self->date);
  //g_free (self->priv->a_string);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS(gtk_date_entry_parent_class)->finalize (gobject);
}


/*
**
*/
void gtk_date_entry_set_date(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print(" \n[dateentry] set date\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (priv->date, julian_days);
	}
	else
	{
		g_date_set_time_t(priv->date, time(NULL));
	}
	gtk_date_entry_entry_set_text(dateentry);
}

/*
**
*/
void gtk_date_entry_set_mindate(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print(" \n[dateentry] set mindate\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&priv->mindate, julian_days);
	}
}


/*
**
*/
void gtk_date_entry_set_maxdate(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print(" \n[dateentry] set maxdate\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&priv->maxdate, julian_days);
	}
}


guint32 gtk_date_entry_get_date(GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print(" \n[dateentry] get date\n") );

	g_return_val_if_fail (GTK_IS_DATE_ENTRY (dateentry), 0);

	return(g_date_get_julian(priv->date));
}


static void 
gtk_date_entry_entry_set_text(GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
gchar buffer[256];

	DB( g_print("\n[dateentry] entry set text\n") );

	g_date_clamp(priv->date, &priv->mindate, &priv->maxdate);

	
	if(g_date_valid(priv->date) == TRUE)
	{
		g_date_strftime (buffer, 256 - 1, "%x", priv->date);
		gtk_entry_set_text (GTK_ENTRY (priv->entry), buffer);
		
		DB( g_print(" = %s\n", buffer) );
	}
	else
		gtk_entry_set_text (GTK_ENTRY (priv->entry), "??");


	/* emit the signal */
	if(priv->lastdate != g_date_get_julian(priv->date))
	{
		DB( g_print(" **emit 'changed' signal**\n") );

		g_signal_emit_by_name (dateentry, "changed", NULL, NULL);
	}

	priv->lastdate = g_date_get_julian(priv->date);

}


static void
gtk_date_entry_entry_activate(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GtkDateEntryPrivate *priv = dateentry->priv;
const gchar *str;

	DB( g_print("\n[dateentry] entry_activate\n") );

 	str = gtk_entry_get_text (GTK_ENTRY (priv->entry));

	//1) we parse the string according to the locale
	g_date_set_parse (priv->date, str);
	if(g_date_valid(priv->date) == FALSE)
	{
		//2) give a try to tokens: day, day/month, month/day
		hb_date_parse_tokens(gtkentry, user_data);
	}

	//3) at last if date still invalid, put today's dateentry_signals
	// we should consider just warn the user here
	if(g_date_valid(priv->date) == FALSE)
	{
		/* today's date */
		g_date_set_time_t(priv->date, time(NULL));
	}

	gtk_date_entry_entry_set_text(dateentry);

}


static void 
gtk_date_entry_calendar_year(GtkWidget *calendar, GtkDateEntry *dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
guint year, month, day;

	DB( g_print(" (dateentry) year changed\n") );

	gtk_calendar_get_date (GTK_CALENDAR (priv->calendar), &year, &month, &day);
	if( year < 1900)
		g_object_set(calendar, "year", 1900, NULL);

	if( year > 2200)
		g_object_set(calendar, "year", 2200, NULL);
	
}


static void
gtk_date_entry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
guint year, month, day;

	DB( g_print(" (dateentry) calendar_getfrom\n") );

	gtk_calendar_get_date (GTK_CALENDAR (priv->calendar), &year, &month, &day);
	g_date_set_dmy (priv->date, day, month + 1, year);
	gtk_date_entry_entry_set_text(dateentry);
}


static gint
gtk_date_entry_calendar_select(GtkWidget * calendar, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print(" (dateentry) calendar_select\n") );

	gtk_date_entry_calendar_getfrom(NULL, dateentry);
	
	gtk_date_entry_popdown(dateentry);
	return FALSE;
}


static gint
gtk_date_entry_entry_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print("\n[dateentry] entry key pressed: state=%04x, keyval=%04x\n", event->state, event->keyval) );

	if( event->keyval == GDK_KEY_Up )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_add_days (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_add_months (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_add_years (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		return TRUE;
	}
	else
	if( event->keyval == GDK_KEY_Down )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_subtract_days (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_subtract_months (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_subtract_years (priv->date, 1);
			gtk_date_entry_entry_set_text(dateentry);
		}
		return TRUE;
	}

	return FALSE;
}


static void
gtk_date_entry_popup_position (GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
gint x, y;
gint bwidth, bheight;
GtkRequisition req;
GdkWindow *gdkwindow;
GtkAllocation allocation;

	DB( g_print("\n[dateentry] position popup\n") );

	gtk_widget_get_preferred_size (priv->popup_window, NULL, &req);

	gdkwindow = gtk_widget_get_window(priv->button);
	gdk_window_get_origin (gdkwindow, &x, &y);

	gtk_widget_get_allocation(priv->button, &allocation);
	x += allocation.x;
	y += allocation.y;
	bwidth = allocation.width;
	bheight = allocation.height;

	x += bwidth - req.width;
	y += bheight;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	gtk_window_move (GTK_WINDOW (priv->popup_window), x, y);
}


static void
gtk_date_entry_button_clicked (GtkWidget * widget, GtkDateEntry * dateentry)
{
GdkEvent *event;

	DB( g_print("\n[dateentry] button_clicked\n") );

/* Obtain the GdkEvent that triggered
	 * the date button's "clicked" signal. */
	event = gtk_get_current_event ();
	
	gtk_date_entry_popup(dateentry, event);

}


static void
gtk_date_entry_popup(GtkDateEntry * dateentry, GdkEvent *event)
{
GtkDateEntryPrivate *priv = dateentry->priv;
const char *str;
int month;
GdkDevice *event_device;
GdkDevice *assoc_device;
GdkDevice *keyboard_device;
GdkDevice *pointer_device;
GdkWindow *window;
GdkGrabStatus grab_status;
guint event_time;


	DB( g_print("\n[dateentry] popup_display\n****\n\n") );

/* update */
	str = gtk_entry_get_text (GTK_ENTRY (priv->entry));
	g_date_set_parse (priv->date, str);

	if(g_date_valid(priv->date) == TRUE)
	{
		/* GtkCalendar expects month to be in 0-11 range (inclusive) */
		month = g_date_get_month (priv->date) - 1;
		gtk_calendar_select_month (GTK_CALENDAR (priv->calendar),
				   CLAMP (month, 0, 11),
				   g_date_get_year (priv->date));
        gtk_calendar_select_day (GTK_CALENDAR (priv->calendar),
				 g_date_get_day (priv->date));
	}


	/* popup */
	gtk_date_entry_popup_position(dateentry);
	gtk_widget_show (priv->popup_window);
	gtk_widget_grab_focus (priv->popup_window);
	gtk_grab_add (priv->popup_window);

	window = gtk_widget_get_window (priv->popup_window);

	g_return_if_fail (priv->grab_keyboard == NULL);
	g_return_if_fail (priv->grab_pointer == NULL);

	event_device = gdk_event_get_device (event);
	assoc_device = gdk_device_get_associated_device (event_device);

	event_time = gdk_event_get_time (event);

	if (gdk_device_get_source (event_device) == GDK_SOURCE_KEYBOARD) {
		keyboard_device = event_device;
		pointer_device = assoc_device;
	} else {
		keyboard_device = assoc_device;
		pointer_device = event_device;
	}

	if (keyboard_device != NULL) {
		grab_status = gdk_device_grab (
			keyboard_device,
			window,
			GDK_OWNERSHIP_WINDOW,
			TRUE,
			GDK_KEY_PRESS_MASK |
			GDK_KEY_RELEASE_MASK,
			NULL,
			event_time);
		if (grab_status == GDK_GRAB_SUCCESS) {
			priv->grab_keyboard =
				g_object_ref (keyboard_device);
		}
	}

	if (pointer_device != NULL) {
		grab_status = gdk_device_grab (
			pointer_device,
			window,
			GDK_OWNERSHIP_WINDOW,
			TRUE,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK,
			NULL,
			event_time);
		if (grab_status == GDK_GRAB_SUCCESS) {
			priv->grab_pointer =
				g_object_ref (pointer_device);
		} else if (priv->grab_keyboard != NULL) {
			gdk_device_ungrab (
				priv->grab_keyboard,
				event_time);
			g_object_unref (priv->grab_keyboard);
			priv->grab_keyboard = NULL;
		}
	}

	gdk_window_focus (window, event_time);

}


static void
gtk_date_entry_popdown(GtkDateEntry *dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print("\n[dateentry] popdown\n") );

  gtk_widget_hide (priv->popup_window);
  gtk_grab_remove (priv->popup_window);

	if (priv->grab_keyboard != NULL) {
		gdk_device_ungrab (
			priv->grab_keyboard,
			GDK_CURRENT_TIME);
		g_object_unref (priv->grab_keyboard);
		priv->grab_keyboard = NULL;
	}

	if (priv->grab_pointer != NULL) {
		gdk_device_ungrab (
			priv->grab_pointer,
			GDK_CURRENT_TIME);
		g_object_unref (priv->grab_pointer);
		priv->grab_pointer = NULL;
	}

}





static gint
gtk_date_entry_popup_key_event (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] popup_key_event%d\n", event->keyval) );

	DB( g_print(" -> key=%d\n", event->keyval) );

	if (event->keyval != GDK_KEY_Escape && event->keyval != GDK_KEY_Return)
		return FALSE;

	g_signal_stop_emission_by_name (widget, "key-press-event");

	gtk_date_entry_popdown(dateentry);

	return TRUE;
}


static gint
gtk_date_entry_popup_button_press (GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
//GtkDateEntryPrivate *priv = dateentry->priv;
GtkWidget *child;

	DB( g_print("\n[dateentry] popup_button_press\n") );

	child = gtk_get_event_widget (event);

	/* We don't ask for button press events on the grab widget, so
	 *  if an event is reported directly to the grab widget, it must
	 *  be on a window outside the application (and thus we remove
	 *  the popup window). Otherwise, we check if the widget is a child
	 *  of the grab widget, and only remove the popup window if it
	 *  is not.
	 */
	if (child != widget) {
		while (child) {
			if (child == widget)
				return FALSE;
			child = gtk_widget_get_parent (child);
		}
	}

	gtk_date_entry_popdown(dateentry);

	return TRUE;
}

