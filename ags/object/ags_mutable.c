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

#include <ags/object/ags_mutable.h>

void ags_mutable_base_init(AgsMutableInterface *interface);

/**
 * SECTION:ags_mutable
 * @short_description: unique access to recalls
 * @title: AgsMutable
 * @section_id: AgsMutable
 * @include: ags/object/ags_mutable.h
 *
 * The #AgsMutable interface gives you a unique access to mutable classes Its
 * purpose is to set muted.
 */

GType
ags_mutable_get_type()
{
  static GType ags_type_mutable = 0;

  if(!ags_type_mutable){
    static const GTypeInfo ags_mutable_info = {
      sizeof(AgsMutableInterface),
      (GBaseInitFunc) ags_mutable_base_init,
      NULL, /* base_finalize */
    };

    ags_type_mutable = g_type_register_static(G_TYPE_INTERFACE,
					       "AgsMutable\0", &ags_mutable_info,
					       0);
  }

  return(ags_type_mutable);
}

void
ags_mutable_base_init(AgsMutableInterface *interface)
{
  /* empty */
}

/**
 * ags_mutable_set_muted:
 * @mutable: an #AgsMutable
 * @muted: if %TRUE then muted, else playing
 *
 * Mute a class instance.
 *
 * Since: 0.4
 */
void
ags_mutable_set_muted(AgsMutable *mutable, gboolean muted)
{
  AgsMutableInterface *mutable_interface;

  g_return_if_fail(AGS_IS_MUTABLE(mutable));
  mutable_interface = AGS_MUTABLE_GET_INTERFACE(mutable);
  g_return_if_fail(mutable_interface->set_muted);
  mutable_interface->set_muted(mutable, muted);
}
