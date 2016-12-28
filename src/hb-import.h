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

#ifndef __HB_IMPORT_H__
#define __HB_IMPORT_H__


typedef struct _ImportContext ImportContext;
struct _ImportContext
{
	GList			*trans_list;	// trn storage
	gint 			next_acc_key;	//max key account when start

	gint			datefmt;
	const gchar		*encoding;

	gint			nb_src_acc, nb_new_acc;	
	gint			cnt_new_ope;
	gint			cnt_new_pay;
	gint			cnt_new_cat;
	gint			cnt_err_date;
	gint			nb_duplicate;
};


typedef struct _OfxContext OfxContext;
struct _OfxContext
{
	GList		*trans_list;
	Account 	*curr_acc;
	gboolean	curr_acc_isnew;
	ImportContext *ictx;
};


typedef struct _QifContext QifContext;
typedef struct _qif_split	QIFSplit;
typedef struct _qif_tran	QIF_Tran;

#define QIF_UNKNOW_ACCOUNT_NAME "(unknown)"

struct _QifContext
{
	GList   *q_acc;
	GList   *q_cat;
	GList   *q_pay;
	GList   *q_tra;

	gboolean	is_ccard;
};

struct _qif_split
{
	gchar		*category;
	gdouble		amount;
	gchar		*memo;
};

struct _qif_tran
{
	gchar		*account;
	gchar		*date;
	gdouble		amount;
	gboolean	reconciled;
	gboolean	cleared;
	gchar		*info;
	gchar		*payee;
	gchar		*memo;
	gchar		*category;

	gint		nb_splits;
	QIFSplit	splits[TXN_MAX_SPLIT];
};


enum QIF_Type
{
	QIF_NONE,
	QIF_HEADER,
	QIF_ACCOUNT,
	QIF_CATEGORY,
	QIF_CLASS,
	QIF_MEMORIZED,
	QIF_TRANSACTION,
	QIF_SECURITY,
	QIF_PRICES
};


GList *account_import_qif(gchar *filename, ImportContext *ictx);
gdouble hb_qif_parser_get_amount(gchar *string);



Account *import_create_account(gchar *name, gchar *number);
GList *homebank_ofx_import(gchar *filename, ImportContext *ictx);

gboolean hb_csv_row_valid(gchar **str_array, guint nbcolumns, gint *csvtype);
gchar **hb_csv_row_get(gchar *string, gchar *delimiter, gint max_tokens);

GList *homebank_csv_import(gchar *filename, ImportContext *ictx);

#endif

