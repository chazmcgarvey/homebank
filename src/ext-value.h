
#ifndef __EXT_VALUE_H__
#define __EXT_VALUE_H__

#include "homebank.h"


#define DA_TYPE_ACC         (da_acc_get_type())
#define DA_TYPE_TRANSACTION (da_transaction_get_type())

#define obj(_1, _2, _3, _4, PREFIX) GType PREFIX##get_type(void);
#include "ext-value.h"
#undef obj


#define val(CTYPE, NAME, PART, GTYPE)                   \
static inline GValue* EXT_##NAME(GValue* v, CTYPE c) {  \
    g_value_init(v, GTYPE);                             \
    g_value_set_##PART(v, c);                           \
    return v;                                           \
}
#define obj(CTYPE, NAME, PART, GTYPE, _5) val(CTYPE*, NAME, PART, GTYPE)
#include "ext-value.h"
#undef val
#undef obj


const GValue* ext_value_undef(void);
const GValue* ext_value_true(void);
const GValue* ext_value_false(void);

#define EXT_UNDEF (ext_value_undef())
#define EXT_TRUE  (ext_value_true())
#define EXT_FALSE (ext_value_false())

GValue* EXT_LIST(GValue* v, ...);
GValue* EXT_HASH(GValue* v, ...);
GValue* EXT_JULIAN(GValue* v, guint32 d);


#else

#ifdef val
// C type, name, fundamental, GType
val(gboolean,       BOOLEAN,        boolean,    G_TYPE_BOOLEAN)
val(gint,           INT,            int,        G_TYPE_INT)
val(guint,          UINT,           uint,       G_TYPE_UINT)
val(gdouble,        DOUBLE,         double,     G_TYPE_DOUBLE)
val(gchar,          CHAR,           schar,      G_TYPE_CHAR)
val(gchar*,         STRING,         string,     G_TYPE_STRING)
val(GPtrArray*,     ARRAY,          boxed,      G_TYPE_PTR_ARRAY)
val(GHashTable*,    HASH_TABLE,     boxed,      G_TYPE_HASH_TABLE)
val(GDate*,         DATE,           boxed,      G_TYPE_DATE)
val(void*,          OBJECT,         object,     G_TYPE_OBJECT)
#endif

#ifdef obj
// C type, name, fundamental, GType, prefix
obj(Account,        ACCOUNT,        pointer,    DA_TYPE_ACC,            da_acc_)
obj(Transaction,    TRANSACTION,    pointer,    DA_TYPE_TRANSACTION,    da_transaction_)
#endif

#endif

