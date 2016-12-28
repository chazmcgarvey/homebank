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


static gchar *hb_csv_strndup (gchar *str, gsize n)
{
gchar *new_str;
gchar *twoquote;

	if (str)
	{
		new_str = g_new (gchar, n + 1);
		if(*str=='\"')
		{
			str++; n--;
		}
		if(str[n-1]=='\"')
			n--;
		
		strncpy (new_str, str, n);
		new_str[n] = '\0';
		
		// replace ""
		twoquote = strstr(new_str, "\"\"");
		if(twoquote)
			strcpy (twoquote, twoquote+1);
		
		//todo: replace &amp; &lt; &gt; &apos; &quot; ??
		
	}
	else
		new_str = NULL;

	return new_str;
}


static gchar *hb_csv_find_delimiter(gchar *string)
{
 gchar *s = string;
gboolean enclosed = FALSE;

	while( *s != '\0' )
	{
		if( *s == ';' && enclosed == FALSE )
			break;

		if( *s == '\"' )
		{
			enclosed = !enclosed;
		}
	
		s++;
	}
	
	return s;
}


gboolean hb_csv_row_valid(gchar **str_array, guint nbcolumns, gint *csvtype)
{
gboolean valid = TRUE;
guint i;
extern int errno;

#if MYDEBUG == 1
gchar *type[5] = { "string", "date", "int", "double" };
gint lasttype;
#endif

	DB( g_print("\n** hb_string_csv_valid: init %d\n", valid) );

	DB( g_print(" -> length %d, nbcolumns %d\n", g_strv_length( str_array ), nbcolumns) );

	if( g_strv_length( str_array ) != nbcolumns )
	{
		valid = FALSE;
		goto csvend;
	}

	for(i=0;i<nbcolumns;i++)
	{
#if MYDEBUG == 1
		lasttype = csvtype[i];
#endif

		if(valid == FALSE)
		{
			DB( g_print(" -> fail on column %d, type: %s\n", i, type[lasttype]) );
			break;
		}

		DB( g_print(" -> control column %d, type: %d, valid: %d '%s'\n", i, lasttype, valid, str_array[i]) );

		switch( csvtype[i] )
		{
			case CSV_DATE:
				valid = hb_string_isdate(str_array[i]);
				break;
			case CSV_STRING:
				valid = hb_string_isprint(str_array[i]);
				break;
			case CSV_INT:
				valid = hb_string_isdigit(str_array[i]);
				break;
			case CSV_DOUBLE	:
			
				//todo: use strtod (to take care or . or ,)
				g_ascii_strtod(str_array[i], NULL);
				//todo : see this errno
				if( errno )
				{
					DB( g_print("errno: %d\n", errno) );
					valid = FALSE;
				}
				break;
		}
	}

csvend:

	DB( g_print(" --> return %d\n", valid) );

	return valid;
}


gchar **hb_csv_row_get(gchar *string, gchar *delimiter, gint max_tokens)
{
GSList *string_list = NULL, *slist;
gchar **str_array, *s;
guint n = 0;
gchar *remainder;

	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (delimiter != NULL, NULL);
	g_return_val_if_fail (delimiter[0] != '\0', NULL);

	if (max_tokens < 1)
		max_tokens = G_MAXINT;

	remainder = string;
	s = hb_csv_find_delimiter (remainder);
	if (s)
	{
	gsize delimiter_len = strlen (delimiter);

		while (--max_tokens && s && *s != '\0')
		{
		gsize len;

			len = s - remainder;
			string_list = g_slist_prepend (string_list, hb_csv_strndup (remainder, len));
			DB( g_print("   stored=[%s]\n", (gchar *)string_list->data) );

			n++;
			remainder = s + delimiter_len;
			s = hb_csv_find_delimiter (remainder);
		}
	}
	if (*string)
	{
	gsize len;
	
		len = s - remainder;
		n++;
		string_list = g_slist_prepend (string_list, hb_csv_strndup (remainder, len));
		DB( g_print("   stored=[%s]\n", (gchar *)string_list->data) );
	}

	str_array = g_new (gchar*, n + 1);

	str_array[n--] = NULL;
	for (slist = string_list; slist; slist = slist->next)
		str_array[n--] = slist->data;

	g_slist_free (string_list);

	return str_array;
}


GList *homebank_csv_import(gchar *filename, ImportContext *ictx)
{
GIOChannel *io;
GList *list = NULL;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};

	DB( g_print("\n[import] homebank csv\n") );

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr;
	gsize length;
	gint io_stat;
	gboolean isvalid;
	gint count = 0;
	gint error = 0;
	Account *tmp_acc;
	Payee *payitem;
	Category *catitem;
	GError *err = NULL;


		gchar *accname = g_strdup_printf(_("(account %d)"), da_acc_get_max_key() + 1);
		tmp_acc = import_create_account(accname, NULL);
		g_free(accname);


		if( ictx->encoding != NULL )
		{
			g_io_channel_set_encoding(io, ictx->encoding, NULL);
		}

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, &length, NULL, &err);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( *tmpstr != '\0' )
				{
				gchar **str_array;

					count++;

					hb_string_strip_crlf(tmpstr);
					DB( g_print("\n (row-%04d) ->|%s|<-\n", count, tmpstr) );

					// 0:date; 1:paymode; 2:info; 3:payee, 4:wording; 5:amount; 6:category; 7:tags
					str_array = hb_csv_row_get(tmpstr, ";", 8);
					isvalid = hb_csv_row_valid(str_array, 8, csvtype);

					 DB( g_print(" valid %d, '%s'\n", isvalid, tmpstr) );

					if( !isvalid )
					{
						g_warning ("csv parse: line %d, invalid column count or data", count);
						error++;
						//todo log line in error to report user
					}
					else
					{
					Transaction *newope = da_transaction_malloc();

						//DB( g_print(" ->%s\n", tmpstr ) );

						newope->date		 = hb_date_get_julian(str_array[0], ictx->datefmt);
						if( newope->date == 0 )
						{
							g_warning ("csv parse: line %d, parse date failed", count);
							ictx->cnt_err_date++;
						}
						
						newope->paymode		 = atoi(str_array[1]);
						newope->info		 = g_strdup(str_array[2]);

						/* payee */
						g_strstrip(str_array[3]);
						payitem = da_pay_get_by_name(str_array[3]);
						if(payitem == NULL)
						{
							payitem = da_pay_malloc();
							payitem->name = g_strdup(str_array[3]);
							payitem->imported = TRUE;
							da_pay_append(payitem);

							if( payitem->imported == TRUE )
								ictx->cnt_new_pay += 1;
						}

						newope->kpay = payitem->key;
						newope->wording		 = g_strdup(str_array[4]);
						newope->amount		 = hb_qif_parser_get_amount(str_array[5]);

						/* category */
						g_strstrip(str_array[6]);
						catitem = da_cat_append_ifnew_by_fullname(str_array[6], TRUE);
						if( catitem != NULL )
						{
							newope->kcat = catitem->key;

							if( catitem->imported == TRUE && catitem->key > 0 )
								ictx->cnt_new_cat += 1;
						}

						/* tags */
						transaction_tags_parse(newope, str_array[7]);


						newope->kacc		= tmp_acc->key;
						//newope->kxferacc = accnum;

						newope->flags |= OF_ADDED;

						if( newope->amount > 0)
							newope->flags |= OF_INCOME;

						/*
						DB( g_print(" storing %s : %s : %s :%s : %s : %s : %s : %s\n",
							str_array[0], str_array[1], str_array[2],
							str_array[3], str_array[4], str_array[5],
							str_array[6], str_array[7]
							) );
						*/

						list = g_list_append(list, newope);

						g_strfreev (str_array);
					}
				}
				g_free(tmpstr);
			}

		}
		g_io_channel_unref (io);

	/*
		ui_dialog_msg_infoerror(data->window, error > 0 ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
			_("Transaction CSV import result"),
			_("%d transactions inserted\n%d errors in the file"),
			count, error);
		*/
	}


	return list;
}


