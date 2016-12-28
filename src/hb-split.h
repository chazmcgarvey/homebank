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

#ifndef __HB_SPLIT_H__
#define __HB_SPLIT_H__


#define TXN_MAX_SPLIT 10

typedef struct _split Split;


struct _split
{
	guint32		kcat;
	gdouble		amount;
	gchar		*memo;
};


void da_splits_append(Split *txn_splits[], Split *new_split);
void da_splits_free(Split *txn_splits[]);
guint da_splits_count(Split *txn_splits[]);
guint da_splits_clone(Split *stxn_splits[], Split *dtxn_splits[]);

Split *da_split_new(guint32 kcat, gdouble amount, gchar	*memo);
guint da_splits_parse(Split *ope_splits[], gchar *cats, gchar *amounts, gchar *memos);
guint da_splits_tostring(Split *ope_splits[], gchar **cats, gchar **amounts, gchar **memos);

void split_cat_consistency (Split *txn_splits[]);


#endif