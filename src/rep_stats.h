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

#ifndef __HOMEBANK_REPDIST_H__
#define __HOMEBANK_REPDIST_H__

enum {
	HID_REPDIST_MINDATE,
	HID_REPDIST_MAXDATE,
	HID_REPDIST_RANGE,
	HID_REPDIST_VIEW,
	MAX_REPDIST_HID
};


enum
{
	LST_REPDIST_POS,
	LST_REPDIST_KEY,
	LST_REPDIST_NAME,
	LST_REPDIST_EXPENSE,
	LST_REPDIST_EXPRATE,
	LST_REPDIST_INCOME,
	LST_REPDIST_INCRATE,
	LST_REPDIST_BALANCE,
	LST_REPDIST_BALRATE,
	NUM_LST_REPDIST
};

enum
{
	BY_REPDIST_CATEGORY,
	BY_REPDIST_SUBCATEGORY,
	BY_REPDIST_PAYEE,
	BY_REPDIST_TAG,
	BY_REPDIST_MONTH,
	BY_REPDIST_YEAR,
};

struct ui_repdist_data
{
	GQueue		*txn_queue;
	Filter		*filter;

	gboolean	detail;
	gboolean	legend;
	gboolean	rate;
	gdouble		total_expense;
	gdouble		total_income;
	gdouble		total_balance;

	GtkWidget	*window;

	GtkUIManager	*ui;
	GtkActionGroup *actions;

	GtkWidget	*TB_bar;

	GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*CY_by;
	GtkWidget	*CY_view;
	GtkWidget	*RG_zoomx, *LB_zoomx;
	GtkWidget	*LV_report;
	GtkWidget	*CM_byamount;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*TX_daterange;
	GtkWidget	*TX_total[3];

	GtkWidget	*RE_chart;

	GtkWidget	*GR_detail;
	GtkWidget	*LV_detail;



	gulong		handler_id[MAX_REPDIST_HID];

};



GtkWidget *ui_repdist_window_new(void);

#endif
