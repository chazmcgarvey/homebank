/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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

#ifndef __HOMEBANK_IMPORT_H__
#define __HOMEBANK_IMPORT_H__


enum
{
	PAGE_WELCOME,
	PAGE_SELECTFILE,
	PAGE_IMPORT,
	PAGE_PROPERTIES,
	PAGE_ACCOUNT,
	PAGE_TRANSACTION,
	PAGE_CONFIRM,
	NUM_PAGE
};


struct import_data
{
	GtkWidget	*assistant;
	GtkWidget	*pages[NUM_PAGE];

	GtkWidget	*GR_page;
	
	GdkPixbuf	*head_pixbuf;
	GdkPixbuf	*side_pixbuf;
	
	GtkWidget	*filechooser;
	GtkWidget	*user_info;
	GtkWidget	*ok_image;
	GtkWidget	*ko_image;

	GtkWidget	*TX_filepath;
	GtkWidget	*TX_filename;
	GtkWidget	*TX_encoding;
	GtkWidget	*TX_datefmt;

	GtkWidget	*TX_filedetails;

	GtkWidget   *GR_duplicate;

//	GtkWidget	*LA_acc;
	GtkWidget	*NB_maxgap;

	GtkWidget	*BT_refresh;
	GtkWidget	*CY_dateorder;

	GtkWidget   *IM_acc;
	GtkWidget   *LB_acc;
	GtkWidget	*LV_acc;
	GtkWidget	*BT_edit;
	
	GtkWidget   *IM_txn;
	GtkWidget   *LB_txn;
	GtkWidget	*imported_ope;
	GtkWidget	*duplicat_ope;
	
	GtkWidget	*TX_acc_upd;
	GtkWidget	*TX_acc_new;
	GtkWidget	*TX_trn_imp;
	GtkWidget	*TX_trn_nop;
	GtkWidget	*TX_trn_asg;

	gchar		*filepath;
	gchar		*filename;
	guint		filetype;

	/* count imported items */
	guint		imp_cnt_acc;
	guint		imp_cnt_trn;
	guint		imp_cnt_asg;

	gboolean	valid;

//	guint		step;
//	guint		maxstep;


	// import context
	ImportContext	ictx;
	
};

struct import_target_data
{
	GtkWidget   *label1, *label2;
	GtkWidget	*getwidget1;
	GtkWidget	*getwidget2;
	GtkWidget	*radio[2];
};


GtkWidget *ui_import_assistant_new (gint filetype);
Account *import_create_account(gchar *name, gchar *number);
const gchar *homebank_file_getencoding(gchar *filename);
gchar *homebank_utf8_ensure(gchar *buffer);


#endif

