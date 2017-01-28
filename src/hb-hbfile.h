/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2017 Maxime DOYEN
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

#ifndef __HB_HBFILE_H__
#define __HB_HBFILE_H__

GList *hbfile_transaction_get_all(void);
GQueue *hbfile_transaction_get_partial(guint32 minjulian, guint32 maxjulian);
GQueue *hbfile_transaction_get_partial_budget(guint32 minjulian, guint32 maxjulian);

gboolean hbfile_file_hasbackup(gchar *filepath);

void hbfile_change_owner(gchar *owner);
void hbfile_change_filepath(gchar *filepath);

void hbfile_cleanup(gboolean file_clear);
void hbfile_setup(gboolean file_clear);

void hbfile_anonymize(void);
void hbfile_sanity_check(void);

void hbfile_change_basecurrency(guint32 key);
void hbfile_replace_basecurrency(Currency4217 *curfmt);

#endif

