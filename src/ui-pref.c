/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2014 Maxime DOYEN
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

#include "ui-pref.h"
#include "dsp_mainwindow.h"
#include "gtk-chart-colors.h"


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


enum {
	LST_PREF_SMALLPIXBUF,
	LST_PREF_ICON,
	LST_PREF_NAME,
	LST_PREF_PAGE,
	LST_PREF_MAX
};

enum
{
  COLUMN_VISIBLE,
  COLUMN_NAME,
  COLUMN_ID,
  NUM_COLUMNS
};

enum
{
	PREF_GENERAL,
	PREF_INTERFACE,
	PREF_COLUMNS,
	PREF_DISPLAY,
	PREF_IMPORT,
	PREF_REPORT,
	PREF_EURO,
	PREF_MAX
};

GdkPixbuf *pref_pixbuf[PREF_MAX];


static gchar *pref_pixname[PREF_MAX] = {
"prf-general",
"prf-interface",
"prf-columns",
"prf-display",
"prf-import",
"prf-report",
"prf-euro",			// to be renamed
//"prf_charts.svg"
};

static gchar *pref_name[PREF_MAX]    = {
N_("General"),
N_("Interface"),
N_("Transactions"),
N_("Display format"),
N_("Import/Export"),
N_("Report"),
N_("Euro minor")
//
};

static gchar *CYA_TOOLBAR_STYLE[] = {
N_("System defaults"),
N_("Icons only"),
N_("Text only"),
N_("Text under icons"),
N_("Text beside icons"),
NULL
};

gchar *CYA_TANGO_COLORS[] = {
"----",
N_("Tango light"),
N_("Tango medium"),
N_("Tango dark"),
NULL
};

gchar *CYA_IMPORT_DATEORDER[] = {
N_("m-d-y"),
N_("d-m-y"),
N_("y-m-d"),
NULL
};

gchar *CYA_IMPORT_OFXNAME[] = {
N_("Memo"),
N_("Payee"),
NULL
};

gchar *CYA_IMPORT_OFXMEMO[] = {
N_("Ignore"),
N_("Append to Info"),
N_("Append to Memo"),
NULL
};


extern gchar *CYA_CHART_COLORSCHEME[];
extern gchar *CYA_MONTHS[];

typedef struct
{
	gchar		*iso;
	gchar		*name;
	gdouble		value;
	//gchar		*prefix_symbol;		/* max symbol is 3 digits in unicode */
	//gchar		*suffix_symbol;		/* but mostly is 1 digit */
	gchar		*symbol;
	gboolean	is_prefix;
	gchar		*decimal_char;
	gchar		*grouping_char;
	gshort		frac_digits;
} EuroParams;


/*
source:
 http://en.wikipedia.org/wiki/Currencies_of_the_European_Union
 http://www.xe.com/euro.php
 http://fr.wikipedia.org/wiki/Liste_des_unit%C3%A9s_mon%C3%A9taires_remplac%C3%A9es_par_l%27euro
 http://www.inter-locale.com/LocalesDemo.jsp
*/
static EuroParams euro_params[] =
{
//                           , rate     , symb  , prfx , dec, grp, frac
// ---------------------------------------------------------------------
	{ ""   , "--------"      , 1.0		, ""    , FALSE, ",", ".", 2  },
	{ "ATS", "Austria"       , 13.7603	, "S"   , TRUE , ",", ".", 2  },	// -S 1.234.567,89
	{ "BEF", "Belgium"       , 40.3399	, "BF"  , TRUE , ",", ".", 2  },	// BF 1.234.567,89 -
	{ "FIM", "Finland"       , 5.94573	, "mk"  , FALSE, ",", " ", 2  },	// -1 234 567,89 mk
	{ "FRF", "France"        , 6.55957	, "F"   , FALSE, ",", " ", 2  },	// -1 234 567,89 F
	{ "DEM", "Germany"       , 1.95583	, "DM"  , FALSE, ",", ".", 2  },	// -1.234.567,89 DM
	{ "GRD", "Greece"        , 340.750	, "d"   , TRUE , ".", ",", 2  },	// ??
	{ "IEP", "Ireland"       , 0.787564 , "£"   , TRUE , ".", ",", 2  },	// -£1,234,567.89
	{ "ITL", "Italy"         , 1936.27	, "L"   , TRUE , "" , ".", 0  },	// L -1.234.567
	{ "LUF", "Luxembourg"    , 40.3399	, "LU"  , TRUE , ",", ".", 2  },	// LU 1.234.567,89 -
	{ "NLG", "Netherlands"   , 2.20371	, "F"   , TRUE , ",", ".", 2  },	// F 1.234.567,89-
	{ "PTE", "Portugal"      , 200.482	, "Esc.", FALSE, "$", ".", 2  },	// -1.234.567$89 Esc.
	{ "ESP", "Spain"         , 166.386	, "Pts" , TRUE , "" , ".", 0  },	// -Pts 1.234.567
/* 2007 */
	{ "SIT", "Slovenia"      , 239.640	, "tol" , TRUE , ",", ".", 2  },	//
/* 2008 */
	{ "CYP", "Cyprus"        , 0.585274 , ""    , TRUE , ",", "" , 2  },	//
	{ "MTL", "Malta"         , 0.429300 , ""    , TRUE , ",", "" , 2  },	//
/* 2009 */
	{ "SKK", "Slovaquia"     , 30.12600 , "Sk"  , FALSE, ",", " ", 2  },	//
/* 2011 */
	{ "EEK", "Estonia"       , 15.6466  , "kr"  , FALSE, ",", " ", 2  },	//
/* 2014 */
	{ "LVL", "Latvia"        , 0.702804 , "lat.", FALSE, ",", "" , 2  },	// 2014 target for euro earliest
	
/* future */
	{ "CZK", "Czech republic", 28.36	, "Kč"  , FALSE, ",", " ", 2  },	// non-fixé - 2015 earliest
	{ "HUF", "Hungary"       , 261.51	, "Ft"  , TRUE , ",", " ", 2  },	// non-fixé - No current target for euro
	{ "LTL", "Lithuania"     , 3.45280	, "Lt." , TRUE , ",", "" , 2  },	// 2014 target for euro earliest
	{ "PLN", "Poland"        , 0.25     , "zł"  , FALSE, ",", "" , 2  },	// non-fixé - No current target for euro
	{ "BGN", "Bulgaria"      , 1.95583	, "лв." , TRUE , ",", " ", 2  },	// non-fixé - 2014 target for euro
	{ "RON", "Romania"       , 3.5155	, "Leu" , FALSE, ",", ".", 2  },	// non-fixé - 2015 target for euro earliest
	{ "HRK", "Croatia"       , 1.0000   , "kn"  , FALSE, "" , ".", 0  },	// non-fixé - 2015 target for euro earliest
	
//	{ "   ", ""    , 1.00000	, ""   , ""  , FALSE, ",", "", 2  },
	
};


GtkWidget *pref_list_create(void);
GtkWidget *list_txn_colprefcreate(void);

static void list_txn_colpref_get(GtkTreeView *treeview, gboolean *columns);





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
typedef struct 
{
	gchar   *locale;
	gchar   *name;
	
} LangName;

static LangName languagenames[] =
{
// af ar ast be bg ca cs cy da de el en_AU en_CA en_GB es et eu fa fi fr ga gl he hr hu id is it 
//ja ka ko lt lv ms nb nds nl oc pl pt_BR pt pt_PT ro ru si sk sl sr sv tr uk vi zh_CN zh_TW
	
	{ "aa", "Afar" },
	{ "ab", "Abkhazian" },
	{ "ae", "Avestan" },
	{ "af", "Afrikaans" },
	{ "ak", "Akan" },
	{ "am", "Amharic" },
	{ "an", "Aragonese" },
	{ "ar", "Arabic" },
	{ "as", "Assamese" },
		{ "ast", "Asturian, Bable, Leonese, Asturleonese" },
	{ "av", "Avaric" },
	{ "ay", "Aymara" },
	{ "az", "Azerbaijani" },
	{ "ba", "Bashkir" },
	{ "be", "Belarusian" },
	{ "bg", "Bulgarian" },
	{ "bh", "Bihari" },
	{ "bi", "Bislama" },
	{ "bm", "Bambara" },
	{ "bn", "Bengali" },
	{ "bo", "Tibetan" },
	{ "br", "Breton" },
	{ "bs", "Bosnian" },
	{ "ca", "Catalan" },
	{ "ce", "Chechen" },
	{ "ch", "Chamorro" },
	{ "co", "Corsican" },
	{ "cr", "Cree" },
	{ "cs", "Czech" },
	{ "cu", "Old Church Slavonic" },
	{ "cv", "Chuvash" },
	{ "cy", "Welsh" },
	{ "da", "Danish" },
	{ "de", "German" },
	{ "dv", "Divehi" },
	{ "dz", "Dzongkha" },
	{ "ee", "Ewe" },
	{ "el", "Greek" },
	{ "en", "English" },
	{ "eo", "Esperanto" },
	{ "es", "Spanish" },
	{ "et", "Estonian" },
	{ "eu", "Basque" },
	{ "fa", "Persian" },
	{ "ff", "Fulah" },
	{ "fi", "Finnish" },
	{ "fj", "Fijian" },
	{ "fo", "Faroese" },
	{ "fr", "French" },
	{ "fy", "Western Frisian" },
	{ "ga", "Irish" },
	{ "gd", "Scottish Gaelic" },
	{ "gl", "Galician" },
	{ "gn", "Guarani" },
	{ "gu", "Gujarati" },
	{ "gv", "Manx" },
	{ "ha", "Hausa" },
	{ "he", "Hebrew" },
	{ "hi", "Hindi" },
	{ "ho", "Hiri Motu" },
	{ "hr", "Croatian" },
	{ "ht", "Haitian" },
	{ "hu", "Hungarian" },
	{ "hy", "Armenian" },
	{ "hz", "Herero" },
	{ "ia", "Interlingua" },
	{ "id", "Indonesian" },
	{ "ie", "Interlingue" },
	{ "ig", "Igbo" },
	{ "ii", "Sichuan Yi" },
	{ "ik", "Inupiaq" },
	{ "io", "Ido" },
	{ "is", "Icelandic" },
	{ "it", "Italian" },
	{ "iu", "Inuktitut" },
	{ "ja", "Japanese" },
	{ "jv", "Javanese" },
	{ "ka", "Georgian" },
	{ "kg", "Kongo" },
	{ "ki", "Kikuyu" },
	{ "kj", "Kwanyama" },
	{ "kk", "Kazakh" },
	{ "kl", "Kalaallisut" },
	{ "km", "Khmer" },
	{ "kn", "Kannada" },
	{ "ko", "Korean" },
	{ "kr", "Kanuri" },
	{ "ks", "Kashmiri" },
	{ "ku", "Kurdish" },
	{ "kv", "Komi" },
	{ "kw", "Cornish" },
	{ "ky", "Kirghiz" },
	{ "la", "Latin" },
	{ "lb", "Luxembourgish" },
	{ "lg", "Ganda" },
	{ "li", "Limburgish" },
	{ "ln", "Lingala" },
	{ "lo", "Lao" },
	{ "lt", "Lithuanian" },
	{ "lu", "Luba-Katanga" },
	{ "lv", "Latvian" },
	{ "mg", "Malagasy" },
	{ "mh", "Marshallese" },
	{ "mi", "Māori" },
	{ "mk", "Macedonian" },
	{ "ml", "Malayalam" },
	{ "mn", "Mongolian" },
	{ "mo", "Moldavian" },
	{ "mr", "Marathi" },
	{ "ms", "Malay" },
	{ "mt", "Maltese" },
	{ "my", "Burmese" },
	{ "na", "Nauru" },
	{ "nb", "Norwegian Bokmål" },
	{ "nd", "North Ndebele" },
		{ "nds", "Low German, Low Saxon" },
	{ "ne", "Nepali" },
	{ "ng", "Ndonga" },
	{ "nl", "Dutch" },
	{ "nn", "Norwegian Nynorsk" },
	{ "no", "Norwegian" },
	{ "nr", "South Ndebele" },
	{ "nv", "Navajo" },
	{ "ny", "Chichewa" },
	{ "oc", "Occitan" },
	{ "oj", "Ojibwa" },
	{ "om", "Oromo" },
	{ "or", "Oriya" },
	{ "os", "Ossetian" },
	{ "pa", "Panjabi" },
	{ "pi", "Pāli" },
	{ "pl", "Polish" },
	{ "ps", "Pashto" },
	{ "pt", "Portuguese" },
	{ "qu", "Quechua" },
	{ "rm", "Romansh" },
	{ "rn", "Kirundi" },
	{ "ro", "Romanian" },
	{ "ru", "Russian" },
	{ "rw", "Kinyarwanda" },
	{ "sa", "Sanskrit" },
	{ "sc", "Sardinian" },
	{ "sd", "Sindhi" },
	{ "se", "Northern Sami" },
	{ "sg", "Sango" },
	{ "si", "Sinhalese" },
	{ "sk", "Slovak" },
	{ "sl", "Slovene" },
	{ "sm", "Samoan" },
	{ "sn", "Shona" },
	{ "so", "Somali" },
	{ "sq", "Albanian" },
	{ "sr", "Serbian" },
	{ "ss", "Swati" },
	{ "st", "Sotho" },
	{ "su", "Sundanese" },
	{ "sv", "Swedish" },
	{ "sw", "Swahili" },
	{ "ta", "Tamil" },
	{ "te", "Telugu" },
	{ "tg", "Tajik" },
	{ "th", "Thai" },
	{ "ti", "Tigrinya" },
	{ "tk", "Turkmen" },
	{ "tl", "Tagalog" },
	{ "tn", "Tswana" },
	{ "to", "Tonga" },
	{ "tr", "Turkish" },
	{ "ts", "Tsonga" },
	{ "tt", "Tatar" },
	{ "tw", "Twi" },
	{ "ty", "Tahitian" },
	{ "ug", "Uighur" },
	{ "uk", "Ukrainian" },
	{ "ur", "Urdu" },
	{ "uz", "Uzbek" },
	{ "ve", "Venda" },
	{ "vi", "Viêt Namese" },
	{ "vo", "Volapük" },
	{ "wa", "Walloon" },
	{ "wo", "Wolof" },
	{ "xh", "Xhosa" },
	{ "yi", "Yiddish" },
	{ "yo", "Yoruba" },
	{ "za", "Zhuang" },
	{ "zh", "Chinese" },
	{ "zu", "Zulu" }

};


static gint
ui_language_combobox_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint retval = 0;
gchar *code1, *code2;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, 0, &code1, 1, &name1, -1);
    gtk_tree_model_get(model, b, 0, &code2, 1, &name2, -1);

	//keep system laguage on top
	if(code1 == NULL) name1 = NULL;
	if(code2 == NULL) name2 = NULL;
	
    retval = hb_string_utf8_compare(name1, name2);

    g_free(name2);
    g_free(name1);

  	return retval;
}


static gchar *languagename_get(const gchar *locale)
{
guint i;

	for (i = 0; i < G_N_ELEMENTS (languagenames); i++)
	{
		if( g_ascii_strncasecmp(locale, languagenames[i].locale, -1) == 0 )
			return languagenames[i].name;
	}

	return NULL;
}


static const gchar *
ui_language_combobox_get_name(const gchar *locale)
{
const gchar *lang;

	DB( g_print("[ui_language_combobox_get_name]\n") );

	// A locale directory name is typically of the form language[_territory]
	lang = languagename_get (locale);
	if (! lang)
	{
	const gchar *delimiter = strchr (locale, '_'); //  strip off the territory suffix

		if (delimiter)
		{
		gchar *copy = g_strndup (locale, delimiter - locale);
			lang = languagename_get (copy);
			g_free (copy);
		}

		if(! lang)
		{
			g_warning(" locale name not found '%s'", locale);
			lang = locale;
		}
		
	}

	return lang;
}


static void
ui_language_combobox_populate(GtkWidget *combobox)
{
GtkTreeModel *model;
GtkTreeIter  iter;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 
	                    0, NULL,
	                    1, _("System Language"),
	                    -1);

	GDir *dir = g_dir_open (homebank_app_get_locale_dir (), 0, NULL);
const gchar *dirname;

	if (! dir)
		return;

	while ((dirname = g_dir_read_name (dir)) != NULL)
	{
	gchar *filename = g_build_filename (homebank_app_get_locale_dir (),
		                      dirname,
		                      "LC_MESSAGES",
		                      GETTEXT_PACKAGE ".mo",
		                      NULL);
		//DB( g_print("- seek for '%s'\n", filename) );
		if (g_file_test (filename, G_FILE_TEST_EXISTS))
		{
		const gchar *lang;
		gchar *label;
			
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);

			lang = ui_language_combobox_get_name(dirname);
			label = g_strdup_printf ("%s [%s]", lang, dirname);

			gtk_list_store_set (GTK_LIST_STORE(model), &iter, 
					            0, dirname,
					            1, label,
					            -1);
			g_free(label);

		}
		g_free (filename);

	}
	g_dir_close (dir);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);


}

static gchar *ui_language_combobox_get_active_id(GtkComboBox *combobox)
{
GtkTreeModel *model;
GtkTreeIter  iter;
gchar *id = 0;

	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox), &iter) )
	{
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
		gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, 
				            0, &id,
				            -1);
	}

	return id;
}


static gint ui_language_combobox_set_active_id(GtkComboBox *combobox, gchar *id)
{
GtkTreeModel *model;
GtkTreeIter  iter;
gboolean valid;
gchar *tmp_id;
gint i;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			0, &tmp_id,
			-1);

		if( tmp_id != NULL)
		{
			if( g_ascii_strncasecmp(id, tmp_id, -1) == 0 )
			{
				gtk_combo_box_set_active_iter(combobox, &iter);
				return i;
			}
		}	
		
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		i++;
	}
	return i;
}



static GtkWidget *
ui_language_combobox_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *combobox;
GtkCellRenderer *renderer;

	store = gtk_list_store_new (2,
		G_TYPE_STRING,
		G_TYPE_STRING
		);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_language_combobox_compare_func, NULL, NULL);

	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 1, NULL);

	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	//gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_COMBO, -1);
	ui_language_combobox_populate(combobox);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	
	return combobox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/





/*
**
*/
static GtkWidget *make_euro_presets(GtkWidget *label)
{
GtkWidget *combobox;
guint i;

	DB( g_print("\n[ui-pref] make euro preset\n") );

	combobox = gtk_combo_box_text_new();
	for (i = 0; i < G_N_ELEMENTS (euro_params); i++)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), euro_params[i].name);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	return combobox;
}

/*
static defpref_currency_display(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct iso4217format *curfmt;

	DB( g_print("\n[ui-pref] display default currency\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	curfmt =  iso4217format_get(PREFS->curr_default);
	if(curfmt != NULL)
	{
	gchar *name = g_strdup_printf("%s (%s)", curfmt->curr_iso_code, iso4217_get_name(curfmt->curr_iso_code));
	
		gtk_label_set_label (data->LB_default, name);
		g_free(name);
	}

}


static void defpref_currency_change(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct iso4217format *curfmt;

	DB( g_print("\n[ui-pref] chnage default currency\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	curfmt = ui_cur_select_dialog_new(data->window);

	DB( g_print("(cur) return  %s\n", curfmt) );

	if( curfmt != NULL )
	{
		g_free(PREFS->curr_default);
		PREFS->curr_default = g_strdup(curfmt->culture);
		
		defpref_currency_display(widget, NULL);
	}

}
*/



static void defpref_pathselect(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint type = GPOINTER_TO_INT(user_data);
gchar **path;
gchar *title;
GtkWidget *entry;
gboolean r;

	DB( g_print("\n[ui-pref] path select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	switch( type )
	{
		case 1:
			path = &PREFS->path_hbfile;
			entry = data->ST_path_hbfile;
			title = _("Choose a default HomeBank files folder");
			break;
		case 2:
			path = &PREFS->path_import;
			entry = data->ST_path_import;
			title = _("Choose a default import folder");
			break;
		case 3:
			path = &PREFS->path_export;
			entry = data->ST_path_export;
			title = _("Choose a default export folder");
			break;
		default:
			return;
	}

	DB( g_print(" - hbfile %p %s at %p\n" , PREFS->path_hbfile, PREFS->path_hbfile, &PREFS->path_hbfile) );
	DB( g_print(" - import %p %s at %p\n" , PREFS->path_import, PREFS->path_import, &PREFS->path_import) );
	DB( g_print(" - export %p %s at %p\n" , PREFS->path_export, PREFS->path_export, &PREFS->path_export) );


	DB( g_print(" - before: %s %p\n" , *path, path) );

	r = ui_file_chooser_folder(GTK_WINDOW(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), title, path);


	DB( g_print(" - after: %s\n", *path) );

	if( r == TRUE )
		gtk_entry_set_text(GTK_ENTRY(entry), *path);


}


/*
** update the date sample label
*/
static void defpref_date_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gchar buffer[256];
const gchar *fmt;
GDate *date;

	DB( g_print("\n[ui-pref] date sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	fmt = gtk_entry_get_text(GTK_ENTRY(data->ST_datefmt));
	date = g_date_new_julian (GLOBALS->today);
	g_date_strftime (buffer, 256-1, fmt, date);
	g_date_free(date);

	gtk_label_set_text(GTK_LABEL(data->LB_date), buffer);

}



/*
** update the number sample label
*/
static void defpref_numberbase_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct CurrencyFmt cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar  buf[128], *ptr;

	DB( g_print("\n[ui-pref] number sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_symbol));
	cur.is_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_num_isprefix));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_num_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits));
	g_snprintf(cur.format, 8-1, "%%.%df", cur.frac_digits);

	ptr = cur.monfmt;
	if(cur.is_prefix)
	{
		ptr = g_stpcpy(ptr, cur.symbol);
		ptr = g_stpcpy(ptr, " ");
	}
	ptr = g_stpcpy(ptr, "%s");
	if(!cur.is_prefix)
	{
		ptr = g_stpcpy(ptr, " ");
		ptr = g_stpcpy(ptr, cur.symbol);
	}
	
	DB( g_print("fmt: %s\n", cur.format) );
	DB( g_print("monfmt: %s\n", cur.monfmt) );

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur.format, 12345.67);
	
	real_mystrfmon(buf, 127, formatd_buf, &cur);
	gtk_label_set_text(GTK_LABEL(data->LB_numberbase), buf);

}

/*
** update the number sample label
*/
static void defpref_numbereuro_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
struct CurrencyFmt cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar  buf[128], *ptr;

	DB( g_print("\n[ui-pref] number sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_symbol));
	cur.is_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));
	g_snprintf(cur.format, 8-1, "%%.%df", cur.frac_digits);
	
	ptr = cur.monfmt;
	if(cur.is_prefix)
	{
		ptr = g_stpcpy(ptr, cur.symbol);
		ptr = g_stpcpy(ptr, " ");
	}
	ptr = g_stpcpy(ptr, "%s");
	if(!cur.is_prefix)
	{
		ptr = g_stpcpy(ptr, " ");
		ptr = g_stpcpy(ptr, cur.symbol);
	}
	
	DB( g_print("fmt: %s\n", cur.format) );
	DB( g_print("monfmt: %s\n", cur.monfmt) );
	
	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur.format, 12345.67);
	
	real_mystrfmon(buf, 127, formatd_buf, &cur);
	gtk_label_set_text(GTK_LABEL(data->LB_numbereuro), buf);

}


/*
** enable/disable euro
*/
static void defpref_eurotoggle(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gboolean bool;

	DB( g_print("\n[ui-pref] euro toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));



	gtk_widget_set_sensitive(data->CY_euro_preset	, bool);
	gtk_widget_set_sensitive(data->ST_euro_country	, bool);
	gtk_widget_set_sensitive(data->NB_euro_value	, bool);

	gtk_widget_set_sensitive(data->ST_euro_symbol, bool);
	gtk_widget_set_sensitive(data->CM_euro_isprefix, bool);
	gtk_widget_set_sensitive(data->ST_euro_decimalchar, bool);
	gtk_widget_set_sensitive(data->ST_euro_groupingchar, bool);	
	gtk_widget_set_sensitive(data->NB_euro_fracdigits, bool);
	gtk_widget_set_sensitive(data->LB_numbereuro, bool);

}



/*
** set euro value widget from a country
*/
static void defpref_europreset(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint country;

	DB( g_print("\n[ui-pref] euro preset\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	country = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_euro_preset));

	gtk_label_set_text(GTK_LABEL(data->ST_euro_country), euro_params[country].name);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), euro_params[country].value);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits), euro_params[country].frac_digits);

	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_symbol)   , euro_params[country].symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix), euro_params[country].is_prefix);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_decimalchar) , euro_params[country].decimal_char);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_groupingchar), euro_params[country].grouping_char);

}

/*
** set preset colors for amount display
*/
static void defpref_colorpreset(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
GdkColor color;
gint preset;
gchar *expcol, *inccol, *wrncol;

	DB( g_print("\n[ui-pref] color preset\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	preset = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_colors));

	switch( preset)
	{
		case 1:		//light
			expcol = LIGHT_EXP_COLOR;
			inccol = LIGHT_INC_COLOR;
			wrncol = LIGHT_WARN_COLOR;
			break;

		case 2:		//medium
			expcol = MEDIUM_EXP_COLOR;
			inccol = MEDIUM_INC_COLOR;
			wrncol = MEDIUM_WARN_COLOR;
			break;

		case 3:	//dark
		default:
			expcol = DEFAULT_EXP_COLOR;
			inccol = DEFAULT_INC_COLOR;
			wrncol = DEFAULT_WARN_COLOR;
			break;
	}


	gdk_color_parse(expcol, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_exp_color), &color);

	gdk_color_parse(inccol, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_inc_color), &color);

	gdk_color_parse(wrncol, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_warn_color), &color);


}




/*
static void setGdkColor_from_RGB(GdkColor *color, guint32 value)
{
guint	tmp;

	tmp = value >> 16;
	color->red   = tmp | tmp<<8;

	tmp = value >> 8 & 0xFF;
	color->green = tmp | tmp<<8;

	tmp = value & 0xFF;
	color->blue  = tmp | tmp<<8;
}
*/


/*
** set :: fill in widgets from PREFS structure
*/
static void defpref_entry_set_text(GtkWidget *widget, gchar *text)
{
	DB( g_print(" set text to '%s'\n", text) );

	if( text != NULL )
	{
		gtk_entry_set_text(GTK_ENTRY(widget), text);
	}
}

static void defpref_set(struct defpref_data *data)
{
GdkColor color;

	DB( g_print("\n[ui-pref] set\n") );

	if(PREFS->language != NULL)
		ui_language_combobox_set_active_id(GTK_COMBO_BOX(data->CY_language), PREFS->language);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX(data->CY_language), 0);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_toolbar), PREFS->toolbar_style);
	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_image_size), PREFS->image_size);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_custom_colors), PREFS->custom_colors);

	gdk_color_parse(PREFS->color_exp, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_exp_color), &color);

	gdk_color_parse(PREFS->color_inc, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_inc_color), &color);

	gdk_color_parse(PREFS->color_warn, &color);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(data->CP_warn_color), &color);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_ruleshint), PREFS->rules_hint);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_fiscyearday), PREFS->fisc_year_day );
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_fiscyearmonth), PREFS->fisc_year_month - 1);
	
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_hbfile), PREFS->path_hbfile);
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_import), PREFS->path_import);
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_export), PREFS->path_export);
	//gtk_entry_set_text(GTK_ENTRY(data->ST_path_navigator), PREFS->path_navigator);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_load_last), PREFS->loadlast);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_append_scheduled), PREFS->appendscheduled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_show_splash), PREFS->showsplash);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_herit_date), PREFS->heritdate);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_hide_reconciled), PREFS->hidereconciled);

	/* display */
	gtk_entry_set_text(GTK_ENTRY(data->ST_datefmt), PREFS->date_format);

	defpref_entry_set_text(data->ST_num_symbol, PREFS->base_cur.symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_num_isprefix), PREFS->base_cur.is_prefix);
	defpref_entry_set_text(data->ST_num_decimalchar, PREFS->base_cur.decimal_char);
	defpref_entry_set_text(data->ST_num_groupingchar, PREFS->base_cur.grouping_char);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits), PREFS->base_cur.frac_digits);

	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_numnbdec), PREFS->num_nbdecimal);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_numseparator), PREFS->num_separator);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_imperial), PREFS->imperial_unit);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_unitismile), PREFS->vehicle_unit_ismile);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_unitisgal), PREFS->vehicle_unit_isgal);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_wal), PREFS->date_range_wal);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_txn), PREFS->date_range_txn);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_rep), PREFS->date_range_rep);
	
	/* euro */
	//defpref_currency_display(data->LB_default, NULL);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable), PREFS->euro_active);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_euro_preset), PREFS->euro_country);
	gchar *buf;
	buf = g_strdup_printf("%s", euro_params[PREFS->euro_country].name);
	gtk_label_set_markup(GTK_LABEL(data->ST_euro_country), buf);
	g_free(buf);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), PREFS->euro_value);

	defpref_entry_set_text(data->ST_euro_symbol, PREFS->minor_cur.symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix), PREFS->minor_cur.is_prefix);
	defpref_entry_set_text(data->ST_euro_decimalchar, PREFS->minor_cur.decimal_char);
	defpref_entry_set_text(data->ST_euro_groupingchar, PREFS->minor_cur.grouping_char);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits), PREFS->minor_cur.frac_digits);


	//gtk_entry_set_text(GTK_ENTRY(data->ST_euro_symbol), PREFS->euro_symbol);
	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_nbdec), PREFS->euro_nbdec);
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_thsep), PREFS->euro_thsep);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_byamount), PREFS->stat_byamount);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_showrate), PREFS->stat_showrate);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_stat_showdetail), PREFS->stat_showdetail);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_budg_showdetail), PREFS->budg_showdetail);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_color_scheme), PREFS->report_color_scheme);

	/* import */
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_dtex_datefmt), PREFS->dtex_datefmt);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_dtex_ofxmemo), PREFS->dtex_ofxmemo);



}


/*
** get :: fill PREFS structure from widgets
*/


static void defpref_entry_get_text(GtkWidget *widget, gchar **storage)
{
const gchar *text;


	DB( g_print("defpref_entry_get_text\n") );

	DB( g_print(" storage is '%p' at '%p'\n", *storage, storage) );

	/* free any previous string */
	if( *storage != NULL )
	{
		DB( g_print(" storage was not null, freeing\n") );

		g_free(*storage);

	}

	*storage = NULL;

	text = gtk_entry_get_text(GTK_ENTRY(widget));
	*storage = g_strdup(text);
	DB( g_print(" storing %s at %p\n", *storage, storage) );

	DB( g_print(" get text to '%s' '%s'\n", text, *storage) );
}


static void defpref_get(struct defpref_data *data)
{
GdkColor color;
const gchar *lang;

	DB( g_print("\n[ui-pref] get\n") );

	g_free(PREFS->language);
	PREFS->language = NULL;
	lang = ui_language_combobox_get_active_id(GTK_COMBO_BOX(data->CY_language));
	if(lang != NULL)
	{
		PREFS->language = g_strdup(lang);
	}
	
	PREFS->toolbar_style = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_toolbar));
	//PREFS->image_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_image_size));

	PREFS->custom_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_custom_colors));

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_exp_color), &color);
	g_free(PREFS->color_exp);
	PREFS->color_exp = gdk_color_to_string(&color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_inc_color), &color);
	g_free(PREFS->color_inc);
	PREFS->color_inc = gdk_color_to_string(&color);

	gtk_color_button_get_color(GTK_COLOR_BUTTON(data->CP_warn_color), &color);
	g_free(PREFS->color_warn);
	PREFS->color_warn = gdk_color_to_string(&color);

	PREFS->rules_hint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_ruleshint));

	PREFS->fisc_year_day = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_fiscyearday));
	PREFS->fisc_year_month = 1 + gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_fiscyearmonth));

	list_txn_colpref_get(GTK_TREE_VIEW(data->LV_opecolumns), PREFS->lst_ope_columns);

	g_free(PREFS->path_hbfile);
	PREFS->path_hbfile = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_hbfile)));

	defpref_entry_get_text(data->ST_path_import, &PREFS->path_import);

	defpref_entry_get_text(data->ST_path_export, &PREFS->path_export);

	PREFS->loadlast  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_load_last));
	PREFS->appendscheduled  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_append_scheduled));
	PREFS->showsplash  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_show_splash));
	PREFS->heritdate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_herit_date));
	PREFS->hidereconciled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_hide_reconciled));

	//g_free(PREFS->path_navigator);
	//PREFS->path_navigator = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_navigator)));

	g_free(PREFS->date_format);
	PREFS->date_format = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_datefmt)));

	defpref_entry_get_text(data->ST_num_symbol, &PREFS->base_cur.symbol);
	PREFS->base_cur.is_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_num_isprefix));
	defpref_entry_get_text(data->ST_num_decimalchar, &PREFS->base_cur.decimal_char);
	defpref_entry_get_text(data->ST_num_groupingchar, &PREFS->base_cur.grouping_char);
	PREFS->base_cur.frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_num_fracdigits));

	//PREFS->num_nbdecimal = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_numnbdec));
	//PREFS->num_separator = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_numseparator));
	//PREFS->imperial_unit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_imperial));
	PREFS->vehicle_unit_ismile = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_unitismile));
	PREFS->vehicle_unit_isgal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_unitisgal));

	PREFS->date_range_wal = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_wal));
	PREFS->date_range_txn = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_txn));
	PREFS->date_range_rep = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_rep));

	PREFS->euro_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));
	PREFS->euro_country = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_euro_preset));
	PREFS->euro_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_value));
	//strcpy(PREFS->euro_symbol, gtk_entry_get_text(GTK_ENTRY(data->ST_euro_symbol)));
	//PREFS->euro_nbdec = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_nbdec));
	//PREFS->euro_thsep = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_thsep));

	defpref_entry_get_text(data->ST_euro_symbol, &PREFS->minor_cur.symbol);
	PREFS->minor_cur.is_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix));
	
	g_free(PREFS->minor_cur.decimal_char);
	PREFS->minor_cur.decimal_char  = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_decimalchar)) );
	
	g_free(PREFS->minor_cur.grouping_char);
	PREFS->minor_cur.grouping_char = g_strdup( gtk_entry_get_text(GTK_ENTRY(data->ST_euro_groupingchar)) );
	PREFS->minor_cur.frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));

	PREFS->stat_byamount   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_byamount));
	PREFS->stat_showrate   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showrate));
	PREFS->stat_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showdetail));

	PREFS->budg_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budg_showdetail));

	PREFS->report_color_scheme = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_color_scheme));

	/* import */
	PREFS->dtex_datefmt = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_dtex_datefmt));
	PREFS->dtex_ofxmemo = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_dtex_ofxmemo));


	//PREFS->chart_legend = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_chartlegend));

}

/*
GtkWidget *defpref_page_charts (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_hbox_new(FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	widget = gtk_check_button_new_with_mnemonic (_("Show legend"));
	data->CM_chartlegend = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Bar width:"), 1.0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 8.0, 32.0);
	//data->NB_numnbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);



	return(container);
}
*/


static GtkWidget *defpref_page_import (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *hbox, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (5, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Date options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Date order:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_IMPORT_DATEORDER);
	data->CY_dtex_datefmt = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("OFX/QFX options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Memo field:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_IMPORT_OFXMEMO);
	data->CY_dtex_ofxmemo = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Files folder"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);


	row++;
	label = make_label(_("_Import:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	widget = make_string(label);
	data->ST_path_import = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("...");
	data->BT_path_import = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	row++;
	label = make_label(_("_Export:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	widget = make_string(label);
	data->ST_path_export = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("...");
	data->BT_path_export = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	return(container);
}




static GtkWidget *defpref_page_reports (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (5, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);


	row = 0;
	label = make_label(_("Initial filter"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Date _range:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_rep = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Charts options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Color Scheme:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, (gchar **)chart_colors);
	data->CY_color_scheme = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Statistics options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show by _amount"));
	data->CM_stat_byamount = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _rate column"));
	data->CM_stat_showrate = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_stat_showdetail = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Budget options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_budg_showdetail = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	return(container);
}


static GtkWidget *defpref_page_euro (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (7, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;

/*
	label = make_label(_("Default currency"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Code:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.0);
	data->LB_default = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);



	row++;
	widget = gtk_button_new_with_mnemonic (_("_Change"));
	data->BT_default = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
*/

	label = make_label(_("General"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Enable"));
	data->CM_euro_enable = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Fill from:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_euro_presets(label);
	data->CY_euro_preset = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Country:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0.0, 0.0);
	data->ST_euro_country = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Value:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_euro(label);
	data->NB_euro_value = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Numbers format"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_euro_symbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Is prefix"));
	data->CM_euro_isprefix = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Decimal char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_decimalchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Grouping char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_groupingchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_euro_fracdigits = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_numbereuro = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

/*
	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_euro_nbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Thousand separator"));
	data->CM_euro_thsep = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Symbol:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 8);
	data->ST_euro_symbol = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
*/

	return(container);
}


/*
GtkWidget *defpref_page_help (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Web navigator"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label("", 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), HB_BOX_SPACING, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = make_label(_("Web _navigator to use:"), 1.0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_path_navigator = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);




	return(container);
}
*/

static GtkWidget *defpref_page_display (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (7, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Date options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Date format:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_datefmt = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	gtk_widget_set_tooltip_text(widget,
	_("%a locale's abbreviated weekday name.\n"
"%A locale's full weekday name. \n"
"%b locale's abbreviated month name. \n"
"%B locale's full month name. \n"
"%c locale's appropriate date and time representation. \n"
"%C century number (the year divided by 100 and truncated to an integer) as a decimal number [00-99]. \n"
"%d day of the month as a decimal number [01,31]. \n"
"%D same as %m/%d/%y. \n"
"%e day of the month as a decimal number [1,31]; a single digit is preceded by a space. \n"
"%j day of the year as a decimal number [001,366]. \n"
"%m month as a decimal number [01,12]. \n"
"%p locale's appropriate date representation. \n"
"%y year without century as a decimal number [00,99]. \n"
"%Y year with century as a decimal number. \n")
);


	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_date = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Numbers options"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Symbol:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 3);
	data->ST_num_symbol = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Is prefix"));
	data->CM_num_isprefix = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Decimal char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label,1);
	data->ST_num_decimalchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Grouping char:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string_maxlength(label, 1);
	data->ST_num_groupingchar = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	#if MYDEBUG
		widget = make_numeric(label, 0.0, 15.0);
	#else
		widget = make_numeric(label, 0.0, 6.0);
	#endif
	data->NB_num_fracdigits = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(NULL, 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_label(NULL, 0, 0.5);
	data->LB_numberbase = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/* obsolete */

	/*
	row++;
	label = make_label(_("_Frac digits:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_numnbdec = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Thousand separator"));
	data->CM_numseparator = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	*/

	row++;
	label = make_label(_("Measurement units"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	/*row++;
	widget = gtk_check_button_new_with_mnemonic (_("Use _Imperial units"));
	data->CM_imperial = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);*/

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Use _miles for meter"));
	data->CM_unitismile = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Use _galons for fuel"));
	data->CM_unitisgal = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	return(container);
}


static GtkWidget *defpref_page_transactions (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *sw, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (3, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, TRUE, TRUE, 0);

	row = 0;
	label = make_label(_("Transaction window"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Date _range:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_txn = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Hide reconciled transactions"));
	data->CM_hide_reconciled = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Multiple add"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Keep the last date"));
	data->CM_herit_date = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Column list"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	widget = (GtkWidget *)list_txn_colprefcreate();
	data->LV_opecolumns = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);
	gtk_widget_set_tooltip_text(widget, _("Drag & drop to change the order"));
	gtk_table_attach (GTK_TABLE (table), sw, 1, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);


	return(container);
}



static GtkWidget *defpref_page_interface (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *hbox, *label, *widget;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (6, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Language"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Language:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_language_combobox_new(label);
	data->CY_language = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	
	row++;
	label = make_label(_("General"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Toolbar:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_TOOLBAR_STYLE);
	data->CY_toolbar = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/*
	row++;
	label = make_label(_("_Size:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	widget = make_numeric(label, 16.0, 48.0);
	data->NB_image_size = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	label = make_label(_("pixels"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	*/

	row++;
	label = make_label(_("Treeview"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show rules hint"));
	data->CM_ruleshint = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Amount colors"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Uses custom colors"));
	data->CM_custom_colors = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Preset:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_TANGO_COLORS);
	data->CY_colors = widget;
	//gtk_table_attach_defaults (GTK_TABLE (table), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("_Expense:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 3, row, row+1);
	
	widget = gtk_color_button_new ();
	data->CP_exp_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	
	label = make_label(_("_Income:"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	widget = gtk_color_button_new ();
	data->CP_inc_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	label = make_label(_("_Warning:"), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	widget = gtk_color_button_new ();
	data->CP_warn_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	return(container);
}

static GtkWidget *defpref_page_general (struct defpref_data *data)
{
GtkWidget *container;
GtkWidget *table, *label, *widget, *hbox;
gint row;

	container = gtk_vbox_new(FALSE, 0);

	table = gtk_table_new (4, 3, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	gtk_box_pack_start (GTK_BOX (container), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Program start"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show splash screen"));
	data->CM_show_splash = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Load last opened file"));
	data->CM_load_last = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Post pending scheduled transactions"));
	data->CM_append_scheduled = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Fiscal year"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	//TRANSLATORS: (fiscal year) starts on
	label = make_label(_("Starts _on:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	widget = make_numeric (label, 1, 28);
	data->NB_fiscyearday = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	widget = make_cycle (NULL, CYA_MONTHS);
	data->CY_fiscyearmonth = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	row++;
	label = make_label(_("Main window reports"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Date _range:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_wal = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	row++;
	label = make_label(_("Files folder"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Default:"), 0, 0.5);
	//----------------------------------------- l, r, t, b
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	widget = make_string(label);
	data->ST_path_hbfile = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_button_new_with_label("...");
	data->BT_path_hbfile = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	return(container);
}

static void defpref_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
struct defpref_data *data;
GtkWidget *notebook;
GtkTreeView *treeview;
GtkTreeModel *model;
GtkTreeIter iter;

GValue        val = { 0, };
gint page;

	DB( g_print("\n[ui-pref] selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		notebook = GTK_WIDGET(user_data);
		treeview = gtk_tree_selection_get_tree_view(treeselection);
		data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");


		gtk_tree_model_get_value(model, &iter, LST_PREF_PAGE, &val);
		page = g_value_get_int (&val);
		DB( g_print(" - active is %d\n", page) );
		g_value_unset (&val);


		gtk_tree_model_get_value(model, &iter, LST_PREF_NAME, &val);
		gtk_label_set_text (GTK_LABEL (data->label), g_value_get_string (&val));
		g_value_unset (&val);

		gtk_tree_model_get_value(model, &iter, LST_PREF_ICON, &val);
		gtk_image_set_from_pixbuf (GTK_IMAGE (data->image),
                             g_value_get_object (&val));
		g_value_unset (&val);



		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), page);

		//defpref_change_page(GTK_WIDGET(gtk_tree_selection_get_tree_view(treeselection)), GINT_TO_POINTER(page));
	}

}


/*
** set the notebook active page from treeview
*/
/*void defpref_change_page(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint page = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;


	DB( g_print("\n[ui-pref] page\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_page));







	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_page), page);
}
*/


/*
** add an empty new account to our temp GList and treeview
*/
static void defpref_clear(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defpref_clear) (data=%p)\n", data) );

	result = ui_dialog_msg_question(
		GTK_WINDOW(data->window),
		_("Clear every preferences ?"),
		_("This will revert the preferences\nto its default values")
		);
	if( result == GTK_RESPONSE_YES )
	{
		homebank_pref_setdefault();
		defpref_set(data);
	}
	
}


// the window creation
GtkWidget *defpref_dialog_new (void)
{
struct defpref_data data;
GtkWidget *window, *content, *mainvbox;

GtkWidget *hbox, *vbox, *sw, *widget, *notebook, *page, *ebox, *image, *label;

      window = gtk_dialog_new_with_buttons (_("Preferences"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_REJECT,
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL);

	data.window = window;
	
	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);

	gtk_window_set_icon_name(GTK_WINDOW (window), GTK_STOCK_PREFERENCES);

	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainvbox = gtk_vbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(mainvbox), 8);

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//left part
	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	
	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	widget = pref_list_create();
	data.LV_page = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	// clear button
	data.BT_clear = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_clear, FALSE, TRUE, 0);

	
	//right part : notebook
	vbox = gtk_vbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);

	ebox = gtk_event_box_new();
	gtk_widget_set_state (ebox, GTK_STATE_SELECTED);
	gtk_box_pack_start (GTK_BOX (vbox), ebox, FALSE, TRUE, 0);
	gtk_widget_show (ebox);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (ebox), hbox);
  gtk_widget_show (hbox);

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

  gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  data.label = label;

  image = gtk_image_new ();
  gtk_box_pack_end (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);
	data.image = image;



	//notebook
	notebook = gtk_notebook_new();
	data.GR_page = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

/*
"general",
"interface",
"display",
"help",
"euro",
"report"
*/

	//general
	page = defpref_page_general(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//interface
	page = defpref_page_interface(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//columns
	page = defpref_page_transactions(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//display
	page = defpref_page_display(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//import
	page = defpref_page_import(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//report
	page = defpref_page_reports(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);

	//euro
	page = defpref_page_euro(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, NULL);


	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data.CM_euro_enable), PREFS->euro_active);

	//connect all our signals
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	g_signal_connect (G_OBJECT (data.BT_clear), "clicked", G_CALLBACK (defpref_clear), NULL);
	
	//path selector
	g_signal_connect (data.BT_path_hbfile, "pressed", G_CALLBACK (defpref_pathselect), GINT_TO_POINTER(1));
	g_signal_connect (data.BT_path_import, "pressed", G_CALLBACK (defpref_pathselect), GINT_TO_POINTER(2));
	g_signal_connect (data.BT_path_export, "pressed", G_CALLBACK (defpref_pathselect), GINT_TO_POINTER(3));

    g_signal_connect (data.CY_colors, "changed", G_CALLBACK (defpref_colorpreset), NULL);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_page)), "changed", G_CALLBACK (defpref_selection), notebook);

	g_signal_connect (data.CM_euro_enable, "toggled", G_CALLBACK (defpref_eurotoggle), NULL);

    g_signal_connect (data.CY_euro_preset, "changed", G_CALLBACK (defpref_europreset), NULL);

	//date
    g_signal_connect (data.ST_datefmt, "changed", G_CALLBACK (defpref_date_sample), NULL);

	//base number
    g_signal_connect (data.ST_num_symbol   , "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
	g_signal_connect (data.CM_num_isprefix, "toggled", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.ST_num_decimalchar, "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.ST_num_groupingchar, "changed", G_CALLBACK (defpref_numberbase_sample), NULL);
    g_signal_connect (data.NB_num_fracdigits, "value-changed", G_CALLBACK (defpref_numberbase_sample), NULL);

	//euro number
    g_signal_connect (data.ST_euro_symbol   , "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
	g_signal_connect (data.CM_euro_isprefix, "toggled", G_CALLBACK (defpref_numbereuro_sample), NULL);
	g_signal_connect (data.ST_euro_decimalchar , "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.ST_euro_groupingchar, "changed", G_CALLBACK (defpref_numbereuro_sample), NULL);
    g_signal_connect (data.NB_euro_fracdigits, "value-changed", G_CALLBACK (defpref_numbereuro_sample), NULL);

	//g_signal_connect (data.BT_default, "pressed", G_CALLBACK (defpref_currency_change), NULL);


	//setup, init and show window
	//defhbfile_setup(&data);
	//defhbfile_update(data.LV_arc, NULL);

	defpref_eurotoggle(window, NULL);

	defpref_set(&data);

	gtk_window_resize(GTK_WINDOW(window), 640, 256);


	//select first row
	GtkTreePath *path = gtk_tree_path_new_first ();

	gtk_tree_selection_select_path (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_page)), path);



	gtk_tree_path_free(path);

	gtk_widget_show_all (window);

	gint result;
	gchar *old_lang;


		//wait for the user
		result = gtk_dialog_run (GTK_DIALOG (window));

		switch( result )
		{
			case GTK_RESPONSE_ACCEPT:
				old_lang = g_strdup(PREFS->language);
				defpref_get(&data);
				homebank_pref_save();
				ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_BALANCE+UF_VISUAL));

				DB( g_print("old='%s' new='%s'\n", old_lang, PREFS->language) );
				
				if(g_ascii_strncasecmp(old_lang == NULL ? "" : old_lang, PREFS->language == NULL ? "" : PREFS->language, -1) != 0)
				{
					ui_dialog_msg_infoerror(GTK_WINDOW(window), GTK_MESSAGE_INFO,
						_("Info"),
						_("You will have to restart HomeBank\nfor the language change to take effect.")
					);
			
				}

				g_free(old_lang);
				break;
		}
	

	// cleanup and destroy
	//defhbfile_cleanup(&data, result);
	gtk_widget_destroy (window);

	return window;
}

// -------------------------------


GtkWidget *pref_list_create(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;
GtkTreeIter    iter;
GtkWidget *cellview;
gint i;

	/* create list store */
	store = gtk_list_store_new(
	  	LST_PREF_MAX,
		GDK_TYPE_PIXBUF,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_INT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (view), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

	/* column 1: icon */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "pixbuf", LST_PREF_SMALLPIXBUF, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", LST_PREF_NAME, NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	cellview = gtk_cell_view_new ();

	//populate our combobox model
	for(i=0;i<PREF_MAX;i++)
	{
	GdkPixbuf *small_pixbuf = NULL;

		gtk_list_store_append(store, &iter);

		/*
		if( pref_pixbuf[i] )
			small_pixbuf = gdk_pixbuf_scale_simple (pref_pixbuf[i], 24, 24, GDK_INTERP_BILINEAR);
		*/
		small_pixbuf = gtk_widget_render_icon (cellview, pref_pixname[i], GTK_ICON_SIZE_DND, NULL);

		gtk_list_store_set(store, &iter,
			LST_PREF_SMALLPIXBUF, small_pixbuf,
			LST_PREF_ICON, pref_pixbuf[i],
			LST_PREF_NAME, _(pref_name[i]),
			LST_PREF_PAGE, i,
			-1);
	}

	gtk_widget_destroy (cellview);

	return(view);
}



void free_pref_icons(void)
{
guint i;

	for(i=0;i<PREF_MAX;i++)
	{
		if(pref_pixbuf[i] != NULL)
			g_object_unref(pref_pixbuf[i]);
	}
}

void load_pref_icons(void)
{
//GError *error = NULL;
GtkWidget *cellview;
guint i;

	cellview = gtk_cell_view_new ();

	for(i=0;i<PREF_MAX;i++)
	{
		pref_pixbuf[i] = gtk_widget_render_icon (cellview, pref_pixname[i], GTK_ICON_SIZE_DIALOG, NULL);
	}

	gtk_widget_destroy (cellview);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


static gchar *list_txn_column_label[] = {
	"----", //datas
	"----", //status
	"----", //date
	N_("Info"    ),
	N_("Payee"   ),
	N_("Memo"    ),
	N_("Amount"  ),
	N_("Expense" ),
	N_("Income"  ),
	N_("Category"),
	N_("Tags"    ),
	N_("Balance" ),
	NULL
};


//static gint n_ope_list_columns = G_N_ELEMENTS (ope_list_columns);


static void
list_txn_colpref_toggled_cell_data_function (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
GtkTreeModel *model = (GtkTreeModel *)data;
GtkTreeIter  iter;
GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
gboolean fixed;

	/* get toggled iter */
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COLUMN_VISIBLE, &fixed, -1);

	/* do something with the value */
	fixed ^= 1;

	/* set new value */
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_VISIBLE, fixed, -1);

	/* clean up */
	gtk_tree_path_free (path);
}


static void list_txn_colpref_get(GtkTreeView *treeview, gint *columns)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean visible;
gint i, id;

	DB( g_print("[lst_txn-colpref] store column order \n") );

	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	i = 0;
	while (valid)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			COLUMN_VISIBLE, &visible,
			COLUMN_ID, &id,
			-1);

		DB( g_print(" - column %d: %d\n",id, visible) );
		// start at index 2 (status, date always displayed
		columns[i+2] = visible == TRUE ? id : -id;

		 /* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		i++;
	}



}


GtkWidget *list_txn_colprefcreate(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;
GtkTreeIter    iter;
gint i;

	DB( g_print("[lst_txn-colpref] create\n") );


	/* create list store */
	store = gtk_list_store_new(
	  	3,
		G_TYPE_BOOLEAN,
		G_TYPE_STRING,
		G_TYPE_UINT
		);

	/* populate */
	for(i=0 ; i < NUM_LST_DSPOPE-1; i++ )   //-1 cause account column avoid
	{
	gint id;
	gboolean visible;

		DB( g_print("eval %d, %s\n", i, list_txn_column_label[i]) );
		
		if(i <= LST_DSPOPE_DATE) // status, date always displayed
			continue;

		//[i-1] here because lst_ope_columns[] do not store LST_DSPOPE_DATAS
		id = ABS(PREFS->lst_ope_columns[i-1]);  
		if(id == 0) id = i;	 //if we pass here, new column or weird into pref file
		visible = (PREFS->lst_ope_columns[i-1] > 0) ? TRUE : FALSE;

		
		DB( g_print(" - pos=%2d, id=%2d - %d '%s'\n", i, id, visible, list_txn_column_label[id]) );

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
			COLUMN_VISIBLE, visible,
	  		COLUMN_NAME, list_txn_column_label[id],
	  		COLUMN_ID  , id,
	  		-1);
		
	}

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);



	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", COLUMN_VISIBLE,
							     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	g_signal_connect (renderer, "toggled",
			    G_CALLBACK (list_txn_colpref_toggled_cell_data_function), store);

	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Column"),
							     renderer,
							     "text", COLUMN_NAME,
							     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);


	return(view);
}

