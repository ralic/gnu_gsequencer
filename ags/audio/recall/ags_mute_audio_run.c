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

#include <ags/audio/recall/ags_mute_audio_run.h>
#include <ags/audio/recall/ags_mute_audio.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>

#include <ags/audio/ags_recall_container.h>

void ags_mute_audio_run_class_init(AgsMuteAudioRunClass *mute_audio_run);
void ags_mute_audio_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_mute_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_mute_audio_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_mute_audio_run_init(AgsMuteAudioRun *mute_audio_run);
void ags_mute_audio_run_finalize(GObject *gobject);
void ags_mute_audio_run_connect(AgsConnectable *connectable);
void ags_mute_audio_run_disconnect(AgsConnectable *connectable);
void ags_mute_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_mute_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);

/**
 * SECTION:ags_mute_audio_run
 * @short_description: copy pattern
 * @title: AgsMuteAudioRun
 * @section_id:
 * @include: ags/audio/recall/ags_mute_audio_run.h
 *
 * The #AgsMuteAudioRun class copy pattern.
 */

static gpointer ags_mute_audio_run_parent_class = NULL;
static AgsConnectableInterface* ags_mute_audio_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_mute_audio_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_mute_audio_run_parent_plugin_interface;

GType
ags_mute_audio_run_get_type()
{
  static GType ags_type_mute_audio_run = 0;

  if(!ags_type_mute_audio_run){
    static const GTypeInfo ags_mute_audio_run_info = {
      sizeof (AgsMuteAudioRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_mute_audio_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMuteAudioRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_mute_audio_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_audio_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_audio_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_mute_audio_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };    

    ags_type_mute_audio_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_RUN,
						     "AgsMuteAudioRun\0",
						     &ags_mute_audio_run_info,
						     0);
    
    g_type_add_interface_static(ags_type_mute_audio_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_mute_audio_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_mute_audio_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_mute_audio_run);
}

void
ags_mute_audio_run_class_init(AgsMuteAudioRunClass *mute_audio_run)
{
  GObjectClass *gobject;

  ags_mute_audio_run_parent_class = g_type_class_peek_parent(mute_audio_run);

  /* GObjectClass */
  gobject = (GObjectClass *) mute_audio_run;

  gobject->finalize = ags_mute_audio_run_finalize;
}

void
ags_mute_audio_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_mute_audio_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_mute_audio_run_connect;
  connectable->disconnect = ags_mute_audio_run_disconnect;
}

void
ags_mute_audio_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_mute_audio_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_mute_audio_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_mute_audio_run_disconnect_dynamic;
}

void
ags_mute_audio_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_mute_audio_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
}

void
ags_mute_audio_run_init(AgsMuteAudioRun *mute_audio_run)
{
  AGS_RECALL(mute_audio_run)->name = "ags-mute\0";
  AGS_RECALL(mute_audio_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(mute_audio_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(mute_audio_run)->xml_type = "ags-mute-audio-run\0";
  AGS_RECALL(mute_audio_run)->port = NULL;
}

void
ags_mute_audio_run_finalize(GObject *gobject)
{
  /* call parent */
  G_OBJECT_CLASS(ags_mute_audio_run_parent_class)->finalize(gobject);
}

void
ags_mute_audio_run_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_mute_audio_run_parent_connectable_interface->connect(connectable);
}

void
ags_mute_audio_run_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_mute_audio_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_mute_audio_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_mute_audio_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);
}

void
ags_mute_audio_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_mute_audio_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);
}

AgsRecall*
ags_mute_audio_run_duplicate(AgsRecall *recall,
			     AgsRecallID *recall_id,
			     guint *n_params, GParameter *parameter)
{
  AgsMuteAudioRun *copy;

  copy = AGS_MUTE_AUDIO_RUN(AGS_RECALL_CLASS(ags_mute_audio_run_parent_class)->duplicate(recall,
											 recall_id,
											 n_params, parameter));

  return((AgsRecall *) copy);
}

/**
 * ags_mute_audio_run_new:
 *
 * Creates an #AgsMuteAudioRun
 *
 * Returns: a new #AgsMuteAudioRun
 *
 * Since: 0.4
 */
AgsMuteAudioRun*
ags_mute_audio_run_new()
{
  AgsMuteAudioRun *mute_audio_run;

  mute_audio_run = (AgsMuteAudioRun *) g_object_new(AGS_TYPE_MUTE_AUDIO_RUN,
						    NULL);

  return(mute_audio_run);
}
