
#ifndef __EXT_H__
#define __EXT_H__

#include <glib.h>

#include "ext-value.h"


typedef gint (*PluginEngineInitializer)(int* argc, char** argv[], char** env[]);
typedef void (*PluginEngineTerminator)();
typedef gboolean (*PluginEngineFileChecker)(const gchar* plugin_filepath);
typedef GHashTable* (*PluginMetadataReader)(const gchar* plugin_filepath);
typedef gint (*PluginLoader)(const gchar* plugin_filepath);
typedef void (*PluginUnloader)(const gchar* plugin_filepath);
typedef void (*PluginExecutor)(const gchar* plugin_filepath);
typedef void (*PluginHookCaller)(const gchar* hook_id, GList* args);

void ext_init(int* argc, char** argv[], char** env[]);
void ext_term(void);
void ext_register(const gchar* type,
		PluginEngineInitializer,
		PluginEngineTerminator,
		PluginEngineFileChecker,
		PluginMetadataReader,
		PluginLoader,
		PluginUnloader,
		PluginExecutor,
		PluginHookCaller);


gchar**  ext_list_plugins(void);
gchar*   ext_find_plugin(const gchar* plugin_filename);
GHashTable* ext_read_plugin_metadata(const gchar* plugin_filename);
gint     ext_load_plugin(const gchar* plugin_filename);
void     ext_unload_plugin(const gchar* plugin_filename);
gboolean ext_is_plugin_loaded(const gchar* plugin_filename);
void     ext_execute_action(const gchar* plugin_filename);
void     ext_hook(const gchar* hook_id, ...);
void     ext_vhook(const gchar* hook_id, GList* args);
gboolean ext_has(const gchar* feature);
void*    ext_symbol_lookup(const gchar* symbol);
void     ext_run_modal(const gchar* title, const gchar* text, const gchar* type);

#endif

