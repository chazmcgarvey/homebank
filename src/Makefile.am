
common_defines = \
	-DSHARE_DIR=\""$(pkgdatadir)"\" \
	-DDATA_DIR=\""$(datadir)"\"


bin_PROGRAMS = homebank


HOMEBANK_CORE = 
	

USER_INTERFACE = 
	

homebank_SOURCES =  \
	dsp_account.c \
	dsp_account.h \
	dsp_mainwindow.c \
	dsp_mainwindow.h \
	enums.h \
	gtk-chart.c \
	gtk-chart.h \
	gtk-chart-colors.c \
	gtk-chart-colors.h \
	gtk-chart-stack.c \
	gtk-chart-stack.h \
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
	language.c \
	language.h \
	list_account.c \
	list_account.h \
	list_operation.c \
	list_operation.h \
	list_topspending.c \
	list_topspending.h \
	list_upcoming.c \
	list_upcoming.h \
	rep_balance.c \
	rep_balance.h \
	rep_budget.c \
	rep_budget.h \
	rep_stats.c \
	rep_stats.h \
	rep_time.c \
	rep_time.h \
	rep_vehicle.c \
	rep_vehicle.h \
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
	ui-transaction.c \
	ui-transaction.h \
	ui-txn-multi.c \
	ui-txn-multi.h \
	ui-widgets.c \
	ui-widgets.h

homebank_LDADD = $(DEPS_LIBS) \
	$(LIBSOUP_LIBS)

AM_CPPFLAGS = \
	$(DEPS_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(common_defines)

