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

#include <ags/audio/recall/ags_mute_channel_run.h>
#include <ags/audio/recall/ags_mute_recycling.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recall_id.h>

#include <stdlib.h>
#include <stdio.h>

void ags_mute_channel_run_class_init(AgsMuteChannelRunClass *mute_channel_run);
void ags_mute_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_mute_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_mute_channel_run_init(AgsMuteChannelRun *mute_channel_run);
void ags_mute_channel_run_connect(AgsConnectable *connectable);
void ags_mute_channel_run_disconnect(AgsConnectable *connectable);
void ags_mute_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_mute_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_mute_channel_run_finalize(GObject *gobject);

AgsRecall* ags_mute_channel_run_duplicate(AgsRecall *recall,
					  AgsRecallID *recall_id,
					  guint *n_params, GParameter *parameter);

static gpointer ags_mute_channel_run_parent_class = NULL;
static AgsConnectableInterface *ags_mute_channel_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_mute_channel_run_parent_dynamic_connectable_interface;

GType
ags_mute_channel_run_get_type()
{
  static GType ags_type_mute_channel_run = 0;

  if(!ags_type_mute_channel_run){
    static const GTypeInfo ags_mute_channel_run_info = {
      sizeof (AgsMuteChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_mute_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMuteChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_mute_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_mute_channel_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_mute_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
						       "AgsMuteChannelRun\0",
						       &ags_mute_channel_run_info,
						       0);

    g_type_add_interface_static(ags_type_mute_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_mute_channel_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_mute_channel_run);
}

void
ags_mute_channel_run_class_init(AgsMuteChannelRunClass *mute_channel_run)
{
  GObjectClass *gobject;

  ags_mute_channel_run_parent_class = g_type_class_peek_parent(mute_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) mute_channel_run;

  gobject->finalize = ags_mute_channel_run_finalize;
}

void
ags_mute_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_mute_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_mute_channel_run_connect;
  connectable->disconnect = ags_mute_channel_run_disconnect;
}

void
ags_mute_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_mute_channel_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_mute_channel_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_mute_channel_run_disconnect_dynamic;
}

void
ags_mute_channel_run_init(AgsMuteChannelRun *mute_channel_run)
{
  AGS_RECALL(mute_channel_run)->name = "ags-mute\0";
  AGS_RECALL(mute_channel_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(mute_channel_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(mute_channel_run)->xml_type = "ags-mute-channel-run\0";
  AGS_RECALL(mute_channel_run)->port = NULL;

  AGS_RECALL(mute_channel_run)->flags |= (AGS_RECALL_OUTPUT_ORIENTATED |
					  AGS_RECALL_INPUT_ORIENTATED);
  AGS_RECALL(mute_channel_run)->child_type = AGS_TYPE_MUTE_RECYCLING;
}

void
ags_mute_channel_run_connect(AgsConnectable *connectable)
{
  ags_mute_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_mute_channel_run_disconnect(AgsConnectable *connectable)
{
  ags_mute_channel_run_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_mute_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_mute_channel_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_mute_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_mute_channel_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_mute_channel_run_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_mute_channel_run_parent_class)->finalize(gobject);
}

AgsRecall*
ags_mute_channel_run_duplicate(AgsRecall *recall,
			       AgsRecallID *recall_id,
			       guint *n_params, GParameter *parameter)
{
  AgsMuteChannelRun *mute;

  mute = (AgsMuteChannelRun *) AGS_RECALL_CLASS(ags_mute_channel_run_parent_class)->duplicate(recall,
											      recall_id,
											      n_params, parameter);
  
  return((AgsRecall *) mute);
}

AgsMuteChannelRun*
ags_mute_channel_run_new(AgsChannel *source)
{
  AgsMuteChannelRun *mute_channel_run;

  mute_channel_run = (AgsMuteChannelRun *) g_object_new(AGS_TYPE_MUTE_CHANNEL_RUN,
							"source\0", source,
							NULL);

  return(mute_channel_run);
}
