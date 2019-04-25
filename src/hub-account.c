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

#include "hub-account.h"
#include "dsp-mainwindow.h"
#include "list-account.h"


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


extern HbKvData CYA_ACC_TYPE[];


static void ui_hub_account_expand_all(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	gtk_tree_view_expand_all(GTK_TREE_VIEW(data->LV_acc));
}


static void ui_hub_account_collapse_all(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	gtk_tree_view_collapse_all(GTK_TREE_VIEW(data->LV_acc));
}


static void ui_hub_account_groups_free(GHashTable *h_group)
{
GHashTableIter grp_iter;
gpointer key, value;

	DB( g_print("\n[hub-account] groups free\n") );

	g_hash_table_iter_init (&grp_iter, h_group);
	while (g_hash_table_iter_next (&grp_iter, &key, &value))
	{
	PnlAccGrp *group = value;
	
		g_ptr_array_free (group->acclist, TRUE);
		g_free(group);
	}

	g_hash_table_destroy (h_group);  
}


static GHashTable *ui_hub_account_groups_get(GtkTreeView *treeview, gint groupby, gboolean showall)
{
GHashTable *h_group;
GList *lacc, *elt;
gchar *groupname;
gint nballoc;
	
	DB( g_print("\n[hub-account] groups get\n") );

	nballoc = da_acc_length ();
	
	DB( g_print(" %d accounts\n", nballoc) );
	
	h_group = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, NULL);

	lacc = g_hash_table_get_values(GLOBALS->h_acc);
	elt = g_list_first(lacc);
	while (elt != NULL)
	{
	Account *acc = elt->data;
	PnlAccGrp *group;
	
		//#1674045 ony rely on nosummary
		//if( showall || !(acc->flags & (AF_CLOSED|AF_NOSUMMARY)) )
		if( showall || !(acc->flags & AF_NOSUMMARY) )
		{
			switch( groupby )
			{
				case DSPACC_GROUP_BY_BANK:
				{
					groupname = _("(no institution)");
					if( (acc->bankname != NULL) && strlen(acc->bankname) > 0 ) 
						groupname = acc->bankname;
				}
				break;

				default:
					//pre 5.1.3 historical by type display
					groupname = hbtk_get_label(CYA_ACC_TYPE, acc->type);
				break;
			}

			//#1820853 groupname could be NULL
			if( groupname != NULL )
			{
				if( g_hash_table_contains(h_group, groupname) == FALSE )
				{
					group = g_malloc0(sizeof(PnlAccGrp));
					if( group )
					{
						group->acclist = g_ptr_array_sized_new(nballoc);
						group->expanded = list_account_level1_expanded(treeview, groupname);
						g_hash_table_insert(h_group, g_strdup(groupname), group );
						DB( g_print(" grp '%s' exp:%d\n", groupname, group->expanded) );
					}
				}

				group = g_hash_table_lookup(h_group, groupname);
				if( group != NULL )
				{
					g_ptr_array_add(group->acclist, (gpointer)acc);
					DB( g_print(" + acc '%s'\n", acc->name) );
				}
			}
		}
		elt = g_list_next(elt);
	}

	g_list_free(lacc);
	
	return h_group;
}





void ui_hub_account_populate(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
GtkTreeModel *model;
GtkTreeIter  iter1, child_iter;
Account *acc;
guint j, nbtype;
gdouble gtbank, gttoday, gtfuture;
GHashTable *h_group;
GHashTableIter grp_iter;
gpointer key, value;

	DB( g_print("\n[hub-account] populate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	h_group = ui_hub_account_groups_get(GTK_TREE_VIEW(data->LV_acc), PREFS->pnl_acc_show_by, data->showall);

	DB( g_print("\n\n populate listview, %d group(s)\n", g_hash_table_size(h_group)) );

	nbtype = 0;
	gtbank = gttoday = gtfuture = 0;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
	gtk_tree_store_clear (GTK_TREE_STORE(model));

	g_hash_table_iter_init (&grp_iter, h_group);
	while (g_hash_table_iter_next (&grp_iter, &key, &value))
	{
	PnlAccGrp *group = value;
	gdouble tbank, ttoday, tfuture;
	gint position;

		if(group != NULL)
		{
			nbtype++;
			//1: Header: Bank, Cash, ...
			DB( g_print(" g '%s'\n", (gchar *)key) );

			//#1663399 keep type position like in dropdown
			position = 0;
			if( PREFS->pnl_acc_show_by == DSPACC_GROUP_BY_TYPE )
			{
			gint t = 0;	

				while(CYA_ACC_TYPE[t].name != NULL && t < 32)
				{
					if( !strcmp(CYA_ACC_TYPE[t].name, key) )
						break;
					t++;
				}

				position = t;
			}

			gtk_tree_store_append (GTK_TREE_STORE(model), &iter1, NULL);
			gtk_tree_store_set (GTK_TREE_STORE(model), &iter1,
					  LST_DSPACC_POS, position,
					  LST_DSPACC_DATATYPE, DSPACC_TYPE_HEADER,
					  LST_DSPACC_NAME, key,
					  -1);

			//2: Accounts for real
			tbank = ttoday = tfuture = 0;
			for(j=0;j<group->acclist->len;j++)
			{
				acc = g_ptr_array_index(group->acclist, j);

				//tbank += acc->bal_bank;
				//ttoday += acc->bal_today;
				//tfuture += acc->bal_future;
				tbank += hb_amount_base(acc->bal_bank, acc->kcur);
				ttoday += hb_amount_base(acc->bal_today, acc->kcur);
				tfuture += hb_amount_base(acc->bal_future, acc->kcur);

				DB( g_print("  + '%s' :: %.2f %.2f %.2f\n", acc->name, acc->bal_bank, acc->bal_today, acc->bal_future) );

				gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter1);
				gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
						LST_DSPACC_DATAS, acc,
						LST_DSPACC_DATATYPE, DSPACC_TYPE_NORMAL,
						LST_DSPACC_BANK, acc->bal_bank,
						LST_DSPACC_TODAY, acc->bal_today,
						LST_DSPACC_FUTURE, acc->bal_future,
					  -1);
			}

			if(group->acclist->len > 1)
			{
				DB( g_print("  + total :: %.2f %.2f %.2f\n", tbank, ttoday, tfuture) );

				// insert the total line
				gtk_tree_store_append (GTK_TREE_STORE(model), &child_iter, &iter1);
				gtk_tree_store_set (GTK_TREE_STORE(model), &child_iter,
						LST_DSPACC_DATATYPE, DSPACC_TYPE_SUBTOTAL,
						LST_DSPACC_NAME, _("Total"),
						LST_DSPACC_BANK, tbank,
						LST_DSPACC_TODAY, ttoday,
						LST_DSPACC_FUTURE, tfuture,
						  -1);
			}

			/* set balance to header to display when collasped */
			DB( g_print(" (enrich group total header) :: %.2f %.2f %.2f\n", tbank, ttoday, tfuture) );
			gtk_tree_store_set (GTK_TREE_STORE(model), &iter1,
					LST_DSPACC_BANK, tbank,
					LST_DSPACC_TODAY, ttoday,
					LST_DSPACC_FUTURE, tfuture,
					  -1);

			if( group->expanded == TRUE )
			{
			GtkTreePath *tmppath = gtk_tree_model_get_path(model, &iter1);

				DB( g_print(" expanding '%s'\n", (gchar *)key) );
				gtk_tree_view_expand_row(GTK_TREE_VIEW(data->LV_acc), tmppath, TRUE);
				gtk_tree_path_free(tmppath);
			}
			
			/* add to grand total */
			gtbank += tbank;
			gttoday += ttoday;
			gtfuture += tfuture;

		}

	}

	DB( g_print(" + grand total :: %.2f %.2f %.2f\n", gtbank, gttoday, gtfuture) );

	// Grand total
	if( nbtype > 1 )
	{
		gtk_tree_store_append (GTK_TREE_STORE(model), &iter1, NULL);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter1,
					LST_DSPACC_DATATYPE, DSPACC_TYPE_SUBTOTAL,
					LST_DSPACC_NAME, _("Grand total"),
					LST_DSPACC_BANK, gtbank,
					LST_DSPACC_TODAY, gttoday,
					LST_DSPACC_FUTURE, gtfuture,
				  -1);
	}

	//gtk_tree_view_expand_all(GTK_TREE_VIEW(data->LV_acc));
	
	ui_hub_account_groups_free(h_group);

}



/* Callback function for the undo action */
/*static void
activate_action (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  g_print ("Action %s activated\n", g_action_get_name (G_ACTION (action)));
}*/

static void
ui_hub_account_activate_toggle (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
struct hbfile_data *data = user_data;
  GVariant *old_state, *new_state;

  old_state = g_action_get_state (G_ACTION (action));
  new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));

  DB( g_print ("Toggle action %s activated, state changes from %d to %d\n",
           g_action_get_name (G_ACTION (action)),
           g_variant_get_boolean (old_state),
           g_variant_get_boolean (new_state)) );

	data->showall = g_variant_get_boolean (new_state);
	ui_hub_account_populate(GLOBALS->mainwindow, NULL);

  g_simple_action_set_state (action, new_state);
  g_variant_unref (old_state);
}

static void
ui_hub_account_activate_radio (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
//struct hbfile_data *data = user_data;
GVariant *old_state, *new_state;

  old_state = g_action_get_state (G_ACTION (action));
  new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));

  DB( g_print ("Radio action %s activated, state changes from %s to %s\n",
           g_action_get_name (G_ACTION (action)),
           g_variant_get_string (old_state, NULL),
           g_variant_get_string (new_state, NULL)) );

	PREFS->pnl_acc_show_by = DSPACC_GROUP_BY_TYPE;
	if( !strcmp("bank", g_variant_get_string(new_state, NULL)) )
		PREFS->pnl_acc_show_by = DSPACC_GROUP_BY_BANK;

	ui_hub_account_populate(GLOBALS->mainwindow, NULL);

  g_simple_action_set_state (action, new_state);
  g_variant_unref (old_state);
}


static const GActionEntry actions[] = {
//	name, function(), type, state, 
//  { "paste", activate_action, NULL, NULL,      NULL, {0,0,0} },
	{ "showall", ui_hub_account_activate_toggle, NULL, "false" , NULL, {0,0,0} },
	{ "groupby", ui_hub_account_activate_radio ,  "s", "'type'", NULL, {0,0,0} }
};


void ui_hub_account_setup(struct hbfile_data *data)
{
GAction *action;
GVariant *new_state;

	if( !G_IS_SIMPLE_ACTION_GROUP(data->action_group_acc) )
		return;

	action = g_action_map_lookup_action (G_ACTION_MAP (data->action_group_acc), "showall");
	if( action )
	{
		new_state = g_variant_new_boolean (data->showall);
		g_simple_action_set_state (G_SIMPLE_ACTION(action), new_state);
	}
	
	action = g_action_map_lookup_action (G_ACTION_MAP (data->action_group_acc), "groupby");
	if( action )
	{
		const gchar *value = "type";
		if( PREFS->pnl_acc_show_by == DSPACC_GROUP_BY_BANK )
			value = "bank";
		
		new_state = g_variant_new_string (value);
		g_simple_action_set_state (G_SIMPLE_ACTION (action), new_state);
	}

}


GtkWidget *ui_hub_account_create(struct hbfile_data *data)
{
GtkWidget *hub, *label, *widget, *sw, *tbar, *hbox, *image;
GtkToolItem *toolitem;

	DB( g_print("\n[hub-account] create\n") );


	hub = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hub), SPACING_SMALL);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (hub), sw, TRUE, TRUE, 0);
	widget = (GtkWidget *)create_list_account();
	data->LV_acc = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	//list toolbar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(tbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_ICONS);
	gtk_style_context_add_class (gtk_widget_get_style_context (tbar), GTK_STYLE_CLASS_INLINE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (hub), tbar, FALSE, FALSE, 0);

	label = make_label_group(_("Your accounts"));
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), label);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, TRUE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);
	
		widget = make_image_button(ICONNAME_HB_BUTTON_EXPAND, _("Expand all"));
		data->BT_expandall = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_HB_BUTTON_COLLAPSE, _("Collapse all"));
		data->BT_collapseall = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, FALSE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);


	//gmenu test (see test folder into gtk)
GMenu *menu, *section;

	menu = g_menu_new ();
	//g_menu_append (menumodel, "About", "actions.undo");
	//g_menu_append (menumodel, "Test", "actions.redo");
	section = g_menu_new ();
	g_menu_append (section, _("Show all"), "actions.showall");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_object_unref (section);

	section = g_menu_new ();
	g_menu_append (section, _("By type"), "actions.groupby::type");
	g_menu_append (section, _("By institution"), "actions.groupby::bank");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_object_unref (section);


	GSimpleActionGroup *group = g_simple_action_group_new ();
	data->action_group_acc = group;
	g_action_map_add_action_entries (G_ACTION_MAP (group), actions, G_N_ELEMENTS (actions), data);


	widget = gtk_menu_button_new();
	gtk_menu_button_set_direction (GTK_MENU_BUTTON(widget), GTK_ARROW_UP);
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	image = gtk_image_new_from_icon_name (ICONNAME_EMBLEM_SYSTEM, GTK_ICON_SIZE_MENU);
	g_object_set (widget, "image", image,  NULL);

	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	gtk_widget_insert_action_group (widget, "actions", G_ACTION_GROUP(group));
	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (widget), G_MENU_MODEL (menu));

	g_signal_connect (G_OBJECT (data->BT_expandall  ), "clicked"      , G_CALLBACK (ui_hub_account_expand_all), NULL);
	g_signal_connect (G_OBJECT (data->BT_collapseall), "clicked"      , G_CALLBACK (ui_hub_account_collapse_all), NULL);

	
	return hub;
}

