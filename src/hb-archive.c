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
#include "hb-archive.h"
#include "hb-split.h"

#include "ext.h"
#include "refcount.h"

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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
da_archive_clean(Archive *item)
{
	if(item != NULL)
	{
		if(item->memo != NULL)
		{
			g_free(item->memo);
			item->memo = NULL;
		}





		//5.3 added as it was a leak
		if(item->tags != NULL)
		{
			g_free(item->tags);
			item->tags = NULL;
		}
		if(item->splits != NULL)
		{
			da_split_destroy(item->splits);
			item->splits = NULL;
			item->flags &= ~(OF_SPLIT); //Flag that Splits are cleared
		}
	}
}


void da_archive_free(Archive *item)
{
	if(rc_unref(item))
	{
		da_archive_clean(item);
		rc_free(item);
	}
}


Archive *da_archive_malloc(void)
{
Archive *item;

	item = rc_alloc(sizeof(Archive));
	item->key = 1;
	return item;
}


Archive *da_archive_clone(Archive *src_item)
{
Archive *new_item = rc_dup(src_item, sizeof(Archive));

	if(new_item)
	{
		//duplicate the string
		new_item->memo = g_strdup(src_item->memo);

		//duplicate tags
		//no g_free here to avoid free the src tags (memdup copie dthe ptr)
		new_item->tags = tags_clone(src_item->tags);

		//duplicate splits
		//no g_free here to avoid free the src tags (memdup copie dthe ptr)
		new_item->splits = da_splits_clone(src_item->splits);
		if( da_splits_length (new_item->splits) > 0 )
			new_item->flags |= OF_SPLIT; //Flag that Splits are active
	}
	return new_item;
}


void da_archive_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Archive *item = tmplist->data;
		da_archive_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static gint da_archive_glist_compare_func(Archive *a, Archive *b)
{
	return hb_string_utf8_compare(a->memo, b->memo);
}


GList *da_archive_sort(GList *list)
{
	return g_list_sort(list, (GCompareFunc)da_archive_glist_compare_func);
}


guint da_archive_length(void)
{
	return g_list_length(GLOBALS->arc_list);
}


/* append a fav with an existing key (from xml file only) */
gboolean
da_archive_append(Archive *item)
{
	GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, item);
	return TRUE;
}


gboolean
da_archive_append_new(Archive *item)
{
	item->key = da_archive_get_max_key() + 1;
	GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, item);
	return TRUE;
}


guint32
da_archive_get_max_key(void)
{
GList *tmplist = g_list_first(GLOBALS->arc_list);
guint32 max_key = 0;

	while (tmplist != NULL)
	{
	Archive *item = tmplist->data;

		max_key = MAX(item->key, max_key);		
		tmplist = g_list_next(tmplist);
	}
	
	return max_key;
}


Archive *
da_archive_get(guint32 key)
{
GList *tmplist;
Archive *retval = NULL;

	tmplist = g_list_first(GLOBALS->arc_list);
	while (tmplist != NULL)
	{
	Archive *item = tmplist->data;

		if(item->key == key)
		{
			retval = item;
			break;
		}
		tmplist = g_list_next(tmplist);
	}
	return retval;
}


void da_archive_consistency(Archive *item)
{
Account *acc;
Category *cat;
Payee *pay;
guint nbsplit;

	// check category exists
	cat = da_cat_get(item->kcat);
	if(cat == NULL)
	{
		g_warning("arc consistency: fixed invalid cat %d", item->kcat);
		item->kcat = 0;
		GLOBALS->changes_count++;
	}

	//#1340142 check split category 	
	if( item->splits != NULL )
	{
		nbsplit = da_splits_consistency(item->splits);
		//# 1416624 empty category when split
		if(nbsplit > 0 && item->kcat > 0)
		{
			g_warning("txn consistency: fixed invalid cat on split txn");
			item->kcat = 0;
			GLOBALS->changes_count++;
		}
	}
	
	// check payee exists
	pay = da_pay_get(item->kpay);
	if(pay == NULL)
	{
		g_warning("arc consistency: fixed invalid pay %d", item->kpay);
		item->kpay = 0;
		GLOBALS->changes_count++;
	}

	// reset dst acc for non xfer transaction
	if( item->paymode != PAYMODE_INTXFER )
		item->kxferacc = 0;

	// delete automation if dst_acc not exists
	if(item->paymode == PAYMODE_INTXFER)
	{
		acc = da_acc_get(item->kxferacc);
		if(acc == NULL)
		{
			item->flags &= ~(OF_AUTO);	//delete flag
		}
	}

}

/* = = = = = = = = = = = = = = = = = = = = */

Archive *da_archive_init_from_transaction(Archive *arc, Transaction *txn)
{
	DB( g_print("\n[scheduled] init from txn\n") );

	//fill it
	arc->amount		= txn->amount;
	arc->kacc		= txn->kacc;
	arc->kxferacc	= txn->kxferacc;
	arc->paymode	= txn->paymode;
	arc->flags		= txn->flags	& (OF_INCOME);
	arc->status		= txn->status;
	arc->kpay		= txn->kpay;
	arc->kcat		= txn->kcat;
	if(txn->memo != NULL)
		arc->memo = g_strdup(txn->memo);
	else
		arc->memo = g_strdup(_("(new archive)"));

	arc->tags    = tags_clone(txn->tags);
	arc->splits  = da_splits_clone(txn->splits);
	if( da_splits_length (arc->splits) > 0 )
		arc->flags |= OF_SPLIT; //Flag that Splits are active
	
	return arc;
}


static guint32 _sched_date_get_next_post(GDate *tmpdate, Archive *arc, guint32 nextdate)
{
guint32 nextpostdate = nextdate;

	//DB( g_print("\n[scheduled] date_get_next_post\n") );

	g_date_set_julian(tmpdate, nextpostdate);

	//DB( g_print("in : %2d-%2d-%4d\n", g_date_get_day(tmpdate), g_date_get_month (tmpdate), g_date_get_year(tmpdate) ) );

	switch(arc->unit)
	{
		case AUTO_UNIT_DAY:
			g_date_add_days(tmpdate, arc->every);
			break;
		case AUTO_UNIT_WEEK:
			g_date_add_days(tmpdate, 7 * arc->every);
			break;
		case AUTO_UNIT_MONTH:
			g_date_add_months(tmpdate, arc->every);
			break;
		case AUTO_UNIT_YEAR:
			g_date_add_years(tmpdate, arc->every);
			break;
	}

	//DB( g_print("out: %2d-%2d-%4d\n", g_date_get_day(tmpdate), g_date_get_month (tmpdate), g_date_get_year(tmpdate) ) );


	/* get the final post date and free */
	nextpostdate = g_date_get_julian(tmpdate);
	
	return nextpostdate;
}


gboolean scheduled_is_postable(Archive *arc)
{
gdouble value;

	value = hb_amount_round(arc->amount, 2);
	if( (arc->flags & OF_AUTO) && (arc->kacc > 0) && (value != 0.0) )
		return TRUE;

	return FALSE;
}


guint32 scheduled_get_postdate(Archive *arc, guint32 postdate)
{
GDate *tmpdate;
GDateWeekday wday;
guint32 finalpostdate;
gint shift;

	DB( g_print("\n[scheduled] get_postdate\n") );


	finalpostdate = postdate;
	
	tmpdate = g_date_new_julian(finalpostdate);
	/* manage weekend exception */
	if( arc->weekend > 0 )
	{
		wday = g_date_get_weekday(tmpdate);

		DB( g_print(" %s wday=%d\n", arc->memo, wday) );

		if( wday >= G_DATE_SATURDAY )
		{
			switch(arc->weekend)
			{
				case 1: /* shift before : sun 7-5=+2 , sat 6-5=+1 */
					shift = wday - G_DATE_FRIDAY;
					DB( g_print("sub=%d\n", shift) );
					g_date_subtract_days (tmpdate, shift);
					break;

				case 2: /* shift after : sun 8-7=1 , sat 8-6=2 */
					shift = 8 - wday;
					DB( g_print("add=%d\n", shift) );
					g_date_add_days (tmpdate, shift);
					break;
			}
		}
	}
	
	/* get the final post date and free */
	finalpostdate = g_date_get_julian(tmpdate);
	g_date_free(tmpdate);
	
	return finalpostdate;
}


guint32 scheduled_get_latepost_count(Archive *arc, guint32 jrefdate)
{
GDate *post_date;
guint32 curdate;
guint32 nblate = 0;

	//DB( g_print("\n[scheduled] get_latepost_count\n") );

	/*
	curdate = jrefdate - arc->nextdate;
	switch(arc->unit)
	{
		case AUTO_UNIT_DAY:
			nbpost = (curdate / arc->every);
			g_print("debug d: %d => %f\n", curdate, nbpost);
			break;

		case AUTO_UNIT_WEEK:
			nbpost = (curdate / ( 7 * arc->every));
			g_print("debug w: %d => %f\n", curdate, nbpost);
			break;

		case AUTO_UNIT_MONTH:
			//approximate is sufficient
			nbpost = (curdate / (( 365.2425 / 12) * arc->every));
			g_print("debug m: %d => %f\n", curdate, nbpost);
			break;

		case AUTO_UNIT_YEAR:
			//approximate is sufficient
			nbpost = (curdate / ( 365.2425 * arc->every));
			g_print("debug y: %d => %f\n", curdate, nbpost);
			break;
	}

	nblate = floor(nbpost);

	if(arc->flags & OF_LIMIT)
		nblate = MIN(nblate, arc->limit);
	
	nblate = MIN(nblate, 11);
	*/
	

	// pre 5.1 way
	post_date = g_date_new();
	curdate = arc->nextdate;
	while(curdate <= jrefdate)
	{
		curdate = _sched_date_get_next_post(post_date, arc, curdate);
		nblate++;
		// break if over limit or at 11 max (to display +10)
		if( nblate >= 11 || ( (arc->flags & OF_LIMIT) && (nblate >= arc->limit) ) )
			break;
	}

	//DB( g_print(" nblate=%d\n", nblate) );

	g_date_free(post_date);

	return nblate;
}


/* return 0 is max number of post is reached */
guint32 scheduled_date_advance(Archive *arc)
{
GDate *post_date;
gushort lastday;

	DB( g_print("\n[scheduled] date_advance\n") );

	DB( g_print(" arc: '%s'\n", arc->memo ) );

	post_date = g_date_new();
	g_date_set_julian(post_date, arc->nextdate);
	// saved the current day number
	lastday = g_date_get_day(post_date);

	arc->nextdate = _sched_date_get_next_post(post_date, arc, arc->nextdate);

	DB( g_print(" raw next post date: %2d-%2d-%4d\n", g_date_get_day(post_date), g_date_get_month (post_date), g_date_get_year(post_date) ) );

	//for day > 28 we might have a gap to compensate later
	if( (arc->unit==AUTO_UNIT_MONTH) || (arc->unit==AUTO_UNIT_YEAR) )
	{
		if( lastday >= 28 )
		{
			DB( g_print(" lastday:%d, daygap:%d\n", lastday, arc->daygap) );
			if( arc->daygap > 0 )
			{
				g_date_add_days (post_date, arc->daygap);
				arc->nextdate = g_date_get_julian (post_date);
				lastday += arc->daygap;
				DB( g_print(" adjusted post date: %2d-%2d-%4d\n", g_date_get_day(post_date), g_date_get_month (post_date), g_date_get_year(post_date) ) );
			}

			arc->daygap = CLAMP(lastday - g_date_get_day(post_date), 0, 3);
		
			DB( g_print(" daygap is %d\n", arc->daygap) );
		}
		else
			arc->daygap = 0;
	}


	//#1556289
	/* check limit, update and maybe break */
	if(arc->flags & OF_LIMIT)
	{
		arc->limit--;
		if(arc->limit <= 0)
		{
			arc->flags ^= (OF_LIMIT | OF_AUTO);	// invert flags
			arc->nextdate = 0;
		}
	}

	g_date_free(post_date);

	return arc->nextdate;
}


/*
 *  return the maximum date a scheduled txn can be posted to
 */
guint32 scheduled_date_get_post_max(void)
{
guint nbdays;
GDate *today, *maxdate;

	DB( g_print("\n[scheduled] date_get_post_max\n") );

	//add until xx of the next month (excluded)
	if(GLOBALS->auto_smode == 0)	
	{
		DB( g_print(" - max is %d of next month\n", GLOBALS->auto_weekday) );
		
		today = g_date_new_julian(GLOBALS->today);

		//we compute user xx weekday of next month
		maxdate = g_date_new_julian(GLOBALS->today);
		g_date_set_day(maxdate, GLOBALS->auto_weekday);
		if(g_date_get_day (today) >= GLOBALS->auto_weekday)
			g_date_add_months(maxdate, 1);
		
		nbdays = g_date_days_between(today, maxdate);
	
		g_date_free(maxdate);
		g_date_free(today);
	}
	else
	{
		nbdays = GLOBALS->auto_nbdays;
	}

	DB( hb_print_date(GLOBALS->today, "today") );
	DB( g_print(" - %d nbdays\n", nbdays) );
	DB( hb_print_date(GLOBALS->today + nbdays, "maxpostdate") );

	return GLOBALS->today + nbdays;
}


gint scheduled_post_all_pending(void)
{
GList *list;
gint count;
guint32 maxpostdate;
Transaction *txn;

	DB( g_print("\n[scheduled] post_all_pending\n") );

	count = 0;

	maxpostdate = scheduled_date_get_post_max();

	txn = da_transaction_malloc();
	
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *arc = list->data;

		DB( g_print("\n eval %d for '%s'\n", scheduled_is_postable(arc), arc->memo) );

		if(scheduled_is_postable(arc) == TRUE)
		{
			DB( g_print(" - every %d limit %d (to %d)\n", arc->every, arc->flags & OF_LIMIT, arc->limit) );
			DB( hb_print_date(arc->nextdate, "next post") );

			if(arc->nextdate < maxpostdate)
			{
			guint32 mydate = arc->nextdate;

				while(mydate < maxpostdate)
				{
					DB( hb_print_date(mydate, arc->memo) );
					
					da_transaction_init_from_template(txn, arc);
					txn->date = scheduled_get_postdate(arc, mydate);
					/* todo: ? fill in cheque number */

					transaction_add(NULL, txn);
					GLOBALS->changes_count++;
					count++;

					da_transaction_clean(txn);

					mydate = scheduled_date_advance(arc);

					//DB( hb_print_date(mydate, "next on") );

					if(mydate == 0)
						goto nextarchive;
				}

			}
		}
nextarchive:
		list = g_list_next(list);
	}

	da_transaction_free (txn);
	
	return count;
}

