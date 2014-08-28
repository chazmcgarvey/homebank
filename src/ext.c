
#include <stdarg.h>
#include <string.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include "ext.h"

extern struct Preferences *PREFS;


const int _hook_recursion_soft_limit = 50;
const int _hook_recursion_hard_limit = 99;


struct PluginEngine
{
	const gchar*            type;
	PluginEngineInitializer init;
	PluginEngineTerminator  term;
	PluginEngineFileChecker check_file;
	PluginMetadataReader    read_metadata;
	PluginLoader            load_plugin;
	PluginUnloader          unload_plugin;
	PluginExecutor          execute;
	PluginHookCaller        call_hook;
};


static GList* _engine_list = NULL;
static GHashTable* _loaded_plugins = NULL;


void ext_init(int* argc, char** argv[], char** env[])
{
	GList *list = g_list_first(_engine_list);
	while (list)
	{
		struct PluginEngine* engine = list->data;
		engine->init(argc, argv, env);
		list = g_list_next(list);
	}
	if (!_loaded_plugins) {
		_loaded_plugins = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	}
}

void ext_term(void)
{
	GList *list = g_list_first(_engine_list);
	while (list)
	{
		struct PluginEngine* engine = list->data;
		engine->term();
		list = g_list_next(list);
	}
	g_list_free(_engine_list);
	_engine_list = NULL;

	if (_loaded_plugins) {
		g_hash_table_unref(_loaded_plugins);
		_loaded_plugins = NULL;
	}
}

void ext_register(const gchar* type,
		PluginEngineInitializer init,
		PluginEngineTerminator term,
		PluginEngineFileChecker check_file,
		PluginMetadataReader read_metadata,
		PluginLoader load_plugin,
		PluginUnloader unload_plugin,
		PluginExecutor execute,
		PluginHookCaller call_hook)
{
	struct PluginEngine* engine = g_malloc0(sizeof(struct PluginEngine));
	engine->type = type;
	engine->init = init;
	engine->term = term;
	engine->check_file = check_file;
	engine->read_metadata = read_metadata;
	engine->load_plugin = load_plugin;
	engine->unload_plugin = unload_plugin;
	engine->execute = execute;
	engine->call_hook = call_hook;
    _engine_list = g_list_append(_engine_list, engine);
}


static struct PluginEngine* _get_engine_for_plugin(const gchar* plugin_filename)
{
	if (!plugin_filename) {
		return NULL;
	}

	GList *list = g_list_first(_engine_list);
	while (list) {
		struct PluginEngine* engine = list->data;
		if (engine->check_file(plugin_filename)) {
			return engine;
		}
		list = g_list_next(list);
	}
	return NULL;
}

static void _read_directory(const gchar* directory, GHashTable* hash)
{
	GDir* dir = g_dir_open(directory, 0, NULL);
	if (!dir) return;

	const gchar* filename;
	while ((filename = g_dir_read_name(dir))) {
		gchar* full = g_build_filename(directory, filename, NULL);
		if (g_file_test(full, G_FILE_TEST_IS_REGULAR) && _get_engine_for_plugin(filename)) {
			g_hash_table_insert(hash, g_strdup(filename), NULL);
		}
		g_free(full);
	}
	g_dir_close(dir);
}

gchar** ext_list_plugins()
{
	GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);

	gchar** it;
	for (it = PREFS->ext_path; it && *it; ++it) {
		_read_directory(*it, hash);
	}

	GList* list = g_list_sort(g_hash_table_get_keys(hash), (GCompareFunc)g_utf8_collate);
	g_hash_table_unref(hash);

	guint len = g_list_length(list);
	gchar** strv = g_new0(gchar**, len + 1);
	int i;
	for (i = 0; i < len; ++i) {
		strv[i] = g_list_nth_data(list, i);
	}
	g_list_free(list);

	return strv;
}

gchar* ext_find_plugin(const gchar* plugin_filename)
{
	if (!plugin_filename) return NULL;

	gchar** it;
	for (it = PREFS->ext_path; *it; ++it) {
		if (!g_path_is_absolute(*it)) continue;

		gchar* full = g_build_filename(*it, plugin_filename, NULL);
		if (g_file_test(full, G_FILE_TEST_IS_REGULAR)) {
			return full;
		}
		g_free(full);
	}

	return NULL;
}

GHashTable* ext_read_plugin_metadata(const gchar* plugin_filename)
{
	gchar* full = ext_find_plugin(plugin_filename);
	if (!full) return NULL;

	GHashTable* ret = NULL;

	struct PluginEngine* engine = _get_engine_for_plugin(plugin_filename);
	if (engine && engine->read_metadata) {
		ret = engine->read_metadata(full);
	}

	g_free(full);
	return ret;
}

gint ext_load_plugin(const gchar* plugin_filename)
{
	gchar* full = ext_find_plugin(plugin_filename);
	if (!full) return -1;

	gint ret = -1;

	struct PluginEngine* engine = _get_engine_for_plugin(plugin_filename);
	if (engine && engine->load_plugin && engine->load_plugin(full) == 0) {
		g_hash_table_insert(_loaded_plugins, g_strdup(plugin_filename), NULL);
		ret = 0;
	}

	g_free(full);
	return ret;
}

void ext_unload_plugin(const gchar* plugin_filename)
{
	gchar* full = ext_find_plugin(plugin_filename);
	if (!full) return;

	struct PluginEngine* engine = _get_engine_for_plugin(plugin_filename);
	if (engine && engine->unload_plugin) {
		engine->unload_plugin(full);
	}

	g_free(full);
	g_hash_table_remove(_loaded_plugins, plugin_filename);
}

gboolean ext_is_plugin_loaded(const gchar* plugin_filename)
{
	return g_hash_table_contains(_loaded_plugins, plugin_filename);
}

void ext_execute_action(const gchar* plugin_filename)
{
	gchar* full = ext_find_plugin(plugin_filename);
	if (!full) return;

	struct PluginEngine* engine = _get_engine_for_plugin(plugin_filename);
	if (engine && engine->execute) {
		engine->execute(full);
	}

	g_free(full);
}

void ext_hook(const gchar* hook_id, ...)
{
	GList *list = NULL;

	va_list ap;
	va_start(ap, hook_id);
	for (;;) {
		GValue* val = (GValue*)va_arg(ap, GValue*);
		if (!val) break;
		list = g_list_append(list, val);
	}
	va_end(ap);

	ext_vhook(hook_id, list);
	g_list_free(list);
}

void ext_vhook(const gchar* hook_id, GList* args)
{
	static int recursion_level = 0;

	if (_hook_recursion_hard_limit <= recursion_level) {
		return;
	} else if (_hook_recursion_soft_limit <= recursion_level) {
		int level = recursion_level;
		recursion_level = -1;
		GValue val_level = G_VALUE_INIT;
		ext_hook("deep_hook_recursion", EXT_INT(&val_level, level), NULL);
		recursion_level = level;
	}

	++recursion_level;

	g_print("ext_hook: %s (level %d)\n", hook_id, recursion_level);
	GList *list = g_list_first(_engine_list);
	while (list)
	{
		struct PluginEngine* engine = list->data;
		engine->call_hook(hook_id, args);
		list = g_list_next(list);
	}

	--recursion_level;
}

gboolean ext_has(const gchar* feature)
{
#ifdef OFX_ENABLE
	if (0 == g_utf8_collate(feature, "libofx")) {
		return TRUE;
	}
#endif
#ifdef PERL_ENABLE
	if (0 == g_utf8_collate(feature, "perl")) {
		return TRUE;
	}
#endif
	return FALSE;
}


void* ext_symbol_lookup(const gchar* symbol)
{
	static GModule* module = NULL;
	if (!module) module = g_module_open(NULL, 0);

	void* ptr;
	if (module && g_module_symbol(module, symbol, &ptr)) {
		return ptr;
	}

	return NULL;
}


void ext_run_modal(const gchar* title, const gchar* text, const gchar* type)
{
	GtkMessageType t = GTK_MESSAGE_INFO;
	if (0 == g_utf8_collate(type, "error")) {
		t = GTK_MESSAGE_ERROR;
	}
	if (0 == g_utf8_collate(type, "warn")) {
		t = GTK_MESSAGE_WARNING;
	}
	if (0 == g_utf8_collate(type, "question")) {
		t = GTK_MESSAGE_QUESTION;
	}

	GtkWidget* dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT, t,
			GTK_BUTTONS_CLOSE, "%s", text);
	if (title) {
		gtk_window_set_title(GTK_WINDOW(dialog), title);
	}

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

