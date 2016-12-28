/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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


/* = = = = = = = = = = = = = = = = = = = = */
/* Archive */

Archive *da_archive_malloc(void)
{
	return g_malloc0(sizeof(Archive));
}

Archive *da_archive_clone(Archive *src_item)
{
Archive *new_item = g_memdup(src_item, sizeof(Archive));

	if(new_item)
	{
		//duplicate the string
		new_item->wording = g_strdup(src_item->wording);
		
		if( da_splits_clone(src_item->splits, new_item->splits) > 0)
			new_item->flags |= OF_SPLIT; //Flag that Splits are active
	}
	return new_item;
}

void da_archive_free(Archive *item)
{
	if(item != NULL)
	{
		if(item->wording != NULL)
			g_free(item->wording);

		da_splits_free(item->splits);
		//item->flags &= ~(OF_SPLIT); //Flag that Splits are cleared		
		
		g_free(item);
	}
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

static gint da_archive_glist_compare_func(Archive *a, Archive *b)
{
	return hb_string_utf8_compare(a->wording, b->wording);
}


GList *da_archive_sort(GList *list)
{
	return g_list_sort(list, (GCompareFunc)da_archive_glist_compare_func);
}

guint da_archive_length(void)
{
	return g_list_length(GLOBALS->arc_list);
}

void da_archive_consistency(Archive *item)
{
Account *acc;
Category *cat;
Payee *pay;

	// check category exists
	cat = da_cat_get(item->kcat);
	if(cat == NULL)
	{
		g_warning("arc consistency: fixed invalid cat %d", item->kcat);
		item->kcat = 0;
		GLOBALS->changes_count++;
	}
	
	split_cat_consistency(item->splits);
	
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
	//fill it
	arc->amount		= txn->amount;
	arc->kacc		= txn->kacc;
	arc->kxferacc	= txn->kxferacc;
	arc->paymode		= txn->paymode;
	arc->flags			= txn->flags	& (OF_INCOME);
	arc->status		= txn->status;
	arc->kpay			= txn->kpay;
	arc->kcat		= txn->kcat;
	if(txn->wording != NULL)
		arc->wording 		= g_strdup(txn->wording);
	else
		arc->wording 		= g_strdup(_("(new archive)"));

	if( da_splits_clone(txn->splits, arc->splits) > 0)
		arc->flags |= OF_SPLIT; //Flag that Splits are active
	
	return arc;
}




static guint32 _sched_date_get_next_post(Archive *arc, guint32 nextdate)
{
GDate *tmpdate;
guint32 nextpostdate = nextdate;

	DB( g_print("\n[scheduled] _sched_date_get_next_post\n") );

	tmpdate = g_date_new_julian(nextpostdate);
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

	/* get the final post date and free */
	nextpostdate = g_date_get_julian(tmpdate);
	g_date_free(tmpdate);
	
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

	DB( g_print("\n[scheduled] scheduled_get_postdate\n") );


	finalpostdate = postdate;
	
	tmpdate = g_date_new_julian(finalpostdate);
	/* manage weekend exception */
	if( arc->weekend > 0 )
	{
		wday = g_date_get_weekday(tmpdate);

		DB( g_print(" %s wday=%d\n", arc->wording, wday) );

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
guint32 curdate = jrefdate - arc->nextdate;
guint32 nblate = 0;

	/*
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
	curdate = arc->nextdate;
	while(curdate <= jrefdate)
	{
		curdate = _sched_date_get_next_post(arc, curdate);
		nblate++;
		// break if over limit or at 11 max (to display +10)
		if(nblate >= arc->limit || nblate >= 11)
			break;
	}

	return nblate;
}


/* return 0 is max number of post is reached */
guint32 scheduled_date_advance(Archive *arc)
{
	arc->nextdate = _sched_date_get_next_post(arc, arc->nextdate);
	
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

		DB( g_print("\n eval %d for '%s'\n", scheduled_is_postable(arc), arc->wording) );

		if(scheduled_is_postable(arc) == TRUE)
		{
			DB( g_print(" - every %d limit %d (to %d)\n", arc->every, arc->flags & OF_LIMIT, arc->limit) );
			DB( hb_print_date(arc->nextdate, "next post") );

			if(arc->nextdate < maxpostdate)
			{
			guint32 mydate = arc->nextdate;

				while(mydate < maxpostdate)
				{
					DB( hb_print_date(mydate, arc->wording) );
					
					da_transaction_init_from_template(txn, arc);
					txn->date = scheduled_get_postdate(arc, mydate);
					/* todo: ? fill in cheque number */

					transaction_add(txn, NULL, 0);
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

