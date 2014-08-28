/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2014 Maxime DOYEN
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

#define QIF_UNKNOW_ACCOUNT_NAME "(unknown)"


enum
{
	PAGE_INTRO,
	PAGE_FILE,
	PAGE_RESULT,
	PAGE_OPTIONS,
	PAGE_CONFIRM,
	NUM_PAGE
};


typedef struct _OfxContext OfxContext;
struct _OfxContext
{
	GList		*trans_list;
	Account 	*curr_acc;
	gboolean	curr_acc_isnew;
};


typedef struct _ImportContext ImportContext;
struct _ImportContext
{
	GList		*trans_list;		// trn storage
	gint 		cnt_initial_acc;	//max key account when start
	gint		cnt_new_acc;		//
	gint		cnt_new_ope;
	gint		cnt_new_pay;
	gint		cnt_new_cat;
	gint		datefmt;
	const gchar		*encoding;
	gint		cnt_err_date;
	gint		nb_duplicate;
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
	GtkWidget	*TX_filedetails;

	GtkWidget	*GR_error;
	GtkWidget   *GR_options;
	GtkWidget   *GR_duplicate;

//	GtkWidget	*LA_acc;
	GtkWidget	*NB_maxgap;

	GtkWidget	*BT_refresh;
	GtkWidget	*CY_dateorder;

	GtkWidget	*LV_acc;
	GtkWidget	*BT_edit;
	
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
	GtkWidget	*getwidget1;
	GtkWidget	*getwidget2;
	GtkWidget	*radio[2];
};


GtkWidget *ui_import_window_new (void);
Account *import_create_account(gchar *name, gchar *number);
const gchar *homebank_file_getencoding(gchar *filename);
gchar *homebank_utf8_ensure(gchar *buffer);

#endif


