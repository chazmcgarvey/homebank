/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2019 Maxime DOYEN
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

#include "dsp-mainwindow.h"


#ifndef __HUB_TRANSACTION_H__
#define __HUB_TRANSACTION_H__

typedef enum {
	HUB_TXN_TYPE_NONE,
	HUB_TXN_TYPE_FUTURE,
	HUB_TXN_TYPE_REMIND
} HbHubTxnType;


void ui_hub_transaction_populate(struct hbfile_data *data);
GtkWidget *ui_hub_transaction_create(struct hbfile_data *data, HbHubTxnType type);

#endif