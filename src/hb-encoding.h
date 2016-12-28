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

#ifndef __HB_ENCODING_H__
#define __HB_ENCODING_H__


struct _GeditEncoding
{
	gint   index;
	const gchar *charset;
	const gchar *name;
};

typedef struct _GeditEncoding GeditEncoding;

typedef enum
{

  GEDIT_ENCODING_ISO_8859_1,
  GEDIT_ENCODING_ISO_8859_2,
  GEDIT_ENCODING_ISO_8859_3,
  GEDIT_ENCODING_ISO_8859_4,
  GEDIT_ENCODING_ISO_8859_5,
  GEDIT_ENCODING_ISO_8859_6,
  GEDIT_ENCODING_ISO_8859_7,
  GEDIT_ENCODING_ISO_8859_8,
  GEDIT_ENCODING_ISO_8859_8_I,
  GEDIT_ENCODING_ISO_8859_9,
  GEDIT_ENCODING_ISO_8859_10,
  GEDIT_ENCODING_ISO_8859_13,
  GEDIT_ENCODING_ISO_8859_14,
  GEDIT_ENCODING_ISO_8859_15,
  GEDIT_ENCODING_ISO_8859_16,

  GEDIT_ENCODING_UTF_7,
  GEDIT_ENCODING_UTF_16,
  GEDIT_ENCODING_UTF_16_BE,
  GEDIT_ENCODING_UTF_16_LE,
  GEDIT_ENCODING_UTF_32,
  GEDIT_ENCODING_UCS_2,
  GEDIT_ENCODING_UCS_4,

  GEDIT_ENCODING_ARMSCII_8,
  GEDIT_ENCODING_BIG5,
  GEDIT_ENCODING_BIG5_HKSCS,
  GEDIT_ENCODING_CP_866,

  GEDIT_ENCODING_EUC_JP,
  GEDIT_ENCODING_EUC_JP_MS,
  GEDIT_ENCODING_CP932,
  GEDIT_ENCODING_EUC_KR,
  GEDIT_ENCODING_EUC_TW,

  GEDIT_ENCODING_GB18030,
  GEDIT_ENCODING_GB2312,
  GEDIT_ENCODING_GBK,
  GEDIT_ENCODING_GEOSTD8,
  GEDIT_ENCODING_HZ,

  GEDIT_ENCODING_IBM_850,
  GEDIT_ENCODING_IBM_852,
  GEDIT_ENCODING_IBM_855,
  GEDIT_ENCODING_IBM_857,
  GEDIT_ENCODING_IBM_862,
  GEDIT_ENCODING_IBM_864,

  GEDIT_ENCODING_ISO_2022_JP,
  GEDIT_ENCODING_ISO_2022_KR,
  GEDIT_ENCODING_ISO_IR_111,
  GEDIT_ENCODING_JOHAB,
  GEDIT_ENCODING_KOI8_R,
  GEDIT_ENCODING_KOI8__R,
  GEDIT_ENCODING_KOI8_U,

  GEDIT_ENCODING_SHIFT_JIS,
  GEDIT_ENCODING_TCVN,
  GEDIT_ENCODING_TIS_620,
  GEDIT_ENCODING_UHC,
  GEDIT_ENCODING_VISCII,

  GEDIT_ENCODING_WINDOWS_1250,
  GEDIT_ENCODING_WINDOWS_1251,
  GEDIT_ENCODING_WINDOWS_1252,
  GEDIT_ENCODING_WINDOWS_1253,
  GEDIT_ENCODING_WINDOWS_1254,
  GEDIT_ENCODING_WINDOWS_1255,
  GEDIT_ENCODING_WINDOWS_1256,
  GEDIT_ENCODING_WINDOWS_1257,
  GEDIT_ENCODING_WINDOWS_1258,

  GEDIT_ENCODING_LAST,

  GEDIT_ENCODING_UTF_8,
  GEDIT_ENCODING_UNKNOWN

} GeditEncodingIndex;

const GeditEncoding *gedit_encoding_get_from_index (gint index);

const GeditEncoding *gedit_encoding_get_utf8 (void);

const gchar *homebank_file_getencoding(gchar *filename);
gchar *homebank_utf8_ensure(gchar *buffer);

#endif

