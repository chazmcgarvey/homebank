
#include <glib/gstdio.h>
#include <gmodule.h>

#include "ext.h"


static gint ext_native_init(int* argc, char** argv[], char** env[]);
static void ext_native_term(void);
static gboolean ext_native_check_file(const gchar* plugin_filename);
static GHashTable* ext_native_read_plugin_metadata(const gchar* plugin_filepath);
static gint ext_native_load_plugin(const gchar* plugin_filepath);
static void ext_native_unload_plugin(const gchar* plugin_filepath);
static void ext_native_execute_action(const gchar* plugin_filepath);
static void ext_native_call_hook(const gchar* hook_id, GList* args);

static gchar* _read_data_for_keyword(const gchar* keyword, const gchar* bytes, gsize len);


static GHashTable* _loaded_plugins = NULL;


static gint ext_native_init(int* argc, char** argv[], char** env[])
{
	if (!_loaded_plugins) {
		_loaded_plugins = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_module_close);
	}
	return 0;
}

static void ext_native_term(void)
{
	if (_loaded_plugins) {
		ext_native_call_hook("unload", NULL);
		g_hash_table_unref(_loaded_plugins);
		_loaded_plugins = NULL;
	}
}

static gboolean ext_native_check_file(const gchar* plugin_filename)
{
	if (g_str_has_suffix(plugin_filename, "."G_MODULE_SUFFIX)) {
		return TRUE;
	}
	if (g_str_has_suffix(plugin_filename, ".la")) {
		// allow a .la file only if no actual native plugin is found
		gboolean check = FALSE;
		gchar* copy = g_strdup(plugin_filename);
		gchar* ext  = g_strrstr(copy, ".la");
		if (ext) {
			*ext = '\0';
			gchar* native_filename = g_strconcat(copy, "."G_MODULE_SUFFIX, NULL);
			gchar* native_filepath = ext_find_plugin(native_filename);
			check = !native_filepath;
			g_free(native_filepath);
			g_free(native_filename);
		}
		g_free(copy);
		return check;
	}
	return FALSE;
}

static GHashTable* ext_native_read_plugin_metadata(const gchar* plugin_filepath)
{
	GMappedFile* file = g_mapped_file_new(plugin_filepath, FALSE, NULL);
	if (!file) {
		g_printerr("mapping plugin file at %s failed\n", plugin_filepath);
		return NULL;
	}

	gchar* bytes = g_mapped_file_get_contents(file);
	gsize len = g_mapped_file_get_length(file);
	if (len == 0 || !bytes) {
		g_mapped_file_unref(file);
		g_printerr("no data in plugin file at %s failed\n", plugin_filepath);
		return NULL;
	}

	GHashTable* table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	const gchar* keywords[] = { "name", "version", "abstract", "author", "website", NULL };
	const gchar** it;
	for (it = keywords; *it; ++it) {
		gchar* value = _read_data_for_keyword(*it, bytes, len);
		g_hash_table_insert(table, g_strdup(*it), value);
	}

	g_mapped_file_unref(file);

	return table;
}

static gint ext_native_load_plugin(const gchar* plugin_filepath)
{
	if (g_hash_table_contains(_loaded_plugins, plugin_filepath)) {
		return 0;
	}

	GModule* module = g_module_open(plugin_filepath, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if (!module) {
		g_printerr("Could not load native plugin: %s\n", g_module_error());
		return -1;
	}

	g_hash_table_insert(_loaded_plugins, g_strdup(plugin_filepath), module);

	void (*symbol)();
	if (g_module_symbol(module, "load", (gpointer)&symbol)) {
		symbol();
	}

	return 0;
}

static void ext_native_unload_plugin(const gchar* plugin_filepath)
{
	GModule* module = g_hash_table_lookup(_loaded_plugins, plugin_filepath);
	if (module) {
		void (*symbol)();
		if (g_module_symbol(module, "unload", (gpointer)&symbol)) {
			symbol();
		}
	}

	g_hash_table_remove(_loaded_plugins, plugin_filepath);
}

static void ext_native_execute_action(const gchar* plugin_filepath)
{
	GModule* module = g_hash_table_lookup(_loaded_plugins, plugin_filepath);
	if (module) {
		void (*symbol)();
		if (g_module_symbol(module, "execute", (gpointer)&symbol)) {
			symbol();
		}
	}
}

static void ext_native_call_hook(const gchar* hook_id, GList* args)
{
	gchar* symbol_name = g_strconcat("on_", hook_id, NULL);
	void (*symbol)(GList*);

	GHashTableIter it;
	g_hash_table_iter_init(&it, _loaded_plugins);
	GModule* module;

	while (g_hash_table_iter_next(&it, NULL, (gpointer*)&module)) {
		if (g_module_symbol(module, symbol_name, (gpointer)&symbol)) {
			symbol(args);
		}
	}

	g_free(symbol_name);
}


static gchar* _read_data_for_keyword(const gchar* keyword, const gchar* bytes, gsize len)
{
	gchar* value = NULL;

	gchar* pattern = g_strdup_printf("[\\x00\\t\\n ]%s\\s*[=:]\\s*([^\\x00]+)", keyword);
	GRegex* r = g_regex_new(pattern, G_REGEX_CASELESS, 0, NULL);
	g_free(pattern);

	GMatchInfo* match = NULL;
	if (g_regex_match_full(r, bytes, len, 0, 0, &match, NULL)) {
		value = g_match_info_fetch(match, 1);
	}

	g_match_info_free(match);
	g_regex_unref(r);

	return value;
}


static void _register(void) __attribute__((constructor));
static void _register()
{
	ext_register("native",
			ext_native_init,
			ext_native_term,
			ext_native_check_file,
			ext_native_read_plugin_metadata,
			ext_native_load_plugin,
			ext_native_unload_plugin,
			ext_native_execute_action,
			ext_native_call_hook);
}

