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

static void
hb_qif_parser_parse(ImportContext *ctx, GenFile *genfile);

/* = = = = = = = = = = = = = = = = */

GList *homebank_qif_import(ImportContext *ictx, GenFile *genfile)
{
	DB( g_print("\n[import] homebank QIF\n") );

	hb_qif_parser_parse(ictx, genfile);

	return ictx->gen_lst_txn;;
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
hb_qif_parser_guess_datefmt(ImportContext *ctx)
{
gboolean retval = TRUE;
GList *qiflist;
gboolean r, valid;
gint d, m, y;

	DB( g_print("(qif) get_datetype\n") );

	qiflist = g_list_first(ctx->gen_lst_txn);
	while (qiflist != NULL)
	{
	GenTxn *item = qiflist->data;

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
hb_qif_parser_parse(ImportContext *ctx, GenFile *genfile)
{
GIOChannel *io;
GenTxn tran = { 0 };

	DB( g_print("\n[qif] hb_qif_parser_parse\n") );

	io = g_io_channel_new_file(genfile->filepath, "r", NULL);
	if(io != NULL)
	{
	gchar *qif_line;
	GError *err = NULL;
	gint io_stat;
	gint type = QIF_NONE;
	gchar *value = NULL;
	GenAcc tmpgenacc = { 0 };
	GenAcc *genacc;

		DB( g_print(" -> encoding should be %s\n", genfile->encoding) );
		if( genfile->encoding != NULL )
		{
			g_io_channel_set_encoding(io, genfile->encoding, NULL);
		}

		DB( g_print(" -> encoding is %s\n", g_io_channel_get_encoding(io)) );

		// within a single qif file, if there is no accoutn data
		// then txn are related to a single account
		genacc = NULL;

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
							g_strstrip(value);
							tmpgenacc.name = g_strdup(value);
							DB ( g_print(" name: '%s'\n", value) );
							break;
						}

						case 'T':   // Type of account
						{
							DB ( g_print(" type: '%s'\n", value) );
							// added for 5.0.1
							if( g_ascii_strcasecmp("CCard", value) == 0 )
							{
								tmpgenacc.is_ccard = TRUE;
							}
							break;
						}
						/*
						case 'D':   // Description
						{

							DB ( g_print(" description: '%s'\n", value) );
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
						}*/

						case '^':   // end
						{
						Account *dst_acc;
						
							genacc = hb_import_gen_acc_get_next (ctx, FILETYPE_QIF, tmpgenacc.name, NULL);
							dst_acc = hb_import_acc_find_existing(tmpgenacc.name, NULL );
							if( dst_acc != NULL )
							{
								DB( g_print(" - set dst_acc to %d\n", dst_acc->key) );
								genacc->kacc = dst_acc->key;
							}
							genacc->is_ccard = tmpgenacc.is_ccard;
							
							g_free(tmpgenacc.name);
							tmpgenacc.name = NULL;
							tmpgenacc.is_ccard = FALSE;

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
								tran.rawpayee = g_strdup(value);
							}
							break;
						}

						case 'M':   // memo
						{
							if(*value != '\0')
							{
								g_free(tran.memo);
								tran.rawmemo = g_strdup(value);
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
									GenSplit *s = &tran.splits[tran.nb_splits];
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
									GenSplit *s = &tran.splits[tran.nb_splits];
										if(*value != '\0')
										{
											g_free(s->memo);
											s->memo = g_strdup(value);
										}
										break;
									}

									case '$':   // split amount
									{
									GenSplit *s = &tran.splits[tran.nb_splits];
					
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
						GenTxn *newitem;

							//fix: 380550
							if( tran.date )
							{
								//ensure we have an account
								//todo: check this
								if(genacc == NULL)
								{
									genacc = hb_import_gen_acc_get_next (ctx, FILETYPE_QIF, NULL, NULL);
								}
								
								tran.account = g_strdup(genacc->name);

								DB ( g_print(" -> store qif txn: dat:'%s' amt:%.2f pay:'%s' mem:'%s' cat:'%s' acc:'%s' nbsplit:%d\n", tran.date, tran.amount, tran.payee, tran.memo, tran.category, tran.account, tran.nb_splits) );

								newitem = da_gen_txn_malloc();
								da_gen_txn_move(&tran, newitem);
								da_gen_txn_append(ctx, newitem);
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

		g_io_channel_unref (io);
	}

}



