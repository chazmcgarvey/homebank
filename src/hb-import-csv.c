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


static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};



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


static gchar *hb_csv_find_delimiter(gchar *string, gchar delimiter)
{
 gchar *s = string;
gboolean enclosed = FALSE;

	while( *s != '\0' )
	{
		if( (*s == delimiter) && (enclosed == FALSE) )
			break;

		if( *s == '\"' )
		{
			enclosed = !enclosed;
		}
	
		s++;
	}
	
	return s;
}


static gboolean hb_csv_row_valid(gchar **str_array, guint nbcolumns, gint *csvtype)
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


static gchar **hb_csv_row_get(gchar *string, gchar delimiter, gint max_tokens)
{
GSList *string_list = NULL, *slist;
gchar **str_array, *s;
guint n = 0;
gchar *remainder;

	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (delimiter != '\0', NULL);

	if (max_tokens < 1)
		max_tokens = G_MAXINT;

	remainder = string;
	s = hb_csv_find_delimiter (remainder, delimiter);
	if (s)
	{
		while (--max_tokens && s && *s != '\0')
		{
		gsize len;

			len = s - remainder;
			string_list = g_slist_prepend (string_list, hb_csv_strndup (remainder, len));
			DB( g_print("   stored=[%s]\n", (gchar *)string_list->data) );

			n++;
			remainder = s + 1;
			s = hb_csv_find_delimiter (remainder, delimiter);
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


static gchar hb_csv_get_separator(void)
{
static const gchar sep[] = PRF_DTEX_CSVSEP_BUFFER;
	return sep[PREFS->dtex_csvsep];	
}


gboolean hb_csv_test_line(gchar *rawline)
{
gchar **str_array;
gchar sep;
gboolean isvalid = FALSE;

	hb_string_strip_crlf(rawline);
	sep = hb_csv_get_separator();
	str_array = hb_csv_row_get(rawline, sep, 8);
	isvalid = hb_csv_row_valid(str_array, 8, csvtype);

	g_strfreev (str_array);

	return isvalid;
}


GList *homebank_csv_import(ImportContext *ictx, GenFile *genfile)
{
GIOChannel *io;
//GList *list = NULL;

	DB( g_print("\n[import] homebank csv\n") );

	io = g_io_channel_new_file(genfile->filepath, "r", NULL);
	if(io != NULL)
	{
	gchar *tmpstr;
	gchar sep;
	gsize length;
	gint io_stat;
	gboolean isvalid;
	gint count = 0;
	gint error = 0;
	GenAcc *newacc;
	GError *err = NULL;


		newacc = hb_import_gen_acc_get_next(ictx, FILETYPE_CSV_HB, NULL, NULL);

		if( genfile->encoding != NULL )
		{
			g_io_channel_set_encoding(io, genfile->encoding, NULL);
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

					sep = hb_csv_get_separator();

					hb_string_strip_crlf(tmpstr);
					str_array = hb_csv_row_get(tmpstr, sep, 8);

					// 0:date; 1:paymode; 2:info; 3:payee, 4:wording; 5:amount; 6:category; 7:tags
					isvalid = hb_csv_row_valid(str_array, 8, csvtype);

					DB( g_print("\n (row-%04d) ->|%s|<-\n", count, tmpstr) );
					DB( g_print(" valid %d, '%s'\n", isvalid, tmpstr) );

					if( !isvalid )
					{
						g_warning ("csv parse: line %d, invalid column count or data", count);
						error++;
						//todo log line in error to report user
					}
					else
					{
					GenTxn *newope = da_gen_txn_malloc();;

						DB( g_print(" adding txn\n" ) );

						/* convert to generic transaction */
						newope->date		= g_strdup(str_array[0]);			
						newope->paymode		= atoi(str_array[1]);
						//todo: reinforce controls here
						// csv file are standalone, so no way to link a target txn
						//added 5.1.8 forbid to import 5=internal xfer
						if(newope->paymode == PAYMODE_INTXFER)
							newope->paymode = PAYMODE_XFER;
						newope->rawinfo		= g_strdup(str_array[2]);
						newope->rawpayee	= g_strdup(g_strstrip(str_array[3]));						
						newope->rawmemo		= g_strdup(str_array[4]);
						newope->amount		= hb_qif_parser_get_amount(str_array[5]);
						newope->category	= g_strdup(g_strstrip(str_array[6]));
						newope->tags		= g_strdup(str_array[7]);
						newope->account		= g_strdup(newacc->name);

						/* todo: move this eval date valid */
						//guint32 juliantmp = hb_date_get_julian(str_array[0], ictx->datefmt);
						///if( juliantmp == 0 )
						//	ictx->cnt_err_date++;

						/*
						DB( g_print(" storing %s : %s : %s :%s : %s : %s : %s : %s\n",
							str_array[0], str_array[1], str_array[2],
							str_array[3], str_array[4], str_array[5],
							str_array[6], str_array[7]
							) );
						*/

						da_gen_txn_append(ictx, newope);

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


	return ictx->gen_lst_txn;
}


