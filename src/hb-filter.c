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
		g_free(flt->memo);
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
	flt->mindate = HB_MINDATE;
	flt->maxdate = HB_MAXDATE;
}


static void filter_clear(Filter *flt)
{
guint i;

	for(i=0;i<FILTER_MAX;i++)
	{
		flt->option[i] = 0;
	}
	
	g_free(flt->info);
	g_free(flt->memo);
	g_free(flt->tag);
	flt->info = NULL;
	flt->memo = NULL;
	flt->tag = NULL;

	*flt->last_tab = '\0';
}


void filter_default_all_set(Filter *flt)
{
GHashTableIter iter;
gpointer key, value;
gint i;

	DB( g_print("(filter) reset %p\n", flt) );

	filter_clear(flt);

	flt->nbdaysfuture = 0;

	flt->range  = FLT_RANGE_LAST12MONTHS;
	flt->type   = FLT_TYPE_ALL;
	flt->status = FLT_STATUS_ALL;

	flt->forceremind = PREFS->showremind;

	flt->option[FILTER_DATE] = 1;
	filter_default_date_set(flt);

	for(i=0;i<NUM_PAYMODE_MAX;i++)
		flt->paymode[i] = TRUE;

	filter_preset_daterange_set(flt, flt->range, 0);

	// set all account
	g_hash_table_iter_init (&iter, GLOBALS->h_acc);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Account *item = value;
		item->filter = TRUE;
	}

	// set all payee
	g_hash_table_iter_init (&iter, GLOBALS->h_pay);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Payee *item = value;
		item->filter = TRUE;
	}

	// set all payee
	g_hash_table_iter_init (&iter, GLOBALS->h_cat);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Category *item = value;
		item->filter = TRUE;
	}

}


static void filter_set_date_bounds(Filter *flt, guint32 kacc)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;

	DB( g_print("(filter) set date bounds %p\n", flt) );

	flt->mindate = 0;
	flt->maxdate = 0;

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		//#1674045 ony rely on nosummary
		//if( !(acc->flags & AF_CLOSED) )
		{
		Transaction *txn;
		
			DB( g_print(" - do '%s'\n", acc->name) );

			lnk_txn = g_queue_peek_head_link(acc->txn_queue);
			if(lnk_txn) {
				txn = lnk_txn->data;
				if( (kacc == 0) || (txn->kacc == kacc) )
				{
					if( flt->mindate == 0 )
						flt->mindate = txn->date;
					else
						flt->mindate = MIN(flt->mindate, txn->date);
				}
			}

			lnk_txn = g_queue_peek_tail_link(acc->txn_queue);
			if(lnk_txn) {
				txn = lnk_txn->data;
				if( (kacc == 0) || (txn->kacc == kacc) )
				{
					if( flt->maxdate == 0 )
						flt->maxdate = txn->date;
					else
						flt->maxdate = MAX(flt->maxdate, txn->date);
				}
			}

		}
		lnk_acc = g_list_next(lnk_acc);
	}
	
	if( flt->mindate == 0 )
		flt->mindate = HB_MINDATE;
	
	if( flt->maxdate == 0 )
		flt->maxdate = HB_MAXDATE;	
	
	g_list_free(lst_acc);
}


void filter_preset_daterange_add_futuregap(Filter *filter, gint nbdays)
{

	if( nbdays <= 0 )
	{
		filter->nbdaysfuture = 0;
		return;
	}
	
	switch( filter->range )
	{
		case FLT_RANGE_THISMONTH:
		case FLT_RANGE_THISQUARTER:
		case FLT_RANGE_THISYEAR:
		case FLT_RANGE_LAST30DAYS:
		case FLT_RANGE_LAST60DAYS:
		case FLT_RANGE_LAST90DAYS:
		case FLT_RANGE_LAST12MONTHS:
			filter->nbdaysfuture = nbdays;
			break;
	}

}


void filter_preset_daterange_set(Filter *flt, gint range, guint32 kacc)
{
GDate *date;
guint32 refjuliandate, month, year, qnum;

	DB( g_print("(filter) daterange set %p %d\n", flt, range) );

	//filter_default_date_set(flt);
	filter_set_date_bounds(flt, kacc);

	flt->range = range;
	
	// by default refjuliandate is today
	// but we adjust if to max transaction date found
	// removed for 5.0.4
	refjuliandate = GLOBALS->today;
	/*if(flt->maxdate < refjuliandate)
		refjuliandate = flt->maxdate;*/

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
	flt->cleared  = TRUE;
	//#1602835 fautly set
	//flt->forceadd = TRUE;
	//flt->forcechg = TRUE;

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
			if(catitem != NULL)
				catitem->filter = TRUE;
			break;

		case FLT_STATUS_UNRECONCILED:
			flt->option[FILTER_STATUS] = 2;
			flt->reconciled = TRUE;
			flt->cleared = FALSE;
			break;

		case FLT_STATUS_UNCLEARED:
			flt->option[FILTER_STATUS] = 2;
			flt->reconciled = FALSE;
			flt->cleared = TRUE;
			break;

		case FLT_STATUS_RECONCILED:
			flt->option[FILTER_STATUS] = 1;
			flt->reconciled = TRUE;
			flt->cleared = FALSE;
			break;

		case FLT_STATUS_CLEARED:
			flt->option[FILTER_STATUS] = 1;
			flt->reconciled = FALSE;
			flt->cleared = TRUE;
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
		//#1668036 always try match on txn memo first
		if(txn->memo)
		{
			retval |= filter_text_compare(txn->memo, needle, FALSE);
		}
		if(retval) goto end;
		
		//#1509485
		if(txn->flags & OF_SPLIT)
		{
		guint count, i;
		Split *split;

			count = da_splits_count(txn->splits);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
		
				split = txn->splits[i];
				tmpinsert = filter_text_compare(split->memo, needle, FALSE);
				retval |= tmpinsert;
				if( tmpinsert )
					break;
			}
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
		//#1509485
		if(txn->flags & OF_SPLIT)
		{
		guint count, i;
		Split *split;

			count = da_splits_count(txn->splits);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
				
				split = txn->splits[i];
				catitem = da_cat_get(split->kcat);
				if(catitem)
				{
				gchar *fullname = da_cat_get_fullname (catitem);

					tmpinsert = filter_text_compare(fullname, needle, FALSE);
					retval |= tmpinsert;
					g_free(fullname);
				}

				if( tmpinsert )
					break;
			}
		}
		else
		{
			catitem = da_cat_get(txn->kcat);
			if(catitem)
			{
			gchar *fullname = da_cat_get_fullname (catitem);

				retval |= filter_text_compare(fullname, needle, FALSE);
				g_free(fullname);
			}
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
		if(retval) goto end;
	}

	//#1741339 add quicksearch for amount
	if(flags & FLT_QSEARCH_AMOUNT)
	{
	gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
	
		hb_strfnum(formatd_buf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->amount, txn->kcur, FALSE);
		retval |= filter_text_compare(formatd_buf, needle, FALSE);
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

	/* force display */
	if(flt->forceadd == TRUE && (txn->flags & OF_ADDED))
		goto end;

	if(flt->forcechg == TRUE && (txn->flags & OF_CHANGED))
		goto end;

	/* force remind if not filter on status */
	if(flt->forceremind == TRUE && (txn->status == TXN_STATUS_REMIND))
		goto end;

/* date */
	if(flt->option[FILTER_DATE]) {
		insert = ( (txn->date >= flt->mindate) && (txn->date <= (flt->maxdate + flt->nbdaysfuture) ) ) ? 1 : 0;
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
			count = da_splits_count(txn->splits);
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
			insert1 = ( txn->status == TXN_STATUS_RECONCILED ) ? 1 : 0;
		if(flt->cleared)
			insert2 = ( txn->status == TXN_STATUS_CLEARED ) ? 1 : 0;

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

/* info/memo/tag */
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

		if(flt->memo)
		{
			//#1668036 always try match on txn memo first
			if(txn->memo)
			{
				insert2 = filter_text_compare(txn->memo, flt->memo, flt->exact);
			}

			if( (insert2 == 0) && (txn->flags & OF_SPLIT) )
			{
			guint count, i;
			Split *split;

				count = da_splits_count(txn->splits);
				for(i=0;i<count;i++)
				{
				gint tmpinsert = 0;
			
					split = txn->splits[i];
					tmpinsert = filter_text_compare(split->memo, flt->memo, flt->exact);
					insert2 |= tmpinsert;
					if( tmpinsert )
						break;
				}
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
//	DB( g_print(" [%d] %s => %d (%d)\n", txn->account, txn->memo, insert, count) );
	return(insert);
}

