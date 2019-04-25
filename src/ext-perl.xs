
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <string.h>

#undef _
#include "homebank.h"
#include "ext.h"
#include "refcount.h"

extern struct HomeBank *GLOBALS;
#include "dsp-mainwindow.h"
#include "dsp-account.h"
#include "ui-transaction.h"


static gint ext_perl_init(int* argc, char** argv[], char** env[]);
static void ext_perl_term(void);
static gboolean ext_perl_check_file(const gchar* plugin_filepath);
static GHashTable* ext_perl_read_plugin_metadata(const gchar* plugin_filepath);
static gint ext_perl_load_plugin(const gchar* plugin_filepath);
static void ext_perl_unload_plugin(const gchar* plugin_filepath);
static void ext_perl_execute_action(const gchar* plugin_filepath);
static void ext_perl_call_hook(const gchar* hook_id, GList* args);

static SV* val_to_sv(GValue* val);
static GValue* sv_to_val(SV* sv);

static gboolean gperl_value_from_sv(GValue* value, SV* sv);
static SV*      gperl_sv_from_value(const GValue* value, gboolean copy_boxed);


static inline GValue* EXT_SV(GValue* v, SV* sv, GType type)
{
	g_value_init(v, type);
	gperl_value_from_sv(v, sv);
	return v;
}


#define EXT_P2C_OBJECT(PKG, ARG, VAR, TYP)  \
if (sv_derived_from(ARG, PKG)) {            \
    IV iv = SvIV((SV*)SvRV(ARG));           \
    VAR = INT2PTR(TYP, iv);                 \
} else {                                    \
    croak(#VAR" is not of type "PKG);       \
}

#define EXT_C2P_OBJECT(PKG, ARG, VAR)       \
sv_setref_pv(ARG, PKG, (void*)VAR)


static inline GPtrArray* SvGptrarray(const SV* sv)
{
	if (SvROK(sv)) {
		sv = MUTABLE_SV(SvRV(sv));
	}
	if (SvTYPE(sv) == SVt_PVAV) {
		AV* av = (AV*)sv;
		int i;
		int top = av_len(av);
		GPtrArray* array = g_ptr_array_new();
		for (i = 0; i <= top; ++i) {
			SV** item = av_fetch(av, i, 0);
			if (!item) continue;
			g_ptr_array_add(array, sv_to_val(*item));
		}
		return array;
		// TODO- leaking
	} else {
		croak("var is not an array");
	}
}

static inline SV* newSVgptrarray(const GPtrArray* a)
{
	if (a) {
		AV* av = newAV();
		int i;
		for (i = 0; i < a->len; ++i) {
			GValue* item = g_ptr_array_index(a, i);
			av_push(av, val_to_sv(item));
		}
		return newRV((SV*)av);
	}
	return &PL_sv_undef;
}


static inline GHashTable* SvGhashtable(const SV* sv)
{
	if (SvROK(sv)) {
		sv = MUTABLE_SV(SvRV(sv));
	}
	if (SvTYPE(sv) == SVt_PVHV) {
		HV* hv = (HV*)sv;
		hv_iterinit(hv);
		gchar* key;
		I32 len;
		SV* item;
		GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);
		while ((item = hv_iternextsv(hv, &key, &len))) {
			g_hash_table_insert(hash, key, sv_to_val(item));
		}
		return hash;
		// TODO- leaking
	} else {
		croak("var is not a hash");
	}
}

static inline SV* newSVghashtable(GHashTable* h)
{
	if (h) {
		HV* hv = newHV();
		GHashTableIter it;
		g_hash_table_iter_init(&it, h);
		gchar* key = NULL;
		GValue* item = NULL;
		while (g_hash_table_iter_next(&it, (gpointer*)&key, (gpointer*)&item)) {
			hv_store(hv, key, -g_utf8_strlen(key, -1), val_to_sv(item), 0);
		}
		return newRV((SV*)hv);
	}
	return &PL_sv_undef;
}


static inline gboolean SvGboolean(SV* sv)
{
	if (!sv) {
		return FALSE;
	}
	if (SvROK(sv)) {
		return !!SvIV(SvRV(sv));
	} else {
		return SvTRUE(sv);
	}
}

static inline SV* newSVgboolean(gboolean b)
{
	return sv_setref_iv(newSV(0), "HomeBank::Boolean", !!b);
}


static inline gchar* SvGchar_ptr(SV* sv)
{
	return SvPVutf8_nolen(sv);
}

static inline SV* newSVgchar_ptr(const gchar* str)
{
	if (!str) return &PL_sv_undef;

	SV* sv = newSVpv(str, 0);
	SvUTF8_on(sv);
	return sv;
}


static inline GObject* SvGobject(const SV* sv)
{
	GObject* (*func)(const SV*) = ext_symbol_lookup("gperl_get_object");
	if (func) {
		return func(sv);
	}
	return NULL;
}

static inline SV* newSVgobject(const GObject* o)
{
	SV* (*func)(const GObject*, gboolean) = ext_symbol_lookup("gperl_new_object");
	if (func) {
		return func(o, FALSE);
	}
	return &PL_sv_undef;
}


static PerlInterpreter* context = NULL;


static gint ext_perl_init(int* argc, char** argv[], char** env[])
{
	int ret = 0;

	PERL_SYS_INIT3(argc, argv, env);
	context = perl_alloc();
	perl_construct(context);

	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
	PL_origalen = 1;
	PL_perl_destruct_level = 1;

	gchar* bootstrap = g_strdup_printf("-e"
		"use lib '%s';"
		"use HomeBank;"
		"HomeBank->bootstrap;",
		homebank_app_get_pkglib_dir());
	char *args[] = { "", bootstrap };

	EXTERN_C void xs_init(pTHX);
	if (perl_parse(context, xs_init, 2, args, NULL) || perl_run(context)) {
		ext_perl_term();
		ret = -1;
	}

	g_free(bootstrap);
	return ret;
}

static void ext_perl_term(void)
{
	if (context) {
		perl_destruct(context);
		perl_free(context);
		context = NULL;
	}
	PERL_SYS_TERM();
}

static gboolean ext_perl_check_file(const gchar* plugin_filepath)
{
	if (g_str_has_suffix(plugin_filepath, ".pl")) {
		return TRUE;
	}
	return FALSE;
}

static GHashTable* ext_perl_read_plugin_metadata(const gchar* plugin_filepath)
{
	GHashTable* table = NULL;

	if (!context) return NULL;
	PERL_SET_CONTEXT(context);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	mXPUSHs(newSVgchar_ptr(plugin_filepath));
	PUTBACK;

	int ret = call_pv("HomeBank::read_metadata", G_SCALAR | G_EVAL);

	SPAGAIN;

	if (ret == 1) {
		table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		SV* sv = POPs;
		if (SvROK(sv)) {
			sv = MUTABLE_SV(SvRV(sv));
		}
		if (SvTYPE(sv) == SVt_PVHV) {
			HV* hv = (HV*)sv;
			hv_iterinit(hv);
			gchar* key;
			I32 len;
			SV* item;
			while ((item = hv_iternextsv(hv, &key, &len))) {
				if (SvPOK(item)) {
					gchar* val = SvPVutf8_nolen(item);
					g_hash_table_insert(table, g_strdup(key), g_strdup(val));
				}
			}
		}
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return table;
}

static gint ext_perl_load_plugin(const gchar* plugin_filepath)
{
	if (!context) return -1;
	PERL_SET_CONTEXT(context);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	mXPUSHs(newSVgchar_ptr(plugin_filepath));
	PUTBACK;
	call_pv("HomeBank::load_plugin", G_DISCARD | G_EVAL);
	SPAGAIN;

	gint ret = 0;
	if (SvTRUE(ERRSV)) {
		g_printerr("%s", SvPV_nolen(ERRSV));
		ret = -1;
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return ret;
}

static void ext_perl_unload_plugin(const gchar* plugin_filepath)
{
	if (!context) return;
	PERL_SET_CONTEXT(context);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	mXPUSHs(newSVgchar_ptr(plugin_filepath));
	PUTBACK;
	call_pv("HomeBank::unload_plugin", G_DISCARD | G_EVAL);
	SPAGAIN;

	if (SvTRUE(ERRSV)) {
		g_printerr("%s", SvPV_nolen(ERRSV));
	}

	PUTBACK;
	FREETMPS;
	LEAVE;
}

static void ext_perl_execute_action(const gchar* plugin_filepath)
{
	if (!context) return;
	PERL_SET_CONTEXT(context);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	mXPUSHs(newSVgchar_ptr(plugin_filepath));
	PUTBACK;
	call_pv("HomeBank::execute_action", G_DISCARD | G_EVAL);
	SPAGAIN;

	if (SvTRUE(ERRSV)) {
		g_printerr("%s", SvPV_nolen(ERRSV));
	}

	PUTBACK;
	FREETMPS;
	LEAVE;
}

static void ext_perl_call_hook(const gchar* hook_id, GList* args)
{
	if (!context) return;
	PERL_SET_CONTEXT(context);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	mXPUSHs(newSVgchar_ptr(hook_id));

	GList *list = g_list_first(args);
	while (list) {
		GValue* val = list->data;
		XPUSHs(sv_2mortal(val_to_sv(val)));
		list = g_list_next(list);
	}

	PUTBACK;
	call_pv("HomeBank::call_hook", G_ARRAY);
	SPAGAIN;
	POPi;
	PUTBACK;
	FREETMPS;
	LEAVE;
}


static SV* val_to_sv(GValue* val)
{
	if (!val || !G_IS_VALUE(val) || G_VALUE_TYPE(val) == G_TYPE_NONE) {
		return &PL_sv_undef;
	}
	if (G_VALUE_TYPE(val) == G_TYPE_BOOLEAN) {
		return newSVgboolean(g_value_get_boolean(val));
	}
	if (G_VALUE_TYPE(val) == G_TYPE_PTR_ARRAY) {
		return newSVgptrarray((GPtrArray*)g_value_get_boxed(val));
	}
	if (G_VALUE_TYPE(val) == G_TYPE_HASH_TABLE) {
		return newSVghashtable((GHashTable*)g_value_get_boxed(val));
	}
#define obj(CTYPE, _2, PART, GTYPE, _5)                         \
	if (G_VALUE_TYPE(val) == GTYPE) {                           \
		SV* sv = newSV(0);                                      \
		CTYPE* ptr = (CTYPE*)g_value_get_##PART(val);           \
		EXT_C2P_OBJECT("HomeBank::"#CTYPE, sv, rc_ref(ptr));    \
		return sv;                                              \
	}
#include "ext-value.h"
#undef obj
	return gperl_sv_from_value(val, FALSE);
}

static GValue* sv_to_val(SV* sv)
{
	GValue* val = g_new0(GValue, 1);

	if (SvUOK(sv)) return EXT_SV(val, sv, G_TYPE_UINT);
	if (SvIOK(sv)) return EXT_SV(val, sv, G_TYPE_INT);
	if (SvNOK(sv)) return EXT_SV(val, sv, G_TYPE_DOUBLE);
	if (SvPOK(sv)) return EXT_SV(val, sv, G_TYPE_STRING);
	if (sv_isobject(sv)) {
		if (sv_derived_from(sv, "HomeBank::Boolean")) {
			return EXT_BOOLEAN(val, SvGboolean(sv));
		}
#define obj(CTYPE, NAME, _3, _4, _5)                                \
		if (sv_derived_from(sv, "HomeBank::"#CTYPE)) {              \
			CTYPE* ptr;                                             \
			EXT_P2C_OBJECT("HomeBank::"#CTYPE, sv, ptr, CTYPE*);    \
			return EXT_##NAME(val, ptr);                            \
		}
#include "ext-value.h"
#undef obj
		return EXT_SV(val, sv, G_TYPE_OBJECT);
	}
	if (SvROK(sv)) {
		sv = SvRV(sv);
		switch (SvTYPE(sv)) {
			case SVt_IV:
				return EXT_BOOLEAN(val, SvGboolean(sv));
			case SVt_PVAV:
				return EXT_ARRAY(val, SvGptrarray(sv));
			case SVt_PVHV:
				return EXT_HASH_TABLE(val, SvGhashtable(sv));
			default:
				break;
		}
	}
	switch (SvTYPE(sv)) {
		case SVt_PVAV:
			return EXT_ARRAY(val, SvGptrarray(sv));
		case SVt_PVHV:
			return EXT_HASH_TABLE(val, SvGhashtable(sv));
		default:
			break;
	}

	g_free(val);
	return NULL;
}


static gboolean gperl_value_from_sv(GValue* value, SV* sv)
{
	gboolean (*func)(GValue*, SV*) = ext_symbol_lookup("gperl_value_from_sv");
	if (func) return func(value, sv);

	GType type = G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value));
	if (!SvOK(sv)) return TRUE;
	switch (type) {
		case G_TYPE_CHAR:
		{
			gchar *tmp = SvGchar_ptr(sv);
			g_value_set_schar(value, (gint8)(tmp ? tmp[0] : 0));
			break;
		}
		case G_TYPE_UCHAR:
		{
			char *tmp = SvPV_nolen(sv);
			g_value_set_uchar(value, (guchar)(tmp ? tmp[0] : 0));
			break;
		}
		case G_TYPE_BOOLEAN:
			g_value_set_boolean(value, SvTRUE(sv));
			break;
		case G_TYPE_INT:
			g_value_set_int(value, SvIV(sv));
			break;
		case G_TYPE_UINT:
			g_value_set_uint(value, SvIV(sv));
			break;
		case G_TYPE_LONG:
			g_value_set_long(value, SvIV(sv));
			break;
		case G_TYPE_ULONG:
			g_value_set_ulong(value, SvIV(sv));
			break;
		case G_TYPE_FLOAT:
			g_value_set_float(value, (gfloat)SvNV(sv));
			break;
		case G_TYPE_DOUBLE:
			g_value_set_double(value, SvNV(sv));
			break;
		case G_TYPE_STRING:
			g_value_set_string(value, SvGchar_ptr(sv));
			break;
	}
	return TRUE;
}

static SV* gperl_sv_from_value(const GValue* value, gboolean copy_boxed)
{
	SV* (*func)(const GValue*, gboolean) = ext_symbol_lookup("gperl_sv_from_value");
	if (func) return func(value, copy_boxed);

	GType type = G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value));
	switch (type) {
		case G_TYPE_CHAR:
			return newSViv(g_value_get_schar(value));
		case G_TYPE_UCHAR:
			return newSVuv(g_value_get_uchar(value));
		case G_TYPE_BOOLEAN:
			return newSViv(g_value_get_boolean(value));
		case G_TYPE_INT:
			return newSViv(g_value_get_int(value));
		case G_TYPE_UINT:
			return newSVuv(g_value_get_uint(value));
		case G_TYPE_LONG:
			return newSViv(g_value_get_long(value));
		case G_TYPE_ULONG:
			return newSVuv(g_value_get_ulong(value));
		case G_TYPE_FLOAT:
			return newSVnv(g_value_get_float(value));
		case G_TYPE_DOUBLE:
			return newSVnv(g_value_get_double(value));
		case G_TYPE_STRING:
			return newSVgchar_ptr(g_value_get_string(value));
	}
	return &PL_sv_undef;
}


static void _register(void) __attribute__((constructor));
static void _register()
{
	ext_register("perl",
			ext_perl_init,
			ext_perl_term,
			ext_perl_check_file,
			ext_perl_read_plugin_metadata,
			ext_perl_load_plugin,
			ext_perl_unload_plugin,
			ext_perl_execute_action,
			ext_perl_call_hook);
}


MODULE = HomeBank  PACKAGE = HomeBank

PROTOTYPES: ENABLE

const gchar*
version(void)
	CODE:
		RETVAL = VERSION;
	OUTPUT:
		RETVAL

const gchar*
config_dir(void)
	CODE:
		RETVAL = homebank_app_get_config_dir();
	OUTPUT:
		RETVAL

gboolean
has(const gchar* CLASS, ...)
	PREINIT:
		int i;
	CODE:
		PERL_UNUSED_ARG(CLASS);
		RETVAL = TRUE;
		for (i = 1; i < items; ++i) {
			gchar* feature = SvGchar_ptr(ST(i));
			if (!feature || !ext_has(feature)) {
				RETVAL = FALSE;
				break;
			}
		}
	OUTPUT:
		RETVAL

GObject*
main_window(void)
	CODE:
		RETVAL = G_OBJECT(GLOBALS->mainwindow);
	OUTPUT:
		RETVAL

GObject*
main_ui_manager(void)
	PREINIT:
		struct hbfile_data *data;
	CODE:
		RETVAL = NULL;
		if (GLOBALS->mainwindow) {
			data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GLOBALS->mainwindow, GTK_TYPE_WINDOW)), "inst_data");
			if (data) {
				RETVAL = G_OBJECT(data->manager);
			}
		}
	OUTPUT:
		RETVAL

void
info(const gchar* CLASS, const gchar* title, const gchar* text)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		ext_run_modal(title, text, "info");

void
warn(const gchar* CLASS, const gchar* title, const gchar* text)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		ext_run_modal(title, text, "warn");

void
error(const gchar* CLASS, const gchar* title, const gchar* text)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		ext_run_modal(title, text, "error");

void
hook(const gchar* CLASS, const gchar* hook_name, ...)
	PREINIT:
		int i;
		GList* list = NULL;
	CODE:
		PERL_UNUSED_ARG(CLASS);
		for (i = 2; i < items; ++i) {
			SV* sv = ST(i);
			GValue *val = sv_to_val(sv);
			list = g_list_append(list, val);
		}
	CLEANUP:
		ext_vhook(hook_name, list);
		g_list_free(list);
		// TODO free all the things

GObject*
open_prefs(const gchar* CLASS)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		RETVAL = G_OBJECT(defpref_dialog_new(PREF_GENERAL));
	OUTPUT:
		RETVAL


MODULE = HomeBank  PACKAGE = HomeBank::File

const gchar*
owner(const gchar* CLASS, ...)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		if (1 < items) {
			hbfile_change_owner(g_strdup(SvGchar_ptr(ST(1))));
		}
		RETVAL = GLOBALS->owner;
	OUTPUT:
		RETVAL

void
transactions(const gchar* CLASS)
	PPCODE:
		PERL_UNUSED_ARG(CLASS);

		GList* acc_list = g_hash_table_get_values(GLOBALS->h_acc);
		GList* acc_link = g_list_first(acc_list);
		for (; acc_link; acc_link = g_list_next(acc_link)) {
			Account *acc = acc_link->data;

			GList* txn_link = g_queue_peek_head_link(acc->txn_queue);
			for (; txn_link; txn_link = g_list_next(txn_link)) {
				Transaction* txn = txn_link->data;

				GValue val = G_VALUE_INIT;
				SV* sv = val_to_sv(EXT_TRANSACTION(&val, txn));
				mXPUSHs(sv);
			}
		}

		g_list_free(acc_list);

void
anonymize(void)
	CODE:
		hbfile_anonymize();

void
baz(const gchar* CLASS, Account* account)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		g_print("hello: %s\n", account->name);

GPtrArray*
meh(const gchar* CLASS, GPtrArray* asdf)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		g_print("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
		if (asdf) {
			;
		} else {
			g_print("the array is nil\n");
		}
		RETVAL = asdf;
	OUTPUT:
		RETVAL
	CLEANUP:
		g_ptr_array_unref(asdf);

GHashTable*
foo(const gchar* CLASS, GHashTable* asdf)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		g_print("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
		if (asdf) {
			GHashTableIter it;
			g_hash_table_iter_init(&it, asdf);
			gchar* key = NULL;
			GValue* item = NULL;
			while (g_hash_table_iter_next(&it, (gpointer*)&key, (gpointer*)&item)) {
				g_print("hash with key: %s\n", key);
			}
		} else {
			g_print("the hash is nil\n");
		}
		RETVAL = asdf;
	OUTPUT:
		RETVAL
	CLEANUP:
		g_hash_table_unref(asdf);


MODULE = HomeBank  PACKAGE = HomeBank::Account

void
compute_balances(const gchar* CLASS)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		account_compute_balances();

Account*
new(void)
	CODE:
		RETVAL = da_acc_malloc();
	OUTPUT:
		RETVAL

void
DESTROY(Account* SELF)
	CODE:
		da_acc_free(SELF);

Account*
get(const gchar* CLASS, guint key)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		RETVAL = rc_ref(da_acc_get(key));
	OUTPUT:
		RETVAL

Account*
get_by_name(const gchar* CLASS, const gchar* name)
	CODE:
		PERL_UNUSED_ARG(CLASS);
		RETVAL = rc_ref(da_acc_get_by_name((gchar*)name));
	OUTPUT:
		RETVAL

const gchar*
name(Account* SELF, ...)
	CODE:
		if (1 < items) {
			account_rename(SELF, SvGchar_ptr(ST(1)));
		}
		RETVAL = SELF->name;
	OUTPUT:
		RETVAL

const gchar*
number(Account* SELF, ...)
	CODE:
		if (1 < items) {
			g_free(SELF->number);
			SELF->number = g_strdup(SvGchar_ptr(ST(1)));
		}
		RETVAL = SELF->number;
	OUTPUT:
		RETVAL

const gchar*
bankname(Account* SELF, ...)
	CODE:
		if (1 < items) {
			g_free(SELF->bankname);
			SELF->bankname = g_strdup(SvGchar_ptr(ST(1)));
		}
		RETVAL = SELF->bankname;
	OUTPUT:
		RETVAL

gdouble
initial(Account* SELF, ...)
	CODE:
		if (1 < items) {
			SELF->initial = SvNV(ST(1));
		}
		RETVAL = SELF->initial;
	OUTPUT:
		RETVAL

gdouble
minimum(Account* SELF, ...)
	CODE:
		if (1 < items) {
			SELF->minimum = SvNV(ST(1));
		}
		RETVAL = SELF->minimum;
	OUTPUT:
		RETVAL

guint
cheque1(Account* SELF, ...)
	ALIAS:
		check1 = 1
	CODE:
		PERL_UNUSED_VAR(ix);
		if (1 < items) {
			SELF->cheque1 = SvUV(ST(1));
		}
		RETVAL = SELF->cheque1;
	OUTPUT:
		RETVAL

guint
cheque2(Account* SELF, ...)
	ALIAS:
		check2 = 1
	CODE:
		PERL_UNUSED_VAR(ix);
		if (1 < items) {
			SELF->cheque2 = SvUV(ST(1));
		}
		RETVAL = SELF->cheque2;
	OUTPUT:
		RETVAL

gdouble
balance(Account* SELF)
	ALIAS:
		bank_balance    = 1
		future_balance  = 2
	CODE:
		switch (ix) {
			case 1:
				RETVAL = SELF->bal_bank;
				break;
			case 2:
				RETVAL = SELF->bal_future;
				break;
			default:
				RETVAL = SELF->bal_today;
				break;
		}
	OUTPUT:
		RETVAL

gboolean
is_inserted(Account* SELF)
	CODE:
		RETVAL = da_acc_get(SELF->key) == SELF;
	OUTPUT:
		RETVAL

gboolean
is_used(Account* SELF)
	CODE:
		RETVAL = account_is_used(SELF->key);
	OUTPUT:
		RETVAL

gboolean
insert(Account* SELF)
	CODE:
		if (SELF->key == 0 || account_is_used(SELF->key))
			RETVAL = da_acc_append(rc_ref(SELF));
		else
			RETVAL = da_acc_insert(rc_ref(SELF));
	OUTPUT:
		RETVAL

void
remove(Account* SELF)
	CODE:
		da_acc_remove(SELF->key);

void
transactions(Account* SELF)
	PPCODE:
		GList* list = g_queue_peek_head_link(SELF->txn_queue);
		for (; list; list = g_list_next(list)) {
			Transaction* txn = list->data;
			GValue val = G_VALUE_INIT;
			SV* sv = val_to_sv(EXT_TRANSACTION(&val, txn));
			mXPUSHs(sv);
		}

GObject*
open(Account* SELF)
	CODE:
		RETVAL = G_OBJECT(register_panel_window_new(SELF));
	OUTPUT:
		RETVAL


MODULE = HomeBank  PACKAGE = HomeBank::Transaction

Transaction*
new(void)
	CODE:
		RETVAL = da_transaction_malloc();
	OUTPUT:
		RETVAL

void
DESTROY(Transaction* SELF)
	CODE:
		da_transaction_free(SELF);

gdouble
amount(Transaction* SELF, ...)
	CODE:
		if (1 < items) {
			SELF->amount = SvNV(ST(1));
		}
		RETVAL = SELF->amount;
	OUTPUT:
		RETVAL

guint
account_num(Transaction* SELF, ...)
	CODE:
		if (1 < items) {
			SELF->kacc = SvIV(ST(1));
		}
		RETVAL = SELF->kacc;
	OUTPUT:
		RETVAL

guint
paired_account_num(Transaction* SELF, ...)
	CODE:
		if (1 < items) {
			SELF->kxferacc = SvIV(ST(1));
		}
		RETVAL = SELF->kxferacc;
	OUTPUT:
		RETVAL

void
date(Transaction* SELF, ...)
	PPCODE:
		if (1 < items) {
			SELF->date = SvIV(ST(1));
		}
		if (GIMME_V == G_ARRAY) {
			GDate* d = g_date_new_julian(SELF->date);
			mXPUSHp("day", 3);
			mXPUSHi(g_date_get_day(d));
			mXPUSHp("month", 5);
			mXPUSHi(g_date_get_month(d));
			mXPUSHp("year", 4);
			mXPUSHi(g_date_get_year(d));
			g_date_free(d);
			XSRETURN(6);
		} else {
			XSRETURN_IV(SELF->date);
		}

const gchar*
memo(Transaction* SELF, ...)
	CODE:
		if (1 < items) {
			if (SELF->memo) g_free(SELF->memo);
			SELF->memo = g_strdup(SvGchar_ptr(ST(1)));
		}
		RETVAL = SELF->memo ? SELF->memo : "";
	OUTPUT:
		RETVAL

const gchar*
info(Transaction* SELF, ...)
	CODE:
		if (1 < items) {
			if (SELF->info) g_free(SELF->info);
			SELF->info = g_strdup(SvGchar_ptr(ST(1)));
		}
		RETVAL = SELF->info ? SELF->info : "";
	OUTPUT:
		RETVAL

GObject*
open(Transaction* SELF)
	CODE:
		RETVAL = G_OBJECT(create_deftransaction_window(NULL, TRANSACTION_EDIT_MODIFY, FALSE, 0));
		deftransaction_set_transaction(GTK_WIDGET(RETVAL), SELF);
	OUTPUT:
		RETVAL

Transaction*
pair_with(Transaction* SELF, Transaction* other, ...)
	PREINIT:
		int i;
		GList* list = NULL;
	CODE:
		if (2 < items) {
			list = g_list_append(list, other);
			for (i = 2; i < items; ++i) {
				Transaction* ptr = NULL;
				SV* sv = ST(i);
				EXT_P2C_OBJECT("HomeBank::Transaction", sv, ptr, Transaction*);
				list = g_list_append(list, ptr);
			}
			ui_dialog_transaction_xfer_select_child(NULL, SELF, list, &other);
		}
		if (other) {
			transaction_xfer_change_to_child(SELF, other);
			SELF->paymode = PAYMODE_INTXFER;
		}
		RETVAL = other;
	OUTPUT:
		RETVAL
	CLEANUP:
		g_list_free(list);

void
dump(Transaction* SELF)
	CODE:
		g_print("txn: %p (%s) at %u (%d/%d) flags:%d, paymode:%d, kpay:%d, kcat:%d", SELF,
			SELF->memo, SELF->date, SELF->kacc, SELF->kxferacc, SELF->flags, SELF->paymode, SELF->kpay, SELF->kcat);

