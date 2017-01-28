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

#include "hb-preferences.h"
#include "hb-filter.h"
#include "gtk-chart-colors.h"

#ifdef G_OS_WIN32
#include <windows.h>
#endif

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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void homebank_pref_init_monetary(void)
{

	DB( g_print("\n[system] get currency code\n") );
	
#ifdef G_OS_UNIX
	struct lconv *lc = localeconv();

	g_stpcpy (PREFS->IntCurrSymbol, lc->int_curr_symbol);
	g_strstrip(PREFS->IntCurrSymbol);

	DB( g_print("- stored '%s' from linux system\n", PREFS->IntCurrSymbol) );
#else

	#ifdef G_OS_WIN32
    #define BUFFER_SIZE 512
    char buffer[BUFFER_SIZE];
    //LPWSTR wcBuffer = buffer;
    LPSTR wcBuffer = buffer;
    int iResult;

	//https://msdn.microsoft.com/en-us/library/windows/desktop/dd464799%28v=vs.85%29.aspx

	//LOCALE_SINTLSYMBOL
	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SINTLSYMBOL, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_SINTLSYMBOL='%s'\n", buffer) );
	 	g_stpcpy (PREFS->IntCurrSymbol, buffer);
		g_strstrip(PREFS->IntCurrSymbol);

		DB( g_print("- stored '%s' from windows system\n", PREFS->IntCurrSymbol) );
    }
	#else

 	g_stpcpy (PREFS->IntCurrSymbol, "USD");

	DB( g_print("- stored '%s' as default\n", PREFS->IntCurrSymbol) );
	
	#endif
	
#endif
	

}


/*
static void homebank_pref_init_monetary(void)
{
	DB( g_print("\n[preferences] monetary\n") );


#ifdef G_OS_UNIX

struct lconv *lc = localeconv();

	DB( g_print("\n[preferences] monetary unix\n") );

	DB( g_print("int_curr_symbol '%s'\n", lc->int_curr_symbol) );
	
	DB( g_print("mon_decimal_point is utf8: %d\n", g_utf8_validate(lc->mon_decimal_point, -1, NULL)) );
	DB( g_print("mon_decimal_point '%s'\n", lc->mon_decimal_point) );
	DB( g_print("mon_decimal_point %x %x %x %x\n", lc->mon_decimal_point[0], lc->mon_decimal_point[1], lc->mon_decimal_point[2], lc->mon_decimal_point[3]) );

	DB( g_print("mon_thousands_sep is utf8: %d\n", g_utf8_validate(lc->mon_thousands_sep, -1, NULL)) );
	DB( g_print("mon_thousands_sep '%s'\n", lc->mon_thousands_sep) );
	DB( g_print("mon_thousands_sep %x %x %x %x\n", lc->mon_thousands_sep[0], lc->mon_thousands_sep[1], lc->mon_thousands_sep[2], lc->mon_thousands_sep[3]) );


	DB( g_print("frac_digits '%d'\n", (gint)lc->frac_digits) );

	DB( g_print("currency_symbol '%s'\n", lc->currency_symbol) );

	DB( g_print("p_cs_precedes '%d'\n", lc->p_cs_precedes) );

	DB( g_print("n_cs_precedes '%d'\n", lc->n_cs_precedes) );

	//PREFS->base_cur.iso_code = g_strdup(g_strstrip(lc->int_curr_symbol));

	if( lc->p_cs_precedes || lc->n_cs_precedes )
	{
		PREFS->base_cur.symbol = g_strdup(lc->currency_symbol);
		PREFS->base_cur.sym_prefix = TRUE;
		DB( g_print("locale mon cs is a prefix\n") );
	}
	else
	{
		PREFS->base_cur.symbol = g_strdup(lc->currency_symbol);
		PREFS->base_cur.sym_prefix = FALSE;
	}

	PREFS->base_cur.decimal_char  = g_strdup(lc->mon_decimal_point);

	PREFS->base_cur.grouping_char = g_strdup(lc->mon_thousands_sep);

	//todo:fix
	//PREFS->base_cur.grouping_char = g_locale_to_utf8(lc->mon_thousands_sep, -1, NULL, NULL, NULL);
	//PREFS->base_cur.grouping_char = g_convert (lc->mon_thousands_sep, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

	DB( g_print(" -> grouping_char: '%s'\n", PREFS->base_cur.grouping_char) );

	PREFS->base_cur.frac_digits   = lc->frac_digits;

	//fix 378992/421228
	if( PREFS->base_cur.frac_digits > MAX_FRAC_DIGIT )
	{
		PREFS->base_cur.frac_digits = 2;
		g_free(PREFS->base_cur.decimal_char);
		PREFS->base_cur.decimal_char  = g_strdup(".");
	}

#else
	#ifdef G_OS_WIN32

    #define BUFFER_SIZE 512
    char buffer[BUFFER_SIZE];
    //LPWSTR wcBuffer = buffer;
    LPSTR wcBuffer = buffer;
    int iResult;
    gsize toto;

	//https://msdn.microsoft.com/en-us/library/windows/desktop/dd464799%28v=vs.85%29.aspx

	//LOCALE_SINTLSYMBOL

	
    //see g_locale_to_utf8 here
	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SCURRENCY, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_SCURRENCY='%s'\n", buffer) );
        PREFS->base_cur.symbol = g_locale_to_utf8(buffer, -1, NULL, &toto, NULL);
    }

	// LOCALE_ICURRENCY
	//PREFS->base_cur.sym_prefix =
	
	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_SDECIMAL='%s'\n", buffer) );
        PREFS->base_cur.decimal_char  = g_locale_to_utf8(buffer, -1, NULL, &toto, NULL);
    }

	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_STHOUSAND='%s'\n", buffer) );
        PREFS->base_cur.grouping_char = g_locale_to_utf8(buffer, -1, NULL, &toto, NULL);
    }

	// LOCALE_ICURRDIGITS
 	PREFS->base_cur.frac_digits   = 2;

	#else

	PREFS->base_cur.iso_code      = g_strdup("USD");
	PREFS->base_cur.symbol        = NULL; //g_strdup("");
	PREFS->base_cur.sym_prefix    = FALSE;
	PREFS->base_cur.decimal_char  = g_strdup(".");
	PREFS->base_cur.grouping_char = NULL; //g_strdup("");
	PREFS->base_cur.frac_digits   = 2;

	#endif
#endif

}
*/



static void homebank_pref_init_wingeometry(struct WinGeometry *wg, gint l, gint t, gint w, gint h)
{
	wg->l = l;
	wg->t = t;
	wg->w = w;
	wg->h = h;
	wg->s = 0;
}


//vehicle_unit_100
//vehicle_unit_distbyvol
//=> used for column title

static void _homebank_pref_init_measurement_units(void)
{
	// unit is kilometer
	if(!PREFS->vehicle_unit_ismile)
	{
		PREFS->vehicle_unit_dist = "%d km";
		PREFS->vehicle_unit_100  = "100 km";
	}
	// unit is miles
	else
	{
		PREFS->vehicle_unit_dist = "%d mi.";
		PREFS->vehicle_unit_100  = "100 mi.";
	}

	// unit is Liters
	if(!PREFS->vehicle_unit_isgal)
	{
		//TRANSLATORS: format a liter number with l/L as abbreviation
		PREFS->vehicle_unit_vol  = _("%.2f l");
		if(!PREFS->vehicle_unit_ismile)
			//TRANSLATORS: kilometer per liter
			PREFS->vehicle_unit_distbyvol  = _("km/l");
		else
			//TRANSLATORS: miles per liter
			PREFS->vehicle_unit_distbyvol  = _("mi./l");
	}
	// unit is gallon
	else
	{
		PREFS->vehicle_unit_vol  = "%.2f gal.";
		if(!PREFS->vehicle_unit_ismile)
			PREFS->vehicle_unit_distbyvol  = "km/gal.";
		else
			PREFS->vehicle_unit_distbyvol  = "mi./gal.";
	}

}


void homebank_pref_free(void)
{
	DB( g_print("\n[preferences] free\n") );


	g_free(PREFS->date_format);

	g_free(PREFS->color_exp);
	g_free(PREFS->color_inc);
	g_free(PREFS->color_warn);

	g_free(PREFS->path_hbfile);
	g_free(PREFS->path_import);
	g_free(PREFS->path_export);
	//g_free(PREFS->path_navigator);

	g_free(PREFS->language);


	g_free(PREFS->minor_cur.symbol);
	g_free(PREFS->minor_cur.decimal_char);
	g_free(PREFS->minor_cur.grouping_char);

	memset(PREFS, 0, sizeof(struct Preferences));
}


void homebank_pref_setdefault(void)
{
gint i;

	DB( g_print("\n[preferences] pref init\n") );

	homebank_pref_free();

	PREFS->language = NULL;

	PREFS->date_format = g_strdup(DEFAULT_FORMAT_DATE);

	PREFS->path_hbfile = g_strdup_printf("%s", g_get_home_dir ());
	PREFS->path_import = g_strdup_printf("%s", g_get_home_dir ());
	PREFS->path_export = g_strdup_printf("%s", g_get_home_dir ());
	//PREFS->path_navigator = g_strdup(DEFAULT_PATH_NAVIGATOR);

	PREFS->showsplash = TRUE;
	PREFS->loadlast = TRUE;
	PREFS->appendscheduled = FALSE;
	PREFS->do_update_currency = FALSE;

	PREFS->heritdate = FALSE;
	PREFS->hidereconciled = FALSE;
	PREFS->showremind = TRUE;

	PREFS->toolbar_style = 4;	//text beside icons
	PREFS->custom_colors = TRUE;
	PREFS->color_exp  = g_strdup(DEFAULT_EXP_COLOR);
	PREFS->color_inc  = g_strdup(DEFAULT_INC_COLOR);
	PREFS->color_warn = g_strdup(DEFAULT_WARN_COLOR);
	PREFS->grid_lines = GTK_TREE_VIEW_GRID_LINES_NONE;

	/* fiscal year */
	PREFS->fisc_year_day = 1;
	PREFS->fisc_year_month = 1;

	/* windows position/size */
	homebank_pref_init_wingeometry(&PREFS->wal_wg, 0, 0, 1024, 600);
	homebank_pref_init_wingeometry(&PREFS->acc_wg, 0, 0, 1024, 600);
	homebank_pref_init_wingeometry(&PREFS->sta_wg, 0, 0, 800, 494);
	homebank_pref_init_wingeometry(&PREFS->tme_wg, 0, 0, 800, 494);
	homebank_pref_init_wingeometry(&PREFS->ove_wg, 0, 0, 800, 494);
	homebank_pref_init_wingeometry(&PREFS->bud_wg, 0, 0, 800, 494);
	homebank_pref_init_wingeometry(&PREFS->cst_wg, 0, 0, 800, 494);

	homebank_pref_init_wingeometry(&PREFS->txn_wg, 0, 0, -1, -1);

	homebank_pref_init_monetary();

	PREFS->wal_toolbar = TRUE;
	PREFS->wal_spending = TRUE;
	PREFS->wal_upcoming = TRUE;
	PREFS->wal_vpaned = 600/2;
	PREFS->wal_hpaned = 1024/2;

	PREFS->pnl_acc_col_acc_width = -1;
	PREFS->pnl_acc_show_by = 0;
	PREFS->pnl_upc_col_pay_width = -1;
	PREFS->pnl_upc_col_mem_width = -1;


	i = 0;
	/* prior v4.5
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_STATUS;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_DATE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INFO;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_PAYEE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_WORDING;
	PREFS->lst_ope_columns[i++] = -LST_DSPOPE_AMOUNT;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_EXPENSE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INCOME;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_CATEGORY;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_TAGS;
	*/
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_STATUS;  //always displayed
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_DATE;	  //always displayed
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INFO;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_PAYEE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_CATEGORY;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_TAGS;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_CLR;
	PREFS->lst_ope_columns[i++] = -LST_DSPOPE_AMOUNT;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_EXPENSE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_INCOME;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_BALANCE;
	PREFS->lst_ope_columns[i++] = LST_DSPOPE_WORDING;

	PREFS->lst_ope_sort_id    = LST_DSPOPE_DATE;
	PREFS->lst_ope_sort_order = GTK_SORT_ASCENDING;

	for( i=0;i<NUM_LST_DSPOPE;i++)
		PREFS->lst_ope_col_size[i] = -1;

	//PREFS->base_cur.nbdecimal = 2;
	//PREFS->base_cur.separator = TRUE;

	PREFS->date_range_wal = FLT_RANGE_LASTMONTH;
	PREFS->date_range_txn = FLT_RANGE_LAST12MONTHS;
	PREFS->date_future_nbdays = 0;
	PREFS->date_range_rep = FLT_RANGE_THISYEAR;

	//import/export
	PREFS->dtex_nointro = TRUE;
	//PREFS->dtex_datefmt
	PREFS->dtex_ofxname = 1;
	PREFS->dtex_ofxmemo = 2;
	PREFS->dtex_qifmemo = TRUE;
	PREFS->dtex_qifswap = FALSE;

	//todo: add intelligence here
	PREFS->euro_active  = FALSE;

	PREFS->euro_country = 0;
	PREFS->euro_value   = 1.0;
	
	da_cur_initformat(&PREFS->minor_cur);
	
	//PREFS->euro_nbdec   = 2;
	//PREFS->euro_thsep   = TRUE;
	//PREFS->euro_symbol	= g_strdup("??");

	PREFS->stat_byamount   = FALSE;
	PREFS->stat_showdetail = FALSE;
	PREFS->stat_showrate   = FALSE;
	PREFS->budg_showdetail = FALSE;
	PREFS->report_color_scheme = CHART_COLMAP_HOMEBANK;

	//PREFS->chart_legend = FALSE;

	PREFS->vehicle_unit_ismile = FALSE;
	PREFS->vehicle_unit_isgal  = FALSE;

	_homebank_pref_init_measurement_units();

}


/*
** load preference from homedir/.homebank (HB_DATA_PATH)
*/
static void homebank_pref_get_wingeometry(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	struct WinGeometry *storage)
{
	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
	gint *wg;
	gsize length;

		wg = g_key_file_get_integer_list(key_file, group_name, key, &length, NULL);
		memcpy(storage, wg, 5*sizeof(gint));
		g_free(wg);
		// #606613 ensure left/top to be > 0
		if(storage->l < 0)
			storage->l = 0;

		if(storage->t < 0)
			storage->t = 0;
	}
}




static void homebank_pref_get_boolean(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gboolean *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_boolean(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_integer(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gint *storage)
{

	DB( g_print(" search %s in %s\n", key, group_name) );


	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_integer(key_file, group_name, key, NULL);

		DB( g_print(" store integer %d for %s at %x\n", *storage, key, *storage) );
	}
}

static void homebank_pref_get_guint32(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	guint32 *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = g_key_file_get_integer(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_short(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gshort *storage)
{

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		*storage = (gshort)g_key_file_get_integer(key_file, group_name, key, NULL);
	}
}

static void homebank_pref_get_string(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gchar **storage)
{
gchar *string;

	if( g_key_file_has_key(key_file, group_name, key, NULL) )
	{
		/* free any previous string */
		if( *storage != NULL )
		{
			//DB( g_print(" storage was not null, freeing\n") );

			g_free(*storage);
		}

		*storage = NULL;

		string = g_key_file_get_string(key_file, group_name, key, NULL);
		if( string != NULL )
		{
			*storage = g_strdup(string);

			//DB( g_print(" store '%s' for %s at %x\n", string, key, *storage) );
		}
	}

/*
	if (error)
    {
      g_warning ("error: %s\n", error->message);
      g_error_free(error);
      error = NULL;
    }
*/


}


static void homebank_pref_currfmt_convert(Currency *cur, gchar *prefix, gchar *suffix)
{

	if( (prefix != NULL) && (strlen(prefix) > 0) )
	{
		cur->symbol = g_strdup(prefix);
		cur->sym_prefix = TRUE;
	}
	else if( (suffix != NULL) )
	{
		cur->symbol = g_strdup(suffix);
		cur->sym_prefix = FALSE;
	}
}


gboolean homebank_pref_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
guint32 version;
GError *error = NULL;

	DB( g_print("\n[preferences] pref load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{
		filename = g_build_filename(homebank_app_get_config_dir(), "preferences", NULL );

		DB( g_print(" - filename: %s\n", filename) );


		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{

			group = "General";

				DB( g_print(" -> ** General\n") );

				//since 4.51 version is integer
				homebank_pref_get_guint32 (keyfile, group, "Version", &version);
				if(version == 0)	// old double number
				{
					gdouble v = g_key_file_get_double (keyfile, group, "Version", NULL);
					version = (guint32)(v * 10);
				}

				DB( g_print(" - version: %d\n", version) );

				homebank_pref_get_string(keyfile, group, "Language", &PREFS->language);

				homebank_pref_get_short(keyfile, group, "BarStyle" , &PREFS->toolbar_style);

				if(version <= 6 && PREFS->toolbar_style == 0)	// force system to text beside
				{
					PREFS->toolbar_style = 4;
				}

				if(version <= 2)	// retrieve old settings
				{
				guint32 color;

					homebank_pref_get_guint32(keyfile, group, "ColorExp" , &color);
					g_free(PREFS->color_exp);
					PREFS->color_exp = g_strdup_printf("#%06x", color);

					homebank_pref_get_guint32(keyfile, group, "ColorInc" , &color);
					g_free(PREFS->color_inc);
					PREFS->color_inc = g_strdup_printf("#%06x", color);

					homebank_pref_get_guint32(keyfile, group, "ColorWarn", &color);
					g_free(PREFS->color_warn);
					PREFS->color_warn = g_strdup_printf("#%06x", color);
				}
				else
				{
					homebank_pref_get_boolean(keyfile, group, "CustomColors", &PREFS->custom_colors);

					homebank_pref_get_string(keyfile, group, "ColorExp" , &PREFS->color_exp);
					homebank_pref_get_string(keyfile, group, "ColorInc" , &PREFS->color_inc);
					homebank_pref_get_string(keyfile, group, "ColorWarn", &PREFS->color_warn);

					if( version <= 500 )
					{
					gboolean rules_hint;
						homebank_pref_get_boolean(keyfile, group, "RulesHint", &rules_hint);
						if( rules_hint == TRUE )
							PREFS->grid_lines = GTK_TREE_VIEW_GRID_LINES_HORIZONTAL;

					}
					else
						homebank_pref_get_short(keyfile, group, "GridLines", &PREFS->grid_lines);
				}

				DB( g_print(" - color exp: %s\n", PREFS->color_exp) );
				DB( g_print(" - color inc: %s\n", PREFS->color_inc) );
				DB( g_print(" - color wrn: %s\n", PREFS->color_warn) );


				homebank_pref_get_string(keyfile, group, "WalletPath", &PREFS->path_hbfile);
				homebank_pref_get_string(keyfile, group, "ImportPath", &PREFS->path_import);
				homebank_pref_get_string(keyfile, group, "ExportPath", &PREFS->path_export);

				homebank_pref_get_boolean(keyfile, group, "ShowSplash", &PREFS->showsplash);
				homebank_pref_get_boolean(keyfile, group, "LoadLast", &PREFS->loadlast);
				homebank_pref_get_boolean(keyfile, group, "AppendScheduled", &PREFS->appendscheduled);
				homebank_pref_get_boolean(keyfile, group, "UpdateCurrency", &PREFS->do_update_currency);	

				homebank_pref_get_boolean(keyfile, group, "HeritDate", &PREFS->heritdate);
				homebank_pref_get_boolean(keyfile, group, "HideReconciled", &PREFS->hidereconciled);
				homebank_pref_get_boolean(keyfile, group, "ShowRemind", &PREFS->showremind);

				if( g_key_file_has_key(keyfile, group, "ColumnsOpe", NULL) )
				{
				gboolean *bsrc;
				gint *src, i, j;
				gsize length;

					if(version <= 2)	//retrieve old 0.1 or 0.2 visibility boolean
					{
						bsrc = g_key_file_get_boolean_list(keyfile, group, "ColumnsOpe", &length, &error);
						if( length == NUM_LST_DSPOPE-1 )
						{
							//and convert
							for(i=0; i<NUM_LST_DSPOPE-1 ; i++)
							{
								PREFS->lst_ope_columns[i] = (bsrc[i] == TRUE) ? i+1 : -(i+1);
							}
						}
						g_free(bsrc);
					}
					else
					{
						src = g_key_file_get_integer_list(keyfile, group, "ColumnsOpe", &length, &error);

						DB( g_print(" - length %d (max=%d)\n", length, NUM_LST_DSPOPE) );

						if( length == NUM_LST_DSPOPE-1 )
						{
							DB( g_print(" - copying column order from pref file\n") );
							memcpy(PREFS->lst_ope_columns, src, length*sizeof(gint));
						}
						else
						{
							if(version <= 7)
							{
								if( length == NUM_LST_DSPOPE-2 )	//1 less column before v4.5.1
								{
									DB( g_print(" - upgrade from v7\n") );
									DB( g_print(" - copying column order from pref file\n") );
									memcpy(PREFS->lst_ope_columns, src, length*sizeof(gint));
									//append balance column
									PREFS->lst_ope_columns[10] = LST_DSPOPE_BALANCE;
								}
							}

							if(version < 500)
							{
								if( length == NUM_LST_DSPOPE-2 )	//1 less column before v4.5.1
								{
									DB( g_print(" - upgrade prior v5.0\n") );
									DB( g_print(" - copying column order from pref file\n") );
									gboolean added = FALSE;
									for(i=0,j=0; i<NUM_LST_DSPOPE-1 ; i++)
									{
										if( added == FALSE &&
										    (ABS(src[i]) == LST_DSPOPE_AMOUNT ||
										    ABS(src[i]) == LST_DSPOPE_EXPENSE ||
										    ABS(src[i]) == LST_DSPOPE_INCOME) )
										{
											PREFS->lst_ope_columns[j++] = LST_DSPOPE_CLR;
											added = TRUE;
										}
										PREFS->lst_ope_columns[j++] = src[i];
									}
								}
							}

						}

						g_free(src);
					}

				}

				if( g_key_file_has_key(keyfile, group, "ColumnsOpeWidth", NULL) )
				{
				gint *src;
				gsize length;

					src = g_key_file_get_integer_list(keyfile, group, "ColumnsOpeWidth", &length, &error);

					DB( g_print(" - length %d (max=%d)\n", length, NUM_LST_DSPOPE) );

					if( length == NUM_LST_DSPOPE-1 )
					{
						DB( g_print(" - copying column width from pref file\n") );
						memcpy(PREFS->lst_ope_col_size, src, length*sizeof(gint));
					}
				}
			
				homebank_pref_get_integer(keyfile, group, "OpeSortId", &PREFS->lst_ope_sort_id);
				homebank_pref_get_integer(keyfile, group, "OpeSortOrder", &PREFS->lst_ope_sort_order);

			    DB( g_print(" - set sort to %d %d\n", PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order) );

				homebank_pref_get_short(keyfile, group, "FiscYearDay", &PREFS->fisc_year_day);
				homebank_pref_get_short(keyfile, group, "FiscYearMonth", &PREFS->fisc_year_month);


			group = "Windows";

				DB( g_print(" -> ** Windows\n") );

				homebank_pref_get_wingeometry(keyfile, group, "Wal", &PREFS->wal_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Acc", &PREFS->acc_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Sta", &PREFS->sta_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Tme", &PREFS->tme_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Ove", &PREFS->ove_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Bud", &PREFS->bud_wg);
				homebank_pref_get_wingeometry(keyfile, group, "Car", &PREFS->cst_wg);

				homebank_pref_get_wingeometry(keyfile, group, "Txn", &PREFS->txn_wg);

				if(version <= 7)	//set maximize to 0
				{
					PREFS->wal_wg.s = 0;
					PREFS->acc_wg.s = 0;
					PREFS->txn_wg.s = 0;
					PREFS->sta_wg.s = 0;
					PREFS->tme_wg.s = 0;
					PREFS->ove_wg.s = 0;
					PREFS->bud_wg.s = 0;
					PREFS->cst_wg.s = 0;
				}
				homebank_pref_get_integer(keyfile, group, "WalVPaned", &PREFS->wal_vpaned);
				homebank_pref_get_integer(keyfile, group, "WalHPaned", &PREFS->wal_hpaned);
				homebank_pref_get_boolean(keyfile, group, "WalToolbar", &PREFS->wal_toolbar);
				homebank_pref_get_boolean(keyfile, group, "WalSpending", &PREFS->wal_spending);
				homebank_pref_get_boolean(keyfile, group, "WalUpcoming", &PREFS->wal_upcoming);

			//since 5.1.3
			group = "Panels";

				DB( g_print(" -> ** Panels\n") );

				homebank_pref_get_short(keyfile, group, "AccColAccW", &PREFS->pnl_acc_col_acc_width);
				homebank_pref_get_short(keyfile, group, "AccShowBy" , &PREFS->pnl_acc_show_by);

				homebank_pref_get_short(keyfile, group, "UpcColPayW", &PREFS->pnl_upc_col_pay_width);
				homebank_pref_get_short(keyfile, group, "UpcColMemW", &PREFS->pnl_upc_col_mem_width);


			group = "Format";

				DB( g_print(" -> ** Format\n") );

				homebank_pref_get_string(keyfile, group, "DateFmt", &PREFS->date_format);

				if(version < 460)
				{
				gboolean useimperial;

					homebank_pref_get_boolean(keyfile, group, "UKUnits", &useimperial);
					if(useimperial)
					{
						PREFS->vehicle_unit_ismile = TRUE;
						PREFS->vehicle_unit_isgal = TRUE;
					}
				}

				homebank_pref_get_boolean(keyfile, group, "UnitIsMile", &PREFS->vehicle_unit_ismile);
				homebank_pref_get_boolean(keyfile, group, "UnitIsGal", &PREFS->vehicle_unit_isgal);


			group = "Filter";

				DB( g_print(" -> ** Filter\n") );

				homebank_pref_get_integer(keyfile, group, "DateRangeWal", &PREFS->date_range_wal);
				homebank_pref_get_integer(keyfile, group, "DateRangeTxn", &PREFS->date_range_txn);
				homebank_pref_get_integer(keyfile, group, "DateFutureNbDays", &PREFS->date_future_nbdays);
				homebank_pref_get_integer(keyfile, group, "DateRangeRep", &PREFS->date_range_rep);

				if(version <= 7)
				{
					// shift date range >= 5, since we inserted a new one at position 5
					if(PREFS->date_range_wal >= FLT_RANGE_LASTYEAR)
						PREFS->date_range_wal++;
					if(PREFS->date_range_txn >= FLT_RANGE_LASTYEAR)
						PREFS->date_range_txn++;
					if(PREFS->date_range_rep >= FLT_RANGE_LASTYEAR)
						PREFS->date_range_rep++;
				}


			group = "Euro";

				DB( g_print(" -> ** Euro\n") );

				//homebank_pref_get_string(keyfile, group, "DefCurrency" , &PREFS->curr_default);

				homebank_pref_get_boolean(keyfile, group, "Active", &PREFS->euro_active);
				homebank_pref_get_integer(keyfile, group, "Country", &PREFS->euro_country);

				gchar *ratestr = g_key_file_get_string (keyfile, group, "ChangeRate", NULL);
				if(ratestr != NULL) PREFS->euro_value = g_ascii_strtod(ratestr, NULL);

				if(version <= 1)
				{
					homebank_pref_get_string(keyfile, group, "Symbol", &PREFS->minor_cur.symbol);
					PREFS->minor_cur.frac_digits = g_key_file_get_integer (keyfile, group, "NBDec", NULL);

					//PREFS->euro_nbdec = g_key_file_get_integer (keyfile, group, "NBDec", NULL);
					//PREFS->euro_thsep = g_key_file_get_boolean (keyfile, group, "Sep", NULL);
					//gchar *tmpstr = g_key_file_get_string  (keyfile, group, "Symbol", &error);
				}
				else
				{
					if(version < 460)
					{
					gchar *prefix = NULL;
					gchar *suffix = NULL;

						homebank_pref_get_string(keyfile, group, "PreSymbol", &prefix);
						homebank_pref_get_string(keyfile, group, "SufSymbol", &suffix);
						homebank_pref_currfmt_convert(&PREFS->minor_cur, prefix, suffix);
						g_free(prefix);
						g_free(suffix);
					}
					else
					{
						homebank_pref_get_string(keyfile, group, "Symbol", &PREFS->minor_cur.symbol);
						homebank_pref_get_boolean(keyfile, group, "IsPrefix", &PREFS->minor_cur.sym_prefix);
					}
					homebank_pref_get_string(keyfile, group, "DecChar"  , &PREFS->minor_cur.decimal_char);
					homebank_pref_get_string(keyfile, group, "GroupChar", &PREFS->minor_cur.grouping_char);
					homebank_pref_get_short(keyfile, group, "FracDigits", &PREFS->minor_cur.frac_digits);

					//fix 378992/421228
					if( PREFS->minor_cur.frac_digits > MAX_FRAC_DIGIT )
						PREFS->minor_cur.frac_digits = MAX_FRAC_DIGIT;

					da_cur_initformat(&PREFS->minor_cur);
				}

			//PREFS->euro_symbol = g_locale_to_utf8(tmpstr, -1, NULL, NULL, NULL);

			group = "Report";

				DB( g_print(" -> ** Report\n") );

				homebank_pref_get_boolean(keyfile, group, "StatByAmount", &PREFS->stat_byamount);
				homebank_pref_get_boolean(keyfile, group, "StatDetail", &PREFS->stat_showdetail);
				homebank_pref_get_boolean(keyfile, group, "StatRate", &PREFS->stat_showrate);
				homebank_pref_get_boolean(keyfile, group, "BudgDetail", &PREFS->budg_showdetail);
				homebank_pref_get_integer(keyfile, group, "ColorScheme", &PREFS->report_color_scheme);

			group = "Exchange";

				DB( g_print(" -> ** Exchange\n") );

				//homebank_pref_get_boolean(keyfile, group, "DoIntro", &PREFS->dtex_nointro);
				homebank_pref_get_integer(keyfile, group, "DateFmt", &PREFS->dtex_datefmt);
				homebank_pref_get_integer(keyfile, group, "OfxName", &PREFS->dtex_ofxname);
				homebank_pref_get_integer(keyfile, group, "OfxMemo", &PREFS->dtex_ofxmemo);
				homebank_pref_get_boolean(keyfile, group, "QifMemo", &PREFS->dtex_qifmemo);
				homebank_pref_get_boolean(keyfile, group, "QifSwap", &PREFS->dtex_qifswap);


			//group = "Chart";
			//PREFS->chart_legend = g_key_file_get_boolean (keyfile, group, "Legend", NULL);


			/*
			#if MYDEBUG == 1
			gsize length;
			gchar *contents = g_key_file_to_data (keyfile, &length, NULL);
			//g_print(" keyfile:\n%s\n len=%d\n", contents, length);
			g_free(contents);
			#endif
			*/

		}
		g_free(filename);
		g_key_file_free (keyfile);

		_homebank_pref_init_measurement_units();
	}

	return retval;
}

static void homebank_pref_set_string(
	GKeyFile *key_file,
    const gchar *group_name,
    const gchar *key,
	gchar *string)
{

	DB( g_print(" - homebank_pref_set_string :: group='%s' key='%s' value='%s'\n", group_name, key, string) );

	if( string != NULL && *string != '\0')
		g_key_file_set_string  (key_file, group_name, key, string);
	else
		g_key_file_set_string  (key_file, group_name, key, "");

}


/*
** save preference to homedir/.homebank (HB_DATA_PATH)
*/
gboolean homebank_pref_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
gsize length;

	DB( g_print("\n[preferences] pref save\n") );

	keyfile = g_key_file_new();
	if(keyfile )
	{

		DB( g_print(" -> ** general\n") );


		group = "General";
		g_key_file_set_integer  (keyfile, group, "Version", PREF_VERSION);

		homebank_pref_set_string  (keyfile, group, "Language", PREFS->language);

		g_key_file_set_integer (keyfile, group, "BarStyle", PREFS->toolbar_style);
		//g_key_file_set_integer (keyfile, group, "BarImageSize", PREFS->image_size);

		g_key_file_set_boolean (keyfile, group, "CustomColors", PREFS->custom_colors);
		g_key_file_set_string (keyfile, group, "ColorExp" , PREFS->color_exp);
		g_key_file_set_string (keyfile, group, "ColorInc" , PREFS->color_inc);
		g_key_file_set_string (keyfile, group, "ColorWarn", PREFS->color_warn);

		g_key_file_set_integer (keyfile, group, "GridLines", PREFS->grid_lines);

		homebank_pref_set_string  (keyfile, group, "WalletPath"   , PREFS->path_hbfile);
		homebank_pref_set_string  (keyfile, group, "ImportPath"   , PREFS->path_import);
		homebank_pref_set_string  (keyfile, group, "ExportPath"   , PREFS->path_export);
		//g_key_file_set_string  (keyfile, group, "NavigatorPath", PREFS->path_navigator);

		g_key_file_set_boolean (keyfile, group, "ShowSplash", PREFS->showsplash);
		g_key_file_set_boolean (keyfile, group, "LoadLast", PREFS->loadlast);
		g_key_file_set_boolean (keyfile, group, "AppendScheduled", PREFS->appendscheduled);
		g_key_file_set_boolean (keyfile, group, "UpdateCurrency", PREFS->do_update_currency);

		g_key_file_set_boolean (keyfile, group, "HeritDate", PREFS->heritdate);
		g_key_file_set_boolean (keyfile, group, "HideReconciled", PREFS->hidereconciled);
		g_key_file_set_boolean (keyfile, group, "ShowRemind", PREFS->showremind);

		g_key_file_set_integer_list(keyfile, group, "ColumnsOpe", PREFS->lst_ope_columns, NUM_LST_DSPOPE-1);
		g_key_file_set_integer_list(keyfile, group, "ColumnsOpeWidth", PREFS->lst_ope_col_size, NUM_LST_DSPOPE-1);
		g_key_file_set_integer     (keyfile, group, "OpeSortId" , PREFS->lst_ope_sort_id);
		g_key_file_set_integer     (keyfile, group, "OpeSortOrder" , PREFS->lst_ope_sort_order);

		g_key_file_set_integer     (keyfile, group, "FiscYearDay" , PREFS->fisc_year_day);
		g_key_file_set_integer     (keyfile, group, "FiscYearMonth" , PREFS->fisc_year_month);

		// added v3.4
		DB( g_print(" -> ** windows\n") );

		group = "Windows";
		g_key_file_set_integer_list(keyfile, group, "Wal", (gint *)&PREFS->wal_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Acc", (gint *)&PREFS->acc_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Sta", (gint *)&PREFS->sta_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Tme", (gint *)&PREFS->tme_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Ove", (gint *)&PREFS->ove_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Bud", (gint *)&PREFS->bud_wg, 5);
		g_key_file_set_integer_list(keyfile, group, "Car", (gint *)&PREFS->cst_wg, 5);

		g_key_file_set_integer_list(keyfile, group, "Txn", (gint *)&PREFS->txn_wg, 5);

		g_key_file_set_integer (keyfile, group, "WalVPaned" , PREFS->wal_vpaned);
		g_key_file_set_integer (keyfile, group, "WalHPaned" , PREFS->wal_hpaned);
		g_key_file_set_boolean (keyfile, group, "WalToolbar", PREFS->wal_toolbar);
		g_key_file_set_boolean (keyfile, group, "WalSpending", PREFS->wal_spending);
		g_key_file_set_boolean (keyfile, group, "WalUpcoming", PREFS->wal_upcoming);

		//since 5.1.3
		DB( g_print(" -> ** Panels\n") );

		group = "Panels";
		g_key_file_set_integer(keyfile, group, "AccColAccW", PREFS->pnl_acc_col_acc_width);
		g_key_file_set_integer(keyfile, group, "AccShowBy" , PREFS->pnl_acc_show_by);

		g_key_file_set_integer(keyfile, group, "UpcColPayW", PREFS->pnl_upc_col_pay_width);
		g_key_file_set_integer(keyfile, group, "UpcColMemW", PREFS->pnl_upc_col_mem_width);


		DB( g_print(" -> ** format\n") );

		group = "Format";
		homebank_pref_set_string  (keyfile, group, "DateFmt"   , PREFS->date_format);

		//g_key_file_set_boolean (keyfile, group, "UKUnits" , PREFS->imperial_unit);
		g_key_file_set_boolean (keyfile, group, "UnitIsMile" , PREFS->vehicle_unit_ismile);
		g_key_file_set_boolean (keyfile, group, "UnitIsGal" , PREFS->vehicle_unit_isgal);


		DB( g_print(" -> ** filter\n") );

		group = "Filter";
		g_key_file_set_integer (keyfile, group, "DateRangeWal", PREFS->date_range_wal);
		g_key_file_set_integer (keyfile, group, "DateRangeTxn", PREFS->date_range_txn);
		g_key_file_set_integer (keyfile, group, "DateFutureNbdays", PREFS->date_future_nbdays);
		g_key_file_set_integer (keyfile, group, "DateRangeRep", PREFS->date_range_rep);

		DB( g_print(" -> ** euro\n") );

	//euro options
		group = "Euro";

		//homebank_pref_set_string(keyfile, group, "DefCurrency" , PREFS->curr_default);

		g_key_file_set_boolean (keyfile, group, "Active" , PREFS->euro_active);
		if( PREFS->euro_active )
		{
			g_key_file_set_integer (keyfile, group, "Country", PREFS->euro_country);
			gchar ratestr[64];
			g_ascii_dtostr(ratestr, 63, PREFS->euro_value);
			homebank_pref_set_string  (keyfile, group, "ChangeRate", ratestr);
			homebank_pref_set_string  (keyfile, group, "Symbol" , PREFS->minor_cur.symbol);
			g_key_file_set_boolean    (keyfile, group, "IsPrefix" , PREFS->minor_cur.sym_prefix);
			homebank_pref_set_string  (keyfile, group, "DecChar"   , PREFS->minor_cur.decimal_char);
			homebank_pref_set_string  (keyfile, group, "GroupChar" , PREFS->minor_cur.grouping_char);
			g_key_file_set_integer (keyfile, group, "FracDigits", PREFS->minor_cur.frac_digits);
		}

	//report options
		DB( g_print(" -> ** report\n") );

		group = "Report";
		g_key_file_set_boolean (keyfile, group, "StatByAmount", PREFS->stat_byamount);
		g_key_file_set_boolean (keyfile, group, "StatDetail"  , PREFS->stat_showdetail);
		g_key_file_set_boolean (keyfile, group, "StatRate"    , PREFS->stat_showrate);
		g_key_file_set_boolean (keyfile, group, "BudgDetail"  , PREFS->budg_showdetail);
		g_key_file_set_integer (keyfile, group, "ColorScheme" , PREFS->report_color_scheme);


		group = "Exchange";
		//g_key_file_set_boolean (keyfile, group, "DoIntro", PREFS->dtex_nointro);
		g_key_file_set_integer (keyfile, group, "DateFmt", PREFS->dtex_datefmt);
		g_key_file_set_integer (keyfile, group, "OfxName", PREFS->dtex_ofxname);
		g_key_file_set_integer (keyfile, group, "OfxMemo", PREFS->dtex_ofxmemo);
		g_key_file_set_boolean (keyfile, group, "QifMemo", PREFS->dtex_qifmemo);
		g_key_file_set_boolean (keyfile, group, "QifSwap", PREFS->dtex_qifswap);


		//group = "Chart";
		//g_key_file_set_boolean (keyfile, group, "Legend", PREFS->chart_legend);

		//g_key_file_set_string  (keyfile, group, "", PREFS->);
		//g_key_file_set_boolean (keyfile, group, "", PREFS->);
		//g_key_file_set_integer (keyfile, group, "", PREFS->);

		DB( g_print(" -> ** g_key_file_to_data\n") );

		gchar *contents = g_key_file_to_data (keyfile, &length, NULL);

		//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

		filename = g_build_filename(homebank_app_get_config_dir(), "preferences", NULL );

		DB( g_print(" -> filename: %s\n", filename) );

		g_file_set_contents(filename, contents, length, NULL);

		DB( g_print(" -> contents: %s\n", contents) );

		DB( g_print(" -> freeing filename\n") );
		g_free(filename);

		DB( g_print(" -> freeing buffer\n") );

		g_free(contents);

		DB( g_print(" -> freeing keyfile\n") );

		g_key_file_free (keyfile);
	}

	_homebank_pref_init_measurement_units();

	return retval;
}

