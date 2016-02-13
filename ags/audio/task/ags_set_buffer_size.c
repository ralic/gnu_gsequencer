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

#include <ags/audio/task/ags_set_buffer_size.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_audio_signal.h>

void ags_set_buffer_size_class_init(AgsSetBufferSizeClass *set_buffer_size);
void ags_set_buffer_size_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_set_buffer_size_init(AgsSetBufferSize *set_buffer_size);
void ags_set_buffer_size_connect(AgsConnectable *connectable);
void ags_set_buffer_size_disconnect(AgsConnectable *connectable);
void ags_set_buffer_size_finalize(GObject *gobject);

void ags_set_buffer_size_launch(AgsTask *task);

void ags_set_buffer_size_audio_signal(AgsSetBufferSize *set_buffer_size, AgsAudioSignal *audio_signal);
void ags_set_buffer_size_recycling(AgsSetBufferSize *set_buffer_size, AgsRecycling *recycling);
void ags_set_buffer_size_channel(AgsSetBufferSize *set_buffer_size, AgsChannel *channel);
void ags_set_buffer_size_audio(AgsSetBufferSize *set_buffer_size, AgsAudio *audio);
void ags_set_buffer_size_soundcard(AgsSetBufferSize *set_buffer_size, GObject *soundcard);

/**
 * SECTION:ags_set_buffer_size
 * @short_description: resizes buffer size
 * @title: AgsSetAudioChannels
 * @section_id:
 * @include: ags/audio/task/ags_set_buffer_size.h
 *
 * The #AgsSetAudioChannels task resizes buffer size of #AgsSoundcard.
 */

static gpointer ags_set_buffer_size_parent_class = NULL;
static AgsConnectableInterface *ags_set_buffer_size_parent_connectable_interface;

GType
ags_set_buffer_size_get_type()
{
  static GType ags_type_set_buffer_size = 0;

  if(!ags_type_set_buffer_size){
    static const GTypeInfo ags_set_buffer_size_info = {
      sizeof (AgsSetBufferSizeClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_set_buffer_size_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSetBufferSize),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_set_buffer_size_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_set_buffer_size_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_set_buffer_size = g_type_register_static(AGS_TYPE_TASK,
						      "AgsSetBufferSize\0",
						      &ags_set_buffer_size_info,
						      0);
    
    g_type_add_interface_static(ags_type_set_buffer_size,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_set_buffer_size);
}

void
ags_set_buffer_size_class_init(AgsSetBufferSizeClass *set_buffer_size)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_set_buffer_size_parent_class = g_type_class_peek_parent(set_buffer_size);

  /* gobject */
  gobject = (GObjectClass *) set_buffer_size;

  gobject->finalize = ags_set_buffer_size_finalize;

  /* task */
  task = (AgsTaskClass *) set_buffer_size;

  task->launch = ags_set_buffer_size_launch;
}

void
ags_set_buffer_size_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_set_buffer_size_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_set_buffer_size_connect;
  connectable->disconnect = ags_set_buffer_size_disconnect;
}

void
ags_set_buffer_size_init(AgsSetBufferSize *set_buffer_size)
{
  set_buffer_size->gobject = NULL;
  set_buffer_size->buffer_size = 128;
}

void
ags_set_buffer_size_connect(AgsConnectable *connectable)
{
  ags_set_buffer_size_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_set_buffer_size_disconnect(AgsConnectable *connectable)
{
  ags_set_buffer_size_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_set_buffer_size_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_set_buffer_size_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_set_buffer_size_launch(AgsTask *task)
{
  AgsSetBufferSize *set_buffer_size;
  GObject *gobject;

  set_buffer_size = AGS_SET_BUFFER_SIZE(task);

  gobject = set_buffer_size->gobject;

  if(AGS_IS_SOUNDCARD(gobject)){
    ags_set_buffer_size_soundcard(set_buffer_size, AGS_SOUNDCARD(gobject));
  }else if(AGS_IS_AUDIO(gobject)){
    ags_set_buffer_size_audio(set_buffer_size, AGS_AUDIO(gobject));
  }else if(AGS_IS_CHANNEL(gobject)){
    ags_set_buffer_size_channel(set_buffer_size, AGS_CHANNEL(gobject));
  }else if(AGS_IS_RECYCLING(gobject)){
    ags_set_buffer_size_recycling(set_buffer_size, AGS_RECYCLING(gobject));
  }else if(AGS_IS_AUDIO_SIGNAL(gobject)){
    ags_set_buffer_size_audio_signal(set_buffer_size, AGS_AUDIO_SIGNAL(gobject));
  }
}

void
ags_set_buffer_size_audio_signal(AgsSetBufferSize *set_buffer_size, AgsAudioSignal *audio_signal)
{
  ags_audio_signal_realloc_buffer_size(audio_signal,
				       set_buffer_size->buffer_size);
}

void
ags_set_buffer_size_recycling(AgsSetBufferSize *set_buffer_size, AgsRecycling *recycling)
{
  GList *list;
  
  list = recycling->audio_signal;

  while(list != NULL){
    ags_set_buffer_size_audio_signal(set_buffer_size, AGS_AUDIO_SIGNAL(list->data));

    list = list->next;
  }
}

void
ags_set_buffer_size_channel(AgsSetBufferSize *set_buffer_size, AgsChannel *channel)
{
  AgsRecycling *recycling;

  recycling = channel->first_recycling;
  
  while(recycling != NULL){
    ags_set_buffer_size_recycling(set_buffer_size, recycling);

    recycling = recycling->next;
  }
}

void
ags_set_buffer_size_audio(AgsSetBufferSize *set_buffer_size, AgsAudio *audio)
{
  AgsChannel *channel;

  /* AgsOutput */
  channel = audio->output;

  while(channel != NULL){
    ags_set_buffer_size_channel(set_buffer_size, channel);

    channel = channel->next;
  }

  /* AgsInput */
  channel = audio->input;

  while(channel != NULL){
    ags_set_buffer_size_channel(set_buffer_size, channel);

    channel = channel->next;
  }
}

void
ags_set_buffer_size_soundcard(AgsSetBufferSize *set_buffer_size, GObject *soundcard)
{
  GList *list;

  guint channels;
  guint samplerate;
  guint buffer_size;
  guint format;
  
  /*  */
  ags_soundcard_get_presets(soundcard,
			    &channels,
			    &samplerate,
			    &buffer_size,
			    &format);

  ags_soundcard_set_presets(soundcard,
			    channels,
			    samplerate,
			    set_buffer_size->buffer_size,
			    format);

  /* AgsAudio */
  list = ags_soundcard_get_audio(AGS_SOUNDCARD(soundcard));

  while(list != NULL){
    ags_set_buffer_size_audio(set_buffer_size, AGS_AUDIO(list->data));

    list = list->next;
  }
}

/**
 * ags_set_buffer_size_new:
 * @soundcard: the #AgsSoundcard reset
 * @buffer_size: the new count of buffer size
 *
 * Creates an #AgsSetAudioChannels.
 *
 * Returns: an new #AgsSetAudioChannels.
 *
 * Since: 0.4
 */
AgsSetBufferSize*
ags_set_buffer_size_new(GObject *gobject,
			guint buffer_size)
{
  AgsSetBufferSize *set_buffer_size;

  set_buffer_size = (AgsSetBufferSize *) g_object_new(AGS_TYPE_SET_BUFFER_SIZE,
						      NULL);

  set_buffer_size->gobject = gobject;
  set_buffer_size->buffer_size = buffer_size;

  return(set_buffer_size);
}