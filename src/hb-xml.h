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

#ifndef __HB_XML_H__
#define __HB_XML_H__


enum
{
	XML_UNSET,
	XML_OK,
	XML_IO_ERROR,
	XML_FILE_ERROR,
	XML_VERSION_ERROR,
};


typedef struct _ParseContext ParseContext;
struct _ParseContext
{
	gdouble	file_version;	//version of the xml structure
	gint	data_version;   //last hb version file was saved with
};


gint homebank_load_xml(gchar *filename);
gint homebank_save_xml(gchar *filename);


#endif
