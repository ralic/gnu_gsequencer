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

#include <ags/object/ags_distributed_manager.h>

void ags_distributed_manager_class_init(AgsDistributedManagerInterface *interface);

/**
 * SECTION:ags_distributed_manager
 * @short_description: access distributed ressources
 * @title: AgsDistributedManager
 * @section_id:
 * @include: ags/object/ags_distributed_manager.h
 *
 * The #AgsDistributed_Manager interface gives you a unique access distributed resources.
 */

GType
ags_distributed_manager_get_type()
{
  static GType ags_type_distributed_manager = 0;

  if(!ags_type_distributed_manager){
    ags_type_distributed_manager = g_type_register_static_simple(G_TYPE_INTERFACE,
								 "AgsDistributedManager\0",
								 sizeof(AgsDistributedManagerInterface),
								 (GClassInitFunc) ags_distributed_manager_class_init,
								 0, NULL, 0);
  }

  return(ags_type_distributed_manager);
}

void
ags_distributed_manager_class_init(AgsDistributedManagerInterface *interface)
{
  /* empty */
}

/**
 * ags_distributed_manager_set_url:
 * @distributed_manager: The #AgsDistributedManager
 * @url: the url to set
 *
 * Sets the url of @distributed_manager.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_set_url(AgsDistributedManager *distributed_manager,
				gchar *url)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->set_url);
  
  distributed_manager_interface->set_url(distributed_manager,
					 url);
}

/**
 * ags_distributed_manager_get_url:
 * @distributed_manager: the #AgsDistributedManager
 *
 * Gets the URL of @distributed_manager.
 *
 * Returns: the URL as string
 *
 * Since: 0.7.1
 */
gchar*
ags_distributed_manager_get_url(AgsDistributedManager *distributed_manager)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->get_url, NULL);

  return(distributed_manager_interface->get_url(distributed_manager));
}

/**
 * ags_distributed_manager_set_port:
 * @distributed_manager: The #AgsDistributedManager
 * @port: the ports to set
 * @port_count: the number of ports
 *
 * Sets the ports of @distributed_manager.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_set_ports(AgsDistributedManager *distributed_manager,
				  guint *port, guint port_count)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->set_ports);
  
  distributed_manager_interface->set_ports(distributed_manager,
					   port, port_count);
}

/**
 * ags_distributed_manager_get_ports:
 * @distributed_manager: the #AgsDistributedManager
 * @port_count: the number of ports returned
 *
 * Gets the ports of @distributed_manager.
 *
 * Returns: the port as string %NULL-terminated array
 *
 * Since: 0.7.1
 */
guint*
ags_distributed_manager_get_ports(AgsDistributedManager *distributed_manager,
				  guint *port_count)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->get_ports, NULL);

  return(distributed_manager_interface->get_ports(distributed_manager,
						  port_count));
}

/**
 * ags_distributed_manager_set_soundcard:
 * @distributed_manager: The #AgsDistributedManager
 * @client_uuid: the location to fetch from
 * @soundcard: the soundcard to set
 *
 * Sets the soundcard at @client_uuid.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_set_soundcard(AgsDistributedManager *distributed_manager,
				      gchar *client_uuid,
				      GList *soundcard)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->set_soundcard);
  
  distributed_manager_interface->set_soundcard(distributed_manager,
					       client_uuid,
					       soundcard);
}

/**
 * ags_distributed_manager_get_soundcard:
 * @client_uuid: the client uuid
 * @distributed_manager: the #AgsDistributedManager
 *
 * Gets the soundcard of @distributed_manager associated with @client_uuid.
 *
 * Returns: the soundcard as #GList-struct
 *
 * Since: 0.7.1
 */
GList*
ags_distributed_manager_get_soundcard(AgsDistributedManager *distributed_manager,
				      gchar *client_uuid)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->get_soundcard, NULL);

  return(distributed_manager_interface->get_soundcard(distributed_manager,
						      client_uuid));
}

/**
 * ags_distributed_manager_set_sequencer:
 * @distributed_manager: The #AgsDistributedManager
 * @client_uuid: the location to fetch from
 * @sequencer: the sequencer to set
 *
 * Sets the sequencer at @client_uuid.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_set_sequencer(AgsDistributedManager *distributed_manager,
				      gchar *client_uuid,
				      GList *sequencer)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->set_sequencer);
  
  distributed_manager_interface->set_sequencer(distributed_manager,
					       client_uuid,
					       sequencer);
}

/**
 * ags_distributed_manager_get_sequencer:
 * @client_uuid: the client uuid
 * @distributed_manager: the #AgsDistributedManager
 *
 * Gets the sequencer of @distributed_manager associated with @client_uuid.
 *
 * Returns: the sequencer as #GList-struct
 *
 * Since: 0.7.1
 */
GList*
ags_distributed_manager_get_sequencer(AgsDistributedManager *distributed_manager,
				      gchar *client_uuid)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->get_sequencer, NULL);

  return(distributed_manager_interface->get_sequencer(distributed_manager,
						      client_uuid));
}

/**
 * ags_distributed_manager_register_soundcard:
 * @distributed_manager: The #AgsDistributedManager
 * @is_output: if %TRUE the used as sink, else as source
 *
 * Fetches @soundcard of @distributed_manager.
 *
 * Returns: a new #AgsSoundcard
 *
 * Since: 0.7.1
 */
GObject*
ags_distributed_manager_register_soundcard(AgsDistributedManager *distributed_manager,
					   gboolean is_output)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->register_soundcard, NULL);

  return(distributed_manager_interface->register_soundcard(distributed_manager,
							   is_output));
}

/**
 * ags_distributed_manager_unregister_soundcard:
 * @distributed_manager: The #AgsDistributedManager
 * @soundcard: the #AgsSoundcard
 *
 * Releases @soundcard in @distributed_manager.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_unregister_soundcard(AgsDistributedManager *distributed_manager,
					     GObject *soundcard)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->unregister_soundcard);
  
  distributed_manager_interface->unregister_soundcard(distributed_manager,
						      soundcard);
}

/**
 * ags_distributed_manager_register_sequencer:
 * @distributed_manager: The #AgsDistributedManager
 * @is_output: if %TRUE the used as sink, else as source
 *
 * Fetches @sequencer of @distributed_manager.
 *
 * Returns: a new #AgsSequencer
 *
 * Since: 0.7.1
 */
GObject*
ags_distributed_manager_register_sequencer(AgsDistributedManager *distributed_manager,
					   gboolean is_output)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_val_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager), NULL);
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_val_if_fail(distributed_manager_interface->register_sequencer, NULL);

  return(distributed_manager_interface->register_sequencer(distributed_manager,
							   is_output));
}

/**
 * ags_distributed_manager_unregister_sequencer:
 * @distributed_manager: The #AgsDistributedManager
 * @sequencer: the #AgsSequencer
 *
 * Releases @sequencer in @distributed_manager.
 *
 * Since: 0.7.1
 */
void
ags_distributed_manager_unregister_sequencer(AgsDistributedManager *distributed_manager,
					     GObject *sequencer)
{
  AgsDistributedManagerInterface *distributed_manager_interface;

  g_return_if_fail(AGS_IS_DISTRIBUTED_MANAGER(distributed_manager));
  distributed_manager_interface = AGS_DISTRIBUTED_MANAGER_GET_INTERFACE(distributed_manager);
  g_return_if_fail(distributed_manager_interface->unregister_sequencer);
  
  distributed_manager_interface->unregister_sequencer(distributed_manager,
						      sequencer);
}
