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

#ifndef __HOMEBANK_REPTIME_H__
#define __HOMEBANK_REPTIME_H__

enum
{
	LST_REPTIME_POS,
	LST_REPTIME_KEY,
	LST_REPTIME_TITLE,
	LST_REPTIME_AMOUNT,
	NUM_LST_REPTIME
};

/* for choose options */
enum
{
	FOR_REPTIME_ACCOUNT,
	FOR_REPTIME_CATEGORY,
	FOR_REPTIME_PAYEE,
	NUM_FOR_REPTIME
};


/* view by choose options */
enum
{
	GROUPBY_REPTIME_DAY,
	GROUPBY_REPTIME_WEEK,
	GROUPBY_REPTIME_MONTH,
	GROUPBY_REPTIME_QUARTER,
	GROUPBY_REPTIME_YEAR,
};


enum {
	HID_REPTIME_MINDATE,
	HID_REPTIME_MAXDATE,
	HID_REPTIME_RANGE,
	HID_REPTIME_VIEW,
	MAX_REPTIME_HID
};

struct ui_reptime_data
{
	GQueue		*txn_queue;
	Filter		*filter;

	gboolean	detail;
	gint		charttype;
	guint32		accnum;

	GtkWidget	*window;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkWidget	*TB_bar;

	GtkWidget	*TX_info;
	GtkWidget	*TX_daterange;
	GtkWidget	*CY_for;
	GtkWidget	*CY_view;
	GtkWidget	*RG_zoomx, *LB_zoomx;
	GtkWidget	*CM_minor;
	GtkWidget	*CM_cumul;
	GtkWidget	*LV_report;


	//GtkWidget	*GR_select;
	GtkWidget	*CM_all;
	GtkWidget	*LB_acc, *PO_acc;
	GtkWidget	*LB_cat, *PO_cat;
	GtkWidget	*LB_pay, *PO_pay;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*RE_line;

	GtkWidget	*GR_detail;
	GtkWidget	*LV_detail;

	gulong		handler_id[MAX_REPTIME_HID];

};




GtkWidget *ui_reptime_window_new(guint32 accnum);

#endif
