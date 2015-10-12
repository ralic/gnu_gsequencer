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

#include <ags/audio/recall/ags_envelope_audio_signal.h>
#include <ags/audio/recall/ags_envelope_channel.h>

#include <ags/object/ags_connectable.h>

#include <ags/main.h>

#include <ags/object/ags_dynamic_connectable.h>

#include <ags/audio/ags_devout.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>

#include <stdlib.h>

void ags_envelope_audio_signal_class_init(AgsEnvelopeAudioSignalClass *envelope_audio_signal);
void ags_envelope_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_envelope_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_envelope_audio_signal_init(AgsEnvelopeAudioSignal *envelope_audio_signal);
void ags_envelope_audio_signal_connect(AgsConnectable *connectable);
void ags_envelope_audio_signal_disconnect(AgsConnectable *connectable);
void ags_envelope_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_envelope_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_envelope_audio_signal_finalize(GObject *gobject);

void ags_envelope_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_envelope_audio_signal_duplicate(AgsRecall *recall,
					       AgsRecallID *recall_id,
					       guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_envelope_audio_signal
 * @short_description: envelopes audio signal
 * @title: AgsEnvelopeAudioSignal
 * @section_id:
 * @include: ags/audio/recall/ags_envelope_audio_signal.h
 *
 * The #AgsEnvelopeAudioSignal class envelopes the audio signal.
 */

static gpointer ags_envelope_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_envelope_audio_signal_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_envelope_audio_signal_parent_dynamic_connectable_interface;

GType
ags_envelope_audio_signal_get_type()
{
  static GType ags_type_envelope_audio_signal = 0;

  if(!ags_type_envelope_audio_signal){
    static const GTypeInfo ags_envelope_audio_signal_info = {
      sizeof (AgsEnvelopeAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_envelope_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsEnvelopeAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_envelope_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_envelope_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_envelope_audio_signal_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_envelope_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							    "AgsEnvelopeAudioSignal\0",
							    &ags_envelope_audio_signal_info,
							    0);

    g_type_add_interface_static(ags_type_envelope_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_envelope_audio_signal,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_envelope_audio_signal);
}

void
ags_envelope_audio_signal_class_init(AgsEnvelopeAudioSignalClass *envelope_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_envelope_audio_signal_parent_class = g_type_class_peek_parent(envelope_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) envelope_audio_signal;

  gobject->finalize = ags_envelope_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) envelope_audio_signal;

  recall->run_inter = ags_envelope_audio_signal_run_inter;  
  recall->duplicate = ags_envelope_audio_signal_duplicate;
}

void
ags_envelope_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_envelope_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_envelope_audio_signal_connect;
  connectable->disconnect = ags_envelope_audio_signal_disconnect;
}

void
ags_envelope_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_envelope_audio_signal_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_envelope_audio_signal_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_envelope_audio_signal_disconnect_dynamic;
}

void
ags_envelope_audio_signal_init(AgsEnvelopeAudioSignal *envelope_audio_signal)
{
  AGS_RECALL(envelope_audio_signal)->name = "ags-envelope\0";
  AGS_RECALL(envelope_audio_signal)->version = AGS_EFFECTS_DEFAULT_VERSION;
  AGS_RECALL(envelope_audio_signal)->build_id = AGS_BUILD_ID;
  AGS_RECALL(envelope_audio_signal)->xml_type = "ags-envelope-audio-signal\0";
  AGS_RECALL(envelope_audio_signal)->port = NULL;

  AGS_RECALL(envelope_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_envelope_audio_signal_connect(AgsConnectable *connectable)
{
  if((AGS_RECALL_CONNECTED & (AGS_RECALL(connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_envelope_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_envelope_audio_signal_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_envelope_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_envelope_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  if((AGS_RECALL_DYNAMIC_CONNECTED & (AGS_RECALL(dynamic_connectable)->flags)) != 0){
    return;
  }

  /* call parent */
  ags_envelope_audio_signal_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_envelope_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_envelope_audio_signal_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_envelope_audio_signal_finalize(GObject *gobject)
{
  /* empty */

  /* call parent */
  G_OBJECT_CLASS(ags_envelope_audio_signal_parent_class)->finalize(gobject);
}

void
ags_envelope_audio_signal_run_inter(AgsRecall *recall)
{
  AgsEnvelopeChannel *envelope_channel;
  AgsEnvelopeAudioSignal *envelope_audio_signal;
  AgsAudioSignal *source;
  GList *stream_source;
  gboolean audio_enveloped, channel_enveloped;
  guint buffer_size;
  guint i;
  GValue audio_value = {0,};
  GValue channel_value = {0,};

  AGS_RECALL_CLASS(ags_envelope_audio_signal_parent_class)->run_inter(recall);

  envelope_audio_signal = AGS_ENVELOPE_AUDIO_SIGNAL(recall);

  source = AGS_RECALL_AUDIO_SIGNAL(envelope_audio_signal)->source;
  stream_source = source->stream_current;
  buffer_size = source->buffer_size;

  if(stream_source == NULL){
    ags_recall_done(recall);
    return;
  }

  /* check channel */
  envelope_channel = AGS_ENVELOPE_CHANNEL(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->recall_channel);

  //FIXME:JK: access ports safe
  ags_audio_signal_envelope(source,
			    envelope_channel->attack->port_value.ags_port_double,
			    envelope_channel->decay->port_value.ags_port_double,
			    envelope_channel->sustain->port_value.ags_port_double,
			    envelope_channel->release->port_value.ags_port_double,
			    envelope_channel->ratio->port_value.ags_port_double);
}

AgsRecall*
ags_envelope_audio_signal_duplicate(AgsRecall *recall,
				    AgsRecallID *recall_id,
				    guint *n_params, GParameter *parameter)
{
  AgsEnvelopeAudioSignal *envelope;

  envelope = (AgsEnvelopeAudioSignal *) AGS_RECALL_CLASS(ags_envelope_audio_signal_parent_class)->duplicate(recall,
													    recall_id,
													    n_params, parameter);

  return((AgsRecall *) envelope);
}

/**
 * ags_envelope_audio_signal_new:
 * @source: the source #AgsAudioSignal
 *
 * Creates an #AgsEnvelopeAudioSignal
 *
 * Returns: a new #AgsEnvelopeAudioSignal
 *
 * Since: 0.6
 */
AgsEnvelopeAudioSignal*
ags_envelope_audio_signal_new(AgsAudioSignal *source)
{
  AgsEnvelopeAudioSignal *envelope_audio_signal;

  envelope_audio_signal = (AgsEnvelopeAudioSignal *) g_object_new(AGS_TYPE_ENVELOPE_AUDIO_SIGNAL,
								  "source\0", source,
								  NULL);

  return(envelope_audio_signal);
}
