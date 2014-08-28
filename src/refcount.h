
#ifndef __REFCOUNT_H__
#define __REFCOUNT_H__

#include <glib.h>


static inline gpointer rc_alloc(size_t size)
{
	gpointer chunk = g_malloc0(size + sizeof(long));
	(*(long*)chunk) = 1;
	//g_print("ALLOC: %p (ref %ld)\n", (long*)chunk + 1, *(long*)chunk);
	return (long*)chunk + 1;
}

static inline gpointer rc_ref(gpointer p)
{
	//g_print("  REF: %p (ref %ld)\n", p, *((long*)p - 1));
	if (p) {
		++(*((long*)p - 1));
	}
	return p;
}

static inline gboolean rc_unref(gpointer p)
{
	//g_print("UNREF: %p (ref %ld)\n", p, *((long*)p - 1));
	if (p && --(*((long*)p - 1)) <= 0) {
		return TRUE;
	}
	return FALSE;
}

static inline void rc_free(gpointer p)
{
	//g_print(" FREE: %p (ref %ld)\n", p, *((long*)p - 1));
	g_free((long*)p - 1);
}

static inline gpointer rc_dup(gpointer p, size_t size)
{
	if (p) {
		gpointer chunk = (long*)p - 1;
		gpointer new_chunk = g_memdup(chunk, size + sizeof(long));
		*(long*)new_chunk = 1;
		//g_print("  DUP: %p (ref %ld) -> %p (ref %ld)\n", p, *((long*)p - 1), (long*)new_chunk + 1, *(long*)new_chunk);
		return (long*)new_chunk + 1;
	}
	//g_print("  DUP: NULL\n");
	return NULL;
}


#endif
