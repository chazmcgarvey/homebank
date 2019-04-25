/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2019 Maxime DOYEN
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

typedef struct _generic_file	GenFile;

typedef struct _generic_acc		GenAcc;
typedef struct _generic_split	GenSplit;
typedef struct _generic_txn		GenTxn;


//those are guin32 special values
#define DST_ACC_GLOBAL	100001
#define DST_ACC_NEW		100002
#define DST_ACC_SKIP		100010


struct _generic_file
{
	guint32      key;
	gchar	     *filepath;
	gint		 filetype;
	const gchar	 *encoding;
	gint		 datefmt;
	gboolean	loaded;
	gboolean    invaliddatefmt;
};


struct _generic_acc
{
	guint32		kfile;
	gint		filetype;

	guint32		key;
	gchar	   *name;
	//maybe new user name
	gchar	   *number;
	guint32		kacc;			//100001 = NEW, 100002 = SKIP
	gboolean	is_dupcheck;	//if target account was checked for duplicate
	gboolean	is_ccard;
	gboolean	is_unamed;		//if src account has no name into file
	gdouble		initial;
	gint		n_txnall;		//nb of txn total
	gint		n_txnimp;		//nb of txn to import
	gint		n_txnbaddate;   //nb of txn with bad date
	gint		n_txnduplicate; //nb of txn with duplicate
};


struct _generic_split
{
	gchar		*category;
	gdouble		amount;
	gchar		*memo;
};


struct _generic_txn
{
	guint32		kfile;		//todo: remove this
	guint32		kacc;

	gchar		*account;

	gchar		*rawinfo;	//<n/a> ; check_number
	gchar	    *rawpayee;	//P ; name
	gchar	    *rawmemo;	//M ; memo
	
	gchar		*date;		//D ; date_posted
	gchar		*info;		//N ; <rawinfo> 
	gchar		*payee;
	gchar		*memo;
	gchar		*category;  //L
	gchar		*tags;		//<n/a>
	
	guint32		julian;
	gushort		paymode;	//<n/a> ; transactiontype
	gdouble		amount;		//T ; amount
	gboolean	reconciled; //R
	gboolean	cleared;	//C

	gboolean	to_import;
	gboolean	is_imp_similar;
	gboolean	is_dst_similar;

	gint		nb_splits;
	GenSplit	splits[TXN_MAX_SPLIT];
	GList		*lst_existing;
};


typedef struct _ImportContext ImportContext;
struct _ImportContext
{
	GList	   *gen_lst_file;

	GList	   *gen_lst_acc;
	GList	   *gen_lst_txn;
	guint32		gen_next_acckey;

	//to keep track of where we are
	guint32		curr_kfile;
	guint32		curr_kacc;


	// ofx stuff
	GenAcc 		*curr_acc;
	gboolean	curr_acc_isnew;

	gint		opt_dateorder;
	gint		opt_daygap;
	
	gint		opt_ofxname;
	gint		opt_ofxmemo;

	gboolean	opt_qifmemo;
	gboolean	opt_qifswap;
	gboolean	opt_ucfirst;

	//gboolean	is_ccard;

	//GList			*trans_list;	// trn storage
	//gint 			next_acc_key;	//max key account when start
	//gint			datefmt;
	//const gchar		*encoding;

	/*gint			nb_src_acc, nb_new_acc;	
	gint			cnt_new_ope;
	gint			cnt_new_pay;
	gint			cnt_new_cat;
	gint			cnt_err_date;
	gint			nb_duplicate;*/
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


void da_import_context_new(ImportContext *ctx);
void da_import_context_destroy(ImportContext *ctx);


GenFile *da_gen_file_malloc(void);
void da_gen_file_free(GenFile *genfile);
GenFile *da_gen_file_get(GList *lst_file, guint32 key);
GenFile *da_gen_file_append_from_filename(ImportContext *ictx, gchar *filename);

GenAcc *da_gen_acc_malloc(void);
void da_gen_acc_free(GenAcc *item);
GenAcc *da_gen_acc_get_by_key(GList *lst_acc, guint32 key);

GenTxn *da_gen_txn_malloc(void);
void da_gen_txn_free(GenTxn *item);
GList *da_gen_txn_sort(GList *list);

void da_gen_txn_destroy(ImportContext *ctx);
void da_gen_txn_new(ImportContext *ctx);
void da_gen_txn_move(GenTxn *sitem, GenTxn *ditem);
void da_gen_txn_append(ImportContext *ctx, GenTxn *item);

gchar *hb_import_filetype_char_get(GenAcc *acc);

GenAcc *hb_import_gen_acc_get_next(ImportContext *ictx, gint filetype, gchar *name, gchar *number);
gint hb_import_gen_acc_count_txn(ImportContext *ictx, GenAcc *genacc);

Transaction *hb_import_convert_txn(GenAcc *genacc, GenTxn *gentxn);


void hb_import_load_all(ImportContext *ictx);
gint hb_import_gen_txn_check_target_similar(ImportContext *ictx, GenAcc *genacc);
gint hb_import_gen_txn_check_duplicate(ImportContext *ictx, GenAcc *genacc);

gint hb_import_option_apply(ImportContext *ictx, GenAcc *genacc);

Account *import_create_account(gchar *name, gchar *number);
Account *hb_import_acc_find_existing(gchar *name, gchar *number);


void hb_import_apply(ImportContext *ictx);

GList *homebank_csv_import(ImportContext *ictx, GenFile *genfile);
GList *homebank_ofx_import(ImportContext *ictx, GenFile *genfile);
GList *homebank_qif_import(ImportContext *ictx, GenFile *genfile);

GList *account_import_qif(gchar *filename, ImportContext *ictx);
gdouble hb_qif_parser_get_amount(gchar *string);

gboolean hb_csv_row_valid(gchar **str_array, guint nbcolumns, gint *csvtype);
gchar **hb_csv_row_get(gchar *string, gchar *delimiter, gint max_tokens);

#if MYDEBUG
void _import_context_debug_file_list(ImportContext *ctx);
void _import_context_debug_acc_list(ImportContext *ctx);
void _import_context_debug_txn_list(ImportContext *ctx);
#endif

#endif

