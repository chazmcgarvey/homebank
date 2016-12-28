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
#include "hb-export.h"

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

static void hb_export_qif_elt_txn(GIOChannel *io, Account *acc)
{
GString *elt;
GList *list;
GDate *date;
char amountbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *sbuf;
gint count, i;

	elt = g_string_sized_new(255);

	date = g_date_new ();
	
	list = g_queue_peek_head_link(acc->txn_queue);
	while (list != NULL)
	{
	Transaction *txn = list->data;
	Payee *payee;
	Category *cat;
	gchar *txt;

		g_date_set_julian (date, txn->date);
		//#1270876
		switch(PREFS->dtex_datefmt)
		{
			case 0: //"m-d-y"  
				g_string_append_printf (elt, "D%02d/%02d/%04d\n", 
					g_date_get_month(date),
					g_date_get_day(date),
					g_date_get_year(date)
					);
				break;
			case 1: //"d-m-y"
				g_string_append_printf (elt, "D%02d/%02d/%04d\n", 
					g_date_get_day(date),
					g_date_get_month(date),
					g_date_get_year(date)
					);
				break;
			case 2: //"y-m-d"
				g_string_append_printf (elt, "D%04d/%02d/%02d\n", 
					g_date_get_year(date),
					g_date_get_month(date),
					g_date_get_day(date)
					);
				break;
		}			

		//g_ascii_dtostr (amountbuf, sizeof (amountbuf), txn->amount);
		g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", txn->amount);
		g_string_append_printf (elt, "T%s\n", amountbuf);

		sbuf = "";
		if(txn->status == TXN_STATUS_CLEARED)
			sbuf = "c";
		else
		if(txn->status == TXN_STATUS_RECONCILED)
			sbuf = "R";

		g_string_append_printf (elt, "C%s\n", sbuf);

		if( txn->paymode == PAYMODE_CHECK)
			g_string_append_printf (elt, "N%s\n", txn->info);

		//Ppayee
		payee = da_pay_get(txn->kpay);
		if(payee)
			g_string_append_printf (elt, "P%s\n", payee->name);

		// Mmemo
		g_string_append_printf (elt, "M%s\n", txn->wording);

		// LCategory of transaction
		// L[Transfer account name]
		// LCategory of transaction/Class of transaction
		// L[Transfer account]/Class of transaction
		if( txn->paymode == PAYMODE_INTXFER && txn->kacc == acc->key)
		{
		//#579260
			Account *dstacc = da_acc_get(txn->kxferacc);
			if(dstacc)
				g_string_append_printf (elt, "L[%s]\n", dstacc->name);
		}
		else
		{
			cat = da_cat_get(txn->kcat);
			if(cat)
			{
				txt = da_cat_get_fullname(cat);
				g_string_append_printf (elt, "L%s\n", txt);
				g_free(txt);
			}
		}

		// splits
		count = da_splits_count(txn->splits);
		for(i=0;i<count;i++)
		{
		Split *s = txn->splits[i];
				
			cat = da_cat_get(s->kcat);
			if(cat)
			{
				txt = da_cat_get_fullname(cat);
				g_string_append_printf (elt, "S%s\n", txt);
				g_free(txt);
			}	
				
			g_string_append_printf (elt, "E%s\n", s->memo);
			
			g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", s->amount);
			g_string_append_printf (elt, "$%s\n", amountbuf);
		}
		
		g_string_append (elt, "^\n");


		list = g_list_next(list);
	}

	g_io_channel_write_chars(io, elt->str, -1, NULL, NULL);
	
	g_string_free(elt, TRUE);

	g_date_free(date);
	
}



static void hb_export_qif_elt_acc(GIOChannel *io, Account *acc)
{
GString *elt;
gchar *type;
	
	elt = g_string_sized_new(255);
	
	// account export
	//#987144 fixed account type
	switch(acc->type)
	{
		case ACC_TYPE_BANK : type = "Bank"; break;
		case ACC_TYPE_CASH : type = "Cash"; break;
		case ACC_TYPE_ASSET : type = "Oth A"; break;
		case ACC_TYPE_CREDITCARD : type = "CCard"; break;
		case ACC_TYPE_LIABILITY : type = "Oth L"; break;
		default : type = "Bank"; break;
	}

	g_string_assign(elt, "!Account\n");
	g_string_append_printf (elt, "N%s\n", acc->name);
	g_string_append_printf (elt, "T%s\n", type);
	g_string_append (elt, "^\n");
	g_string_append_printf (elt, "!Type:%s\n", type);

	g_io_channel_write_chars(io, elt->str, -1, NULL, NULL);
	
	g_string_free(elt, TRUE);
}


void hb_export_qif_account_single(gchar *filename, Account *acc)
{
GIOChannel *io;
	
	io = g_io_channel_new_file(filename, "w", NULL);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
		//retval = XML_IO_ERROR;
	}
	else
	{
		hb_export_qif_elt_acc(io, acc);
		hb_export_qif_elt_txn(io, acc);	
		g_io_channel_unref (io);
	}
}


void hb_export_qif_account_all(gchar *filename)
{
GIOChannel *io;
GList *lacc, *list;

	io = g_io_channel_new_file(filename, "w", NULL);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
		//retval = XML_IO_ERROR;
	}
	else
	{
		//todo: save accounts in order
		//todo: save transfer transaction once

		lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
		while (list != NULL)
		{
		Account *item = list->data;

			hb_export_qif_elt_acc(io, item);
			hb_export_qif_elt_txn(io, item);	

			list = g_list_next(list);
		}
		g_list_free(lacc);

		g_io_channel_unref (io);
	}

}




