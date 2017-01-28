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

#include "gtk-chart.h"
#include "ui-widgets.h"

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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


extern gchar *CYA_FLT_RANGE[];




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */



void
gimp_label_set_attributes (GtkLabel *label,
                           ...)
{
  PangoAttribute *attr  = NULL;
  PangoAttrList  *attrs;
  va_list         args;

  g_return_if_fail (GTK_IS_LABEL (label));

  attrs = pango_attr_list_new ();

  va_start (args, label);

  do
    {
      PangoAttrType attr_type = va_arg (args, PangoAttrType);

      if (attr_type <= 0)
        attr_type = PANGO_ATTR_INVALID;

      switch (attr_type)
        {
        case PANGO_ATTR_LANGUAGE:
          attr = pango_attr_language_new (va_arg (args, PangoLanguage *));
          break;

        case PANGO_ATTR_FAMILY:
          attr = pango_attr_family_new (va_arg (args, const gchar *));
          break;

        case PANGO_ATTR_STYLE:
          attr = pango_attr_style_new (va_arg (args, PangoStyle));
          break;

        case PANGO_ATTR_WEIGHT:
          attr = pango_attr_weight_new (va_arg (args, PangoWeight));
          break;

        case PANGO_ATTR_VARIANT:
          attr = pango_attr_variant_new (va_arg (args, PangoVariant));
          break;

        case PANGO_ATTR_STRETCH:
          attr = pango_attr_stretch_new (va_arg (args, PangoStretch));
          break;

        case PANGO_ATTR_SIZE:
          attr = pango_attr_size_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_FONT_DESC:
          attr = pango_attr_font_desc_new (va_arg (args,
                                                   const PangoFontDescription *));
          break;

        case PANGO_ATTR_FOREGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_foreground_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_BACKGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_background_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_UNDERLINE:
          attr = pango_attr_underline_new (va_arg (args, PangoUnderline));
          break;

        case PANGO_ATTR_STRIKETHROUGH:
          attr = pango_attr_strikethrough_new (va_arg (args, gboolean));
          break;

        case PANGO_ATTR_RISE:
          attr = pango_attr_rise_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_SCALE:
          attr = pango_attr_scale_new (va_arg (args, gdouble));
          break;

        default:
          //g_warning ("%s: invalid PangoAttribute type %d", G_STRFUNC, attr_type);
        case PANGO_ATTR_INVALID:
          attr = NULL;
          break;
        }

      if (attr)
        {
          attr->start_index = 0;
          attr->end_index   = -1;
          pango_attr_list_insert (attrs, attr);
        }
    }
  while (attr);

  va_end (args);

  gtk_label_set_attributes (label, attrs);
  pango_attr_list_unref (attrs);
}


void hb_widget_visible(GtkWidget *widget, gboolean visible)
{
	if(!GTK_IS_WIDGET(widget))
		return;

	if(visible)
	{
		gtk_widget_show(widget);
	}
	else
	{
		gtk_widget_hide(widget);
	}
}


void ui_label_set_integer(GtkLabel *label, gint value)
{
gchar buf[16];

	g_snprintf(buf, 16, "%d", value);
	gtk_label_set_text (label, buf);
}


void ui_gtk_entry_set_text(GtkWidget *widget, gchar *text)
{
	DB( g_print(" set text to '%s'\n", text) );

	if( text != NULL )
		gtk_entry_set_text(GTK_ENTRY(widget), text);
	else
		gtk_entry_set_text(GTK_ENTRY(widget), "");
}

void ui_gtk_entry_replace_text(GtkWidget *widget, gchar **storage)
{
const gchar *text;

	DB( g_print(" storage is '%p' at '%p'\n", *storage, storage) );

	/* free any previous string */
	if( *storage != NULL )
	{
		g_free(*storage);
	}

	*storage = NULL;
	text = gtk_entry_get_text(GTK_ENTRY(widget));
	*storage = g_strdup(text);
}


GtkWidget *make_label_group(gchar *str)
{
GtkWidget *label = gtk_label_new (str);

	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	return label;
}


GtkWidget *make_label_widget(char *str)
{
GtkWidget *label = gtk_label_new_with_mnemonic (str);

	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	return label;
}


GtkWidget *make_label(char *str, gfloat xalign, gfloat yalign)
{
GtkWidget *label = gtk_label_new_with_mnemonic (str);

	//todo: deprecated in 3.14
	gtk_misc_set_alignment (GTK_MISC (label), xalign, yalign);
	//gtk_label_set_xalign(GTK_LABEL(label), xalign);
	//gtk_label_set_yalign(GTK_LABEL(label), yalign);
	return label;
}


/*
**
*/
GtkWidget *make_text(gfloat xalign)
{
GtkWidget *entry;

	entry = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE(entry), FALSE);
	g_object_set(entry, "xalign", xalign, NULL);
	return entry;
}


GtkWidget *make_search(GtkWidget *label)
{
GtkWidget *search;

	search = gtk_search_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(search), _("Search...") );

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), search);

	return search;
}



/*
**
*/
GtkWidget *make_string(GtkWidget *label)
{
GtkWidget *entry;

	entry = gtk_entry_new ();

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);

	return entry;
}


GtkWidget *make_image_button(gchar *icon_name, gchar *tooltip_text)
{
GtkWidget *button, *image;

	//todo 3.10 use gtk_button_new_from_icon_name 

	button = gtk_button_new();
	image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);
	g_object_set (button, "image", image, NULL);
	if(tooltip_text != NULL)
		gtk_widget_set_tooltip_text(button, tooltip_text);

	return button;
}



/*
**
*/
GtkWidget *make_memo_entry(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *entry;
GtkEntryCompletion *completion;
GList *lmem, *list;

	store = gtk_list_store_new (1, G_TYPE_STRING);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
    gtk_entry_completion_set_text_column (completion, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_completion (GTK_ENTRY (entry), completion);

	g_object_unref(store);

	//populate
	//gtk_list_store_clear (GTK_LIST_STORE(store));

	lmem = list = g_hash_table_get_keys(GLOBALS->h_memo);
	while (list != NULL)
	{
	GtkTreeIter  iter;

		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (GTK_LIST_STORE(store), &iter, 0, list->data, -1);

		list = g_list_next(list);
	}

	g_list_free(lmem);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);

	return entry;
}


/*
**
*/
GtkWidget *make_string_maxlength(GtkWidget *label, guint max_length)
{
GtkWidget *entry;

	entry = make_string(label);
	gtk_entry_set_max_length(GTK_ENTRY(entry), max_length);

	return entry;
}



static void hb_amount_insert_text_handler (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data)
{
GtkEditable *editable = GTK_EDITABLE(entry);
int i, count=0, dcpos=-1;
gchar *result = g_new0 (gchar, length+1);
const gchar *numtext;

	//g_message("insert_text-handler: text:%s - pos:%d - length:%d", text, *position, length);

	numtext = gtk_entry_get_text(entry);
	for (i=0 ; numtext[i]!='\0' ; i++)
	{
 		if(numtext[i]==',' || numtext[i]=='.')
			dcpos = i;
	}


	//g_message("previous text='%s' dcpos:'%d'", numtext, dcpos);
	for (i=0 ; i < length ; i++)
	{
		if( isdigit(text[i]) && ( (*position <= dcpos+2) || dcpos < 0) )
			goto inserttext;

		if( text[i]=='-' && *position==0 )	/* minus sign only at position 0 */
			goto inserttext;

		if( dcpos < 0 && (text[i]=='.' || text[i]==',') )	/* decimal separator if not in previous string */
			result[count++] = '.';

		continue;

	inserttext:
		result[count++] = text[i];
	}

	if (count > 0) {
		g_signal_handlers_block_by_func (G_OBJECT (editable), G_CALLBACK (hb_amount_insert_text_handler), data);
		gtk_editable_insert_text (editable, result, count, position);
		g_signal_handlers_unblock_by_func (G_OBJECT (editable), G_CALLBACK (hb_amount_insert_text_handler), data);
	}
	g_signal_stop_emission_by_name (G_OBJECT (editable), "insert-text");

	g_free (result);
}

/*
**
*/
GtkWidget *make_amount(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	//adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -G_MAXDOUBLE, G_MAXDOUBLE, 0.01, 1.0, 0.0);
	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, -8589934588, 8589934588, 0.01, 1.0, 0.0);
	spinner = gtk_spin_button_new (adj, 1.0, 2);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	g_signal_connect(G_OBJECT(spinner), "insert-text",
			 G_CALLBACK(hb_amount_insert_text_handler),
			 NULL);

	return spinner;
}


GtkWidget *make_exchange_rate(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 99999, 0.01, 1.0, 0.0);
	spinner = gtk_spin_button_new (adj, 1.0, 6);
	//gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}

/*
**
*/
GtkWidget *make_numeric(GtkWidget *label, gdouble min, gdouble max)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, min, max, 1.0, 10.0, 0.0);
	spinner = gtk_spin_button_new (adj, 0, 0);
	//gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}

/*
**
*/
GtkWidget *make_scale(GtkWidget *label)
{
GtkWidget *scale;

	scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, GTK_CHART_MINBARW, GTK_CHART_MAXBARW, 1.0);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	gtk_range_set_value(GTK_RANGE(scale), GTK_CHART_BARW);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), scale);

	return scale;
}

/*
**
*/
GtkWidget *make_long(GtkWidget *label)
{
GtkWidget *spinner;

	spinner = make_numeric(label, 0.0, G_MAXINT);
	return spinner;
}


GtkWidget *make_year(GtkWidget *label)
{
GtkWidget *spinner;
GtkAdjustment *adj;

	adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 1900, 2200, 1.0, 10.0, 0.0);
	spinner = gtk_spin_button_new (adj, 0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinner), TRUE);
	g_object_set(spinner, "xalign", 1.0, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), spinner);

	return spinner;
}


static gboolean
is_separator (GtkTreeModel *model,
	      GtkTreeIter  *iter,
	      gpointer      data)
{
  //GtkTreePath *path;
  gboolean retval;
  gchar *txt;

	gtk_tree_model_get (model, iter, 0, &txt, -1);

	retval = *txt == 0 ? TRUE : FALSE;
  //path = gtk_tree_model_get_path (model, iter);
  //result = gtk_tree_path_get_indices (path)[0] == 4;
  //gtk_tree_path_free (path);

  return retval;
}

static void
set_sensitive (GtkCellLayout   *cell_layout,
	       GtkCellRenderer *cell,
	       GtkTreeModel    *tree_model,
	       GtkTreeIter     *iter,
	       gpointer         data)
{
  GtkTreePath *path;
  gint *indices;
  gboolean sensitive;

  path = gtk_tree_model_get_path (tree_model, iter);
  indices = gtk_tree_path_get_indices (path);
  sensitive = indices[0] != FLT_RANGE_OTHER;  
  gtk_tree_path_free (path);

  g_object_set (cell, "sensitive", sensitive, NULL);
}


GtkWidget *make_cycle(GtkWidget *label, gchar **items)
{
GtkWidget *combobox;
guint i;

	combobox = gtk_combo_box_text_new ();

	for (i = 0; items[i] != NULL; i++)
	{
		if(*items[i] != 0)
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combobox), _(items[i]));
		else
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combobox), "");
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combobox), is_separator, NULL, NULL);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}


GtkWidget *make_daterange(GtkWidget *label, gboolean custom)
{
GtkWidget *combobox;
GList *renderers, *list;
GtkCellRenderer *renderer;
gchar **items = CYA_FLT_RANGE;
guint i;

	combobox = gtk_combo_box_text_new ();

	for (i = 0; items[i] != NULL; i++)
	{
		if(*items[i] != 0)
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combobox), _(items[i]));
		else
			gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(combobox), "");
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	// special stuffs
	renderers = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT(combobox));
	if(g_list_length(renderers) == 1 && custom == FALSE)
	{
		list = g_list_first(renderers);
		renderer = list->data;
	
	
		gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combobox),
						renderer,
						set_sensitive,
						NULL, NULL);
	}	
	g_list_free(renderers);

	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combobox), is_separator, NULL, NULL);

	return combobox;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
**
*/
GtkWidget *make_radio(gchar **items, gboolean buttonstyle, GtkOrientation orientation)
{
GtkWidget *box, *button;
guint i;

	box = gtk_box_new (orientation, 0);

    button = gtk_radio_button_new_with_label (NULL, _(items[0]));
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), !buttonstyle);
    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
	for (i = 1; items[i] != NULL; i++)
	{
		button = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (button), _(items[i]));
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button), !buttonstyle);
	    gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
	}

	if(buttonstyle)
	{
		gtk_style_context_add_class (gtk_widget_get_style_context (box), GTK_STYLE_CLASS_LINKED);
		gtk_style_context_add_class (gtk_widget_get_style_context (box), GTK_STYLE_CLASS_RAISED);
	}
	
	return box;
}


gint radio_get_active (GtkContainer *container)
{
GList *lchild, *list;
GtkWidget *radio;
gint i, retval = 0;

	if(!GTK_IS_CONTAINER(container))
		return -1;

	lchild = list = gtk_container_get_children (container);
	for(i=0;list != NULL;i++)
	{
		radio = list->data;
		if(GTK_IS_TOGGLE_BUTTON(radio))
		{
			if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == TRUE )
			{
				retval = i;
				break;
			}
		}
		list = g_list_next(list);
	}
	g_list_free(lchild);
	
	return retval;
}

GtkWidget *radio_get_nth_widget (GtkContainer *container, gint nth)
{
GList *lchild, *list;
GtkWidget *radio;

	if(!GTK_IS_CONTAINER(container))
		return NULL;

	lchild = list = gtk_container_get_children (container);
	radio = g_list_nth_data (list, nth);
	g_list_free(lchild);
	return radio;   //may return NULL
}



void radio_set_active (GtkContainer *container, gint active)
{
GList *lchild, *list;
GtkWidget *radio;

	if(!GTK_IS_CONTAINER(container))
		return;

	lchild = list = gtk_container_get_children (container);
	radio = g_list_nth_data (list, active);
	if(radio != NULL && GTK_IS_TOGGLE_BUTTON(radio))
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(radio), TRUE);
	}
	g_list_free(lchild);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
id  ofx  			english                           french
 ---------------------------------------------------------------------
 0	--------      	(none)                            (aucun) 
 1	--------      	credit card                       carte de crédit
 2	OFX_CHECK     	Check                             cheque
 3  OFX_CASH      	Cash  	withdrawal                retrait espece
 4	OFX_XFER      	Transfer                          virement
 5	--------      	internal transfer                 virement compte 
 6	--------      	(debit card)                      (carte de paiement
 7	OFX_REPEATPMT 	Repeating payment/standing order  Paiement recurrent/Virement auto.
		                                                   
 8	OFX_PAYMENT   	Electronic payment                télépaiement
 9	OFX_DEP 	    Deposit                           dépôt
10	OFX_FEE       	FI fee                            frais bancaires

		                                               
	 OFX_DIRECTDEBIT 	Merchant initiated debit     prelevement
	 OFX_OTHER 	Somer other type of transaction      autre

other OFX values:

OFX_CREDIT 	Generic credit
OFX_DEBIT 	Generic debit
OFX_INT 	Interest earned or paid (Note: Depends on signage of amount)
OFX_DIV 	Dividend
OFX_SRVCHG 	Service charge
-OFX_DEP 	Deposit
OFX_ATM 	ATM debit or credit (Note: Depends on signage of amount)
OFX_POS 	Point of sale debit or credit (Note: Depends on signage of amount)
-OFX_XFER 	Transfer
-OFX_CHECK 	Check
-OFX_PAYMENT 	Electronic payment
-OFX_CASH 	Cash withdrawal
OFX_DIRECTDEP 	Direct deposit
OFX_DIRECTDEBIT 	Merchant initiated debit
-OFX_REPEATPMT 	Repeating payment/standing order
OFX_OTHER 	Somer other type of transaction 
*/

enum
{
	LST_PAYMODE_ICONNAME,
	LST_PAYMODE_LABEL,
	NUM_LST_PAYMODE
};

char *paymode_iconnames[NUM_PAYMODE_MAX] =
{
	"pm-none",
	"pm-ccard",
	"pm-check",
	"pm-cash" ,
	"pm-transfer",
	"pm-intransfer",
	"pm-dcard",
	"pm-standingorder",
	"pm-epayment",
	"pm-deposit",
	"pm-fifee",
	"pm-directdebit"
};

char *paymode_label_names[NUM_PAYMODE_MAX] =
{
	N_("(none)"),
	N_("Credit card"),
	N_("Check"),
	N_("Cash"),
	N_("Transfer"),
	N_("Internal transfer"),
	N_("Debit card"),
	N_("Standing order"),
	N_("Electronic payment"),
	N_("Deposit"),
	//TRANSLATORS: Financial institution fee
	N_("FI fee"),
	N_("Direct Debit")
};

/* nota: used in ui-filter */
gchar *get_paymode_icon_name(gint index)
{
	if(index >= NUM_PAYMODE_MAX)
		return NULL;

	return paymode_iconnames[index];
}


/*
** Make a paymode combobox widget
*/

static void
paymode_set_sensitive (GtkCellLayout   *cell_layout,
	       GtkCellRenderer *cell,
	       GtkTreeModel    *tree_model,
	       GtkTreeIter     *iter,
	       gpointer         data)
{
  GtkTreePath *path;
  gint *indices;
  gboolean sensitive;

  path = gtk_tree_model_get_path (tree_model, iter);
  indices = gtk_tree_path_get_indices (path);
  sensitive = (indices[0] == PAYMODE_INTXFER) ? FALSE : TRUE;  
  gtk_tree_path_free (path);
  

  g_object_set (cell, "sensitive", sensitive, NULL);
}



static GtkWidget *make_paymode_internal(GtkWidget *label, gboolean intxfer)
{
GtkListStore *store;
GtkTreeIter iter;
GtkWidget *combobox;
GtkCellRenderer *renderer, *r1, *r2;
guint i;

	store = gtk_list_store_new (
		NUM_LST_PAYMODE,
		G_TYPE_STRING,
		G_TYPE_STRING
		);

	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	renderer = r1 = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "icon-name", LST_PAYMODE_ICONNAME);

	renderer = r2 = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "text", LST_PAYMODE_LABEL);


	if( intxfer == FALSE )
	{
		gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combobox),
						r1,
						paymode_set_sensitive,
						NULL, NULL);
		gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combobox),
						r2,
						paymode_set_sensitive,
						NULL, NULL);
	}

	//populate our combobox model
	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			LST_PAYMODE_ICONNAME, paymode_iconnames[i],
			LST_PAYMODE_LABEL, _(paymode_label_names[i]),
			-1);
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}



GtkWidget *make_paymode(GtkWidget *label)
{
	return make_paymode_internal(label, TRUE);
}

GtkWidget *make_paymode_nointxfer(GtkWidget *label)
{
	return make_paymode_internal(label, FALSE);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#define NUM_NAINEX_MAX 3

enum
{
	LST_NAINEX_ICONNAME,
	LST_NAINEX_LABEL,
	NUM_LST_NAINEX
};

char *nainex_iconnames[NUM_NAINEX_MAX] =
{
	"flt-inactive",
	"flt-include",
	"flt-exclude",
};

char *nainex_label_names[NUM_NAINEX_MAX] =
{
	N_("Inactive"),
	N_("Include"),
	N_("Exclude")
};


/*
** Make a nainex combobox widget
*/
GtkWidget *make_nainex(GtkWidget *label)
{
GtkListStore  *store;
GtkTreeIter    iter;
GtkWidget *combobox;
GtkCellRenderer    *renderer;
guint i;

	//store
	store = gtk_list_store_new (
		NUM_LST_NAINEX,
		G_TYPE_STRING,
		G_TYPE_STRING
		);

	//combobox
	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	//column 1
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "icon-name", LST_NAINEX_ICONNAME);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, FALSE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(combobox), renderer, "text", LST_NAINEX_LABEL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	//populate our combobox model
	for(i=0;i<NUM_NAINEX_MAX;i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			LST_NAINEX_ICONNAME, nainex_iconnames[i],
			LST_NAINEX_LABEL, _(nainex_label_names[i]),
			-1);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}

