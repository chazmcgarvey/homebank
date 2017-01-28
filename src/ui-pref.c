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

#include "ui-pref.h"
#include "dsp_mainwindow.h"
#include "gtk-chart-colors.h"

#include "ui-currency.h"


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
	LST_PREF_ICONNAME,
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


static gchar *pref_iconname[PREF_MAX] = {
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

static gchar *CYA_GRID_LINES[] = {
N_("None"),
N_("Horizontal"),
N_("Vertical"),
N_("Both"),
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
N_("Ignore"),
N_("Memo"),
N_("Payee"),
NULL
};

gchar *CYA_IMPORT_OFXMEMO[] = {
N_("Ignore"),
N_("Append to Info"),
N_("Append to Memo"),
N_("Append to Payee"),
NULL
};


extern gchar *CYA_CHART_COLORSCHEME[];
extern gchar *CYA_MONTHS[];

typedef struct
{
	gshort		id;
	gchar		*iso;
	gchar		*name;
	gdouble		value;
	//gchar		*prefix_symbol;		/* max symbol is 3 digits in unicode */
	//gchar		*suffix_symbol;		/* but mostly is 1 digit */
	gchar		*symbol;
	gboolean	sym_prefix;
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
//                                , rate     , symb  , prfx , dec, grp, frac
// ---------------------------------------------------------------------
	{  0, ""   , "--------"       , 1.0		, ""    , FALSE, ",", ".", 2  },
	{  1, "ATS", "Austria"        , 13.7603	, "S"   , TRUE , ",", ".", 2  },	// -S 1.234.567,89
	{  2, "BEF", "Belgium"        , 40.3399	, "BF"  , TRUE , ",", ".", 2  },	// BF 1.234.567,89 -
	{ 20, "BGN", "Bulgaria"       , 1.95583	, "лв." , TRUE , ",", " ", 2  },	// non-fixé - 2014 target for euro
	{ 24, "HRK", "Croatia"        , 1.0000   , "kn"  , FALSE, "" , ".", 0  },	// non-fixé - 2015 target for euro earliest
	{ 14, "CYP", "Cyprus"         , 0.585274 , "£"   , TRUE , ",", "" , 2  },	//
	{ 23, "CZK", "Czech Republic" , 28.36	   , "Kč"  , FALSE, ",", " ", 2  },	// non-fixé - 2015 earliest
	// Denmark
	{ 17, "EEK", "Estonia"        , 15.6466  , "kr"  , FALSE, ",", " ", 2  },	//
	{  3, "FIM", "Finland"        , 5.94573	, "mk"  , FALSE, ",", " ", 2  },	// -1 234 567,89 mk
	{  4, "FRF", "France"         , 6.55957	, "F"   , FALSE, ",", " ", 2  },	// -1 234 567,89 F
	{  5, "DEM", "Germany"        , 1.95583	, "DM"  , FALSE, ",", ".", 2  },	// -1.234.567,89 DM
	{  6, "GRD", "Greece"         , 340.750	, "d"   , TRUE , ".", ",", 2  },	// ??
	{ 21, "HUF", "Hungary"        , 261.51	, "Ft"  , TRUE , ",", " ", 2  },	// non-fixé - No current target for euro
	{  7, "IEP", "Ireland"        , 0.787564  , "£"   , TRUE , ".", ",", 2  },	// -£1,234,567.89
	{  8, "ITL", "Italy"          , 1936.27	, "L"   , TRUE , "" , ".", 0  },	// L -1.234.567
	{ 18, "LVL", "Latvia"         , 0.702804 , "lat.", FALSE, ",", "" , 2  },	// jan. 2014
	{ 19, "LTL", "Lithuania"      , 3.45280	, "Lt"  , FALSE, ",", "" , 2  },	// jan. 2015
	{  9, "LUF", "Luxembourg"     , 40.3399	, "LU"  , TRUE , ",", ".", 2  },	// LU 1.234.567,89 -
	{ 15, "MTL", "Malta"          , 0.429300 , "Lm"  , TRUE , ",", "" , 2  },	//
	{ 10, "NLG", "Netherlands"    , 2.20371	, "F"   , TRUE , ",", ".", 2  },	// F 1.234.567,89-
	{ 25, "PLN", "Poland"         , 0.25     , "zł"  , FALSE, ",", "" , 2  },	// non-fixé - No current target for euro
	{ 11, "PTE", "Portugal"       , 200.482	, "Esc.", FALSE, "$", ".", 2  },	// -1.234.567$89 Esc.
	{ 22, "RON", "Romania"        , 3.5155	, "Leu" , FALSE, ",", ".", 2  },	// non-fixé - 2015 target for euro earliest
	{ 16, "SKK", "Slovak Republic", 30.12600 , "Sk"  , FALSE, ",", " ", 2  },	//
	{ 13, "SIT", "Slovenia"       , 239.640	, "tol" , TRUE , ",", ".", 2  },	//
	{ 12, "ESP", "Spain"          , 166.386	, "Pts" , TRUE , "" , ".", 0  },	// -Pts 1.234.567
	//Sweden
	//United Kingdom
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

	gtk_combo_box_set_id_column( GTK_COMBO_BOX(combobox), 0);
		
	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	ui_language_combobox_populate(combobox);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
	
	return combobox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

static gint ui_euro_combobox_id_to_active(gint id)
{
guint i, retval;

	DB( g_print("\n[ui-pref] ui_euro_combobox_id_to_active\n") );

	retval = 0;
	for (i = 0; i < G_N_ELEMENTS (euro_params); i++)
	{
		if( euro_params[i].id == id )
		{
			retval = i;
			DB( g_print("- id (country)=%d => %d - %s\n", id, i, euro_params[i].name) );
			break;
		}
	}

	return retval;
}



static gint ui_euro_combobox_active_to_id(gint active)
{
gint id;

	DB( g_print("\n[ui-pref] ui_euro_combobox_active_to_id\n") );

	DB( g_print("- to %d\n", active) );

	id = 0;
	if( active < (gint)G_N_ELEMENTS (euro_params) )
	{
		id = euro_params[active].id;
		DB( g_print("- id (country)=%d '%s'\n", id, euro_params[active].name) );
	}
	return id;
}


static GtkWidget *ui_euro_combobox_new(GtkWidget *label)
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
static void defpref_numbereuro_sample(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
Currency cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar buf[128];

	DB( g_print("\n[ui-pref] number sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_symbol));
	cur.sym_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_euro_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));

	da_cur_initformat (&cur);
	
	DB( g_print("fmt: %s\n", cur.format) );

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur.format, HB_NUMBER_SAMPLE);
	hb_str_formatd(buf, 127, formatd_buf, &cur, TRUE);

	gtk_label_set_text(GTK_LABEL(data->LB_numbereuro), buf);

}


/*
** enable/disable euro
*/
static void defpref_eurotoggle(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gboolean sensitive;

	DB( g_print("\n[ui-pref] euro toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));

	gtk_widget_set_sensitive(data->GRP_currency	 , sensitive);
	gtk_widget_set_sensitive(data->GRP_rate 	 , sensitive);
	gtk_widget_set_sensitive(data->GRP_format	 , sensitive);
}


/*
** set euro value widget from a country
*/
static void defpref_eurosetcurrency(GtkWidget *widget, gint country)
{
struct defpref_data *data;
EuroParams *euro;
gchar *buf;
gint active;
	
	DB( g_print("\n[ui-pref] eurosetcurrency\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	
	active = ui_euro_combobox_id_to_active(country);
	euro = &euro_params[active];
	buf = g_strdup_printf("%s - %s", euro->iso, euro->name);
	gtk_label_set_markup(GTK_LABEL(data->ST_euro_country), buf);
	g_free(buf);
}


/*
** set euro value widget from a country
*/
static void defpref_europreset(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gint active;

	DB( g_print("\n[ui-pref] euro preset\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	active = gtk_combo_box_get_active (GTK_COMBO_BOX(data->CY_euro_preset));
	data->country = ui_euro_combobox_active_to_id (active);;

	defpref_eurosetcurrency(widget, data->country);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), euro_params[active].value);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits), euro_params[active].frac_digits);

	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_symbol)   , euro_params[active].symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix), euro_params[active].sym_prefix);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_decimalchar) , euro_params[active].decimal_char);
	gtk_entry_set_text(GTK_ENTRY(data->ST_euro_groupingchar), euro_params[active].grouping_char);

}


static void defpref_colortoggle(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
gboolean sensitive;

	DB( g_print("\n[ui-pref] color toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_custom_colors));

	gtk_widget_set_sensitive(data->GR_colors	 , sensitive);

}


/*
** set preset colors for amount display
*/
static void defpref_colorpreset(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;
GdkRGBA rgba;
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


	gdk_rgba_parse(&rgba, expcol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_exp_color), &rgba);

	gdk_rgba_parse(&rgba, inccol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_inc_color), &rgba);

	gdk_rgba_parse(&rgba, wrncol);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_warn_color), &rgba);


}


static void defpref_color_scheme_changed(GtkWidget *widget, gpointer user_data)
{
struct defpref_data *data;

	DB( g_print("\n[ui-pref] color scheme chnaged\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	gtk_widget_queue_draw (data->DA_colors);
	
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

static void defpref_set(struct defpref_data *data)
{
GdkRGBA rgba;

	DB( g_print("\n[ui-pref] set\n") );

	if(PREFS->language != NULL)
		gtk_combo_box_set_active_id(GTK_COMBO_BOX(data->CY_language), PREFS->language);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX(data->CY_language), 0);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_toolbar), PREFS->toolbar_style);
	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_image_size), PREFS->image_size);



	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_custom_colors), PREFS->custom_colors);

	gdk_rgba_parse(&rgba, PREFS->color_exp);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_exp_color), &rgba);

	gdk_rgba_parse(&rgba, PREFS->color_inc);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_inc_color), &rgba);

	gdk_rgba_parse(&rgba, PREFS->color_warn);
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(data->CP_warn_color), &rgba);

	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_ruleshint), PREFS->rules_hint);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_gridlines), PREFS->grid_lines);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_fiscyearday), PREFS->fisc_year_day );
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_fiscyearmonth), PREFS->fisc_year_month - 1);
	
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_hbfile), PREFS->path_hbfile);
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_import), PREFS->path_import);
	gtk_entry_set_text(GTK_ENTRY(data->ST_path_export), PREFS->path_export);
	//gtk_entry_set_text(GTK_ENTRY(data->ST_path_navigator), PREFS->path_navigator);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_load_last), PREFS->loadlast);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_append_scheduled), PREFS->appendscheduled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_do_update_currency), PREFS->do_update_currency);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_show_splash), PREFS->showsplash);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_herit_date), PREFS->heritdate);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_hide_reconciled), PREFS->hidereconciled);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_show_remind), PREFS->showremind);

	/* display */
	gtk_entry_set_text(GTK_ENTRY(data->ST_datefmt), PREFS->date_format);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_unitismile), PREFS->vehicle_unit_ismile);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_unitisgal), PREFS->vehicle_unit_isgal);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_wal), PREFS->date_range_wal);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_txn), PREFS->date_range_txn);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_datefuture_nbdays), PREFS->date_future_nbdays);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_daterange_rep), PREFS->date_range_rep);

	/* euro */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable), PREFS->euro_active);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_euro_preset), PREFS->euro_country);
	data->country = PREFS->euro_country;
	defpref_eurosetcurrency(data->window, data->country);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_euro_value), PREFS->euro_value);

	ui_gtk_entry_set_text(data->ST_euro_symbol, PREFS->minor_cur.symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix), PREFS->minor_cur.sym_prefix);
	ui_gtk_entry_set_text(data->ST_euro_decimalchar, PREFS->minor_cur.decimal_char);
	ui_gtk_entry_set_text(data->ST_euro_groupingchar, PREFS->minor_cur.grouping_char);
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
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_dtex_ofxname), PREFS->dtex_ofxname);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_dtex_ofxmemo), PREFS->dtex_ofxmemo);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_dtex_qifmemo), PREFS->dtex_qifmemo);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_dtex_qifswap), PREFS->dtex_qifswap);


}


/*
** get :: fill PREFS structure from widgets
*/

#define RGBA_TO_INT(x) (int)(x*255)

static gchar *gdk_rgba_to_hex(GdkRGBA *rgba)
{
	return g_strdup_printf("#%02x%02x%02x", RGBA_TO_INT(rgba->red), RGBA_TO_INT(rgba->green), RGBA_TO_INT(rgba->blue));
}


static void defpref_get(struct defpref_data *data)
{
GdkRGBA rgba;
const gchar *lang;

	DB( g_print("\n[ui-pref] get\n") );

	g_free(PREFS->language);
	PREFS->language = NULL;
	lang = gtk_combo_box_get_active_id(GTK_COMBO_BOX(data->CY_language));
	if(lang != NULL)
	{
		PREFS->language = g_strdup(lang);
	}
	
	PREFS->toolbar_style = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_toolbar));
	//PREFS->image_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_image_size));

	PREFS->custom_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_custom_colors));

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(data->CP_exp_color), &rgba);
	g_free(PREFS->color_exp);
	PREFS->color_exp = gdk_rgba_to_hex(&rgba);

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(data->CP_inc_color), &rgba);
	g_free(PREFS->color_inc);
	PREFS->color_inc = gdk_rgba_to_hex(&rgba);

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(data->CP_warn_color), &rgba);
	g_free(PREFS->color_warn);
	PREFS->color_warn = gdk_rgba_to_hex(&rgba);

	//PREFS->rules_hint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_ruleshint));
	PREFS->grid_lines = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_gridlines));

	PREFS->fisc_year_day = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_fiscyearday));
	PREFS->fisc_year_month = 1 + gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_fiscyearmonth));

	list_txn_colpref_get(GTK_TREE_VIEW(data->LV_opecolumns), PREFS->lst_ope_columns);

	g_free(PREFS->path_hbfile);
	PREFS->path_hbfile = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_hbfile)));

	ui_gtk_entry_replace_text(data->ST_path_import, &PREFS->path_import);

	ui_gtk_entry_replace_text(data->ST_path_export, &PREFS->path_export);

	PREFS->loadlast  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_load_last));
	PREFS->appendscheduled  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_append_scheduled));
	PREFS->do_update_currency  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_do_update_currency));
		
	PREFS->showsplash  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_show_splash));
	PREFS->heritdate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_herit_date));
	PREFS->hidereconciled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_hide_reconciled));
	PREFS->showremind = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_show_remind));

	//g_free(PREFS->path_navigator);
	//PREFS->path_navigator = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_path_navigator)));

	g_free(PREFS->date_format);
	PREFS->date_format = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_datefmt)));

	PREFS->vehicle_unit_ismile = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_unitismile));
	PREFS->vehicle_unit_isgal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_unitisgal));

	PREFS->date_range_wal = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_wal));
	PREFS->date_range_txn = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_txn));
	PREFS->date_future_nbdays  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_datefuture_nbdays));
	PREFS->date_range_rep = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_daterange_rep));

	PREFS->euro_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_enable));

	PREFS->euro_country = data->country;
	PREFS->euro_value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_value));
	//strcpy(PREFS->euro_symbol, gtk_entry_get_text(GTK_ENTRY(data->ST_euro_symbol)));
	//PREFS->euro_nbdec = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_nbdec));
	//PREFS->euro_thsep = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_thsep));

	ui_gtk_entry_replace_text(data->ST_euro_symbol, &PREFS->minor_cur.symbol);
	PREFS->minor_cur.sym_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_euro_isprefix));
	ui_gtk_entry_replace_text(data->ST_euro_decimalchar, &PREFS->minor_cur.decimal_char);
	ui_gtk_entry_replace_text(data->ST_euro_groupingchar, &PREFS->minor_cur.grouping_char);
	PREFS->minor_cur.frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_euro_fracdigits));

	PREFS->stat_byamount   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_byamount));
	PREFS->stat_showrate   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showrate));
	PREFS->stat_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_stat_showdetail));

	PREFS->budg_showdetail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_budg_showdetail));

	PREFS->report_color_scheme = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_color_scheme));

	/* import */
	PREFS->dtex_datefmt = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_dtex_datefmt));
	PREFS->dtex_ofxname = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_dtex_ofxname));
	PREFS->dtex_ofxmemo = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_dtex_ofxmemo));
	PREFS->dtex_qifmemo = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_dtex_qifmemo));
	PREFS->dtex_qifswap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_dtex_qifswap));

	//PREFS->chart_legend = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_chartlegend));

}


static GtkWidget *defpref_page_import (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *hbox, *label, *widget;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Date options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Date options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("Date order:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_IMPORT_DATEORDER);
	data->CY_dtex_datefmt = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);


	// group :: OFX/QFX options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("OFX/QFX options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("_Name field:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_IMPORT_OFXNAME);
	data->CY_dtex_ofxname = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Memo field:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_IMPORT_OFXMEMO);
	data->CY_dtex_ofxmemo = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: QIF options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("QIF options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("Memos:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = gtk_check_button_new_with_mnemonic (_("_Import"));
	data->CM_dtex_qifmemo = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);
	widget = gtk_check_button_new_with_mnemonic (_("_Swap with payees"));
	data->CM_dtex_qifswap = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 3, row, 1, 1);


	// group :: Files folder
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Files folder"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("_Import:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_hexpand (hbox, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 2, row, 1, 1);

	widget = make_string(label);
	data->ST_path_import = widget;
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(widget)), GTK_STYLE_CLASS_LINKED);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	//widget = gtk_button_new_with_label("...");
	widget = gtk_button_new_from_icon_name(ICONNAME_FOLDER, GTK_ICON_SIZE_BUTTON);
	data->BT_path_import = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	row++;
	label = make_label_widget(_("_Export:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_hexpand (hbox, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 2, row, 1, 1);

	widget = make_string(label);
	data->ST_path_export = widget;
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(widget)), GTK_STYLE_CLASS_LINKED);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	//widget = gtk_button_new_with_label("...");
	widget = gtk_button_new_from_icon_name(ICONNAME_FOLDER, GTK_ICON_SIZE_BUTTON);
	data->BT_path_export = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	return content_grid;
}

#define cube_dim 16

static gboolean
draw_callback (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   user_data)
{
struct defpref_data *data = user_data;
gint index;
GtkColorScheme scheme;
gint w, h;
gint i, x, y;

	index = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_color_scheme));

	colorscheme_init(&scheme, index);
	
	gtk_widget_get_size_request (widget, &w, &h);
	x = y = 0;
	for(i=0;i<scheme.nb_cols;i++)
	{
		cairo_user_set_rgbcol (cr, &scheme.colors[i]);
		cairo_rectangle(cr, x, y, cube_dim, cube_dim);
		cairo_fill(cr);
		x += 1 + cube_dim;
		if( i == 15 )
		{ x = 0; y = 1 + cube_dim; }
	}

	return TRUE;
}



static GtkWidget *defpref_page_reports (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *label, *widget;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Initial filter
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Initial filter"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("Date _range:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_rep = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group ::Charts options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Charts options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label_widget(_("Color scheme:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, chart_colors);
	data->CY_color_scheme = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	widget = gtk_drawing_area_new ();
	data->DA_colors = widget;
	gtk_widget_set_size_request (widget, (1+cube_dim)*16, (1+cube_dim)*2);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	g_signal_connect (data->DA_colors, "draw", G_CALLBACK (draw_callback), data);

	// group :: Statistics options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Statistics options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Show by _amount"));
	data->CM_stat_byamount = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _rate column"));
	data->CM_stat_showrate = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_stat_showdetail = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	// group :: Budget options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Budget options"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Show _details"));
	data->CM_budg_showdetail = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	return content_grid;
}


static GtkWidget *defpref_page_euro (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *label, *widget, *expander;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: General
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("General"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row=1;
	widget = gtk_check_button_new_with_mnemonic (_("_Enable"));
	data->CM_euro_enable = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	// group :: Currency
    group_grid = gtk_grid_new ();
	data->GRP_currency = group_grid;
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Currency"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 4, 1);

	row=1;
	widget = make_label(NULL, 0.0, 0.5);
	data->ST_euro_country = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	//row++;
	label = make_label_widget(_("_Preset:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 2, row, 1, 1);
	widget = ui_euro_combobox_new (label);
	data->CY_euro_preset = widget;
	gtk_widget_set_margin_left (label, 2*SPACING_LARGE);
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 3, row, 1, 1);

	
	// group :: Exchange rate
    group_grid = gtk_grid_new ();
	data->GRP_rate = group_grid;
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(_("Exchange rate"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row=1;
	label = make_label_widget("1 EUR _=");
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_exchange_rate(label);
	data->NB_euro_value = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: Numbers format
    group_grid = gtk_grid_new ();
	data->GRP_format = group_grid;
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Format"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = make_label(NULL, 0, 0.0);
	data->LB_numbereuro = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	row++;
	expander = gtk_expander_new_with_mnemonic(_("_Customize"));
	gtk_grid_attach (GTK_GRID (group_grid), expander, 1, row, 1, 1);

    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (expander), group_grid);
	
	row = 0;
	label = make_label_widget(_("_Symbol:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 3);
	data->ST_euro_symbol = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Is pre_fix"));
	data->CM_euro_isprefix = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Decimal char:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_decimalchar = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Frac digits:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_numeric(label, 0.0, 6.0);
	data->NB_euro_fracdigits = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Grouping char:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 1);
	data->ST_euro_groupingchar = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	return content_grid;
}


static GtkWidget *defpref_page_display (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *label, *widget, *expander;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Date options
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Date"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = make_label(NULL, 0, 0.5);
	data->LB_date = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	expander = gtk_expander_new_with_mnemonic(_("_Customize"));
	gtk_grid_attach (GTK_GRID (group_grid), expander, 1, row, 1, 1);

    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (expander), group_grid);

	row = 1;
	label = make_label_widget(_("_Format:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string(label);
	data->ST_datefmt = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	widget = gtk_image_new_from_icon_name (ICONNAME_INFO, GTK_ICON_SIZE_BUTTON);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 3, row, 1, 1);

	
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
	widget = make_label(NULL, 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL(widget), "<small><a href=\"http://man7.org/linux/man-pages/man3/strftime.3.html\">online reference</a></small>");
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: Measurement units
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Measurement units"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Use _miles for meter"));
	data->CM_unitismile = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Use _gallon for fuel"));
	data->CM_unitisgal = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	return content_grid;
}


static GtkWidget *defpref_page_transactions (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *sw, *label, *widget;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Transaction window
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Transaction window"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("Date _range:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_txn = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Show:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_numeric(NULL, 0, 366);
	
	data->ST_datefuture_nbdays = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	//TRANSLATORS: there is a spinner on the left of this label, and so you have 0....x days in advance the current date
	label = make_label(_("days in advance the current date"), 0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 3, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Hide reconciled transactions"));
	data->CM_hide_reconciled = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Always show remind transactions"));
	data->CM_show_remind = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 2, 1);

	// group :: Multiple add
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Multiple add"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Keep the last date"));
	data->CM_herit_date = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	// group :: Column list
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Column list"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	widget = (GtkWidget *)list_txn_colprefcreate();
	data->LV_opecolumns = widget;
	gtk_widget_set_size_request(data->LV_opecolumns, HB_MINWIDTH_LIST, -1);
	gtk_container_add (GTK_CONTAINER (sw), widget);
	gtk_widget_set_vexpand (sw, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), sw, 1, row, 2, 1);
	gtk_widget_set_tooltip_text(widget, _("Drag & drop to change the order"));

	return content_grid;
}



static GtkWidget *defpref_page_interface (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *hbox, *label, *widget;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: General
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("General"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("_Language:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = ui_language_combobox_new(label);
	data->CY_language = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);
	
	row++;
	label = make_label_widget(_("_Toolbar:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_TOOLBAR_STYLE);
	data->CY_toolbar = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);


	row++;
	//widget = gtk_check_button_new_with_mnemonic (_("Enable rows in alternating colors"));
	//data->CM_ruleshint = widget;
	label = make_label_widget(_("_Grid line:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_GRID_LINES);
	data->CY_gridlines = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: Amount colors
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Amount colors"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Uses custom colors"));
	data->CM_custom_colors = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

    group_grid = gtk_grid_new ();
	data->GR_colors = group_grid;
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	row++;
	label = make_label_widget(_("_Preset:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_TANGO_COLORS);
	data->CY_colors = widget;
	//gtk_grid_attach (GTK_GRID (group_grid), data->CY_option[FILTER_DATE], 1, 2, row, row+1);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Expense:"));
	//----------------------------------------- l, r, t, b
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 2, row, 1, 1);
	
	widget = gtk_color_button_new ();
	data->CP_exp_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	
	label = make_label_widget(_("_Income:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	widget = gtk_color_button_new ();
	data->CP_inc_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	label = make_label_widget(_("_Warning:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	widget = gtk_color_button_new ();
	data->CP_warn_color = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	return content_grid;
}


static GtkWidget *defpref_page_general (struct defpref_data *data)
{
GtkWidget *content_grid, *group_grid, *hbox, *label, *widget;
gint crow, row;

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Program start
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Program start"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_check_button_new_with_mnemonic (_("Show splash screen"));
	data->CM_show_splash = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Load last opened file"));
	data->CM_load_last = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Post pending scheduled transactions"));
	data->CM_append_scheduled = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Update currencies online"));
	data->CM_do_update_currency = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	// group :: Fiscal year
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(_("Fiscal year"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	//TRANSLATORS: (fiscal year) starts on
	label = make_label_widget(_("Starts _on:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 2, row, 1, 1);
	widget = make_numeric (label, 1, 28);
	data->NB_fiscyearday = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	widget = make_cycle (NULL, CYA_MONTHS);
	data->CY_fiscyearmonth = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	// group :: Main window reports
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Main window reports"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("Date _range:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_daterange(label, FALSE);
	data->CY_daterange_wal = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: Files folder
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Files folder"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("_Default:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_hexpand (hbox, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), hbox, 2, row, 1, 1);

	widget = make_string(label);
	data->ST_path_hbfile = widget;
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(widget)), GTK_STYLE_CLASS_LINKED);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	//widget = gtk_button_new_with_label("...");
	widget = gtk_button_new_from_icon_name(ICONNAME_FOLDER, GTK_ICON_SIZE_BUTTON);
	data->BT_path_hbfile = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	return content_grid;
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

		gtk_tree_model_get_value(model, &iter, LST_PREF_ICONNAME, &val);
		//gtk_image_set_from_pixbuf (GTK_IMAGE (data->image), g_value_get_object (&val));
		gtk_image_set_from_icon_name(GTK_IMAGE (data->image), g_value_get_string (&val), GTK_ICON_SIZE_DIALOG);
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

	result = ui_dialog_msg_confirm_alert(
		GTK_WINDOW(data->window),
		_("Reset all preferences"),
		_("Do you really want to reset all\npreferences to default values?"),
	    _("Reset")
		);
	if( result == GTK_RESPONSE_OK )
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
				_("_Cancel"),
				GTK_RESPONSE_REJECT,
				_("_OK"),
				GTK_RESPONSE_ACCEPT,
				NULL);

	data.window = window;
	
	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);

	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_PREFERENCES);

	content = gtk_dialog_get_content_area(GTK_DIALOG (window));			// return a vbox
	mainvbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);

	gtk_container_set_border_width(GTK_CONTAINER(mainvbox), SPACING_MEDIUM);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//left part
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
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
	data.BT_clear = gtk_button_new_with_mnemonic(_("_Reset"));
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_clear, FALSE, TRUE, 0);

	
	//right part : notebook
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);

	ebox = gtk_event_box_new();
	gtk_widget_set_name(ebox, "hbebox");
	GtkStyleContext *context = gtk_widget_get_style_context (ebox);
	#if GTK_MINOR_VERSION <= 18
		gtk_style_context_add_class (context, GTK_STYLE_CLASS_LIST_ROW);
		gtk_widget_set_state_flags(ebox, GTK_STATE_FLAG_SELECTED, TRUE);
	#else
	GtkCssProvider *provider;
		provider = gtk_css_provider_new ();
		gtk_css_provider_load_from_data (provider, 
		"#hbebox { color: @theme_selected_fg_color; background-color: @theme_selected_bg_color; }"
		, -1, NULL);
		gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER(provider), G_MAXUINT);
	
	//	gtk_style_context_set_state(context, GTK_STATE_FLAG_SELECTED);
	#endif

	gtk_box_pack_start (GTK_BOX (vbox), ebox, FALSE, TRUE, 0);
	gtk_widget_show (ebox);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), SPACING_SMALL);
	gtk_container_add (GTK_CONTAINER (ebox), hbox);
	gtk_widget_show (hbox);

	label = gtk_label_new (NULL);
	gtk_widget_set_margin_left(label, SPACING_MEDIUM);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_XX_LARGE,
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

	g_signal_connect (data.CM_custom_colors, "toggled", G_CALLBACK (defpref_colortoggle), NULL);
    g_signal_connect (data.CY_colors, "changed", G_CALLBACK (defpref_colorpreset), NULL);


	
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_page)), "changed", G_CALLBACK (defpref_selection), notebook);

	g_signal_connect (data.CM_euro_enable, "toggled", G_CALLBACK (defpref_eurotoggle), NULL);

    g_signal_connect (data.CY_euro_preset, "changed", G_CALLBACK (defpref_europreset), NULL);

	//date
    g_signal_connect (data.ST_datefmt, "changed", G_CALLBACK (defpref_date_sample), NULL);

	//report
	g_signal_connect (data.CY_color_scheme, "changed", G_CALLBACK (defpref_color_scheme_changed), NULL);


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

	defpref_set(&data);

	defpref_colortoggle(window, NULL);
	defpref_eurotoggle(window, NULL);

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
gint i;

	/* create list store */
	store = gtk_list_store_new(
	  	LST_PREF_MAX,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_INT
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (view), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

	/* column 1: icon */
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_pixbuf_new ();
	g_object_set(renderer, "stock-size", GTK_ICON_SIZE_DND, NULL);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "icon-name", LST_PREF_ICONNAME, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", LST_PREF_NAME, NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	//populate our combobox model
	for(i=0;i<PREF_MAX;i++)
	{
		gtk_list_store_append(store, &iter);

		gtk_list_store_set(store, &iter,
		    LST_PREF_ICONNAME, pref_iconname[i],
			LST_PREF_NAME, _(pref_name[i]),
			LST_PREF_PAGE, i,
			-1);
	}

	return(view);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


extern gchar *list_txn_column_label[];


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
	  		COLUMN_NAME, _(list_txn_column_label[id]),
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

