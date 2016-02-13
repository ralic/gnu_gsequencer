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

#include <ags/audio/ags_sound_provider.h>

#include <math.h>

void ags_sound_provider_class_init(AgsSoundProviderInterface *interface);

GType
ags_sound_provider_get_type()
{
  static GType ags_type_sound_provider = 0;

  if(!ags_type_sound_provider){
    ags_type_sound_provider = g_type_register_static_simple(G_TYPE_INTERFACE,
							    "AgsSoundProvider\0",
							    sizeof(AgsSoundProviderInterface),
							    (GClassInitFunc) ags_sound_provider_class_init,
							    0, NULL, 0);
  }

  return(ags_type_sound_provider);
}

void
ags_sound_provider_class_init(AgsSoundProviderInterface *interface)
{
  /* empty */
}

void
ags_sound_provider_set_soundcard(AgsSoundProvider *sound_provider,
				 GList *soundcard)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider));
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_if_fail(sound_provider_interface->set_soundcard);

  sound_provider_interface->set_soundcard(sound_provider,
					  soundcard);
}

GList*
ags_sound_provider_get_soundcard(AgsSoundProvider *sound_provider)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_val_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider), NULL);
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_val_if_fail(sound_provider_interface->get_soundcard, NULL);

  return(sound_provider_interface->get_soundcard(sound_provider));
}

void
ags_sound_provider_set_sequencer(AgsSoundProvider *sound_provider,
				 GList *sequencer)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider));
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_if_fail(sound_provider_interface->set_sequencer);

  sound_provider_interface->set_sequencer(sound_provider,
					  sequencer);
}

GList*
ags_sound_provider_get_sequencer(AgsSoundProvider *sound_provider)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_val_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider), NULL);
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_val_if_fail(sound_provider_interface->get_sequencer, NULL);

  return(sound_provider_interface->get_sequencer(sound_provider));
}

void
ags_sound_provider_set_distributed_manager(AgsSoundProvider *sound_provider,
					   GList *distributed_manager)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider));
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_if_fail(sound_provider_interface->set_distributed_manager);

  sound_provider_interface->set_distributed_manager(sound_provider,
						    distributed_manager);
}

GList*
ags_sound_provider_get_distributed_manager(AgsSoundProvider *sound_provider)
{
  AgsSoundProviderInterface *sound_provider_interface;

  g_return_val_if_fail(AGS_IS_SOUND_PROVIDER(sound_provider), NULL);
  sound_provider_interface = AGS_SOUND_PROVIDER_GET_INTERFACE(sound_provider);
  g_return_val_if_fail(sound_provider_interface->get_distributed_manager, NULL);

  return(sound_provider_interface->get_distributed_manager(sound_provider));
}