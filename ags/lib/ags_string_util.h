/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_STRING_UTIL__
#define __AGS_STRING_UTIL__

#include <glib.h>
#include <glib-object.h>

gchar* ags_string_util_escape_single_quote(gchar *str);

guint ags_strv_length(gchar **str_array);
gboolean ags_strv_contains(gchar **str_array,
			   gchar *str);

#endif /*__AGS_STRING_UTIL__*/
