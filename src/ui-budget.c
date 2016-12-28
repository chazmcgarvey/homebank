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


#include "homebank.h"

#include "ui-category.h"
#include "ui-budget.h"

extern gchar *CYA_CAT_TYPE[];

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




gchar *months[] = {
"Jan",
"Feb",
"Mar",
"Apr",
"May",
"Jun",
"Jul",
"Aug",
"Sep",
"Oct",
"Nov",
"Dec"
};

static gchar *ui_bud_manage_getcsvbudgetstr(Category *item);
static void ui_bud_manage_update(GtkWidget *treeview, gpointer user_data);
static void ui_bud_manage_set(GtkWidget *widget, gpointer user_data);
static void ui_bud_manage_getlast(struct ui_bud_manage_data *data);
static void ui_bud_manage_selection_change(GtkWidget *treeview, gpointer user_data);
static void ui_bud_manage_toggle(GtkRadioButton *radiobutton, gpointer user_data);
static void ui_bud_manage_selection(GtkTreeSelection *treeselection, gpointer user_data);


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
static gint ui_bud_listview_compare_funct (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint retval = 0;
Category *entry1, *entry2;

	gtk_tree_model_get(model, a, LST_DEFCAT_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFCAT_DATAS, &entry2, -1);

	retval = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
	if(!retval)
	{
		retval = hb_string_utf8_compare(entry1->name, entry2->name);
	}

    return retval;
}


/*
**
*/
static void ui_bud_listview_icon_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Category *item;
gchar *iconname = NULL;

	// get the transaction
	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &item, -1);

	iconname = ( item->flags & GF_BUDGET ) ? ICONNAME_HB_OPE_BUDGET : NULL;

	g_object_set(renderer, "icon-name", iconname, NULL);
}


/*
** draw some text from the stored data structure
*/
static void ui_bud_listview_cell_data_function_text (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar *name;
gchar *string;
gchar type;

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);

	if(entry->key == 0)
		name = g_strdup(_("(no category)"));
	else
		name = entry->name;

type = (entry->flags & GF_INCOME) ? '+' : '-';

	#if MYDEBUG
		string = g_markup_printf_escaped("%s ::(f=%d, %c)", name, entry->flags, type );
	#else
	if(entry->key == 0)
		string = g_strdup(name);
	else
	{
		if(entry->flags & GF_BUDGET)
		{
			if( entry->parent == 0 )
				string = g_markup_printf_escaped("<b>%s</b> [%c]", name, type);
			else
				string = g_markup_printf_escaped(" %c <b><i>%s</i></b>", type, name);
		}
		else
		{
			if( entry->parent == 0 )
				string = g_markup_printf_escaped("%s [%c]", name, type);
			else
				string = g_markup_printf_escaped(" %c <i>%s</i>", type, name);
		}
	}
	#endif

	g_object_set(renderer, "markup", string, NULL);

	g_free(string);
}


static gboolean ui_bud_listview_search_equal_func (GtkTreeModel *model,
                               gint column,
                               const gchar *key,
                               GtkTreeIter *iter,
                               gpointer search_data)
{
  gboolean retval = TRUE;
  gchar *normalized_string;
  gchar *normalized_key;
  gchar *case_normalized_string = NULL;
  gchar *case_normalized_key = NULL;
  Category *item;
	
  //gtk_tree_model_get_value (model, iter, column, &value);
  gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &item, -1);

  if(item !=  NULL)
  {
	  normalized_string = g_utf8_normalize (item->name, -1, G_NORMALIZE_ALL);
	  normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);

	  if (normalized_string && normalized_key)
		{
		  case_normalized_string = g_utf8_casefold (normalized_string, -1);
		  case_normalized_key = g_utf8_casefold (normalized_key, -1);

		  if (strncmp (case_normalized_key, case_normalized_string, strlen (case_normalized_key)) == 0)
		    retval = FALSE;
		}

	  g_free (normalized_key);
	  g_free (normalized_string);
	  g_free (case_normalized_key);
	  g_free (case_normalized_string);
  }
  return retval;
}



/*
**
*/
static GtkWidget *ui_bud_listview_new(void)
{
GtkTreeStore *store;
GtkWidget *treeview;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	//store
	store = gtk_tree_store_new (
		3,
		//NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_UINT
		);

	//sortable
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, ui_bud_listview_compare_funct, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);


	//treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	/* column 1 */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);	

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_bud_listview_cell_data_function_text, GINT_TO_POINTER(1), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	/* icon column */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	//gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_bud_listview_icon_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(treeview), ui_bud_listview_search_equal_func, NULL, NULL);

	
	//gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);

	return(treeview);
}



/*
** index 0 is all month, then 1 -> 12 are months
*/
static gchar *ui_bud_manage_getcsvbudgetstr(Category *item)
{
gchar *retval = NULL;
char buf[G_ASCII_DTOSTR_BUF_SIZE];

	//DB( g_print(" get budgetstr for '%s'\n", item->name) );

	if( !(item->flags & GF_CUSTOM) )
	{
		if( item->budget[0] )
		{
			g_ascii_dtostr (buf, sizeof (buf), item->budget[0]);
			retval = g_strdup(buf);

			//DB( g_print(" => %d: %s\n", 0, retval) );
		}
	}
	else
	{
	gint i;

		for(i=1;i<=12;i++)
		{
			//if( item->budget[i] )
			//{
			gchar *tmp = retval;

				g_ascii_dtostr (buf, sizeof (buf), item->budget[i]);

				if(retval != NULL)
				{
					retval = g_strconcat(retval, ";", buf, NULL);
					g_free(tmp);
				}
				else
					retval = g_strdup(buf);

				//DB( g_print(" => %d: %s\n", i, retval) );

			//}
		}
	}

	return retval;
}


static gint ui_bud_manage_category_exists (GtkTreeModel *model, gchar *level, gchar *type, gchar *name, GtkTreeIter *return_iter)
{
GtkTreeIter  iter, child;
gboolean     valid;
Category *entry;
gint pos = 0;

    if(model == NULL)
		return 0;

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

    while (valid)
    {
		gtk_tree_model_get (model, &iter, LST_DEFCAT_DATAS, &entry, -1);

		if(*level == '1')
		{
			if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
			{
				*return_iter = iter;
				return pos;
			}
		}
		else
		{
			if(*level == '2')
			{
				gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
				gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
				while(n_child > 0)
				{

					gtk_tree_model_get(GTK_TREE_MODEL(model), &child,
						LST_DEFCAT_DATAS, &entry,
						-1);

					if(entry->name && g_ascii_strcasecmp(name, entry->name) == 0)
					{
						*return_iter = child;
						return pos;
					}

					n_child--;
					gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
					pos++;
				}
			}
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		pos++;
    }

	return 0;
}


static void ui_bud_manage_load_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data = user_data;
gchar *filename = NULL;
GIOChannel *io;
const gchar *encoding;


	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");


	DB( g_print("(ui_bud_manage) load csv - data %p\n", data) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		encoding = homebank_file_getencoding(filename);

		io = g_io_channel_new_file(filename, "r", NULL);
		if(io != NULL)
		{
		GtkTreeModel *model;
		GtkTreeIter iter;
		gboolean error = FALSE;
		gchar *tmpstr;
		gint io_stat;

			DB( g_print(" -> encoding should be %s\n", encoding) );
			if( encoding != NULL )
			{
				g_io_channel_set_encoding(io, encoding, NULL);
			}

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));


			for(;;)
			{
				io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
				if( io_stat == G_IO_STATUS_EOF)
					break;
				if( io_stat == G_IO_STATUS_NORMAL)
				{
					if( tmpstr != NULL)
					{
					gchar **str_array;

						hb_string_strip_crlf(tmpstr);

						str_array = g_strsplit (tmpstr, ";", 15);
						// type; sign; name

						if( (g_strv_length (str_array) < 4 || *str_array[1] != '*') && (g_strv_length (str_array) < 15))
						{
							error = TRUE;
							break;
						}

						DB( g_print(" csv read '%s : %s : %s ...'\n", str_array[0], str_array[1], str_array[2]) );

						gint pos = ui_bud_manage_category_exists(model, str_array[0], str_array[1], str_array[2], &iter);

						DB( g_print(" pos=%d\n", pos) );

						if( pos != 0 )
						{
						gboolean budget;
						Category *tmpitem;
						gint i;

							gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
								LST_DEFCAT_DATAS, &tmpitem,
								-1);

							DB( g_print(" found cat, updating '%s'\n", tmpitem->name) );

							data->change++;

							tmpitem->flags &= ~(GF_CUSTOM);		//delete flag
							if( *str_array[1] == '*' )
							{
								tmpitem->budget[0] = g_ascii_strtod(str_array[3], NULL);

								DB( g_print(" monthly '%.2f'\n", tmpitem->budget[0]) );
							}
							else
							{
								tmpitem->flags |= (GF_CUSTOM);

								for(i=1;i<=12;i++)
								{
									tmpitem->budget[i] = g_ascii_strtod(str_array[2+i], NULL);
									DB( g_print(" month %d '%.2f'\n", i, tmpitem->budget[i]) );
								}
							}

							// if any value,set the flag to visual indicator
							budget = FALSE;
							tmpitem->flags &= ~(GF_BUDGET);		//delete flag
							for(i=0;i<=12;i++)
							{
								if(tmpitem->budget[i])
								{
									budget = TRUE;
									break;
								}
							}

							if(budget == TRUE)
								tmpitem->flags |= GF_BUDGET;
						}

						g_strfreev (str_array);
					}
					g_free(tmpstr);
				}

			}

			//update the treeview
			gtk_tree_selection_unselect_all (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)));


			g_io_channel_unref (io);

			if( error == TRUE )
			{
				ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The CSV file must contains the exact numbers of column,\nseparated by a semi-colon, please see the help for more details.")
					);
			}

		}

		g_free( filename );

	}
}


static void ui_bud_manage_save_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data = user_data;
gchar *filename = NULL;
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
GIOChannel *io;

	DB( g_print("(ui_bud_manage) save csv\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
	{

		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

		    while (valid)
		    {
			gchar *outstr, *outvalstr;
			Category *category;
			gchar type;

				gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, LST_DEFCAT_DATAS, &category, -1);

				if( category->name != NULL )
				{

				//level 1: category
					if( category->flags & GF_BUDGET )
					{
						type = (category->flags & GF_CUSTOM) ? ' ' : '*';

						outvalstr = ui_bud_manage_getcsvbudgetstr(category);
						outstr = g_strdup_printf("1;%c;%s;%s\n", type, category->name, outvalstr);
						DB( g_print("%s", outstr) );
						g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
						g_free(outstr);
						g_free(outvalstr);
					}


				//level 2: subcategory
					gint n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
					gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
					while(n_child > 0)
					{
						gtk_tree_model_get(GTK_TREE_MODEL(model), &child, LST_DEFCAT_DATAS, &category, -1);

						type = (category->flags & GF_CUSTOM) ? ' ' : '*';

						outvalstr = ui_bud_manage_getcsvbudgetstr(category);
						if( outvalstr )
						{
							outstr = g_strdup_printf("2;%c;%s;%s\n", type, category->name, outvalstr);
							DB( g_print("%s", outstr) );
							g_io_channel_write_chars(io, outstr, -1, NULL, NULL);
							g_free(outstr);
						}
						g_free(outvalstr);

						n_child--;
						gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
					}
				}

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

		g_free( filename );

	}

}


static void ui_bud_manage_expand_all(GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(ui_bud_manage) expand all (data=%x)\n", (guint)data) );

	gtk_tree_view_expand_all(GTK_TREE_VIEW(data->LV_cat));

}


static void ui_bud_manage_collapse_all(GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(ui_bud_manage) collapse all (data=%x)\n", (guint)data) );

	gtk_tree_view_collapse_all(GTK_TREE_VIEW(data->LV_cat));

}

static void ui_bud_manage_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_bud_manage_data *data;
gboolean name, custom, sensitive;
gint i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n(ui_bud_manage) update %x\n", (gint)data) );


	name = FALSE;
	if(data->cat != NULL)
	{
		name = data->cat->key == 0 ? FALSE : TRUE;
	}
	
	sensitive = name;

	gtk_widget_set_sensitive(data->label_budget, sensitive);
	gtk_widget_set_sensitive(data->CM_type[0], sensitive);
	gtk_widget_set_sensitive(data->CM_type[1], sensitive);

	gtk_widget_set_sensitive(data->label_options, sensitive);
	gtk_widget_set_sensitive(data->CM_force, sensitive);
	
	gtk_widget_set_sensitive(data->BT_clear, sensitive);

#if MYDEBUG == 1
	gint toto = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0]));
	DB( g_print(" monthly = %d\n", toto) );
#endif

	custom = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[1]));
	DB( g_print(" custom = %d\n\n", custom) );

	sensitive = name == FALSE ? FALSE : custom == TRUE ? FALSE: TRUE;
	gtk_widget_set_sensitive(data->spinner[0], sensitive);

	sensitive = name == FALSE ? FALSE : custom;
	for(i=0;i<12;i++)
	{
		gtk_widget_set_sensitive(data->label[i+1], sensitive);
		gtk_widget_set_sensitive(data->spinner[i+1], sensitive);
	}

}


static void ui_bud_manage_clear(GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data;
gchar *title;
gchar *secondtext;
gint result, i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_bud_manage) clear\n") );


	title = _("Are you sure you want to clear input?");

	secondtext = _("If you proceed, every amount will be set to 0.");
	
	result = ui_dialog_msg_confirm_alert(
			GTK_WINDOW(data->window),
			title,
			secondtext,
			_("_Clear")
		);

	if( result == GTK_RESPONSE_OK )
	{
		//g_signal_handler_block(data->CM_type[0], data->handler_id[HID_CUSTOM]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_type[0]), TRUE);
		//g_signal_handler_unblock(data->CM_type[0], data->handler_id[HID_CUSTOM]);

		for(i=0;i<=12;i++)
		{
			//g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), 0);
			data->cat->budget[i] = 0;

			//g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
		}

		data->cat->flags &= ~(GF_BUDGET);	//delete flag

		gtk_widget_queue_draw (data->LV_cat);
	}
		
}


static void ui_bud_manage_set(GtkWidget *widget, gpointer user_data)
{
struct ui_bud_manage_data *data;
gint active;
gint i;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_bud_manage) set\n") );

	active = data->cat->flags & GF_CUSTOM ? 1 : 0;
	//data->custom = active;

	//DB( g_print(" set custom to %d\n", data->custom) );

	g_signal_handler_block(data->CM_type[0], data->handler_id[HID_CUSTOM]);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_type[active]), TRUE);
	g_signal_handler_unblock(data->CM_type[0], data->handler_id[HID_CUSTOM]);

	for(i=0;i<=12;i++)
	{
		//g_signal_handler_block(data->spinner[i], data->spinner_hid[i]);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->spinner[i]), data->cat->budget[i]);
		//DB( g_print(" %.2f\n", data->cat->budget[i]) );

		//g_signal_handler_unblock(data->spinner[i], data->spinner_hid[i]);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_force), (data->cat->flags & GF_FORCED) ? 1 : 0);

}

static gboolean ui_bud_manage_has_budget(GtkSpinButton *spinbutton, gpointer user_data)
{
struct ui_bud_manage_data *data;
gint i;
Category *item;
gboolean retval;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(spinbutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_bud_manage) has budget\n") );

	retval = FALSE;
	
	item = data->cat;

	if( item != NULL )
	{
		item->flags &= ~(GF_BUDGET);	//delete flag
		for(i=0;i<=12;i++)
		{
			gtk_spin_button_update(GTK_SPIN_BUTTON(data->spinner[i]));
			if( gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->spinner[i])) )
			{
				retval = TRUE;
				item->flags |= GF_BUDGET;
				break;
			}
		}
		
	}
	return retval;
}


static void ui_bud_manage_getlast(struct ui_bud_manage_data *data)
{
gboolean budget, change;
gint i;
Category *item;
gdouble oldvalue;
	gint active;

	item = data->lastcatitem;

	DB( g_print("****\n(ui_bud_manage) getlast for '%s'\n", item->name ) );

	if( item != NULL )
	{
	gushort old_flags = item->flags;

		item->flags &= ~(GF_CUSTOM);
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[0])) == FALSE)
			item->flags |= GF_CUSTOM;

		DB( g_print(" custom flag=%d\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type[1]))) );

		// if any value,set the flag to visual indicator
		budget = FALSE;
		change = FALSE;
		item->flags &= ~(GF_BUDGET);	//delete flag
		for(i=0;i<=12;i++)
		{
			gtk_spin_button_update(GTK_SPIN_BUTTON(data->spinner[i]));
			oldvalue = item->budget[i];

			item->budget[i] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->spinner[i]));

			if( oldvalue != item->budget[i])
				change = TRUE;

			DB( g_print(" set value %d to %.2f\n", i, item->budget[i]) );
			if(item->budget[i])
			{
				budget = TRUE;
			}
		}

		item->flags &= ~(GF_FORCED);	//delete flag
		active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_force));
		if(active == 1)
			item->flags |= GF_FORCED;

		if(budget == TRUE || active == 1)
			item->flags |= GF_BUDGET;

		// compute chnages
		if( (old_flags != item->flags) || change )
			data->change++;

		gtk_widget_queue_draw (data->LV_cat);

	}

}


static void ui_bud_manage_selection_change(GtkWidget *treeview, gpointer user_data)
{
struct ui_bud_manage_data *data;
GtkTreeModel *model;
GtkTreeIter iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_bud_manage) changed\n") );

	data->cat = NULL;

	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DEFCAT_DATAS, &item,
			-1);

		DB( g_print(" selected %s\n", item->name) );

		if(data->lastcatitem != NULL && item != data->lastcatitem)
		{
			DB( g_print(" -> should do a get for last selected (%s)\n", data->lastcatitem->name) );
			ui_bud_manage_getlast(data);
		}


		data->cat = item;
		data->lastcatitem = item;

		ui_bud_manage_set(treeview, NULL);
	}
	else
	{
		data->lastcatitem = NULL;
	}

	ui_bud_manage_update(treeview, NULL);

}

static void ui_bud_manage_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
//struct ui_bud_manage_data *data;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_bud_manage) toggle\n") );

	//ui_bud_manage_get(GTK_WIDGET(radiobutton), GINT_TO_POINTER(FIELD_TYPE));

	//data->custom ^= 1;
	ui_bud_manage_update(GTK_WIDGET(radiobutton), NULL);
}


void ui_bud_manage_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_bud_manage_selection_change(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


static gboolean ui_bud_manage_cleanup(struct ui_bud_manage_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(ui_bud_manage) cleanup\n") );

	if(data->lastcatitem != NULL)
	{
		DB( g_print(" -> should do a get for last selected (%s)\n", data->lastcatitem->name) );
		ui_bud_manage_getlast(data);
	}

	//do_application_specific_something ();
	DB( g_print(" accept\n") );

	GLOBALS->changes_count += data->change;

	DB( g_print(" free tmp_list\n") );

	return doupdate;
}

static void ui_bud_manage_populate_listview(struct ui_bud_manage_data *data)
{
gint type;

	type = radio_get_active(GTK_CONTAINER(data->RA_type)) == 1 ? CAT_TYPE_INCOME : CAT_TYPE_EXPENSE;

	ui_cat_listview_populate(data->LV_cat, type);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
}


static void ui_bud_manage_setup(struct ui_bud_manage_data *data)
{

	DB( g_print("(ui_bud_manage) setup\n") );

	data->tmp_list = NULL;
	data->change = 0;
	data->cat = NULL;
	data->lastcatitem = NULL;

	ui_bud_manage_populate_listview(data);
	
}


static void ui_bud_manage_type_changed_cb (GtkToggleButton *button, gpointer user_data)
{
	ui_bud_manage_populate_listview(user_data);
	//g_print(" toggle type=%d\n", gtk_toggle_button_get_active(button));
}





GtkWidget *ui_bud_manage_dialog (void)
{
struct ui_bud_manage_data data;
GtkWidget *dialog, *content_area;
GtkWidget *content_grid, *group_grid, *table, *scrollwin, *label;
GtkWidget *treeview, *hpaned, *bbox, *vbox, *hbox;
GtkWidget *menu, *menuitem, *widget, *image, *tbar;
GtkToolItem *toolitem;
guint i;
gint w, h;
gint crow, row;

	memset(&data, 0, sizeof(struct ui_bud_manage_data));

	dialog = gtk_dialog_new_with_buttons (_("Manage Budget"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    _("_Close"),
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = dialog;

	gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_BUDGET);

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(dialog), -1, h/PHI);

	
	//store our window private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_bud_manage) window=%p, inst_data=%p\n", dialog, &data) );

	//window contents
	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));	 	// return a vbox

  //our table
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	g_object_set(table, "margin", SPACING_MEDIUM, NULL);
	gtk_box_pack_start (GTK_BOX (content_area), table, TRUE, TRUE, 0);

	crow = 0;
	bbox = make_radio(CYA_CAT_TYPE, TRUE, GTK_ORIENTATION_HORIZONTAL);
	data.RA_type = bbox;
	gtk_widget_set_halign (bbox, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (table), bbox, 0, crow, 1, 1);

	widget = radio_get_nth_widget(GTK_CONTAINER(bbox), 1);
	if(widget)
		g_signal_connect (widget, "toggled", G_CALLBACK (ui_bud_manage_type_changed_cb), &data);

	menu = gtk_menu_new ();
	gtk_widget_set_halign (menu, GTK_ALIGN_END);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Import CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_bud_manage_load_csv), &data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("E_xport CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_bud_manage_save_csv), &data);
	
	gtk_widget_show_all (menu);
	
	widget = gtk_menu_button_new();
	image = gtk_image_new_from_icon_name (ICONNAME_HB_BUTTON_MENU, GTK_ICON_SIZE_MENU);

	//gchar *thename;
	//gtk_image_get_icon_name(image, &thename, NULL);
	//g_print("the name is %s\n", thename);

	g_object_set (widget, "image", image, "popup", GTK_MENU(menu),  NULL);

	gtk_widget_set_hexpand (widget, FALSE);
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (table), widget, 0, crow++, 1, 1);



	crow++;
	hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	//gtk_container_set_border_width (GTK_CONTAINER(hpaned), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (table), hpaned, 0, crow++, 1, 1);

	/* left area */
	//list
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_right(vbox, SPACING_SMALL);
	gtk_paned_pack1 (GTK_PANED(hpaned), vbox, TRUE, FALSE);
	
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 	treeview = (GtkWidget *)ui_bud_listview_new();
 	data.LV_cat = treeview;
	gtk_widget_set_size_request(treeview, HB_MINWIDTH_LIST, -1);
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_box_pack_start (GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

	//list toolbar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(tbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_ICONS);
	gtk_box_pack_start (GTK_BOX (vbox), tbar, FALSE, FALSE, 0);

	gtk_style_context_add_class (gtk_widget_get_style_context (tbar), GTK_STYLE_CLASS_INLINE_TOOLBAR);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, TRUE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);
	
		widget = make_image_button(ICONNAME_HB_BUTTON_EXPAND, _("Expand all"));
		data.BT_expand = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_HB_BUTTON_COLLAPSE, _("Collapse all"));
		data.BT_collapse = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	
	/* right area */
	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	//gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_widget_set_margin_left(content_grid, SPACING_SMALL);
	gtk_paned_pack2 (GTK_PANED(hpaned), content_grid, FALSE, FALSE);

	crow = 0;

	
	// group :: General
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Budget for each month"));
	data.label_budget = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	widget = gtk_radio_button_new_with_label (NULL, _("is the same"));
	data.CM_type[0] = widget;
	gtk_widget_set_hexpand (widget, TRUE);
 	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 4, 1);

	row++;
	widget = make_amount(label);
	data.spinner[0] = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	g_signal_connect (G_OBJECT (data.spinner[0]), "value-changed", G_CALLBACK (ui_bud_manage_has_budget), NULL);
	
	widget =  gtk_button_new_with_mnemonic (_("_Clear input"));
	data.BT_clear = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 4, row, 1, 1);

	
	// propagate button
 	/*row++;
	button = gtk_button_new_with_label(_("Propagate"));
	gtk_grid_attach (GTK_GRID (group_grid), button, 1, 2, row, row+1);
	*/

 	row++;
    widget = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (data.CM_type[0]), _("is different"));
	data.CM_type[1] = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 4, 1);

	row++;
	for(i=0;i<12;i++)
	{
	gint l, t;
		
		l = ((i<6) ? 1 : 3);
		t = row + ((i<6) ? i : i-6);

		label = make_label_widget(months[i]);
		data.label[i+1] = label;
		gtk_grid_attach (GTK_GRID (group_grid), label, l, t, 1, 1);

		widget = make_amount(label);
		data.spinner[i+1] = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, l+1, t, 1, 1);

		g_signal_connect (G_OBJECT (data.spinner[i+1]), "value-changed", G_CALLBACK (ui_bud_manage_has_budget), NULL);
		
		//DB( g_print("(ui_bud_manage) %s, col=%d, row=%d", months[i], col, row) );
	}

	// group :: Options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(_("Options"));
	data.label_options = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("_Force monitoring this category"));
	data.CM_force = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 4, 1);


	//connect all our signals
    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cat)), "changed", G_CALLBACK (ui_bud_manage_selection), NULL);
	//g_signal_connect (GTK_TREE_VIEW(data.LV_cat), "row-activated", G_CALLBACK (ui_bud_manage_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_expand), "clicked", G_CALLBACK (ui_bud_manage_expand_all), NULL);
	g_signal_connect (G_OBJECT (data.BT_collapse), "clicked", G_CALLBACK (ui_bud_manage_collapse_all), NULL);

	data.handler_id[HID_CUSTOM] = g_signal_connect (data.CM_type[0], "toggled", G_CALLBACK (ui_bud_manage_toggle), NULL);

	g_signal_connect (G_OBJECT (data.BT_clear), "clicked", G_CALLBACK (ui_bud_manage_clear), NULL);

	//data.custom = FALSE;
	//gtk_widget_set_sensitive(data.table, FALSE);

	//setup, init and show window
	ui_bud_manage_setup(&data);
	ui_bud_manage_update(dialog, NULL);



	gtk_widget_show_all (dialog);

	//result
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
  switch (result)
    {
      case GTK_RESPONSE_ACCEPT:
         //do_application_specific_something ();
         break;
      default:
         //do_nothing_since_dialog_was_cancelled ();
         break;
    }

	// cleanup and destroy
	ui_bud_manage_cleanup(&data, result);
	gtk_widget_destroy (dialog);

	return NULL;
}

