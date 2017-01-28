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


#include "homebank.h"

#include "hb-transaction.h"
#include "hb-xml.h"

#include "ui-dialogs.h"

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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


// v0.1 to v0.2 : we must change account reference by making a +1 to its index references
static void homebank_upgrade_to_v02(void)
{
GList *lst_acc, *lnk_acc;
GList *list;
GHashTable *h_old_acc;
	

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v02\n") );

	//keep old hashtable with us
	h_old_acc = GLOBALS->h_acc;
	da_acc_new();

	lst_acc = g_hash_table_get_values(h_old_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		acc->key++;
		acc->pos++;
		da_acc_insert (acc);

		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;
			entry->kacc++;
			entry->kxferacc++;
			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	//we loose some small memory here
	g_hash_table_steal_all(h_old_acc);

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		entry->kacc++;
		entry->kxferacc++;
		list = g_list_next(list);
	}
}

// v0.2 to v0.3 : we must assume categories exists : bugs 303886, 303738
static void homebank_upgrade_to_v03(void)
{
GList *lst_acc, *lnk_acc;
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v03\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;

			da_transaction_consistency(entry);
			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;

		da_archive_consistency(entry);
		list = g_list_next(list);
	}
}

static void homebank_upgrade_to_v04(void)
{
	DB( g_print("\n[hb-xml] homebank_upgrade_to_v04\n") );

	GLOBALS->arc_list = da_archive_sort(GLOBALS->arc_list);
}


// v0.4 to v0.5 :
// we must assume kxferacc exists in archives for internal xfer : bug 528923
// if not, delete automation from the archive
static void homebank_upgrade_to_v05(void)
{
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v05\n") );

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;

		da_archive_consistency(entry);
		list = g_list_next(list);
	}
}


// v0.5 to v0.6 : we must change kxferacc to 0 on non Xfer transactions
//#677351
static void homebank_upgrade_to_v06(void)
{
GList *lst_acc, *lnk_acc;
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v06\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;
			da_transaction_consistency(entry);
			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		da_archive_consistency(entry);
		list = g_list_next(list);
	}
}


// v0.7 AF_BUDGET deleted instead of AF_NOBUDGET
static void homebank_upgrade_to_v07(void)
{
GList *lacc, *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v07\n") );

	lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *acc = list->data;

		if( acc->flags & AF_OLDBUDGET )	// budget include
		{
			acc->flags &= ~(AF_OLDBUDGET);
		}
		else
		{
			acc->flags |= AF_NOBUDGET;
		}

		list = g_list_next(list);
	}
	g_list_free(lacc);

}

static void homebank_upgrade_to_v08(void)
{
GList *lst_acc, *lnk_acc;
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v08\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;
			da_transaction_consistency(entry);
			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


}


static void homebank_upgrade_to_v10(void)
{
GList *lst_acc, *lnk_acc;
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v10\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;

			entry->status = TXN_STATUS_NONE;
			if(entry->flags & OF_OLDVALID)
				entry->status = TXN_STATUS_RECONCILED;
			else 
				if(entry->flags & OF_OLDREMIND)
					entry->status = TXN_STATUS_REMIND;

			//remove those flags
			entry->flags &= ~(OF_OLDVALID|OF_OLDREMIND);

			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

}


static void homebank_upgrade_to_v11(void)
{
GList *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_to_v11\n") );

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;

		entry->status = TXN_STATUS_NONE;
		if(entry->flags & OF_OLDVALID)
			entry->status = TXN_STATUS_RECONCILED;
		else 
			if(entry->flags & OF_OLDREMIND)
				entry->status = TXN_STATUS_REMIND;

		//remove those flags
		entry->flags &= ~(OF_OLDVALID|OF_OLDREMIND);

		list = g_list_next(list);
	}

}


// v0.6 to v0.7 : assign a default currency
static void homebank_upgrade_to_v12(void)
{
	// set a base currency to the hbfile if not
	DB( g_print("GLOBALS->kcur %d\n", GLOBALS->kcur) );

	ui_dialog_upgrade_choose_currency();
}



// lower v0.6 : we must assume categories/payee exists
// and strong link to xfer
// #632496
static void homebank_upgrade_lower_v06(void)
{
GList *lst_acc, *lnk_acc;
Category *cat;
Payee *pay;
GList *lrul, *list;

	DB( g_print("\n[hb-xml] homebank_upgrade_lower_v06\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *entry = list->data;

			//also strong link internal xfer
			if(entry->paymode == PAYMODE_INTXFER && entry->kxfer == 0)
			{
			Transaction *child = transaction_old_get_child_transfer(entry);
				if(child != NULL)
				{
					transaction_xfer_change_to_child(entry, child);
				}
			}

			da_transaction_consistency(entry);
			
			list = g_list_next(list);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	lrul = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		cat = da_cat_get(entry->kcat);
		if(cat == NULL)
		{
			DB( g_print(" !! fixing cat for rul: %d is unknow\n", entry->kcat) );
			entry->kcat = 0;
		}

		pay = da_pay_get(entry->kpay);
		if(pay == NULL)
		{
			DB( g_print(" !! fixing pay for rul: %d is unknow\n", entry->kpay) );
			entry->kpay = 0;
		}


		list = g_list_next(list);
	}
	g_list_free(lrul);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void homebank_load_xml_acc(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Account *entry = da_acc_malloc();
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "key"     )) { entry->key   = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "flags"   )) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "pos"     )) { entry->pos   = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "type"    )) { entry->type = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "curr"    )) { entry->kcur = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->name = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "number"  )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->number = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "bankname")) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->bankname = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "initial" )) { entry->initial = g_ascii_strtod(attribute_values[i], NULL); }

		else if(!strcmp (attribute_names[i], "minimum" )) { entry->minimum = g_ascii_strtod(attribute_values[i], NULL); }
		else if(!strcmp (attribute_names[i], "cheque1" )) { entry->cheque1 = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "cheque2" )) { entry->cheque2 = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "notes"   )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->notes = g_strdup(attribute_values[i]); }
	}

	//all attribute loaded: append
	da_acc_insert(entry);
}


static void homebank_load_xml_asg(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Assign *entry = da_asg_malloc();
gint exact = 0;
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "key"     )) { entry->key   = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "flags"   )) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "field"   )) { entry->field = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->text = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "payee"   )) { entry->kpay = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "category")) { entry->kcat = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "paymode" )) { entry->paymode = atoi(attribute_values[i]); }
		// prior v08
		else if(!strcmp (attribute_names[i], "exact" )) { exact = atoi(attribute_values[i]); }
	}

	/* in v08 exact moved to flag */
	if( ctx->file_version <= 0.7)
	{
		entry->flags = (ASGF_DOCAT|ASGF_DOPAY);
		if( exact > 0 )
		   entry->flags |= ASGF_EXACT;
	}

	//all attribute loaded: append
	da_asg_append(entry);

}


static void homebank_load_xml_pay(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Payee *entry = da_pay_malloc();
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

			 if(!strcmp (attribute_names[i], "key"  )) { entry->key = atoi(attribute_values[i]); }
		//else if(!strcmp (attribute_names[i], "flags")) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name" )) { entry->name = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "category")) { entry->kcat = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "paymode" )) { entry->paymode = atoi(attribute_values[i]); }
	}

	//all attribute loaded: append
	da_pay_insert(entry);
}


static void homebank_load_xml_prop(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		     if(!strcmp (attribute_names[i], "title"       )) { g_free(GLOBALS->owner); GLOBALS->owner = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "curr"        )) { GLOBALS->kcur = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "car_category")) { GLOBALS->vehicle_category = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "auto_smode"  )) { GLOBALS->auto_smode = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "auto_weekday")) { GLOBALS->auto_weekday = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "auto_nbdays" )) { GLOBALS->auto_nbdays = atoi(attribute_values[i]); }
	}
}


static void homebank_load_xml_cat(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Category *entry = da_cat_malloc();
gboolean budget;
gint i, j;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "key"   )) { entry->key = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "parent")) { entry->parent = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "flags" )) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name"  )) { entry->name = g_strdup(attribute_values[i]); }

		budget = FALSE;
		for(j=0;j<=12;j++)
		{
		gchar *tmpname;

			tmpname = g_strdup_printf ("b%d", j);
			if(!(strcmp (attribute_names[i], tmpname))) { entry->budget[j] = g_ascii_strtod(attribute_values[i], NULL); }
			g_free(tmpname);

			if(entry->budget[j]) budget = TRUE;
		}
		if(budget == TRUE)
			entry->flags |= GF_BUDGET;

	}

	//all attribute loaded: append
	da_cat_insert( entry);
}


static void homebank_load_xml_cur(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Currency *entry = da_cur_malloc ();
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

			 if(!strcmp (attribute_names[i], "key"    )) { entry->key = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name"  )) { entry->name = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "iso"   )) { entry->iso_code = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "symb"  )) { entry->symbol = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "syprf" )) { entry->sym_prefix = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "dchar" )) { entry->decimal_char = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "gchar" )) { entry->grouping_char = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "frac"  )) { entry->frac_digits = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "rate"  )) { entry->rate = g_ascii_strtod(attribute_values[i], NULL); }
		else if(!strcmp (attribute_names[i], "mdate ")) { entry->mdate = atoi(attribute_values[i]); }

	}

	//all attribute loaded: append
	da_cur_insert (entry);
}


static void homebank_load_xml_tag(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Tag *entry = da_tag_malloc();
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "key"  )) { entry->key = atoi(attribute_values[i]); }
		//else if(!strcmp (attribute_names[i], "flags")) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "name" )) { entry->name = g_strdup(attribute_values[i]); }
	}

	//all attribute loaded: append
	da_tag_insert(entry);
}


static void homebank_load_xml_fav(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Archive *entry = da_archive_malloc();
gchar *scat = NULL;
gchar *samt = NULL;
gchar *smem = NULL;
gboolean split = FALSE;
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
		else if(!strcmp (attribute_names[i], "account"    )) { entry->kacc = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "dst_account")) { entry->kxferacc = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "st"         )) { entry->status = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "payee"      )) { entry->kpay = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "category"   )) { entry->kcat = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->wording = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "nextdate"   )) { entry->nextdate = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "every"      )) { entry->every = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "unit"       )) { entry->unit = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "limit"      )) { entry->limit = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "weekend"    )) { entry->weekend = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "scat" 	  )) { scat = (gchar *)attribute_values[i]; split = TRUE; }
		else if(!strcmp (attribute_names[i], "samt"       )) { samt = (gchar *)attribute_values[i]; split = TRUE; }
		else if(!strcmp (attribute_names[i], "smem"       )) { smem = (gchar *)attribute_values[i]; split = TRUE; }

	}

	if(split == TRUE)
	{
		if (da_splits_parse(entry->splits, scat, samt, smem) > 0)
		{
			entry->flags |= OF_SPLIT; //Flag that Splits are active
		}
	}

	//all attribute loaded: append
	GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, entry);
}


static void homebank_load_xml_ope(ParseContext *ctx, const gchar **attribute_names, const gchar **attribute_values)
{
Transaction *entry = da_transaction_malloc();
gchar *scat = NULL;
gchar *samt = NULL;
gchar *smem = NULL;
gboolean split = FALSE;
gint i;

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );

		     if(!strcmp (attribute_names[i], "date"       )) { entry->date = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "amount"     )) { entry->amount = g_ascii_strtod(attribute_values[i], NULL); }
		else if(!strcmp (attribute_names[i], "account"    )) { entry->kacc = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "dst_account")) { entry->kxferacc = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "paymode"    )) { entry->paymode = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "st"         )) { entry->status = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "flags"      )) { entry->flags = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "payee"      )) { entry->kpay = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "category"   )) { entry->kcat = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "wording"    )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->wording = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "info"       )) { if(strcmp(attribute_values[i],"(null)") && attribute_values[i] != NULL) entry->info = g_strdup(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "tags"       ))
		{
			if(attribute_values[i] != NULL && strlen(attribute_values[i]) > 0 && strcmp(attribute_values[i],"(null)") != 0 )
			{
				transaction_tags_parse(entry, attribute_values[i]);
			}
		}
		else if(!strcmp (attribute_names[i], "kxfer"    )) { entry->kxfer = atoi(attribute_values[i]); }
		else if(!strcmp (attribute_names[i], "scat"     )) { scat = (gchar *)attribute_values[i]; split = TRUE; }
		else if(!strcmp (attribute_names[i], "samt"     )) { samt = (gchar *)attribute_values[i]; split = TRUE; }
		else if(!strcmp (attribute_names[i], "smem"     )) { smem = (gchar *)attribute_values[i]; split = TRUE; }
	}

	//bugfix 303886
	//if(entry->kcat < 0)
	//	entry->kcat = 0;

	if(split == TRUE)
	{
		if (da_splits_parse(entry->splits, scat, samt, smem) > 0)
		{
			entry->flags |= OF_SPLIT; //Flag that Splits are active
		}
	}
	
	//all attribute loaded: append
	// we use prepend here, the list will be reversed later for perf reason
	da_transaction_prepend(entry);
}




static void
start_element_handler (GMarkupParseContext *context,
		       const gchar         *element_name,
		       const gchar        **attribute_names,
		       const gchar        **attribute_values,
		       gpointer             user_data,
		       GError             **error)
{
ParseContext *ctx = user_data;
//GtkUIManager *self = ctx->self;

	//DB( g_print("** start element: %s\n", element_name) );

	switch(element_name[0])
	{
		case 'a':
		{
			if(!strcmp (element_name, "account"))   //account
			{
				homebank_load_xml_acc(ctx, attribute_names, attribute_values);
			}
			else if(!strcmp (element_name, "asg"))  //assign
			{
				homebank_load_xml_asg(ctx, attribute_names, attribute_values);
			}
		}
		break;

		case 'p':
		{
			if(!strcmp (element_name, "pay"))
			{
				homebank_load_xml_pay(ctx, attribute_names, attribute_values);
			}
			else if(!strcmp (element_name, "properties"))
			{
				homebank_load_xml_prop(ctx, attribute_names, attribute_values);
			}
		}
		break;

		case 'c':
		{
			if(!strcmp (element_name, "cat"))
			{
				homebank_load_xml_cat(ctx, attribute_names, attribute_values);
			}
			else if(!strcmp (element_name, "cur"))
			{
				homebank_load_xml_cur(ctx, attribute_names, attribute_values);
			}
		}
		break;

		case 't':
		{
			if(!strcmp (element_name, "tags"))
			{
				homebank_load_xml_tag(ctx, attribute_names, attribute_values);
			}
		}

		case 'f':
		{
			if(!strcmp (element_name, "fav"))
			{
				homebank_load_xml_fav(ctx, attribute_names, attribute_values);
			}
		}
		break;

		case 'o':
		{
			if(!strcmp (element_name, "ope"))
			{
				homebank_load_xml_ope(ctx, attribute_names, attribute_values);
			}
		}
		break;
	}
}


/*
static void
end_element_handler (GMarkupParseContext *context,
		     const gchar         *element_name,
		     gpointer             user_data,
		     GError             **error)
{
  ParseContext *ctx = user_data;

	//DB( g_print("-- end element: %s\n", element_name) );


}
*/

static GMarkupParser hb_parser = {
	start_element_handler,
	NULL,	//end_element_handler,
	NULL, //text_handler,
	NULL,
	NULL  //cleanup
};


static gboolean hb_xml_get_version(ParseContext *ctx, gchar *buffer)
{
gchar *v_buffer;

	ctx->file_version = 0.0;
	ctx->data_version = 0;

	/* v3.4 add :: prevent load of future file version */
	v_buffer = g_strstr_len(buffer, 50, "<homebank v=");
	if( v_buffer == NULL )
		return FALSE;

	DB( g_print("- id line: --(%.50s)\n\n", v_buffer) );

	ctx->file_version = g_ascii_strtod(v_buffer+13, NULL);	/* a little hacky, but works ! */
	if( ctx->file_version == 0.0 ) 
		ctx->file_version = 0.1;
	else if( ctx->file_version == 5.0 ) //was a mistake
		ctx->file_version = 1.0;

	v_buffer = g_strstr_len(buffer+13, 50, "d=");
	if( v_buffer )
	{
		DB( g_print(" d=%s)\n\n", v_buffer) );
	
		ctx->data_version = atoi(v_buffer+3);
	}
	return TRUE;
}


/*
** XML load homebank file: hbfile
*/
gint homebank_load_xml(gchar *filename)
{
gint retval;
gchar *buffer;
gsize length;
GError *error = NULL;
ParseContext ctx;
GMarkupParseContext *context;
gboolean rc;

	DB( g_print("\n[hb-xml] homebank_load_xml\n") );

	retval = XML_OK;
	if (!g_file_get_contents (filename, &buffer, &length, &error))
	{
		//g_message ("%s", error->message);
		retval = XML_IO_ERROR;
		g_error_free (error);
	}
	else
	{
		if( hb_xml_get_version(&ctx, buffer) == FALSE )
		{
			return XML_FILE_ERROR;
		}

		if( ctx.file_version > FILE_VERSION )
		{
			DB( g_print("- failed: version %f is not supported (max is %f)\n", ctx.file_version, FILE_VERSION) );
			return XML_VERSION_ERROR;
		}
		else
		{
			DB( g_print("- file ok : v=%.1f data_v=%06d\n", ctx.file_version, ctx.data_version) );

			/* 1st: validate the file is well in utf-8 */
			DB( g_print("- ensure UTF-8\n") );
			buffer = homebank_utf8_ensure(buffer);

			/* then process the buffer */
			#if MYDEBUG == 1
				GTimer *t = g_timer_new();
				g_print("- start parse\n");
			#endif

			context = g_markup_parse_context_new (&hb_parser, 0, &ctx, NULL);

			error = NULL;
			rc = g_markup_parse_context_parse (context, buffer, length, &error);

			if( error )
				g_print("failed: %s\n", error->message);

			if( rc == FALSE )
			{
				error = NULL;
				g_markup_parse_context_end_parse(context, &error);

				if( error )
					g_print("failed: %s\n", error->message);
			}

			g_markup_parse_context_free (context);
			g_free (buffer);

			DB( g_print("- end parse : %f sec\n", g_timer_elapsed(t, NULL)) );
			DB( g_timer_destroy (t) );

			/* file upgrade / bugfix */
			if( ctx.file_version <= 0.1 )
				homebank_upgrade_to_v02();
			if( ctx.file_version <= 0.2 )
				homebank_upgrade_to_v03();
			if( ctx.file_version <= 0.3 )
				homebank_upgrade_to_v04();
			if( ctx.file_version <= 0.4 )
				homebank_upgrade_to_v05();
			if( ctx.file_version <= 0.5 )
			{
				homebank_upgrade_to_v06();
				homebank_upgrade_lower_v06();
			}
			if( ctx.file_version <= 0.6 )
			{
				homebank_upgrade_to_v07();
				hbfile_sanity_check();
			}
			if( ctx.file_version <= 0.7 )	// <= 4.5
			{	
				homebank_upgrade_to_v08();
			}
			if( ctx.file_version <= 0.8 )	// <= 4.6
			{
				hbfile_sanity_check();
			}
			if( ctx.file_version <= 0.9 )	// <= 4.6.3
			{
				hbfile_sanity_check();
				homebank_upgrade_to_v10();
			}
			if( ctx.file_version <= 1.0 )	// <= 5.0.0 
			{
				hbfile_sanity_check();
				homebank_upgrade_to_v11();
			}
			//starting 5.0.4 data upgrade is done without changing file_version
			if( ctx.data_version < 050005 )	// <= 5.0.5 
			{
				hbfile_sanity_check();
			}
			if( ctx.file_version <= 1.1 )	// <= 5.1.0 
			{
				hbfile_sanity_check();
				homebank_upgrade_to_v12();
			}

			// next ?
			
		}
	}

	return retval;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
** misc xml attributes methods
*/
static void hb_xml_append_txt(GString *gstring, gchar *attrname, gchar *value)
{
	if(value != NULL && *value != 0)
	{
		gchar *escaped = g_markup_escape_text(value, -1);
		g_string_append_printf(gstring, "%s=\"%s\" ", attrname, escaped);
		g_free(escaped);
	}
}

static void
append_escaped_text (GString     *str,
                     const gchar *text,
                     gssize       length)
{
  const gchar *p;
  const gchar *end;
  gunichar c;

  p = text;
  end = text + length;

  while (p < end)
    {
      const gchar *next;
      next = g_utf8_next_char (p);

      switch (*p)
        {
        case '&':
          g_string_append (str, "&amp;");
          break;

        case '<':
          g_string_append (str, "&lt;");
          break;

        case '>':
          g_string_append (str, "&gt;");
          break;

        case '\'':
          g_string_append (str, "&apos;");
          break;

        case '"':
          g_string_append (str, "&quot;");
          break;

        default:
          c = g_utf8_get_char (p);
          if ((0x1 <= c && c <= 0x8) ||
              (0xa <= c && c  <= 0xd) ||	//chnaged here from b<->c to a<->d
              (0xe <= c && c <= 0x1f) ||
              (0x7f <= c && c <= 0x84) ||
              (0x86 <= c && c <= 0x9f))
            g_string_append_printf (str, "&#x%x;", c);
          else
            g_string_append_len (str, p, next - p);
          break;
        }

      p = next;
    }
}

// we override g_markup_escape_text from glib to encode \n (LF) & \r (CR)
static void hb_xml_append_txt_crlf(GString *gstring, gchar *attrname, gchar *value)
{
	if(value != NULL && *value != 0)
	{
	gssize length;
	GString *escaped;
		
		//gchar *escaped = g_markup_escape_text(value, -1);
		length = strlen (value);
		escaped = g_string_sized_new (length);
		append_escaped_text (escaped, value, length);
		g_string_append_printf(gstring, "%s=\"%s\" ", attrname, escaped->str);
		g_string_free (escaped, TRUE);
	}
}

static void hb_xml_append_int0(GString *gstring, gchar *attrname, guint32 value)
{
	g_string_append_printf(gstring, "%s=\"%d\" ", attrname, value);
}
	
static void hb_xml_append_int(GString *gstring, gchar *attrname, guint32 value)
{
	if(value != 0)
	{
		hb_xml_append_int0(gstring, attrname, value);
	}
}

static void hb_xml_append_amt(GString *gstring, gchar *attrname, gdouble amount)
{
char buf[G_ASCII_DTOSTR_BUF_SIZE];

	//we must use this, as fprintf use locale decimal settings and not '.'
	g_ascii_dtostr (buf, sizeof (buf), amount);
	g_string_append_printf(gstring, "%s=\"%s\" ", attrname, buf);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/*
** XML properties save
*/
static gint homebank_save_xml_prop(GIOChannel *io)
{
gchar *title;
GString *node;
gint retval = XML_OK;
GError *error = NULL;

	title = GLOBALS->owner == NULL ? "" : GLOBALS->owner;

	node = g_string_sized_new(255);

	g_string_assign(node, "<properties ");
	
	hb_xml_append_txt(node, "title", title);
	hb_xml_append_int(node, "curr", GLOBALS->kcur);
	hb_xml_append_int(node, "car_category", GLOBALS->vehicle_category);
	hb_xml_append_int0(node, "auto_smode", GLOBALS->auto_smode);
	hb_xml_append_int(node, "auto_weekday", GLOBALS->auto_weekday);
	hb_xml_append_int(node, "auto_nbdays", GLOBALS->auto_nbdays);

	g_string_append(node, "/>\n");

	error = NULL;
	g_io_channel_write_chars(io, node->str, -1, NULL, &error);
	if(error)
	{
		retval = XML_IO_ERROR;
		g_error_free(error);
	}

	g_string_free(node, TRUE);
	return retval;
}


/*
** XML currency save
*/
static gint homebank_save_xml_cur(GIOChannel *io)
{
GList *list;
gchar *tmpstr;
char buf1[G_ASCII_DTOSTR_BUF_SIZE];
gint retval = XML_OK;

	list = g_hash_table_get_values(GLOBALS->h_cur);
	while (list != NULL)
	{
	Currency *item = list->data;

		tmpstr = g_markup_printf_escaped(
		    "<cur key=\"%d\" iso=\"%s\" name=\"%s\" symb=\"%s\" syprf=\"%d\" dchar=\"%s\" gchar=\"%s\" frac=\"%d\" rate=\"%s\" mdate=\"%d\" />\n",
			item->key,
			item->iso_code,
		    item->name,
		    item->symbol,
		    item->sym_prefix,
		    item->decimal_char,
		    item->grouping_char,
		    item->frac_digits,
		    g_ascii_dtostr (buf1, sizeof (buf1), item->rate),
		    item->mdate
		);

		g_io_channel_write_chars(io, tmpstr, -1, NULL, NULL);
		g_free(tmpstr);

		list = g_list_next(list);
	}
	g_list_free(list);
	return retval;
}


/*
** XML account save
*/
static gint homebank_save_xml_acc(GIOChannel *io)
{
GList *lacc, *list;
GString *node;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);

	lacc = list = account_glist_sorted(0);
	while (list != NULL)
	{
	Account *item = list->data;

		item->flags &= ~(AF_ADDED|AF_CHANGED);	//delete flag

		g_string_assign(node, "<account ");
		
		hb_xml_append_int(node, "key", item->key);
		hb_xml_append_int(node, "flags", item->flags);
		hb_xml_append_int(node, "pos", item->pos);
		hb_xml_append_int(node, "type", item->type);
		hb_xml_append_int(node, "curr", item->kcur);
		hb_xml_append_txt(node, "name", item->name);
		hb_xml_append_txt(node, "number", item->number);
		hb_xml_append_txt(node, "bankname", item->bankname);
		hb_xml_append_amt(node, "initial", item->initial);

		hb_xml_append_amt(node, "minimum", item->minimum);
		hb_xml_append_int(node, "cheque1", item->cheque1);
		hb_xml_append_int(node, "cheque2", item->cheque2);
		hb_xml_append_txt_crlf(node, "notes", item->notes);

		g_string_append(node, "/>\n");

		error = NULL;
		g_io_channel_write_chars(io, node->str, -1, NULL, &error);

		if(error)
		{
			retval = XML_IO_ERROR;
			g_error_free(error);
		}

		list = g_list_next(list);
	}
	g_list_free(lacc);
	g_string_free(node, TRUE);
	return retval;
}

/*
** XML payee save
*/
static gint homebank_save_xml_pay(GIOChannel *io)
{
GList *lpay, *list;
GString *node;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);

	lpay = list = payee_glist_sorted(0);
	while (list != NULL)
	{
	Payee *item = list->data;

		if(item->key != 0)
		{
			g_string_assign(node, "<pay ");

			hb_xml_append_int(node, "key", item->key);
			hb_xml_append_txt(node, "name", item->name);
			hb_xml_append_int(node, "category", item->kcat);
			hb_xml_append_int(node, "paymode" , item->paymode);

			g_string_append(node, "/>\n");

			error = NULL;
			g_io_channel_write_chars(io, node->str, -1, NULL, &error);

			if(error)
			{
				retval = XML_IO_ERROR;
				g_error_free(error);
			}

		}
		list = g_list_next(list);
	}
	g_list_free(lpay);
	g_string_free(node, TRUE);
	return retval;
}

/*
** XML category save
*/
static gint homebank_save_xml_cat(GIOChannel *io)
{
GList *lcat, *list;
GString *node;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
guint i;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);

	lcat = list = category_glist_sorted(0);
	while (list != NULL)
	{
	Category *item = list->data;

		if(item->key != 0)
		{
			g_string_assign(node, "<cat ");
		
			hb_xml_append_int(node, "key", item->key);
			hb_xml_append_int(node, "parent", item->parent);
			hb_xml_append_int(node, "flags", item->flags);
			hb_xml_append_txt(node, "name", item->name);	

			for(i=0;i<=12;i++)
			{
				if(item->budget[i] != 0)
				{
					g_string_append_printf(node,"b%d=\"%s\" ", i, g_ascii_dtostr (buf, sizeof (buf), item->budget[i]));
				}
			}

			g_string_append(node, "/>\n");
			
			error = NULL;
			g_io_channel_write_chars(io, node->str, -1, NULL, &error);

			if(error)
			{
				retval = XML_IO_ERROR;
				g_error_free(error);
			}

		}
		list = g_list_next(list);
	}
	g_list_free(lcat);
	g_string_free(node, TRUE);
	return retval;
}

/*
** XML tag save
*/
static gint homebank_save_xml_tag(GIOChannel *io)
{
GList *ltag, *list;
gchar *tmpstr;
gint retval = XML_OK;
GError *error = NULL;

	ltag = list = tag_glist_sorted(0);
	while (list != NULL)
	{
	Tag *item = list->data;

		if(item->key != 0)
		{
			tmpstr = g_markup_printf_escaped("<tag key=\"%d\" name=\"%s\"/>\n",
				item->key,
				item->name
			);

			error = NULL;
			g_io_channel_write_chars(io, tmpstr, -1, NULL, &error);
			g_free(tmpstr);
			
			if(error)
			{
				retval = XML_IO_ERROR;
				g_error_free(error);
			}
		}
		list = g_list_next(list);
	}
	g_list_free(ltag);
	return retval;
}


/*
** XML assign save
*/
static gint homebank_save_xml_asg(GIOChannel *io)
{
GList *lasg, *list;
GString *node;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);
	
	lasg = list = assign_glist_sorted(0);
	while (list != NULL)
	{
	Assign *item = list->data;

		g_string_assign(node, "<asg ");

		hb_xml_append_int(node, "key"     , item->key);
		hb_xml_append_int(node, "flags"   , item->flags);
		hb_xml_append_int(node, "field"   , item->field);
		hb_xml_append_txt(node, "name"    , item->text);	
		hb_xml_append_int(node, "payee"   , item->kpay);
		hb_xml_append_int(node, "category", item->kcat);
		hb_xml_append_int(node, "paymode" , item->paymode);

		g_string_append(node, "/>\n");
		
		error = NULL;
		g_io_channel_write_chars(io, node->str, -1, NULL, &error);

		if(error)
		{
			retval = XML_IO_ERROR;
			g_error_free(error);
		}

		list = g_list_next(list);
	}
	g_list_free(lasg);
	g_string_free(node, TRUE);
	return retval;
}



/*
** XML archive save
*/
static gint homebank_save_xml_arc(GIOChannel *io)
{
GList *list;
GString *node;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = list->data;

		g_string_assign(node, "<fav ");

		hb_xml_append_amt(node, "amount", item->amount);
		hb_xml_append_int(node, "account", item->kacc);
		hb_xml_append_int(node, "dst_account", item->kxferacc);
		hb_xml_append_int(node, "paymode", item->paymode);
		hb_xml_append_int(node, "st", item->status);
		hb_xml_append_int(node, "flags", item->flags);
		hb_xml_append_int(node, "payee", item->kpay);
		hb_xml_append_int(node, "category", item->kcat);
		hb_xml_append_txt(node, "wording", item->wording);	
		hb_xml_append_int(node, "nextdate", item->nextdate);
		hb_xml_append_int(node, "every", item->every);
		hb_xml_append_int(node, "unit", item->unit);
		hb_xml_append_int(node, "limit", item->limit);
		hb_xml_append_int(node, "weekend", item->weekend);

		if(da_splits_count(item->splits) > 0)
		{
			gchar *cats, *amounts, *memos;
		
			da_splits_tostring(item->splits, &cats, &amounts, &memos);
			g_string_append_printf(node, "scat=\"%s\" ", cats);
			g_string_append_printf(node, "samt=\"%s\" ", amounts);

			//fix #1173910
			gchar *escaped = g_markup_escape_text(memos, -1);
			g_string_append_printf(node, "smem=\"%s\" ", escaped);
			g_free(escaped);

			g_free(cats);
			g_free(amounts);
			g_free(memos);
		}

		g_string_append(node, "/>\n");

		error = NULL;
		g_io_channel_write_chars(io, node->str, -1, NULL, &error);
		if(error)
		{
			retval = XML_IO_ERROR;
			g_error_free(error);
		}

		list = g_list_next(list);
	}
	g_string_free(node, TRUE);
	return retval;
}


/*
** XML transaction save
*/
static gint homebank_save_xml_ope(GIOChannel *io)
{
GList *lst_acc, *lnk_acc;
GList *list;
GString *node;
gchar *tagstr;
gint retval = XML_OK;
GError *error = NULL;

	node = g_string_sized_new(255);

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		list = g_queue_peek_head_link(acc->txn_queue);
		while (list != NULL)
		{
		Transaction *item = list->data;

			item->flags &= ~(OF_AUTO|OF_ADDED|OF_CHANGED);	//delete flag
			tagstr = transaction_tags_tostring(item);

			g_string_assign(node, "<ope ");
		
			hb_xml_append_int(node, "date", item->date);
			hb_xml_append_amt(node, "amount", item->amount);
			hb_xml_append_int(node, "account", item->kacc);
			hb_xml_append_int(node, "dst_account", item->kxferacc);
			hb_xml_append_int(node, "paymode", item->paymode);
			hb_xml_append_int(node, "st", item->status);
			hb_xml_append_int(node, "flags", item->flags);
			hb_xml_append_int(node, "payee", item->kpay);
			hb_xml_append_int(node, "category", item->kcat);
			hb_xml_append_txt(node, "wording", item->wording);	
			hb_xml_append_txt(node, "info", item->info);	
			hb_xml_append_txt(node, "tags", tagstr);	
			hb_xml_append_int(node, "kxfer", item->kxfer);

		if(da_splits_count(item->splits) > 0)
		{
			gchar *cats, *amounts, *memos;
		
				da_splits_tostring(item->splits, &cats, &amounts, &memos);
				g_string_append_printf(node, "scat=\"%s\" ", cats);
				g_string_append_printf(node, "samt=\"%s\" ", amounts);

				//fix #1173910
				gchar *escaped = g_markup_escape_text(memos, -1);
				g_string_append_printf(node, "smem=\"%s\" ", escaped);
				g_free(escaped);

				g_free(cats);
				g_free(amounts);
				g_free(memos);
			}

			g_string_append(node, "/>\n");

			g_free(tagstr);
		
			error = NULL;
			g_io_channel_write_chars(io, node->str, -1, NULL, &error);
		
			if(error)
			{
				retval = XML_IO_ERROR;
				g_error_free(error);
			}

			list = g_list_next(list);
		}
		
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	g_string_free(node, TRUE);
	return retval;
}

/*
** XML save homebank file: hbfile
*/
gint homebank_save_xml(gchar *filename)
{
GIOChannel *io;
char buf1[G_ASCII_DTOSTR_BUF_SIZE];
gchar *outstr;
gint retval = XML_OK;
GError *error = NULL;

	io = g_io_channel_new_file(filename, "w", &error);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
		retval = XML_IO_ERROR;
		
		if(error)
			g_print("failed: %s\n", error->message);
		
		g_error_free(error);
	}
	else
	{
		g_io_channel_write_chars(io, "<?xml version=\"1.0\"?>\n", -1, NULL, NULL);

		outstr = g_strdup_printf("<homebank v=\"%s\" d=\"%06d\">\n", g_ascii_dtostr (buf1, sizeof (buf1), FILE_VERSION), HB_VERSION_NUM);
		g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
		g_free(outstr);

		retval = homebank_save_xml_prop(io);
		retval = homebank_save_xml_cur(io);
		retval = homebank_save_xml_acc(io);
		retval = homebank_save_xml_pay(io);
		retval = homebank_save_xml_cat(io);
		retval = homebank_save_xml_tag(io);
		retval = homebank_save_xml_asg(io);
		retval = homebank_save_xml_arc(io);
		retval = homebank_save_xml_ope(io);

		g_io_channel_write_chars(io, "</homebank>\n", -1, NULL, NULL);

		g_io_channel_unref (io);
	}
	return retval;
}

