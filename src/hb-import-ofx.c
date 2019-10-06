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
ofx_proc_account_cb(const struct OfxAccountData data, ImportContext *ctx)
{
GenAcc *genacc;
Account *dst_acc;

	DB( g_print("** ofx_proc_account_cb()\n") );

	if(data.account_id_valid==true)
	{
		DB( g_print("  account_id: %s\n", data.account_id) );
		DB( g_print("  account_name: %s\n", data.account_name) );
	}

	//if(data.account_number_valid==true)
	//{
		DB( g_print("  account_number: %s\n", data.account_number) );
	//}


	if(data.account_type_valid==true)
	{
		DB( g_print("  account_type: %d\n", data.account_type) );
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
		DB( g_print("  currency: %s\n", data.currency) );
	}

	//todo: normally should check for validity here
	// in every case we create an account here
	DB( g_print(" -> create generic account: '%s':'%s'\n", data.account_id, data.account_name) );
	genacc = hb_import_gen_acc_get_next (ctx, FILETYPE_OFX, (gchar *)data.account_name, (gchar *)data.account_id);
	ctx->curr_acc_isnew = TRUE;

	dst_acc = hb_import_acc_find_existing((gchar *)data.account_name, (gchar *)data.account_id );
	if( dst_acc != NULL )
	{
		genacc->kacc = dst_acc->key;
		ctx->curr_acc_isnew = FALSE;
		if(dst_acc->type == ACC_TYPE_CREDITCARD)
			genacc->is_ccard = TRUE;
	}

	ctx->curr_acc = genacc;

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
ofx_proc_statement_cb(const struct OfxStatementData data, ImportContext *ctx)
{
	DB( g_print("** ofx_proc_statement_cb()\n") );

#if MYDEBUG == 1
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
ofx_proc_transaction_cb(const struct OfxTransactionData data, ImportContext *ctx)
{
struct tm *temp_tm;
GDate date;
GenTxn *gentxn;

	DB( g_print("** ofx_proc_transaction_cb()\n") );

	gentxn = da_gen_txn_malloc();

// date
	gentxn->julian = 0;
	if(data.date_posted_valid && (data.date_posted != 0))
	{
		temp_tm = localtime(&data.date_posted);
		if( temp_tm != 0)
		{
			g_date_set_dmy(&date, temp_tm->tm_mday, temp_tm->tm_mon+1, temp_tm->tm_year+1900);
			gentxn->julian = g_date_get_julian(&date);
		}
	}
	else if (data.date_initiated_valid && (data.date_initiated != 0))
	{
		temp_tm = localtime(&data.date_initiated);
		if( temp_tm != 0)
		{
			g_date_set_dmy(&date, temp_tm->tm_mday, temp_tm->tm_mon+1, temp_tm->tm_year+1900);
			gentxn->julian = g_date_get_julian(&date);
		}
	}

// amount
	if(data.amount_valid==true)
	{
		gentxn->amount = data.amount;
	}

// check number :: The check number is most likely an integer and can probably be converted properly with atoi(). 
	//However the spec allows for up to 12 digits, so it is not garanteed to work
	if(data.check_number_valid==true)
	{
		gentxn->rawinfo = g_strdup(data.check_number);
	}
	//todo: reference_number ?Might present in addition to or instead of a check_number. Not necessarily a number 

// ofx:name = Can be the name of the payee or the description of the transaction 
	if(data.name_valid==true)
	{
		gentxn->rawpayee = g_strdup(data.name);
	}

//memo ( new for v4.2) #319202 Extra information not included in name 

	DB( g_print(" -> memo is='%d'\n", data.memo_valid) );


	if(data.memo_valid==true)
	{
		gentxn->rawmemo = g_strdup(data.memo);
	}

// payment
	if(data.transactiontype_valid==true)
	{
		switch(data.transactiontype)
		{
			//#740373
			case OFX_CREDIT:
				if(gentxn->amount < 0)
					gentxn->amount *= -1;
				break;
			case OFX_DEBIT:
				if(gentxn->amount > 0)
					gentxn->amount *= -1;
				break;
			case OFX_INT:
					gentxn->paymode = PAYMODE_XFER;
				break;
			case OFX_DIV:
					gentxn->paymode = PAYMODE_XFER;
				break;
			case OFX_FEE:
					gentxn->paymode = PAYMODE_FEE;
				break;
			case OFX_SRVCHG:
					gentxn->paymode = PAYMODE_XFER;
				break;
			case OFX_DEP:
					gentxn->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_ATM:
					gentxn->paymode = PAYMODE_CASH;
				break;
			case OFX_POS:
				if(ctx->curr_acc && ctx->curr_acc->is_ccard == TRUE)
					gentxn->paymode = PAYMODE_CCARD;
				else
					gentxn->paymode = PAYMODE_DCARD;
				break;
			case OFX_XFER:
					gentxn->paymode = PAYMODE_XFER;
				break;
			case OFX_CHECK:
					gentxn->paymode = PAYMODE_CHECK;
				break;
			case OFX_PAYMENT:
					gentxn->paymode = PAYMODE_EPAYMENT;
				break;
			case OFX_CASH:
					gentxn->paymode = PAYMODE_CASH;
				break;
			case OFX_DIRECTDEP:
					gentxn->paymode = PAYMODE_DEPOSIT;
				break;
			case OFX_DIRECTDEBIT:
					gentxn->paymode = PAYMODE_XFER;
				break;
			case OFX_REPEATPMT:
					gentxn->paymode = PAYMODE_REPEATPMT;
				break;
			case OFX_OTHER:

				break;
			default :

				break;
		}
	}

	if( ctx->curr_acc )
	{
		gentxn->account = g_strdup(ctx->curr_acc->name);

		#if MYDEBUG == 1
		if(gentxn->rawinfo)
			g_print(" len info %d %ld\n", (int)strlen(gentxn->rawinfo) , g_utf8_strlen(gentxn->rawinfo, -1));
		if(gentxn->rawmemo)
			g_print(" len memo %d %ld\n", (int)strlen(gentxn->rawmemo) , g_utf8_strlen(gentxn->rawmemo, -1));
		if(gentxn->rawpayee)
			g_print(" len name %d %ld\n", (int)strlen(gentxn->rawpayee), g_utf8_strlen(gentxn->rawpayee, -1));
		#endif

		//#1842935 workaround for libofx truncate bug that can leave invalid UTF-8 string 
		//NAME = A-32 (96 allowed)
		if( gentxn->rawpayee && g_utf8_strlen(gentxn->rawpayee, -1) > 32 )
		{
		gchar *oldtxt = gentxn->rawpayee;
			DB( g_print(" ensure UTF-8 for truncated NAME='%s'\n", oldtxt) );
			gentxn->rawpayee = g_utf8_make_valid(oldtxt, -1);
			g_free(oldtxt);
		}
		//TODO: maybe MEMO = A-255
		
		/* ensure utf-8 here, has under windows, libofx not always return utf-8 as it should */
		#ifndef G_OS_UNIX
		DB( g_print(" ensure UTF-8\n") );

		gentxn->rawinfo  = homebank_utf8_ensure(gentxn->rawinfo);
		gentxn->rawmemo  = homebank_utf8_ensure(gentxn->rawmemo);
		gentxn->rawpayee = homebank_utf8_ensure(gentxn->rawpayee);
		#endif

		da_gen_txn_append(ctx, gentxn);

		DB( g_print(" insert gentxn: acc=%s\n", gentxn->account) );

		if( ctx->curr_acc_isnew == TRUE )
		{
			DB( g_print(" sub amount from initial\n") );
			ctx->curr_acc->initial -= data.amount;
		}
	}
	else
	{
		da_gen_txn_free(gentxn);
		DB( g_print(" no account, insert txn skipped\n") );
	}

	return 0;
}


static LibofxProcStatusCallback
ofx_proc_status_cb(const struct OfxStatusData data, ImportContext *ctx)
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


GList *homebank_ofx_import(ImportContext *ictx, GenFile *genfile)
{
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

	LibofxContextPtr libofx_context = libofx_get_new_context();

	ofx_set_status_cb     (libofx_context, (LibofxProcStatusCallback)     ofx_proc_status_cb     , ictx);
	ofx_set_statement_cb  (libofx_context, (LibofxProcStatementCallback)  ofx_proc_statement_cb  , ictx);
	ofx_set_account_cb    (libofx_context, (LibofxProcAccountCallback)    ofx_proc_account_cb    , ictx);
	ofx_set_transaction_cb(libofx_context, (LibofxProcTransactionCallback)ofx_proc_transaction_cb, ictx);

#ifdef G_OS_WIN32
	//#932959: windows don't like utf8 path, so convert
	gchar *filepath = g_win32_locale_filename_from_utf8(genfile->filepath);
	libofx_proc_file(libofx_context, filepath, AUTODETECT);
	g_free(filepath);
#else
	libofx_proc_file(libofx_context, genfile->filepath, AUTODETECT);
#endif

	libofx_free_context(libofx_context);

	DB( g_print("ofx nb txn=%d\n", g_list_length(ictx->gen_lst_txn) ));

	return ictx->gen_lst_txn;
}

#endif
