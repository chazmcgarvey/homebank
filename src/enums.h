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

#ifndef __HB_ENUMS_H__
#define __HB_ENUMS_H__


/* hbfile/account/import update flags */
enum
{
	UF_TITLE     	= 1 << 0,	//1
	UF_SENSITIVE 	= 1 << 1,	//2
	UF_BALANCE   	= 1 << 2,	//4
	UF_VISUAL    	= 1 << 3,	//8
	UF_REFRESHALL	= 1 << 4	//16
};


/*
** list pixbuf (account/transaction)
*/
enum
{
	LST_PIXBUF_ADD,
	LST_PIXBUF_EDIT,
	LST_PIXBUF_REMIND,
	LST_PIXBUF_VALID,
	LST_PIXBUF_AUTO,
	LST_PIXBUF_WARNING,
	NUM_LST_PIXBUF
};

/*
** paymode pixbuf
*/
enum
{
	PAYMODE_NONE,
	PAYMODE_CCARD,
	PAYMODE_CHECK,
	PAYMODE_CASH,
	PAYMODE_XFER,
	PAYMODE_INTXFER,
	/* 4.1 new payments here */
	PAYMODE_DCARD,
	PAYMODE_REPEATPMT,
	PAYMODE_EPAYMENT,
	PAYMODE_DEPOSIT,
	PAYMODE_FEE,
	/* 4.6 new paymode */
	PAYMODE_DIRECTDEBIT,
//	PAYMODE_,
	NUM_PAYMODE_MAX
};

/*
** toolbar item type
*/
enum
{
	TOOLBAR_SEPARATOR,
	TOOLBAR_BUTTON,
	TOOLBAR_TOGGLE
};

/*
** scheduled unit
*/
enum
{
	AUTO_UNIT_DAY,
	AUTO_UNIT_WEEK,
	AUTO_UNIT_MONTH,
	//AUTO_UNIT_QUARTER,
	AUTO_UNIT_YEAR
};


/* list display transaction (dsp_account) */
enum
{
	LST_DSPOPE_DATAS,   /*  0 so columns id start at 1 */
	LST_DSPOPE_STATUS,	/*  1 fake column */
	LST_DSPOPE_DATE,	/*  2 fake column */
	LST_DSPOPE_INFO,	/*  3 fake column */
	LST_DSPOPE_PAYEE,	/*  4 fake column */
	LST_DSPOPE_WORDING,	/*  5 fake column */
	LST_DSPOPE_AMOUNT,	/*  6 fake column */
	LST_DSPOPE_EXPENSE,	/*  7 fake column */
	LST_DSPOPE_INCOME,	/*  8 fake column */
	LST_DSPOPE_CATEGORY,/*  9 fake column */
	LST_DSPOPE_TAGS,	/* 10 fake column */
	LST_DSPOPE_BALANCE, /* 11 fake column */
	LST_DSPOPE_CLR,     /* 12 fake column */
	/* here we insert account column, only used for detail */
	LST_DSPOPE_ACCOUNT, /* 13 fake column : not stored */
	NUM_LST_DSPOPE
};

/* list_import_transaction */
#define LST_OPE_IMPTOGGLE 2



/* list define archive (defarchive) */
enum
{
	LST_DEFARC_DATAS,
	LST_DEFARC_OLDPOS,
	LST_DEFARC_AUTO,
	NUM_LST_DEFARC
};

/* csv format validator */
enum
{
	CSV_STRING,
	CSV_DATE,
	CSV_INT,
	CSV_DOUBLE
};

enum
{
	PRF_DATEFMT_MDY,
	PRF_DATEFMT_DMY,
	PRF_DATEFMT_YMD,
	NUM_PRF_DATEFMT
};


#endif