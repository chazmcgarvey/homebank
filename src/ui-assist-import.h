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

#ifndef __HOMEBANK_IMPORT_H__
#define __HOMEBANK_IMPORT_H__


#define PAGE_WELCOME		0
#define PAGE_FILES		1
#define PAGE_IMPORT		2

enum
{
	LST_GENFILE_POINTER,
	LST_GENFILE_NAME,
	NUM_LST_FILE
};

enum
{
	LST_GENACC_NAME,
	LST_GENACC_KEY,
	NUM_LST_GENACC
};

enum
{
	LST_GENTXN_POINTER,
	NUM_LST_GENTXN
};

typedef struct _import_txndata ImpTxnData;


struct _import_txndata
{
	// account page
	GtkWidget   *IM_txn, *LB_txn;
	GtkWidget   *LB_acc_title;
	//GtkWidget   *LB_acc_count;
	GtkWidget   *LB_txn_title;
	GtkWidget	*BT_all, *BT_non, *BT_inv;
	GtkWidget   *CY_acc;
	GtkWidget	*IM_unamed;
	GtkWidget	*LV_gentxn;
	GtkWidget	*EX_duptxn;
	GtkWidget	*LV_duptxn;

	GtkWidget	*ST_stack;
	GtkWidget   *GR_misc;
	GtkWidget   *GR_msg;
	GtkWidget   *GR_date;
	GtkWidget   *GR_ofx;
	GtkWidget   *GR_qif;
	GtkWidget	*GR_select;

	GtkWidget   *CY_txn_dateorder;
	GtkWidget	*NB_txn_daygap;
	GtkWidget   *CM_txn_ucfirst;
	GtkWidget   *CY_txn_ofxname;
	GtkWidget   *CY_txn_ofxmemo;
	GtkWidget   *CM_txn_qifmemo;
	GtkWidget   *CM_txn_qifswap;
};


struct import_data
{
	GtkWidget	*assistant;

	//intro
	GtkWidget   *CM_dsta;

	// filechooser
	GtkWidget	*filechooser;
	GtkWidget   *LV_file;
	GtkWidget   *BT_file_add;
	GtkWidget   *BT_file_remove;

	//struct import_txndata txndata[TXN_MAX_ACCOUNT];
	GArray		*txndata;
	
	//summary
	GtkWidget   *TX_summary;

	// import context
	ImportContext	ictx;
	
};


struct import_target_data
{
	GtkWidget   *label1, *label2;
	GtkWidget	*getwidget1;
	GtkWidget	*getwidget2;
	GtkWidget	*radio[2];
};


GtkWidget *ui_import_assistant_new (gchar **paths);
Account *import_create_account(gchar *name, gchar *number);
const gchar *homebank_file_getencoding(gchar *filename);
gchar *homebank_utf8_ensure(gchar *buffer);


#endif

