
#include <stdarg.h>

#include "ext-value.h"


const GValue* ext_value_undef()
{
	static GValue v = G_VALUE_INIT;
	return &v;
}

const GValue* ext_value_true()
{
	static GValue v = G_VALUE_INIT;
	if (!G_VALUE_HOLDS_BOOLEAN(&v)) EXT_BOOLEAN(&v, TRUE);
	return &v;
}

const GValue* ext_value_false()
{
	static GValue v = G_VALUE_INIT;
	if (!G_VALUE_HOLDS_BOOLEAN(&v)) EXT_BOOLEAN(&v, FALSE);
	return &v;
}


GValue* EXT_LIST(GValue* v, ...)
{
	GPtrArray* a = g_ptr_array_new();

	va_list ap;
	va_start(ap, v);

	for (;;) {
		GValue* item = (GValue*)va_arg(ap, GValue*);
		if (!item) break;
		g_ptr_array_add(a, item);
	}

	va_end(ap);

	return EXT_ARRAY(v, a);
}

GValue* EXT_HASH(GValue* v, ...)
{
	GHashTable* h = g_hash_table_new(g_str_hash, g_str_equal);

	va_list ap;
	va_start(ap, v);

	for (;;) {
		gchar* key = (gchar*)va_arg(ap, gchar*);
		if (!key) break;
		GValue* val = (GValue*)va_arg(ap, GValue*);
		g_hash_table_insert(h, key, val);
	}

	va_end(ap);

	return EXT_HASH_TABLE(v, h);
}

GValue* EXT_JULIAN(GValue* v, guint32 d)
{
	GDate* date = g_date_new_julian(d);
	return EXT_DATE(v, date);
}


#define obj(CTYPE, _2, _3, _4, PREFIX)                  \
GType PREFIX##get_type()                                \
{                                                       \
	static GType type = 0;                              \
	if (type == 0)                                      \
		type = g_pointer_type_register_static(#CTYPE);  \
	return type;                                        \
}
#include "ext-value.h"
#undef obj

