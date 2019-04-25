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

#include "dsp-mainwindow.h"

#include "ext.h"

#include "list-account.h"

#include "hub-account.h"
#include "hub-scheduled.h"
#include "hub-spending.h"
#include "hub-transaction.h"

#include "dsp-account.h"
#include "ui-assist-import.h"
#include "ui-assist-start.h"
#include "ui-account.h"
#include "ui-currency.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-archive.h"
#include "ui-assign.h"
#include "ui-budget.h"
#include "ui-pref.h"
#include "ui-hbfile.h"
#include "ui-transaction.h"
#include "ui-tag.h"

#include "rep-balance.h"
#include "rep-budget.h"
#include "rep-stats.h"
#include "rep-time.h"
#include "rep-vehicle.h"

#include "gtk-chart.h"

//old url prior 2019
//#define HOMEBANK_URL_HELP           "http://homebank.free.fr/help/"
//#define HOMEBANK_URL_HELP_ONLINE    "https://launchpad.net/homebank/+addquestion"
//#define HOMEBANK_URL_HELP_PROBLEM   "https://launchpad.net/homebank/+filebug"
//#define HOMEBANK_URL_HELP_TRANSLATE "https://launchpad.net/homebank/+translations"

#define HOMEBANK_URL_HELP           "index.html"
#define HOMEBANK_URL_HELP_ONLINE    "http://homebank.free.fr/support.php"
#define HOMEBANK_URL_HELP_UPDATES   "http://homebank.free.fr/downloads.php"
#define HOMEBANK_URL_HELP_PROBLEM   "http://homebank.free.fr/development.php#bug"
#define HOMEBANK_URL_HELP_TRANSLATE "http://homebank.free.fr/development.php#translate"


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
extern gchar *homebank_pixmaps_dir;


/* our functions prototype */
static void ui_mainwindow_action_new(void);
static void ui_mainwindow_action_open(void);
static void ui_mainwindow_action_save(void);
static void ui_mainwindow_action_saveas(void);
static void ui_mainwindow_action_revert(void);
static void ui_mainwindow_action_openbak(void);
static void ui_mainwindow_action_properties(void);
static void ui_mainwindow_action_close(void);
static void ui_mainwindow_action_quit(void);

static void ui_mainwindow_action_defcurrency(void);
static void ui_mainwindow_action_defaccount(void);
static void ui_mainwindow_action_defpayee(void);
static void ui_mainwindow_action_defcategory(void);
static void ui_mainwindow_action_defarchive(void);
static void ui_mainwindow_action_defbudget(void);
static void ui_mainwindow_action_defassign(void);
static void ui_mainwindow_action_deftag(void);
static void ui_mainwindow_action_preferences(void);

static void ui_mainwindow_action_toggle_toolbar(GtkToggleAction *action);
static void ui_mainwindow_action_toggle_upcoming(GtkToggleAction *action);
static void ui_mainwindow_action_toggle_topspending(GtkToggleAction *action);
static void ui_mainwindow_action_toggle_minor(GtkToggleAction *action);

static void ui_mainwindow_action_showtransactions(void);
static void ui_mainwindow_action_showalltransactions(void);

static void ui_mainwindow_action_addtransactions(void);
static void ui_mainwindow_action_checkscheduled(void);

static void ui_mainwindow_action_statistic(void);
static void ui_mainwindow_action_trendtime(void);
static void ui_mainwindow_action_budget(void);
static void ui_mainwindow_action_balance(void);
static void ui_mainwindow_action_vehiclecost(void);

static void ui_mainwindow_action_import(GtkAction *action);
static void ui_mainwindow_action_export(void);
static void ui_mainwindow_action_anonymize(void);
static void ui_mainwindow_action_file_statistics(void);

static void ui_mainwindow_action_pluginprefs(void);

static void ui_mainwindow_action_help(void);
void ui_mainwindow_action_help_welcome(void);
static void ui_mainwindow_action_help_online(void);
static void ui_mainwindow_action_help_updates(void);
static void ui_mainwindow_action_help_releasenotes(void);
static void ui_mainwindow_action_help_translate(void);
static void ui_mainwindow_action_help_problem(void);
static void ui_mainwindow_action_about(void);


static GtkWidget *ui_mainwindow_create_recent_chooser_menu (GtkRecentManager *manager);

void ui_mainwindow_open(GtkWidget *widget, gpointer user_data);

void ui_mainwindow_save(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_revert(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_action(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_toggle_minor(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_clear(GtkWidget *widget, gpointer user_data);

void ui_mainwindow_update(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_addtransactions(GtkWidget *widget, gpointer user_data);
void ui_mainwindow_recent_add (struct hbfile_data *data, const gchar *path);

void ui_mainwindow_recent_add (struct hbfile_data *data, const gchar *path);

static void ui_mainwindow_showprefs(gint page);

static GtkActionEntry entries[] = {

  /* name, icon-name, label */

  { "FileMenu"   , NULL, N_("_File"), NULL, NULL, NULL },
  //{ "ImportMenu" , NULL, N_("_Import"), NULL, NULL, NULL },
  { "RecentMenu" , NULL, N_("Open _Recent"), NULL, NULL, NULL },
  { "EditMenu"   , NULL, N_("_Edit"), NULL, NULL, NULL },
  { "ViewMenu"   , NULL, N_("_View"), NULL, NULL, NULL },
  { "ManageMenu" , NULL, N_("_Manage"), NULL, NULL, NULL },
  { "TxnMenu"    , NULL, N_("_Transactions"), NULL, NULL, NULL },
  { "ReportMenu" , NULL, N_("_Reports"), NULL, NULL, NULL  },
  { "ToolsMenu"  , NULL, N_("_Tools"), NULL, NULL, NULL },
  { "PluginMenu" , NULL, N_("_Plugins"), NULL, NULL, NULL },
  { "HelpMenu"   , NULL, N_("_Help"), NULL, NULL, NULL },

//  { "Import"       , NULL, N_("Import") },
//  { "Export"       , NULL, N_("Export to") },
	/* name, icon-name, label, accelerator, tooltip */

  /* FileMenu */
  { "New"        , ICONNAME_HB_FILE_NEW    , N_("_New")          , "<control>N", N_("Create a new file"),    G_CALLBACK (ui_mainwindow_action_new) },
  { "Open"       , ICONNAME_HB_FILE_OPEN   , N_("_Open...")      , "<control>O", N_("Open a file"),    G_CALLBACK (ui_mainwindow_action_open) },
  { "Save"       , ICONNAME_HB_FILE_SAVE   , N_("_Save")         , "<control>S", N_("Save the current file"),    G_CALLBACK (ui_mainwindow_action_save) },
  { "SaveAs"     , ICONNAME_SAVE_AS        , N_("Save _As...")    , "<shift><control>S", N_("Save the current file with a different name"),    G_CALLBACK (ui_mainwindow_action_saveas) },

  { "Revert"     , ICONNAME_REVERT         , N_("Revert")        , NULL, N_("Revert to a saved version of this file"),    G_CALLBACK (ui_mainwindow_action_revert) },
  { "OpenBak"    , NULL                   , N_("Restore backup") , NULL, N_("Restore from a backup file"),    G_CALLBACK (ui_mainwindow_action_openbak) },

  { "Properties" , ICONNAME_PROPERTIES     , N_("Properties..."), NULL, N_("Configure the file"),    G_CALLBACK (ui_mainwindow_action_properties) },
  { "Close"      , ICONNAME_CLOSE          , N_("_Close")        , "<control>W", N_("Close the current file"),    G_CALLBACK (ui_mainwindow_action_close) },
  { "Quit"       , ICONNAME_QUIT           , N_("_Quit")         , "<control>Q", N_("Quit HomeBank"),    G_CALLBACK (ui_mainwindow_action_quit) },

  /* Exchange */
  { "Import" , ICONNAME_HB_FILE_IMPORT  , N_("Import...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (ui_mainwindow_action_import) },
  //{ "ImportQIF" , ICONNAME_HB_FILE_IMPORT  , N_("QIF file...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (ui_mainwindow_action_import) },
  //{ "ImportOFX" , ICONNAME_HB_FILE_IMPORT  , N_("OFX/QFX file...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (ui_mainwindow_action_import) },
  //{ "ImportCSV" , ICONNAME_HB_FILE_IMPORT  , N_("CSV file...")     , NULL, N_("Open the import assistant"),    G_CALLBACK (ui_mainwindow_action_import) },

  { "ExportQIF" , ICONNAME_HB_FILE_EXPORT  , N_("Export as QIF...")     , NULL, N_("Export all account in a QIF file"),    G_CALLBACK (ui_mainwindow_action_export) },

  /* EditMenu */
  { "Preferences", ICONNAME_PREFERENCES    , N_("Preferences..."), NULL,    N_("Configure HomeBank"),    G_CALLBACK (ui_mainwindow_action_preferences) },

  /* ManageMenu */
  { "Currency"   , ICONNAME_HB_CURRENCY    , N_("Currencies...") , NULL,    N_("Configure the currencies"), G_CALLBACK (ui_mainwindow_action_defcurrency) },
  { "Account"    , ICONNAME_HB_ACCOUNT     , N_("Acc_ounts...")  , NULL,    N_("Configure the accounts"), G_CALLBACK (ui_mainwindow_action_defaccount) },
  { "Payee"      , ICONNAME_HB_PAYEE       , N_("_Payees...")    , NULL,    N_("Configure the payees"),    G_CALLBACK (ui_mainwindow_action_defpayee) },
  { "Category"   , ICONNAME_HB_CATEGORY    , N_("Categories...") , NULL,    N_("Configure the categories"),    G_CALLBACK (ui_mainwindow_action_defcategory) },
  { "Archive"    , ICONNAME_HB_ARCHIVE     , N_("Scheduled/Template...")  , NULL,    N_("Configure the scheduled/template transactions"),    G_CALLBACK (ui_mainwindow_action_defarchive) },
  { "Budget"     , ICONNAME_HB_BUDGET      , N_("Budget...")     , NULL,    N_("Configure the budget"),    G_CALLBACK (ui_mainwindow_action_defbudget) },
  { "Assign"     , ICONNAME_HB_ASSIGN      , N_("Assignments..."), NULL,    N_("Configure the automatic assignments"),    G_CALLBACK (ui_mainwindow_action_defassign) },
  { "Tag"        , NULL                    , N_("Tags..."),        NULL,    N_("Configure the tags"),    G_CALLBACK (ui_mainwindow_action_deftag) },

  /* TxnMenu */
  { "AddTxn"      , ICONNAME_HB_OPE_ADD    , N_("Add...")              , NULL, N_("Add transactions"),    G_CALLBACK (ui_mainwindow_action_addtransactions) },
  { "ShowTxn"     , ICONNAME_HB_OPE_SHOW   , N_("Show...")             , NULL, N_("Shows selected account transactions"),    G_CALLBACK (ui_mainwindow_action_showtransactions) },
  { "ShowAllTxn"  , ICONNAME_HB_OPE_SHOW   , N_("Show all...")         , NULL, N_("Shows all account transactions"),    G_CALLBACK (ui_mainwindow_action_showalltransactions) },
  { "Scheduler"   , NULL                   , N_("Set scheduler...")    , NULL, N_("Configure the transaction scheduler"),    G_CALLBACK (ui_mainwindow_action_properties) },
  { "AddScheduled", NULL                   , N_("Post scheduled"), NULL, N_("Post pending scheduled transactions"),    G_CALLBACK (ui_mainwindow_action_checkscheduled) },

  /* ReportMenu */
  { "RStatistics" , ICONNAME_HB_REP_STATS  , N_("_Statistics...") , NULL,    N_("Open the Statistics report"),    G_CALLBACK (ui_mainwindow_action_statistic) },
  { "RTrendTime"  , ICONNAME_HB_REP_TIME   , N_("_Trend Time...") , NULL,    N_("Open the Trend Time report"),    G_CALLBACK (ui_mainwindow_action_trendtime) },
  { "RBudget"     , ICONNAME_HB_REP_BUDGET , N_("B_udget...")     , NULL,    N_("Open the Budget report"),    G_CALLBACK (ui_mainwindow_action_budget) },
  { "RBalance"    , ICONNAME_HB_REP_BALANCE, N_("Balance...")  , NULL,    N_("Open the Balance report"),    G_CALLBACK (ui_mainwindow_action_balance) },
  { "RVehiculeCost", ICONNAME_HB_REP_CAR   , N_("_Vehicle cost...")   , NULL,    N_("Open the Vehicle cost report"),    G_CALLBACK (ui_mainwindow_action_vehiclecost) },

  /* Tools */
  { "Welcome"     , NULL              , N_("Show welcome dialog...")  , NULL, NULL, G_CALLBACK (ui_mainwindow_action_help_welcome) },
  { "FileStats"   , NULL              , N_("File statistics...")  , NULL, NULL,    G_CALLBACK (ui_mainwindow_action_file_statistics) },
  { "Anonymize"   , NULL              , N_("Anonymize...")  , NULL, NULL,    G_CALLBACK (ui_mainwindow_action_anonymize) },

  /* Plugins */
  { "PluginPreferences", "prf-plugins", N_("_Plugins..."), "<control>U", N_("Configure plugin preferences"), G_CALLBACK(ui_mainwindow_action_pluginprefs) },

  /* HelpMenu */
  { "Contents"    , ICONNAME_HELP     , N_("_Contents")                    , "F1", N_("Documentation about HomeBank"), G_CALLBACK (ui_mainwindow_action_help) },
  { "Online"      , "lpi-help"        , N_("Get Help Online...")           , NULL, N_("Connect to the LaunchPad website for online help"), G_CALLBACK (ui_mainwindow_action_help_online) },

  { "Updates"     , NULL              , N_("Check for updates...")         , NULL, N_("Visit HomeBank website to check for update"), G_CALLBACK (ui_mainwindow_action_help_updates) },
  { "ReleaseNotes", NULL              , N_("Release Notes")                , NULL, N_("Display the release notes"), G_CALLBACK (ui_mainwindow_action_help_releasenotes) },
  { "Problem"     , "lpi-bug"         , N_("Report a Problem...")          , NULL, N_("Connect to the LaunchPad website to help fix problems"), G_CALLBACK (ui_mainwindow_action_help_problem) },
  { "Translate"   , "lpi-translate"   , N_("Translate this Application..."), NULL, N_("Connect to the LaunchPad website to help translate this application"), G_CALLBACK (ui_mainwindow_action_help_translate) },

  { "About"       , ICONNAME_ABOUT      , N_("_About")     , NULL, N_("About HomeBank")      ,G_CALLBACK (ui_mainwindow_action_about) },

};
static guint n_entries = G_N_ELEMENTS (entries);


static GtkToggleActionEntry toggle_entries[] = {
/*  name         , icon-name, label, accelerator, tooltip, callback, is_active */
  { "Toolbar"    , NULL                 , N_("_Toolbar")  , NULL,    NULL,    G_CALLBACK (ui_mainwindow_action_toggle_toolbar), TRUE },
  { "Spending"   , NULL                 , N_("_Top spending") , NULL,    NULL,    G_CALLBACK (ui_mainwindow_action_toggle_topspending), TRUE },
  { "BottomLists", NULL                 , N_("_Bottom Lists") , NULL,    NULL,    G_CALLBACK (ui_mainwindow_action_toggle_upcoming), TRUE },
  { "AsMinor"    , NULL                 , N_("Euro minor"), "<control>M",    NULL,    G_CALLBACK (ui_mainwindow_action_toggle_minor), FALSE },
};

static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);


static const gchar *ui_info =
"<ui>"

"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='RecentMenu'/>"
"        <separator/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"        <separator/>"
"      <menuitem action='Import'/>"
/*"        <menu action='ImportMenu'>"
"          <menuitem action='ImportQIF'/>"
"          <menuitem action='ImportOFX'/>"
"          <menuitem action='ImportCSV'/>"
"        </menu>"*/
"      <menuitem action='ExportQIF'/>"
//	 future: print to come here
"        <separator/>"
"      <menuitem action='Revert'/>"
"      <menuitem action='OpenBak'/>"
"        <separator/>"
"      <menuitem action='Properties'/>"
"        <separator/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='Toolbar'/>"
"        <separator/>"
"      <menuitem action='Spending'/>"
"      <menuitem action='BottomLists'/>"
"        <separator/>"
"      <menuitem action='AsMinor'/>"
"    </menu>"
"    <menu action='ManageMenu'>"
"      <menuitem action='Account'/>"
"      <menuitem action='Payee'/>"
"      <menuitem action='Category'/>"
"      <menuitem action='Archive'/>"
"      <menuitem action='Budget'/>"
"      <menuitem action='Assign'/>"
"      <menuitem action='Currency'/>"
"      <menuitem action='Tag'/>"
"    </menu>"
"    <menu action='TxnMenu'>"
"      <menuitem action='AddTxn'/>"
"      <menuitem action='ShowTxn'/>"
"      <menuitem action='ShowAllTxn'/>"
"        <separator/>"
"      <menuitem action='Scheduler'/>"
"      <menuitem action='AddScheduled'/>"
"    </menu>"
"    <menu action='ReportMenu'>"
"      <menuitem action='RStatistics'/>"
"      <menuitem action='RTrendTime'/>"
"      <menuitem action='RBalance'/>"
"      <menuitem action='RBudget'/>"
"      <menuitem action='RVehiculeCost'/>"
"    </menu>"
"    <menu action='ToolsMenu'>"
"      <menuitem action='Welcome'/>"
"      <menuitem action='FileStats'/>"
"        <separator/>"
"      <menuitem action='Anonymize'/>"
"    </menu>"
"    <menu action='PluginMenu'>"
"      <separator/>"
"      <menuitem action='PluginPreferences'/>"
"      <separator/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <menuitem action='Online'/>"
"        <separator/>"
"      <menuitem action='Updates'/>"
"      <menuitem action='ReleaseNotes'/>"
"      <menuitem action='Problem'/>"
"      <menuitem action='Translate'/>"
"        <separator/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"

"  <toolbar  name='ToolBar'>"
"    <toolitem action='New'/>"
//	  here Open + recent is coded
"    <toolitem action='Save'/>"
"      <separator/>"
"    <toolitem action='Account'/>"
"    <toolitem action='Payee'/>"
"    <toolitem action='Category'/>"
"    <toolitem action='Archive'/>"
"    <toolitem action='Budget'/>"
"    <toolitem action='Assign'/>"
"    <toolitem action='Currency'/>"
"      <separator/>"
"    <toolitem action='ShowTxn'/>"
"    <toolitem action='AddTxn'/>"
"      <separator/>"
"    <toolitem action='RStatistics'/>"
"    <toolitem action='RTrendTime'/>"
"    <toolitem action='RBalance'/>"
"    <toolitem action='RBudget'/>"
"    <toolitem action='RVehiculeCost'/>"
"      <separator/>"
"  </toolbar>"

"</ui>";



/* TODO: a bouger */


/*
**
*/
void ui_mainwindow_revert(GtkWidget *widget, gpointer user_data)
{
//struct hbfile_data *data;
gchar *basename;
gchar *title;
gchar *secondtext;
gint result;

	DB( g_print("\n[ui-mainwindow] revert\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	basename = g_path_get_basename(GLOBALS->xhb_filepath);
	title = g_strdup_printf (
		_("Revert unsaved changes to file '%s'?"), basename);

	secondtext =
		_("- Changes made to the file will be permanently lost\n"
		"- File will be reloaded from the last save (.xhb~)");

	result = ui_dialog_msg_confirm_alert(
			GTK_WINDOW(GLOBALS->mainwindow),
			title,
			secondtext,
			_("_Revert")
		);

	g_free(title);
	g_free(basename);

	if( result == GTK_RESPONSE_OK )
	{
		DB( g_print(" - should revert\n") );

		hbfile_change_filepath(hb_filename_new_with_extension(GLOBALS->xhb_filepath, "xhb~"));
		ui_mainwindow_open_internal(widget, NULL);
		hbfile_change_filepath(hb_filename_new_with_extension(GLOBALS->xhb_filepath, "xhb"));
	}

}


static void
activate_url (GtkAboutDialog *about,
	      const gchar    *link,
	      gpointer        data)
{
	DB( g_print("activate url %s\n", link) );

	homebank_util_url_show (link);
}

static void hbfile_about(void)
{
GtkWidget *dialog;
GdkPixbuf *pixbuf;
gchar *pathfilename;
gchar *version;

  static const gchar *artists[] = {
    "Maxime DOYEN",
    NULL
  };

  static const gchar *authors[] = {
    "Lead developer:\n" \
    "Maxime DOYEN",
    "\nContributors:\n" \
    "Charles MCGARVEY (Plugin system, Perl support)\n" \
    "Ga\xc3\xabtan LORIDANT (Maths formulas for charts)\n",
    NULL
  };

/*
  const gchar *documenters[] = {
    "Maxime DOYEN",
    NULL
  };
*/

	static const gchar *copyright = "Copyright \xc2\xa9 1995-2019 - Maxime DOYEN";



	version = g_strdup_printf (PACKAGE_VERSION "\n<small>Running against GTK+ %d.%d.%d</small>",
                                                     gtk_get_major_version (),
                                                     gtk_get_minor_version (),
                                                     gtk_get_micro_version ());

	dialog = gtk_about_dialog_new();

	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(GLOBALS->mainwindow));
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(dialog), g_get_application_name ());
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), version);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), copyright);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), _("Free, easy, personal accounting for everyone"));
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_GPL_2_0);

	//gtk_about_dialog_set_wrap_license(GTK_ABOUT_DIALOG(dialog), );
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://homebank.free.fr");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), "Visit the HomeBank website");

	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "homebank");

	pathfilename = g_build_filename(homebank_app_get_images_dir(), "splash.png", NULL);
	pixbuf = gdk_pixbuf_new_from_file(pathfilename, NULL);
	g_free(pathfilename);

	if( pixbuf )
	{
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
		g_object_unref (pixbuf);
	}

	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(dialog), artists);
	//gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(dialog), );
	//gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog), );

	g_signal_connect (dialog, "activate-link", G_CALLBACK (activate_url), NULL);

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	g_free(version);

}


/* hbfile action functions -------------------- */
static void ui_mainwindow_action_new(void)
{
GtkWidget *widget = GLOBALS->mainwindow;

	if( ui_dialog_msg_savechanges(widget,NULL) == TRUE )
	{
		//clear all, and init GLOBALS->xhb_filepath to default
		ui_mainwindow_clear(widget, GINT_TO_POINTER(TRUE)); // GPOINTER_TO_INT(
		ui_mainwindow_update(widget, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_REFRESHALL));

		ui_start_assistant();
		//ui_hub_account_populate(GLOBALS->mainwindow, NULL);
		//ui_hub_scheduled_populate(GLOBALS->mainwindow, NULL);
		//ui_hub_spending_populate(GLOBALS->mainwindow, NULL);
	}
}

static void ui_mainwindow_action_open(void)
{
	ui_mainwindow_open(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
}

static void ui_mainwindow_action_openbak(void)
{
	ui_mainwindow_open(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
}

static void ui_mainwindow_action_save(void)
{
	ui_mainwindow_save(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
}

static void ui_mainwindow_action_saveas(void)
{
	ui_mainwindow_save(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
}

static void ui_mainwindow_action_revert(void)
{
	ui_mainwindow_revert(GLOBALS->mainwindow, NULL);
}

static void ui_mainwindow_action_close(void)
{
GtkWidget *widget = GLOBALS->mainwindow;

	if( ui_dialog_msg_savechanges(widget,NULL) == TRUE )
	{
		//clear all, and init GLOBALS->xhb_filepath to default
		ui_mainwindow_clear(widget, GINT_TO_POINTER(TRUE));
		ui_mainwindow_update(widget, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_REFRESHALL));
	}

}


static void ui_mainwindow_action_quit(void)
{
gboolean result;

	//emulate the wm close button
	g_signal_emit_by_name(GLOBALS->mainwindow, "delete-event", NULL, &result);
}


static void ui_mainwindow_action_file_statistics(void)
{
	ui_dialog_file_statistics();
}


static void ui_mainwindow_action_pluginprefs(void)
{
	ui_mainwindow_showprefs(PREF_PLUGINS);
}


static void ui_mainwindow_action_properties(void)
{
	create_defhbfile_dialog();
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_REFRESHALL));
}

static void ui_mainwindow_action_anonymize(void)
{
gint result;
gchar *title;
gchar *secondtext;

	title = _("Are you sure you want to anonymize the file?");

	secondtext =
		_("Proceeding will anonymize any text, \n"
		"like 'account x', 'payee y', 'memo z', ...");

	result = ui_dialog_msg_confirm_alert(
			GTK_WINDOW(GLOBALS->mainwindow),
			title,
			secondtext,
			_("_Anonymize")
		);

	//#1707201
	//if( result == GTK_RESPONSE_CANCEL )
	//	return;
	if( result == GTK_RESPONSE_OK )
	{
		hbfile_anonymize();
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_REFRESHALL));
	}
}


static void ui_mainwindow_action_defcurrency(void)
{
	ui_cur_manage_dialog();
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_REFRESHALL));
}


static void ui_mainwindow_action_defaccount(void)
{
	ui_acc_manage_dialog();

	//our global list has changed, so update the treeview
	//todo: optimize this, should not call compute balance here
	account_compute_balances ();
	ui_hub_account_populate(GLOBALS->mainwindow, NULL);

	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE));
}

static void ui_mainwindow_action_defpayee(void)
{
	ui_pay_manage_dialog();
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}

static void ui_mainwindow_action_defcategory(void)
{
	ui_cat_manage_dialog();
	//todo:why refresh upcoming here??
	//ui_mainwindow_populate_upcoming(GLOBALS->mainwindow, NULL);
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_REFRESHALL));
}


//TODO: not ideal to do this
void ui_mainwindow_defarchive(Archive *arc)
{
struct hbfile_data *data;
GtkTreeModel *model;

	data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	// upcoming list have direct pointer to the arc (which may change during edit)
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	ui_arc_manage_dialog(arc);

	ui_hub_scheduled_populate(GLOBALS->mainwindow, NULL);

	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void ui_mainwindow_action_defarchive(void)
{
	ui_mainwindow_defarchive(NULL);
}


static void ui_mainwindow_action_defbudget(void)
{
	ui_bud_manage_dialog();
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void ui_mainwindow_action_defassign(void)
{

	ui_asg_manage_dialog();

	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void ui_mainwindow_action_deftag(void)
{

	ui_tag_manage_dialog();

	//ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE));
}


static void ui_mainwindow_action_preferences(void)
{
	ui_mainwindow_showprefs(PREF_GENERAL);
}

static void ui_mainwindow_showprefs(gint page)
{
struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	defpref_dialog_new(page);
	if(!PREFS->euro_active)
	{
	GtkToggleAction *action = (GtkToggleAction *)gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/AsMinor");

		gtk_toggle_action_set_active(action, FALSE);
		ui_mainwindow_action_toggle_minor(action);
	}
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL+UF_REFRESHALL));
}

/* display action */

static void ui_mainwindow_action_toggle_toolbar(GtkToggleAction *action)
{
//struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_toolbar = gtk_toggle_action_get_active(action);
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void ui_mainwindow_action_toggle_upcoming(GtkToggleAction *action)
{
//struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_upcoming = gtk_toggle_action_get_active(action);
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void ui_mainwindow_action_toggle_topspending(GtkToggleAction *action)
{
//struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	PREFS->wal_spending = gtk_toggle_action_get_active(action);
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_VISUAL));
}

static void ui_mainwindow_action_toggle_minor(GtkToggleAction *action)
{
struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	GLOBALS->minor = gtk_toggle_action_get_active(action);

	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_upc));

	// top spending
	gtk_chart_show_minor(GTK_CHART(data->RE_pie), GLOBALS->minor);

	ui_hub_spending_update(data->window, data);

}

static void ui_mainwindow_action_showtransactions(void)
{
struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");
GtkWidget *window;

	//todo:change this
	if( data->acc )
	{
		if( data->acc->window == NULL )
		{
			window = register_panel_window_new(data->acc);
			register_panel_window_init(window, NULL);
		}
		else
		{
			if(GTK_IS_WINDOW(data->acc->window))
				gtk_window_present(GTK_WINDOW(data->acc->window));

		}
	}
}


static void ui_mainwindow_action_showalltransactions(void)
{
GtkWidget *window;

	if( GLOBALS->alltxnwindow == NULL )
	{
		window = register_panel_window_new(NULL);
		register_panel_window_init(window, NULL);
	}
	else
	{
		if(GTK_IS_WINDOW(GLOBALS->alltxnwindow))
			gtk_window_present(GTK_WINDOW(GLOBALS->alltxnwindow));
	}

}


static void ui_mainwindow_action_addtransactions(void)
{
	ui_mainwindow_addtransactions(GLOBALS->mainwindow, NULL);
}

static void ui_mainwindow_action_checkscheduled(void)
{
	ui_hub_scheduled_postall(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));
}

static void ui_mainwindow_action_statistic(void)
{
	ui_repdist_window_new();
}

static void ui_mainwindow_action_trendtime(void)
{
struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	ui_reptime_window_new(data->acc != NULL ? data->acc->key : 0);
}

static void ui_mainwindow_action_budget(void)
{
	repbudget_window_new();
}

static void ui_mainwindow_action_balance(void)
{
struct hbfile_data *data = g_object_get_data(G_OBJECT(GLOBALS->mainwindow), "inst_data");

	repbalance_window_new(data->acc != NULL ? data->acc->key : 0);
}

static void ui_mainwindow_action_vehiclecost(void)
{
	repcost_window_new();
}

static void ui_mainwindow_action_import(GtkAction *action)
{
/*const gchar *name;
gint filetype = FILETYPE_UNKNOWN;

	name = gtk_action_get_name(action);

	if( g_str_has_suffix (name, "QIF"))
	   filetype= FILETYPE_QIF;
	else
	if( g_str_has_suffix (name, "OFX"))
	   filetype= FILETYPE_OFX;
	else
	if( g_str_has_suffix (name, "CSV"))
	   filetype= FILETYPE_CSV_HB;*/

	//DB( g_print("action %s type=%d\n", name, filetype) );

	ui_import_assistant_new(NULL);

}


static void ui_mainwindow_action_about(void)
{
	hbfile_about();


}


static void ui_mainwindow_action_export(void)
{
gchar *filename;

	if( ui_file_chooser_qif(NULL, &filename) == TRUE )
	{
		hb_export_qif_account_all(filename);
		g_free( filename );
	}
}


static void ui_mainwindow_action_help(void)
{
gchar *link;

    link = g_build_filename("file:///", homebank_app_get_help_dir(), HOMEBANK_URL_HELP, NULL );
	homebank_util_url_show (link);
    g_free(link);
}


static void ui_mainwindow_action_help_releasenotes(void)
{
gchar *link;

	#ifdef G_OS_WIN32
    	link = g_build_filename("file:///", homebank_app_get_datas_dir(), "ChangeLog.txt", NULL );
	#else
		link = g_build_filename("file:///", homebank_app_get_datas_dir(), "ChangeLog", NULL );
	#endif
	homebank_util_url_show (link);
    g_free(link);
}


//todo: move this to a ui-assist-welcome.c

static void ui_mainwindow_action_help_welcome1 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 1);
}

static void ui_mainwindow_action_help_welcome2 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 2);
}

static void ui_mainwindow_action_help_welcome3 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 3);
}

static void ui_mainwindow_action_help_welcome4 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 4);
}

static void ui_mainwindow_action_help_welcome5 (GtkButton *button, gpointer user_data)
{
	gtk_dialog_response (GTK_DIALOG(user_data), 5);
}

void ui_mainwindow_action_help_welcome(void)
{
GtkWidget *dialog, *content_area;
GtkWidget *mainvbox, *widget, *label;

	dialog = gtk_dialog_new_with_buttons (_("Welcome to HomeBank"),
			GTK_WINDOW(GLOBALS->mainwindow),
			0,
			_("_Close"),
			GTK_RESPONSE_ACCEPT,
			NULL);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (content_area), mainvbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), SPACING_MEDIUM);

	label = make_label (_("HomeBank"), 0, 0);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

	label = make_label (_("Free, easy, personal accounting for everyone"), 0, 0);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

	widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start (GTK_BOX (content_area), widget, FALSE, FALSE, 0);

	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), SPACING_MEDIUM);

	label = make_label (_("What do you want to do:"), 0, 0);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, 0);

	widget = gtk_button_new_with_mnemonic(_("Read HomeBank _Manual"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (ui_mainwindow_action_help_welcome1), dialog);

	widget = gtk_button_new_with_mnemonic(_("Configure _preferences"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (ui_mainwindow_action_help_welcome2), dialog);

	widget = gtk_button_new_with_mnemonic(_("Create a _new file"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (ui_mainwindow_action_help_welcome3), dialog);

	widget = gtk_button_new_with_mnemonic(_("_Open an existing file"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (ui_mainwindow_action_help_welcome4), dialog);

	widget = gtk_button_new_with_mnemonic(_("Open the _example file"));
	gtk_box_pack_start (GTK_BOX (mainvbox), widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "clicked", G_CALLBACK (ui_mainwindow_action_help_welcome5), dialog);

	//connect all our signals
	g_signal_connect (dialog, "destroy", G_CALLBACK (gtk_widget_destroyed), &dialog);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	// cleanup and destroy
	gtk_widget_destroy (dialog);

	// do appropriate action
	switch(result)
	{
		case 1:
			ui_mainwindow_action_help();
			break;
		case 2:
			ui_mainwindow_action_preferences();
			break;
		case 3:
			ui_mainwindow_action_new();
			break;
		case 4:
			ui_mainwindow_action_open();
			break;
		case 5:
			hbfile_change_filepath(g_build_filename(homebank_app_get_datas_dir(), "example.xhb", NULL));
			ui_mainwindow_open_internal(GLOBALS->mainwindow, NULL);
			break;
	}

}


static void ui_mainwindow_action_help_updates(void)
{
const gchar *link = HOMEBANK_URL_HELP_UPDATES;

	homebank_util_url_show (link);
}


static void ui_mainwindow_action_help_online(void)
{
const gchar *link = HOMEBANK_URL_HELP_ONLINE;

	homebank_util_url_show (link);
}


static void ui_mainwindow_action_help_translate(void)
{
const gchar *link = HOMEBANK_URL_HELP_TRANSLATE;

	homebank_util_url_show (link);
}


static void ui_mainwindow_action_help_problem(void)
{
const gchar *link = HOMEBANK_URL_HELP_PROBLEM;

	homebank_util_url_show (link);
}


/* hbfile functions -------------------- */


/*
**
*/
static void ui_mainwindow_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_mainwindow_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(UF_SENSITIVE));
}


static void ui_mainwindow_close_openbooks(void)
{
GList *lacc, *elt;

	DB( g_print("\n[ui-mainwindow] close openbooks\n") );

	lacc = elt = g_hash_table_get_values(GLOBALS->h_acc);
	while (elt != NULL)
	{
	Account *item = elt->data;

		if(item->window)
		{
			gtk_widget_destroy(GTK_WIDGET(item->window));
			item->window = NULL;
		}

		elt = g_list_next(elt);
	}
	g_list_free(lacc);

}



/*
**
*/
void ui_mainwindow_clear(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
gboolean file_clear = GPOINTER_TO_INT(user_data);

	DB( g_print("\n[ui-mainwindow] clear\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// Close opened account window
	// Clear TreeView
	ui_mainwindow_close_openbooks();
	gtk_tree_store_clear(GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_upc))));
	gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_top))));

	data->showall = FALSE;
	ui_hub_account_setup(data);

	hbfile_cleanup(file_clear);
	hbfile_setup(file_clear);

}


/*
** add some transactions directly
*/
void ui_mainwindow_addtransactions(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
GtkWidget *window;
gint result = 1;
guint32 date;
gint account, count;

	DB( g_print("\n[ui-mainwindow] add transactions\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	/* init the transaction */
	date = homebank_app_date_get_julian();

	//#1656531
	account = 0;
	if(data->acc != NULL)
		account = data->acc->key;

	window = create_deftransaction_window(GTK_WINDOW(data->window), TRANSACTION_EDIT_ADD, FALSE, account);
	count = 0;
	while(result == HB_RESPONSE_ADD || result == HB_RESPONSE_ADDKEEP)
	{
	Transaction *ope;

		/* fill in the transaction */
		if( result == HB_RESPONSE_ADD )
		{
			ope = da_transaction_malloc();
			ope->date = date;
			ope->kacc = account;

			if( PREFS->heritdate == FALSE ) //fix: 318733
				ope->date = GLOBALS->today;

			da_transaction_set_default_template(ope);
		}

		// normally we can't be in addkeep without initialized ope with add

		deftransaction_set_transaction(window, ope);

		result = gtk_dialog_run (GTK_DIALOG (window));

		DB( g_print(" - dialog result is %d\n", result) );

		if(result == HB_RESPONSE_ADD || result == HB_RESPONSE_ADDKEEP || result == GTK_RESPONSE_ACCEPT)
		{
			deftransaction_get(window, NULL);
			transaction_add(GTK_WINDOW(GLOBALS->mainwindow), ope);

			DB( g_print(" - added 1 transaction to %d\n", ope->kacc) );

			ui_hub_account_populate(GLOBALS->mainwindow, NULL);

			count++;
			//todo: still usefull ? store last date
			date = ope->date;
		}

		if( result == HB_RESPONSE_ADD )
		{
			da_transaction_free(ope);
			ope = NULL;
		}

	}


	deftransaction_dispose(window, NULL);
	gtk_widget_destroy (window);

	/* todo optimize this */
	if(count > 0)
	{
		GLOBALS->changes_count += count;
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_REFRESHALL));
	}
}


gboolean ui_mainwindow_open_backup_check_confirm(gchar *filepath)
{
gboolean retval = FALSE;
gchar *basename, *secondtext;
gboolean result;

	basename = g_path_get_basename(filepath);
	secondtext = g_strdup_printf (
	_("Your are about to open the backup file '%s'.\n\nAre you sure you want to do this ?"), basename);

	result = ui_dialog_msg_confirm_alert(
		GTK_WINDOW(GLOBALS->mainwindow),
		_("Open the backup file ?"),
		secondtext,
		_("_Open backup")
	);

	g_free(secondtext);
	g_free(basename);

	if( result == GTK_RESPONSE_OK )
		retval = TRUE;

	return retval;
}


/*
**
*/
void ui_mainwindow_open(GtkWidget *widget, gpointer user_data)
{
//struct hbfile_data *data;
gboolean bakmode = GPOINTER_TO_INT(user_data);;
gboolean doopen = TRUE;
gchar *filename = NULL;

	DB( g_print("\n[ui-mainwindow] open\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//#1791554 do ask for save confirm
	if( bakmode != TRUE )
		doopen = ui_dialog_msg_savechanges(widget,NULL);

	if( doopen == TRUE )
	{
		if( ui_file_chooser_xhb(GTK_FILE_CHOOSER_ACTION_OPEN, &filename, bakmode) == TRUE )
		{
			//#1710955 test for backup open
			if( hbfile_file_isbackup(filename) )
			{
				if( ui_mainwindow_open_backup_check_confirm(filename) == TRUE )
				{
					GLOBALS->hbfile_is_bak = TRUE;
				}
				else
				{
					g_free(filename);
					return;
				}
			}

			hbfile_change_filepath(filename);
			ui_mainwindow_open_internal(widget, NULL);
		}
	}
}


/*
 *	open the file stored in GLOBALS->xhb_filepath
 */
void ui_mainwindow_open_internal(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
gint r;

	DB( g_print("\n[ui-mainwindow] open internal\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( GLOBALS->xhb_filepath != NULL )
	{
		DB( g_print(" - filename: '%s'\n", GLOBALS->xhb_filepath) );

		ui_mainwindow_clear(GLOBALS->mainwindow, GINT_TO_POINTER(FALSE));
		GLOBALS->hbfile_is_new = FALSE;

		r = homebank_load_xml(GLOBALS->xhb_filepath);
		if( r == XML_OK )
		{
			DB( g_print(" - file loaded ok : rcode=%d\n", r) );

			GLOBALS->xhb_timemodified = hbfile_file_get_time_modified(GLOBALS->xhb_filepath);
			hbfile_file_hasrevert(GLOBALS->xhb_filepath);

			if(PREFS->appendscheduled)
				scheduled_post_all_pending();

			if(PREFS->do_update_currency)
				ui_cur_manage_dialog_update_currencies(GTK_WINDOW(GLOBALS->mainwindow));

			homebank_lastopenedfiles_save();

			//todo: delete this after computing done at xml read
			account_compute_balances();

			ui_mainwindow_recent_add(data, GLOBALS->xhb_filepath);
		}
		else
		{
		gchar *msg = _("Unknown error");

			switch(r)
			{
				case XML_IO_ERROR:
					msg = _("I/O error for file '%s'.");
					break;
				case XML_FILE_ERROR:
					msg = _("The file '%s' is not a valid HomeBank file.");
					break;
				case XML_VERSION_ERROR:
					msg = _("The file '%s' was saved with a higher version of HomeBank\nand cannot be loaded by the current version.");
					break;
			}

			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
				_("File error"),
				msg,
				GLOBALS->xhb_filepath
				);

			ui_mainwindow_clear(GLOBALS->mainwindow, GINT_TO_POINTER(TRUE));

		}

		ui_hub_account_populate(GLOBALS->mainwindow, NULL);
		ui_hub_scheduled_populate(GLOBALS->mainwindow, NULL);
		ui_hub_spending_populate(GLOBALS->mainwindow, NULL);
		ui_hub_transaction_populate(data);

		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_VISUAL));
	}


}


/*
**
*/
void ui_mainwindow_save(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
gboolean saveas = GPOINTER_TO_INT(user_data);
gchar *filename = NULL;
gint r = XML_UNSET;

	DB( g_print("\n[ui-mainwindow] save\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( GLOBALS->hbfile_is_new == TRUE )
		saveas = 1;

	//#1710955 test for backup open
	if( GLOBALS->hbfile_is_bak == TRUE )
	{
		//todo: later for backup, should also remove datetime and .bak
		hbfile_change_filepath(hb_filename_new_with_extension(GLOBALS->xhb_filepath, "xhb"));
		saveas = 1;
	}

	if(saveas == 1)
	{
		if(ui_file_chooser_xhb(GTK_FILE_CHOOSER_ACTION_SAVE, &filename, FALSE) == TRUE)
		{
			DB( g_print(" + should save as '%s'\n", filename) );
			homebank_file_ensure_xhb(filename);
			homebank_backup_current_file();
			r = homebank_save_xml(GLOBALS->xhb_filepath);
			GLOBALS->hbfile_is_new = FALSE;
			GLOBALS->hbfile_is_bak = FALSE;
		}
		else
			return;
	}
	else
	{
	guint64 time_modified = hbfile_file_get_time_modified (GLOBALS->xhb_filepath);
	gint result = GTK_RESPONSE_OK;

		DB( g_print(" + should quick save '%s'\n + time: open=%lu :: now=%lu\n", GLOBALS->xhb_filepath, GLOBALS->xhb_timemodified, time_modified) );

		if( GLOBALS->xhb_timemodified != time_modified )
		{
			result = ui_dialog_msg_confirm_alert(
					GTK_WINDOW(GLOBALS->mainwindow),
					_("The file has been modified since reading it."),
					_("If you save it, all the external changes could be lost. Save it anyway?"),
					_("S_ave Anyway")
				);

			if( result != GTK_RESPONSE_OK )
				return;
		}

		DB( g_print(" + saving...\n") );
		homebank_file_ensure_xhb(NULL);
		homebank_backup_current_file();
		r = homebank_save_xml(GLOBALS->xhb_filepath);
	}

	if(r == XML_OK)
	{
		DB( g_print(" + OK...\n") );
		GLOBALS->changes_count = 0;
		GLOBALS->xhb_timemodified = hbfile_file_get_time_modified (GLOBALS->xhb_filepath);
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_VISUAL));
	}
	else
	{
	gchar *msg = _("I/O error for file '%s'.");

		ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
			_("File error"),
			msg,
			GLOBALS->xhb_filepath
			);
	}
}


void ui_mainwindow_update(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
gint flags;

	DB( g_print("\n[ui-mainwindow] update %p\n", user_data) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	flags = GPOINTER_TO_INT(user_data);

	/* set window title */
	if(flags & UF_TITLE)
	{
	gchar *basename;
	gchar *changed;

		DB( g_print(" 1: wintitle %p\n", data->wintitle) );

		basename = g_path_get_basename(GLOBALS->xhb_filepath);

		DB( g_print(" global changes: %d\n", GLOBALS->changes_count) );

		g_free(data->wintitle);

		changed = (GLOBALS->changes_count > 0) ? "*" : "";

#if MYDEBUG == 1
		data->wintitle = g_strdup_printf("%s%s (%d)- %s - " PROGNAME, changed, basename, GLOBALS->changes_count, GLOBALS->owner);
#else
		data->wintitle = g_strdup_printf("%s%s - %s - " PROGNAME, changed, basename, GLOBALS->owner);
#endif

	    gtk_window_set_title (GTK_WINDOW (gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), data->wintitle);

		g_free(basename);
	}

	/* update disabled things */
	if(flags & UF_SENSITIVE)
	{
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	GtkTreePath		*path;
	gboolean	active,sensitive;

		DB( g_print(" 2: disabled, opelist count\n") );

		//#1656531
		data->acc = NULL;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
		active = gtk_tree_selection_get_selected(selection, &model, &iter);
		if(active)
		{
		Account *acc;
		gint depth;

			path = gtk_tree_model_get_path(model, &iter);
			depth =	gtk_tree_path_get_depth(path);

			if( depth > 1 )
			{
				DB( g_print(" depth is %d\n", depth) );

				gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, LST_DSPACC_DATAS, &acc, -1);
				data->acc = acc;
			}
			else
				active = FALSE;
		}

		DB( g_print(" changes %d - new %d\n", GLOBALS->changes_count, GLOBALS->hbfile_is_new) );

	// save
		sensitive = (GLOBALS->changes_count != 0 ) ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Save"), sensitive);

	// backup
		sensitive = ( (GLOBALS->changes_count != 0) && GLOBALS->xhb_hasrevert ) ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Revert"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/OpenBak"), sensitive);

	// define off ?
		sensitive = GLOBALS->define_off == 0 ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Account"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Payee"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Category"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Budget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/EditMenu/Preferences"), sensitive);

	// empty account list: disable Archives, Edit, Filter, Add, Statistics, Overdrawn, Car Cost
		sensitive = da_acc_length() > 0 ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/FileMenu/Close"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Archive"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/TxnMenu/AddTxn"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/TxnMenu/ShowTxn"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RStatistics"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RTrendTime"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RBudget"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RBalance"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ReportMenu/RVehiculeCost"), sensitive);

	// empty category list: disable Budget
		sensitive = da_cat_length() > 1 ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Budget"), sensitive);

		//#1501129 no need to disable, P & C can be created from assign dialog
		//sensitive = ((da_cat_length() > 1) || (da_pay_length() > 1)) ? TRUE : FALSE;
		//gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/ManageMenu/Assign"), sensitive);

	// empty archive list: disable scheduled check
		sensitive = g_list_length(GLOBALS->arc_list) > 0 ? TRUE : FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/TxnMenu/AddScheduled"), sensitive);

	// no active account: disable Edit, Over
		sensitive = (active == TRUE ) ? TRUE : FALSE;
		if(data->acc && data->acc->window != NULL)
			sensitive = FALSE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->manager, "/MenuBar/TxnMenu/ShowTxn"), sensitive);
	}

	/* update toolbar, list */
	if(flags & UF_VISUAL)
	{
		DB( g_print(" 8: visual\n") );

		if(PREFS->toolbar_style == 0)
			gtk_toolbar_unset_style(GTK_TOOLBAR(data->toolbar));
		else
			gtk_toolbar_set_style(GTK_TOOLBAR(data->toolbar), PREFS->toolbar_style-1);

		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (data->LV_acc), PREFS->grid_lines);
		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (data->LV_upc), PREFS->grid_lines);
		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_upc));

		DB( g_print(" - show toolbar=%d\n", PREFS->wal_toolbar) );
		if(PREFS->wal_toolbar)
			gtk_widget_show(GTK_WIDGET(data->toolbar));
		else
			gtk_widget_hide(GTK_WIDGET(data->toolbar));


		DB( g_print(" - show top_spending=%d\n", PREFS->wal_spending) );

		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_wal);

		if(PREFS->wal_spending)
			gtk_widget_show(GTK_WIDGET(data->GR_top));
		else
			gtk_widget_hide(GTK_WIDGET(data->GR_top));



		DB( g_print(" - show upcoming=%d\n", PREFS->wal_upcoming) );
		if(PREFS->wal_upcoming)
			gtk_widget_show(GTK_WIDGET(data->GR_upc));
		else
			gtk_widget_hide(GTK_WIDGET(data->GR_upc));

		DB( g_print(" minor %d\n", PREFS->euro_active) );
		gtk_action_set_visible(gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/AsMinor"), PREFS->euro_active);
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{

		DB( g_print(" 4: balances\n") );

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));

		//minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

		/*
		hb-label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->bank, minor);
		hb-label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->today, minor);
		hb-label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->future, minor);
		*/
	}

	if(flags & UF_REFRESHALL)
	{
		DB( g_print(" 16: refreshall\n") );

		ui_hub_account_populate(GLOBALS->mainwindow, NULL);
		ui_hub_spending_populate(GLOBALS->mainwindow, NULL);
		ui_hub_scheduled_populate(GLOBALS->mainwindow, NULL);
		ui_hub_transaction_populate(data);
	}


}



static void
  ui_mainwindow_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
  {
    GtkTreeModel *model;
    GtkTreeIter   iter;

    DB( g_print ("\n[ui-mainwindow] A row has been double-clicked!\n") );

    model = gtk_tree_view_get_model(treeview);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
	Account *acc;

       gtk_tree_model_get(model, &iter, LST_DSPACC_DATAS, &acc, -1);

		if( acc != NULL )
		{

       DB( g_print ("Double-clicked row contains name %s\n", acc->name) );

		ui_mainwindow_action_showtransactions();

       //g_free(name);
    	}
    }
  }


static void ui_mainwindow_destroy(GtkTreeView *treeview, gpointer user_data)
{
	DB( g_print("\n[ui-mainwindow] destroy\n") );

}


/*
**
*/
static gboolean ui_mainwindow_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct hbfile_data *data = user_data;
struct WinGeometry *wg;
gboolean retval = FALSE;

	GValue widget_value = G_VALUE_INIT;
	ext_hook("main_window_disposal", EXT_OBJECT(&widget_value, widget), NULL);

	DB( g_print("\n[ui-mainwindow] delete-event\n") );

	//store position and size
	wg = &PREFS->wal_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(widget));
	GdkWindowState state = gdk_window_get_state(gdk_window);
	wg->s = (state & GDK_WINDOW_STATE_MAXIMIZED) ? 1 : 0;
	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d s=%d, state=%d\n", wg->l, wg->t, wg->w, wg->h, wg->s, state & GDK_WINDOW_STATE_MAXIMIZED) );

 	PREFS->wal_vpaned = gtk_paned_get_position(GTK_PANED(data->vpaned));
 	PREFS->wal_hpaned = gtk_paned_get_position(GTK_PANED(data->hpaned));
	DB( g_print(" - vpaned=%d hpaned=%d\n", PREFS->wal_vpaned, PREFS->wal_hpaned) );

	if(PREFS->pnl_list_tab)
		g_free(PREFS->pnl_list_tab);
	PREFS->pnl_list_tab = g_strdup(gtk_stack_get_visible_child_name(GTK_STACK(data->stack)));

	//todo
	if(ui_dialog_msg_savechanges(widget, NULL) == FALSE)
	{
		retval = TRUE;
	}
	else
	{
		//todo: retval is useless and below should move to destroy
		retval = TRUE;
		gtk_widget_destroy(data->LV_top);

		g_free(data->wintitle);
		da_flt_free(data->filter);
		g_free(user_data);

		gtk_main_quit();
	}

	//TRUE:stop other handlers from being invoked for the event | FALSE: propagate
	return retval;
}


static void ui_mainwindow_recent_chooser_item_activated_cb (GtkRecentChooser *chooser, struct hbfile_data *data)
{
	gchar *uri, *path;
	GError *error = NULL;

	uri = gtk_recent_chooser_get_current_uri (chooser);

	path = g_filename_from_uri (uri, NULL, NULL);
	if (error)
	{
		g_warning ("Could not convert uri \"%s\" to a local path: %s", uri, error->message);
		g_error_free (error);
		return;
	}

	if( ui_dialog_msg_savechanges(data->window, NULL) == TRUE )
	{

		//todo: FixMe
		/*
		if (! load)
		{
			gpw_recent_remove (gpw, path);
		}
		*/

		hbfile_change_filepath(path);
		ui_mainwindow_open_internal(data->window, NULL);
	}
	else
	{
		g_free (path);
	}
	g_free (uri);
}


void ui_mainwindow_recent_add (struct hbfile_data *data, const gchar *path)
{
	GtkRecentData *recent_data;
	gchar *uri;
	GError *error = NULL;

	DB( g_print("\n[ui-mainwindow] recent_add\n") );

	DB( g_print(" - file has .xhb suffix = %d\n", g_str_has_suffix (path, ".xhb") ) );

	if( g_str_has_suffix (path, ".xhb") == FALSE )	//ignore reverted file
		return;

	uri = g_filename_to_uri (path, NULL, &error);
	if (error)
	{
		g_warning ("Could not convert uri \"%s\" to a local path: %s", uri, error->message);
		g_error_free (error);
		return;
	}

	recent_data = g_slice_new (GtkRecentData);

	recent_data->display_name   = NULL;
	recent_data->description    = NULL;
	recent_data->mime_type      = "application/x-homebank";
	recent_data->app_name       = (gchar *) g_get_application_name ();
	recent_data->app_exec       = g_strjoin (" ", g_get_prgname (), "%u", NULL);
	recent_data->groups         = NULL;
	recent_data->is_private     = FALSE;

	if (!gtk_recent_manager_add_full (data->recent_manager,
				          uri,
				          recent_data))
	{
      		g_warning ("Unable to add '%s' to the list of recently used documents", uri);
	}

	g_free (uri);
	g_free (recent_data->app_exec);
	g_slice_free (GtkRecentData, recent_data);

}





enum
{
	TARGET_URI_LIST
};

static GtkTargetEntry drop_types[] =
{
	{"text/uri-list", 0, TARGET_URI_LIST}
};

static void ui_mainwindow_drag_data_received (GtkWidget *widget,
			GdkDragContext *context,
			gint x, gint y,
			GtkSelectionData *selection_data,
			guint info, guint time, GtkWindow *window)
{
gchar **uris, **str;
gchar *newseldata;
gint n_uris, filetype, slen;
GError *error = NULL;

	if (info != TARGET_URI_LIST)
		return;

	DB( g_print("\n[ui-mainwindow] drag_data_received\n") );

	/* On MS-Windows, it looks like `selection_data->data' is not NULL terminated. */
	slen = gtk_selection_data_get_length(selection_data);
	newseldata = g_new (gchar, slen + 1);
	memcpy (newseldata, gtk_selection_data_get_data(selection_data), slen);
	newseldata[slen] = 0;
	//DB( g_print(" - seldata ='%s'\n", gtk_selection_data_get_data(selection_data) ) );
	//DB( g_print(" - newseldata ='%s'\n", newseldata ) );

	uris = g_uri_list_extract_uris (newseldata);
	n_uris = g_strv_length(uris);
	DB( g_print(" - dragged %d files (len=%d)\n", n_uris, slen ) );

	g_free(newseldata);

	//single file: check for xhb
	if(n_uris == 1)
	{
		filetype = hb_filename_type_get_by_extension(*uris);

		DB( g_print(" - filetype is homebank (%d)\n", filetype) );

		if( filetype == FILETYPE_HOMEBANK )
		{
		gchar *path = g_filename_from_uri (*uris, NULL, &error);

			if( path != NULL )
			{
				DB( g_print(" - path is '%s'\n", path) );
				hbfile_change_filepath(g_strdup(path));
				ui_mainwindow_open_internal(GTK_WIDGET(window), NULL);
				goto end_drop;
			}
			else
			{
				g_warning ("Could not convert uri to local path: %s", error->message);
				g_error_free (error);
			}
			g_free (path);
		}
		/* we no more manage error here
		ui_dialog_msg_infoerror(GTK_WINDOW(window), GTK_MESSAGE_ERROR,
					_("File error"),
					_("The file %s is not a valid HomeBank file."),
					path);
		*/
	}

	//collect known filetype to import
	DB( g_print(" - collect %d files\n", n_uris) );

	gchar **paths = g_new (gchar *, n_uris + 1);
	slen = 0;
	for (str = uris; *str; str++)
	{
		filetype = hb_filename_type_get_by_extension(*str);
		if( filetype != FILETYPE_HOMEBANK && filetype != FILETYPE_UNKNOWN )
		{
		gchar *path = g_filename_from_uri (*str, NULL, NULL);

			if( path != NULL )
			{
				DB( g_print(" - append %d '%s'\n", slen, path ) );
				paths[slen++] = path;
			}
		}
	}
	paths[slen] = NULL;

	if( slen > 0 )
	{
		ui_import_assistant_new( paths );
	}


end_drop:
	g_strfreev (uris);
}


static GtkWidget *ui_mainwindow_create_recent_chooser_menu (GtkRecentManager *manager)
{
GtkWidget *recent_menu;
GtkRecentFilter *filter;

	recent_menu = gtk_recent_chooser_menu_new_for_manager (manager);
	gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu), FALSE);
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu), GTK_RECENT_SORT_MRU);
	//todo: add a user pref for this
	gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER (recent_menu), 10);
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), FALSE);
	//gtk_recent_chooser_menu_set_show_numbers (GTK_RECENT_CHOOSER_MENU (recent_menu), TRUE);

	filter = gtk_recent_filter_new ();
	//gtk_recent_filter_add_application (filter, g_get_application_name());
	gtk_recent_filter_add_pattern (filter, "*.[Xx][Hh][Bb]");
	gtk_recent_chooser_set_filter (GTK_RECENT_CHOOSER (recent_menu), filter);

	return recent_menu;
}


static void ui_mainwindow_create_menu_bar_and_toolbar(struct hbfile_data *data, GtkWidget *mainvbox)
{
GtkUIManager *manager;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	manager = gtk_ui_manager_new ();
	data->manager = manager;

	gtk_window_add_accel_group (GTK_WINDOW (data->window),
				gtk_ui_manager_get_accel_group(manager));

	actions = gtk_action_group_new ("MainWindow");
	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (actions,
			entries,
			n_entries,
			NULL);

	gtk_action_group_add_toggle_actions (actions,
			toggle_entries,
			n_toggle_entries,
			NULL);

	gtk_ui_manager_insert_action_group (data->manager, actions, 0);
	g_object_unref (actions);
	data->actions = actions;

	/* set short labels to use in the toolbar */
	action = gtk_action_group_get_action(actions, "Open");
	g_object_set(action, "short_label", _("Open"), NULL);

	//action = gtk_action_group_get_action(action_group, "Save");
	//g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Account");
	g_object_set(action, "short_label", _("Account"), NULL);

	action = gtk_action_group_get_action(actions, "Payee");
	g_object_set(action, "short_label", _("Payee"), NULL);

	action = gtk_action_group_get_action(actions, "Category");
	g_object_set(action, "short_label", _("Category"), NULL);

	action = gtk_action_group_get_action(actions, "Archive");
	//TRANSLATORS: an archive is stored transaction buffers (kind of bookmark to prefill manual insertion)
	g_object_set(action, "short_label", _("Archive"), NULL);

	action = gtk_action_group_get_action(actions, "Budget");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(actions, "ShowTxn");
	g_object_set(action, "short_label", _("Show"), NULL);

	action = gtk_action_group_get_action(actions, "AddTxn");
	g_object_set(action, "is_important", TRUE, "short_label", _("Add"), NULL);

	action = gtk_action_group_get_action(actions, "RStatistics");
	g_object_set(action, "short_label", _("Statistics"), NULL);

	action = gtk_action_group_get_action(actions, "RBudget");
	g_object_set(action, "short_label", _("Budget"), NULL);

	action = gtk_action_group_get_action(actions, "RBalance");
	g_object_set(action, "short_label", _("Balance"), NULL);

	action = gtk_action_group_get_action(actions, "RVehiculeCost");
	g_object_set(action, "short_label", _("Vehicle cost"), NULL);

	/* now load the UI definition */
	gtk_ui_manager_add_ui_from_string (data->manager, ui_info, -1, &error);
	if (error != NULL)
	{
		g_message ("Building menus failed: %s", error->message);
		g_error_free (error);
	}


	data->recent_manager = gtk_recent_manager_get_default ();

	data->menubar = gtk_ui_manager_get_widget (manager, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (mainvbox),
			    data->menubar,
			    FALSE,
			    FALSE,
			    0);

	/* recent files menu */
	data->recent_menu = ui_mainwindow_create_recent_chooser_menu (data->recent_manager);

	g_signal_connect (data->recent_menu,
			  "item-activated",
			  G_CALLBACK (ui_mainwindow_recent_chooser_item_activated_cb),
			  data);

	GtkWidget *widget = gtk_ui_manager_get_widget (data->manager, "/MenuBar/FileMenu/RecentMenu");
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), data->recent_menu);


	data->toolbar = gtk_ui_manager_get_widget (manager, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (mainvbox),
			    data->toolbar,
			    FALSE,
			    FALSE,
			    0);

	/* add the custom Open button to the toolbar */
	GtkWidget *image = gtk_image_new_from_icon_name (ICONNAME_HB_FILE_OPEN, GTK_ICON_SIZE_BUTTON);
	GtkToolItem *open_button = gtk_menu_tool_button_new(image, _("_Open"));
	gtk_tool_item_set_tooltip_text (open_button, _("Open a file"));

	GtkWidget *recent_menu = ui_mainwindow_create_recent_chooser_menu (data->recent_manager);
	gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (open_button), recent_menu);
	gtk_menu_tool_button_set_arrow_tooltip_text (GTK_MENU_TOOL_BUTTON (open_button), _("Open a recently used file"));

	g_signal_connect (recent_menu,
			  "item-activated",
			  G_CALLBACK (ui_mainwindow_recent_chooser_item_activated_cb),
			  data);

	action = gtk_action_group_get_action (data->actions, "Open");
	g_object_set (action, "short_label", _("Open"), NULL);
	//gtk_action_connect_proxy (action, GTK_WIDGET (open_button));
	gtk_activatable_set_related_action (GTK_ACTIVATABLE (open_button), action);

	gtk_toolbar_insert (GTK_TOOLBAR (data->toolbar), open_button, 1);

}


/*
** the window creation
*/
GtkWidget *create_hbfile_window(GtkWidget *do_widget)
{
struct hbfile_data *data;
struct WinGeometry *wg;
GtkWidget *mainvbox, *vbox, *box, *vpaned, *hpaned, *sidebar, *stack;
GtkWidget *widget, *page;
GtkWidget *window;
GtkAction *action;

	DB( g_print("\n[ui-mainwindow] create main window\n") );

	data = g_malloc0(sizeof(struct hbfile_data));
	if(!data) return NULL;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", window, data) );

	// this is our mainwindow, so store it to GLOBALS data
	data->window = window;
	GLOBALS->mainwindow = window;

	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	ui_mainwindow_create_menu_bar_and_toolbar (data, mainvbox);

#if HB_UNSTABLE_SHOW == TRUE
GtkWidget *bar, *label;

	bar = gtk_info_bar_new ();
	gtk_box_pack_start (GTK_BOX (mainvbox), bar, FALSE, FALSE, 0);
	gtk_info_bar_set_message_type (GTK_INFO_BAR (bar), GTK_MESSAGE_WARNING);
	label = make_label(NULL, 0.5, 0.5);
	gtk_label_set_markup (GTK_LABEL(label), "Unstable Development Version");
	gtk_box_pack_start (GTK_BOX (gtk_info_bar_get_content_area (GTK_INFO_BAR (bar))), label, FALSE, FALSE, 0);
#endif

	/* Add the main area */
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    //gtk_container_set_border_width (GTK_CONTAINER(vbox), SPACING_MEDIUM);
    gtk_box_pack_start (GTK_BOX (mainvbox), vbox, TRUE, TRUE, 0);

	vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	data->vpaned = vpaned;
    gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 0);

	hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	data->hpaned = hpaned;
	gtk_paned_pack1 (GTK_PANED(vpaned), hpaned, FALSE, FALSE);

		widget = ui_hub_account_create(data);
		//gtk_widget_set_size_request (widget, 100, -1);
		gtk_paned_pack1 (GTK_PANED(hpaned), widget, FALSE, FALSE);

		widget = ui_hub_spending_create(data);
		//gtk_widget_set_size_request (widget, -1, 100);
		gtk_paned_pack2 (GTK_PANED(hpaned), widget, TRUE, FALSE);

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	data->GR_upc = box;
	gtk_paned_pack2 (GTK_PANED(vpaned), box, TRUE, FALSE);

	sidebar = gtk_stack_sidebar_new ();
	gtk_box_pack_start (GTK_BOX (box), sidebar, FALSE, FALSE, 0);

	stack = gtk_stack_new ();
	//gtk_stack_set_transition_type (GTK_STACK (stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);
	gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR (sidebar), GTK_STACK (stack));
	data->stack = stack;
    gtk_box_pack_start (GTK_BOX (box), stack, TRUE, TRUE, 0);

		page = ui_hub_scheduled_create(data);
		gtk_stack_add_titled (GTK_STACK (stack), page, "sched", _("Scheduled"));
		//gtk_paned_pack2 (GTK_PANED(vpaned), widget, TRUE, FALSE);

		page = ui_hub_transaction_create(data, HUB_TXN_TYPE_FUTURE);
		gtk_stack_add_titled (GTK_STACK (stack), page, "futur", _("Future"));

		page = ui_hub_transaction_create(data, HUB_TXN_TYPE_REMIND);
		gtk_stack_add_titled (GTK_STACK (stack), page, "remin", _("Remind"));


	//setup, init and show window
	wg = &PREFS->wal_wg;
	if(wg->s == 0)
	{
		gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
		gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);
	}
	else
		gtk_window_maximize(GTK_WINDOW(window));

	gtk_widget_show_all (window);

	//#1662197/1660910 moved after resize/show
	DB( g_print(" - vpaned=%d hpaned=%d\n", PREFS->wal_vpaned, PREFS->wal_hpaned) );

	if(PREFS->wal_hpaned > 0)
		gtk_paned_set_position(GTK_PANED(data->hpaned), PREFS->wal_hpaned);
	if(PREFS->wal_vpaned > 0)
		gtk_paned_set_position(GTK_PANED(data->vpaned), PREFS->wal_vpaned);

	if( PREFS->pnl_list_tab != NULL )
		gtk_stack_set_visible_child_name (GTK_STACK(data->stack), PREFS->pnl_list_tab);


	//todo: move this elsewhere
	DB( g_print(" - setup stuff\n") );

	data->filter = da_flt_malloc();
	filter_reset(data->filter);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_wal);

	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/Toolbar");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_toolbar);
	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/Spending");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_spending);
	action = gtk_ui_manager_get_action(data->manager, "/MenuBar/ViewMenu/BottomLists");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), PREFS->wal_upcoming);

	/* Drag and drop support, set targets to NULL because we add the
	   default uri_targets below */

	/* support for opening a file by dragging onto the project window */
	gtk_drag_dest_set (GTK_WIDGET (window),
			   GTK_DEST_DEFAULT_ALL,
			   drop_types,
	           G_N_ELEMENTS (drop_types),
			   GDK_ACTION_COPY);

	g_signal_connect (G_OBJECT (window), "drag-data-received",
			  G_CALLBACK (ui_mainwindow_drag_data_received), window);

	//connect all our signals
	DB( g_print(" - connect signals\n") );

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), "changed", G_CALLBACK (ui_mainwindow_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data->LV_acc    ), "row-activated", G_CALLBACK (ui_mainwindow_onRowActivated), GINT_TO_POINTER(2));

	/* GtkWindow events */
    g_signal_connect (window, "delete-event", G_CALLBACK (ui_mainwindow_dispose), (gpointer)data);
	g_signal_connect (window, "destroy", G_CALLBACK (ui_mainwindow_destroy), NULL);

	//gtk_action_group_set_sensitive(data->actions, FALSE);

	return window;
}

