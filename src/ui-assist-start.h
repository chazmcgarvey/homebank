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

#ifndef __HB_UI_ASSIST_START_GTK_H__
#define __HB_UI_ASSIST_START_GTK_H__


struct assist_start_data
{
	GtkWidget	*window;
	//GtkWidget	*pages[NUM_PAGE];
	GtkWidget	*ST_owner;
	GtkWidget   *LB_currency;
	GtkWidget   *BT_change;

	GtkWidget	*TX_lang;
	GtkWidget	*TX_file;
	GtkWidget	*ok_image, *ko_image;
	GtkWidget	*CM_load;

	GtkWidget	*ST_name;
	GtkWidget	*CY_type;
	GtkWidget	*ST_number;
	GtkWidget	*ST_initial;
	GtkWidget	*ST_minimum;

	Currency4217 *curfmt;
	gchar	   *pathfilename;
};



GtkWidget *ui_start_assistant(void);


#endif
