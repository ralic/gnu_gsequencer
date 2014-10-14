/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2013 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/object/ags_portlet.h>

#include <stdio.h>

void ags_portlet_base_init(AgsPortletInterface *interface);

GType
ags_portlet_get_type()
{
  static GType ags_type_portlet = 0;

  if(!ags_type_portlet){
    static const GTypeInfo ags_portlet_info = {
      sizeof(AgsPortletInterface),
      (GBaseInitFunc) ags_portlet_base_init,
      NULL, /* base_finalize */
    };

    ags_type_portlet = g_type_register_static(G_TYPE_INTERFACE,
					      "AgsPortlet\0", &ags_portlet_info,
					      0);
  }

  return(ags_type_portlet);
}

void
ags_portlet_base_init(AgsPortletInterface *interface)
{
  /* empty */
}

/**
 * ags_portlet_set_port:
 * @portlet: an #AgsPortlet
 * @port: the #AgsPort
 *
 * Set port.
 */
void
ags_portlet_set_port(AgsPortlet *portlet, AgsPort *port)
{
  AgsPortletInterface *portlet_interface;

  g_return_if_fail(AGS_IS_PORTLET(portlet));
  portlet_interface = AGS_PORTLET_GET_INTERFACE(portlet);
  g_return_if_fail(portlet_interface->set_port);
  portlet_interface->set_port(portlet, port);
}

/**
 * ags_portlet_get_port:
 * @portlet: an #AgsPortlet
 *
 * Get port.
 *
 * Returns: the #AgsPort
 */
AgsPort*
ags_portlet_get_port(AgsPortlet *portlet)
{
  AgsPortletInterface *portlet_interface;

  g_return_val_if_fail(AGS_IS_PORTLET(portlet), NULL);
  portlet_interface = AGS_PORTLET_GET_INTERFACE(portlet);
  g_return_val_if_fail(portlet_interface->get_port, NULL);
  portlet_interface->get_port(portlet);
}

/**
 * ags_portlet_get_safe_property:
 * @portlet: an #AgsPortlet
 *
 * Retrieve thread-safe properties.
 *
 * Returns: a #GList containing properties
 */
GList*
ags_portlet_list_safe_properties(AgsPortlet *portlet)
{
  AgsPortletInterface *portlet_interface;

  g_return_val_if_fail(AGS_IS_PORTLET(portlet), NULL);
  portlet_interface = AGS_PORTLET_GET_INTERFACE(portlet);
  g_return_val_if_fail(portlet_interface->list_safe_properties, NULL);
  portlet_interface->list_safe_properties(portlet);
}

/**
 * ags_portlet_get_safe_property:
 * @portlet: an #AgsPortlet
 * @property_name: propertie's name
 * @value: the #GValue
 *
 * Get property thread safe.
 *
 * Returns: the #AgsPort
 */
void
ags_portlet_get_safe_property(AgsPortlet *portlet, gchar *property_name, GValue *value)
{
  AgsPortletInterface *portlet_interface;

  g_return_if_fail(AGS_IS_PORTLET(portlet));
  portlet_interface = AGS_PORTLET_GET_INTERFACE(portlet);
  g_return_if_fail(portlet_interface->safe_get_property);
  portlet_interface->safe_get_property(portlet, property_name, value);
}

/**
 * ags_portlet_set_safe_property:
 * @portlet: an #AgsPortlet
 * @property_name: propertie's name
 * @value: the #GValue
 *
 * Set property thread safe.
 */
void
ags_portlet_set_safe_property(AgsPortlet *portlet, gchar *property_name, GValue *value)
{
  AgsPortletInterface *portlet_interface;

  g_return_if_fail(AGS_IS_PORTLET(portlet));
  portlet_interface = AGS_PORTLET_GET_INTERFACE(portlet);
  g_return_if_fail(portlet_interface->safe_set_property);
  portlet_interface->safe_set_property(portlet, property_name, value);
}
