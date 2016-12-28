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

#include "dsp_mainwindow.h"
#include "hb-preferences.h"
#include "language.h"



#ifdef G_OS_WIN32
#include <windows.h>
#endif

#define APPLICATION_NAME "HomeBank"

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
struct HomeBank *GLOBALS;
struct Preferences *PREFS;


/* installation paths */
static gchar *config_dir   = NULL;
static gchar *images_dir   = NULL;
static gchar *pixmaps_dir  = NULL;
static gchar *locale_dir   = NULL;
static gchar *help_dir     = NULL;
static gchar *datas_dir    = NULL;


//#define MARKUP_STRING "<span size='small'>%s</span>"


/* Application arguments */
static gboolean arg_version = FALSE;
static gchar **files = NULL;

static GOptionEntry option_entries[] =
{
	{ "version", '\0', 0, G_OPTION_ARG_NONE, &arg_version,
	  N_("Output version information and exit"), NULL },

	{ G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &files,
	  NULL, N_("[FILE]") },

	{ NULL }
};


/*
** try to determine the file type (if supported for import by homebank)
**
**
*/
gint homebank_alienfile_recognize(gchar *filename)
{
GIOChannel *io;
gint i, retval = FILETYPE_UNKNOW;
gchar *tmpstr;
gint io_stat;
GError *err = NULL;
static gint csvtype[7] = {
					CSV_DATE,
					CSV_INT,
					CSV_STRING,
					CSV_STRING,
					CSV_STRING,
					CSV_DOUBLE,
					CSV_STRING,
					};


	DB( g_print("\n[homebank] alienfile_recognize\n") );


	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		g_io_channel_set_encoding(io, NULL, NULL);	/* set to binary mode */

		for(i=0;i<4;i++)
		{
			if( retval != FILETYPE_UNKNOW )
				break;

			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, &err);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( *tmpstr != '\0' )
				{
					DB( g_print(" line %d: '%s' retval=%d\n", i, tmpstr, retval) );

					/* native homebank file */
					if( g_str_has_prefix(tmpstr, "<homebank v="))
					{
						DB( g_print(" type is HomeBank\n") );
						retval = FILETYPE_HOMEBANK;
					}
					else

					// QIF file ?
					if( g_str_has_prefix(tmpstr, "!Type") ||
					    g_str_has_prefix(tmpstr, "!type") ||
					    g_str_has_prefix(tmpstr, "!Option") ||
					    g_str_has_prefix(tmpstr, "!option") ||
					    g_str_has_prefix(tmpstr, "!Account") ||
					    g_str_has_prefix(tmpstr, "!account")
					    )
					{
						DB( g_print(" type is QIF\n") );
						retval = FILETYPE_QIF;
					}
					else

					/* is it OFX ? */
					if( g_strstr_len(tmpstr, 10, "OFX") != NULL)
					{
						DB( g_print(" type is OFX\n") );
						retval = FILETYPE_OFX;
					}

					/* is it csv homebank ? */
					else
					{
					gchar **str_array;
					gboolean isvalid = FALSE;

						hb_string_strip_crlf(tmpstr);
						str_array = hb_csv_row_get(tmpstr, ";", 8);
						isvalid = hb_csv_row_valid(str_array, 8, csvtype);

						DB( g_print(" hbcsv %d\n", isvalid) );

						if( isvalid == TRUE  )
						{
							DB( g_print(" type is CSV homebank\n") );
							retval = FILETYPE_CSV_HB;
						}

						g_strfreev (str_array);
					}
					g_free(tmpstr);
				}
			}
		}
		g_io_channel_unref (io);
	}

	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = */


/*
** ensure the filename ends with '.xhb'
*/
void homebank_file_ensure_xhb(gchar *filename)
{
	DB( g_print("\n[homebank] file_ensure_xhb\n") );
	filename = (filename == NULL) ? g_strdup(GLOBALS->xhb_filepath) : filename;
	if( g_str_has_suffix (filename, ".xhb") == FALSE )
	{
	gchar *newfilename;
	
		newfilename = hb_util_filename_new_with_extension(filename, "xhb");
		hbfile_change_filepath(newfilename);
		DB( g_print(" - changed to: '%s'\n", GLOBALS->xhb_filepath) );
	}
	//#1460390
	else
	{
		hbfile_change_filepath(filename);
	}
}


static gboolean homebank_copy_file(gchar *srcfile, gchar *dstfile)
{
gchar *buffer;
gsize length;
//GError *error = NULL;
gboolean retval = FALSE;

	if (g_file_get_contents (srcfile, &buffer, &length, NULL))
	{
		if(g_file_set_contents(dstfile, buffer, length, NULL))
		{
			retval = TRUE;
		}
		g_free(buffer);
	}
	return retval;
}


void homebank_backup_current_file(void)
{
gchar *bakfilename;

	DB( g_print("\n[homebank] backup_current_file\n") );

	bakfilename = hb_util_filename_new_with_extension (GLOBALS->xhb_filepath, "xhb~");
	if( g_file_test(bakfilename, G_FILE_TEST_EXISTS) )
	{
		DB( g_print(" - delete existing: '%s'\n", bakfilename) );
		g_remove(bakfilename);
	}

	DB( g_print(" - copy '%s' => '%s'\n", GLOBALS->xhb_filepath, bakfilename) );

	//#512046 copy file not to broke potential links
	//retval = g_rename(pathname, newname);
	homebank_copy_file (GLOBALS->xhb_filepath, bakfilename);
	
	g_free(bakfilename);
}

/* = = = = = = = = = = = = = = = = = = = = */
/* url open */


#ifdef G_OS_WIN32
#define SW_NORMAL 1

static gboolean
homebank_util_url_show_win32 (const gchar *url)
{
int retval;
gchar *errmsg;

	/* win32 API call */
	retval = ShellExecuteA (NULL, "open", url, NULL, NULL, SW_NORMAL);

	if (retval < 0 || retval > 32)
		return TRUE;

	errmsg = g_win32_error_message(retval);
	DB( g_print ("%s\n", errmsg) );
	g_free(errmsg);

	return FALSE;
}

#else

static gboolean
homebank_util_url_show_unix (const gchar *url)
{
gboolean retval;
GError *err = NULL;

	retval = gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (GLOBALS->mainwindow)), url, GDK_CURRENT_TIME, &err);

	if (!retval)
	{
		ui_dialog_msg_infoerror(GTK_WINDOW(GLOBALS->mainwindow), GTK_MESSAGE_ERROR,
			_("Browser error."),
			_("Could not display the URL '%s'"),
			url
			);
	}

	if(err != NULL)
	{
		g_print ("%s\n", err->message);
		g_error_free (err);
	}

	return retval;
}

#endif

gboolean
homebank_util_url_show (const gchar *url)
{

	if(url == NULL)
		return FALSE;


#ifdef G_OS_WIN32
	return homebank_util_url_show_win32 (url);
#else
	return homebank_util_url_show_unix (url);
#endif
}


/* = = = = = = = = = = = = = = = = = = = = */
/* lastopenedfiles */

/*
** load lastopenedfiles from homedir/.homebank
*/
gboolean homebank_lastopenedfiles_load(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename, *lastfilename;

	DB( g_print("\n[homebank] lastopenedfiles load\n") );

	keyfile = g_key_file_new();
	if(keyfile)
	{

		filename = g_build_filename(homebank_app_get_config_dir(), "lastopenedfiles", NULL );

		DB( g_print(" - filename: %s\n", filename) );

		if(g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
		{
			group = "HomeBank";

            DB( g_print(" - load keyfile ok\n") );

			if(g_key_file_has_key(keyfile, group, "LastOpenedFile", NULL))
			{
                DB( g_print(" - keyfile has key ok\n") );

				lastfilename = g_key_file_get_string  (keyfile, group, "LastOpenedFile", NULL);

				DB( g_print(" - lastfile loaded: %s\n", lastfilename ) );
				// #593082
				if (g_file_test (lastfilename, G_FILE_TEST_EXISTS) != FALSE)
				{
					DB( g_print(" - file exists\n") );

					hbfile_change_filepath(lastfilename);

					retval = TRUE;
				}
			}
		}
		g_free(filename);
		g_key_file_free (keyfile);
	}

	DB( g_print(" - return: %d\n", retval) );

	return retval;
}


/*
** save lastopenedfiles to homedir/.homebank (HB_DATA_PATH)
*/
gboolean homebank_lastopenedfiles_save(void)
{
GKeyFile *keyfile;
gboolean retval = FALSE;
gchar *group, *filename;
gsize length;

	DB( g_print("\n[homebank] lastopenedfiles save\n") );

	if( GLOBALS->xhb_filepath != NULL )
	{
		keyfile = g_key_file_new();
		if(keyfile )
		{
			DB( g_print(" - saving '%s'\n", GLOBALS->xhb_filepath) );

			group = "HomeBank";
			g_key_file_set_string  (keyfile, group, "LastOpenedFile", GLOBALS->xhb_filepath);

			gchar *contents = g_key_file_to_data( keyfile, &length, NULL);

			//DB( g_print(" keyfile:\n%s\nlen=%d\n", contents, length) );

			filename = g_build_filename(homebank_app_get_config_dir(), "lastopenedfiles", NULL );
			g_file_set_contents(filename, contents, length, NULL);
			g_free(filename);

			g_free(contents);
			g_key_file_free (keyfile);
		}
	}

	return retval;
}



/* = = = = = = = = = = = = = = = = = = = = */
/* Main homebank */
#ifdef G_OS_WIN32
static GtkCssProvider *provider;

static void
homebank_theme_changed (GtkSettings *settings, GParamSpec  *pspec, gpointer     data)
{

	if (pspec == NULL || g_str_equal (pspec->name, "gtk-theme-name"))
	{
		gchar *theme;
		GdkScreen *screen;

		g_object_get (settings, "gtk-theme-name", &theme, NULL);
		screen = gdk_screen_get_default ();

		DB( g_print("theme %s\n", theme) );

		if (g_str_equal (theme, "gtk-win32"))
		{
			if (provider == NULL)
			{
				gchar *filename;

                filename = g_build_filename(homebank_app_get_datas_dir(), "homebank-gtk-win32.css", NULL );
                DB( g_print("tweak file %s\n", filename) );

                if( g_file_test(filename, G_FILE_TEST_EXISTS) )
                {
                    provider = gtk_css_provider_new ();
                    gtk_css_provider_load_from_path (provider, filename, NULL);
                }
                g_free (filename);
			}

            if(provider != NULL)
            {
                DB( g_print(" assign provider %p to screen %p\n", provider, screen) );

                gtk_style_context_add_provider_for_screen (screen,
                                       GTK_STYLE_PROVIDER (provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
		}
		else if (provider != NULL)
		{
			gtk_style_context_remove_provider_for_screen (screen,
								      GTK_STYLE_PROVIDER (provider));
			g_clear_object (&provider);
		}

		g_free (theme);
	}
}

static void
homebank_setup_theme_extensions (void)
{
	GtkSettings *settings;

	settings = gtk_settings_get_default ();
	provider = NULL;
	g_signal_connect (settings, "notify", G_CALLBACK (homebank_theme_changed), NULL);
	homebank_theme_changed (settings, NULL, NULL);
}
#endif


static void
homebank_icon_theme_setup()
{
	DB( g_print("\n[homebank] icon_theme_setup\n") );

	GLOBALS->icontheme = gtk_icon_theme_get_default();

	DB( g_print(" - prepend theme search path: %s\n", homebank_app_get_pixmaps_dir()) );
	gtk_icon_theme_prepend_search_path (GLOBALS->icontheme, homebank_app_get_pixmaps_dir());
	//DB( g_print(" - append theme search path: %s\n", homebank_app_get_pixmaps_dir()) );
	//gtk_icon_theme_append_search_path (GLOBALS->icontheme, homebank_app_get_pixmaps_dir());


	#if MYDEBUG == 1
	GtkIconTheme *ic = gtk_icon_theme_get_default();
	guint i;
	gchar **paths;

		DB( g_print(" - get default icon theme\n") );

		gtk_icon_theme_get_search_path(ic, &paths, NULL);
		for(i=0;i<g_strv_length(paths);i++)
		{
			g_print(" - path %d: %s\n", i, paths[i]);
		}

		g_strfreev(paths);

	#endif


}


const gchar *
homebank_app_get_config_dir (void)
{
	return config_dir;
}

const gchar *
homebank_app_get_images_dir (void)
{
	return images_dir;
}

const gchar *
homebank_app_get_pixmaps_dir (void)
{
	return pixmaps_dir;
}

const gchar *
homebank_app_get_locale_dir (void)
{
	return locale_dir;
}

const gchar *
homebank_app_get_help_dir (void)
{
	return help_dir;
}

const gchar *
homebank_app_get_datas_dir (void)
{
	return datas_dir;
}


/* build package paths at runtime */
static void
build_package_paths (void)
{
	DB( g_print("\n[homebank] build_package_paths\n") );

#ifdef G_OS_WIN32
	gchar *prefix;

	prefix = g_win32_get_package_installation_directory_of_module (NULL);
	locale_dir   = g_build_filename (prefix, "share", "locale", NULL);
	images_dir   = g_build_filename (prefix, "share", PACKAGE, "images", NULL);
	pixmaps_dir  = g_build_filename (prefix, "share", PACKAGE, "icons", NULL);
	help_dir     = g_build_filename (prefix, "share", PACKAGE, "help", NULL);
	datas_dir    = g_build_filename (prefix, "share", PACKAGE, "datas", NULL);
	#ifdef PORTABLE_APP
		DB( g_print(" - app is portable under windows\n") );
		config_dir   = g_build_filename(prefix, "config", NULL);
	#else
		config_dir   = g_build_filename(g_get_user_config_dir(), HB_DATA_PATH, NULL);
	#endif
	g_free (prefix);
#else
	locale_dir   = g_build_filename (DATA_DIR, "locale", NULL);
	images_dir   = g_build_filename (SHARE_DIR, "images", NULL);
	pixmaps_dir  = g_build_filename (DATA_DIR, PACKAGE, "icons", NULL);
	help_dir     = g_build_filename (DATA_DIR, PACKAGE, "help", NULL);
	datas_dir    = g_build_filename (DATA_DIR, PACKAGE, "datas", NULL);
	config_dir   = g_build_filename(g_get_user_config_dir(), HB_DATA_PATH, NULL);

	//#870023 Ubuntu packages the help files in "/usr/share/doc/homebank-data/help/" for some strange reason
	if(! g_file_test(help_dir, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
	{
		g_free (help_dir);
		help_dir = g_build_filename ("/usr", "share", "doc", "homebank-data", "help", NULL);
	}
#endif

	DB( g_print(" - config_dir : %s\n", config_dir) );
	DB( g_print(" - images_dir : %s\n", images_dir) );
	DB( g_print(" - pixmaps_dir: %s\n", pixmaps_dir) );
	DB( g_print(" - locale_dir : %s\n", locale_dir) );
	DB( g_print(" - help_dir   : %s\n", help_dir) );
	DB( g_print(" - datas_dir  : %s\n", datas_dir) );

}


guint32 homebank_app_date_get_julian(void)
{
GDate *date;
	//init global default value
	date = g_date_new();
	g_date_set_time_t(date, time(NULL));
	GLOBALS->today = g_date_get_julian(date);
	g_date_free(date);
	return GLOBALS->today;
}


static gboolean homebank_check_app_dir_migrate_file(gchar *srcdir, gchar *dstdir, gchar *filename)
{
gchar *srcpath;
gchar *dstpath;
gchar *buffer;
gsize length;
//GError *error = NULL;
gboolean retval = FALSE;

	DB( g_print("\n[homebank] check_app_dir_migrate_file\n") );

	srcpath = g_build_filename(srcdir, filename, NULL );
	dstpath = g_build_filename(dstdir, filename, NULL );

	if (g_file_get_contents (srcpath, &buffer, &length, NULL))
	{
		if(g_file_set_contents(dstpath, buffer, length, NULL))
		{
			//g_print("sould delete %s\n", srcpath);
			g_remove(srcpath);
			retval = TRUE;
		}
	}

	g_free(dstpath);
	g_free(srcpath);

	return retval;
}

/*
 * check/create user home directory for .homebank (HB_DATA_PATH) directory
 */
static void homebank_check_app_dir()
{
gchar *homedir;
const gchar *configdir;
gboolean exists;

	DB( g_print("\n[homebank] check_app_dir\n") );

	/* check if <userdir>/.config exist */
#ifndef G_OS_WIN32
	configdir = g_get_user_config_dir();
	DB( g_print(" - check '%s' exists\n", configdir) );
	if(!g_file_test(configdir, G_FILE_TEST_IS_DIR))
	{
		DB( g_print(" - creating dir\n") );
		g_mkdir(configdir, 0755);
	}
#endif

	/* check for XDG .config/homebank */
	configdir = homebank_app_get_config_dir();
	DB( g_print(" - config_dir is: '%s'\n", configdir) );
	exists = g_file_test(configdir, G_FILE_TEST_IS_DIR);
	if(exists)
	{
		/* just update folder security */
		DB( g_print(" - chmod 0700\n") );
		g_chmod(configdir, 0700);
		GLOBALS->first_run = FALSE;
	}
	else
	{
		/* create the config dir */
		DB( g_print(" - create config_dir\n") );
		g_mkdir(configdir, 0755);
		g_chmod(configdir, 0700);

		/* any old homedir configuration out there ? */
		homedir = g_build_filename(g_get_home_dir (), ".homebank", NULL );
		DB(	g_print(" - homedir is: '%s'\n", homedir) );

		exists = g_file_test(homedir, G_FILE_TEST_IS_DIR);
		if(exists)
		{
		gboolean f1, f2;
			/* we must do the migration properly */
			DB( g_print(" - migrate old 2 files\n") );
			f1 = homebank_check_app_dir_migrate_file(homedir, config_dir, "preferences");
			f2 = homebank_check_app_dir_migrate_file(homedir, config_dir, "lastopenedfiles");
			if(f1 && f2)
			{
				DB( g_print(" - removing old dir\n") );
				g_rmdir(homedir);
			}
		}
		g_free(homedir);
		GLOBALS->first_run = TRUE;
	}

}


/*
** application cleanup: icons, GList, memory
*/
static void homebank_cleanup()
{

	DB( g_print("\n[homebank] cleanup\n") );

	//v3.4 save windows size/position
	homebank_pref_save();

	hbfile_cleanup(TRUE);

	/* free our global datas */
	if( PREFS  )
	{
		homebank_pref_free();
		g_free(PREFS);
	}

	if(GLOBALS)
	{
		g_free(GLOBALS);
	}

	g_free (config_dir);
	g_free (images_dir);
	g_free (pixmaps_dir);
	g_free (locale_dir);
	g_free (help_dir);

}



/*
** application setup: icons, GList, memory
*/
static gboolean homebank_setup()
{

	DB( g_print("\n[homebank] setup\n") );

	GLOBALS = g_malloc0(sizeof(struct HomeBank));
	if(!GLOBALS) return FALSE;
	PREFS   = g_malloc0(sizeof(struct Preferences));
	if(!PREFS) return FALSE;

	// check homedir for .homebank dir
	homebank_check_app_dir();

	homebank_pref_setdefault();
	homebank_pref_load();

	hbfile_setup(TRUE);

	homebank_icon_theme_setup();

#ifdef G_OS_WIN32
	homebank_setup_theme_extensions();
#endif

	homebank_app_date_get_julian();


	#if MYDEBUG == 1

	g_print(" - user_name: %s\n", g_get_user_name ());
	g_print(" - real_name: %s\n", g_get_real_name ());
	g_print(" - user_cache_dir: %s\n", g_get_user_cache_dir());
	g_print(" - user_data_dir: %s\n", g_get_user_data_dir ());
	g_print(" - user_config_dir: %s\n", g_get_user_config_dir ());
	//g_print(" - system_data_dirs: %s\n", g_get_system_data_dirs ());
	//g_print(" - system_config_dirs: %s\n", g_get_system_config_dirs ());

	g_print(" - home_dir: %s\n", g_get_home_dir ());
	g_print(" - tmp_dir: %s\n", g_get_tmp_dir ());
	g_print(" - current_dir: %s\n", g_get_current_dir ());

	#endif

	return TRUE;
}


/* = = = = = = = = = = = = = = = = = = = = */
/* Main homebank */

static GtkWidget *
homebank_construct_splash()
{
GtkWidget *window;
GtkWidget *frame, *vbox, *image;
//gchar *ver_string, *markup, *version;
gchar *pathfilename;

	DB( g_print("\n[homebank_construct_splash]\n") );

	window = gtk_window_new(GTK_WINDOW_POPUP);	//TOPLEVEL DONT WORK
	gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);

	gtk_window_set_title (GTK_WINDOW (window), "HomeBank");
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	pathfilename = g_build_filename(homebank_app_get_images_dir(), "splash.png", NULL);
	image = gtk_image_new_from_file((const gchar *)pathfilename);
	g_free(pathfilename);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (window), frame);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	/*
	ver_string = g_strdup_printf(_("Version: HomeBank-%s"), VERSION);

	version = gtk_label_new(NULL);
	markup = g_markup_printf_escaped(MARKUP_STRING, ver_string);
	gtk_label_set_markup(GTK_LABEL(version), markup);
	g_free(markup);
	g_free(ver_string);
	*/

	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	//gtk_box_pack_start (GTK_BOX (vbox), version, FALSE, FALSE, 0);

	return window;
}

static void
homebank_init_i18n (void)
{
  /*  We may change the locale later if the user specifies a language
   *  in the gimprc file. Here we are just initializing the locale
   *  according to the environment variables and set up the paths to
   *  the message catalogs.
   */

	setlocale (LC_ALL, "");

	bindtextdomain (GETTEXT_PACKAGE, homebank_app_get_locale_dir ());
//#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
//#endif

	textdomain (GETTEXT_PACKAGE);

	/*#ifdef G_OS_WIN32
	gchar *wl = g_win32_getlocale ();
	DB( g_print(" - win32 locale is '%s'\n", wl) );
	g_free(wl);
	#endif*/

}


int
main (int argc, char *argv[])
{
GOptionContext *option_context;
GOptionGroup *option_group;
GError *error = NULL;
GtkWidget *mainwin;
GtkWidget *splash = NULL;
gboolean openlast;

	DB( g_print("\n--------------------------------" ) );
	DB( g_print("\nhomebank starting...\n" ) );

	build_package_paths();

	homebank_init_i18n ();

	/* Set up option groups */
	option_context = g_option_context_new (NULL);

	//g_option_context_set_summary (option_context, _(""));

	option_group = g_option_group_new ("homebank",
					   N_("HomeBank options"),
					   N_("HomeBank options"),
					   NULL, NULL);
	g_option_group_add_entries (option_group, option_entries);
	g_option_context_set_main_group (option_context, option_group);
	g_option_group_set_translation_domain (option_group, GETTEXT_PACKAGE);

	/* Add Gtk option group */
	g_option_context_add_group (option_context, gtk_get_option_group (FALSE));

	/* Parse command line */
	if (!g_option_context_parse (option_context, &argc, &argv, &error))
	{
		g_option_context_free (option_context);

		if (error)
		{
			g_print ("%s\n", error->message);
			g_error_free (error);
		}
		else
			g_print ("An unknown error occurred\n");

		return -1;
	}

	g_option_context_free (option_context);
	option_context = NULL;

	if (arg_version != FALSE)
	{
		/* Print version information and exit */
		g_print ("%s\n", PACKAGE " " VERSION);
		return 0;
	}

	/* Pass NULL here since we parsed the gtk+ args already...
	 * from this point all we need a DISPLAY variable to be set.
	 */
	gtk_init (NULL, NULL);

	//todo: sanity check gtk version here ?

	g_set_application_name (APPLICATION_NAME);

	if( homebank_setup() )
	{
		/*  change the locale if a language is specified  */
		language_init (PREFS->language);

		if( PREFS->showsplash == TRUE )
		{
			splash = homebank_construct_splash();
			gtk_window_set_auto_startup_notification (FALSE);
			gtk_widget_show_all (splash);
			gtk_window_set_auto_startup_notification (TRUE);

			// make sure splash is up
			while (gtk_events_pending ())
				gtk_main_iteration ();
		}

		gtk_window_set_default_icon_name ("homebank");

		DB( g_print(" - creating window\n" ) );

		mainwin = (GtkWidget *)create_hbfile_window (NULL);

		if(mainwin)
		{

			//todo: pause on splash
			if( PREFS->showsplash == TRUE )
			{
				//g_usleep( G_USEC_PER_SEC * 1 );
				gtk_widget_hide(splash);
				gtk_widget_destroy(splash);
			}


#if HB_UNSTABLE == TRUE
/*			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(mainwin),
                      GTK_DIALOG_DESTROY_WITH_PARENT,
                      GTK_MESSAGE_WARNING,
                      GTK_BUTTONS_CLOSE,
                      "This is a beta version of HomeBank (UNSTABLE)"
                      );

			gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog),
						"DO NOT USE with important files or do a backup first.\n"
						"This kind of release is for <b>TESTING ONLY</b>.\n"
						"<u>It may be buggy, crash, or lose your data</u>.\n\n"

						"For unstable bugs report, questions, suggestions:\n"
			            " - <b>DO NOT USE LaunchPad</b>\n"
			            " - direct email to: &lt;homebank@free.fr&gt;\n\n"

			            "<i>Thanks !</i>"
			            );

			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);*/
#endif

			if(GLOBALS->first_run)
			{
				ui_mainwindow_action_help_welcome();
			}


			while (gtk_events_pending ()) /* make sure splash is gone */
				gtk_main_iteration ();


			DB( g_print(" - open last file ?\n" ) );

			// load a file ?
			/* load 1st file specified on commandline */
			openlast = PREFS->loadlast;
			if (files != NULL)
			{
				if (g_file_test (files[0], G_FILE_TEST_EXISTS) != FALSE)
				{
					DB( g_print(" - should load %s\n", files[0] ) );
					hbfile_change_filepath(g_strdup(files[0]));
					ui_mainwindow_open_internal(mainwin, NULL);
					openlast = FALSE;
				}
				else
				{
					g_warning (_("Unable to open '%s', the file does not exist.\n"), files[0]);

				}
				g_strfreev (files);
			}


			DB( g_print(" - GLOBALS->xhb_filepath: '%s'\n", GLOBALS->xhb_filepath ) );

			if( openlast )
			{
				if( homebank_lastopenedfiles_load() == TRUE )
					ui_mainwindow_open_internal(mainwin, NULL);
			}

			/* -- hack to generate a big file -- */


			/* update the mainwin display */
			ui_mainwindow_update(mainwin, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_VISUAL));

		DB( g_print(" - gtk_main()\n" ) );

			gtk_main ();
		}

	}


    homebank_cleanup();

	return EXIT_SUCCESS;
}

#ifdef G_OS_WIN32
/* In case we build this as a windowed application */

#ifdef __GNUC__
#define _stdcall  __attribute__((stdcall))
#endif

int _stdcall
WinMain (struct HINSTANCE__ *hInstance,
	 struct HINSTANCE__ *hPrevInstance,
	 char               *lpszCmdLine,
	 int                 nCmdShow)
{
	return main (__argc, __argv);
}
#endif

