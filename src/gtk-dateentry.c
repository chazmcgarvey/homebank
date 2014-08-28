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

static void         gtk_dateentry_class_init      (GtkDateEntryClass *klass);
static void         gtk_dateentry_init            (GtkDateEntry      *dateentry);
static void         gtk_dateentry_destroy         (GtkObject     *dateentry);
static void         gtk_dateentry_popup_display   (GtkDateEntry *dateentry);
static gint	    gtk_dateentry_arrow_press     (GtkWidget * widget,
					  	  GtkDateEntry * dateentry);
static gint         gtk_dateentry_button_press    (GtkWidget     *widget,
				                  GdkEvent      *event,
                                                  gpointer data);

static void gtk_dateentry_entry_parse(GtkWidget * calendar, gpointer user_data);
static gint gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry);
static gint gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data);
static void gtk_dateentry_calendar_year(GtkWidget * calendar, GtkDateEntry * dateentry);
static void gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry);
static gint gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry);
static gint key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gint gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer data);

static void gtk_dateentry_entry_set_text(GtkDateEntry * dateentry);

/*
static void
gtk_dateentry_set_property (GObject         *object,
                        guint            prop_id,
                        const GValue    *value,
                        GParamSpec      *pspec);

static void
gtk_dateentry_get_property (GObject         *object,
                        guint            prop_id,
                        GValue          *value,
                        GParamSpec      *pspec);
*/

static GtkHBoxClass *parent_class = NULL;
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
g_date_fill_parse_tokens (const gchar *str, GDateParseTokens *pt)
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

static void g_date_determine_dmy(void)
{
GDate d;
gchar buf[128];
GDateParseTokens testpt;
gint i;

  DB( g_print("\n[dateentry] determine dmy\n") );


  g_date_clear (&d, 1);              /* clear for scratch use */
  
  
      /* had to pick a random day - don't change this, some strftimes
       * are broken on some days, and this one is good so far. */
      g_date_set_dmy (&d, 4, 7, 1976);
      
      g_date_strftime (buf, 127, "%x", &d);
      
      g_date_fill_parse_tokens (buf, &testpt);
      
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
            //case 76:
              //using_twodigit_years = TRUE; /* FALL THRU */
            case 1976:
              dmy_order[2] = G_DATE_YEAR;
              break;
            }
          ++i;
        }

       DB( g_print(" dmy legend: 0=day, 1=month, 2=year\n") );   
       DB( g_print(" dmy is: %d %d %d\n", dmy_order[0], dmy_order[1], dmy_order[2]) );   
}        
        

//end 


GType
gtk_dateentry_get_type ()
{
static GType dateentry_type = 0;

	//DB( g_print("\n[dateentry] get_type\n") );

	if (!dateentry_type)
    {
		static const GTypeInfo dateentry_info =
		{
		sizeof (GtkDateEntryClass),
		NULL,		/* base_init */
		NULL,		/* base_finalize */
		(GClassInitFunc) gtk_dateentry_class_init,
		NULL,		/* class_finalize */
		NULL,		/* class_data */
		sizeof (GtkDateEntry),
		0,		/* n_preallocs */
		(GInstanceInitFunc) gtk_dateentry_init,
		NULL
		};

		//dateentry_type = gtk_type_unique (gtk_hbox_get_type (), &dateentry_info);

		dateentry_type = g_type_register_static (GTK_TYPE_HBOX, "GtkDateEntry",
							 &dateentry_info, 0);


	}
	return dateentry_type;
}

static void
gtk_dateentry_class_init (GtkDateEntryClass * klass)
{
  //GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  //GtkWidgetClass *widget_class;

  //gobject_class = (GObjectClass*) klass;
  object_class = (GtkObjectClass*) klass;
  //widget_class = (GtkWidgetClass*) klass;

  parent_class = g_type_class_peek_parent (klass);

	DB( g_print("\n[dateentry] class_init\n") );


	object_class->destroy = gtk_dateentry_destroy;

  dateentry_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkDateEntryClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

	g_date_determine_dmy();

 /*
	gobject_class->set_property = gtk_dateentry_set_property;
	gobject_class->get_property = gtk_dateentry_get_property;

	g_object_class_install_property (gobject_class,
			PROPERTY_DATE,
			g_param_spec_uint(	"date",
								"Date",
							    "The date currently selected",
							    0, G_MAXUINT,
							    0,
							    (G_PARAM_READABLE | G_PARAM_WRITABLE)
							   )
			);
	*/
}

static gboolean gtk_dateentry_focus(GtkWidget     *widget,
                                                        GdkEventFocus *event,
                                                        gpointer       user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] focus-out-event %d\n", gtk_widget_is_focus(GTK_WIDGET(dateentry))) );

	gtk_dateentry_entry_parse(GTK_WIDGET(dateentry), dateentry);

	return FALSE;
}

static void
gtk_dateentry_init (GtkDateEntry *dateentry)
{
GtkWidget *widget;
GtkWidget *arrow;

	DB( g_print("\n[dateentry] init\n") );

	/* initialize datas */
	dateentry->date = g_date_new();

	g_date_set_time_t(dateentry->date, time(NULL));

	g_date_set_dmy(&dateentry->mindate, 1, 1, 1900);
	g_date_set_dmy(&dateentry->maxdate, 31, 12, 2200);


	widget=GTK_WIDGET(dateentry);
	gtk_box_set_homogeneous(GTK_BOX(widget), FALSE);
	
	dateentry->entry = gtk_entry_new ();
	gtk_widget_set_size_request(dateentry->entry, 90, -1);
	gtk_box_pack_start (GTK_BOX (dateentry), dateentry->entry, TRUE, TRUE, 0);

	dateentry->arrow = gtk_toggle_button_new ();
	arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (dateentry->arrow), arrow);
	gtk_box_pack_end (GTK_BOX (dateentry), dateentry->arrow, FALSE, FALSE, 0);

	gtk_widget_show (dateentry->entry);
	gtk_widget_show (dateentry->arrow);


    /* our popup window */
	dateentry->popwin = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (dateentry->popwin,
				gtk_widget_get_events(dateentry->popwin) | GDK_KEY_PRESS_MASK);

	dateentry->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (dateentry->popwin), dateentry->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (dateentry->frame), GTK_SHADOW_OUT);
	gtk_widget_show (dateentry->frame);

	dateentry->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (dateentry->frame), dateentry->calendar);
	gtk_widget_show (dateentry->calendar);

	// dateentry signals
	g_signal_connect (GTK_OBJECT (dateentry->entry), "activate",
				G_CALLBACK (gtk_dateentry_entry_parse), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->entry), "focus-out-event",
				G_CALLBACK (gtk_dateentry_focus), dateentry);


	g_signal_connect (GTK_OBJECT (dateentry->entry), "key_press_event",
				G_CALLBACK (gtk_dateentry_entry_key), dateentry);

	// arrow/popwin signals
	g_signal_connect (GTK_OBJECT (dateentry->arrow), "toggled",
				G_CALLBACK (gtk_dateentry_arrow_press), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->popwin), "key_press_event",
				G_CALLBACK (key_press_popup), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->popwin), "button_press_event",
				G_CALLBACK (gtk_dateentry_button_press), dateentry);

	// calendar signals
	g_signal_connect (GTK_OBJECT (dateentry->calendar), "prev-year",
				G_CALLBACK (gtk_dateentry_calendar_year), dateentry);
	g_signal_connect (GTK_OBJECT (dateentry->calendar), "next-year",
				G_CALLBACK (gtk_dateentry_calendar_year), dateentry);
	g_signal_connect (GTK_OBJECT (dateentry->calendar), "prev-month",
				G_CALLBACK (gtk_dateentry_calendar_year), dateentry);
	g_signal_connect (GTK_OBJECT (dateentry->calendar), "next-month",
				G_CALLBACK (gtk_dateentry_calendar_year), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->calendar), "day-selected",
				G_CALLBACK (gtk_dateentry_calendar_getfrom), dateentry);

	g_signal_connect (GTK_OBJECT (dateentry->calendar), "day-selected-double-click",
				G_CALLBACK (gtk_dateentry_calendar_select), dateentry);

	//gtk_dateentry_calendar_getfrom(NULL, dateentry);
}


GtkWidget *gtk_dateentry_new ()
{
GtkDateEntry *dateentry;

	DB( g_print("\n[dateentry] new\n") );

	dateentry = g_object_new (GTK_TYPE_DATE_ENTRY, NULL);

	return GTK_WIDGET(dateentry);
}


static void gtk_dateentry_destroy (GtkObject * object)
{
GtkDateEntry *dateentry;

	DB( g_print(" \n[dateentry] destroy\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (object));

	dateentry = GTK_DATE_ENTRY (object);

	DB( g_print(" free gtkentry: %p\n", dateentry->entry) );
	DB( g_print(" free arrow: %p\n", dateentry->arrow) );
	DB( g_print(" free popwin: %p\n", dateentry->popwin) );

	DB( g_print(" free dateentry: %p\n", dateentry) );

	if(dateentry->popwin)
		gtk_widget_destroy (dateentry->popwin);
	dateentry->popwin = NULL;

	if(dateentry->date)
		g_date_free(dateentry->date);
	dateentry->date = NULL;

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/*
**
*/
void gtk_dateentry_set_date(GtkDateEntry *dateentry, guint32 julian_days)
{
	DB( g_print(" \n[dateentry] set date\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (dateentry->date, julian_days);
	}
	else
	{
		g_date_set_time_t(dateentry->date, time(NULL));
	}
	gtk_dateentry_entry_set_text(dateentry);
}

/*
**
*/
void gtk_dateentry_set_mindate(GtkDateEntry *dateentry, guint32 julian_days)
{
	DB( g_print(" \n[dateentry] set date\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&dateentry->mindate, julian_days);
	}
}


/*
**
*/
void gtk_dateentry_set_maxdate(GtkDateEntry *dateentry, guint32 julian_days)
{
	DB( g_print(" \n[dateentry] set date\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&dateentry->maxdate, julian_days);
	}
}


/*
**
*/
guint32 gtk_dateentry_get_date(GtkDateEntry * dateentry)
{
	DB( g_print(" \n[dateentry] get date\n") );

	g_return_val_if_fail (GTK_IS_DATE_ENTRY (dateentry), 0);

	return(g_date_get_julian(dateentry->date));
}


/*
static void
gtk_dateentry_set_property (GObject         *object,
                        guint            prop_id,
                        const GValue    *value,
                        GParamSpec      *pspec)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);

	DB( g_print("\n[dateentry] set %d\n", prop_id) );


  switch (prop_id)
    {
	case PROPERTY_DATE:
	   DB( g_print(" -> date to %d\n", g_value_get_uint (value)) );

		g_date_set_julian (dateentry->date, g_value_get_uint (value));
		gtk_dateentry_entry_set_text(dateentry);
	break;


    default:
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
gtk_dateentry_get_property (GObject         *object,
                        guint            prop_id,
                        GValue          *value,
                        GParamSpec      *pspec)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);

	DB( g_print("\n[dateentry] get\n") );

  switch (prop_id)
    {
	case PROPERTY_DATE:
	   DB( g_print(" -> date is %d\n", 0) );
		g_value_set_uint (value, g_date_get_julian(dateentry->date));
		break;

    default:
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}
*/


/*
** fill in our gtkentry from our GDate
*/
static void gtk_dateentry_entry_set_text(GtkDateEntry * dateentry)
{
gchar buffer[256];

	DB( g_print("\n[dateentry] date2entry\n") );

	g_date_clamp(dateentry->date, &dateentry->mindate, &dateentry->maxdate);

	
	if(g_date_valid(dateentry->date) == TRUE)
	{
		g_date_strftime (buffer, 256 - 1, "%x", dateentry->date);
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), buffer);
		
		DB( g_print(" = %s\n", buffer) );
	}
	else
		gtk_entry_set_text (GTK_ENTRY (dateentry->entry), "??");


	/* emit the signal */
	if(dateentry->lastdate != g_date_get_julian(dateentry->date))
	{
		DB( g_print(" **emit 'changed' signal**\n") );

		g_signal_emit_by_name (dateentry, "changed", NULL, NULL);
	}

	dateentry->lastdate = g_date_get_julian(dateentry->date);

}


static void gtk_dateentry_tokens(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
const gchar *str;
GDateParseTokens pt;

 	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));

	g_date_fill_parse_tokens(str, &pt);
	DB( g_print(" -> parsetoken return is %d values :%d %d %d\n", pt.num_ints, pt.n[0], pt.n[1], pt.n[2]) );

	// initialize with today's date
	g_date_set_time_t(dateentry->date, time(NULL));
	
	switch( pt.num_ints )
	{
		case 1:
			DB( g_print(" -> seizured 1 number\n") );
			if(g_date_valid_day(pt.n[0]))
				g_date_set_day(dateentry->date, pt.n[0]);
			break;
		case 2:
			DB( g_print(" -> seizured 2 numbers\n") );
			if( dmy_order[0] != G_DATE_YEAR )
			{
				if( dmy_order[0] == G_DATE_DAY )
				{
					if(g_date_valid_day(pt.n[0]))
					    g_date_set_day(dateentry->date, pt.n[0]);
					if(g_date_valid_month(pt.n[1]))
						g_date_set_month(dateentry->date, pt.n[1]);
				}
				else
				{
					if(g_date_valid_day(pt.n[1]))
					    g_date_set_day(dateentry->date, pt.n[1]);
					if(g_date_valid_month(pt.n[0]))
						g_date_set_month(dateentry->date, pt.n[0]);
				}
			}
			break;
	}


	
}





/*
** parse the gtkentry and store the GDate
*/
static void gtk_dateentry_entry_parse(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
const gchar *str;

	DB( g_print("\n[dateentry] entry_parse\n") );

 	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));

	//1) we parse the string according to the locale
	g_date_set_parse (dateentry->date, str);
	if(g_date_valid(dateentry->date) == FALSE)
	{
		//2) give a try to tokens: day, day/month, month/day
		gtk_dateentry_tokens(gtkentry, user_data);
	}

	//3) at last if date still invalid, put today's dateentry_signals
	// we should consider just warn the user here
	if(g_date_valid(dateentry->date) == FALSE)
	{
		/* today's date */
		g_date_set_time_t(dateentry->date, time(NULL));
	}

	gtk_dateentry_entry_set_text(dateentry);

}

static void gtk_dateentry_calendar_year(GtkWidget *calendar, GtkDateEntry *dateentry)
{
guint year, month, day;

	DB( g_print(" (dateentry) year changed\n") );

	gtk_calendar_get_date (GTK_CALENDAR (dateentry->calendar), &year, &month, &day);
	if( year < 1900)
		g_object_set(calendar, "year", 1900, NULL);

	if( year > 2200)
		g_object_set(calendar, "year", 2200, NULL);
	
}

/*
** store the calendar date to GDate, update our gtkentry
*/
static void gtk_dateentry_calendar_getfrom(GtkWidget * calendar, GtkDateEntry * dateentry)
{
guint year, month, day;

	DB( g_print(" (dateentry) get from calendar\n") );

	gtk_calendar_get_date (GTK_CALENDAR (dateentry->calendar), &year, &month, &day);
	g_date_set_dmy (dateentry->date, day, month + 1, year);
	gtk_dateentry_entry_set_text(dateentry);
}


static gint gtk_dateentry_calendar_select(GtkWidget * calendar, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print(" (dateentry) calendar_select\n") );

	gtk_dateentry_hide_popdown_window(dateentry);
	gtk_dateentry_calendar_getfrom(NULL, dateentry);
	return FALSE;
}


static gint
gtk_dateentry_entry_key (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] entry key pressed: state=%04x, keyval=%04x\n", event->state, event->keyval) );

	if( event->keyval == GDK_KEY_Up )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_add_days (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_add_months (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_add_years (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		return TRUE;
	}
	else
	if( event->keyval == GDK_KEY_Down )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_subtract_days (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_subtract_months (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_subtract_years (dateentry->date, 1);
			gtk_dateentry_entry_set_text(dateentry);
		}
		return TRUE;
	}

	return FALSE;
}


static void
position_popup (GtkDateEntry * dateentry)
{
gint x, y;
gint bwidth, bheight;
GtkRequisition req;
GdkWindow *gdkwindow;
GtkAllocation allocation;

	DB( g_print("\n[dateentry] position popup\n") );

	gtk_widget_size_request (dateentry->popwin, &req);

	gdkwindow = gtk_widget_get_window(dateentry->arrow);
	gdk_window_get_origin (gdkwindow, &x, &y);

	gtk_widget_get_allocation(dateentry->arrow, &allocation);
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

	gtk_window_move (GTK_WINDOW (dateentry->popwin), x, y);
}



static void
gtk_dateentry_popup_display (GtkDateEntry * dateentry)
{
const char *str;
int month;

  //gint height, width, x, y;
  //gint old_width, old_height;

	DB( g_print("\n[dateentry] popup_display\n****\n\n") );

  //old_width = dateentry->popwin->allocation.width;
  //old_height  = dateentry->popwin->allocation.height;


/* update */
	str = gtk_entry_get_text (GTK_ENTRY (dateentry->entry));
	g_date_set_parse (dateentry->date, str);

	if(g_date_valid(dateentry->date) == TRUE)
	{
		/* GtkCalendar expects month to be in 0-11 range (inclusive) */
		month = g_date_get_month (dateentry->date) - 1;
		gtk_calendar_select_month (GTK_CALENDAR (dateentry->calendar),
				   CLAMP (month, 0, 11),
				   g_date_get_year (dateentry->date));
        gtk_calendar_select_day (GTK_CALENDAR (dateentry->calendar),
				 g_date_get_day (dateentry->date));
	}

	position_popup(dateentry);

	gtk_widget_show (dateentry->popwin);

  gtk_grab_add (dateentry->popwin);

  // this close the popup */
	GdkWindow *gdkwindow;
	gdkwindow = gtk_widget_get_window(dateentry->popwin);

  gdk_pointer_grab (gdkwindow, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

}

static void
gtk_dateentry_hide_popdown_window(GtkDateEntry *dateentry)
{
	DB( g_print("\n[dateentry] hide_popdown_window\n") );

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dateentry->arrow), FALSE);

  gtk_grab_remove(dateentry->popwin);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_widget_hide(dateentry->popwin);
}

static gint
gtk_dateentry_arrow_press (GtkWidget * widget, GtkDateEntry * dateentry)
{
  GtkToggleButton *button;

	DB( g_print("\n[dateentry] arrow_press\n") );

  button = GTK_TOGGLE_BUTTON(widget);

  if(!gtk_toggle_button_get_active(button)){
     gtk_widget_hide (dateentry->popwin);
     gtk_grab_remove (dateentry->popwin);
     gdk_pointer_ungrab (GDK_CURRENT_TIME);

	gtk_dateentry_calendar_getfrom(NULL, dateentry);
     return TRUE;
  }

  gtk_dateentry_popup_display(dateentry);
  return TRUE;
}

static gint
key_press_popup (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;


	DB( g_print("\n[dateentry] key pressed%d\n", event->keyval) );

	if (event->keyval != GDK_KEY_Escape)
		return FALSE;

	g_signal_stop_emission_by_name (widget, "key_press_event");

	gtk_dateentry_hide_popdown_window(dateentry);


	return TRUE;
}


static gint
gtk_dateentry_button_press (GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
GtkWidget *child;

DB( g_print("\n[dateentry] button_press\n") );

	child = gtk_get_event_widget (event);

	if (child != widget)
	{
		while (child)
		{
			if (child == widget)
				return FALSE;
			child = gtk_widget_get_parent(child);
		}
	}

	gtk_widget_hide (widget);
	gtk_grab_remove (widget);
	gdk_pointer_ungrab (GDK_CURRENT_TIME);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_DATE_ENTRY(user_data)->arrow), FALSE);

	return TRUE;
}

