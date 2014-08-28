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

#ifndef __HOMEBANK_H__
#define __HOMEBANK_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h> 		/* isprint */
#include <errno.h>
#include <math.h>
#include <libintl.h>
#include <locale.h>
#include <stdlib.h>		/* atoi, atof, atol */
#include <string.h>		/* memset, memcpy, strcmp, strcpy */
#include <time.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "enums.h"
#include "hb-preferences.h"

#include "hb-transaction.h"
#include "hb-account.h"
#include "hb-archive.h"
#include "hb-assign.h"
#include "hb-category.h"
#include "hb-encoding.h"
#include "hb-export.h"
#include "hb-filter.h"
#include "hb-import.h"
#include "hb-misc.h"
#include "hb-payee.h"
#include "hb-report.h"
#include "hb-tag.h"
#include "hb-hbfile.h"
#include "hb-xml.h"

#include "ui-dialogs.h"
#include "ui-pref.h"
#include "ui-widgets.h"

#define _(str) gettext (str)
#define gettext_noop(str) (str)
#define N_(str) gettext_noop (str)

/* = = = = = = = = */
/* = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

#define HB_UNSTABLE			FALSE
#define HB_VERSION			"4.6.3"
#define FILE_VERSION		0.9
#define PREF_VERSION		460

#if HB_UNSTABLE == FALSE
	#define	PROGNAME		"HomeBank"
	#define HB_DATA_PATH	"homebank"
#else
	#define	PROGNAME		"HomeBank " HB_VERSION " (unstable)"
	#define HB_DATA_PATH	"homebank_unstable"
#endif

#ifdef G_OS_WIN32
	#define GETTEXT_PACKAGE "homebank"
	#define LOCALE_DIR      "locale"
	#define PIXMAPS_DIR     "images"
	#define HELP_DIR        "help"
	#define PACKAGE_VERSION HB_VERSION
	#define PACKAGE         "homebank"
	#define VERSION         HB_VERSION

	//#define PORTABLE_APP
	//#define NOOFX

	#define ENABLE_NLS 1
#endif

/* container spacing */
#define PHI 1.61803399

#define HB_MAINBOX_SPACING	12
#define HB_BOX_SPACING		6

#define HB_HSPACE_SPACING	18  /* used to separate dialog block */
#define HB_TABROW_SPACING	6
#define HB_TABCOL_SPACING	6

/* widget minimum width */
#define HB_MINWIDTH_LIST	161
#define HB_MINWIDTH_COMBO	80


/* for transaction dialog */
#define GTK_RESPONSE_ADD	 1


#define HB_NUMBER_SAMPLE	20457.99


enum
{
	FILETYPE_UNKNOW,
	FILETYPE_HOMEBANK,
	FILETYPE_OFX,
	FILETYPE_QIF,
	FILETYPE_CSV_HB,
	FILETYPE_AMIGA_HB,
	NUM_FILETYPE
};

/*
** stock icons
*/

/* Custom HomeBank named icons */
#define HB_STOCK_ACCOUNT         "hb-account"
#define HB_STOCK_ARCHIVE         "hb-archive"
#define HB_STOCK_ASSIGN          "hb-assign"
#define HB_STOCK_BUDGET          "hb-budget"
#define HB_STOCK_CATEGORY        "hb-category"
#define HB_STOCK_PAYEE           "hb-payee"
#define HB_STOCK_FILTER          "hb-filter"
#define HB_STOCK_OPE_ADD         "hb-ope-add"
#define HB_STOCK_OPE_HERIT       "hb-ope-herit"
#define HB_STOCK_OPE_EDIT        "hb-ope-edit"
#define HB_STOCK_OPE_SHOW        "hb-ope-show"
#define HB_STOCK_OPE_DELETE      "hb-ope-delete"
#define HB_STOCK_OPE_VALID       "hb-ope-valid"
#define HB_STOCK_OPE_REMIND      "hb-ope-remind"
#define HB_STOCK_OPE_AUTO        "hb-ope-auto"
#define HB_STOCK_REP_STATS       "hb-rep-stats"
#define HB_STOCK_REP_TIME        "hb-rep-time"
#define HB_STOCK_REP_BALANCE     "hb-rep-balance"
#define HB_STOCK_REP_BUDGET      "hb-rep-budget"
#define HB_STOCK_REP_CAR         "hb-rep-vehicle"


/*
** Global application datas
*/
struct HomeBank
{
	// hbfile storage
	GHashTable	*h_cur;			//currencies
	GHashTable	*h_acc;			//accounts
	GHashTable	*h_pay;			//payees
	GHashTable	*h_cat;			//categories
	GHashTable	*h_tag;			//tags
	GHashTable	*h_rul;			//assign rules

	GHashTable	*h_memo;		//memo/description

	GList		*arc_list;		//archives
	GList		*ope_list;		//transactions

	// hbfile (saved properties)
	gchar		*owner;
	gshort		auto_smode;
	gshort		auto_weekday;
	gshort		auto_nbdays;

	guint32		vehicle_category;
	//guint32		kcur;			// base currency

	// hbfile (unsaved properties)
	guint		changes_count;
	gboolean	hbfile_is_new;
	gchar		*xhb_filepath;
	gboolean	xhb_hasbak;		//file has backup (*.xhb~) used for revert menu sensitivity

	// really global stuffs
	gboolean	first_run;
	guint32		today;			//today's date
	gint		define_off;		//>0 when a stat, account window is opened
	gboolean	minor;

	GtkWidget	*mainwindow;	//should be global to access attached window data
	GdkPixbuf	*lst_pixbuf[NUM_LST_PIXBUF];
	gint		lst_pixbuf_maxwidth;

};

gint homebank_alienfile_recognize(gchar *filename);
gchar *homebank_filepath_with_extention(gchar *path, gchar *extension);
gchar *homebank_filename_without_extention(gchar *path);
void homebank_file_ensure_xhb(void);
void homebank_backup_current_file(gchar *pathname);
gboolean homebank_util_url_show (const gchar *url);
gboolean homebank_lastopenedfiles_load(void);
gboolean homebank_lastopenedfiles_save(void);


void homebank_window_set_icon_from_file(GtkWindow *window, gchar *filename);

const gchar *homebank_app_get_config_dir (void);
const gchar *homebank_app_get_images_dir (void);
const gchar *homebank_app_get_pixmaps_dir (void);
const gchar *homebank_app_get_locale_dir (void);
const gchar *homebank_app_get_help_dir (void);
const gchar *homebank_app_get_datas_dir (void);
const gchar *homebank_app_get_pkglib_dir (void);
guint32 homebank_app_date_get_julian(void);

/* - - - - obsolete things - - - - */

/*

typedef struct _budget		Budget;

struct _budget
{
	guint	key;
	gushort	flags;
	guint	cat_key;
	guint	year;
	gdouble	value[13];
};
*/

/*
struct _investment
{
	guint	date;
	gdouble	buy_amount;
	gdouble	curr_amount;
	gdouble	commission;
	guint	number;
	guint	account;
	gchar	*name;
	gchar	*symbol;
	gchar	*note;
};
*/




#endif
