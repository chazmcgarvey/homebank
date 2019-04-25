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


/* = = = = = = = = = = = = = = = = */

void da_flt_free(Filter *flt)
{
	DB( g_print("da_flt_free\n") );
	if(flt != NULL)
	{
		g_free(flt->memo);
		g_free(flt->info);
		g_free(flt->tag);
		g_free(flt);
	}
}


Filter *da_flt_malloc(void)
{
	DB( g_print("da_flt_malloc\n") );
	return g_malloc0(sizeof(Filter));
}


/* = = = = = = = = = = = = = = = = = = = = */


void filter_status_acc_clear_except(Filter *flt, guint32 selkey)
{
GHashTableIter iter;
gpointer key, value;

	// set all account
	g_hash_table_iter_init (&iter, GLOBALS->h_acc);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Account *item = value;
		item->flt_select = item->key == selkey ? TRUE : FALSE;
	}


}


void filter_status_pay_clear_except(Filter *flt, guint32 selkey)
{
GHashTableIter iter;
gpointer key, value;

	// set all payee
	g_hash_table_iter_init (&iter, GLOBALS->h_pay);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Payee *item = value;
		item->flt_select = item->key == selkey ? TRUE : FALSE;
	}


}


void filter_status_cat_clear_except(Filter *flt, guint32 selkey)
{
GHashTableIter iter;
gpointer key, value;

	// set all payee
	g_hash_table_iter_init (&iter, GLOBALS->h_cat);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
	Category *item = value;
		item->flt_select = item->key == selkey ? TRUE : FALSE;
	}

}


/* = = = = = = = = = = = = = = = = */


void filter_reset(Filter *flt)
{
gint i;

	DB( g_print("\n[filter] default reset all %p\n", flt) );

	for(i=0;i<FILTER_MAX;i++)
	{
		flt->option[i] = 0;
	}

	flt->option[FILTER_DATE] = 1;
	flt->range  = FLT_RANGE_LAST12MONTHS;
	filter_preset_daterange_set(flt, flt->range, 0);

	for(i=0;i<NUM_PAYMODE_MAX;i++)
		flt->paymode[i] = TRUE;

	g_free(flt->info);
	g_free(flt->memo);
	g_free(flt->tag);
	flt->info = NULL;
	flt->memo = NULL;
	flt->tag = NULL;

	//unsaved
	flt->nbdaysfuture = 0;
	flt->type   = FLT_TYPE_ALL;
	flt->status = FLT_STATUS_ALL;
	flt->forceremind = PREFS->showremind;

	*flt->last_tab = '\0';
}


void filter_set_tag_by_id(Filter *flt, guint32 key)
{
Tag *tag;

	DB( g_print("\n[filter] set tag by id\n") );
	
	if(flt->tag)
	{
		g_free(flt->tag);
		flt->tag = NULL;
	}

	tag = da_tag_get(key);
	if(tag)
	{
		flt->tag = g_strdup(tag->name);
	}
}


static void filter_set_date_bounds(Filter *flt, guint32 kacc)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;

	DB( g_print("\n[filter] set date bounds %p\n", flt) );

	flt->mindate = 0;
	flt->maxdate = 0;

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		//#1674045 only rely on nosummary
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
		//changed 5.3
		//flt->mindate = HB_MINDATE;
		flt->mindate = GLOBALS->today - 365;
	
	if( flt->maxdate == 0 )
		//changed 5.3
		//flt->maxdate = HB_MAXDATE;
		flt->maxdate = GLOBALS->today + flt->nbdaysfuture;
	
	g_list_free(lst_acc);
}


void filter_preset_daterange_add_futuregap(Filter *filter, gint nbdays)
{

	DB( g_print("\n[filter] range add future gap\n") );
	
	//fixed > 5.1.7
	/*if( nbdays <= 0 )
	{
		filter->nbdaysfuture = 0;
		return;
	}*/

	filter->nbdaysfuture = 0;
	
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
GDate *tmpdate;
guint32 jtoday, jfiscal;
guint16 month, year, yfiscal, qnum;

	DB( g_print("\n[filter] daterange set %p %d\n", flt, range) );

	flt->range = range;
	
	jtoday = GLOBALS->today;

	tmpdate  = g_date_new_julian(jtoday);

	month = g_date_get_month(tmpdate);
	year  = g_date_get_year(tmpdate);
	DB( hb_print_date(jtoday , "today ") );

	g_date_set_dmy(tmpdate, PREFS->fisc_year_day, PREFS->fisc_year_month, year);
	jfiscal = g_date_get_julian(tmpdate);
	DB( hb_print_date(jfiscal, "fiscal") );

	yfiscal = (jtoday >= jfiscal) ? year : year-1;
	qnum = 0;

	if( range == FLT_RANGE_THISQUARTER || range == FLT_RANGE_LASTQUARTER )
	{
		g_date_set_dmy(tmpdate, PREFS->fisc_year_day, PREFS->fisc_year_month, yfiscal);
		while( (qnum < 5) && (g_date_get_julian(tmpdate) < jtoday) )
		{
			qnum++;
			g_date_add_months (tmpdate, 3);
		}
		DB( g_print(" qnum: %d\n", qnum ) );
	}
		
	switch( range )
	{
		case FLT_RANGE_THISMONTH:
		case FLT_RANGE_LASTMONTH:
			g_date_set_dmy(tmpdate, 1, month, year);
			if( range == FLT_RANGE_LASTMONTH )
				g_date_subtract_months(tmpdate, 1);
			flt->mindate = g_date_get_julian(tmpdate);
			month = g_date_get_month(tmpdate);
			year = g_date_get_year(tmpdate);
			g_date_add_days(tmpdate, g_date_get_days_in_month(month, year));
			flt->maxdate = g_date_get_julian(tmpdate) - 1;
			break;

		case FLT_RANGE_THISQUARTER:
		case FLT_RANGE_LASTQUARTER:
			g_date_set_dmy(tmpdate, PREFS->fisc_year_day, PREFS->fisc_year_month, yfiscal);
			if( range == FLT_RANGE_LASTQUARTER )
				g_date_subtract_months(tmpdate, 3);
			g_date_add_months(tmpdate, 3 * (qnum-1) );
			flt->mindate = g_date_get_julian(tmpdate);
			g_date_add_months(tmpdate, 3);
			flt->maxdate = g_date_get_julian(tmpdate) - 1;
			break;

		case FLT_RANGE_THISYEAR:
		case FLT_RANGE_LASTYEAR:
			g_date_set_dmy(tmpdate, PREFS->fisc_year_day, PREFS->fisc_year_month, yfiscal);
			if( range == FLT_RANGE_LASTYEAR )
				g_date_subtract_years(tmpdate, 1);
			flt->mindate = g_date_get_julian(tmpdate);
			g_date_add_years (tmpdate, 1);
			flt->maxdate = g_date_get_julian(tmpdate) - 1;
			break;

		case FLT_RANGE_LAST30DAYS:
			flt->mindate = jtoday - 30;
			flt->maxdate = jtoday;
			break;

		case FLT_RANGE_LAST60DAYS:
			flt->mindate = jtoday - 60;
			flt->maxdate = jtoday;
			break;

		case FLT_RANGE_LAST90DAYS:
			flt->mindate = jtoday - 90;
			flt->maxdate = jtoday;
			break;

		case FLT_RANGE_LAST12MONTHS:
			g_date_set_julian (tmpdate, jtoday);
			g_date_subtract_months(tmpdate, 12);
			flt->mindate = g_date_get_julian(tmpdate);
			flt->maxdate = jtoday;
			break;

		// case FLT_RANGE_OTHER:
			//nothing to do
			
		case FLT_RANGE_ALLDATE:
			filter_set_date_bounds(flt, kacc);
			break;
	}
	g_date_free(tmpdate);
}


void filter_preset_type_set(Filter *flt, gint type)
{

	DB( g_print("\n[filter] preset type set\n") );
	
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

	DB( g_print("\n[filter] preset status set\n") );
	
	/* any status */
	flt->status = status;
	flt->option[FILTER_STATUS] = 0;
	flt->option[FILTER_CATEGORY] = 0;
	flt->option[FILTER_PAYMODE] = 0;
	flt->reconciled = TRUE;
	flt->cleared  = TRUE;
	//#1602835 fautly set
	//flt->forceadd = TRUE;
	//flt->forcechg = TRUE;

	switch( status )
	{
		case FLT_STATUS_UNCATEGORIZED:
			flt->option[FILTER_CATEGORY] = 1;
			filter_status_cat_clear_except(flt, 0);
			flt->option[FILTER_PAYMODE] = 1;
			flt->paymode[PAYMODE_INTXFER] = FALSE;
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


gchar *filter_daterange_text_get(Filter *flt)
{
gchar buffer1[128];
gchar buffer2[128];
gchar buffer3[128];
GDate *date;
gchar *retval = NULL;

	DB( g_print("\n[filter] daterange text get\n") );
	
	date = g_date_new_julian(flt->mindate);
	g_date_strftime (buffer1, 128-1, PREFS->date_format, date);
	
	g_date_set_julian(date, flt->maxdate);
	g_date_strftime (buffer2, 128-1, PREFS->date_format, date);
	
	if( flt->nbdaysfuture > 0 )
	{
		g_date_set_julian(date, flt->maxdate + flt->nbdaysfuture);
		g_date_strftime (buffer3, 128-1, PREFS->date_format, date);
		retval = g_strdup_printf("%s — <s>%s</s> %s", buffer1, buffer2, buffer3);
	}
	else
		retval = g_strdup_printf("%s — %s", buffer1, buffer2);
	
	g_date_free(date);

	//return g_strdup_printf(_("<i>from</i> %s <i>to</i> %s — "), buffer1, buffer2);
	return retval;
}


/* used for quicksearch text into transaction */
gboolean filter_txn_search_match(gchar *needle, Transaction *txn, gint flags)
{
gboolean retval = FALSE;
Payee *payitem;
Category *catitem;
gchar *tags;

	DB( g_print("\n[filter] tnx search match\n") );
	
	if(flags & FLT_QSEARCH_MEMO)
	{
		//#1668036 always try match on txn memo first
		if(txn->memo)
		{
			retval |= hb_string_utf8_strstr(txn->memo, needle, FALSE);
		}
		if(retval) goto end;
		
		//#1509485
		if(txn->flags & OF_SPLIT)
		{
		guint count, i;
		Split *split;

			count = da_splits_length(txn->splits);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
		
				split = da_splits_get(txn->splits, i);
				tmpinsert = hb_string_utf8_strstr(split->memo, needle, FALSE);
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
			retval |= hb_string_utf8_strstr(txn->info, needle, FALSE);
		}
		if(retval) goto end;
	}

	if(flags & FLT_QSEARCH_PAYEE)
	{
		payitem = da_pay_get(txn->kpay);
		if(payitem)
		{
			retval |= hb_string_utf8_strstr(payitem->name, needle, FALSE);
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

			count = da_splits_length(txn->splits);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
				
				split = da_splits_get(txn->splits, i);
				catitem = da_cat_get(split->kcat);
				if(catitem)
				{
					tmpinsert = hb_string_utf8_strstr(catitem->fullname, needle, FALSE);
					retval |= tmpinsert;
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
				retval |= hb_string_utf8_strstr(catitem->fullname, needle, FALSE);
			}
		}
		if(retval) goto end;
	}
	
	if(flags & FLT_QSEARCH_TAGS)
	{
		tags = tags_tostring(txn->tags);
		if(tags)
		{
			retval |= hb_string_utf8_strstr(tags, needle, FALSE);
		}
		g_free(tags);
		if(retval) goto end;
	}

	//#1741339 add quicksearch for amount
	if(flags & FLT_QSEARCH_AMOUNT)
	{
	gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
	
		hb_strfnum(formatd_buf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->amount, txn->kcur, FALSE);
		retval |= hb_string_utf8_strstr(formatd_buf, needle, FALSE);
	}

	
end:
	return retval;
}


gint filter_txn_match(Filter *flt, Transaction *txn)
{
Account *accitem;
Payee *payitem;
Category *catitem;
gint insert;

	//DB( g_print("\n[filter] txn match\n") );

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
			insert = ( accitem->flt_select == TRUE ) ? 1 : 0;
			if(flt->option[FILTER_ACCOUNT] == 2) insert ^= 1;
		}
	}
	if(!insert) goto end;

/* payee */
	if(flt->option[FILTER_PAYEE]) {
		payitem = da_pay_get(txn->kpay);
		if(payitem)
		{
			insert = ( payitem->flt_select == TRUE ) ? 1 : 0;
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
			count = da_splits_length(txn->splits);
			for(i=0;i<count;i++)
			{
			gint tmpinsert = 0;
				
				split = da_splits_get(txn->splits, i);
				catitem = da_cat_get(split->kcat);
				if(catitem)
				{
					tmpinsert = ( catitem->flt_select == TRUE ) ? 1 : 0;
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
				insert = ( catitem->flt_select == TRUE ) ? 1 : 0;
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
				insert1 = hb_string_utf8_strstr(txn->info, flt->info, flt->exact);
			}
		}
		else
			insert1 = 1;

		if(flt->memo)
		{
			//#1668036 always try match on txn memo first
			if(txn->memo)
			{
				insert2 = hb_string_utf8_strstr(txn->memo, flt->memo, flt->exact);
			}

			if( (insert2 == 0) && (txn->flags & OF_SPLIT) )
			{
			guint count, i;
			Split *split;

				count = da_splits_length(txn->splits);
				for(i=0;i<count;i++)
				{
				gint tmpinsert = 0;
			
					split = da_splits_get(txn->splits, i);
					tmpinsert = hb_string_utf8_strstr(split->memo, flt->memo, flt->exact);
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
			tags = tags_tostring(txn->tags);
			if(tags)
			{
				insert3 = hb_string_utf8_strstr(tags, flt->tag, flt->exact);
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

