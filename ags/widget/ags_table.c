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

#include <ags/widget/ags_table.h>

void ags_table_class_init(AgsTableClass *table);
void ags_table_init(AgsTable *table);

GType
ags_table_get_type(void)
{
  static GType ags_type_table = 0;

  if(!ags_type_table){
    static const GTypeInfo ags_table_info = {
      sizeof(AgsTableClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_table_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(AgsTable),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_table_init,
    };

    ags_type_table = g_type_register_static(GTK_TYPE_TABLE,
					    "AgsTable\0", &ags_table_info,
					    0);
  }

  return(ags_type_table);
}

void
ags_table_class_init(AgsTableClass *table)
{
}

void
ags_table_init(AgsTable *table)
{
}

AgsTable*
ags_table_new(guint row, guint columns, gboolean homogeneous)
{
  AgsTable *table;

  table = (AgsTable *) g_object_new(AGS_TYPE_TABLE,
				    "n-rows\0", row,
				    "n-columns\0", columns,
				    "homogeneous\0", homogeneous,
				    NULL);
  
  return(table);
}
