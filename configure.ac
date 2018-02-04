#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)
AC_INIT([homebank], [5.1.7])
#AC_INIT([homebank], [x.x-rc])

AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.9 foreign])

# If the source code has changed at all, increment REVISION
# If any interfaces have been added, removed, or changed, increment CURRENT, and set REVISION to 0.
# If any interfaces have been added since the last public release, then increment AGE.
# If any interfaces have been removed since the last public release, then set AGE to 0.
# Reference: http://www.gnu.org/software/libtool/manual.html#Versioning

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_INTLTOOL

# Checks for libraries.
PKG_CHECK_MODULES(DEPS, gtk+-3.0 >= 3.16 glib-2.0 >= 2.39)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)
AC_CHECK_LIB(m, pow)

PKG_CHECK_MODULES(LIBSOUP, [libsoup-2.4 >= 2.26])
AC_SUBST(LIBSOUP_CFLAGS)
AC_SUBST(LIBSOUP_LIBS)

# general usage flags
CFLAGS="${CFLAGS} -Wall -Wmissing-prototypes"

# disable deprecated warnings
CFLAGS="${CFLAGS} -Wno-deprecated-declarations"

# extended flags
#CFLAGS="${CFLAGS} -Wextra -Wno-unused-parameter"

# profiling valgrind/gprof flags
#CFLAGS="${CFLAGS} -g -O0 -pg"

# pre-relase # 2 - error as warning for 
#CFLAGS="${CFLAGS} -Werror"

# gtk3 migration
#CFLAGS="${CFLAGS} -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE"

# GLib/Gtk deprecated flags
#CFLAGS="${CFLAGS} -DG_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"


# Check for libofx
AC_CHECK_HEADER(libofx/libofx.h,,noofx=true)
AC_ARG_WITH(ofx,
    [  --without-ofx           build without ofx support [default=with]],
    [build_ofx=$withval],
    [build_ofx=yes]
)
if test x$build_ofx != xno
then
    if test x$noofx != xtrue
    then
        AC_CHECK_LIB(ofx, ofx_set_status_cb, OFX_0_7="-DOFX_ENABLE")
        DEPS_LIBS="-lofx ${DEPS_LIBS}"
        CFLAGS="${CFLAGS} $OFX_0_7"
    else
        noofx=true
        AC_MSG_RESULT([Libofx header missing. Check your libofx installation])
		CFLAGS="${CFLAGS} -DNOOFX"
    fi
else
    noofx=true
    CFLAGS="${CFLAGS} -DNOOFX"
fi
AM_CONDITIONAL(NOOFX, test x$noofx = xtrue)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_CHECK_FUNCS([floor localeconv memset modf pow setlocale sqrt strcasecmp strtoul])

# International support
ALL_LINGUAS="af ar ast be bg br ca cs cy da de el en_AU en_CA en_GB es et eu fa fi fr gl he hr hu id is it ja ko lt lv mr ms nb nds nl oc pl pt_BR pt pt_PT ro ru si sk sl sr sv tr uk vi zh_CN zh_TW" 

AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

AC_CONFIG_FILES([
Makefile
src/Makefile 
data/homebank.desktop.in
data/Makefile
data/datas/Makefile
images/Makefile
mime/Makefile
pixmaps/Makefile
themes/Makefile
themes/hicolor/Makefile
po/Makefile.in
doc/Makefile
doc/images/Makefile
])

AC_OUTPUT

# *************************************
# *************************************

echo
echo $PACKAGE $VERSION
echo 
echo Compiler................ : $CC
echo Build with OFX support.. : $build_ofx
if test "x$noofx" = "xtrue" ; then
echo ........................ : **error** libofx header is missing, ofx feature will be disabled. Check your libofx installation
fi
eval eval echo Path to images.......... : $datadir/homebank/images
eval eval echo Path to locale.......... : $datadir/locale
eval eval echo Path to help............ : $datadir/homebank/help

echo
eval eval echo HomeBank will be installed in $bindir.
echo
echo configure complete, now type \'make\'
echo

