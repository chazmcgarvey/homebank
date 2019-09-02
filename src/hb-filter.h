/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2019 Maxime DOYEN
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

#ifndef __HB_FILTER_H__
#define __HB_FILTER_H__


/*
** filter options
*/
enum
{
	FILTER_DATE,
	FILTER_STATUS,
	FILTER_PAYMODE,
	FILTER_AMOUNT,
	FILTER_ACCOUNT,
	FILTER_CATEGORY,
	FILTER_PAYEE,
	FILTER_TEXT,
	FILTER_MAX
};


enum
{
	FLT_RANGE_THISMONTH = 0,
	FLT_RANGE_LASTMONTH = 1,
	FLT_RANGE_THISQUARTER = 2,
	FLT_RANGE_LASTQUARTER = 3,
	FLT_RANGE_THISYEAR = 4,
	FLT_RANGE_LASTYEAR = 5,	 //was not existing on 4.5
	// 6 separator
	FLT_RANGE_LAST30DAYS = 7,
	FLT_RANGE_LAST60DAYS = 8,
	FLT_RANGE_LAST90DAYS = 9,
	FLT_RANGE_LAST12MONTHS = 10,
	// 11 separator
	FLT_RANGE_OTHER = 12,
	// 13 separator
	FLT_RANGE_ALLDATE = 14
};


enum
{
	FLT_TYPE_EXPENSE = 0,
	FLT_TYPE_INCOME = 1,
	// 2 separator
	FLT_TYPE_ALL = 3
};


enum
{
	FLT_STATUS_UNCATEGORIZED = 0,
	FLT_STATUS_UNRECONCILED = 1,
	FLT_STATUS_UNCLEARED = 2,
	FLT_STATUS_RECONCILED = 3,
	FLT_STATUS_CLEARED = 4,
	// 5 separator
	FLT_STATUS_ALL = 6
};


enum
{
	FLT_QSEARCH_MEMO     = 1<<0,
	FLT_QSEARCH_INFO     = 1<<1,
	FLT_QSEARCH_PAYEE    = 1<<2,
	FLT_QSEARCH_CATEGORY = 1<<3,
	FLT_QSEARCH_TAGS     = 1<<4,
	FLT_QSEARCH_AMOUNT   = 1<<5
};


typedef struct _filter	Filter;

struct _filter
{




	guint32		mindate, maxdate;
	gint		range;

	gshort		option[FILTER_MAX];
	gboolean	reconciled;
	gboolean	cleared;

	gboolean	paymode[NUM_PAYMODE_MAX];
	gdouble		minamount, maxamount;
	gboolean	exact;
	gchar		*info;
	gchar		*memo;
	gchar		*tag;

	/* unsaved datas */
	gint		nbdaysfuture;
	gint		type;			/* ! not sure necessary to store this :: dsp_account: inc/exp used */
	gint		status;			/* ! not sure necessary to store this :: dsp_account */
	gboolean	forceadd;
	gboolean	forcechg;
	gboolean	forceremind;
	gchar		last_tab[8];	/* keep last active tab */
};


Filter *da_flt_malloc(void);
void da_flt_free(Filter *flt);

void da_flt_destroy(void);
void da_flt_new(void);

void da_flt_status_acc_set(Filter *flt, guint32 kacc, gboolean status);
void da_flt_status_pay_set(Filter *flt, guint32 kpay, gboolean status);
void da_flt_status_cat_set(Filter *flt, guint32 kcat, gboolean status);
gboolean da_flt_status_acc_get(Filter *flt, guint32 kacc);
gboolean da_flt_status_pay_get(Filter *flt, guint32 kpay);
gboolean da_flt_status_cat_get(Filter *flt, guint32 kcat);

void filter_status_acc_clear_except(Filter *flt, guint32 selkey);
void filter_status_pay_clear_except(Filter *flt, guint32 selkey);
void filter_status_cat_clear_except(Filter *flt, guint32 selkey);

void filter_reset(Filter *flt);
void filter_preset_daterange_set(Filter *flt, gint range, guint32 kacc);
void filter_preset_type_set(Filter *flt, gint value);
gboolean filter_preset_daterange_future_enable(gint range);
void filter_preset_daterange_add_futuregap(Filter *filter, gint nbdays);

void filter_set_tag_by_id(Filter *flt, guint32 key);
void filter_preset_status_set(Filter *flt, gint value);

gchar *filter_daterange_text_get(Filter *flt);
gboolean filter_txn_search_match(gchar *needle, Transaction *txn, gint flags);

gint filter_txn_match(Filter *flt, Transaction *ope);

#endif
