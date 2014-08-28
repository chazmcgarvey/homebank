
#include <gmodule.h>

#include "ext.h"


const gchar* metadata[] = {
"NAME:     Some Native Plugin",
"VERSION:  0.0105",
"ABSTRACT: Native plugins are also possible.",
"AUTHOR:   Charles McGarvey <chazmcgarvey@brokenzipper.com>",
"WEBSITE:  http://acme.tld/",
};


G_MODULE_EXPORT void load(void);
G_MODULE_EXPORT void unload(void);
G_MODULE_EXPORT void execute(void);

G_MODULE_EXPORT void on_create_main_window(GList* args);
G_MODULE_EXPORT void on_enter_main_loop(GList* args);


G_MODULE_EXPORT void load()
{
	g_print("loading native plugin....... %p\n", load);
}

G_MODULE_EXPORT void unload()
{
	g_print("destroy native plugin....... %p\n", unload);
}

G_MODULE_EXPORT void execute()
{
	g_print("Configuring that native plugin!!!\n");
}

static GtkWidget* win = NULL;

G_MODULE_EXPORT void on_create_main_window(GList* args)
{
	GList* it = g_list_first(args);
	win = g_value_get_object(it->data);
	/*gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), "This is the native hello-world plugin!");*/
}

G_MODULE_EXPORT void on_enter_main_loop(GList* args)
{
	g_print("setting main window title.....\n");
	if (win) {
		gtk_window_set_title(GTK_WINDOW(win), "This is the native hello-world plugin!");
	} else {
		g_printerr("the main window is not set :(\n");
	}
}

