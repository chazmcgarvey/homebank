/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2016 Maxime DOYEN
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

#include "hb-import.h"

#ifndef NOOFX
#include <libofx/libofx.h>
#endif


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



#ifndef NOOFX
/*
**** OFX part
****
**** this part is quite weird,but works
** id is ACCTID

*/

static Account * ofx_get_account_by_id(gchar *id)
{
GList *lacc, *list;

	DB( g_print("\n[import] ofx_get_account_by_id\n") );
	DB( g_print(" -> searching for '%s'\n",id) );

	lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *accitem = list->data;

		if( accitem->imported == FALSE)
		{
			if(accitem->name && accitem->number && strlen(accitem->number) )
			{
				// todo: maybe smartness should be done here
				if(g_strstr_len(id, -1, accitem->number) != NULL)
				{
					return accitem;
				}
			}
		}
		list = g_list_next(list);
	}
	g_list_free(lacc);
	return NULL;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ofx_proc_account_cb:
 *
 * The ofx_proc_account_cb event is always generated first, to allow the application to create accounts
 * or ask the user to match an existing account before the ofx_proc_statement and ofx_proc_transaction
 * event are received. An OfxAccountData is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_account_cb(const struct OfxAccountData data, OfxContext *ctx)
{
Account *tmp_acc, *dst_acc;

	DB( g_print("** ofx_proc_account_cb()\n") );

	if(data.account_id_valid==true)
	{
		DB( g_print("  account_id: '%s'\n", data.account_id) );
		DB( g_print("  account_name: '%s'\n", data.account_name) );
	}

	//if(data.account_number_valid==true)
	//{
		DB( g_print("  account_number: '%s'\n", data.account_number) );
	//}


	if(data.account_type_valid==true)
	{
		DB( g_print("  account_type: '%d'\n", data.account_type) );
		/*
		enum:
		OFX_CHECKING 	A standard checking account
		OFX_SAVINGS 	A standard savings account
		OFX_MONEYMRKT 	A money market account
		OFX_CREDITLINE 	A line of credit
		OFX_CMA 	Cash Management Account
		OFX_CREDITCARD 	A credit card account
		OFX_INVESTMENT 	An investment account 
		*/
	}

	if(data.currency_valid==true)
	{
		DB( g_print("  currency: '%s'\n", data.currency) );
	}


	//find target account
	dst_acc = ofx_get_account_by_id( (gchar *)data.account_id );
	DB( g_print(" ** hb account found result is %x\n", (unsigned int)dst_acc) );


	// in every case we create an account here
	tmp_acc = import_create_account((gchar *)data.account_name, (gchar *)data.account_id);
	DB( g_print(" -> creating tmp account: %d %s - %x\n", tmp_acc->key, data.account_id, (unsigned int)tmp_acc) );

	if( dst_acc != NULL )
	{
		tmp_acc->imp_key = dst_acc->key;
	}


	ctx->curr_acc = tmp_acc;
	ctx->curr_acc_isnew = TRUE;








	DB( fputs("\n",stdout) );
	return 0;
}


/**
 * ofx_proc_statement_cb:
 *
 * The ofx_proc_statement_cb event is sent after all ofx_proc_transaction events have been sent.
 * An OfxStatementData is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_statement_cb(const struct OfxStatementData data, OfxContext *ctx)
{
	DB( g_print("** ofx_proc_statement_cb()\n") );

#ifdef MYDEBUG
	if(data.ledger_balance_date_valid==true)
	{
	struct tm temp_tm;

		temp_tm = *localtime(&(data.ledger_balance_date));
		g_print("ledger_balance_date : %d%s%d%s%d%s", temp_tm.tm_mday, "/", temp_tm.tm_mon+1, "/", temp_tm.tm_year+1900, "\n");
	}
#endif

	if(data.ledger_balance_valid==true)
	{
		if( ctx->curr_acc != NULL && ctx->curr_acc_isnew == TRUE )
		{
			ctx->curr_acc->initial = data.ledger_balance;
		}
		DB( g_print("ledger_balance: $%.2f%s",data.ledger_balance,"\n") );
	}

	return 0;
}

/**
 * ofx_proc_statement_cb:
 *
 * An ofx_proc_transaction_cb event is generated for every transaction in the ofx response,
 * after ofx_proc_statement (and possibly ofx_proc_security is generated.
 * An OfxTransactionData structure is passed to this event.
 *
 */
static LibofxProcStatementCallback
ofx_proc_transaction_cb(const struct OfxTransactionData data, OfxContext *ctx)
{
struct tm *temp_tm;
GDate date;
Transaction *newope;

	DB( g_print("\n** ofx_proc_transaction_cb()\n") );

	newope = da_transaction_malloc();

// date
	newope->date = 0;
	if(data.date_posted_valid && (data.date_posted != 0))
	{
		temp_tm = localtime(&data.date_posted);
		if( temp_tm != 0)
		{
			g_date_set_dmy(&date, temp_tm->tm_mday, temp_tm->tm_mon+1, temp_tm->tm_year+1900);
			newope->date = g_date_get_julian(&date);
		}
	}
	else if (data.date_initiated_valid && (data.date_initiated != 0))
	{
		temp_tm = localtime(&data.date_initiated);
		g_date_set_dmy(&date, temp_tm->tm_mday, temp_tm->tm_mon+1, temp_tm->tm_year+1900);
		newope->date = g_date_get_julian(&date);
	}

// amount
	if(data.amount_valid==true)
	{
		newope->amount = data.amount;
	}

// check number :: The check number is most likely an integer and can probably be converted properly with atoi(). 
	//However the spec allows for up to 12 digits, so it is not garanteed to work
	if(data.check_number_valid==true)
	{
		newope->info = g_strdup(data.check_number);
	}
	//todo: reference_number ?Might present in addition to or instead of a check_number. Not necessarily a number 

	//ucfirst
	//ucword



// ofx:name = Can be the name of the payee or the description of the transaction 
	if(data.name_valid==true)
	{
	Payee *payitem;
	gchar *name = NULL;

		//#462919 name to payee or memo
		DB( g_print(" -> ofxname option: '%d'\n", PREFS->dtex_ofxname) );
		switch(PREFS->dtex_ofxname)
		{
			case 1: //to memo
				DB( g_print(" -> name to memo: '%s'\n", data.name) );
				newope->wording = g_strdup(data.name);

				//test
				//strip_extra_spaces(newope->wording);

				break;
			case 2: //to payee
				//manage memo append to payee as well
				if( (data.memo_valid==true) && (PREFS->dtex_ofxmemo == 3) )
				{
					name = g_strjoin(" ", data.name, data.memo, NULL);
				}
				else
					name = g_strdup(data.name);
				
				g_strstrip(name);
				//test
				//strip_extra_spaces(name);
				
				DB( g_print(" -> name to payee: '%s'\n", name) );
				
				payitem = da_pay_get_by_name(name);
				if(payitem == NULL)
				{
					DB( g_print(" -> create new payee\n") );

					payitem = da_pay_malloc();
					payitem->name = name;
					payitem->imported = TRUE;
					da_pay_append(payitem);

					if( payitem->imported == TRUE )
						ctx->ictx->cnt_new_pay += 1;
				}
				else
				{
					g_free(name);
				}
				
				newope->kpay = payitem->key;
				break;
		}
	}

//memo ( new for v4.2) Extra information not included in name 

	DB( g_print(" -> memo is='%d'\n", data.memo_valid) );

	if(data.memo_valid==true)
	{
	gchar *old = NULL;

		DB( g_print(" -> oxfmemo option: '%d'\n", PREFS->dtex_ofxmemo) );
		switch(PREFS->dtex_ofxmemo)
		{
			case 1:	//add to info
				old = newope->info;
				if(old == NULL)
					newope->info = g_strdup(data.memo);
				else
				{
					newope->info = g_strjoin(" ", old, data.memo, NULL);
					g_free(old);
				}
				break;

			case 2: //add to description
				old = newope->wording;
				if(old == NULL)
					newope->wording = g_strdup(data.memo);
				else
				{
					newope->wording = g_strjoin(" ", old, data.memo, NULL);
					g_free(old);
				}

				DB( g_print(" -> should concatenate ='%s'\n", data.memo) );
				DB( g_print(" -> old='%s', new ='%s'\n", old, newope->wording) );

				break;
			//case 3 add to payee is managed above
		}
		
	}

// payment
	if(data.transactiontype_valid==true)
	{
		switch(data.transactiontype)
		{
			//#740373
			case OFX_CREDIT:
				if(newope->amount < 0)
					newope->amount *= -1;
				break;
			case OFX_DEBIT:
				if(newope->amount > 0)
					newope->amount *= -1;
				break;
			case OFX_INT:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_DIV:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_FEE:
					newope->paymode = PAYMODE_FEE;
				break;
			case OFX_SRVCHG:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_DEP:
					newope->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_ATM:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_POS:
				if(ctx->curr_acc && ctx->curr_acc->type == ACC_TYPE_CREDITCARD)
					newope->paymode = PAYMODE_CCARD;
				else
					newope->paymode = PAYMODE_DCARD;
				break;
			case OFX_XFER:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_CHECK:
					newope->paymode = PAYMODE_CHECK;
				break;
			case OFX_PAYMENT:
					newope->paymode = PAYMODE_EPAYMENT;
				break;
			case OFX_CASH:
					newope->paymode = PAYMODE_CASH;
				break;
			case OFX_DIRECTDEP:
					newope->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_DIRECTDEBIT:
					newope->paymode = PAYMODE_XFER;
				break;
			case OFX_REPEATPMT:
					newope->paymode = PAYMODE_REPEATPMT;
				break;
			case OFX_OTHER:

				break;
			default :

				break;
		}
	}

	if( ctx->curr_acc )
	{

		newope->kacc = ctx->curr_acc->key;
		newope->flags |= OF_ADDED;

		if( newope->amount > 0)
			newope->flags |= OF_INCOME;

		/* ensure utf-8 here, has under windows, libofx not always return utf-8 as it should */
	#ifndef G_OS_UNIX
		DB( g_print(" ensure UTF-8\n") );

		newope->info = homebank_utf8_ensure(newope->info);
		newope->wording = homebank_utf8_ensure(newope->wording);
	#endif

		ctx->trans_list = g_list_append(ctx->trans_list, newope);

		DB( g_print(" insert newope: acc=%d\n", newope->kacc) );

		if( ctx->curr_acc_isnew == TRUE )
		{
			DB( g_print(" sub amount from initial\n") );
			ctx->curr_acc->initial -= data.amount;
		}
	}
	else
	{
		da_transaction_free(newope);
	}

	return 0;
}



static LibofxProcStatusCallback
ofx_proc_status_cb(const struct OfxStatusData data, OfxContext *ctx)
{
	DB( g_print("** ofx_proc_status_cb()\n") );

   if(data.ofx_element_name_valid==true){
     DB( g_print("    Ofx entity this status is relevent to: '%s'\n", data.ofx_element_name) );
   }
   if(data.severity_valid==true){
     DB( g_print("    Severity: ") );
     switch(data.severity){
     case INFO : DB( g_print("INFO\n") );
       break;
     case WARN : DB( g_print("WARN\n") );
       break;
     case ERROR : DB( g_print("ERROR\n") );
       break;
     default: DB( g_print("WRITEME: Unknown status severity!\n") );
     }
   }
   if(data.code_valid==true){
     DB( g_print("    Code: %d, name: %s\n    Description: %s\n", data.code, data.name, data.description) );
   }
   if(data.server_message_valid==true){
     DB( g_print("    Server Message: %s\n", data.server_message) );
   }
   DB( g_print("\n") );
	
	return 0;
}


GList *homebank_ofx_import(gchar *filename, ImportContext *ictx)
{
OfxContext ctx = { 0 };

/*extern int ofx_PARSER_msg;
extern int ofx_DEBUG_msg;
extern int ofx_WARNING_msg;
extern int ofx_ERROR_msg;
extern int ofx_INFO_msg;
extern int ofx_STATUS_msg;*/

	DB( g_print("\n[import] ofx import (libofx=%s) \n", LIBOFX_VERSION_RELEASE_STRING) );

	/*ofx_PARSER_msg	= false;
	ofx_DEBUG_msg	= false;
	ofx_WARNING_msg = false;
	ofx_ERROR_msg	= false;
	ofx_INFO_msg	= false;
	ofx_STATUS_msg	= false;*/
	
	ctx.ictx = ictx;

	LibofxContextPtr libofx_context = libofx_get_new_context();

	ofx_set_status_cb     (libofx_context, (LibofxProcStatusCallback)     ofx_proc_status_cb     , &ctx);
	ofx_set_statement_cb  (libofx_context, (LibofxProcStatementCallback)  ofx_proc_statement_cb  , &ctx);
	ofx_set_account_cb    (libofx_context, (LibofxProcAccountCallback)    ofx_proc_account_cb    , &ctx);
	ofx_set_transaction_cb(libofx_context, (LibofxProcTransactionCallback)ofx_proc_transaction_cb, &ctx);

#ifdef G_OS_WIN32
	//#932959: windows don't like utf8 path, so convert
	gchar *file = g_win32_locale_filename_from_utf8(filename);
	libofx_proc_file(libofx_context, file, AUTODETECT);
	g_free(file);
#else
	libofx_proc_file(libofx_context, filename, AUTODETECT);
#endif

	libofx_free_context(libofx_context);

	return ctx.trans_list;
}

#endif
