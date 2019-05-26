/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2019 Maxime DOYEN
 *
 *	This file is part of HomeBank.
 *
 *	HomeBank is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	HomeBank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */


#include "homebank.h"


/* = = = = = = = = = = = = = = = = = = = = */


gchar *RA_ARC_TYPE[] = { 
	N_("Scheduled"), 
	N_("Template"), 
	NULL
};


gchar *CYA_ASG_FIELD[] = { 
	N_("Memo"), 
	N_("Payee"), 
	NULL
};


gchar *CYA_CAT_TYPE[] = { 
	N_("Expense"), 
	N_("Income"), 
	NULL
};


gchar *CYA_CATSUBCAT[] = { 
	N_("Category"), 
	N_("Subcategory"), 
	NULL
};


gchar *nainex_label_names[] =
{
	N_("Inactive"),
	N_("Include"),
	N_("Exclude"),
	NULL
};




/* = = = = = = = = = = = = = = = = = = = = */


HbKvData CYA_ACC_TYPE[] = 
{
	{ ACC_TYPE_NONE,		N_("(no type)") },
	{ ACC_TYPE_BANK,		N_("Bank")	},
	{ ACC_TYPE_CASH,		N_("Cash")	},
	{ ACC_TYPE_ASSET,		N_("Asset")	},
	{ ACC_TYPE_CREDITCARD,	N_("Credit card") },
	{ ACC_TYPE_LIABILITY,	N_("Liability") },
//	{ ACC_TYPE_CHECKING, 	N_("Checkings") },
//	{ ACC_TYPE_SAVINGS, 	N_("Savings") },

//	{ ACC_TYPE_MUTUALFUND, 	N_("Mutual Fund") },
//	{ ACC_TYPE_INCOME, 		N_("Income") },
//	{ ACC_TYPE_EXPENSE, 	N_("Expense") },
//	{ ACC_TYPE_EQUITY, 		N_("Equity") },

	{ 0, NULL }
};


HbKvData CYA_FLT_RANGE[] = {
	{ FLT_RANGE_THISMONTH,		N_("This month") },
	{ FLT_RANGE_LASTMONTH,		N_("Last month") },
	{ FLT_RANGE_THISQUARTER,	N_("This quarter") },
	{ FLT_RANGE_LASTQUARTER,	N_("Last quarter") },
	{ FLT_RANGE_THISYEAR,		N_("This year") },
	{ FLT_RANGE_LASTYEAR,		N_("Last year") },
	{ HBTK_IS_SEPARATOR, "" },
	{ FLT_RANGE_LAST30DAYS,		N_("Last 30 days") },
	{ FLT_RANGE_LAST60DAYS,		N_("Last 60 days") },
	{ FLT_RANGE_LAST90DAYS,		N_("Last 90 days") },
	{ FLT_RANGE_LAST12MONTHS,	N_("Last 12 months") },
	{ HBTK_IS_SEPARATOR, "" },
//	{ FLT_RANGE_OTHER,			N_("Other...") },
	{ FLT_RANGE_OTHER,			N_("custom") },
	{ HBTK_IS_SEPARATOR, "" },
	{ FLT_RANGE_ALLDATE,		N_("All date") },
	{ 0, NULL }
};


gchar *CYA_ARC_UNIT[] = {
	N_("Day"), 
	N_("Week"), 
	N_("Month"), 
	N_("Year"), 
	NULL
};


gchar *RA_ARC_WEEKEND[] = { 
	N_("Possible"), 
	N_("Before"), 
	N_("After"), 
	NULL
};


gchar *CYA_KIND[] = {
	N_("Exp. & Inc."),
	N_("Expense"),
	N_("Income"),
	NULL
};


gchar *CYA_FLT_TYPE[] = {
	N_("Expense"),
	N_("Income"),
	"",
	N_("Any Type"),
	NULL
};

gchar *CYA_FLT_STATUS[] = {
	N_("Uncategorized"),
	N_("Unreconciled"),
	N_("Uncleared"),
	N_("Reconciled"),
	N_("Cleared"),
	"",
	N_("Any Status"),
	NULL
};

/*gchar *OLD_CYA_FLT_RANGE[] = {
	N_("This month"),
	N_("Last month"),
	N_("This quarter"),
	N_("Last quarter"),
	N_("This year"),
	N_("Last year"),
	"",
	N_("Last 30 days"),
	N_("Last 60 days"),
	N_("Last 90 days"),
	N_("Last 12 months"),
	"",
	N_("Other..."),
	"",
	N_("All date"),
	NULL
};*/

//ui_filter.c only
gchar *CYA_SELECT[] =
{
	"----",
	N_("All month"),
	N_("January"),
	N_("February"),
	N_("March"),
	N_("April"),
	N_("May"),
	N_("June"),
	N_("July"),
	N_("August"),
	N_("September"),
	N_("October"),
	N_("November"),
	N_("December"),
	NULL
};


/* = = = = = = = = = = = = = = = = = = = = */

//in prefs.c only
gchar *CYA_MONTHS[] =
{
	N_("January"),
	N_("February"),
	N_("March"),
	N_("April"),
	N_("May"),
	N_("June"),
	N_("July"),
	N_("August"),
	N_("September"),
	N_("October"),
	N_("November"),
	N_("December"),
	NULL
};


//hb_report.c rep_time.c ui_budget
gchar *CYA_ABMONTHS[] =
{
	NULL,
	N_("Jan"),
	N_("Feb"),
	N_("Mar"),
	N_("Apr"),
	N_("May"),
	N_("Jun"),
	N_("Jul"),
	N_("Aug"),
	N_("Sep"),
	N_("Oct"),
	N_("Nov"),
	N_("Dec"),
	NULL
};
