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

#ifndef __HB_ACCOUNT_H__
#define __HB_ACCOUNT_H__



typedef struct _account		Account;


struct _account
{
	guint32		key;
	gushort		flags;
	gushort		type;
	guint32		pos;		//display position
	guint32		kcur;
	gchar		*name;
	gchar		*number;
	gchar		*bankname;
	gdouble		initial;
	gdouble		warning;
	gdouble		minimum;
	guint32		cheque1;
	guint32		cheque2;
	gchar	    *notes;

	/* unsaved datas */
	GQueue		*txn_queue;
	GtkWindow	*window;	//dsp_account window opened

	gdouble     bal_bank;	//bank balance (reconciled transaction)
	gdouble     bal_today;	//today balance (every transaction until today)
	gdouble     bal_future;	//future balance (every transaction)

	gboolean	filter;		//true if selected into filter

	// import datas
	gboolean	imported;
	guint32		imp_key;	// 0 create new / x to map to existing
	gchar		*imp_name;  // name in the file
};

// 0 is free
#define AF_CLOSED		(1<<1)
#define AF_ADDED		(1<<2)
#define AF_CHANGED		(1<<3)
#define AF_NOSUMMARY	(1<<4)
#define AF_NOBUDGET		(1<<5)
#define AF_NOREPORT		(1<<6)

#define AF_OLDBUDGET	(1<<0)

enum
{
	ACC_TYPE_NONE       = 0,
	ACC_TYPE_BANK       = 1,	//Banque
	ACC_TYPE_CASH       = 2,	//Espèce
	ACC_TYPE_ASSET      = 3,	//Actif (avoir)
	ACC_TYPE_CREDITCARD = 4,	//Carte crédit
	ACC_TYPE_LIABILITY  = 5,	//Passif (dettes)
//	ACC_TYPE_STOCK      = 6,	//Actions
//	ACC_TYPE_MUTUALFUND = 7,	//Fond de placement
//	ACC_TYPE_INCOME     = 8,	//Revenus
//	ACC_TYPE_EXPENSE    = 9,	//Dépenses
//	ACC_TYPE_EQUITY     = 10,	//Capitaux propres
//	ACC_TYPE_,
	ACC_TYPE_MAXVALUE
};



Account *da_acc_malloc(void);
void da_acc_free(Account *item);
Account *da_acc_malloc(void);

void da_acc_destroy(void);
void da_acc_new(void);

guint		da_acc_length(void);
gboolean	da_acc_create_none(void);
gboolean	da_acc_remove(guint32 key);
gboolean	da_acc_insert(Account *acc);
gboolean	da_acc_append(Account *item);
guint32		da_acc_get_max_key(void);
Account		*da_acc_get_by_name(gchar *name);
Account		*da_acc_get_by_imp_name(gchar *name);
Account		*da_acc_get(guint32 key);
void da_acc_consistency(Account *item);


gboolean account_is_used(guint32 key);
gboolean account_exists(gchar *name);
gboolean account_rename(Account *item, gchar *newname);
void account_set_currency(Account *item, guint32 kcur);
void account_compute_balances(void);
gboolean account_balances_add(Transaction *trn);
gboolean account_balances_sub(Transaction *trn);

GList *account_glist_sorted(gint column);

void account_convert_euro(Account *acc);
#endif
