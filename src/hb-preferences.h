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

#ifndef __HB_PREFERENCES_H__
#define __HB_PREFERENCES_H__

#define DEFAULT_FORMAT_DATE			"%x"

#define MAX_FRAC_DIGIT		6

//Tango light
#define LIGHT_EXP_COLOR		"#fcaf3e"	//Orange
#define LIGHT_INC_COLOR		"#8ae234"	//Chameleon
#define LIGHT_WARN_COLOR	"#ef2929"	//Scarlett Red

//Tango medium
#define MEDIUM_EXP_COLOR	"#f57900"	//Orange
#define MEDIUM_INC_COLOR	"#73d216"	//Chameleon
#define MEDIUM_WARN_COLOR	"#cc0000"	//Scarlett Red

//Tango dark
#define DEFAULT_EXP_COLOR		"#ce5c00"	//Orange
#define DEFAULT_INC_COLOR		"#4e9a36"	//Chameleon
#define DEFAULT_WARN_COLOR		"#a40000"	//Scarlett Red

/*
** Preference datas
*/
struct CurrencyFmt
{
	//gchar		*prefix_symbol;		/* max symbol is 3 digits in unicode */
	//gchar		*suffix_symbol;		/* but mostly is 1 digit, and most are prefix ~100  */
	gchar	    *symbol;
	gboolean	is_prefix;
	gchar		*decimal_char;	
	gchar		*grouping_char;	
	gshort		frac_digits;
	gchar		format[8];			/* hold decimal format: '%.xf' */
	gchar		monfmt[32];			/* hold monetary format: 'prefix %s suffix' */ 
};

struct WinGeometry
{
	gint		l, t, w, h, s;
};


struct Preferences
{
	//general
	gboolean	showsplash;
	gboolean	loadlast;
	gboolean	appendscheduled;

	gchar	   *language;
	
	gchar		*path_hbfile;
	gchar		*path_import;
	gchar		*path_export;

	gboolean	heritdate;
	gboolean	hidereconciled;
	gint		date_range_wal;
	gint		date_range_txn;
	gint		date_range_rep;

	gshort		fisc_year_day;
	gshort		fisc_year_month;
	
	//interface
	gshort		toolbar_style;
	gboolean	custom_colors;
	gchar		*color_exp;
	gchar		*color_inc;
	gchar		*color_warn;
	gboolean	rules_hint;

	//display format
	gchar		*date_format;

	struct	CurrencyFmt base_cur;

	//gshort	num_nbdecimal;
	//gboolean	num_separator;
	//gboolean	imperial_unit;

	//help system
	//gboolean	show_tooltips;
	//gboolean	show_help_button;
	//gboolean	show_tipofday;
	//gchar		*path_navigator;

	//currency
	//gchar		*curr_default;
	
	//euro zone
	gboolean	euro_active;
	gint		euro_country;
	gdouble		euro_value;

	struct	CurrencyFmt minor_cur;

	//gshort	euro_nbdec;
	//gboolean	euro_thsep;
	//gchar		*euro_symbol;

	//report options
	gboolean	stat_byamount;
	gboolean	stat_showrate;
	gboolean	stat_showdetail;
	gboolean	budg_showdetail;
	gint		report_color_scheme;

	//data exchange options
	gint		dtex_ofxmemo;
	gint		dtex_datefmt;

	//chart options
	gboolean	chart_legend;


	
	/* internal : not saved*/

	gint 	lst_ope_columns[NUM_LST_DSPOPE+1];
	gint	lst_ope_sort_id;
	gint	lst_ope_sort_order;

	/* windows size an position */
	struct WinGeometry	wal_wg;
	struct WinGeometry	acc_wg;
	struct WinGeometry	sta_wg;
	struct WinGeometry	tme_wg;
	struct WinGeometry	ove_wg;
	struct WinGeometry	bud_wg;
	struct WinGeometry	cst_wg;

	gboolean	wal_toolbar;
	gboolean	wal_spending;
	gboolean	wal_upcoming;

	gint		wal_vpaned;
	gint		wal_hpaned;

	//vehiclecost units (mile/gal or km/liters)
	gboolean	vehicle_unit_ismile;	// true is unit is mile, default Km
	gboolean	vehicle_unit_isgal;		// true is unit is gallon, default Liter
	
	gchar	   *vehicle_unit_dist;
	gchar	   *vehicle_unit_vol;
	gchar	   *vehicle_unit_100;
	gchar	   *vehicle_unit_distbyvol;

};


void homebank_prefs_set_default(void);
void homebank_pref_free(void);
void homebank_pref_createformat(void);
void homebank_pref_init_measurement_units(void);
gboolean homebank_pref_load(void);
gboolean homebank_pref_save(void);
void homebank_pref_setdefault(void);

#endif
