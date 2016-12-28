## Process this file with automake to produce Makefile.in

SUBDIRS = datas


@INTLTOOL_DESKTOP_RULE@
desktopdir 		 = $(datadir)/applications
desktop_in_files = homebank.desktop.in
desktop_DATA	 = $(desktop_in_files:.desktop.in=.desktop)


@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_in_files = homebank.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)


EXTRA_DIST =	\
	$(desktop_in_files) \
	$(appdata_in_files)

CLEANFILES =	\
	$(desktop_DATA) \
	$(appdata_DATA)

