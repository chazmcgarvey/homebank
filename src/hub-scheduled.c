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


#include "homebank.h"

#include "dsp-mainwindow.h"
#include "list-scheduled.h"
#include "hub-scheduled.h"

#include "ui-transaction.h"


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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* scheduled */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static Archive *
ui_hub_scheduled_get_selected_item(GtkTreeView *treeview)
{
GtkTreeSelection *treeselection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if( gtk_tree_selection_get_selected(treeselection, &model, &iter) )
	{
	Archive *arc;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPUPC_DATAS, &arc, -1);
		return arc;
	}

	return NULL;
}


static void ui_hub_scheduled_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
//struct hbfile_data *data;
Archive *arc;

	DB( g_print ("\n[hub-scheduled] row double-clicked\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(treeview, GTK_TYPE_WINDOW)), "inst_data");

	arc = ui_hub_scheduled_get_selected_item(treeview);
	ui_mainwindow_defarchive(arc);
}


static void ui_hub_scheduled_do_post(Archive *arc, gboolean doedit, gpointer user_data)
{
struct hbfile_data *data = user_data;
GtkWidget *window;
gint result;
Transaction *txn;

	DB( g_print("\n[hub-scheduled] do post\n") );

	
	window =  create_deftransaction_window(GTK_WINDOW(data->window), TRANSACTION_EDIT_ADD, TRUE, 0);

	/* fill in the transaction */
	txn = da_transaction_malloc();
	da_transaction_init_from_template(txn, arc);
	txn->date = scheduled_get_postdate(arc, arc->nextdate);

	deftransaction_set_transaction(window, txn);

	result = gtk_dialog_run (GTK_DIALOG (window));

	DB( g_print(" - dialog result is %d\n", result) );

	if(result == HB_RESPONSE_ADD || result == GTK_RESPONSE_ACCEPT)
	{
		deftransaction_get(window, NULL);
		transaction_add(GTK_WINDOW(GLOBALS->mainwindow), txn);
		GLOBALS->changes_count++;

		scheduled_date_advance(arc);

		DB( g_print(" - added 1 transaction to %d\n", txn->kacc) );
	}

	da_transaction_free(txn);

	deftransaction_dispose(window, NULL);
	gtk_widget_destroy (window);

}


static void ui_hub_scheduled_editpost_cb(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data = user_data;

	DB( g_print("\n[hub-scheduled] editpost\n") );
	
	Archive *arc = ui_hub_scheduled_get_selected_item(GTK_TREE_VIEW(data->LV_upc));

	if( (arc != NULL) )
	{
		ui_hub_scheduled_do_post(arc, TRUE, data);
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE|UF_REFRESHALL));
	}
}


static void ui_hub_scheduled_post_cb(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data = user_data;

	DB( g_print("\n[hub-scheduled] post\n") );

	Archive *arc = ui_hub_scheduled_get_selected_item(GTK_TREE_VIEW(data->LV_upc));

	if( (arc != NULL) )
	{
		if( scheduled_is_postable(arc) )
		{
		Transaction *txn = da_transaction_malloc ();

			da_transaction_init_from_template(txn, arc);
			txn->date = scheduled_get_postdate(arc, arc->nextdate);
			transaction_add(GTK_WINDOW(GLOBALS->mainwindow), txn);

			GLOBALS->changes_count++;
			scheduled_date_advance(arc);

			da_transaction_free (txn);
		}
		else
		{
			ui_hub_scheduled_do_post(arc, FALSE, data);
		}

		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE|UF_REFRESHALL));
	}
}


static void ui_hub_scheduled_skip_cb(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data = user_data;

	DB( g_print("\n[hub-scheduled] skip\n") );
	
	Archive *arc = ui_hub_scheduled_get_selected_item(GTK_TREE_VIEW(data->LV_upc));
	if( (arc != NULL) && (arc->flags & OF_AUTO) )
	{
		GLOBALS->changes_count++;
		scheduled_date_advance(arc);

		ui_hub_scheduled_populate(GLOBALS->mainwindow, NULL);
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));
	}
}



static void ui_hub_scheduled_update(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
//gint filter;

	DB( g_print("\n[hub-scheduled] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//filter = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_sched_filter));

	Archive *arc = ui_hub_scheduled_get_selected_item(GTK_TREE_VIEW(data->LV_upc));

	if(arc)
	{
		DB( g_print("archive is %s\n", arc->memo) );
		
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_skip), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_post), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_editpost), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_skip), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_post), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->BT_sched_editpost), FALSE);
	}

}



static void ui_hub_scheduled_selection_cb(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_hub_scheduled_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(UF_SENSITIVE));
}



/*
** called after load, importamiga, on demand
*/
void ui_hub_scheduled_postall(GtkWidget *widget, gpointer user_data)
{
//struct hbfile_data *data;
gint count;
gint usermode = GPOINTER_TO_INT(user_data);

	DB( g_print("\n[hub-scheduled] post all\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	count = scheduled_post_all_pending();

	//inform the user
	if(usermode == TRUE)
	{
	gchar *txt;

		//#125534
		if( count > 0 )
		{
			ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_REFRESHALL));
		}
		
		if(count == 0)
			txt = _("No transaction to add");
		else
			txt = _("transaction added: %d");

		ui_dialog_msg_infoerror(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_INFO,
			_("Check scheduled transactions result"),
			txt,
			count);
	}

}


void ui_hub_scheduled_populate(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gdouble totexp = 0;
gdouble totinc = 0;
gint count = 0;
gchar buffer[256];
guint32 maxpostdate;
GDate *date;
//Account *acc;

	DB( g_print("\n[hub-scheduled] populate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	homebank_app_date_get_julian();

	maxpostdate = scheduled_date_get_post_max();

	date = g_date_new_julian (maxpostdate);
	g_date_strftime (buffer, 256-1, PREFS->date_format, date);
	g_date_free(date);

	gtk_label_set_text(GTK_LABEL(data->LB_maxpostdate), buffer);

	
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *arc = list->data;
	Account *acc;
	gdouble inc, exp;
	guint nbdays, nblate;

		if((arc->flags & OF_AUTO) ) //&& arc->kacc > 0)
		{
			count++;
			nbdays = arc->nextdate - maxpostdate;
			nblate = scheduled_get_latepost_count(arc, GLOBALS->today);
			
			DB( g_print(" - append '%s' : %d\n", arc->memo, nbdays) );

			if(arc->flags & OF_INCOME)
			{
				inc = arc->amount;
				exp = 0.0;
			}
			else
			{
				exp = arc->amount;
				inc = 0.0;
			}

			/* insert normal txn */
			acc = da_acc_get(arc->kacc);
			if( acc)
			{
				totinc += hb_amount_base(inc, acc->kcur);
				totexp += hb_amount_base(exp, acc->kcur);
			}
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				  LST_DSPUPC_DATAS, arc,
			      LST_DSPUPC_NEXT, nbdays,
				  LST_DSPUPC_ACCOUNT, acc,
			      LST_DSPUPC_MEMO, arc->memo,
			      LST_DSPUPC_EXPENSE, exp,
			      LST_DSPUPC_INCOME, inc,
			      LST_DSPUPC_NB_LATE, nblate,
				  -1);

			/* insert internal xfer txn : 1378836 */
			if(arc->paymode == PAYMODE_INTXFER)
			{
				acc = da_acc_get(arc->kxferacc);
				if( acc)
				{
					totinc += hb_amount_base(-inc, acc->kcur);
					totexp += hb_amount_base(-exp, acc->kcur);
				}
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					  LST_DSPUPC_DATAS, arc,
				      LST_DSPUPC_NEXT, nbdays,
					  LST_DSPUPC_ACCOUNT, acc,
					  LST_DSPUPC_MEMO, arc->memo,
					  LST_DSPUPC_EXPENSE, -inc,
					  LST_DSPUPC_INCOME, -exp,
					  LST_DSPUPC_NB_LATE, nblate,
					  -1);
			}

		}
		list = g_list_next(list);
	}

	// insert total
	if(count > 0 )
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			  LST_DSPUPC_DATAS, NULL,
			  LST_DSPUPC_ACCOUNT, NULL,
			  LST_DSPUPC_MEMO, _("Total"),
			  LST_DSPUPC_EXPENSE, totexp,
		      LST_DSPUPC_INCOME, totinc,
		  -1);
	}

	ui_hub_scheduled_update(widget, NULL);
	
}


GtkWidget *ui_hub_scheduled_create(struct hbfile_data *data)
{
GtkWidget *hub, *hbox, *vbox, *bbox, *sw, *tbar;
GtkWidget *label, *widget;
GtkToolItem *toolitem;

	DB( g_print("\n[hub-scheduled] create\n") );
	
	hub = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hub), SPACING_SMALL);
	//data->GR_upc = hub;

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	//gtk_widget_set_margin_top(GTK_WIDGET(vbox), 0);
	//gtk_widget_set_margin_bottom(GTK_WIDGET(vbox), SPACING_SMALL);
	//gtk_widget_set_margin_start(GTK_WIDGET(vbox), 2*SPACING_SMALL);
	//gtk_widget_set_margin_end(GTK_WIDGET(vbox), SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (hub), vbox, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	
	widget = (GtkWidget *)create_list_upcoming();
	data->LV_upc = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	tbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(tbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_ICONS);
	gtk_style_context_add_class (gtk_widget_get_style_context (tbar), GTK_STYLE_CLASS_INLINE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (vbox), tbar, FALSE, FALSE, 0);

	label = make_label_group(_("Scheduled transactions"));
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), label);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, FALSE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);


	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), bbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

		widget = gtk_button_new_with_label(_("Skip"));
		data->BT_sched_skip = widget;
		gtk_box_pack_start (GTK_BOX (bbox), widget, FALSE, FALSE, 0);

		widget = gtk_button_new_with_label(_("Edit & Post"));
		data->BT_sched_editpost = widget;
		gtk_box_pack_start (GTK_BOX (bbox), widget, FALSE, FALSE, 0);

		//TRANSLATORS: Posting a scheduled transaction is the action to materialize it into its target account.
		//TRANSLATORS: Before that action the automated transaction occurrence is pending and not yet really existing.
		widget = gtk_button_new_with_label (_("Post"));
		data->BT_sched_post = widget;
		gtk_box_pack_start (GTK_BOX (bbox), widget, FALSE, FALSE, 0);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, FALSE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	hbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_valign (hbox, GTK_ALIGN_CENTER);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

		label = make_label(_("maximum post date"), 0.0, 0.7);
		gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
		gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

		label = make_label(NULL, 0.0, 0.7);
		data->LB_maxpostdate = label;
		gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
		gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_upc)), "changed", G_CALLBACK (ui_hub_scheduled_selection_cb), NULL);
	g_signal_connect (GTK_TREE_VIEW(data->LV_upc), "row-activated", G_CALLBACK (ui_hub_scheduled_onRowActivated), NULL);
	g_signal_connect (G_OBJECT (data->BT_sched_skip), "clicked", G_CALLBACK (ui_hub_scheduled_skip_cb), data);
	g_signal_connect (G_OBJECT (data->BT_sched_editpost), "clicked", G_CALLBACK (ui_hub_scheduled_editpost_cb), data);
	g_signal_connect (G_OBJECT (data->BT_sched_post), "clicked", G_CALLBACK (ui_hub_scheduled_post_cb), data);


	
	return hub;
}
