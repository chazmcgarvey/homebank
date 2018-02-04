/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2018 Maxime DOYEN
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

#ifndef __HB_PREFERENCE_GTK_H__
#define __HB_PREFERENCE_GTK_H__


struct defpref_data
{
	GtkWidget	*window;

	GtkWidget	*LV_page;
	GtkWidget	*GR_page;

	GtkWidget	*label;
	GtkWidget	*image;
	GtkWidget   *BT_clear;

	GtkWidget	*CY_language;
	GtkWidget	*CY_toolbar;

	GtkWidget   *GR_colors;
	GtkWidget	*CY_colors;
	GtkWidget	*CM_custom_colors;
	GtkWidget	*CP_exp_color;
	GtkWidget	*CP_inc_color;
	GtkWidget	*CP_warn_color;
	//GtkWidget	*CM_ruleshint;
	GtkWidget	*CY_gridlines;

	GtkWidget	*LV_opecolumns;
	GtkWidget	*BT_go_up;
	GtkWidget	*BT_go_down;

	GtkWidget   *NB_fiscyearday;
	GtkWidget   *CY_fiscyearmonth;
	
	GtkWidget	*CM_runwizard;

	GtkWidget	*ST_path_hbfile, *BT_path_hbfile;
	GtkWidget	*ST_path_import, *BT_path_import;
	GtkWidget	*ST_path_export, *BT_path_export;

	GtkWidget	*CM_load_last;
	GtkWidget	*CM_show_splash;
	GtkWidget	*CM_append_scheduled;
	GtkWidget   *CM_do_update_currency;
	GtkWidget	*CM_herit_date;
	GtkWidget	*CM_hide_reconciled;
	GtkWidget	*CM_show_remind;

	GtkWidget	*ST_datefmt;
	GtkWidget	*LB_date;

	GtkWidget	*CM_unitismile;
	GtkWidget	*CM_unitisgal;

	GtkWidget	*CY_daterange_wal;
	GtkWidget	*CY_daterange_txn;
	GtkWidget   *ST_datefuture_nbdays;
	GtkWidget	*CY_daterange_rep;
	
	/* currencies */
	GtkWidget	*LB_default;
	GtkWidget	*BT_default; 
	
	GtkWidget	*CM_euro_enable;
	GtkWidget	*GRP_currency;
	GtkWidget	*GRP_rate;
	GtkWidget	*GRP_format;
	 
	GtkWidget	*CY_euro_preset;
	GtkWidget	*ST_euro_country;
	GtkWidget	*NB_euro_value;

	GtkWidget	*ST_euro_symbol;
	GtkWidget	*CM_euro_isprefix;
	GtkWidget	*ST_euro_decimalchar;	
	GtkWidget	*ST_euro_groupingchar;	
	GtkWidget	*NB_euro_fracdigits;
	GtkWidget	*LB_numbereuro;

	//GtkWidget	*ST_euro_symbol;
	//GtkWidget	*NB_euro_nbdec;
	//GtkWidget	*CM_euro_thsep;

	GtkWidget	*CM_stat_byamount;
	GtkWidget	*CM_stat_showdetail;
	GtkWidget	*CM_stat_showrate;

	GtkWidget	*CM_budg_showdetail;

	GtkWidget	*CY_color_scheme;
	GtkWidget   *DA_colors;

	GtkWidget	*CM_chartlegend;

	GtkWidget	*CY_dtex_datefmt;
	GtkWidget	*CY_dtex_ofxname;
	GtkWidget	*CY_dtex_ofxmemo;
	GtkWidget	*CM_dtex_qifmemo;
	GtkWidget	*CM_dtex_qifswap;

	gint		country;

};


typedef struct 
{
	gchar   *locale;
	gchar   *name;
} LangName;


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



void free_pref_icons(void);
void load_pref_icons(void);

GtkWidget *defpref_dialog_new (void);

#endif
