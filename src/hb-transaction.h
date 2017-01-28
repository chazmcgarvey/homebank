/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2017 Maxime DOYEN
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


#ifndef __HB_TRANSACTION_H__
#define __HB_TRANSACTION_H__

#include "hb-split.h"


typedef struct _transaction	Transaction;


struct _transaction
{
	gdouble		amount;
	guint32		kacc;
	gushort		paymode;
	gushort		flags;
	guint32		kpay;
	guint32		kcat;
	gchar		*wording;

	guint32		date;
	gushort		pos;
	gushort     status;
	gchar		*info;
	guint32		*tags;
	guint32		kxfer;		//strong link xfer key
	guint32		kxferacc;
	
	Split		*splits[TXN_MAX_SPLIT+1];

	/* unsaved datas */
	GList		*same;		//used for import todo: change this
	guint32		kcur;
	gdouble		balance;
};

#include "hb-archive.h"

#define OF_OLDVALID	(1<<0)  //deprecated since 5.x
#define OF_INCOME	(1<<1)
#define OF_AUTO		(1<<2)	//scheduled
#define OF_ADDED	(1<<3)  //tmp flag
#define OF_CHANGED	(1<<4)  //tmp flag
#define OF_OLDREMIND	(1<<5)  //deprecated since 5.x
#define OF_CHEQ2	(1<<6)
#define OF_LIMIT	(1<<7)	//scheduled
#define OF_SPLIT	(1<<8)

typedef enum {
	TXN_STATUS_NONE,
	TXN_STATUS_CLEARED,
	TXN_STATUS_RECONCILED,
	TXN_STATUS_REMIND,
	//TXN_VOID
} HbTxnStatus;


Transaction *da_transaction_malloc(void);
Transaction *da_transaction_copy(Transaction *src_txn, Transaction *dst_txn);
Transaction *da_transaction_init_from_template(Transaction *txn, Archive *arc);
Transaction *da_transaction_clone(Transaction *src_item);
void da_transaction_clean(Transaction *item);
void da_transaction_free(Transaction *item);

GList *da_transaction_new(void);
void da_transaction_destroy(void);

void da_transaction_queue_sort(GQueue *queue);
GList *da_transaction_sort(GList *list);
gboolean da_transaction_prepend(Transaction *item);
gboolean da_transaction_insert_sorted(Transaction *item);


/*
** transaction edit type
*/
enum
{
	TRANSACTION_EDIT_ADD,
	TRANSACTION_EDIT_INHERIT,
	TRANSACTION_EDIT_MODIFY
};


guint da_transaction_length(void);
void transaction_add_treeview(Transaction *ope, GtkWidget *treeview, guint32 accnum);
void transaction_add(Transaction *ope, GtkWidget *treeview, guint32 accnum);

gboolean transaction_acc_move(Transaction *txn, guint32 okacc, guint32 nkacc);

Transaction *transaction_xfer_child_strong_get(Transaction *src);
void transaction_xfer_search_or_add_child(GtkWindow *parentwindow, Transaction *ope, gboolean manual);
void transaction_xfer_change_to_child(Transaction *ope, Transaction *child);
void transaction_xfer_sync_child(Transaction *ope, Transaction *child);
void transaction_xfer_remove_child(Transaction *src);
Transaction *transaction_old_get_child_transfer(Transaction *src);

guint transaction_tags_count(Transaction *ope);
void transaction_tags_clone(Transaction *src_txn, Transaction *dst_txn);
guint transaction_tags_parse(Transaction *ope, const gchar *tagstring);
gchar *transaction_tags_tostring(Transaction *ope);
gint transaction_auto_assign(GList *ope_list, guint32 key);

void da_transaction_consistency(Transaction *item);

#endif
