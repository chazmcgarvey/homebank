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

#ifndef __HB_FILTER_GTK_H__
#define __HB_FILTER_GTK_H__

enum
{
	BUTTON_ALL,
	BUTTON_NONE,
	BUTTON_INVERT,
	MAX_BUTTON
};





struct ui_flt_manage_data
{
	Filter		*filter;

	GtkWidget	*notebook;

	GtkWidget	*CY_option[FILTER_MAX];

	GtkWidget	*PO_mindate, *PO_maxdate;
	GtkWidget	*CY_month, *NB_year;

	GtkWidget	*CM_reconciled;
	GtkWidget   *CM_cleared;

	GtkWidget	*CM_forceadd, *CM_forcechg, *CM_forceremind;

	GtkWidget	*CM_paymode[NUM_PAYMODE_MAX];

	GtkWidget	*ST_minamount, *ST_maxamount;

	GtkWidget	*CM_exact;
	GtkWidget	*ST_info, *ST_wording, *ST_tag;

	GtkWidget	*LV_acc, *BT_acc[MAX_BUTTON];
	GtkWidget	*LV_pay, *BT_pay[MAX_BUTTON];
	GtkWidget	*LV_cat, *BT_cat[MAX_BUTTON];

	gboolean	show_account;


};


gint ui_flt_manage_dialog_new(GtkWidget *widget, Filter *filter, gboolean show_account);


#endif
