/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2012 Maxime DOYEN
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

#include "hb-currency.h"
#include <libsoup/soup.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

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

extern Currency4217 iso4217cur[];
extern guint n_iso4217cur;

Currency4217 *iso4217format_get(gchar *code);

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void 
da_cur_free(Currency *item)
{
	DB( g_print("da_cur_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->iso_code) );

		g_free(item->name);
		g_free(item->iso_code);
		g_free(item->symbol);
		g_free(item->decimal_char);
		g_free(item->grouping_char);

		g_free(item);
	}
}


Currency *
da_cur_malloc(void)
{
	DB( g_print("da_cur_malloc\n") );
	return g_malloc0(sizeof(Currency));
}


void
da_cur_destroy(void)
{
	DB( g_print("da_cur_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_cur);
}

void
da_cur_new(void)
{
Currency4217 *curfmt;
	
	DB( g_print("da_cur_new\n") );
	GLOBALS->h_cur = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_cur_free);

	// insert default base currency
	curfmt = iso4217format_get(PREFS->IntCurrSymbol);
	if(curfmt == NULL)
		curfmt = iso4217format_get("USD");

	if(curfmt)
	{
	DB( g_printf("curfmt %p\n", curfmt) );

		currency_add_from_user(curfmt);
	}
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_cur_max_key_ghfunc(gpointer key, Currency *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_cur_iso_grfunc(gpointer key, Currency *item, gchar *iso)
{
	if( iso && item->iso_code )
	{
		if(!strcasecmp(iso, item->iso_code))
			return TRUE;
	}
	return FALSE;
}

guint
da_cur_length(void)
{
	return g_hash_table_size(GLOBALS->h_cur);
}

gboolean
da_cur_remove(guint32 key)
{
	DB( g_print("da_cur_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_cur, &key);
}


void 
da_cur_init_from4217(Currency *cur, Currency4217 *curfmt)
{
	cur->symbol        = g_strdup(curfmt->curr_symbol);
	cur->sym_prefix    = curfmt->curr_is_prefix;
	cur->decimal_char  = g_strdup(curfmt->curr_dec_char);
	cur->grouping_char = g_strdup(curfmt->curr_grp_char);
	cur->frac_digits   = curfmt->curr_frac_digit;
	da_cur_initformat(cur);
}


void
da_cur_initformat(Currency *item)
{
	DB( g_print("init currency %s, %d \n", item->symbol, item->frac_digits) );

	// for formatd
	g_snprintf(item->format , 8-1, "%%.%df", item->frac_digits);

	if(item->sym_prefix == TRUE)
		g_snprintf(item->monfmt , 32-1, "%s %%s", item->symbol);
	else
		g_snprintf(item->monfmt , 32-1, "%%s %s", item->symbol);

	DB( g_print("fmt '%s'\n", item->format) );
	DB( g_print("monfmt '%s'\n", item->monfmt) );
}


gboolean
da_cur_insert(Currency *item)
{
guint32 *new_key;

	DB( g_print("da_cur_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_cur, new_key, item);	

	da_cur_initformat(item);
	
	return TRUE;
}


gboolean
da_cur_append(Currency *item)
{
Currency *existitem;
guint32 *new_key;

	DB( g_print("da_cur_append\n") );

	/* ensure no duplicate */
	//g_strstrip(item->name);
	if(item->iso_code != NULL)
	{
		existitem = da_cur_get_by_iso_code( item->iso_code );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_cur_get_max_key() + 1;
			item->key = *new_key;
			//item->pos = da_cur_length() + 1;

			DB( g_print(" -> insert id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_cur, new_key, item);	

			da_cur_initformat(item);

			return TRUE;
		}

		
	}

	DB( g_print(" -> %s already exist: %d\n", item->iso_code, item->key) );

	return FALSE;
}


guint32
da_cur_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_cur, (GHFunc)da_cur_max_key_ghfunc, &max_key);
	return max_key;
}



Currency *
da_cur_get_by_iso_code(gchar *iso_code)
{
	DB( g_print("da_cur_get_by_iso_code\n") );

	return g_hash_table_find(GLOBALS->h_cur, (GHRFunc)da_cur_iso_grfunc, iso_code);
}

Currency *
da_cur_get(guint32 key)
{
	//DB( g_print("da_cur_get\n") );

	return g_hash_table_lookup(GLOBALS->h_cur, &key);
}





/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * currency_is_used:
 * 
 * controls if a currency is used [base or account]
 * 
 * Return value: TRUE if used, FALSE, otherwise
 */
gboolean
currency_is_used(guint32 key)
{
GList *list;
gboolean retval;

	if(GLOBALS->kcur == key)
		return TRUE;
	
	retval = FALSE;
	
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if(item->kcur == key)
		{	
			retval = TRUE;
			goto end;
		}		
		list = g_list_next(list);
	}

end:
	g_list_free(list);

	return retval;
}



Currency4217 *iso4217format_get(gchar *code)
{
Currency4217 *cur;
guint i;

	for (i = 0; i< n_iso4217cur; i++)
	{
		cur = &iso4217cur[i];
		if(g_strcmp0(cur->curr_iso_code, code) == 0)
		{
			return cur;
		}
	}
	return NULL;
}


static void currency_get_system_format(Currency *item)
{
	DB( g_printf("\n[(currency] get format from system '%s'\n", item->iso_code) );


#ifdef G_OS_UNIX

struct lconv *lc = localeconv();

	DB( g_print("int_curr_symbol '%s'\n", lc->int_curr_symbol) );
	
	DB( g_print("mon_decimal_point is utf8: %d\n", g_utf8_validate(lc->mon_decimal_point, -1, NULL)) );
	DB( g_print("mon_decimal_point '%s'\n", lc->mon_decimal_point) );
	DB( g_print("mon_decimal_point %x %x %x %x\n", lc->mon_decimal_point[0], lc->mon_decimal_point[1], lc->mon_decimal_point[2], lc->mon_decimal_point[3]) );

	DB( g_print("mon_thousands_sep is utf8: %d\n", g_utf8_validate(lc->mon_thousands_sep, -1, NULL)) );
	DB( g_print("mon_thousands_sep '%s'\n", lc->mon_thousands_sep) );
	DB( g_print("mon_thousands_sep %x %x %x %x\n", lc->mon_thousands_sep[0], lc->mon_thousands_sep[1], lc->mon_thousands_sep[2], lc->mon_thousands_sep[3]) );


	DB( g_print("frac_digits '%d'\n", (gint)lc->frac_digits) );

	DB( g_print("currency_symbol '%s'\n", lc->currency_symbol) );

	DB( g_print("p_cs_precedes '%d'\n", lc->p_cs_precedes) );

	DB( g_print("n_cs_precedes '%d'\n", lc->n_cs_precedes) );

	/* ok assign */
	item->symbol = g_strdup(lc->currency_symbol);

	if( lc->p_cs_precedes || lc->n_cs_precedes )
	{
		item->sym_prefix  = TRUE;
		DB( g_print("locale mon cs is a prefix\n") );
	}
	else
	{
		item->sym_prefix  = FALSE;
	}

	item->decimal_char = g_strdup(lc->mon_decimal_point);

	item->grouping_char = g_strdup(lc->mon_thousands_sep);

	//todo:fix
	//PREFS->base_cur.grouping_char = g_locale_to_utf8(lc->mon_thousands_sep, -1, NULL, NULL, NULL);
	//PREFS->base_cur.grouping_char = g_convert (lc->mon_thousands_sep, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);

	DB( g_print(" -> grouping_char: '%s'\n", item->grouping_char) );

	item->frac_digits = lc->frac_digits;

	//fix 378992/421228
	if( item->frac_digits > MAX_FRAC_DIGIT )
	{
		item->frac_digits = 2;
		g_free(item->decimal_char);
		item->decimal_char = g_strdup(".");
	}

#else
	#ifdef G_OS_WIN32
    #define BUFFER_SIZE 512
    char buffer[BUFFER_SIZE];
    //LPWSTR wcBuffer = buffer;
    LPSTR wcBuffer = buffer;
    int iResult;

	//https://msdn.microsoft.com/en-us/library/windows/desktop/dd464799%28v=vs.85%29.aspx

    //see g_locale_to_utf8 here
	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SCURRENCY, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_SCURRENCY='%s'\n", buffer) );
        item->symbol = g_locale_to_utf8(buffer, -1, NULL, NULL, NULL);
    }

	item->sym_prefix = FALSE;
	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_IPOSSYMPRECEDES, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_IPOSSYMPRECEDES='%s'\n", buffer) );
		//todo item->sym_prefix
        
    }

	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_SDECIMAL='%s'\n", buffer) );
        item->decimal_char = g_locale_to_utf8(buffer, -1, NULL, NULL, NULL);
    }

	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_STHOUSAND='%s'\n", buffer) );
        item->grouping_char = g_locale_to_utf8(buffer, -1, NULL, NULL, NULL);
    }

	iResult = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, wcBuffer, BUFFER_SIZE);
    if(iResult > 0)
    {
        DB( g_print("LOCALE_ICURRDIGITS='%s'\n", buffer) );
        item->frac_digits = atoi(buffer);
    }
	else
	 	item->frac_digits = 2;

	#endif
#endif


}


Currency *currency_add_from_user(Currency4217 *curfmt)
{
Currency *item;
	
	DB( g_printf("\n[(currency] found adding %s\n", curfmt->curr_iso_code) );

	//item = da_cur_get_by_iso_code(curfmt->curr_iso_code);
	
	item = da_cur_malloc();
	//no mem alloc here
	//item->key = i;
	if(curfmt != NULL)
	{
		item->name = g_strdup(curfmt->name);
		//item->country = cur.country_name;
		item->iso_code = g_strdup(curfmt->curr_iso_code);
		
		//1634615 if the currency match the system, fill with it
		if(!strcmp(item->iso_code, PREFS->IntCurrSymbol))
		{
			currency_get_system_format(item);
		}
		else
		{
			item->frac_digits = curfmt->curr_frac_digit;
			item->symbol = g_strdup(curfmt->curr_symbol);
			item->sym_prefix = curfmt->curr_is_prefix;
			item->decimal_char = g_strdup(curfmt->curr_dec_char);
			item->grouping_char = g_strdup(curfmt->curr_grp_char);
		}
	}
	else
	{
		item->name = g_strdup("unknow");
		//item->country = cur.country_name;
		item->iso_code = g_strdup("XXX");
		item->frac_digits = 2;
		item->symbol = g_strdup("XXX");
		item->sym_prefix = FALSE;
		item->decimal_char = g_strdup(".");
		item->grouping_char = NULL;
	}
		
	da_cur_append(item);

	return item;
}





static void
start_element_handler (GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names,
						const gchar **attribute_values, gpointer user_data, GError **error)
{
ParseExchangeContext *ctx = user_data;
gint i;

	//DB( g_print("** start element: '%s' iso=%s\n", element_name, ctx->iso) );

	ctx->elt_name = element_name;

	switch(element_name[0])
	{
		case 'r':
		{
			if(!strcmp (element_name, "rate"))
			{
				i = 0;
				//DB( g_print(" att='%s' val='%s'\n", attribute_names[i], attribute_values[i]) );
				//we have only 1 attribute id :: store isocode pair
				if(attribute_names[i] != NULL && !strcmp (attribute_names[i], "id"))
				{
					g_stpcpy (ctx->iso, attribute_values[i]);		
				}
			}
		}
		break;
	}
}


static void 
text_handler(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error)
{
ParseExchangeContext *ctx = user_data;

	if(text_len == 0)
		return;

	//DB( g_print("** text: '%s' %d\n", text, text_len) );

	if(!strcmp (ctx->elt_name, "Rate"))
	{
		ctx->rate = g_ascii_strtod(text, NULL);
		//DB( g_print(" stored '%s' %.2f\n", text, ctx->rate) );
	}
}


static void 
end_element_handler (GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error)
{
ParseExchangeContext *ctx = user_data;
Currency *cur;

	DB( g_print("** end element: '%s'\n", element_name) );

	if(!strcmp (element_name, "rate"))
	{
		DB( g_print(" should store here !!\n") );
		DB( g_print(" %s %f\n", ctx->iso, ctx->rate) );
		cur = da_cur_get_by_iso_code (ctx->iso + 3);
		if(cur)
		{
			DB( g_print(" found cur='%s'\n", cur->iso_code) );
			cur->rate = ctx->rate;
			cur->mdate = GLOBALS->today;
		}
		
		//clean all
		ctx->elt_name = NULL;
		*ctx->iso = '\0';
		ctx->rate = 0.0;
	}
}
       

static GMarkupParser hb_xchange_parser = {
	start_element_handler,
	end_element_handler,
	text_handler,
	NULL,
	NULL  //cleanup
};


static gboolean currency_online_parse(const gchar *buffer, GError **error)
{
GMarkupParseContext *context;
ParseExchangeContext ctx;
gboolean retval;

	memset(&ctx, 0, sizeof(ParseExchangeContext));
	context = g_markup_parse_context_new (&hb_xchange_parser, 0, &ctx, NULL);

	retval = g_markup_parse_context_parse (context, buffer, -1, error);
	//retval = g_markup_parse_context_parse (context, badyahooxml, -1, error);
	g_markup_parse_context_free (context);

	return retval;
}


static gchar *currency_get_query(void)
{
GList *list;
GString *node;
Currency *base;
Currency *item;
gint i;

//http://query.yahooapis.com/v1/public/yql
//?q=select * from yahoo.finance.xchange where pair in ("EURGBP","EURUSD")
//&env=store://datatables.org/alltableswithkeys
	
	node = g_string_sized_new(1024);
	g_string_append(node, "http://query.yahooapis.com/v1/public/yql");
	g_string_append(node, "?q=select * from yahoo.finance.xchange where pair in (");

	base = da_cur_get (GLOBALS->kcur);

	list = g_hash_table_get_values(GLOBALS->h_cur);
	i = g_list_length (list) - 1;
	
	while (list != NULL)
	{
	item = list->data;

		if(item->key != GLOBALS->kcur)
		{
			g_string_append_printf(node, "\"%s%s\"", base->iso_code, item->iso_code);
			if(i > 1)
			{
				g_string_append(node, ",");
			}
			i--;
		}
		list = g_list_next(list);
	}
	g_list_free(list);

	g_string_append(node, ")&env=store://datatables.org/alltableswithkeys");

	return g_string_free(node, FALSE);
}


gboolean currency_sync_online(GError **error)
{
SoupSession *session;
SoupMessage *msg;
gchar *query;
gboolean retval = TRUE;

	DB( g_printf("\n[currency] sync online\n") );

	query = currency_get_query();
	DB( g_printf(" - query is '%s'\n", query) );
	
	session = soup_session_new ();
	msg = soup_message_new ("GET", query);
	if(msg != NULL)
	{
		soup_session_send_message (session, msg);
		DB( g_print("status_code: %d %d\n", msg->status_code, SOUP_STATUS_IS_SUCCESSFUL(msg->status_code) ) );
		DB( g_print("reason: %s\n", msg->reason_phrase) );
		DB( g_print("datas: %s\n", msg->response_body->data) );
		
		if( SOUP_STATUS_IS_SUCCESSFUL(msg->status_code) == TRUE )
		{
			retval = currency_online_parse(msg->response_body->data, error);
		}
		else
		{
			*error = g_error_new_literal(1, msg->status_code, msg->reason_phrase);
			retval = FALSE;
		}
	}
	else
	{
		*error = g_error_new_literal(1, 0, "cannot parse URI");
		retval = FALSE;
	}

	g_free(query);
	
	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */



//struct iso_4217_currency iso_4217_currencies[];

/*debug testing
static void fill_currency(void)
{
gint i;
struct iso_4217_currency cur;
Currency *item;
	
	for (i = 0; i< 500; i++)
	{
		cur = iso_4217_currencies[i];

		if(cur.iso_code == NULL)
			break;
		
		item = da_cur_malloc();
		//no mem alloc here
		item->key = i;
		item->name = cur.currency;
		item->country = cur.country;
		item->iso_code = cur.iso_code;

		da_cur_insert(item);

	}

	

}*/

Currency4217 iso4217cur[] =
{
	{ "AED", 2, ".", ",", TRUE, "د.إ.‏", "UAE Dirham" },
	{ "AFN", 0, ",", ",", TRUE, "؋", "Afghani" },
	{ "ALL", 0, ",", " ", FALSE, "Lekë", "Lek" },
	{ "AMD", 2, ".", ",", FALSE, "֏", "Armenian Dram" },
	{ "ANG", 2, ",", ",", TRUE, "NAf.", "Netherlands Antillian Guilder" },
	{ "AOA", 2, ",", " ", FALSE, "Kz", "Kwanza" },
	{ "ARS", 2, ",", ".", TRUE, "$", "Argentine Peso" },
	{ "AUD", 2, ".", ",", TRUE, "$", "Australian Dollar" },
	{ "AWG", 2, ",", ".", TRUE, "Afl.", "Aruban Guilder" },
	{ "AZN", 2, ",", " ", TRUE, "₼", "Azerbaijanian Manat" },
	{ "BAM", 2, ",", ".", FALSE, "KM", "Convertible Marks" },
	{ "BBD", 2, ".", ",", TRUE, "$", "Barbados Dollar" },
	{ "BDT", 2, ".", ",", TRUE, "৳", "Taka" },
	{ "BGN", 2, ",", " ", FALSE, "лв.", "Bulgarian Lev" },
	{ "BHD", 3, ".", ",", TRUE, "د.ب.‏", "Bahraini Dinar" },
	{ "BIF", 0, ",", " ", FALSE, "FBu", "Burundi Franc" },
	{ "BMD", 2, ".", ",", TRUE, "$", "Bermudian Dollar" },
	{ "BND", 2, ",", ".", TRUE, "$", "Brunei Dollar" },
	{ "BOB", 2, ",", ".", TRUE, "Bs", "Boliviano" },
	{ "BOV", 2, ".", "", FALSE, "BOV", "Mvdol" },
	{ "BRL", 2, ",", ".", TRUE, "R$", "Brazilian Real" },
	{ "BSD", 2, ".", ",", TRUE, "$", "Bahamian Dollar" },
	{ "BTN", 2, ".", ",", TRUE, "Nu.", "Ngultrum" },
	{ "BWP", 2, ".", " ", TRUE, "P", "Pula" },
	{ "BYR", 0, ",", " ", FALSE, "Br", "Belarussian Ruble" },
	{ "BZD", 2, ".", ",", TRUE, "$", "Belize Dollar" },
	{ "CAD", 2, ",", " ", TRUE, "$", "Canadian Dollar" },
	{ "CDF", 2, ",", " ", TRUE, "FC", "Congolese Franc" },
	{ "CHE", 2, ".", "", FALSE, "CHE", "WIR Euro" },
	{ "CHF", 2, ",", "'", TRUE, "CHF", "Swiss Franc" },
	{ "CHW", 2, ".", "", FALSE, "CHW", "WIR Franc" },
	{ "CLF", 2, ".", "", FALSE, "CLF", "Unidades de fomento" },
	{ "CLP", 0, ",", ".", TRUE, "$", "Chilean Peso" },
	{ "CNY", 2, ".", ",", TRUE, "¥", "Yuan Renminbi" },
	{ "COP", 0, ",", ".", TRUE, "$", "Colombian Peso" },
	{ "COU", 2, ".", "", FALSE, "COU", "Unidad de Valor Real" },
	{ "CRC", 0, ",", ".", TRUE, "₡", "Costa Rican Colon" },
	{ "CUP", 2, ".", ",", TRUE, "$", "Cuban Peso" },
	{ "CVE", 2, "$", " ", FALSE, "​", "Cape Verde Escudo" },
	{ "CYP", 2, ".", "", FALSE, "CYP", "Cyprus Pound" },
	{ "CZK", 2, ",", " ", FALSE, "Kč", "Czech Koruna" },
	{ "DJF", 0, ",", " ", TRUE, "Fdj", "Djibouti Franc" },
	{ "DKK", 2, ",", ".", TRUE, "kr", "Danish Krone" },
	{ "DOP", 2, ".", ",", TRUE, "$", "Dominican Peso" },
	{ "DZD", 2, ",", " ", FALSE, "DA", "Algerian Dinar" },
	{ "EEK", 2, ".", "", FALSE, "EEK", "Kroon" },
	{ "EGP", 2, ".", ",", TRUE, "ج.م.‏", "Egyptian Pound" },
	{ "ERN", 2, ".", ",", TRUE, "Nfk", "Nakfa" },
	{ "ETB", 2, ".", ",", TRUE, "Br", "Ethiopian Birr" },
	{ "EUR", 2, ",", " ", TRUE, "€", "Euro" },
	{ "FJD", 2, ".", ",", TRUE, "$", "Fiji Dollar" },
	{ "FKP", 2, ".", ",", TRUE, "£", "Falkland Islands Pound" },
	{ "GBP", 2, ".", ",", TRUE, "£", "Pound Sterling" },
	{ "GEL", 2, ",", " ", TRUE, "₾", "Lari" },
	{ "GHS", 2, ".", ",", TRUE, "GH₵", "Ghana Cedi" },
	{ "GIP", 2, ".", ",", TRUE, "£", "Gibraltar Pound" },
	{ "GMD", 2, ".", ",", TRUE, "D", "Dalasi" },
	{ "GNF", 0, ",", " ", TRUE, "FG", "Guinea Franc" },
	{ "GTQ", 2, ".", ",", TRUE, "Q", "Quetzal" },
	{ "GYD", 0, ".", ",", TRUE, "$", "Guyana Dollar" },
	{ "HKD", 2, ".", ",", TRUE, "$", "Hong Kong Dollar" },
	{ "HNL", 2, ".", ",", TRUE, "L", "Lempira" },
	{ "HRK", 2, ",", ".", FALSE, "kn", "Croatian Kuna" },
	{ "HTG", 2, ",", " ", FALSE, "G", "Gourde" },
	{ "HUF", 2, ",", " ", FALSE, "HUF", "Forint" },
	{ "IDR", 0, ",", ".", TRUE, "Rp", "Rupiah" },
	{ "ILS", 2, ".", ",", TRUE, "₪", "New Israeli Sheqel" },
	{ "INR", 2, ".", ",", TRUE, "₹", "Indian Rupee" },
	{ "IQD", 2, ".", ",", TRUE, "د.ع.‏", "Iraqi Dinar" },
	{ "IRR", 2, "/", ",", TRUE, "ريال", "Iranian Rial" },
	{ "ISK", 0, ",", ".", FALSE, "ISK", "Iceland Krona" },
	{ "JMD", 2, ".", ",", TRUE, "$", "Jamaican Dollar" },
	{ "JOD", 3, ".", ",", TRUE, "د.ا.‏", "Jordanian Dinar" },
	{ "JPY", 0, ".", ",", TRUE, "¥", "Yen" },
	{ "KES", 2, ".", ",", TRUE, "Ksh", "Kenyan Shilling" },
	{ "KGS", 2, ",", " ", FALSE, "сом", "Som" },
	{ "KHR", 2, ".", ",", FALSE, "៛", "Riel" },
	{ "KMF", 0, ",", " ", TRUE, "CF", "Comoro Franc" },
	{ "KPW", 2, ".", "", FALSE, "KPW", "North Korean Won" },
	{ "KRW", 0, ".", ",", TRUE, "₩", "Won" },
	{ "KWD", 3, ".", ",", TRUE, "د.ك.‏", "Kuwaiti Dinar" },
	{ "KYD", 2, ".", ",", TRUE, "$", "Cayman Islands Dollar" },
	{ "KZT", 2, ",", " ", FALSE, "₸", "Tenge" },
	{ "LAK", 0, ",", ".", TRUE, "₭", "Kip" },
	{ "LBP", 2, ".", ",", TRUE, "ل.ل.‏", "Lebanese Pound" },
	{ "LKR", 2, ".", ",", TRUE, "Rs.", "Sri Lanka Rupee" },
	{ "LRD", 2, ".", ",", TRUE, "$", "Liberian Dollar" },
	{ "LSL", 2, ".", "", FALSE, "LSL", "Loti" },
	{ "LTL", 2, ".", "", FALSE, "LTL", "Lithuanian Litas" },
	{ "LVL", 2, ".", "", FALSE, "LVL", "Latvian Lats" },
	{ "LYD", 3, ".", ",", TRUE, "د.ل.‏", "Libyan Dinar" },
	{ "MAD", 2, ",", " ", FALSE, "DH", "Moroccan Dirham" },
	{ "MDL", 2, ",", " ", FALSE, "L", "Moldovan Leu" },
	{ "MGA", 0, ",", " ", TRUE, "Ar", "Malagasy Ariary" },
	{ "MKD", 2, ",", " ", TRUE, "den", "Denar" },
	{ "MMK", 0, ".", ",", TRUE, "K", "Kyat" },
	{ "MNT", 0, ".", ",", TRUE, "₮", "Tugrik" },
	{ "MOP", 2, ",", " ", TRUE, "MOP", "Pataca" },
	{ "MRO", 0, ",", " ", TRUE, "UM", "Ouguiya" },
	{ "MTL", 2, ".", "", FALSE, "MTL", "Maltese Lira" },
	{ "MUR", 0, ",", " ", TRUE, "Rs", "Mauritius Rupee" },
	{ "MVR", 2, ".", ",", FALSE, "ރ.", "Rufiyaa" },
	{ "MWK", 2, ".", ",", TRUE, "MK", "Kwacha" },
	{ "MXN", 2, ".", ",", TRUE, "$", "Mexican Peso" },
	{ "MXV", 2, ".", "", FALSE, "MXV", "Mexican Unidad de Inversion (UDI)" },
	{ "MYR", 2, ".", ",", TRUE, "RM", "Malaysian Ringgit" },
	{ "MZN", 2, ",", " ", FALSE, "MTn", "Metical" },
	{ "NAD", 2, ",", " ", TRUE, "$", "Namibia Dollar" },
	{ "NGN", 2, ".", ",", TRUE, "₦", "Naira" },
	{ "NIO", 2, ".", ",", TRUE, "C$", "Cordoba Oro" },
	{ "NOK", 2, ",", " ", TRUE, "kr", "Norwegian Krone" },
	{ "NPR", 2, ".", ",", TRUE, "रु", "Nepalese Rupee" },
	{ "NZD", 2, ".", ",", TRUE, "$", "New Zealand Dollar" },
	{ "OMR", 3, ".", ",", TRUE, "ر.ع.‏", "Rial Omani" },
	{ "PAB", 2, ".", ",", TRUE, "B/.", "Balboa" },
	{ "PEN", 2, ".", ",", TRUE, "S/.", "Nuevo Sol" },
	{ "PGK", 2, ".", ",", TRUE, "K", "Kina" },
	{ "PHP", 2, ",", ",", TRUE, "₱", "Philippine Peso" },
	{ "PKR", 0, ".", ",", TRUE, "Rs", "Pakistan Rupee" },
	{ "PLN", 2, ",", " ", FALSE, "zł", "Zloty" },
	{ "PYG", 0, ",", ".", TRUE, "₲", "Guarani" },
	{ "QAR", 2, ".", ",", TRUE, "ر.ق.‏", "Qatari Rial" },
	{ "RON", 2, ",", ".", FALSE, "RON", "New Leu" },
	{ "RSD", 0, ",", ".", FALSE, "RSD", "Serbian Dinar" },
	{ "RUB", 2, ",", " ", TRUE, "₽", "Russian Ruble" },
	{ "RWF", 0, ",", " ", TRUE, "RF", "Rwanda Franc" },
	{ "SAR", 2, ".", ",", TRUE, "ر.س.‏", "Saudi Riyal" },
	{ "SBD", 2, ".", ",", TRUE, "$", "Solomon Islands Dollar" },
	{ "SCR", 2, ",", " ", TRUE, "SR", "Seychelles Rupee" },
	{ "SDG", 2, ".", ",", TRUE, "SDG", "Sudanese Pound" },
	{ "SEK", 2, ",", ".", FALSE, "kr", "Swedish Krona" },
	{ "SGD", 2, ".", ",", TRUE, "$", "Singapore Dollar" },
	{ "SHP", 2, ".", ",", TRUE, "£", "Saint Helena Pound" },
	{ "SLL", 0, ".", ",", TRUE, "Le", "Leone" },
	{ "SOS", 0, ".", ",", TRUE, "S", "Somali Shilling" },
	{ "SRD", 2, ",", ".", TRUE, "$", "Surinam Dollar" },
	{ "STD", 0, ",", " ", FALSE, "Db", "Dobra" },
	{ "SVC", 2, ".", "", FALSE, "SVC", "El Salvador Colon" },
	{ "SYP", 0, ",", " ", TRUE, "LS", "Syrian Pound" },
	{ "SZL", 2, ",", " ", TRUE, "E", "Lilangeni" },
	{ "THB", 2, ".", ",", TRUE, "฿", "Baht" },
	{ "TJS", 2, ",", " ", FALSE, "смн", "Somoni" },
	{ "TMM", 2, ".", "", FALSE, "TMM", "Manat" },
	{ "TND", 3, ",", " ", TRUE, "DT", "Tunisian Dinar" },
	{ "TOP", 2, ".", ",", TRUE, "T$", "Pa'anga" },
	{ "TRY", 2, ",", ".", FALSE, "₺", "New Turkish Lira" },
	{ "TTD", 2, ".", ",", TRUE, "$", "Trinidad and Tobago Dollar" },
	{ "TWD", 2, ".", ",", TRUE, "NT$", "New Taiwan Dollar" },
	{ "TZS", 0, ".", ",", TRUE, "TSh", "Tanzanian Shilling" },
	{ "UAH", 2, ",", " ", FALSE, "₴", "Hryvnia" },
	{ "UGX", 0, ".", ",", TRUE, "USh", "Uganda Shilling" },
	{ "USD", 2, ",", " ", TRUE, "$", "US Dollar" },
	{ "USN", 2, ".", "", FALSE, "USN", "US Dollar (Next day)" },
	{ "USS", 2, ".", "", FALSE, "USS", "US Dollar (Same day)" },
	{ "UYI", 2, ".", "", FALSE, "UYI", "Uruguay Peso en Unidades Indexadas" },
	{ "UYU", 2, ",", ".", TRUE, "$", "Peso Uruguayo" },
	{ "UZS", 0, ",", " ", TRUE, "soʻm", "Uzbekistan Sum" },
	{ "VEF", 2, ",", ".", TRUE, "Bs.", "Bolivar Fuerte" },
	{ "VND", 2, ",", ".", FALSE, "₫", "Dong" },
	{ "VUV", 0, ",", " ", TRUE, "VT", "Vatu" },
	{ "WST", 2, ".", ",", TRUE, "WS$", "Tala" },
	{ "XAF", 0, ",", " ", TRUE, "FCFA", "CFA Franc BEAC" },
	{ "XAG", 2, ".", "", FALSE, "XAG", "Silver" },
	{ "XAU", 2, ".", "", FALSE, "XAU", "Gold" },
	{ "XBA", 2, ".", "", FALSE, "XBA", "European Composite Unit (EURCO)" },
	{ "XBB", 2, ".", "", FALSE, "XBB", "European Monetary Unit (E.M.U.-6)" },
	{ "XBC", 2, ".", "", FALSE, "XBC", "European Unit of Account 9 (E.U.A.-9)" },
	{ "XBD", 2, ".", "", FALSE, "XBD", "European Unit of Account 17 (E.U.A.-17)" },
	{ "XCD", 2, ",", " ", TRUE, "$", "East Caribbean Dollar" },
	{ "XDR", 2, ",", " ", TRUE, "XDR", "Special Drawing Rights" },
	{ "XFO", 2, ".", "", FALSE, "XFO", "Gold-Franc" },
	{ "XFU", 2, ".", "", FALSE, "XFU", "UIC-Franc" },
	{ "XOF", 0, ",", " ", TRUE, "CFA", "CFA Franc BCEAO" },
	{ "XPD", 2, ".", "", FALSE, "XPD", "Palladium" },
	{ "XPF", 0, ",", " ", FALSE, "FCFP", "CFP Franc" },
	{ "XPT", 2, ".", "", FALSE, "XPT", "Platinum" },
	{ "XTS", 2, ".", "", FALSE, "XTS", "Code for testing purposes" },
	{ "XXX", 2, ".", "", FALSE, "XXX", "No currency" },
	{ "YER", 2, ".", ",", TRUE, "ر.ي.‏", "Yemeni Rial" },
	{ "ZAR", 2, ",", " ", TRUE, "R", "Rand" },
	{ "ZMK", 2, ".", "", FALSE, "ZMK", "Zambian Kwacha" },
	{ "ZWD", 2, ".", "", FALSE, "ZWD", "Zimbabwe Dollar" }
};
guint n_iso4217cur = G_N_ELEMENTS (iso4217cur);

