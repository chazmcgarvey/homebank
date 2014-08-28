# HomeBank Makefile.am

#SUBDIRS = src
SUBDIRS = src data images mime po doc


# don't forget to do a 'make check' 
# to ensure all files are in po/POTFILES.in

#pot: Makefile
#${MAKE} -C po $(PACKAGE).pot

INTLTOOL_FILES = intltool-extract.in \
                 intltool-merge.in \
                 intltool-update.in
                 
DISTCLEANFILES = ... intltool-extract \
                 intltool-merge \
                 intltool-update \
                 po/.intltool-merge-cache
