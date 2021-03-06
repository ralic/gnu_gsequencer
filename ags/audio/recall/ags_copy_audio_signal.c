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

#include <ags/audio/recall/ags_copy_audio_signal.h>
#include <ags/audio/recall/ags_copy_channel.h>
#include <ags/audio/recall/ags_copy_recycling.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <ags/audio/recall/ags_copy_channel.h>

#include <stdlib.h>

void ags_copy_audio_signal_class_init(AgsCopyAudioSignalClass *copy_audio_signal);
void ags_copy_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_copy_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_copy_audio_signal_init(AgsCopyAudioSignal *copy_audio_signal);
void ags_copy_audio_signal_connect(AgsConnectable *connectable);
void ags_copy_audio_signal_disconnect(AgsConnectable *connectable);
void ags_copy_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_copy_audio_signal_finalize(GObject *gobject);

void ags_copy_audio_signal_run_init_pre(AgsRecall *recall);
void ags_copy_audio_signal_run_pre(AgsRecall *recall);
void ags_copy_audio_signal_run_inter(AgsRecall *recall);
AgsRecall* ags_copy_audio_signal_duplicate(AgsRecall *recall,
					   AgsRecallID *recall_id,
					   guint *n_params, GParameter *parameter);

/**
 * SECTION:ags_copy_audio_signal
 * @short_description: copys audio signal
 * @title: AgsCopyAudioSignal
 * @section_id:
 * @include: ags/audio/recall/ags_copy_audio_signal.h
 *
 * The #AgsCopyAudioSignal class copys the audio signal.
 */

static gpointer ags_copy_audio_signal_parent_class = NULL;
static AgsConnectableInterface *ags_copy_audio_signal_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_copy_audio_signal_parent_dynamic_connectable_interface;

GType
ags_copy_audio_signal_get_type()
{
  static GType ags_type_copy_audio_signal = 0;

  if(!ags_type_copy_audio_signal){
    static const GTypeInfo ags_copy_audio_signal_info = {
      sizeof (AgsCopyAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_copy_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCopyAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_copy_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_copy_audio_signal_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_copy_audio_signal = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
							"AgsCopyAudioSignal\0",
							&ags_copy_audio_signal_info,
							0);

    g_type_add_interface_static(ags_type_copy_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_copy_audio_signal,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);
  }

  return(ags_type_copy_audio_signal);
}

void
ags_copy_audio_signal_class_init(AgsCopyAudioSignalClass *copy_audio_signal)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;

  ags_copy_audio_signal_parent_class = g_type_class_peek_parent(copy_audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) copy_audio_signal;

  gobject->finalize = ags_copy_audio_signal_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) copy_audio_signal;

  recall->run_init_pre = ags_copy_audio_signal_run_init_pre;
  recall->run_pre = ags_copy_audio_signal_run_pre;
  recall->run_inter = ags_copy_audio_signal_run_inter;  
  recall->duplicate = ags_copy_audio_signal_duplicate;
}

void
ags_copy_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_copy_audio_signal_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_copy_audio_signal_connect;
  connectable->disconnect = ags_copy_audio_signal_disconnect;
}

void
ags_copy_audio_signal_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_copy_audio_signal_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_copy_audio_signal_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_copy_audio_signal_disconnect_dynamic;
}

void
ags_copy_audio_signal_init(AgsCopyAudioSignal *copy_audio_signal)
{
  AGS_RECALL(copy_audio_signal)->name = "ags-copy\0";
  AGS_RECALL(copy_audio_signal)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(copy_audio_signal)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(copy_audio_signal)->xml_type = "ags-copy-audio-signal\0";
  AGS_RECALL(copy_audio_signal)->port = NULL;

  AGS_RECALL(copy_audio_signal)->child_type = G_TYPE_NONE;
}

void
ags_copy_audio_signal_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_copy_audio_signal_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_copy_audio_signal_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_copy_audio_signal_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_copy_audio_signal_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_copy_audio_signal_finalize(GObject *gobject)
{  
  /* call parent */
  G_OBJECT_CLASS(ags_copy_audio_signal_parent_class)->finalize(gobject);
}
void
ags_copy_audio_signal_run_init_pre(AgsRecall *recall)
{
  GObject *soundcard;
  AgsRecycling *recycling;
  AgsAudioSignal *destination;
  AgsRecallID *parent_recall_id;
  AgsRecyclingContext *recycling_context;
  
  AgsCopyRecycling *copy_recycling;
  AgsCopyAudioSignal *copy_audio_signal;

  AgsMutexManager *mutex_manager;
  
  GList *stream;

  gdouble delay;
  guint attack;
  guint length;
  
  pthread_mutex_t *application_mutex;
  pthread_mutex_t *recycling_mutex;
  
  copy_audio_signal = AGS_COPY_AUDIO_SIGNAL(recall);
  copy_recycling = AGS_COPY_RECYCLING(recall->parent);

  soundcard = recall->soundcard;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* recycling */
  recall->flags &= (~AGS_RECALL_PERSISTENT);
  recycling = AGS_RECALL_RECYCLING(copy_recycling)->destination;

  pthread_mutex_lock(application_mutex);

  recycling_mutex = ags_mutex_manager_lookup(mutex_manager,
					     (GObject *) recycling);
	
  pthread_mutex_unlock(application_mutex);

  /* recycling context */
  recycling_context = recall->recall_id->recycling_context;

  parent_recall_id = ags_recall_id_find_recycling_context(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->destination->recall_id,
							  recycling_context->parent);

  //TODO:JK: unclear
  attack = 0;
  delay = 0.0;

  /* create new audio signal */
  destination = ags_audio_signal_new((GObject *) soundcard,
				     (GObject *) recycling,
				     (GObject *) parent_recall_id);
  
  g_object_set(copy_audio_signal,
	       "destination\0", destination,
	       NULL);  
  ags_recycling_create_audio_signal_with_defaults(recycling,
						  destination,
						  delay, attack);
  length = 1; // (guint) (2.0 * soundcard->delay[soundcard->tic_counter]) + 1;
  ags_audio_signal_stream_resize(destination,
				 length);

  ags_connectable_connect(AGS_CONNECTABLE(destination));
  
  destination->stream_current = destination->stream_beginning;

  pthread_mutex_lock(recycling_mutex);

  ags_recycling_add_audio_signal(recycling,
				 destination);

  pthread_mutex_unlock(recycling_mutex);

#ifdef AGS_DEBUG
  g_message("copy %x to %x\0", destination, parent_recall_id);
  g_message("creating destination\0");
#endif
  
  /* call parent */
  AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->run_init_pre(recall);
}

void
ags_copy_audio_signal_run_pre(AgsRecall *recall)
{
  AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->run_pre(recall);

  if(AGS_RECALL_AUDIO_SIGNAL(recall)->source->stream_current != NULL){
    void *buffer;

    guint buffer_size;

    buffer = (signed short *) AGS_RECALL_AUDIO_SIGNAL(recall)->destination->stream_current->data;
    buffer_size = AGS_RECALL_AUDIO_SIGNAL(recall)->destination->buffer_size;

    ags_audio_buffer_util_clear_buffer(buffer, 1,
				       buffer_size, ags_audio_buffer_util_format_from_soundcard(AGS_RECALL_AUDIO_SIGNAL(recall)->destination->format));
  }
}

void
ags_copy_audio_signal_run_inter(AgsRecall *recall)
{
  AgsCopyChannel *copy_channel;
  AgsCopyAudioSignal *copy_audio_signal;
  AgsAudioSignal *source, *destination;

  GList *stream_source, *stream_destination;

  gchar *str;
  gboolean muted;
  guint buffer_size;
  guint copy_mode;
  
  GValue value = {0,};

  AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->run_inter(recall);

  copy_audio_signal = AGS_COPY_AUDIO_SIGNAL(recall);

  source = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->source;
  stream_source = source->stream_current;

  destination = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->destination;
  
  if(stream_source == NULL){
    if(destination != NULL){
      ags_recycling_remove_audio_signal(destination->recycling,
					destination);
      g_object_run_dispose(destination);
      g_object_unref(destination);
    }

    ags_recall_done(recall);

    ags_recycling_remove_audio_signal(source->recycling,
				      source);
    g_object_unref(source);
    
    return;
  }

  //FIXME:JK: attack probably needs to be removed

  if(destination == NULL){
    g_warning("no destination\0");
    return;
  }

  copy_channel = AGS_COPY_CHANNEL(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->recall_channel);

  g_value_init(&value, G_TYPE_BOOLEAN);
  ags_port_safe_read(copy_channel->muted,
		     &value);

  muted = g_value_get_boolean(&value);
  g_value_unset(&value);

  if(muted){
    return;
  }

  stream_destination = destination->stream_current;
  //  attack = AGS_RECALL_AUDIO_SIGNAL(copy_audio_signal)->attack;

  if(stream_destination->next == NULL){
    ags_audio_signal_add_stream(destination);
  }

  stream_destination = destination->stream_current;
  buffer_size = source->buffer_size;

  if(stream_destination != NULL){
    void *buffer_source;

    gboolean resample;
    
    copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(destination->format),
						    ags_audio_buffer_util_format_from_soundcard(source->format));
    
    if(stream_destination->next == NULL){
      ags_audio_signal_add_stream(destination);
    }

    resample = FALSE;
    
    if(stream_destination->next == NULL){
      ags_audio_signal_add_stream(destination);
    }

    /* check if resample */
    buffer_source = stream_source->data;

    if(source->samplerate != destination->samplerate){
      buffer_source = ags_audio_buffer_util_resample(buffer_source, 1,
						     ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
						     source->length,
						     destination->samplerate);
      
      resample = TRUE;
    }

    if((AGS_RECALL_INITIAL_RUN & (AGS_RECALL_AUDIO_SIGNAL(recall)->flags)) != 0){
      AGS_RECALL_AUDIO_SIGNAL(recall)->flags &= (~AGS_RECALL_INITIAL_RUN);
      ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, source->attack,
						  buffer_source, 1, 0,
						  buffer_size - source->attack, copy_mode);
    }else{
      if(source->attack != 0 && stream_source->prev != NULL){
	void *buffer_source_prev;
	
	buffer_source_prev = stream_source->prev->data;

	if(resample){
	  buffer_source_prev = ags_audio_buffer_util_resample(buffer_source_prev, 1,
							      ags_audio_buffer_util_format_from_soundcard(source->format), source->samplerate,
							      source->length,
							      destination->samplerate);

	}

	ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, 0,
						    buffer_source_prev, 1, buffer_size - source->attack,
						    source->attack, copy_mode);

	if(resample){
	  free(buffer_source_prev);
	}
      }

      ags_audio_buffer_util_copy_buffer_to_buffer(stream_destination->data, 1, source->attack,
						  buffer_source, 1, 0,
						  buffer_size - source->attack, copy_mode);
    }

    if(resample){
      free(buffer_source);
    }
  }
}

AgsRecall*
ags_copy_audio_signal_duplicate(AgsRecall *recall,
				AgsRecallID *recall_id,
				guint *n_params, GParameter *parameter)
{
  AgsCopyAudioSignal *copy;

  copy = (AgsCopyAudioSignal *) AGS_RECALL_CLASS(ags_copy_audio_signal_parent_class)->duplicate(recall,
												recall_id,
												n_params, parameter);

  return((AgsRecall *) copy);
}

/**
 * ags_copy_audio_signal_new:
 * @destination: the destination #AgsAudioSignal
 * @source: the source #AgsAudioSignal
 * @soundcard: the #GObject defaulting to
 *
 * Creates an #AgsCopyAudioSignal
 *
 * Returns: a new #AgsCopyAudioSignal
 *
 * Since: 0.4
 */
AgsCopyAudioSignal*
ags_copy_audio_signal_new(AgsAudioSignal *destination,
			  AgsAudioSignal *source,
			  GObject *soundcard)
{
  AgsCopyAudioSignal *copy_audio_signal;

  copy_audio_signal = (AgsCopyAudioSignal *) g_object_new(AGS_TYPE_COPY_AUDIO_SIGNAL,
							  "destination\0", destination,
							  "source\0", source,
							  "soundcard\0", soundcard,
							  NULL);

  return(copy_audio_signal);
}
