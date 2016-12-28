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

#ifndef __GTK_DATE_ENTRY_H__
#define __GTK_DATE_ENTRY_H__

G_BEGIN_DECLS

#define GTK_TYPE_DATE_ENTRY            (gtk_date_entry_get_type ())
#define GTK_DATE_ENTRY(obj)			   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntry))
#define GTK_DATE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass)
#define GTK_IS_DATE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATE_ENTRY))
#define GTK_IS_DATE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATE_ENTRY))
#define GTK_DATE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass))

typedef struct _GtkDateEntry		GtkDateEntry;
typedef struct _GtkDateEntryClass	GtkDateEntryClass;
typedef struct _GtkDateEntryPrivate	GtkDateEntryPrivate;


struct _GtkDateEntry
{
	GtkBox box;

	/*< private >*/
	GtkDateEntryPrivate *priv;
};


struct _GtkDateEntryClass
{
	GtkBoxClass parent_class;

	/* signals */
	void     (* changed)          (GtkDateEntry *dateentry);

	/* Padding for future expansion */
	void (*_gtk_reserved0) (void);
	void (*_gtk_reserved1) (void);
	void (*_gtk_reserved2) (void);
	void (*_gtk_reserved3) (void);
};


struct _GtkDateEntryPrivate
{
	GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *arrow;
	GtkWidget *popup_window;
	GtkWidget *frame;
	GtkWidget *calendar;

	GDate	*date;
	guint32	lastdate;

	GDate	mindate, maxdate;
	GdkDevice   *device;
	gboolean	has_grab;
	gboolean    popup_in_progress;

	GdkDevice *grab_pointer;
	GdkDevice *grab_keyboard;
};


GType		gtk_date_entry_get_type(void) G_GNUC_CONST;

GtkWidget	*gtk_date_entry_new(void);

guint32		gtk_date_entry_get_date(GtkDateEntry * dateentry);
void		gtk_date_entry_set_date(GtkDateEntry * dateentry, guint32 julian_days);
void		gtk_date_entry_set_mindate(GtkDateEntry * dateentry, guint32 julian_days);
void		gtk_date_entry_set_maxdate(GtkDateEntry * dateentry, guint32 julian_days);

G_END_DECLS

#endif /* __GTK_DATE_ENTRY_H__ */


