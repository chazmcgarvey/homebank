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

#ifndef _HOMEBANK_H_
#define _HOMEBANK_H_

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
#include "hb-currency.h"
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

/* = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

#define HB_UNSTABLE			FALSE
#define HB_UNSTABLE_SHOW	FALSE


#define HB_VERSION_MAJOR	5
#define HB_VERSION_MINOR	1
#define HB_VERSION_MICRO	7

#define HB_VERSION			"5.1.7"
#define HB_VERSION_NUM	(HB_VERSION_MAJOR*10000) + (HB_VERSION_MINOR*100) + HB_VERSION_MICRO

#define FILE_VERSION		1.2
#define PREF_VERSION		517

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
#define SPACING_TINY		3
#define SPACING_SMALL		6
#define SPACING_MEDIUM		12
#define SPACING_LARGE		18


#define HB_MINDATE  693596	  //01/01/1900
#define HB_MAXDATE  803533	  //31/12/2200

/* widget minimum width */
#define HB_MINWIDTH_LIST	161
#define HB_MINHEIGHT_LIST	260

#define HB_MINWIDTH_SEARCH	240
#define HB_MINWIDTH_COLUMN  48


/* miscellaneous */
#define PHI 1.61803399

/* official GTK_RESPONSE are negative */
#define GTK_RESPONSE_ADD		 1
#define GTK_RESPONSE_ADDKEEP	 2

#define HB_NUMBER_SAMPLE	1234567.89


typedef enum
{
	FILETYPE_UNKNOW,
	FILETYPE_HOMEBANK,
	FILETYPE_OFX,
	FILETYPE_QIF,
	FILETYPE_CSV_HB,
//	FILETYPE_AMIGA_HB,
	NUM_FILETYPE
} HbFileType;


/* ---- icon size as defined into gtkiconfactory.c ---- */
/* GTK_ICON_SIZE_MENU 16
 * GTK_ICON_SIZE_BUTTON 20
 * GTK_ICON_SIZE_SMALL_TOOLBAR 18
 * GTK_ICON_SIZE_LARGE_TOOLBAR 24 (default for toolbar)
 * GTK_ICON_SIZE_DND 32
 * GTK_ICON_SIZE_DIALOG 48
 */


/* -------- named icons (Standard Icon Name) -------- */
#define ICONNAME_NEW				"document-new"
#define ICONNAME_OPEN				"document-open"
#define ICONNAME_SAVE				"document-save"
#define ICONNAME_SAVE_AS			"document-save-as"	  //obsolete
#define ICONNAME_REVERT			    "document-revert"	  //obsolete
#define ICONNAME_PRINT				"document-print"		//unused
#define ICONNAME_PROPERTIES			"document-properties"   //obsolete
#define ICONNAME_CLOSE				"window-close"	  //obsolete
#define ICONNAME_QUIT				"application-exit"	  //obsolete
#define ICONNAME_FIND				"edit-find"
#define ICONNAME_CLEAR				"edit-clear"
#define ICONNAME_WARNING			"dialog-warning"
#define ICONNAME_ERROR				"dialog-error"
#define ICONNAME_INFO				"dialog-information"
#define ICONNAME_HELP				"help-browser"	  //obsolete
#define ICONNAME_ABOUT				"help-about"	  //obsolete
#define ICONNAME_PREFERENCES		"preferences-system"	  //obsolete
#define ICONNAME_REFRESH			"view-refresh"		

#define ICONNAME_FOLDER				"folder-symbolic"
#define ICONNAME_EMBLEM_SYSTEM		"emblem-system-symbolic"


#define ICONNAME_LIST_ADD			"list-add-symbolic"
#define ICONNAME_LIST_REMOVE		"list-remove-symbolic"

#define ICONNAME_CHANGES_PREVENT	"changes-prevent-symbolic"

// custom or gnome not found
#define ICONNAME_HB_BUTTON_MENU		"open-menu-symbolic"
#define ICONNAME_HB_TOGGLE_SIGN		"toggle-sign-symbolic"


/* -------- named icons (Custom to homebank) -------- */
#define ICONNAME_HB_CURRENCY		"hb-currency"
#define ICONNAME_HB_ACCOUNT         "hb-account"
#define ICONNAME_HB_ARCHIVE         "hb-archive"
#define ICONNAME_HB_ASSIGN          "hb-assign"
#define ICONNAME_HB_BUDGET          "hb-budget"
#define ICONNAME_HB_CATEGORY        "hb-category"
#define ICONNAME_HB_PAYEE           "hb-payee"
#define ICONNAME_HB_OPE_SHOW        "hb-ope-show"   //? "view-register
#define ICONNAME_HB_REP_STATS       "hb-rep-stats"
#define ICONNAME_HB_REP_TIME        "hb-rep-time"
#define ICONNAME_HB_REP_BALANCE     "hb-rep-balance"
#define ICONNAME_HB_REP_BUDGET      "hb-rep-budget"
#define ICONNAME_HB_REP_CAR         "hb-rep-vehicle"

#define ICONNAME_HB_VIEW_LIST	    "hb-view-list"   //"view-list-text"
#define ICONNAME_HB_VIEW_BAR	    "hb-view-bar"    //"view-chart-bar"
#define ICONNAME_HB_VIEW_COLUMN	    "hb-view-column" //"view-chart-column"
#define ICONNAME_HB_VIEW_LINE	    "hb-view-line"   //"view-chart-line"
#define ICONNAME_HB_VIEW_STACK	    "hb-view-stack"  //"view-chart-stack"
#define ICONNAME_HB_VIEW_PIE	    "hb-view-pie"    //"view-chart-pie"
#define ICONNAME_HB_VIEW_DONUT	    "hb-view-donut"  //"view-chart-donut"
#define ICONNAME_HB_SHOW_LEGEND	    "hb-legend"		 //"view-legend"
#define ICONNAME_HB_SHOW_RATE	    "hb-rate"	    // obsolete ?
#define ICONNAME_HB_FILTER          "hb-filter"		 //"edit-filter"

#define ICONNAME_HB_FILE_IMPORT		"hb-file-import"	//document-import
#define ICONNAME_HB_FILE_EXPORT		"hb-file-export"	//document-export
#define ICONNAME_HB_FILE_VALID		"hb-file-valid"
#define ICONNAME_HB_FILE_INVALID	"hb-file-invalid"

#define ICONNAME_HB_BUTTON_COLLAPSE	"btn-collapse-symbolic"
#define ICONNAME_HB_BUTTON_EXPAND	"btn-expand-symbolic"
#define ICONNAME_HB_BUTTON_SPLIT	"btn-split"

#define ICONNAME_HB_OPE_AUTO        "hb-ope-auto"   //? 
#define ICONNAME_HB_OPE_BUDGET      "hb-ope-budget" //? 
#define ICONNAME_HB_OPE_ADD         "hb-ope-add"	//? "edit-add"
#define ICONNAME_HB_OPE_HERIT       "hb-ope-herit"  //? "edit-clone"
#define ICONNAME_HB_OPE_EDIT        "hb-ope-edit"   //
#define ICONNAME_HB_OPE_MULTIEDIT   "hb-ope-multiedit"   //
#define ICONNAME_HB_OPE_DELETE      "hb-ope-delete" //? "edit-delete"
#define ICONNAME_CONVERT			"hb-ope-convert"
#define ICONNAME_HB_ASSIGN_RUN      "hb-assign-run"

#define ICONNAME_HB_OPE_VALID       "hb-ope-valid"  // obsolete ?
#define ICONNAME_HB_OPE_CLEARED     "hb-ope-cleared"
#define ICONNAME_HB_OPE_RECONCILED  "hb-ope-reconciled"
#define ICONNAME_HB_OPE_REMIND      "hb-ope-remind"



/*
** Global application datas
*/
struct HomeBank
{
	// hbfile storage
	GHashTable		*h_cur;			//currencies
	GHashTable		*h_acc;			//accounts
	GHashTable		*h_pay;			//payees
	GHashTable		*h_cat;			//categories
	GHashTable		*h_tag;			//tags
	GHashTable		*h_rul;			//assign rules

	GHashTable		*h_memo;		//memo/description

	GList			*arc_list;		//archives

	//#1419304 we keep the deleted txn to a stack trash
	GTrashStack		*txn_stk;

	// hbfile (saved properties)
	gchar			*owner;
	gshort			auto_smode;
	gshort			auto_weekday;
	gshort			auto_nbdays;

	guint32			vehicle_category;
	guint32			kcur;			// base currency

	// hbfile (unsaved properties)
	guint			changes_count;
	gboolean		hbfile_is_new;
	gboolean		hbfile_is_bak;
	gchar			*xhb_filepath;
	gboolean		xhb_hasbak;		//file has backup (*.xhb~) used for revert menu sensitivity

	// really global stuffs
	gboolean		first_run;
	guint32			today;			//today's date
	gint			define_off;		//>0 when a stat, account window is opened
	gboolean		minor;

	GtkWidget		*mainwindow;	//should be global to access attached window data
	GtkIconTheme	*icontheme;
	//GdkPixbuf		*lst_pixbuf[NUM_LST_PIXBUF];
	//gint			lst_pixbuf_maxwidth;

};

gint homebank_alienfile_recognize(gchar *filename);
gchar *homebank_filepath_with_extention(gchar *path, gchar *extension);
gchar *homebank_filename_without_extention(gchar *path);
void homebank_file_ensure_xhb(gchar *filename);
void homebank_backup_current_file(void);
gboolean homebank_util_url_show (const gchar *url);
gchar *homebank_lastopenedfiles_load(void);
gboolean homebank_lastopenedfiles_save(void);


void homebank_window_set_icon_from_file(GtkWindow *window, gchar *filename);

const gchar *homebank_app_get_config_dir (void);
const gchar *homebank_app_get_images_dir (void);
const gchar *homebank_app_get_pixmaps_dir (void);
const gchar *homebank_app_get_locale_dir (void);
const gchar *homebank_app_get_help_dir (void);
const gchar *homebank_app_get_datas_dir (void);
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
