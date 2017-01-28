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

#include "homebank.h"

#include "ui-transaction.h"
#include "hb-transaction.h"
#include "gtk-dateentry.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-account.h"
#include "ui-split.h"


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


extern gchar *CYA_TYPE[];

gchar *CYA_OPERATION[] = {
	N_("Add transaction"),
	N_("Inherit transaction"),
	N_("Modify transaction")
};


gchar *CYA_TXN_STATUS[] = {
	N_("None"),
	N_("Cleared"),
	N_("Reconciled"),
	N_("Remind"),
	NULL
};


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void deftransaction_template_popover_populate(struct deftransaction_data *data, GList *srclist);

static void deftransaction_update(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gboolean sensitive;

	DB( g_print("\n[ui-transaction] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//# 1419476 empty category when no split either...
	if( (data->ope->flags & (OF_SPLIT)) )
	{
		//# 1416624 empty category when split
		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_grp), 0);
	}

	/* disable amount+category if split is set */
	sensitive = (data->ope->flags & (OF_SPLIT)) ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->ST_amount, sensitive);
	gtk_widget_set_sensitive(data->PO_grp, sensitive);
}

static void deftransaction_update_warnsign(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gboolean warning = FALSE;
gdouble amount;
gint amttype;
Category *cat;

	DB( g_print("\n[ui-transaction] update warning sign\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cat = ui_cat_comboboxentry_get(GTK_COMBO_BOX(data->PO_grp));
	if(cat != NULL && cat->key > 0)
	{
		amount = hb_amount_round(gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount)), 2);
		if(amount != 0.0)
		{
			amttype = (amount > 0) ? 1 : -1;
			warning = (category_type_get(cat) != amttype) ? TRUE : FALSE;
		}
	}

	if(warning)
		gtk_widget_show_all(data->IB_warnsign);
	else
		gtk_widget_hide(data->IB_warnsign);

}


static void deftransaction_update_transfer(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gboolean sensitive;
guint kacc, kdst;

	DB( g_print("\n[ui-transaction] update transfer\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	sensitive = TRUE;

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	if(kacc == 0) { sensitive = FALSE; goto end; }

	/* coherent seizure
	 * - target account selected
	 * - source != target
	 * - same currency
	 */
	if( gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode)) == PAYMODE_INTXFER )
	{
	Account *srcacc, *dstacc;

		kdst = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_accto));

		if(kdst == 0) { sensitive = FALSE; goto end; }
		if(kdst == kacc) {
			sensitive = FALSE;
			goto end;
		}

		srcacc = da_acc_get(kacc);
		dstacc = da_acc_get(kdst);
		if(srcacc->kcur != dstacc->kcur) {
			sensitive = FALSE;
		}
	}

end:
	DB( g_print(" sensitive %d\n", sensitive) );

	//#1437551
	//gtk_widget_set_sensitive(gtk_dialog_get_action_area(GTK_DIALOG (data->window)), sensitive);
	gtk_dialog_set_response_sensitive(GTK_DIALOG (data->window), GTK_RESPONSE_ACCEPT, sensitive);
	gtk_dialog_set_response_sensitive(GTK_DIALOG (data->window), GTK_RESPONSE_ADD, sensitive);
	gtk_dialog_set_response_sensitive(GTK_DIALOG (data->window), GTK_RESPONSE_ADDKEEP, sensitive);

}


static void deftransaction_update_payee(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Category *cat;
gint paymode;
Payee *pay;

	DB( g_print("\n[ui-transaction] update payee\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// only set for empty category
	// 1635053 and also paymode unset
	cat = ui_cat_comboboxentry_get(GTK_COMBO_BOX(data->PO_grp));
	paymode = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	if( (cat == NULL || cat->key == 0) && (paymode == PAYMODE_NONE) )
	{
		pay = ui_pay_comboboxentry_get(GTK_COMBO_BOX(data->PO_pay));
		if( pay != NULL )
		{
			g_signal_handlers_block_by_func (G_OBJECT (data->PO_grp), G_CALLBACK (deftransaction_update_warnsign), NULL);
			ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_grp), pay->kcat);
			g_signal_handlers_unblock_by_func (G_OBJECT (data->PO_grp), G_CALLBACK (deftransaction_update_warnsign), NULL);

			gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), pay->paymode);
		}
	}
}


static void deftransaction_update_accto(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
guint kacc;

	DB( g_print("\n[ui-transaction] update accto\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	DB( g_print(" acc is %d\n", kacc) );

	//g_signal_handlers_block_by_func (G_OBJECT (data->PO_accto), G_CALLBACK (deftransaction_update_transfer), NULL);
	//ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_accto), 0);
	//g_signal_handlers_unblock_by_func (G_OBJECT (data->PO_accto), G_CALLBACK (deftransaction_update_transfer), NULL);

	ui_acc_comboboxentry_populate_except(GTK_COMBO_BOX(data->PO_accto), GLOBALS->h_acc, kacc, ACC_LST_INSERT_NORMAL);

	deftransaction_update_transfer(widget, user_data);
}



void deftransaction_set_amount_from_split(GtkWidget *widget, gdouble amount)
{
struct deftransaction_data *data;

	DB( g_print("\n[ui-transaction] set_amount_from_split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("- amount=%.2f\n", amount) );

	data->ope->amount = amount;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount);
	
	deftransaction_update(widget, NULL);
	
}


static void deftransaction_set(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *entry;
gchar *tagstr, *txt;

	DB( g_print("\n[ui-transaction] set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_print(" - ope=%p data=%p tags:%p\n", data->ope, data, entry->tags) );

	//DB( g_print(" set date to %d\n", entry->date) );
	//g_object_set(GTK_DATE_ENTRY(data->PO_date), "date", (guint32)entry->ope_Date);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_date), (guint)entry->date);

	txt = (entry->wording != NULL) ? entry->wording : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_word), txt);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), entry->amount);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_amount), (entry->ope_Flags & OF_INCOME) ? 1 : 0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_cheque), (entry->flags & OF_CHEQ2) ? 1 : 0);

	txt = (entry->info != NULL) ? entry->info : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), txt);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_grp), entry->kcat);
	ui_pay_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_pay), entry->kpay);

	tagstr = transaction_tags_tostring(entry);

	DB( g_print(" - tags: '%s'\n", txt) );

	txt = (tagstr != NULL) ? tagstr : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_tags), txt);
	g_free(tagstr);

	radio_set_active(GTK_CONTAINER(data->RA_status), entry->status );
	
	//as we trigger an event on this
	//let's place it at the end to avoid misvalue on the trigger function
	g_signal_handlers_block_by_func (G_OBJECT (data->PO_acc), G_CALLBACK (deftransaction_update_accto), NULL);

	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_acc), entry->kacc);
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_accto), entry->kxferacc);
	
	g_signal_handlers_unblock_by_func (G_OBJECT (data->PO_acc), G_CALLBACK (deftransaction_update_accto), NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), entry->paymode);

	DB( g_print(" - acc is: %d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc)) ) );
}


void deftransaction_get(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *entry;
gchar *txt;
gdouble value;
gint active;

	DB( g_print("\n[ui-transaction] get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_print(" - ope = %p\n", entry) );

	//DB( g_print(" get date to %d\n", entry->ope_Date) );
	entry->date = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_date));
	//g_object_get(GTK_DATE_ENTRY(data->PO_date), "date", entry->ope_Date);

	//free any previous string
	if(	entry->wording )
	{
		g_free(entry->wording);
		entry->wording = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->wording = g_strdup(txt);
	}

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	entry->amount = value;

	/* for internal transfer add, amount must be expense */
	// #617936
	/*
	if( entry->paymode == PAYMODE_INTXFER && data->type == OPERATION_EDIT_ADD )
	{
		if( entry->amount > 0 )
			entry->amount *= -1;
	}
	*/

	//free any previous string
	if(	entry->info )
	{
		g_free(entry->info);
		entry->info = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->info = g_strdup(txt);
	}

	entry->paymode  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	entry->kcat     = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_grp));
	entry->kpay     = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_pay));
	entry->kacc     = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
	entry->kxferacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_accto));

	/* tags */
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tags));
	DB( g_print(" - tags: '%s'\n", txt) );
	transaction_tags_parse(entry, txt);

	entry->status = radio_get_active(GTK_CONTAINER(data->RA_status));

	//#1615245: moved here, after get combo entry key
	if( entry->paymode != PAYMODE_INTXFER )
	{
		//#677351: revert kxferacc to 0
		entry->kxferacc = 0;
	}

	/* flags */
	//entry->flags = 0;
	entry->flags &= (OF_SPLIT);	//(split is set in hb_transaction)

	if(	data->type == TRANSACTION_EDIT_ADD || data->type == TRANSACTION_EDIT_INHERIT)
	entry->flags |= OF_ADDED;

	if(	data->type == TRANSACTION_EDIT_MODIFY)
	entry->flags |= OF_CHANGED;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque));
	if(active == 1) entry->flags |= OF_CHEQ2;

	//active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_amount));
	active = entry->amount > 0 ? TRUE : FALSE;
	if(active == TRUE) entry->flags |= OF_INCOME;

}




static gboolean deftransaction_amount_focusout(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
struct deftransaction_data *data;
gushort paymode;
gdouble amount;

	DB( g_print("\n[ui-transaction] amount focus-out-event %d\n", gtk_widget_is_focus(widget)) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));

	// for internal transfer add, amount must be expense by default
	if( paymode == PAYMODE_INTXFER && data->type == TRANSACTION_EDIT_ADD )
	{
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if(amount > 0)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);
	}

	deftransaction_update_warnsign(widget, NULL);
	
	return FALSE;
}


static void deftransaction_toggleamount(GtkWidget *widget, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data)
{
struct deftransaction_data *data;
guint count, i;
Split *split;
gdouble value;

	DB( g_print("\n[ui-transaction] toggleamount\n") );

	if(icon_pos == GTK_ENTRY_ICON_PRIMARY)
	{
		data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

		gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_amount));
		
		value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		value *= -1;
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value);
	
		if( data->ope->flags & OF_SPLIT )
		{
			count = da_splits_count(data->ope->splits);
			DB( g_print("- count = %d\n", count) );
			for(i=0;i<count;i++)
			{
				split = data->ope->splits[i];
				split->amount *= -1;
			}
		}

		deftransaction_update_warnsign(widget, NULL);
	}

}


static void deftransaction_button_split_cb(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *ope;
gdouble amount;
gint nbsplit;

	DB( g_print("\n[ui-transaction] doing split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	ope = data->ope;

	amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	ui_split_dialog(data->window, ope->splits, amount, &deftransaction_set_amount_from_split);

	//eval split to garantee disabled items
	ope->flags &= ~(OF_SPLIT);
	nbsplit = da_splits_count(ope->splits);
	if(nbsplit > 0)
		data->ope->flags |= (OF_SPLIT);

	deftransaction_update(data->window, NULL);	
}




/*
**
*/
static void deftransaction_paymode(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gint payment;
gint page;
gboolean sensitive;

	DB( g_print("\n[ui-transaction] paymode change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	/* todo: prefill the cheque number ? */
	if( data->type != TRANSACTION_EDIT_MODIFY )
	{
	gboolean expense = (gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount)) > 0 ? FALSE : TRUE);

		DB( g_print(" - payment: %d\n", PAYMODE_CHECK) );
		DB( g_print(" - expense: %d\n", expense) );
		DB( g_print(" - acc is: %d\n", ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc)) ) );

		if(payment == PAYMODE_CHECK)
		{
			if(expense == TRUE)
			{
			Account *acc;
			gint active = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			guint cheque;
			gchar *cheque_str;

				DB( g_print(" - should fill cheque number for account %d\n", active) );

				//#1410166
				if( active > 0 )
				{
					acc = da_acc_get( active );
					if(acc != NULL)
					{
						cheque = ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque))==TRUE ? acc->cheque2 : acc->cheque1 );
						cheque_str = g_strdup_printf("%d", cheque + 1);
						gtk_entry_set_text(GTK_ENTRY(data->ST_info), cheque_str);
						g_free(cheque_str);
					}
				}
			}
		}
	}


	if(payment == PAYMODE_CHECK)
		page = 1;

	sensitive = (payment == PAYMODE_INTXFER) ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->BT_split, sensitive);

	sensitive = page == 1 ? TRUE : FALSE;
	hb_widget_visible(data->CM_cheque, sensitive);
	
	if(payment == PAYMODE_INTXFER)
	{
		page = 2;
		// for internal transfer add, amount must be expense by default
		gdouble amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if( data->type == TRANSACTION_EDIT_ADD )
		{
			if(amount > 0)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);
		}
		//else //#1460370
		//if( data->type == TRANSACTION_EDIT_MODIFY )
	//	{
			if(amount > 0)
			{
				gtk_label_set_text_with_mnemonic (GTK_LABEL(data->LB_accto), _("From acc_ount:"));
			}
			else
			{
				gtk_label_set_text_with_mnemonic (GTK_LABEL(data->LB_accto), _("To acc_ount:"));	
			}
		//}		
		
		
	}

	deftransaction_update_accto(widget, user_data);
	DB( g_print(" payment: %d, page: %d\n", payment, page) );

	sensitive = page == 2 ? TRUE : FALSE;
	hb_widget_visible(data->LB_accto, sensitive);
	hb_widget_visible(data->PO_accto, sensitive);

}


/*
** called from outside
*/
gint deftransaction_external_edit(GtkWindow *parent, Transaction *old_txn, Transaction *new_txn)
{
GtkWidget *dialog;
gboolean result;

	dialog = create_deftransaction_window(GTK_WINDOW(parent), TRANSACTION_EDIT_MODIFY, FALSE);
	deftransaction_set_transaction(dialog, new_txn);

	result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_ACCEPT)
	{
		deftransaction_get(dialog, NULL);

		account_balances_sub(old_txn);
		account_balances_add(new_txn);

		if( new_txn->paymode == PAYMODE_INTXFER )
		{
			//nota: if kxfer is 0, the user may have just changed the paymode to xfer
			DB( g_print(" - kxfer = %d\n", new_txn->kxfer) );

			if(new_txn->kxfer > 0)	//1) search a strong linked child
			{
			Transaction *ltxn;

				DB( g_print(" - old_txn: kacc=%d kxferacc=%d\n", old_txn->kacc, old_txn->kxferacc) );
				
				//#1584342 was faultly old_txn
				ltxn = transaction_xfer_child_strong_get(new_txn);
				if(ltxn != NULL) //should never be the case
				{
					DB( g_print(" - strong link found, do sync\n") );
					transaction_xfer_sync_child(new_txn, ltxn);
				}
				else
				{
					DB( g_print(" - no, somethin' went wrong here...\n") );
				}
			}
			else
			{
				//2) any standard transaction that match ?
				transaction_xfer_search_or_add_child(GTK_WINDOW(dialog), new_txn, FALSE);
			}
		}

		//#1250061 : manage ability to break an internal xfer
		if(old_txn->paymode == PAYMODE_INTXFER && new_txn->paymode != PAYMODE_INTXFER)
		{
		GtkWidget *p_dialog;
		gboolean break_result;
			
			DB( g_print(" - should break internal xfer\n") );

			p_dialog = gtk_message_dialog_new
			(
				GTK_WINDOW(parent),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_YES_NO,
				_("Do you want to break the internal transfer ?\n\n"
				  "Proceeding will delete the target transaction.")
			);

			break_result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
			gtk_widget_destroy( p_dialog );

			if(break_result == GTK_RESPONSE_YES)
			{
				transaction_xfer_remove_child(old_txn);
			}
			else	//force paymode to internal xfer
			{
				new_txn->paymode = PAYMODE_INTXFER;
			}
		}
	}
	
	//1638035: manage account change
	if( old_txn->kacc != new_txn->kacc )
	{
		//locked from ui, but test anyway: forbid change for internal transfer
		if( new_txn->paymode == PAYMODE_INTXFER )
		{
			new_txn->kacc = old_txn->kacc;
		}
		else
		{
			//todo: maybe we should restrict this also to same currency account
			account_balances_sub(new_txn);
			transaction_acc_move(new_txn, old_txn->kacc, new_txn->kacc);
			account_balances_add(new_txn);
		}
	}
	
	
	deftransaction_dispose(dialog, NULL);
	gtk_widget_destroy (dialog);

	return result;
}


void deftransaction_set_transaction(GtkWidget *widget, Transaction *ope)
{
struct deftransaction_data *data;


	DB( g_print("\n[ui-transaction] set transaction (from out)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->ope = ope;

	DB( g_print(" - ope=%p data=%p\n", data->ope, data) );

	DB( g_print(" - call init\n") );

	deftransaction_set(widget, NULL);
	deftransaction_paymode(widget, NULL);
	deftransaction_update(widget, NULL);
	deftransaction_update_warnsign(widget, NULL);

}


void deftransaction_dispose(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;

	DB( g_print("\n[ui-transaction] dispose\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_free(data);
}


static void deftransaction_setup(struct deftransaction_data *data)
{

	DB( g_print("\n[ui-transaction] setup\n") );

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->h_cat);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_accto), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);

	if( data->showtemplate )
	{
		deftransaction_template_popover_populate (data, GLOBALS->arc_list);
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(data->modelfilter));
	}
}

static GtkWidget *
create_popover (GtkWidget       *parent,
                GtkWidget       *child,
                GtkPositionType  pos)
{
  GtkWidget *popover;

  popover = gtk_popover_new (parent);
  gtk_popover_set_position (GTK_POPOVER (popover), pos);
  gtk_container_add (GTK_CONTAINER (popover), child);
  gtk_container_set_border_width (GTK_CONTAINER (popover), SPACING_SMALL);
  gtk_widget_show (child);

/*	gtk_widget_set_margin_start (popover, SPACING_MEDIUM);
	gtk_widget_set_margin_end (popover, SPACING_MEDIUM);
	gtk_widget_set_margin_top (popover, SPACING_MEDIUM);
	gtk_widget_set_margin_bottom (popover, SPACING_MEDIUM);*/

  return popover;
}


static void deftransaction_template_popover_onRowActivated(GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct deftransaction_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");
	model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
	Archive *arc;
	Transaction *txn;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPTPL_DATAS, &arc, -1);

		txn = data->ope;
		da_transaction_init_from_template(txn, arc);
		if( da_splits_clone(arc->splits, txn->splits) > 0)
			txn->flags |= OF_SPLIT; //Flag that Splits are active

		DB( g_print(" calls\n") );

		deftransaction_set(GTK_WIDGET(treeview), NULL);
		deftransaction_paymode(GTK_WIDGET(treeview), NULL);
		deftransaction_update(GTK_WIDGET(treeview), NULL);

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->MB_template), FALSE);
	}
}



static void deftransaction_template_popover_populate(struct deftransaction_data *data, GList *srclist)
{
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;

	//insert all glist item into treeview
	model  = data->model;
	gtk_list_store_clear(GTK_LIST_STORE(model));

	list = g_list_first(srclist);
	while (list != NULL)
	{
	Archive *entry = list->data;

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 
			LST_DSPTPL_DATAS, entry,
			LST_DSPTPL_NAME, entry->wording,
			 -1);

		//DB( g_print(" populate_treeview: %d %08x\n", i, list->data) );

		list = g_list_next(list);
	}
}


static void
deftransaction_template_popover_refilter (GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data = user_data;

	DB( g_print(" text changed\n") );

	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(data->modelfilter));
}


static gboolean deftransaction_template_popover_func_visible (GtkTreeModel *model, GtkTreeIter  *iter, gpointer user_data)
{
struct deftransaction_data *data = user_data;
Archive *entry;
gchar *str;
gboolean visible = TRUE;
gboolean showsched;
	
	showsched = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_showsched));

	gchar *needle = g_ascii_strdown(gtk_entry_get_text(GTK_ENTRY(data->ST_search)), -1);

	gtk_tree_model_get (model, iter,
		LST_DSPTPL_DATAS, &entry,
		LST_DSPTPL_NAME, &str, -1);

	if( entry )
	{
		if( (entry->flags & OF_AUTO) && !showsched)
		{
			visible = FALSE;
		}
		else
		{
			gchar *haystack = g_ascii_strdown(str, -1);

			if (str && g_strrstr (haystack, needle) == NULL )
			{
				visible = FALSE;
			}

			DB( g_print("filter: '%s' '%s' %d\n", str, needle, visible) );

			g_free(haystack);
		}
	}
	g_free(needle);
	g_free (str);

	return visible;
}


static GtkWidget *deftransaction_template_popover_create(struct deftransaction_data *data)
{
GtkListStore *store;
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkWidget *box, *widget, *scrollwin, *treeview;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);

	widget = make_search(NULL);
	data->ST_search = widget;
	gtk_box_pack_start (GTK_BOX(box), widget, FALSE, FALSE, 0);


	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX(box), scrollwin, TRUE, TRUE, 0);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	store = gtk_list_store_new(NUM_LST_DSPTPL, 
		G_TYPE_POINTER,
		G_TYPE_STRING);
		
	data->model = GTK_TREE_MODEL(store);
	
	data->modelfilter = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(GTK_TREE_MODEL(data->model), NULL));
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(data->modelfilter), deftransaction_template_popover_func_visible, data, NULL);


	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(data->modelfilter));
	data->LV_arc = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	gtk_widget_grab_focus(treeview);

	/* column for bug numbers */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL,
		                                             renderer,
		                                             "text",
		                                             LST_DSPTPL_NAME,
		                                             NULL);
	//gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);

	widget = gtk_check_button_new_with_mnemonic(_("Show _scheduled"));
	data->CM_showsched = widget;
	gtk_box_pack_start (GTK_BOX(box), widget, FALSE, FALSE, 0);

	gtk_widget_show_all (box);

	//signals
	g_signal_connect (data->CM_showsched, "toggled", G_CALLBACK (deftransaction_template_popover_refilter), data);
	g_signal_connect (data->ST_search, "search-changed", G_CALLBACK (deftransaction_template_popover_refilter), data);

	return box;
}


static GtkWidget *deftransaction_make_block1(struct deftransaction_data *data)
{
GtkWidget *group_grid, *hbox, *label, *widget, *image;
gint row;

	group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);

	row = 0;
	label = make_label_widget(_("_Date:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = gtk_date_entry_new();
	data->PO_date = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_widget_set_tooltip_text(widget, _("Date accepted here are:\nday,\nday/month or month/day,\nand complete date into your locale"));

	row++;
	label = make_label_widget(_("_Amount:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 1, row, 1, 1);

		widget = make_amount(label);
		data->ST_amount = widget;
		gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget), GTK_ENTRY_ICON_PRIMARY, ICONNAME_HB_TOGGLE_SIGN);
		gtk_entry_set_icon_tooltip_text(GTK_ENTRY(widget), GTK_ENTRY_ICON_PRIMARY, _("Toggle amount sign"));
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		image = gtk_image_new_from_icon_name (ICONNAME_HB_BUTTON_SPLIT, GTK_ICON_SIZE_BUTTON);
		widget = gtk_button_new();
		g_object_set (widget, "image", image, NULL);
		data->BT_split = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		gtk_widget_set_tooltip_text(widget, _("Transaction splits"));

	row++;
	label = make_label_widget(_("Pa_yment:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_paymode(label);
	data->NU_mode = widget;
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic(_("Of notebook _2"));
	data->CM_cheque = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	label = make_label_widget(_("_Info:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_string(label);
	data->ST_info = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	label = make_label_widget(_("A_ccount:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	label = make_label_widget(_("To acc_ount:"));
	data->LB_accto = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_accto = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	label = make_label_widget(_("_Payee:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = ui_pay_comboboxentry_new(label);
	data->PO_pay = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available"));

	row++;
	label = make_label_widget(_("_Category:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = ui_cat_comboboxentry_new(label);
	data->PO_grp = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available"));

	row++;
	label = make_label_widget(_("_Status:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_radio(CYA_TXN_STATUS, TRUE, GTK_ORIENTATION_HORIZONTAL);
	data->RA_status = widget;
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	label = make_label_widget(_("M_emo:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_memo_entry(label);
	data->ST_word = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	label = make_label_widget(_("Ta_gs:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_string(label);
	data->ST_tags = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	return group_grid;
}


static gboolean 
deftransaction_getgeometry(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
struct WinGeometry *wg;

	DB( g_print("\n[ui-transaction] get geometry\n") );

	//store size
	wg = &PREFS->txn_wg;
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, NULL);

	DB( g_print(" window: w=%d\n", wg->w) );

	return FALSE;
}


GtkWidget *create_deftransaction_window (GtkWindow *parent, gint type, gboolean postmode)
{
struct deftransaction_data *data;
struct WinGeometry *wg;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *bar;
gint crow;

	DB( g_print("\n[ui-transaction] new\n") );

	data = g_malloc0(sizeof(struct deftransaction_data));

	/*
	dialog = gtk_dialog_new_with_buttons (_(CYA_OPERATION[data->type]),
					    GTK_WINDOW (parent),
					    0,
					    NULL,
					    NULL);
	*/
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(dialog), _(CYA_OPERATION[type]));
	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(parent));
	
	


	//store our window private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)data);
	DB( g_print(" - window=%p, inst_data=%p\n", dialog, data) );

	data->window = dialog;
	data->type = type;

	// if you add/remove response_id also change into deftransaction_update_transfer
	if(type == TRANSACTION_EDIT_MODIFY)
	{
		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    _("_Cancel"), GTK_RESPONSE_REJECT,
			_("_OK"), GTK_RESPONSE_ACCEPT,
			NULL);
	}
	else
	{
		if(!postmode)
		{
			gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				_("_Close"), GTK_RESPONSE_REJECT,
				_("_Add & keep"), GTK_RESPONSE_ADDKEEP,
				_("_Add"), GTK_RESPONSE_ADD,
				NULL);
		}
		else
		{
			gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				_("_Close"), GTK_RESPONSE_REJECT,
				_("_Post"), GTK_RESPONSE_ADD,
				NULL);
		}
	}

	switch(type)
	{
		case TRANSACTION_EDIT_ADD:
			gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_OPE_ADD);
			break;
		case TRANSACTION_EDIT_INHERIT:
			gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_OPE_HERIT);
			break;
		case TRANSACTION_EDIT_MODIFY:
			gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_OPE_EDIT);
			break;
	}

	//gtk_window_set_decorated(GTK_WINDOW(dialog), TRUE);

	//window contents
	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_grid_set_column_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	//gtk_grid_set_column_homogeneous(GTK_GRID (content_grid), TRUE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (content_area), content_grid);
	gtk_widget_show_all(content_grid);
	
	crow = 0;

	data->showtemplate = FALSE;
	if( data->type != TRANSACTION_EDIT_MODIFY && da_archive_length() > 0 && !postmode )
	{
	GtkWidget *box, *menubutton, *image;

		data->showtemplate = TRUE;

		menubutton = gtk_menu_button_new ();
		data->MB_template = menubutton;
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
		label = gtk_label_new_with_mnemonic (_("Use a _template"));
		gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);
		image = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
		gtk_box_pack_start (GTK_BOX(box), image, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(menubutton), box);

		gtk_menu_button_set_direction (GTK_MENU_BUTTON(menubutton), GTK_ARROW_DOWN );
		gtk_widget_set_halign (menubutton, GTK_ALIGN_END);
		gtk_grid_attach (GTK_GRID (content_grid), menubutton, 0, crow, 1, 1);
		gtk_widget_show_all(menubutton);


		GtkWidget *template = deftransaction_template_popover_create(data);
		GtkWidget *popover = create_popover (menubutton, template, GTK_POS_BOTTOM);
		gtk_widget_set_size_request (popover, HB_MINWIDTH_LIST, HB_MINHEIGHT_LIST);
		gtk_widget_set_vexpand (popover, TRUE);
		gtk_widget_set_hexpand (popover, TRUE);

		/*gtk_widget_set_margin_start (popover, 10);
		gtk_widget_set_margin_end (popover, 10);
		gtk_widget_set_margin_bottom (popover, 10);*/

		gtk_menu_button_set_popover(GTK_MENU_BUTTON(menubutton), popover);
	
		g_signal_connect (GTK_TREE_VIEW(data->LV_arc), "row-activated", G_CALLBACK (deftransaction_template_popover_onRowActivated), NULL);
	
	}

	crow++;
	group_grid = deftransaction_make_block1(data);
	//gtk_widget_set_hexpand (GTK_WIDGET(group_grid), TRUE);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow, 1, 1);
	gtk_widget_show_all(group_grid);
	
	/*crow++;
	group_grid = deftransaction_make_block2(data);
	gtk_widget_set_hexpand (GTK_WIDGET(group_grid), TRUE);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 1, crow, 1, 1);
	gtk_widget_show_all(group_grid);*/

	crow++;
	bar = gtk_info_bar_new ();
	data->IB_warnsign = bar;
	gtk_info_bar_set_message_type (GTK_INFO_BAR (bar), GTK_MESSAGE_WARNING);
	label = gtk_label_new (_("Warning: amount and category sign don't match"));
    gtk_box_pack_start (GTK_BOX (gtk_info_bar_get_content_area (GTK_INFO_BAR (bar))), label, TRUE, TRUE, 0);
	gtk_grid_attach (GTK_GRID (content_grid), bar, 0, crow, 1, 1);

	
	//connect all our signals
	g_signal_connect (dialog, "configure-event", G_CALLBACK (deftransaction_getgeometry), (gpointer)data);


	g_signal_connect (G_OBJECT (data->ST_amount), "focus-out-event", G_CALLBACK (deftransaction_amount_focusout), NULL);
	g_signal_connect (G_OBJECT (data->ST_amount), "icon-release", G_CALLBACK (deftransaction_toggleamount), NULL);
	g_signal_connect (G_OBJECT (data->BT_split), "clicked", G_CALLBACK (deftransaction_button_split_cb), NULL);

	g_signal_connect (data->NU_mode  , "changed", G_CALLBACK (deftransaction_paymode), NULL);
	g_signal_connect (data->CM_cheque, "toggled", G_CALLBACK (deftransaction_paymode), NULL);

	g_signal_connect (data->PO_pay  , "changed", G_CALLBACK (deftransaction_update_payee), NULL);
	g_signal_connect (data->PO_acc  , "changed", G_CALLBACK (deftransaction_update_accto), NULL);
	g_signal_connect (data->PO_accto, "changed", G_CALLBACK (deftransaction_update_transfer), NULL);
	g_signal_connect (data->PO_grp  , "changed", G_CALLBACK (deftransaction_update_warnsign), NULL);


	//setup, init and show window
	deftransaction_setup(data);

	wg = &PREFS->txn_wg;
	gtk_window_set_default_size(GTK_WINDOW(dialog), wg->w, -1);

	//gtk_widget_show_all (dialog);
	//gtk_widget_hide(data->IB_warnsign);

	return dialog;
}
