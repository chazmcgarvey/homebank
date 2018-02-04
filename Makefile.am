# HomeBank Makefile.am

#SUBDIRS = src
SUBDIRS = src data images mime pixmaps themes po doc


datasdir   = $(datadir)/homebank/datas/
datas_DATA = \
ChangeLog 

EXTRA_DIST   = $(datas_DATA)


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


# we clean every folder under /usr/share/homebank as well on uninstall
uninstall-hook:
	-rm -rf $(datadir)/homebank/datas
	-rm -rf $(datadir)/homebank/help
	-rm -rf $(datadir)/homebank/icons
	-rm -rf $(datadir)/homebank/images
#-rmdir $(datadir)/homebank
