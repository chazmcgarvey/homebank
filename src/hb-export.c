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

#include "homebank.h"
#include "hb-export.h"

/****************************************************************************/
/* Debug macros                                                             */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;

/* = = = = = = = = = = = = = = = = = = = = */

static void hb_export_qif_elt_txn(GIOChannel *io, Account *acc)
{
GString *elt;
GList *list;
GDate *date;
char amountbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *sbuf;
gint count, i;

	elt = g_string_sized_new(255);

	date = g_date_new ();
	
	list = g_queue_peek_head_link(acc->txn_queue);
	while (list != NULL)
	{
	Transaction *txn = list->data;
	Payee *payee;
	Category *cat;
	gchar *txt;

		g_date_set_julian (date, txn->date);
		//#1270876
		switch(PREFS->dtex_datefmt)
		{
			case 0: //"m-d-y"  
				g_string_append_printf (elt, "D%02d/%02d/%04d\n", 
					g_date_get_month(date),
					g_date_get_day(date),
					g_date_get_year(date)
					);
				break;
			case 1: //"d-m-y"
				g_string_append_printf (elt, "D%02d/%02d/%04d\n", 
					g_date_get_day(date),
					g_date_get_month(date),
					g_date_get_year(date)
					);
				break;
			case 2: //"y-m-d"
				g_string_append_printf (elt, "D%04d/%02d/%02d\n", 
					g_date_get_year(date),
					g_date_get_month(date),
					g_date_get_day(date)
					);
				break;
		}			

		//g_ascii_dtostr (amountbuf, sizeof (amountbuf), txn->amount);
		g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", txn->amount);
		g_string_append_printf (elt, "T%s\n", amountbuf);

		sbuf = "";
		if(txn->status == TXN_STATUS_CLEARED)
			sbuf = "c";
		else
		if(txn->status == TXN_STATUS_RECONCILED)
			sbuf = "R";

		g_string_append_printf (elt, "C%s\n", sbuf);

		if( txn->paymode == PAYMODE_CHECK)
			g_string_append_printf (elt, "N%s\n", txn->info);

		//Ppayee
		payee = da_pay_get(txn->kpay);
		if(payee)
			g_string_append_printf (elt, "P%s\n", payee->name);

		// Mmemo
		g_string_append_printf (elt, "M%s\n", txn->memo);

		// LCategory of transaction
		// L[Transfer account name]
		// LCategory of transaction/Class of transaction
		// L[Transfer account]/Class of transaction
		if( txn->paymode == PAYMODE_INTXFER && txn->kacc == acc->key)
		{
		//#579260
			Account *dstacc = da_acc_get(txn->kxferacc);
			if(dstacc)
				g_string_append_printf (elt, "L[%s]\n", dstacc->name);
		}
		else
		{
			cat = da_cat_get(txn->kcat);
			if(cat)
			{
				txt = da_cat_get_fullname(cat);
				g_string_append_printf (elt, "L%s\n", txt);
				g_free(txt);
			}
		}

		// splits
		count = da_splits_count(txn->splits);
		for(i=0;i<count;i++)
		{
		Split *s = txn->splits[i];
				
			cat = da_cat_get(s->kcat);
			if(cat)
			{
				txt = da_cat_get_fullname(cat);
				g_string_append_printf (elt, "S%s\n", txt);
				g_free(txt);
			}	
				
			g_string_append_printf (elt, "E%s\n", s->memo);
			
			g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", s->amount);
			g_string_append_printf (elt, "$%s\n", amountbuf);
		}
		
		g_string_append (elt, "^\n");


		list = g_list_next(list);
	}

	g_io_channel_write_chars(io, elt->str, -1, NULL, NULL);
	
	g_string_free(elt, TRUE);

	g_date_free(date);
	
}



static void hb_export_qif_elt_acc(GIOChannel *io, Account *acc)
{
GString *elt;
gchar *type;
	
	elt = g_string_sized_new(255);
	
	// account export
	//#987144 fixed account type
	switch(acc->type)
	{
		case ACC_TYPE_BANK : type = "Bank"; break;
		case ACC_TYPE_CASH : type = "Cash"; break;
		case ACC_TYPE_ASSET : type = "Oth A"; break;
		case ACC_TYPE_CREDITCARD : type = "CCard"; break;
		case ACC_TYPE_LIABILITY : type = "Oth L"; break;
		default : type = "Bank"; break;
	}

	g_string_assign(elt, "!Account\n");
	g_string_append_printf (elt, "N%s\n", acc->name);
	g_string_append_printf (elt, "T%s\n", type);
	g_string_append (elt, "^\n");
	g_string_append_printf (elt, "!Type:%s\n", type);

	g_io_channel_write_chars(io, elt->str, -1, NULL, NULL);
	
	g_string_free(elt, TRUE);
}


void hb_export_qif_account_single(gchar *filename, Account *acc)
{
GIOChannel *io;
	
	io = g_io_channel_new_file(filename, "w", NULL);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
		//retval = XML_IO_ERROR;
	}
	else
	{
		hb_export_qif_elt_acc(io, acc);
		hb_export_qif_elt_txn(io, acc);	
		g_io_channel_unref (io);
	}
}


void hb_export_qif_account_all(gchar *filename)
{
GIOChannel *io;
GList *lacc, *list;

	io = g_io_channel_new_file(filename, "w", NULL);
	if(io == NULL)
	{
		g_message("file error on: %s", filename);
		//retval = XML_IO_ERROR;
	}
	else
	{
		//todo: save accounts in order
		//todo: save transfer transaction once

		lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
		while (list != NULL)
		{
		Account *item = list->data;

			hb_export_qif_elt_acc(io, item);
			hb_export_qif_elt_txn(io, item);	

			list = g_list_next(list);
		}
		g_list_free(lacc);

		g_io_channel_unref (io);
	}

}


/* = = = = = = = = beta feature version = = = = = = = = */

//static GtkPrintSettings *settings = NULL;
//static GtkPageSetup *page_setup = NULL;


static void papersize(PdfPrintContext *ppc)
{
//GList *list, *item;
const gchar *name;
GtkPaperSize *ps;

	DB( g_print("[papersize]\n") );
	
	name = gtk_paper_size_get_default();

	DB( g_print("- def paper is %s\n", name) );
	
	ps = gtk_paper_size_new(name);



  /*GtkPageSetup *new_page_setup;

  if (settings == NULL)
    settings = gtk_print_settings_new ();

  new_page_setup = gtk_print_run_page_setup_dialog (NULL,
                                                    page_setup, settings);

  if (page_setup)
    g_object_unref (page_setup);

  page_setup = new_page_setup;
*/

//#if MYDEBUG == 1
	gdouble w, h, mt, mb, ml, mr;
	w = gtk_paper_size_get_width(ps, GTK_UNIT_MM);
	h = gtk_paper_size_get_height(ps, GTK_UNIT_MM);
	mt = gtk_paper_size_get_default_top_margin(ps, GTK_UNIT_MM);
	mr = gtk_paper_size_get_default_right_margin(ps, GTK_UNIT_MM);
	mb = gtk_paper_size_get_default_bottom_margin(ps, GTK_UNIT_MM);
	ml = gtk_paper_size_get_default_left_margin(ps, GTK_UNIT_MM);
	
	DB( g_print("- name: %s\n", gtk_paper_size_get_display_name(ps)) );
	DB( g_print("- w: %f (%f)\n- h: %f (%f)\n", w, w/PANGO_SCALE, h, h/PANGO_SCALE) );
	DB( g_print("- margin: %f %f %f %f\n", mt, mr, mb, ml) );

	ppc->w = w * 2.83;
	ppc->h = h * 2.83;
	ppc->mt = mt * 2.83;
	ppc->mr = mr * 2.83;
	ppc->mb = mb * 2.83;
	ppc->ml = ml * 2.83;

//#endif

	gtk_paper_size_free(ps);

	/* list all paper size */
	/*
	list = gtk_paper_size_get_paper_sizes (FALSE);
	item = g_list_first(list);
	while(item != NULL)
	{
		ps = item->data;
		if(ps != NULL)
		{
			g_print("- name: %s\n", gtk_paper_size_get_display_name(ps));
			gtk_paper_size_free(ps);
		}
		item = g_list_next(item);
	}
	g_list_free (list);
	*/
}


#define FONT "Helvetica 9px"

//#define PDF_MARGIN 24
#define PDF_COL_MARGIN 8
#define PDF_LINE_MARGIN 2
#define PDF_FONT_NORMAL 9
#define PDF_FONT_TITLE 12


#define HELPDRAW 0
#define RULEHINT 1

#define HEX_R(xcol) (((xcol>>24) & 0xFF)/255)
#define HEX_G(xcol) (((xcol>>16) & 0xFF)/255)
#define HEX_B(xcol) (((xcol>> 8) & 0xFF)/255)


#if HELPDRAW == 1
static void hb_pdf_draw_help_rect(cairo_t *cr, gint32 xcol, double x, double y, double w, double h)
{
	cairo_save (cr);
	
	cairo_set_line_width(cr, 1.0);
	cairo_set_source_rgba(cr, HEX_R(xcol), HEX_G(xcol), HEX_B(xcol), 1.0);	//alpha is unused for now
	cairo_rectangle (cr, x, y, w, h);
	cairo_stroke(cr);
	
	cairo_restore(cr);
}
#endif




static void hb_pdf_draw_line(PdfPrintContext *ppc, cairo_t *cr, gdouble y, gboolean bold, gboolean rulehint)
{
PangoLayout *layout;
gint i;
gdouble x;

	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (cr);

	//desc = pango_font_description_from_string (FONT);
	if(bold)
		pango_font_description_set_weight(ppc->desc, PANGO_WEIGHT_BOLD);
	else
		pango_font_description_set_weight(ppc->desc, PANGO_WEIGHT_NORMAL);

	pango_layout_set_font_description (layout, ppc->desc);

	
	x = ppc->ml;

	/* rule hint */
	#if RULEHINT == 1
	if( rulehint )
	{
		cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
		cairo_rectangle (cr, x, y, ppc->w - ppc->ml - ppc->mr, PDF_FONT_NORMAL);
		cairo_fill(cr);
	}
	#endif


	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_move_to(cr, x, y);

	for(i=0;i<PDF_NUMCOL;i++)
	{
		if(ppc->column_txt[i] != NULL)
		{
		int width, height;

			pango_layout_set_text (layout, ppc->column_txt[i], -1);
			pango_layout_get_size (layout, &width, &height);
			if( i==1 || i==2 || i==3 )
			{
				pango_layout_set_width(layout, ppc->column_width[i]*PANGO_SCALE);
				pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
			}

			//cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
			
			if( i==0 || i==4 || i==6 ) // pad right: date/amount/balance
			{
				//if(*ppc->column_txt[i] != '-')
				   //cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.66); //grey
				cairo_move_to(cr, x + ppc->column_width[i] - (width/PANGO_SCALE) , y);
			}
			else
				cairo_move_to(cr, x, y);
			
			pango_cairo_show_layout (cr, layout);

			/* test line */
			/*cairo_set_line_width(cr, 1.0);
			cairo_move_to(cr, x, y + .5);
			cairo_line_to(cr, x + ppc->column_width[i], y+.5);
			cairo_stroke(cr);
			*/
			
		}
		x += ppc->column_width[i] + PDF_COL_MARGIN;
	}

	g_object_unref (layout);	

}


static void hb_pdf_set_col_title(PdfPrintContext *ppc)
{
	ppc->column_txt[0] = _("Date");
	ppc->column_txt[1] = _("Info");
	ppc->column_txt[2] = _("Payee");
	ppc->column_txt[3] = _("Memo");
	ppc->column_txt[4] = _("Amount");
	ppc->column_txt[5] = "C";
	ppc->column_txt[6] = _("Balance");
}			


void hb_export_pdf_listview(GtkTreeView *treeview, gchar *filepath, gchar *accname)
{
cairo_surface_t *surf;
cairo_t *cr;
PdfPrintContext ppc;
PangoFontDescription *desc;
PangoLayout *layout;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gint i, col;

	
	DB( g_print("[gtk-chart] export to pdf\n") );

	model = gtk_tree_view_get_model(treeview);
	
	papersize(&ppc);

	//gchar *filename = "/home/max/Desktop/hb-txn-export.pdf";
	double width;	//=210 * 2.83;
	double height;	//=297 * 2.83;
	
	width  = ppc.w;
	height = ppc.h;

	surf = cairo_pdf_surface_create (filepath, width, height);
	
	if( cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS )
	//todo: manage error later on
		return;

	
	cr = cairo_create (surf);
	//cairo_pdf_surface_set_size(surf, width * 2.83, height * 2.83);

	//g_print("width=%d\n", cairo_image_surface_get_width( surf));
	double x1, x2, y1, y2;
	cairo_clip_extents (cr, &x1, &y1, &x2, &y2);

	DB( g_print("surface w=%f, h=%f\n", x2 - x1, y2 - y1) );
	double pwidth = x2 - x1;
	
	
	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (cr);

	/* get and copy the font from the treeview widget */
	gtk_style_context_get(gtk_widget_get_style_context(GTK_WIDGET(treeview)), GTK_STATE_FLAG_NORMAL, "font", &desc, NULL);
	ppc.desc = pango_font_description_copy(desc);

	DB( g_print("family: %s\n", pango_font_description_get_family(desc)) );
	DB( g_print("size: %d (%d)\n", pango_font_description_get_size (desc), pango_font_description_get_size (desc )/PANGO_SCALE) );


	
	/* header is 1 line for date page number at top, then a title in bold, then 2 empty lines */
	gint header_height = PDF_FONT_NORMAL * 2 + PDF_FONT_TITLE;
	gint nb_lines = gtk_tree_model_iter_n_children(model, NULL);

	/* should include here the headertitle line */
	
	gint lpp = floor ((height-header_height-ppc.mt-ppc.mb) / (PDF_FONT_NORMAL + PDF_LINE_MARGIN));
	gint page, num_pages = (nb_lines - 1) / lpp + 1;

	DB( g_print("\n - should pdf %d lines, lpp=%d, num_pages=%d\n", nb_lines, lpp, num_pages) );


	gint tot_lines = 0;
	gint cur_page_line = 1;

	gchar dbuffer[255];
	gchar amtbuf[G_ASCII_DTOSTR_BUF_SIZE];
	gchar balbuf[G_ASCII_DTOSTR_BUF_SIZE];

	GDate *date = g_date_new ();

	//cairo_set_font_size(cr, PDF_FONT_NORMAL);
	pango_font_description_set_absolute_size(ppc.desc, PDF_FONT_NORMAL * PANGO_SCALE);
	pango_layout_set_font_description (layout, ppc.desc);
	
	/* reset struct */
	hb_pdf_set_col_title(&ppc);
	
	for(col=0;col<PDF_NUMCOL;col++)
	{
	int tw, th;

		ppc.column_width[col] = 0;
		pango_layout_set_text (layout, ppc.column_txt[col], -1);
		pango_layout_get_size (layout, &tw, &th);
		ppc.column_width[col] = MAX(ppc.column_width[col], tw / PANGO_SCALE);
	}


	DB( g_print(" - compute width\n") );

	/* first pass to get max width */
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Transaction *txn;
	int tw, th;
			
		gtk_tree_model_get (model, &iter, LST_DSPOPE_DATAS, &txn, -1);

		i = 0;
		g_date_set_julian (date, txn->date);
		g_date_strftime (dbuffer, 255-1, "%x", date);
		pango_layout_set_text (layout, dbuffer, -1);
		pango_layout_get_size (layout, &tw, &th);
		ppc.column_width[i] = MAX(ppc.column_width[i], tw / PANGO_SCALE);
		
		i = 1;
		if(txn->info != NULL && strlen(txn->info) > 0)
		{
			pango_layout_set_text (layout, txn->info, -1);
			pango_layout_get_size (layout, &tw, &th);
			ppc.column_width[i] = MAX(ppc.column_width[i], tw / PANGO_SCALE);
		}
		
		i = 4;
		hb_strfnum(amtbuf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->amount, txn->kcur, GLOBALS->minor);
		pango_layout_set_text (layout, amtbuf, -1);
		pango_layout_get_size (layout, &tw, &th);
		ppc.column_width[i] = MAX(ppc.column_width[i], tw / PANGO_SCALE);

		i = 5;
		pango_layout_set_text (layout, "R", -1);
		pango_layout_get_size (layout, &tw, &th);
		ppc.column_width[i] = MAX(ppc.column_width[i], tw / PANGO_SCALE);

		i = 6;
		hb_strfnum(balbuf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->balance, txn->kcur, GLOBALS->minor);
		pango_layout_set_text (layout, balbuf, -1);
		pango_layout_get_size (layout, &tw, &th);
		ppc.column_width[i] = MAX(ppc.column_width[i], tw / PANGO_SCALE);

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	/* distribute remaining size */
	gdouble tmp = pwidth - ppc.ml - ppc.mr - (PDF_COL_MARGIN*PDF_NUMCOL);


	DB( g_print(" page width=%f, remain width=%f\n", pwidth, tmp) );
	
	tmp -= ppc.column_width[0];
	tmp -= ppc.column_width[4];
	tmp -= ppc.column_width[5];
	tmp -= ppc.column_width[6];
	
	/* info=1/4 payee=1/4 memo=2/4 */
	ppc.column_width[1] = tmp / 4;;
	ppc.column_width[2] = tmp / 4;
	ppc.column_width[3] = 2*tmp / 4;

	DB( g_print(" page width=%f, remain width=%f\n", width, tmp) );
	
	#if MYDEBUG == 1
	for(i=0;i<PDF_NUMCOL;i++)
		g_print(" col%d=%g ", i, ppc.column_width[i]);

	g_print("\n");
	#endif

	DB( g_print("\n - start printing\n") );
	
	gint y;
	page = 1;
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Transaction *txn;
	int tw, th;
			
		gtk_tree_model_get (model, &iter, LST_DSPOPE_DATAS, &txn, -1);

		//DB( g_print(" - %d, %d, %s\n", x, y, txn->memo) );
		if(cur_page_line == 1)
		{
			//helpdraw
			#if HELPDRAW == 1
			//page with margin
			hb_pdf_draw_help_rect(cr, 0xFF0000FF, ppc.ml+0.5, ppc.mt+0.5, width-(ppc.ml+ppc.mr), height - (ppc.mt+ppc.mb));
			hb_pdf_draw_help_rect(cr, 0xFF00FFFF, ppc.ml+0.5, ppc.mt+0.5, width-(ppc.ml+ppc.mr), header_height);
			#endif

			cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

			// draw account title
			pango_font_description_set_absolute_size(ppc.desc, PDF_FONT_TITLE * PANGO_SCALE);
			pango_layout_set_font_description (layout, ppc.desc);

			pango_layout_set_text (layout, accname, -1);
			pango_layout_get_pixel_size (layout, &tw, &th);
			cairo_move_to(cr, pwidth/2 - (tw/2), ppc.mt);
			pango_cairo_show_layout (cr, layout);

			// draw column titles
			pango_font_description_set_absolute_size(ppc.desc, PDF_FONT_NORMAL * PANGO_SCALE);
			pango_layout_set_font_description (layout, ppc.desc);

			g_sprintf(dbuffer, "Page %d/%d", page, num_pages);
			pango_layout_set_text (layout, dbuffer, -1);
			pango_layout_get_pixel_size (layout, &tw, &th);
			cairo_move_to(cr, pwidth - ppc.mr - tw, ppc.mt);
			pango_cairo_show_layout (cr, layout);

			//x = ppc.ml;
			y = ppc.mt + header_height - (PDF_FONT_NORMAL + PDF_LINE_MARGIN);
			hb_pdf_set_col_title(&ppc);

			hb_pdf_draw_line(&ppc, cr, y, TRUE, FALSE);
		}

		/* print a single line */
		//x = ppc.ml;
		y = ppc.mt + header_height + (cur_page_line * (PDF_FONT_NORMAL + PDF_LINE_MARGIN));



		/* reset struct */
		for(i=0;i<PDF_NUMCOL;i++)
		{
			ppc.column_txt[i] = NULL;
		}
		
		i = 0;
		g_date_set_julian (date, txn->date);
		g_date_strftime (dbuffer, 255-1, "%x", date);
		ppc.column_txt[i] = dbuffer;
		
		i = 1;
		ppc.column_txt[i] = txn->info;

		i = 2;
		Payee *p = da_pay_get(txn->kpay);
		if(p)
			ppc.column_txt[i] = p->name;

		i = 3;
		/*Category *c = da_cat_get(txn->kcat);
		if(c)
			ppc.column_txt[i] = da_cat_get_fullname(c);*/
		ppc.column_txt[i] = txn->memo;
			
		i = 4;
		hb_strfnum(amtbuf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->amount, txn->kcur, GLOBALS->minor);
		ppc.column_txt[i] = amtbuf;

		i = 5;
		ppc.column_txt[i] = "";
		if(txn->status == TXN_STATUS_CLEARED)
			ppc.column_txt[i] = "c";
		else
		if(txn->status == TXN_STATUS_RECONCILED)
			ppc.column_txt[i] = "R";
		
		i = 6;
		hb_strfnum(balbuf, G_ASCII_DTOSTR_BUF_SIZE-1, txn->balance, txn->kcur, GLOBALS->minor);
		ppc.column_txt[i] = balbuf;

		hb_pdf_draw_line(&ppc, cr, y, FALSE, (cur_page_line % 2));
		
		/* free any fullcat name */
		/*if(ppc.column_txt[3] != NULL)
			g_free(ppc.column_txt[3]);*/
		
		/* export page */		
		if(cur_page_line >= lpp)
		{
			DB( g_print("\n - next page %d\n", page) );
			
			cairo_show_page(cr);
			cur_page_line = 0;
			page++;
		}

		cur_page_line++;
		tot_lines++;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	g_date_free(date);

	g_object_unref (layout);
	pango_font_description_free (ppc.desc);

	cairo_destroy (cr);
	cairo_surface_destroy (surf);

}


