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

#include "homebank.h"
#include "hb-import.h"


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





Account *import_create_account(gchar *name, gchar *number)
{
Account *accitem, *existitem;

	//first check we do not have already this imported account
	existitem = da_acc_get_by_imp_name(name);
	if(existitem != NULL)
		return existitem;

	DB( g_print(" ** create acc: '%s' '%s'\n", name, number) );

	accitem = da_acc_malloc();
	accitem->key  = da_acc_get_max_key() + 1;
	accitem->pos  = da_acc_length() + 1;

	// existing named account ?
	existitem = da_acc_get_by_name(name);
	if(existitem != NULL)
		accitem->imp_key = existitem->key;

	if(!existitem && *name != 0)
		accitem->name = g_strdup(name);
	else
		accitem->name = g_strdup_printf(_("(account %d)"), accitem->key);

	accitem->imp_name = g_strdup(name);

	if(number)
		accitem->number = g_strdup(number);

	//fixed 5.1.2
	accitem->kcur = GLOBALS->kcur;

	accitem->imported = TRUE;
	da_acc_insert(accitem);

	return accitem;
}







