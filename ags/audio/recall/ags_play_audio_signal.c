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

#include <ags/audio/recall/ags_play_audio_signal.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <ags/audio/recall/ags_play_channel.h>
#include <ags/audio/recall/ags_play_channel_run_master.h>

#include <stdlib.h>

void ags_play_audio_signal_class_init(AgsPlayAudioSignalClass *play_audio_signal);
void ags_play_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_play_audio_signal_init(AgsPlayAudioSignal *play_audio_signal);
void ags_play_audio_signal_connect(AgsConnectable *connectable);
void ags_play_audio_signal_disconnect(AgsConnectable *connectable);
void ags_play_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_audio_signal_finalize(GObject *gobject);

void ags_play_audio_signal_run_init_pre(AgsRecall *recall);
void ags_play_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_play_audio_signal_duplicate(AgsRecall *recall,
					   AgsRecallID *recall_id,
					   guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_play_audio_signal
 * @short_description: plays audio signal
 * @title: AgsPlayAudioSignal
 * @section_id:
 * @include: ags/audio/recall/ags_play_audio_signal.h
 *
 * The #AgsPlayAudioSignal class plays the audio signal.
 */

static gpointer ags_play_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_play_audio_signal_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_play_audio_signal_parent_dynamic_connectable_interface;

GType
ags_play_audio_signal_get_type()
{
  static GType ags_type_play_audio_signal = 0;

  if(!ags_type_play_audio_signal){
    static const GTypeInfo ags_play_audio_signal_info = {
      sizeof (AgsPlayAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_audio_signal_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_play_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							"AgsPlayAudioSignal\0",
							&ags_play_audio_signal_info,
							0);

    g_type_add_interface_static(ags_type_play_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_audio_signal,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_play_audio_signal);
}

void
ags_play_audio_signal_class_init(AgsPlayAudioSignalClass *play_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_play_audio_signal_parent_class = g_type_class_peek_parent(play_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) play_audio_signal;

  gobject->finalize = ags_play_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) play_audio_signal;

  recall->run_init_pre = ags_play_audio_signal_run_init_pre;
  recall->run_inter = ags_play_audio_signal_run_inter;
  recall->duplicate = ags_play_audio_signal_duplicate;
}

void
ags_play_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_audio_signal_connect;
  connectable->disconnect = ags_play_audio_signal_disconnect;
}

void
ags_play_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_play_audio_signal_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_play_audio_signal_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_play_audio_signal_disconnect_dynamic;
}

void
ags_play_audio_signal_init(AgsPlayAudioSignal *play_audio_signal)
{
  AGS_RECALL(play_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_play_audio_signal_finalize(GObject *gobject)
{
  /* empty */
  
  /* call parent */
  G_OBJECT_CLASS(ags_play_audio_signal_parent_class)->finalize(gobject);
}

void
ags_play_audio_signal_connect(AgsConnectable *connectable)
{
  ags_play_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_play_audio_signal_disconnect(AgsConnectable *connectable)
{
  ags_play_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_play_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_play_audio_signal_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_play_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  ags_play_audio_signal_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_play_audio_signal_run_init_pre(AgsRecall *recall)
{  
  /* call parent */
  AGS_RECALL_CLASS(ags_play_audio_signal_parent_class)->run_init_pre(recall);

  /* empty */
}

void
ags_play_audio_signal_run_inter(AgsRecall *recall)
{
  GObject *soundcard;
  AgsRecycling *recycling;
  AgsAudioSignal *source;
  AgsRecallChannelRun *play_channel_run;
  AgsPlayChannel *play_channel;
  AgsPlayAudioSignal *play_audio_signal;
  
  AgsMutexManager *mutex_manager;

  GList *stream;

  void *buffer_source;
  void *buffer0, *buffer1;
  
  gboolean muted;
  guint audio_channel, pcm_channels;
  guint samplerate;
  guint buffer_size, soundcard_buffer_size;
  guint soundcard_format;
  guint copy_mode;
  gboolean resample;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;
  pthread_mutex_t *recycling_mutex;
  
  GValue muted_value = {0,};
  GValue audio_channel_value = {0,};

  play_audio_signal = AGS_PLAY_AUDIO_SIGNAL(recall);

  soundcard = recall->soundcard;
  source = AGS_AUDIO_SIGNAL(AGS_RECALL_AUDIO_SIGNAL(play_audio_signal)->source);
  stream = source->stream_current;

  if(soundcard == NULL){
    g_warning("no soundcard\0");
    return;
  }
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  pthread_mutex_lock(application_mutex);

  soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					  (GObject *) soundcard);
  
  pthread_mutex_unlock(application_mutex);

  pthread_mutex_lock(soundcard_mutex);

  ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
			    &pcm_channels,
			    &samplerate,
			    &soundcard_buffer_size,
			    &soundcard_format);
  
  if(stream == NULL){
    pthread_mutex_unlock(soundcard_mutex);

    ags_recall_done(recall);
  
    ags_recycling_remove_audio_signal(source->recycling,
				      source);
    g_object_unref(source);

    return;
  }

  buffer0 = ags_soundcard_get_buffer(AGS_SOUNDCARD(soundcard)); // ags_soundcard_get_next_buffer(AGS_SOUNDCARD(soundcard));
  buffer1 = NULL;
  
  if(buffer0 == NULL && 
     buffer1 == NULL){
    pthread_mutex_unlock(soundcard_mutex);
    
    g_warning("no output buffer\0");
    
    return;
  }

  if(recall->parent == NULL ||
     recall->parent->parent == NULL){
    play_channel_run = NULL;
    play_channel = NULL;
    audio_channel = AGS_RECALL_AUDIO_SIGNAL(play_audio_signal)->audio_channel;
  }else{
    play_channel_run = AGS_RECALL_CHANNEL_RUN(recall->parent->parent);
    play_channel = (AgsPlayChannel *) play_channel_run->recall_channel;

    g_value_init(&muted_value, G_TYPE_BOOLEAN);
    ags_port_safe_read(play_channel->muted,
		       &muted_value);

    muted = g_value_get_boolean(&muted_value);
    g_value_unset(&muted_value);

    if(muted){
      pthread_mutex_unlock(soundcard_mutex);
      return;
    }

    g_value_init(&audio_channel_value, G_TYPE_UINT64);
    ags_port_safe_read(play_channel->audio_channel,
		       &audio_channel_value);

    audio_channel = g_value_get_uint64(&audio_channel_value);
    g_value_unset(&audio_channel_value);
  }

  buffer_size = source->buffer_size;
  copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(soundcard_format),
						  ags_audio_buffer_util_format_from_soundcard(source->format));

  /* check if resample */
  buffer_source = stream->data;
  resample = FALSE;

  if(source->samplerate != samplerate){
    buffer_source = ags_audio_buffer_util_resample(buffer_source, 1,
						   ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
						   buffer_size,
						   samplerate);
      
    resample = TRUE;
  }
  
  if((AGS_RECALL_INITIAL_RUN & (AGS_RECALL_AUDIO_SIGNAL(recall)->flags)) != 0){
    ags_audio_buffer_util_copy_buffer_to_buffer(buffer0, pcm_channels, audio_channel + source->attack * pcm_channels,
						buffer_source, 1, 0,
						soundcard_buffer_size - source->attack, copy_mode);
  }else{
    if(source->attack != 0 && stream->prev != NULL){
      void *buffer_source_prev;
	
      buffer_source_prev = stream->prev->data;

      if(resample){
	buffer_source_prev = ags_audio_buffer_util_resample(buffer_source_prev, 1,
							    ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
							    source->length,
							    samplerate);

      }
	
      ags_audio_buffer_util_copy_buffer_to_buffer(buffer0, pcm_channels, audio_channel,
						  buffer_source_prev, 1, soundcard_buffer_size - source->attack,
						  source->attack, copy_mode);

      if(resample){
	free(buffer_source_prev);
      }
    }

    ags_audio_buffer_util_copy_buffer_to_buffer(buffer0, pcm_channels, audio_channel + source->attack * pcm_channels,
						buffer_source, 1, 0,
						soundcard_buffer_size - source->attack, copy_mode);
  }

  if(resample){
    free(buffer_source);
  }

  pthread_mutex_unlock(soundcard_mutex);
  
  /* call parent */
  AGS_RECALL_CLASS(ags_play_audio_signal_parent_class)->run_inter(recall);
}

AgsRecall*
ags_play_audio_signal_duplicate(AgsRecall *recall,
				AgsRecallID *recall_id,
				guint *n_params, GParameter *parameter)
{
  AgsPlayAudioSignal *copy;

  copy = (AgsPlayAudioSignal *) AGS_RECALL_CLASS(ags_play_audio_signal_parent_class)->duplicate(recall, recall_id,
												n_params, parameter);

  return((AgsRecall *) copy);
}

/**
 * ags_play_audio_signal_new:
 * @source: the source #AgsAudioSignal
 * @soundcard: the #GObject outputting to
 * @audio_channel: the audio channel
 *
 * Creates an #AgsPlayAudioSignal
 *
 * Returns: a new #AgsPlayAudioSignal
 *
 * Since: 0.4
 */
AgsPlayAudioSignal*
ags_play_audio_signal_new(AgsAudioSignal *source,
			  GObject *soundcard,
			  guint audio_channel)
{
  AgsPlayAudioSignal *play_audio_signal;

  play_audio_signal = (AgsPlayAudioSignal *) g_object_new(AGS_TYPE_PLAY_AUDIO_SIGNAL,
							  "source\0", source,
							  "soundcard\0", soundcard,
							  "audio_channel\0", audio_channel,
							  NULL);

  return(play_audio_signal);
}
