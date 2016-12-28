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

#ifndef __LIST_ACCOUNT__H__
#define __LIST_ACCOUNT__H__

/* lst acc datatype */
enum
{
	DSPACC_TYPE_NORMAL,
	DSPACC_TYPE_HEADER,
	DSPACC_TYPE_SUBTOTAL,
	DSPACC_TYPE_TOTAL
};


/* list display account */
enum
{
	LST_DSPACC_DATAS,
	LST_DSPACC_DATATYPE,
	LST_DSPACC_STATE,	/* fake column */
	LST_DSPACC_NAME,	/* fake column */
	LST_DSPACC_BANK,
	LST_DSPACC_TODAY,
	LST_DSPACC_FUTURE,
	NUM_LST_DSPACC
};


GtkWidget *create_list_account(void);

#endif
