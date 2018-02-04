/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2018 Maxime DOYEN
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

//#include "ui-assist-import.h"
#include "hb-import.h"

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
static QIF_Tran *
da_qif_tran_malloc(void)
{
	return g_malloc0(sizeof(QIF_Tran));
}


static void
da_qif_tran_free(QIF_Tran *item)
{
gint i;

	if(item != NULL)
	{
		if(item->date != NULL)
			g_free(item->date);
		if(item->info != NULL)
			g_free(item->info);
		if(item->payee != NULL)
			g_free(item->payee);
		if(item->memo != NULL)
			g_free(item->memo);
		if(item->category != NULL)
			g_free(item->category);
		if(item->account != NULL)
			g_free(item->account);

		for(i=0;i<TXN_MAX_SPLIT;i++)
		{
		QIFSplit *s = &item->splits[i];
		
			if(s->memo != NULL)
				g_free(s->memo);
			if(s->category != NULL)
				g_free(s->category);	
		}

		g_free(item);
	}
}


static void
da_qif_tran_destroy(QifContext *ctx)
{
GList *qiflist = g_list_first(ctx->q_tra);

	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;
		da_qif_tran_free(item);
		qiflist = g_list_next(qiflist);
	}
	g_list_free(ctx->q_tra);
	ctx->q_tra = NULL;
}


static void
da_qif_tran_new(QifContext *ctx)
{
	ctx->q_tra = NULL;
}


static void
da_qif_tran_move(QIF_Tran *sitem, QIF_Tran *ditem)
{
	if(sitem != NULL && ditem != NULL)
	{
		memcpy(ditem, sitem, sizeof(QIF_Tran));
		memset(sitem, 0, sizeof(QIF_Tran));
	}
}


static void
da_qif_tran_append(QifContext *ctx, QIF_Tran *item)
{
	ctx->q_tra = g_list_append(ctx->q_tra, item);
}


/* = = = = = = = = = = = = = = = = */

gdouble
hb_qif_parser_get_amount(gchar *string)
{
gdouble amount;
gint l, i;
gchar *new_str, *p;
gint  ndcount = 0;
gchar dc;

	//DB( g_print("\n[qif] hb_qif_parser_get_amount\n") );


	amount = 0.0;
	dc = '?';

	l = strlen(string) - 1;

	// the first non-digit is a grouping, or a decimal separator
	// if the non-digit is after a 3 digit serie, it might be a grouping

	for(i=l;i>=0;i--)
	{
		//DB( g_print(" %d :: %c :: ds='%c' ndcount=%d\n", i, string[i], dc, ndcount) );
		
		if( string[i] == '-' || string[i] == '+' ) continue;

		if( g_ascii_isdigit( string[i] ))
		{
			ndcount++;
		}
		else
		{
			if( (ndcount != 3) && (string[i] == '.' || string[i]==',') )
			{	
				dc = string[i];
			}
			ndcount = 0;
		}
	}

	//DB( g_print(" s='%s' :: ds='%c'\n", string, dc) );


	new_str = g_malloc (l+3);   //#1214077
	p = new_str;
	for(i=0;i<=l;i++)
	{
		if( g_ascii_isdigit( string[i] ) || string[i] == '-' )
		{
			*p++ = string[i];
		}
		else
			if( string[i] == dc )
				*p++ = '.';
	}
	*p++ = '\0';
	amount = g_ascii_strtod(new_str, NULL);

	//DB( g_print(" -> amount was='%s' => to='%s' double='%f'\n", string, new_str, amount) );

	g_free(new_str);

	return amount;
}

/*	O if m-d-y (american)
	1 if d-m-y (european) */
/* obsolete 4.5
static gint
hb_qif_parser_guess_datefmt(QifContext *ctx)
{
gboolean retval = TRUE;
GList *qiflist;
gboolean r, valid;
gint d, m, y;

	DB( g_print("(qif) get_datetype\n") );

	qiflist = g_list_first(ctx->q_tra);
	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;

		r = hb_qif_parser_get_dmy(item->date, &d, &m, &y);
		valid = g_date_valid_dmy(d, m, y);

		DB( g_print(" -> date: %s :: %d %d %d :: %d\n", item->date, d, m, y, valid ) );

		if(valid == FALSE)
		{
			retval = FALSE;
			break;
		}

		qiflist = g_list_next(qiflist);
	}

	return retval;
}
*/

static Transaction *
account_qif_get_child_transfer(Transaction *src, GList *list)
{
Transaction *item;

	DB( g_print(" \n[qif] get_child_transfer\n") );

	DB( g_print(" search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->kacc, src->kxferacc) );

	list = g_list_first(list);
	while (list != NULL)
	{
		item = list->data;
		if( item->paymode == PAYMODE_INTXFER)
		{
			if( src->date == item->date &&
			    src->kacc == item->kxferacc &&
			    src->kxferacc == item->kacc &&
			    ABS(src->amount) == ABS(item->amount) )
			{
				DB( g_print(" found : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->kacc, item->kxferacc) );

				return item;
			}
		}
		list = g_list_next(list);
	}

	DB( g_print(" not found...\n") );

	return NULL;
}


static gint
hb_qif_parser_get_block_type(gchar *qif_line)
{
gchar **typestr;
gint type = QIF_NONE;

	DB( g_print("--------\n[qif] block type\n") );

	//DB( g_print(" -> str: %s type: %d\n", qif_line, type) );


	if(g_str_has_prefix(qif_line, "!Account") || g_str_has_prefix(qif_line, "!account"))
	{
		type = QIF_ACCOUNT;
	}
	else
	{
		typestr = g_strsplit(qif_line, ":", 2);

		if( g_strv_length(typestr) == 2 )
		{
			gchar *qif_line = g_utf8_casefold(typestr[1], -1);

			//DB( g_print(" -> str[1]: %s\n", typestr[1]) );

			if( g_str_has_prefix(qif_line, "bank") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "cash") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "ccard") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "invst") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "oth a") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "oth l") )
			{
				type = QIF_TRANSACTION;
			}
			else
			if( g_str_has_prefix(qif_line, "security") )
			{
				type = QIF_SECURITY;
			}
			else
			if( g_str_has_prefix(qif_line, "prices") )
			{
				type = QIF_PRICES;
			}

			g_free(qif_line);
		}
		g_strfreev(typestr);
	}

	//DB( g_print(" -> return type: %d\n", type) );


	return type;
}

static void
hb_qif_parser_parse(QifContext *ctx, gchar *filename, const gchar *encoding)
{
GIOChannel *io;
QIF_Tran tran = { 0 };

	DB( g_print("\n[qif] hb_qif_parser_parse\n") );

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *qif_line;
	GError *err = NULL;
	gint io_stat;
	gint type = QIF_NONE;
	gchar *value = NULL;
	gchar *cur_acc;

		DB( g_print(" -> encoding should be %s\n", encoding) );
		if( encoding != NULL )
		{
			g_io_channel_set_encoding(io, encoding, NULL);
		}

		DB( g_print(" -> encoding is %s\n", g_io_channel_get_encoding(io)) );

		cur_acc = g_strdup(QIF_UNKNOW_ACCOUNT_NAME);

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &qif_line, NULL, NULL, &err);

			if( io_stat == G_IO_STATUS_EOF )
				break;
			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_NORMAL )
			{
				hb_string_strip_crlf(qif_line);

				//DB (g_print("** new QIF line: '%s' **\n", qif_line));

				//start qif parsing
				if(g_str_has_prefix(qif_line, "!")) /* !Type: or !Option: or !Account otherwise ignore */
				{
					type = hb_qif_parser_get_block_type(qif_line);
					DB ( g_print("-> ---- QIF block: '%s' (type = %d) ----\n", qif_line, type) );
				}

				value = &qif_line[1];

				if( type == QIF_ACCOUNT )
				{
					switch(qif_line[0])
					{
						case 'N':   // Name
						{
							g_free(cur_acc);
							g_strstrip(value);
							cur_acc = g_strdup(value);
							DB ( g_print(" name: '%s'\n", value) );
							break;
						}

						case 'T':   // Type of account
						{

							DB ( g_print(" type: '%s'\n", value) );
							break;
						}
						
						case 'L':   // Credit limit (only for credit card accounts)
						if(g_str_has_prefix(qif_line, "L"))
						{

							DB ( g_print(" credit limit: '%s'\n", value) );
							break;
						}

						case '$':   // Statement balance amount
						{

							DB ( g_print(" balance: '%s'\n", value) );
							break;
						}

						case '^':   // end
						{
							DB ( g_print("should create account '%s' here\n", cur_acc) );

							DB ( g_print(" ----------------\n") );
							break;
						}
					}
				}

				if( type == QIF_TRANSACTION )
				{
					switch(qif_line[0])
					{
						case 'D':   //date
						{
						gchar *ptr;

							// US Quicken seems to be using the ' to indicate post-2000 two-digit years
							//(such as 01/01'00 for Jan 1 2000)
							ptr = g_strrstr (value, "\'");
							if(ptr != NULL) { *ptr = '/'; }

							ptr = g_strrstr (value, " ");
							if(ptr != NULL) { *ptr = '0'; }

							g_free(tran.date);
							tran.date = g_strdup(value);
							break;
						}

						case 'T':   // amount
						{
							tran.amount = hb_qif_parser_get_amount(value);
							break;
						}

						case 'C':   // cleared status
						{
							tran.reconciled = FALSE;
							if(g_str_has_prefix(value, "X") || g_str_has_prefix(value, "R") )
							{
								tran.reconciled = TRUE;
							}
							tran.cleared = FALSE;
							if(g_str_has_prefix(value, "*") || g_str_has_prefix(value, "c") )
							{
								tran.cleared = TRUE;
							}
							break;
						}

						case 'N':   // check num or reference number
						{
							if(*value != '\0')
							{
								g_free(tran.info);
								g_strstrip(value);
								tran.info = g_strdup(value);
							}
							break;
						}

						case 'P':   // payee
						{
							if(*value != '\0')
							{
								g_free(tran.payee);
								g_strstrip(value);
								tran.payee = g_strdup(value);
							}
							break;
						}

						case 'M':   // memo
						{
							if(*value != '\0')
							{
								g_free(tran.memo);
								tran.memo = g_strdup(value);
							}
							break;
						}

						case 'L':   // category
						{
							// LCategory of transaction
							// L[Transfer account name]
							// LCategory of transaction/Class of transaction
							// L[Transfer account]/Class of transaction
							// this is managed at insertion
							if(*value != '\0')
							{
								g_free(tran.category);
								g_strstrip(value);
								tran.category = g_strdup(value);
							}
							break;
						}

						case 'S':
						case 'E':
						case '$':
						{
							if(tran.nb_splits < TXN_MAX_SPLIT)
							{
								switch(qif_line[0])
								{
									case 'S':   // split category
									{
									QIFSplit *s = &tran.splits[tran.nb_splits];
										if(*value != '\0')
										{
											g_free(s->category);
											g_strstrip(value);
											s->category = g_strdup(value);
										}
										break;
									}
									
									case 'E':   // split memo
									{
									QIFSplit *s = &tran.splits[tran.nb_splits];
										if(*value != '\0')
										{
											g_free(s->memo);
											s->memo = g_strdup(value);
										}
										break;
									}

									case '$':   // split amount
									{
									QIFSplit *s = &tran.splits[tran.nb_splits];
					
										s->amount = hb_qif_parser_get_amount(value);
										// $ line normally end a split
										#if MYDEBUG == 1
										g_print(" -> new split added: [%d] S=%s, E=%s, $=%.2f\n", tran.nb_splits, s->category, s->memo, s->amount);
										#endif
					
										tran.nb_splits++;						
										break;
									}
								}
								
							}
							// end split
							break;
						}
							
						case '^':   // end of line
						{
						QIF_Tran *newitem;

							//fix: 380550
							if( tran.date )
							{
								tran.account = g_strdup(cur_acc);

								DB ( g_print(" -> store qif txn: dat:'%s' amt:%.2f pay:'%s' mem:'%s' cat:'%s' acc:'%s' nbsplit:%d\n", tran.date, tran.amount, tran.payee, tran.memo, tran.category, tran.account, tran.nb_splits) );

								newitem = da_qif_tran_malloc();
								da_qif_tran_move(&tran, newitem);
								da_qif_tran_append(ctx, newitem);
							}

							//unvalid tran
							tran.date = 0;
							//todo: should clear mem alloc here
						
							tran.nb_splits = 0;
							break;
						}

					}
					// end of switch

				}   
				// end QIF_TRANSACTION
			}
			// end of stat normal
			g_free(qif_line);
		}
		// end of for loop

		g_free(cur_acc);
		g_io_channel_unref (io);
	}

}


/*
** this is our main qif entry point
*/
GList *
account_import_qif(gchar *filename, ImportContext *ictx)
{
QifContext ctx = { 0 };
GList *qiflist;
GList *list = NULL;

	DB( g_print("\n[qif] account import qif\n") );

	// allocate our GLists
	da_qif_tran_new(&ctx);
	ctx.is_ccard = FALSE;

	// parse !!
	hb_qif_parser_parse(&ctx, filename, ictx->encoding);

	// check iso date format in file
	//isodate = hb_qif_parser_check_iso_date(&ctx);
	//DB( g_print(" -> date is dd/mm/yy: %d\n", isodate) );

	DB( g_print("\n\n -> start transform all qif txn to hb txn\n") );

	DB( g_print(" -> %d qif txn\n",  g_list_length(ctx.q_tra)) );

	// transform our qif transactions to homebank ones
	qiflist = g_list_first(ctx.q_tra);
	while (qiflist != NULL)
	{
	QIF_Tran *item = qiflist->data;
	Transaction *newope, *child;
	Account *accitem;
	Payee *payitem;
	Category *catitem;
	gchar *name, *tmpmemo, *tmppayee;
	gint nsplit;

		newope = da_transaction_malloc();

		newope->date		 = hb_date_get_julian(item->date, ictx->datefmt);
		if( newope->date == 0 )
			ictx->cnt_err_date++;
		
		//newope->paymode	 = atoi(str_array[1]);
		//newope->info		 = g_strdup(str_array[2]);

		//#916690 manage memo, swap memo/payee
		tmpmemo  = item->memo;
		tmppayee = item->payee;
		if( PREFS->dtex_qifswap )
		{
			tmpmemo  = item->payee;
			tmppayee  = item->memo;
		}

		if( PREFS->dtex_qifmemo )
			newope->memo	 = g_strdup(tmpmemo);
		
		newope->info		 = g_strdup(item->info);
		newope->amount		 = item->amount;

		//#773282 invert amount for ccard accounts
		if(ctx.is_ccard)
			newope->amount *= -1;

		// payee + append
		if( tmppayee != NULL )
		{
			payitem = da_pay_get_by_name(tmppayee);
			if(payitem == NULL)
			{
				//DB( g_print(" -> append pay: '%s'\n", tmppayee ) );

				payitem = da_pay_malloc();
				payitem->name = g_strdup(tmppayee);
				payitem->imported = TRUE;
				da_pay_append(payitem);

				ictx->cnt_new_pay += 1;
			}
			newope->kpay = payitem->key;
		}

		// LCategory of transaction
		// L[Transfer account name]
		// LCategory of transaction/Class of transaction
		// L[Transfer account]/Class of transaction
		if( item->category != NULL )
		{
			if(g_str_has_prefix(item->category, "["))	// this is a transfer account name
			{
			gchar *accname;

				//DB ( g_print(" -> transfer to: '%s'\n", item->category) );

				//remove brackets
				accname = hb_strdup_nobrackets(item->category);

				accitem = import_create_account(accname, NULL);

				newope->kxferacc = accitem->key;
				newope->paymode = PAYMODE_INTXFER;

				g_free(accname);
			}
			else
			{
				//DB ( g_print(" -> append cat: '%s'\n", item->category) );

				catitem = da_cat_append_ifnew_by_fullname(item->category, TRUE );
				if( catitem != NULL )
				{
					ictx->cnt_new_cat += 1;
					newope->kcat = catitem->key;
				}
			}
		}

		// splits, if not a xfer
		if( newope->paymode != PAYMODE_INTXFER )
		{
			for(nsplit=0;nsplit<item->nb_splits;nsplit++)
			{
			QIFSplit *s = &item->splits[nsplit];
			Split *hbs;
			guint32 kcat = 0;
		
				DB( g_print(" -> append split %d: '%s' '%.2f' '%s'\n", nsplit, s->category, s->amount, s->memo) );

				if( s->category != NULL )
				{
					catitem = da_cat_append_ifnew_by_fullname(s->category, TRUE ); // TRUE = imported
					if( catitem != NULL )
					{
						kcat = catitem->key;
					}
				}

				hbs = da_split_new(kcat, s->amount, s->memo);
				
				da_splits_append(newope->splits, hbs);
				
				//da_transaction_splits_append(newope, hbs);
				hbs = NULL;				
			}
		}

		// account + append
		name = strcmp(QIF_UNKNOW_ACCOUNT_NAME, item->account) == 0 ? "" : item->account;

		DB( g_print(" -> account name is '%s'\n", name ) );

		accitem = import_create_account(name, NULL);

		newope->kacc = accitem->key;

		newope->flags |= OF_ADDED;
		if( newope->amount > 0 )
			newope->flags |= OF_INCOME;

		if( item->reconciled )
			newope->status = TXN_STATUS_RECONCILED;
		else
		if( item->cleared )
			newope->status = TXN_STATUS_CLEARED;
		
		child = account_qif_get_child_transfer(newope, list);
		if( child != NULL)
		{
			//DB( g_print(" -> transaction already exist\n" ) );
			da_transaction_free(newope);
		}
		else
		{
			//DB( g_print(" -> append trans. acc:'%s', memo:'%s', val:%.2f\n", item->account, item->memo, item->amount ) );
			list = g_list_append(list, newope);
		}

		qiflist = g_list_next(qiflist);
	}

	// destroy our GLists
	da_qif_tran_destroy(&ctx);

	DB( g_print(" -> %d txn converted\n", g_list_length(list)) );
	DB( g_print(" -> %d errors\n", ictx->cnt_err_date) );

	return list;
}

