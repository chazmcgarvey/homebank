
plugindir		= $(pkglibdir)/plugins

native_la_LDFLAGS	= -module -avoid-version -shared -export-dynamic

native_la_SOURCES	= native.c

native_la_CPPFLAGS	= $(DEPS_CFLAGS) -I$(top_srcdir)/src
native_la_LIBADD	= $(DEPS_LIBS)

plugin_LTLIBRARIES	= native.la

