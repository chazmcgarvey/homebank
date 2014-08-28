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

#ifndef __HB_ARCHIVE_GTK_H__
#define __HB_ARCHIVE_GTK_H__

enum {
	HID_ARC_MEMO,
	HID_ARC_VALID,
	HID_ARC_REMIND,
	HID_ARC_MAX
};


struct ui_arc_manage_data
{
	GList	*tmp_list;
	gint	change;
	//guint32	lastkey;
	Archive		*lastarcitem;


	GtkWidget	*LV_arc;

	GtkWidget	*PO_pay;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_amount;	//, *BT_split;
	GtkWidget	*CM_valid;
	GtkWidget	*CM_remind;
	GtkWidget	*GR_cheque;
	GtkWidget	*CM_cheque;

	GtkWidget	*NU_mode;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*LB_accto, *PO_accto;

	GtkWidget	*CM_auto;
	GtkWidget	*NB_every;
	GtkWidget	*CY_unit;
	GtkWidget	*PO_next;
	GtkWidget	*CM_limit;
	GtkWidget	*NB_limit;
	GtkWidget   *CY_weekend;

	GtkWidget	*BT_add, *BT_rem;

	gulong		handler_id[HID_ARC_MAX];

};


GtkWidget *ui_arc_manage_dialog (void);

#endif