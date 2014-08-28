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

#include "homebank.h"
#include "hb-filter.h"

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;




/* = = = = = = = = = = = = = = = = = = = = */
/* Filter */

Filter *da_filter_malloc(void)
{
	return g_malloc0(sizeof(Filter));
}

void da_filter_free(Filter *flt)
{
	if(flt != NULL)
	{
		g_free(flt->wording);
		g_free(flt->info);
		g_free(flt->tag);
		g_free(flt);
	}
}

/* = = = = = = = = = = = = = = = = = = = = */

gchar *filter_daterange_text_get(Filter *flt)
{
gchar buffer1[128];
gchar buffer2[128];
GDate *date;

	date = g_date_new_julian(flt->mindate);
	g_date_strftime (buffer1, 128-1, PREFS->date_format, date);
	g_date_set_julian(date, flt->maxdate);
	g_date_strftime (buffer2, 128-1, PREFS->date_format, date);
	g_date_free(date);

	return g_strdup_printf(_("<i>from</i> %s <i>to</i> %s"), buffer1, buffer2);
}



static void filter_default_date_set(Filter *flt)
{
	flt->mindate = 693596;	//01/01/1900
	flt->maxdate = 803533;	//31/12/2200
}


static void filter_clear(Filter *flt)
{
guint i;

	for(i=0;i<FILTER_MAX;i++)
	{
		flt->option[i] = 0;
	}
	
	g_free(flt->info);
	g_free(flt->wording);
	g_free(flt->tag);
	flt->info = NULL;
	flt->wording = NULL;
	flt->tag = NULL;

	flt->last_tab = 0;
}


void filter_default_all_set(Filter *flt)
{
gint i;

	DB( g_print("(filter) reset %p\n", flt) );

	filter_clear(flt);

	flt->range  = FLT_RANGE_LAST12MONTHS;
	flt->type   = FLT_TYPE_ALL;
	flt->status = FLT_STATUS_ALL;

	flt->option[FILTER_DATE] = 1;
	filter_default_date_set(flt);

	for(i=0;i<NUM_PAYMODE_MAX;i++)
		flt->paymode[i] = TRUE;

	filter_preset_daterange_set(flt, flt->range);

}


void filter_preset_daterange_set(Filter *flt, gint range)
{
GDate *date;
GList *list;
guint32 refjuliandate, month, year, qnum;

	// any date :: todo : get date of current accout only when account 
	flt->range = range;
	if(g_list_length(GLOBALS->ope_list) > 0) // get all transaction date bound
	{
		GLOBALS->ope_list = da_transaction_sort(GLOBALS->ope_list);
		list = g_list_first(GLOBALS->ope_list);
		flt->mindate = ((Transaction *)list->data)->date;
		list = g_list_last(GLOBALS->ope_list);
		flt->maxdate = ((Transaction *)list->data)->date;
	}
	else
		filter_default_date_set(flt);
	
	
	// by default refjuliandate is today
	// but we adjust if to max transaction date found
	refjuliandate = GLOBALS->today;
	if(flt->maxdate < refjuliandate)
		refjuliandate = flt->maxdate;

	date  = g_date_new_julian(refjuliandate);
	month = g_date_get_month(date);
	year  = g_date_get_year(date);
	qnum  = ((month - 1) / 3) + 1;

	DB( g_print("m=%d, y=%d, qnum=%d\n", month, year, qnum) );

	switch( range )
	{
		case FLT_RANGE_THISMONTH:
			g_date_set_day(date, 1);
			flt->mindate = g_date_get_julian(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year)-1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_LASTMONTH:
			g_date_set_day(date, 1);
			g_date_subtract_months(date, 1);
			flt->mindate = g_date_get_julian(date);
			month = g_date_get_month(date);
			year = g_date_get_year(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year)-1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_THISQUARTER:
			g_date_set_day(date, 1);
			g_date_set_month(date, (qnum-1)*3+1);
			flt->mindate = g_date_get_julian(date);
			g_date_add_months(date, 3);
			g_date_subtract_days(date, 1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_LASTQUARTER:
			g_date_set_day(date, 1);
			g_date_set_month(date, (qnum-1)*3+1);
			g_date_subtract_months(date, 3);
			flt->mindate = g_date_get_julian(date);
			g_date_add_months(date, 3);
			g_date_subtract_days(date, 1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_THISYEAR:
			g_date_set_dmy(date, PREFS->fisc_year_day, PREFS->fisc_year_month, year);
			if( refjuliandate >= g_date_get_julian (date))
			{
				flt->mindate = g_date_get_julian(date);
			}
			else
			{
				g_date_set_dmy(date, PREFS->fisc_year_day, PREFS->fisc_year_month, year-1);
				flt->mindate = g_date_get_julian(date);
			}
			g_date_add_years (date, 1);
			g_date_subtract_days (date, 1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_LASTYEAR:
			g_date_set_dmy(date, PREFS->fisc_year_day, PREFS->fisc_year_month, year);
			if( refjuliandate >= g_date_get_julian (date))
			{
				g_date_set_dmy(date, PREFS->fisc_year_day, PREFS->fisc_year_month, year-1);
				flt->mindate = g_date_get_julian(date);
			}
			else
			{
				g_date_set_dmy(date, PREFS->fisc_year_day, PREFS->fisc_year_month, year-2);
				flt->mindate = g_date_get_julian(date);
			}
			g_date_add_years (date, 1);
			g_date_subtract_days (date, 1);
			flt->maxdate = g_date_get_julian(date);
			break;

		case FLT_RANGE_LAST30DAYS:
			flt->mindate = refjuliandate - 30;
			flt->maxdate = refjuliandate;
			break;

		case FLT_RANGE_LAST60DAYS:
			flt->mindate = refjuliandate - 60;
			flt->maxdate = refjuliandate;
			break;

		case FLT_RANGE_LAST90DAYS:
			flt->mindate = refjuliandate - 90;
			flt->maxdate = refjuliandate;
			break;

		case FLT_RANGE_LAST12MONTHS:
			g_date_subtract_months(date, 12);
			flt->mindate = g_date_get_julian(date);
			flt->maxdate = refjuliandate;
			break;

		// case FLT_RANGE_OTHER:

		// case FLT_RANGE_ALLDATE:


	}
	g_date_free(date);

}

void filter_preset_type_set(Filter *flt, gint type)
{

	/* any type */
	flt->type = type;
	flt->option[FILTER_AMOUNT] = 0;
	flt->minamount = G_MINDOUBLE;
	flt->maxamount = G_MINDOUBLE;

	switch( type )
	{
		case FLT_TYPE_EXPENSE:
			flt->option[FILTER_AMOUNT] = 1;
			flt->minamount = -G_MAXDOUBLE;
			flt->maxamount = G_MINDOUBLE;
			break;

		case FLT_TYPE_INCOME:
			flt->option[FILTER_AMOUNT] = 1;
			flt->minamount = G_MINDOUBLE;
			flt->maxamount = G_MAXDOUBLE;
			break;
	}

}


void filter_preset_status_set(Filter *flt, gint status)
{
Category *catitem;
GList *lcat, *list;

	/* any status */
	flt->status = status;
	flt->option[FILTER_STATUS] = 0;
	flt->reconciled = TRUE;
	flt->reminded = TRUE;
	flt->forceadd = FALSE;
	flt->forcechg = FALSE;

	flt->option[FILTER_CATEGORY] = 0;
	lcat = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
		catitem = list->data;
		catitem->filter = FALSE;
		list = g_list_next(list);
	}
	g_list_free(lcat);

	switch( status )
	{
		case FLT_STATUS_UNCATEGORIZED:
			flt->option[FILTER_CATEGORY] = 1;
			catitem = da_cat_get(0);	// no category
			catitem->filter = TRUE;
			break;

		case FLT_STATUS_UNRECONCILED:
			flt->option[FILTER_STATUS] = 2;
			flt->reconciled = TRUE;
			//#1336882
			flt->reminded = FALSE;
			break;
	}

}


static gint filter_text_compare(gchar *txntext, gchar *searchtext, gboolean exact)
{
gint retval = 0;

	if( exact )
	{
		if( g_strstr_len(txntext, -1, searchtext) != NULL )
		{
			DB( g_print(" found case '%s'\n", searchtext) );
			retval = 1;
		}
	}
	else
	{
	gchar *word   = g_utf8_casefold(txntext, -1);
	gchar *needle = g_utf8_casefold(searchtext, -1);

		if( g_strrstr(word, needle) != NULL )
		{
			DB( g_print(" found nocase '%s'\n", needle) );
			retval = 1;
		}

		g_free(word);
		g_free(needle);
	}
	return retval;
}


/* used for quicksearch text into transaction */
gboolean filter_txn_search_match(gchar *needle, Transaction *txn, gint flags)
{
gboolean retval = FALSE;
Payee *payitem;
Category *catitem;
gchar *tags;

	if(flags & FLT_QSEARCH_MEMO)
	{
		if(txn->wording)
		{
			retval |= filter_text_compare(txn->wording, needle, FALSE);
		}
		if(retval) goto end;
	}
	
	if(flags & FLT_QSEARCH_INFO)
	{
		if(txn->info)
		{
			retval |= filter_text_compare(txn->info, needle, FALSE);
		}
		if(retval) goto end;
	}

	if(flags & FLT_QSEARCH_PAYEE)
	{
		payitem = da_pay_get(txn->kpay);
		if(payitem)
		{
			retval |= filter_text_compare(payitem->name, needle, FALSE);
		}
		if(retval) goto end;
	}

	if(flags & FLT_QSEARCH_CATEGORY)
	{
		catitem = da_cat_get(txn->kcat);
		if(catitem)
		{
		gchar *fullname = da_cat_get_fullname (catitem);

			retval |= filter_text_compare(fullname, needle, FALSE);
			g_free(fullname);
		}
		if(retval) goto end;
	}
	
	if(flags & FLT_QSEARCH_TAGS)
	{
		tags = transaction_tags_tostring(txn);
		if(tags)
		{
			retval |= filter_text_compare(tags, needle, FALSE);
		}
		g_free(tags);
		//if(retval) goto end;
	}

	
end:
	return retval;
}


gint filter_test(Filter *flt, Transaction *txn)
{
Account *accitem;
Payee *payitem;
Category *catitem;
gint insert;

	//DB( g_print("(filter) test\n") );

	insert = 1;

/*** start filtering ***/

	/* add/change force */
	if(flt->forceadd == TRUE && (txn->flags & OF_ADDED))
		goto end;

	if(flt->forcechg == TRUE && (txn->flags & OF_CHANGED))
		goto end;

/* date */
	if(flt->option[FILTER_DATE]) {
		insert = ( (txn->date >= flt->mindate) && (txn->date <= flt->maxdate) ) ? 1 : 0;
		if(flt->option[FILTER_DATE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* account */
	if(flt->option[FILTER_ACCOUNT]) {
		accitem = da_acc_get(txn->kacc);
		if(accitem)
		{
			insert = ( accitem->filter == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_ACCOUNT] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* payee */
	if(flt->option[FILTER_PAYEE]) {
		payitem = da_pay_get(txn->kpay);
		if(payitem)
		{
			insert = ( payitem->filter == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_PAYEE] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* category */
	if(flt->option[FILTER_CATEGORY]) {
		if(txn->flags & OF_SPLIT)
		{
		guint count, i;
		Split *split;

			insert = 0;	 //fix: 1151259
			count = da_transaction_splits_count(txn);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
				
				split = txn->splits[i];
				catitem = da_cat_get(split->kcat);
				if(catitem)
				{
					tmpinsert = ( catitem->filter == TRUE ) ? 1 : 0;
					if(flt->option[FILTER_CATEGORY] == 2) tmpinsert ^= 1;
				}
				insert |= tmpinsert;
			}
		}
		else
		{
			catitem = da_cat_get(txn->kcat);
			if(catitem)
			{
				insert = ( catitem->filter == TRUE ) ? 1 : 0;
				if(flt->option[FILTER_CATEGORY] == 2) insert ^= 1;
			}
		}
	}
	if(!insert) goto end;

/* status */
	if(flt->option[FILTER_STATUS]) {
	gint insert1 = 0, insert2 = 0;

		if(flt->reconciled)
			insert1 = ( txn->flags & OF_VALID ) ? 1 : 0;
		if(flt->reminded)
			insert2 = ( txn->flags & OF_REMIND ) ? 1 : 0;

		insert = insert1 | insert2;
		if(flt->option[FILTER_STATUS] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* paymode */
	if(flt->option[FILTER_PAYMODE]) {
		insert = ( flt->paymode[txn->paymode] == TRUE) ? 1 : 0;
		if(flt->option[FILTER_PAYMODE] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* amount */
	if(flt->option[FILTER_AMOUNT]) {
		insert = ( (txn->amount >= flt->minamount) && (txn->amount <= flt->maxamount) ) ? 1 : 0;

		if(flt->option[FILTER_AMOUNT] == 2) insert ^= 1;
	}
	if(!insert) goto end;

/* info/wording/tag */
	if(flt->option[FILTER_TEXT])
	{
	gchar *tags;
	gint insert1, insert2, insert3;

		insert1 = insert2 = insert3 = 0;
		if(flt->info)
		{
			if(txn->info)
			{
				insert1 = filter_text_compare(txn->info, flt->info, flt->exact);
			}
		}
		else
			insert1 = 1;

		if(flt->wording)
		{
			if(txn->wording)
			{
				insert2 = filter_text_compare(txn->wording, flt->wording, flt->exact);
			}
		}
		else
			insert2 = 1;

		if(flt->tag)
		{
			tags = transaction_tags_tostring(txn);
			if(tags)
			{
				insert3 = filter_text_compare(tags, flt->tag, flt->exact);
			}
			g_free(tags);
		}
		else
			insert3 = 1;

		insert = insert1 && insert2 && insert3 ? 1 : 0;

		if(flt->option[FILTER_TEXT] == 2) insert ^= 1;

	}
	if(!insert) goto end;

end:
//	DB( g_print(" %d :: %d :: %d\n", flt->mindate, txn->date, flt->maxdate) );
//	DB( g_print(" [%d] %s => %d (%d)\n", txn->account, txn->wording, insert, count) );
	return(insert);
}

