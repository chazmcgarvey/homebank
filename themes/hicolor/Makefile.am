## Process this file with automake to produce Makefile.in

NULL =

public_icons_themes = \
	hicolor	\
	$(NULL)

public_icons = \
	$(NULL)

private_icons = \
	hicolor_actions_16x16_btn-collapse.png \
	hicolor_actions_16x16_btn-expand.png \
	hicolor_status_16x16_btn-split.png \
	hicolor_status_16x16_flt-exclude.png \
	hicolor_status_16x16_flt-inactive.png \
	hicolor_status_16x16_flt-include.png \
	hicolor_actions_16x16_hb-ope-auto.png \
	hicolor_actions_16x16_hb-ope-budget.png \
	hicolor_actions_16x16_hb-ope-edit.png \
	hicolor_actions_16x16_hb-ope-cleared.png \
	hicolor_actions_16x16_hb-ope-remind.png \
	hicolor_actions_16x16_hb-ope-reconciled.png \
	hicolor_actions_24x24_hb-account.png \
	hicolor_actions_24x24_hb-archive.png \
	hicolor_actions_24x24_hb-assign.png \
	hicolor_actions_24x24_hb-assign-run.png \
	hicolor_actions_24x24_hb-budget.png \
	hicolor_actions_24x24_hb-category.png \
	hicolor_actions_24x24_hb-currency.png \
	hicolor_actions_24x24_hb-file-export.png \
	hicolor_actions_24x24_hb-file-import.png \
	hicolor_actions_24x24_hb-file-valid.png \
	hicolor_actions_24x24_hb-file-invalid.png \
	hicolor_actions_24x24_hb-filter.png \
	hicolor_actions_24x24_hb-legend.png \
	hicolor_actions_24x24_hb-ope-add.png \
	hicolor_actions_24x24_hb-ope-cleared.png \
	hicolor_actions_24x24_hb-ope-delete.png \
	hicolor_actions_24x24_hb-ope-edit.png \
	hicolor_actions_24x24_hb-ope-herit.png \
	hicolor_actions_24x24_hb-ope-multiedit.png \
	hicolor_actions_24x24_hb-ope-reconciled.png \
	hicolor_actions_24x24_hb-ope-convert.png \
	hicolor_actions_24x24_hb-ope-show.png \
	hicolor_actions_24x24_hb-payee.png \
	hicolor_actions_24x24_hb-rate.png \
	hicolor_actions_24x24_hb-rep-balance.png \
	hicolor_actions_24x24_hb-rep-budget.png \
	hicolor_actions_24x24_hb-rep-stats.png \
	hicolor_actions_24x24_hb-rep-time.png \
	hicolor_actions_24x24_hb-rep-vehicle.png \
	hicolor_actions_24x24_hb-view-bar.png \
	hicolor_actions_24x24_hb-view-column.png \
	hicolor_actions_24x24_hb-view-donut.png \
	hicolor_actions_24x24_hb-view-line.png \
	hicolor_actions_24x24_hb-view-list.png \
	hicolor_actions_24x24_hb-view-pie.png \
	hicolor_actions_24x24_hb-view-stack.png \
	hicolor_status_16x16_pm-none.png \
	hicolor_status_16x16_pm-ccard.png \
	hicolor_status_16x16_pm-dcard.png \
	hicolor_status_16x16_pm-check.png \
	hicolor_status_16x16_pm-cash.png \
	hicolor_status_16x16_pm-transfer.png \
	hicolor_status_16x16_pm-intransfer.png \
	hicolor_status_16x16_pm-none.png \
	hicolor_status_16x16_pm-standingorder.png \
	hicolor_status_16x16_pm-epayment.png \
	hicolor_status_16x16_pm-deposit.png \
	hicolor_status_16x16_pm-fifee.png \
	hicolor_status_16x16_pm-directdebit.png \
	hicolor_status_48x48_prf-columns.png \
	hicolor_status_48x48_prf-display.png \
	hicolor_status_48x48_prf-euro.png \
	hicolor_status_48x48_prf-general.png \
	hicolor_status_48x48_prf-import.png \
	hicolor_status_48x48_prf-interface.png \
	hicolor_status_48x48_prf-report.png \
	hicolor_actions_scalable_toggle-sign-symbolic.svg \
	$(NULL)


EXTRA_DIST = \
	$(public_icons)		\
	$(private_icons)	\
	$(noinst_DATA)		\
	$(NULL)

###############################################################################

gtk_update_icon_cache = gtk-update-icon-cache -f -t

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		for theme in $(public_icons_themes); do \
			$(gtk_update_icon_cache) $(datadir)/icons/$$theme; \
		done; \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		for theme in $(public_icons_themes); do \
			echo "***   $(gtk_update_icon_cache) $(datadir)/icons/$$theme"; \
		done; \
	fi

install-icons:
	for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done; \
	for icon in $(private_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(pkgdatadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(pkgdatadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

uninstall-icons:
	-for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		rm -f $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done; \
	for icon in $(private_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		rm -f $(DESTDIR)$(pkgdatadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

install-data-local: install-icons update-icon-cache

uninstall-local: uninstall-icons update-icon-cache
