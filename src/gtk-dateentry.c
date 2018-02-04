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


static guint dateentry_signals[LAST_SIGNAL] = {0,};


G_DEFINE_TYPE(GtkDateEntry, gtk_date_entry, GTK_TYPE_BOX)




static void
update_text(GtkDateEntry *self)
{
GtkDateEntryPrivate *priv = self->priv;
gchar label[24];

	DB( g_print("\n[dateentry] update text\n") );

	g_date_strftime (label, 17 - 1, "%x", priv->date);
	gtk_entry_set_text (GTK_ENTRY (priv->entry), label);
	DB( g_print(" = %s\n", label) );
}


static void
eval_date(GtkDateEntry *self)
{
GtkDateEntryPrivate *priv = self->priv;

	g_date_clamp(priv->date, &priv->mindate, &priv->maxdate);
	
	update_text(self);
	
	if(priv->lastdate != g_date_get_julian(priv->date))
	{
		DB( g_print(" **emit 'changed' signal**\n") );
		g_signal_emit_by_name (self, "changed", NULL, NULL);
	}

	priv->lastdate = g_date_get_julian(priv->date);
}

static void
parse_date(GtkDateEntry *self)
{
GtkDateEntryPrivate *priv = self->priv;

	DB( g_print("\n[dateentry] parse date\n") );

	g_date_set_parse (priv->date, gtk_entry_get_text (GTK_ENTRY (priv->entry)));
	if(!g_date_valid(priv->date))
	{
		g_date_set_time_t(priv->date, time(NULL));
	}
	eval_date(self);
}


static void
gtk_date_entry_cb_calendar_day_selected(GtkWidget * calendar, GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
guint year, month, day;

	DB( g_print("\n[dateentry] calendar_day_selected\n") );

	gtk_calendar_get_date (GTK_CALENDAR (priv->calendar), &year, &month, &day);
	g_date_set_dmy (priv->date, day, month + 1, year);
	eval_date(dateentry);	
}


static gint
gtk_date_entry_cb_calendar_day_select_double_click(GtkWidget * calendar, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print("\n[dateentry] calendar_day_select_double_click\n") );

	gtk_widget_hide (priv->popover);	

	return FALSE;
}


static void 
gtk_date_entry_cb_calendar_monthyear(GtkWidget *calendar, GtkDateEntry *dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
guint year, month, day;

	DB( g_print("\n[dateentry] cb_calendar_monthyear\n") );

	gtk_calendar_get_date (GTK_CALENDAR (priv->calendar), &year, &month, &day);
	if( year < 1900)
		g_object_set(calendar, "year", 1900, NULL);

	if( year > 2200)
		g_object_set(calendar, "year", 2200, NULL);
	
}


static gint
gtk_date_entry_cb_entry_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print("\n[dateentry] entry key pressed: state=%04x, keyval=%04x\n", event->state, event->keyval) );

	if( event->keyval == GDK_KEY_Up )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_add_days (priv->date, 1);
			eval_date(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_add_months (priv->date, 1);
			eval_date(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_add_years (priv->date, 1);
			eval_date(dateentry);
		}
		return TRUE;
	}
	else
	if( event->keyval == GDK_KEY_Down )
	{
		if( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) )
		{
			g_date_subtract_days (priv->date, 1);
			eval_date(dateentry);
		}
		else
		if( event->state & GDK_SHIFT_MASK )
		{
			g_date_subtract_months (priv->date, 1);
			eval_date(dateentry);
		}
		else
		if( event->state & GDK_CONTROL_MASK )
		{
			g_date_subtract_years (priv->date, 1);
			eval_date(dateentry);
		}
		return TRUE;
	}

	return FALSE;
}


static void
gtk_date_entry_cb_entry_activate(GtkWidget *gtkentry, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] entry_activate\n") );

	parse_date(dateentry);
	eval_date(dateentry);
}


static gboolean 
gtk_date_entry_cb_entry_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
GtkDateEntry *dateentry = user_data;

	DB( g_print("\n[dateentry] entry focus-out-event %d\n", gtk_widget_is_focus(GTK_WIDGET(dateentry))) );

	parse_date(dateentry);
	eval_date(dateentry);
	return FALSE;
}


static void
gtk_date_entry_cb_button_clicked (GtkWidget * widget, GtkDateEntry * dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
//GdkRectangle rect;
int month;

	DB( g_print("\n[dateentry] button_clicked\n") );

	/* GtkCalendar expects month to be in 0-11 range (inclusive) */
	month = g_date_get_month (priv->date) - 1;
	
	g_signal_handler_block(priv->calendar, priv->hid_dayselect);
	
	gtk_calendar_select_month (GTK_CALENDAR (priv->calendar),
			   CLAMP (month, 0, 11),
			   g_date_get_year (priv->date));
    gtk_calendar_select_day (GTK_CALENDAR (priv->calendar),
			 g_date_get_day (priv->date));
			 
	g_signal_handler_unblock(priv->calendar, priv->hid_dayselect);

	gtk_popover_set_relative_to (GTK_POPOVER (priv->popover), GTK_WIDGET (priv->entry));
	//gtk_widget_get_clip(priv->arrow, &rect);
	//gtk_popover_set_pointing_to (GTK_POPOVER (priv->popover), &rect);

	gtk_widget_show_all (priv->popover);
}


static void 
gtk_date_entry_destroy (GtkWidget *object)
{
GtkDateEntry *dateentry = GTK_DATE_ENTRY (object);
GtkDateEntryPrivate *priv = dateentry->priv;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_DATE_ENTRY(object));

	DB( g_print("\n[dateentry] destroy\n") );

	DB( g_print(" free gtkentry: %p\n", priv->entry) );
	DB( g_print(" free arrow: %p\n", priv->button) );

	DB( g_print(" free dateentry: %p\n", dateentry) );

	if(priv->date)
		g_date_free(priv->date);
	priv->date = NULL;

	GTK_WIDGET_CLASS (gtk_date_entry_parent_class)->destroy (object);
}



static void
gtk_date_entry_dispose (GObject *gobject)
{
//GtkDateEntry *self = GTK_DATE_ENTRY (gobject);

	DB( g_print("\n[dateentry] dispose\n") );

	
  //g_clear_object (&self->priv->an_object);

  G_OBJECT_CLASS (gtk_date_entry_parent_class)->dispose (gobject);
}




static void
gtk_date_entry_finalize (GObject *gobject)
{
//GtkDateEntry *self = GTK_DATE_ENTRY (gobject);

	DB( g_print("\n[dateentry] finalize\n") );

	
	//g_date_free(self->date);
  //g_free (self->priv->a_string);

  /* Always chain up to the parent class; as with dispose(), finalize()
   * is guaranteed to exist on the parent's class virtual function table
   */
  G_OBJECT_CLASS(gtk_date_entry_parent_class)->finalize (gobject);
}



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
	
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(dateentry)), GTK_STYLE_CLASS_LINKED);

	priv->entry = gtk_entry_new ();
	gtk_entry_set_width_chars(GTK_ENTRY(priv->entry), 16);
	gtk_entry_set_max_width_chars(GTK_ENTRY(priv->entry), 16);
	gtk_box_pack_start (GTK_BOX (dateentry), priv->entry, TRUE, TRUE, 0);

	priv->button = gtk_button_new ();
	priv->arrow = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_container_add (GTK_CONTAINER (priv->button), priv->arrow);
	gtk_box_pack_end (GTK_BOX (dateentry), priv->button, FALSE, FALSE, 0);

	priv->popover = gtk_popover_new (priv->button);
	gtk_popover_set_position(GTK_POPOVER(priv->popover), GTK_POS_BOTTOM);
	gtk_container_set_border_width (GTK_CONTAINER (priv->popover), 6);
	priv->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (priv->popover), priv->calendar);

	gtk_widget_show_all (GTK_WIDGET(dateentry));

	/* initialize datas */
	priv->date = g_date_new();
	g_date_set_time_t(priv->date, time(NULL));
	g_date_set_dmy(&priv->mindate,  1,  1, 1900);	//693596
	g_date_set_dmy(&priv->maxdate, 31, 12, 2200);	//803533
	update_text(dateentry);


	g_signal_connect (priv->entry, "key-press-event",
				G_CALLBACK (gtk_date_entry_cb_entry_key_pressed), dateentry);

	g_signal_connect_after (priv->entry, "focus-out-event",
				G_CALLBACK (gtk_date_entry_cb_entry_focus_out), dateentry);

	g_signal_connect (priv->entry, "activate",
				G_CALLBACK (gtk_date_entry_cb_entry_activate), dateentry);


	g_signal_connect (priv->button, "clicked",
				G_CALLBACK (gtk_date_entry_cb_button_clicked), dateentry);


	g_signal_connect (priv->calendar, "prev-year",
				G_CALLBACK (gtk_date_entry_cb_calendar_monthyear), dateentry);
	g_signal_connect (priv->calendar, "next-year",
				G_CALLBACK (gtk_date_entry_cb_calendar_monthyear), dateentry);
	g_signal_connect (priv->calendar, "prev-month",
				G_CALLBACK (gtk_date_entry_cb_calendar_monthyear), dateentry);
	g_signal_connect (priv->calendar, "next-month",
				G_CALLBACK (gtk_date_entry_cb_calendar_monthyear), dateentry);

	priv->hid_dayselect = g_signal_connect (priv->calendar, "day-selected",
				G_CALLBACK (gtk_date_entry_cb_calendar_day_selected), dateentry);

	g_signal_connect (priv->calendar, "day-selected-double-click",
				G_CALLBACK (gtk_date_entry_cb_calendar_day_select_double_click), dateentry);

}


GtkWidget *
gtk_date_entry_new ()
{
GtkDateEntry *dateentry;

	DB( g_print("\n[dateentry] new\n") );

	dateentry = g_object_new (GTK_TYPE_DATE_ENTRY, NULL);

	return GTK_WIDGET(dateentry);
}


/*
**
*/
void 
gtk_date_entry_set_mindate(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print("\n[dateentry] set mindate\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&priv->mindate, julian_days);
	}
}


/*
**
*/
void 
gtk_date_entry_set_maxdate(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print("\n[dateentry] set maxdate\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (&priv->maxdate, julian_days);
	}
}


/*
**
*/
void
gtk_date_entry_set_date(GtkDateEntry *dateentry, guint32 julian_days)
{
GtkDateEntryPrivate *priv = dateentry->priv;

	DB( g_print("\n[dateentry] set date\n") );

	g_return_if_fail (GTK_IS_DATE_ENTRY (dateentry));

	if(g_date_valid_julian(julian_days))
	{
		g_date_set_julian (priv->date, julian_days);
	}
	else
	{
		g_date_set_time_t(priv->date, time(NULL));
	}
	eval_date(dateentry);
}


/*
**
*/
guint32
gtk_date_entry_get_date(GtkDateEntry *dateentry)
{
GtkDateEntryPrivate *priv = dateentry->priv;
	
	DB( g_print("\n[dateentry] get date\n") );

	g_return_val_if_fail (GTK_IS_DATE_ENTRY (dateentry), 0);

	return(g_date_get_julian(priv->date));
}

