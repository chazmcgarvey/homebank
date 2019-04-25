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
#include "hb-report.h"

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


extern gchar *CYA_ABMONTHS[];

/* = = = = = = = = = = = = = = = = = = = = */
/* CarCost */

void da_vehiclecost_free(CarCost *item)
{
	if(item != NULL)
	{
		g_free(item);
	}
}


CarCost *da_vehiclecost_malloc(void)
{
	return g_malloc0(sizeof(CarCost));
}

void da_vehiclecost_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	CarCost *item = tmplist->data;
		da_vehiclecost_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


/* = = = = = = = = = = = = = = = = = = = = */


/*
** return the month list position correponding to the passed date
*/
static guint DateInMonth(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
guint pos;

	//todo
	// this return sometimes -1, -2 which is wrong

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	pos = ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1);

	//g_print(" from=%d-%d ope=%d-%d => %d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos);

	g_date_free(date2);
	g_date_free(date1);

	return(pos);
}



//for fiscal sub gap between 1st fiscal and 1/1/year

//int quarterNumber = (date.Month-1)/3+1;
//DateTime firstDayOfQuarter = new DateTime(date.Year, (quarterNumber-1)*3+1,1);
//DateTime lastDayOfQuarter = firstDayOfQuarter.AddMonths(3).AddDays(-1);

static guint DateInQuarter(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
guint quarter, pos;

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	//#1758532 shift to first quarter day of 'from date' 
	quarter = ((g_date_get_month(date1)-1)/3)+1;
	DB( g_print("-- from=%02d/%d :: Q%d\n", g_date_get_month(date1), g_date_get_year(date1), quarter) );
	g_date_set_day(date1, 1);
	g_date_set_month(date1, ((quarter-1)*3)+1);

	pos = (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/3;

	DB( g_print("-- from=%02d/%d ope=%02d/%d => pos=%d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos) );

	g_date_free(date2);
	g_date_free(date1);

	return(pos);
}


static guint DateInHalfYear(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
guint hyear, pos;

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	// shift to first half year of 'from date'
	hyear = ((g_date_get_month(date1)-1)/6)+1;
	DB( g_print("-- from=%02d/%d :: Q%d\n", g_date_get_month(date1), g_date_get_year(date1), hyear) );
	g_date_set_day(date1, 1);
	g_date_set_month(date1, ((hyear-1)*6)+1);
	
	pos = (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/6;

	DB( g_print(" from=%d-%d ope=%d-%d => %d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos) );

	g_date_free(date2);
	g_date_free(date1);
	
	return(pos);
}


/*
** return the year list position correponding to the passed date
*/
static guint DateInYear(guint32 from, guint32 opedate)
{
GDate *date;
guint year_from, year_ope, pos;

	date = g_date_new_julian(from);
	year_from = g_date_get_year(date);

	g_date_set_julian(date, opedate);
	year_ope = g_date_get_year(date);
	g_date_free(date);

	pos = year_ope - year_from;

	//g_print(" from=%d ope=%d => %d\n", year_from, year_ope, pos);

	return(pos);
}


gint report_items_count(gint src, guint32 jfrom, guint32 jto)
{
GDate *date1, *date2;
gint nbsrc = 0;

	switch(src)
	{
		case REPORT_SRC_CATEGORY:
		case REPORT_SRC_SUBCATEGORY:
			nbsrc = da_cat_get_max_key() + 1;
			break;
		case REPORT_SRC_PAYEE:
			nbsrc = da_pay_get_max_key() + 1;
			break;
		case REPORT_SRC_ACCOUNT:
			nbsrc = da_acc_get_max_key() + 1;
			break;
		case REPORT_SRC_TAG:
			nbsrc = da_tag_length();
			break;
		case REPORT_SRC_MONTH:
			date1 = g_date_new_julian(jfrom);
			date2 = g_date_new_julian(jto);
			nbsrc = ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1) + 1;
			g_date_free(date2);
			g_date_free(date1);
			break;
		case REPORT_SRC_YEAR:
			date1 = g_date_new_julian(jfrom);
			date2 = g_date_new_julian(jto);
			nbsrc = g_date_get_year(date2) - g_date_get_year(date1) + 1;
			g_date_free(date2);
			g_date_free(date1);
			break;
	}

	return nbsrc;
}



gint report_items_get_pos(gint tmpsrc, guint jfrom, Transaction *ope)
{
gint pos = 0;

	switch(tmpsrc)
	{
		case REPORT_SRC_CATEGORY:
			pos = category_report_id(ope->kcat, FALSE);
			break;
		case REPORT_SRC_SUBCATEGORY:
			pos = ope->kcat;
			break;
		case REPORT_SRC_PAYEE:
			pos = ope->kpay;
			break;
		case REPORT_SRC_ACCOUNT:
			pos = ope->kacc;
			break;
		case REPORT_SRC_MONTH:
			pos = DateInMonth(jfrom, ope->date);
			break;
		case REPORT_SRC_YEAR:
			pos = DateInYear(jfrom, ope->date);
			break;
	}
	return pos;
}


gint report_interval_get_pos(gint intvl, guint jfrom, Transaction *ope)
{
gint pos = 0;

	switch(intvl)
	{
		case REPORT_INTVL_DAY:
			pos = ope->date - jfrom;
			break;
		case REPORT_INTVL_WEEK:
			pos = (ope->date - jfrom)/7;
			break;
		case REPORT_INTVL_MONTH:
			pos = DateInMonth(jfrom, ope->date);
			break;
		case REPORT_INTVL_QUARTER:
			pos = DateInQuarter(jfrom, ope->date);
			break;
		case REPORT_INTVL_HALFYEAR:
			pos = DateInHalfYear(jfrom, ope->date);
			break;
		case REPORT_INTVL_YEAR:
			pos = DateInYear(jfrom, ope->date);
			break;
	}
	
	return pos;
}



gint report_interval_count(gint intvl, guint32 jfrom, guint32 jto)
{
GDate *date1, *date2;
gint nbintvl = 0;

	date1 = g_date_new_julian(jfrom);
	date2 = g_date_new_julian(jto);
	
	switch(intvl)
	{
		case REPORT_INTVL_DAY:
			nbintvl = 1 + (jto - jfrom);
			break;
		case REPORT_INTVL_WEEK:
			nbintvl = 1 + ((jto - jfrom) / 7);
			break;
		case REPORT_INTVL_MONTH:
			nbintvl = 1 + ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1);
			break;
		case REPORT_INTVL_QUARTER:
			nbintvl = 1 + (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/3;
			break;
		case REPORT_INTVL_HALFYEAR:
			nbintvl = 1 + (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/6;
			break;
		case REPORT_INTVL_YEAR:
			nbintvl = 1 + g_date_get_year(date2) - g_date_get_year(date1);
			break;
	}

	g_date_free(date2);
	g_date_free(date1);
	
	return nbintvl;
}


void report_interval_snprint_name(gchar *s, gint slen, gint intvl, guint32 jfrom, gint idx)
{
GDate *date = g_date_new_julian(jfrom);
	
	switch(intvl)
	{
		case REPORT_INTVL_DAY:
			g_date_add_days(date, idx);
			g_date_strftime (s, slen, PREFS->date_format, date);
			break;

		case REPORT_INTVL_WEEK:
			g_date_add_days(date, idx*7);
			//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
			//TRANSLATORS: printf string for year of week W, ex. 2019-W52 for week 52 of 2019
			g_snprintf(s, slen, _("%d-w%d"), g_date_get_year(date), g_date_get_monday_week_of_year(date));
			break;

		case REPORT_INTVL_MONTH:
			g_date_add_months(date, idx);
			//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
			g_snprintf(s, slen, "%d-%s", g_date_get_year(date), _(CYA_ABMONTHS[g_date_get_month(date)]));
			break;

		case REPORT_INTVL_QUARTER:
			g_date_add_months(date, idx*3);
			//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
			//todo: will be innacurrate here if fiscal year start not 1/jan
			//TRANSLATORS: printf string for year of quarter Q, ex. 2019-Q4 for quarter 4 of 2019
			g_snprintf(s, slen, _("%d-q%d"), g_date_get_year(date), ((g_date_get_month(date)-1)/3)+1);
			break;

		case REPORT_INTVL_HALFYEAR:
			g_date_add_months(date, idx*6);
			g_snprintf(s, slen, "%d-%s", g_date_get_year(date), g_date_get_month(date) < 7 ? "h1" : "h2");
			break;

		case REPORT_INTVL_YEAR:
			g_date_add_years(date, idx);
			g_snprintf(s, slen, "%d", g_date_get_year(date));
			break;
		default:
			*s ='\0';
			break;
	}

	g_date_free(date);
}


//TODO: maybe migrate this to filter as well
//#1562372 in case of a split we need to take amount for filter categories only
gdouble report_txn_amount_get(Filter *flt, Transaction *txn)
{
gdouble amount;

	amount = txn->amount;

	if( flt->option[FILTER_CATEGORY] > 0 )	//not inactive
	{
		if( txn->flags & OF_SPLIT )
		{
		guint i, nbsplit = da_splits_length(txn->splits);
		Split *split;
		Category *catentry;
		gint sinsert;

			amount = 0.0;

			for(i=0;i<nbsplit;i++)
			{
				split = da_splits_get(txn->splits, i);
				catentry = da_cat_get(split->kcat);
				if(catentry == NULL) continue;
				sinsert = ( catentry->flt_select == TRUE ) ? 1 : 0;
				if(flt->option[FILTER_CATEGORY] == 2) sinsert ^= 1;

				DB( g_print(" split '%s' insert=%d\n",catentry->name, sinsert) );

				if( (flt->option[FILTER_CATEGORY] == 0) || sinsert)
				{
					amount += split->amount;
				}
			}

		}
	}
	return amount;
}




