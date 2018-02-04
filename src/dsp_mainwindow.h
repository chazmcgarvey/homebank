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

#ifndef __HB_MANWINDOW_GTK_H__
#define __HB_MANWINDOW_GTK_H__

struct hbfile_data
{
	GtkWidget	*window;

	GtkWidget	*toolbar;
	GtkWidget	*menubar;
	GtkWidget	*vpaned;
	GtkWidget	*hpaned;

	/* panel: your account */
	GtkWidget	*LV_acc;
	GtkWidget   *BT_expandall;
	GtkWidget   *BT_collapseall;
	gboolean	showall;
	GSimpleActionGroup *action_group_acc;

	GtkWidget	*GR_top;
	GtkWidget	*LV_top;
	gdouble		toptotal;
	GtkWidget	*CY_range;
	GtkWidget	*RE_pie;

	GtkWidget	*GR_upc;
	GtkWidget	*LV_upc;
	GtkWidget   *LB_maxpostdate;
	GtkWidget   *BT_sched_skip;
	GtkWidget   *BT_sched_post;
	GtkWidget   *BT_sched_editpost;

	GtkWidget   *RA_type;
	
	gchar	*wintitle;

	Account *acc;

	gint	busy;

	GtkUIManager	*manager;
	GtkActionGroup *actions;

	GtkRecentManager *recent_manager;
	GtkWidget *recent_menu;

	Filter		*filter;
	
	/*
	UBYTE	accnum;
	UBYTE	pad0;
	struct	Account *acc;

	ULONG	mindate, maxdate;
	ULONG	change;
	ULONG	keyvalue;
	UBYTE	title[140];
	UBYTE	Filename[108];
	UBYTE	csvpath[108];
	*/
};


GtkWidget *create_hbfile_window(GtkWidget *do_widget);
void ui_mainwindow_populate_accounts(GtkWidget *widget, gpointer user_data);
gboolean ui_mainwindow_open_backup_check_confirm(gchar *filepath);
void ui_mainwindow_open_internal(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_update(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_action_help_welcome(void);

#endif /* __HB_MANWINDOW_GTK_H__ */
