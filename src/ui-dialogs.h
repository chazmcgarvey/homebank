/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2018 Maxime DOYEN
 *
 *  This file is part of HomeBank.
 *
 *  HomeBank is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  HomeBank is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HB_DIALOGS_GTK_H__
#define __HB_DIALOGS_GTK_H__



gint ui_dialog_msg_confirm_alert(GtkWindow *parent, gchar *title, gchar *secondtext, gchar *actionverb);

gint ui_dialog_about(GtkWindow *parent, gchar *title, gchar *message_format, ...);
gint ui_dialog_msg_question(GtkWindow *parent, gchar *title, gchar *message_format, ...);
void ui_dialog_msg_infoerror(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...);
gboolean ui_file_chooser_qif(GtkWindow *parent, gchar **storage_ptr);
gboolean ui_file_chooser_csv(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr, gchar *name);
gboolean ui_file_chooser_xhb(GtkFileChooserAction action, gchar **storage_ptr);
gboolean ui_file_chooser_folder(GtkWindow *parent, gchar *title, gchar **storage_ptr);

gint ui_dialog_export_pdf(GtkWindow *parent, gchar **storage_ptr);

void ui_dialog_upgrade_choose_currency(void);

gboolean ui_dialog_msg_savechanges(GtkWidget *widget, gpointer user_data);

void ui_dialog_file_statistics(void);

Transaction *ui_dialog_transaction_xfer_select_child(Transaction *stxn, GList *matchlist);

#endif

