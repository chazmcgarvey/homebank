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

#ifndef __GTK_DATEENTRY_H__
#define __GTK_DATEENTRY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_DATE_ENTRY            (gtk_dateentry_get_type ())
#define GTK_DATE_ENTRY(obj)			   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntry))
#define GTK_DATE_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass)
#define GTK_IS_DATE_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATE_ENTRY))
#define GTK_IS_DATE_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATE_ENTRY))
#define GTK_DATE_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATE_ENTRY, GtkDateEntryClass))


typedef struct _GtkDateEntry		GtkDateEntry;
typedef struct _GtkDateEntryClass	GtkDateEntryClass;

/* you should access only the entry and list fields directly */
struct _GtkDateEntry
{
	/*< private >*/
	GtkHBox hbox;

	/*< public >*/
	GtkWidget *entry;
    GtkWidget *arrow;
	GtkWidget *popup;
	GtkWidget *popwin;
	GtkWidget *frame;
	GtkWidget *calendar;

	GDate	*date;
	guint32	lastdate;

	GDate	mindate, maxdate;

};

struct _GtkDateEntryClass
{
	GtkHBoxClass parent_class;

  /* signals */
  void     (* changed)          (GtkDateEntry *dateentry);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GType		gtk_dateentry_get_type(void);

GtkWidget	*gtk_dateentry_new(void);

guint32		gtk_dateentry_get_date(GtkDateEntry * dateentry);
void		gtk_dateentry_set_date(GtkDateEntry * dateentry, guint32 julian_days);
void		gtk_dateentry_set_mindate(GtkDateEntry * dateentry, guint32 julian_days);
void		gtk_dateentry_set_maxdate(GtkDateEntry * dateentry, guint32 julian_days);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_DATE_ENTRY_H__ */


