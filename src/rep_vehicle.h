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

#ifndef __HOMEBANK_REPVEHICLE_H__
#define __HOMEBANK_REPVEHICLE_H__

enum {
	HID_REPVEHICLE_MINDATE,
	HID_REPVEHICLE_MAXDATE,
	HID_REPVEHICLE_RANGE,
	HID_REPVEHICLE_VEHICLE,
	MAX_REPVEHICLE_HID
};

enum {
	CAR_RES_METER = 1,
	CAR_RES_FUEL,
	CAR_RES_FUELCOST,
	CAR_RES_OTHERCOST,
	CAR_RES_TOTALCOST,
	MAX_CAR_RES
};


struct repvehicle_data
{
	GtkWidget	*window;

	//GtkWidget	*TX_info;
	GtkWidget	*CM_minor;
	GtkWidget	*LV_report;
	GtkWidget	*PO_cat;

	GtkWidget	*PO_mindate, *PO_maxdate;

	GtkWidget	*CY_range;
	GtkWidget	*GR_result;

	GtkWidget	*LA_avera[MAX_CAR_RES];
	GtkWidget	*LA_total[MAX_CAR_RES];

	GList		*vehicle_list;

	guint		total_dist;
	gdouble		total_fuel;
	gdouble		total_fuelcost;
	gdouble		total_misccost;


	Filter		*filter;

	gulong		handler_id[MAX_REPVEHICLE_HID];
};

//extern gchar *CYA_FLT_SELECT[];

/* list stat */
enum
{
	LST_CAR_DATE,
	LST_CAR_WORDING,
	LST_CAR_METER,
	LST_CAR_FUEL,
	LST_CAR_PRICE,
	LST_CAR_AMOUNT,
	LST_CAR_DIST,
	LST_CAR_100KM,
	LST_CAR_DISTBYVOL,
	LST_CAR_PARTIAL,
	NUM_LST_CAR
};




GtkWidget *repcost_window_new(void);

#endif
