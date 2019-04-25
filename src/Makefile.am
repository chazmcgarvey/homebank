
common_defines = \
	-DSHARE_DIR=\""$(pkgdatadir)"\" \
	-DDATA_DIR=\""$(datadir)"\"


bin_PROGRAMS = homebank


HOMEBANK_CORE = 
	

USER_INTERFACE = 
	

homebank_SOURCES =  \
	dsp-account.c \
	dsp-account.h \
	dsp-mainwindow.c \
	dsp-mainwindow.h \
	enums.h \
	gtk-chart.c \
	gtk-chart.h \
	gtk-chart-colors.c \
	gtk-chart-colors.h \
	gtk-chart-progress.c \
	gtk-chart-progress.h \
	gtk-dateentry.c \
	gtk-dateentry.h \
	hb-account.c \
	hb-account.h \
	hb-archive.c \
	hb-archive.h \
	hb-assign.c \
	hb-assign.h \
	hb-category.c \
	hb-category.h \
	hb-currency.c \
	hb-currency.h \
	hb-encoding.c \
	hb-encoding.h \
	hb-export.c \
	hb-export.h \
	hb-filter.c \
	hb-filter.h \
	hb-hbfile.c \
	hb-hbfile.h \
	hb-import.c \
	hb-import.h \
	hb-import-ofx.c \
	hb-import-qif.c \
	hb-import-csv.c \
	hb-misc.c \
	hb-misc.h \
	hb-payee.c \
	hb-payee.h \
	hb-preferences.c \
	hb-preferences.h \
	hb-report.c \
	hb-report.h \
	hb-tag.c \
	hb-tag.h \
	hb-split.c \
	hb-split.h \
	hb-transaction.c \
	hb-transaction.h \
	hb-xml.c \
	hb-xml.h \
	homebank.c \
	homebank.h \
	hub-account.c \
	hub-account.h \
	hub-scheduled.c \
	hub-scheduled.h \
	hub-spending.c \
	hub-spending.h \
	hub-transaction.c \
	hub-transaction.h \
	language.c \
	language.h \
	list-account.c \
	list-account.h \
	list-operation.c \
	list-operation.h \
	list-scheduled.c \
	list-scheduled.h \
	rep-balance.c \
	rep-balance.h \
	rep-budget.c \
	rep-budget.h \
	rep-stats.c \
	rep-stats.h \
	rep-time.c \
	rep-time.h \
	rep-vehicle.c \
	rep-vehicle.h \
	ui-account.c \
	ui-account.h \
	ui-archive.c \
	ui-archive.h \
	ui-assign.c \
	ui-assign.h \
	ui-assist-import.c \
	ui-assist-import.h \
	ui-assist-start.c \
	ui-assist-start.h \
	ui-budget.c \
	ui-budget.h \
	ui-category.c \
	ui-category.h \
	ui-currency.c \
	ui-currency.h \
	ui-dialogs.c \
	ui-dialogs.h \
	ui-filter.c \
	ui-filter.h \
	ui-hbfile.c \
	ui-hbfile.h \
	ui-payee.c \
	ui-payee.h \
	ui-pref.c \
	ui-pref.h \
	ui-split.c \
	ui-split.h \
	ui-tag.c \
	ui-tag.h \
	ui-transaction.c \
	ui-transaction.h \
	ui-txn-multi.c \
	ui-txn-multi.h \
	ui-widgets-data.c \
	ui-widgets.c \
	ui-widgets.h


homebank_LDADD = $(DEPS_LIBS) \
	$(LIBSOUP_LIBS)

AM_CPPFLAGS = \
	$(DEPS_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(common_defines)

