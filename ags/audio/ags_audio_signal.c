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

#include <ags/audio/ags_audio_signal.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <stdint.h>
//TODO:JK: do vector optimization
//#include <emmintrin.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * SECTION:ags_audio_signal
 * @short_description: Contains the audio data and its alignment
 * @title: AgsAudioSignal
 * @section_id:
 * @include: ags/audio/ags_audio_signal.h
 *
 * #AgsAudioSignal organizes audio data within a #GList whereby data
 * pointing to the buffer.
 */

void ags_audio_signal_class_init(AgsAudioSignalClass *audio_signal_class);
void ags_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_signal_init(AgsAudioSignal *audio_signal);
void ags_audio_signal_set_property(GObject *gobject,
				   guint prop_id,
				   const GValue *value,
				   GParamSpec *param_spec);
void ags_audio_signal_get_property(GObject *gobject,
				   guint prop_id,
				   GValue *value,
				   GParamSpec *param_spec);
void ags_audio_signal_connect(AgsConnectable *connectable);
void ags_audio_signal_disconnect(AgsConnectable *connectable);
void ags_audio_signal_dispose(GObject *gobject);
void ags_audio_signal_finalize(GObject *gobject);

void ags_audio_signal_real_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size);

enum{
  PROP_0,
  PROP_SOUNDCARD,
  PROP_RECYCLING,
  PROP_RECALL_ID,
  PROP_SAMPLERATE,
  PROP_BUFFER_SIZE,
  PROP_FORMAT,
  PROP_WORD_SIZE,
  PROP_LENGTH,
  PROP_FIRST_FRAME,
  PROP_LAST_FRAME,
  PROP_FRAME_COUNT,
  PROP_LOOP_START,
  PROP_LOOP_END,
  PROP_DELAY,
  PROP_ATTACK,
  PROP_STREAM,
  PROP_STREAM_END,
  PROP_STREAM_CURRENT,
  PROP_NOTE,
};

enum{
  REALLOC_BUFFER_SIZE,
  LAST_SIGNAL,
};

static gpointer ags_audio_signal_parent_class = NULL;
static guint audio_signal_signals[LAST_SIGNAL];

GType
ags_audio_signal_get_type(void)
{
  static GType ags_type_audio_signal = 0;

  if(!ags_type_audio_signal){
    static const GTypeInfo ags_audio_signal_info = {
      sizeof (AgsAudioSignalClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_signal_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioSignal),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_signal_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_signal_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_audio_signal = g_type_register_static(G_TYPE_OBJECT,
						   "AgsAudioSignal\0",
						   &ags_audio_signal_info,
						   0);

    g_type_add_interface_static(ags_type_audio_signal,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_audio_signal);
}

void
ags_audio_signal_class_init(AgsAudioSignalClass *audio_signal)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_audio_signal_parent_class = g_type_class_peek_parent(audio_signal);

  /* GObjectClass */
  gobject = (GObjectClass *) audio_signal;

  gobject->set_property = ags_audio_signal_set_property;
  gobject->get_property = ags_audio_signal_get_property;

  gobject->dispose = ags_audio_signal_dispose;
  gobject->finalize = ags_audio_signal_finalize;

  /* properties */
  /**
   * AgsAudioSignal:soundcard:
   *
   * The assigned #AgsSoundcard providing default settings.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("soundcard\0",
				   "assigned soundcard\0",
				   "The soundcard it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /**
   * AgsAudioSignal:recycling:
   *
   * The assigned #AgsRecycling linking tree.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("recycling\0",
				   "assigned recycling\0",
				   "The recycling it is assigned with\0",
				   AGS_TYPE_RECYCLING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECYCLING,
				  param_spec);

  /**
   * AgsAudioSignal:recall-id:
   *
   * The assigned #AgsRecallID providing context.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("recall_id\0",
				   "assigned recall id\0",
				   "The recall id it is assigned with\0",
				   AGS_TYPE_RECALL_ID,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL_ID,
				  param_spec);

  /**
   * AgsAudioSignal:samplerate:
   *
   * The samplerate to be used.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("samplerate\0",
				 "using samplerate\0",
				 "The samplerate to be used\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SAMPLERATE,
				  param_spec);

  /**
   * AgsAudioSignal:buffer-size:
   *
   * The buffer size to be used.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("buffer-size\0",
				 "using buffer size\0",
				 "The buffer size to be used\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_BUFFER_SIZE,
				  param_spec);

  /**
   * AgsAudioSignal:format:
   *
   * The format to be used.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("format\0",
				 "using format\0",
				 "The format to be used\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FORMAT,
				  param_spec);

  /**
   * AgsAudioSignal:word-size:
   *
   * The word size of frame.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("word-size\0",
				 "frame word size\0",
				 "The word size of a frame\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_WORD_SIZE,
				  param_spec);

  /**
   * AgsAudioSignal:length:
   *
   * The length of the stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("length\0",
				 "stream length\0",
				 "The length of the stream\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LENGTH,
				  param_spec);

  /**
   * AgsAudioSignal:first-frame:
   *
   * The first frame of stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("first-frame\0",
				 "stream's first frame\0",
				 "The first frame of the stream\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FIRST_FRAME,
				  param_spec);

  /**
   * AgsAudioSignal:last-frame:
   *
   * The last frame of stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("last-frame\0",
				 "stream's last frame\0",
				 "The last frame of the stream\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LAST_FRAME,
				  param_spec);

  /**
   * AgsAudioSignal:frame-count:
   *
   * The initial size of audio data.
   * 
   * Since: 1.0.0
   */
  param_spec = g_param_spec_uint("frame-count\0",
				 "frame count of audio data\0",
				 "The initial frame count of audio data\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FRAME_COUNT,
				  param_spec);

  /**
   * AgsAudioSignal:loop-start:
   *
   * The loop start of stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("loop-start\0",
				 "stream's loop start\0",
				 "The loop start of the stream\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LOOP_START,
				  param_spec);

  /**
   * AgsAudioSignal:loop-end:
   *
   * The loop end of stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("loop-end\0",
				 "stream's loop end\0",
				 "The loop end of the stream\0",
				 0,
				 G_MAXUINT32,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_LOOP_END,
				  param_spec);

  /**
   * AgsAudioSignal:delay:
   *
   * The delay to be used.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_double("delay\0",
				   "using delay\0",
				   "The delay to be used\0",
				   0.0,
				   65535.0,
				   0.0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DELAY,
				  param_spec);

  /**
   * AgsAudioSignal:attack:
   *
   * The attack to be used.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("attack\0",
				 "using attack\0",
				 "The attack to be used\0",
				 0,
				 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_ATTACK,
				  param_spec);


  /**
   * AgsAudioSignal:stream:
   *
   * The stream it contains.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_pointer("stream\0",
				    "containing stream\0",
				    "The stream it contains\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM,
				  param_spec);

  /**
   * AgsAudioSignal:stream-end:
   *
   * The end of stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_pointer("stream-end\0",
				    "end of stream\0",
				    "The stream's end\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM_END,
				  param_spec);

  /**
   * AgsAudioSignal:stream-current:
   *
   * The current stream.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_pointer("stream-current\0",
				    "current stream\0",
				    "The current stream\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM_CURRENT,
				  param_spec);

  /**
   * AgsAudioSignal:note:
   *
   * The assigned #AgsNote providing default settings.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("note\0",
				   "assigned note\0",
				   "The note it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_NOTE,
				  param_spec);

  /* AgsAudioSignalClass */
  audio_signal->realloc_buffer_size = ags_audio_signal_real_realloc_buffer_size;

  /* signals */
  /**
   * AgsAudioSignal::realloc-buffer-size:
   * @audio_signal: the object to realloc buffer size
   * @buffer_size: new buffer size
   *
   * The ::reallloc-buffer-size signal is invoked to notify modified buffer size.
   */
  audio_signal_signals[REALLOC_BUFFER_SIZE] =
    g_signal_new("realloc-buffer-size\0",
		 G_TYPE_FROM_CLASS (audio_signal),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsAudioSignalClass, realloc_buffer_size),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__UINT,
		 G_TYPE_NONE, 1,
		 G_TYPE_UINT);
}

void
ags_audio_signal_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_audio_signal_connect;
  connectable->disconnect = ags_audio_signal_disconnect;
}

void
ags_audio_signal_init(AgsAudioSignal *audio_signal)
{
  AgsConfig *config;
  
  gchar *str;
  
  audio_signal->flags = 0;

  audio_signal->soundcard = NULL;

  audio_signal->recycling = NULL;
  audio_signal->recall_id = NULL;

  config = ags_config_get_instance();

  /* samplerate */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "samplerate\0");
  
  if(str == NULL){
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD_0,
			       "samplerate\0");
  }  

  if(str != NULL){
    audio_signal->samplerate = g_ascii_strtoull(str,
						NULL,
						10);
    free(str);
  }else{
    audio_signal->samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
  }

  /* buffer-size */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "buffer-size\0");

  if(str == NULL){
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD_0,
			       "buffer-size\0");
  }
  
  if(str != NULL){
    audio_signal->buffer_size = g_ascii_strtoull(str,
						 NULL,
						 10);
    free(str);
  }else{
    audio_signal->buffer_size = AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE;
  }

  /* format */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "format\0");

  if(str == NULL){
    str = ags_config_get_value(config,
			       AGS_CONFIG_SOUNDCARD_0,
			       "format\0");
  }
  
  if(str != NULL){
    audio_signal->format = g_ascii_strtoull(str,
					    NULL,
					    10);
    free(str);
  }else{
    audio_signal->format = AGS_SOUNDCARD_SIGNED_16_BIT;
  }

  /*  */
  audio_signal->length = 0;
  audio_signal->last_frame = 0;

  audio_signal->frame_count = 0;
  audio_signal->loop_start = 0;
  audio_signal->loop_end = 0;

  audio_signal->delay = 0.0;
  audio_signal->attack = 0;

  audio_signal->stream_beginning = NULL;
  audio_signal->stream_current = NULL;
  audio_signal->stream_end = NULL;

  audio_signal->note = NULL;
}

void
ags_audio_signal_set_property(GObject *gobject,
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *param_spec)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      guint samplerate;
      guint buffer_size;
      guint format;
      
      soundcard = g_value_get_object(value);

      if(audio_signal->soundcard == soundcard)
	return;

      if(audio_signal->soundcard != NULL){
	g_object_unref(audio_signal->soundcard);
      }
      
      if(soundcard != NULL){
	g_object_ref(soundcard);
      }
      
      audio_signal->soundcard = soundcard;

      if(soundcard != NULL){
	ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
				  NULL,
				  &samplerate,
				  &buffer_size,
				  &format);

	g_object_set(audio_signal,
		     "samplerate\0", samplerate,
		     "buffer-size\0", buffer_size,
		     "format\0", format,
		     NULL);
      }
    }
    break;
  case PROP_RECYCLING:
    {
      GObject *recycling;

      recycling = g_value_get_object(value);

      if(audio_signal->recycling == recycling){
	return;
      }

      if(audio_signal->recycling != NULL){
	g_object_unref(audio_signal->recycling);
      }
      
      if(recycling != NULL){
	g_object_ref(recycling);
      }
      
      audio_signal->recycling = recycling;
    }
    break;
  case PROP_RECALL_ID:
    {
      GObject *recall_id;

      recall_id = g_value_get_object(value);
      
      if(audio_signal->recall_id == recall_id)
	return;
      
      if(audio_signal->recall_id != NULL)
	g_object_unref(audio_signal->recall_id);

      if(recall_id != NULL)
	g_object_ref(recall_id);

      audio_signal->recall_id = recall_id;
    }
    break;
  case PROP_SAMPLERATE:
    {
      guint samplerate;

      samplerate = g_value_get_uint(value);

      ags_audio_signal_set_samplerate(audio_signal,
				      samplerate);
    }
    break;
  case PROP_BUFFER_SIZE:
    {
      guint buffer_size;

      buffer_size = g_value_get_uint(value);

      ags_audio_signal_set_buffer_size(audio_signal,
				       buffer_size);
    }
  case PROP_FORMAT:
    {
      guint format;

      format = g_value_get_uint(value);

      ags_audio_signal_set_format(audio_signal,
				  format);
    }
    break;
  case PROP_WORD_SIZE:
    {
      //TODO:JK: implement me
    }
    break;
  case PROP_LENGTH:
    {
      guint length;

      length = g_value_get_uint(value);

      audio_signal->length = length;
    }
    break;
  case PROP_FIRST_FRAME:
    {
      guint first_frame;

      first_frame = g_value_get_uint(value);

      audio_signal->first_frame = first_frame;
    }
    break;
  case PROP_LAST_FRAME:
    {
      guint last_frame;

      last_frame = g_value_get_uint(value);

      audio_signal->last_frame = last_frame;
    }
    break;
  case PROP_FRAME_COUNT:
    {
      guint frame_count;

      frame_count = g_value_get_uint(value);

      audio_signal->frame_count = frame_count;
    }
    break;
  case PROP_LOOP_START:
    {
      guint loop_start;

      loop_start = g_value_get_uint(value);

      audio_signal->loop_start = loop_start;
    }
    break;
  case PROP_LOOP_END:
    {
      guint loop_end;

      loop_end = g_value_get_uint(value);

      audio_signal->loop_end = loop_end;
    }
    break;
  case PROP_DELAY:
    {
      gdouble delay;

      delay = g_value_get_double(value);

      audio_signal->delay = delay;
    }
    break;
  case PROP_ATTACK:
    {
      guint attack;

      attack = g_value_get_uint(value);

      audio_signal->attack = attack;
    }
    break;
  case PROP_STREAM:
    {
      gpointer data;

      data = g_value_get_pointer(value);

      if(data == NULL ||
	 g_list_find(audio_signal->stream_beginning,
		     data) != NULL){
      }

      audio_signal->stream_beginning = g_list_append(audio_signal->stream_beginning,
						     data);
      audio_signal->stream_end = g_list_last(audio_signal->stream_beginning);
    }
    break;
  case PROP_STREAM_CURRENT:
    {
      GList *current;

      current = g_value_get_pointer(value);

      audio_signal->stream_current = current;
    }
    break;
  case PROP_NOTE:
    {
      GObject *note;

      note = g_value_get_object(value);

      if(audio_signal->note == note)
	return;

      if(audio_signal->note != NULL)
	g_object_unref(audio_signal->note);

      if(note != NULL)
	g_object_ref(note);

      audio_signal->note = note;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_signal_get_property(GObject *gobject,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *param_spec)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    g_value_set_object(value, audio_signal->soundcard);
    break;
  case PROP_RECYCLING:
    g_value_set_object(value, audio_signal->recycling);
    break;
  case PROP_RECALL_ID:
    g_value_set_object(value, audio_signal->recall_id);
    break;
  case PROP_SAMPLERATE:
    {
      g_value_set_uint(value, audio_signal->samplerate);
    }
    break;
  case PROP_BUFFER_SIZE:
    {
      g_value_set_uint(value, audio_signal->buffer_size);
    }
  case PROP_FORMAT:
    {
      g_value_set_uint(value, audio_signal->format);
    }
    break;
  case PROP_WORD_SIZE:
    {
      //TODO:JK: implement me
    }
    break;
  case PROP_LENGTH:
    {
      g_value_set_uint(value, audio_signal->length);
    }
    break;
  case PROP_FIRST_FRAME:
    {
      g_value_set_uint(value, audio_signal->first_frame);
    }
    break;
  case PROP_LAST_FRAME:
    {
      g_value_set_uint(value, audio_signal->last_frame);
    }
    break;
  case PROP_FRAME_COUNT:
    {
      g_value_set_uint(value, audio_signal->frame_count);
    }
    break;
  case PROP_LOOP_START:
    {
      g_value_set_uint(value, audio_signal->loop_start);
    }
    break;
  case PROP_LOOP_END:
    {
      g_value_set_uint(value, audio_signal->loop_end);
    }
    break;
  case PROP_DELAY:
    {
      g_value_set_double(value, audio_signal->delay);
    }
    break;
  case PROP_ATTACK:
    {
      g_value_set_uint(value, audio_signal->attack);
    }
    break;
  case PROP_STREAM:
    {
      g_value_set_pointer(value, g_list_copy(audio_signal->stream_beginning));
    }
    break;
  case PROP_STREAM_END:
    {
      g_value_set_pointer(value, audio_signal->stream_end);
    }
    break;
  case PROP_STREAM_CURRENT:
    {
      g_value_set_pointer(value, audio_signal->stream_current);
    }
    break;
  case PROP_NOTE:
    g_value_set_object(value, audio_signal->note);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_signal_connect(AgsConnectable *connectable)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(connectable);

  if((AGS_AUDIO_SIGNAL_CONNECTED & (audio_signal->flags)) != 0){
    return;
  }

  audio_signal->flags |= AGS_AUDIO_SIGNAL_CONNECTED;
}

void
ags_audio_signal_disconnect(AgsConnectable *connectable)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(connectable);

  if((AGS_AUDIO_SIGNAL_CONNECTED & (audio_signal->flags)) == 0){
    return;
  }

  audio_signal->flags &= (~AGS_AUDIO_SIGNAL_CONNECTED);
}

void
ags_audio_signal_dispose(GObject *gobject)
{
  AgsAudioSignal *audio_signal;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

  /* soundcard */
  if(audio_signal->soundcard != NULL){
    g_object_unref(audio_signal->soundcard);

    audio_signal->soundcard = NULL;
  }

  /* recycling */
  if(audio_signal->recycling != NULL){
    g_object_unref(audio_signal->recycling);
    
    audio_signal->recycling = NULL;  
  }

  /* recall id */
  if(audio_signal->recall_id != NULL){
    g_object_unref(audio_signal->recall_id);

    audio_signal->recall_id = NULL;
  }

  /* note */
  if(audio_signal->note != NULL){
    g_object_unref(audio_signal->note);

    audio_signal->note = NULL;
  }
}

void
ags_audio_signal_finalize(GObject *gobject)
{
  AgsAudioSignal *audio_signal;

  guint *ids;
  guint i, n_ids;

  audio_signal = AGS_AUDIO_SIGNAL(gobject);

#ifdef AGS_DEBUG
  g_message("finalize AgsAudioSignal\0");
#endif

  if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0){
    g_warning("AGS_AUDIO_SIGNAL_TEMPLATE: destroying\n\0");
  }

  /* disconnect */
  ids = g_signal_list_ids(AGS_TYPE_AUDIO_SIGNAL,
			  &n_ids);
  
  for(i = 0; i < n_ids; i++){
    g_signal_handlers_disconnect_matched(gobject,
					 G_SIGNAL_MATCH_ID,
					 ids[i],
					 0,
					 NULL,
					 NULL,
					 NULL);
  }

  g_free(ids);

  /* soundcard */
  if(audio_signal->soundcard != NULL){
    g_object_unref(audio_signal->soundcard);
  }

  /* recycling */
  if(audio_signal->recycling != NULL){
    g_object_unref(audio_signal->recycling);
  }

  /* recall id */
  if(audio_signal->recall_id != NULL){
    g_object_unref(audio_signal->recall_id);
  }

  /* audio data */
  if(audio_signal->stream_beginning != NULL){
    g_list_free_full(audio_signal->stream_beginning,
		     (GDestroyNotify) ags_stream_free);
  }

  /* note */
  if(audio_signal->note != NULL){
    g_object_unref(audio_signal->note);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_audio_signal_parent_class)->finalize(gobject);
}

/**
 * ags_stream_alloc:
 * @buffer_size: the buffer size
 *
 * Allocs an audio buffer.
 *
 * Returns: the audio data array
 *
 * Since: 0.3
 */
void*
ags_stream_alloc(guint buffer_size,
		 guint format)
{
  void *buffer;
  guint word_size;
  
  switch(format){
  case AGS_SOUNDCARD_SIGNED_8_BIT:
    {
      buffer = (signed char *) malloc(buffer_size * sizeof(signed char));
      word_size = sizeof(signed char);
    }
    break;
  case AGS_SOUNDCARD_SIGNED_16_BIT:
    {
      buffer = (signed short *) malloc(buffer_size * sizeof(signed short));
      word_size = sizeof(signed short);
    }
    break;
  case AGS_SOUNDCARD_SIGNED_24_BIT:
    {
      buffer = (signed long *) malloc(buffer_size * sizeof(signed long));
      //NOTE:JK: The 24-bit linear samples use 32-bit physical space
      word_size = sizeof(signed long);
    }
    break;
  case AGS_SOUNDCARD_SIGNED_32_BIT:
    {
      buffer = (signed long *) malloc(buffer_size * sizeof(signed long));
      word_size = sizeof(signed long);
    }
    break;
  case AGS_SOUNDCARD_SIGNED_64_BIT:
    {
      buffer = (signed long long *) malloc(buffer_size * sizeof(signed long long));
      word_size = sizeof(signed long long);
    }
    break;
  default:
    g_warning("ags_stream_alloc(): unsupported word size\0");
    return(NULL);
  }

  memset(buffer, 0, buffer_size * word_size);

  return(buffer);
}

void
ags_stream_free(signed short *buffer)
{
  free(buffer);
}

/**
 * ags_audio_signal_set_samplerate:
 * @audio_signal: the #AgsAudioSignal
 * @samplerate: the samplerate
 *
 * Set samplerate.
 *
 * Since: 0.7.45
 */
void
ags_audio_signal_set_samplerate(AgsAudioSignal *audio_signal, guint samplerate)
{
  audio_signal->samplerate = samplerate;
}

/**
 * ags_audio_signal_set_buffer_size:
 * @audio_signal: the #AgsAudioSignal
 * @buffer_size: the buffer size
 *
 * Set buffer size.
 *
 * Since: 0.7.45
 */
void
ags_audio_signal_set_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size)
{
  GList *stream;

  stream = audio_signal->stream_beginning;

  while(stream != NULL){
    switch(audio_signal->format){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       buffer_size * sizeof(signed char));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       buffer_size * sizeof(signed short));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       buffer_size * sizeof(signed long));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       buffer_size * sizeof(signed long));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       buffer_size * sizeof(signed long long));
      }
      break;
    default:
      g_warning("ags_audio_signal_set_buffer_size() - unsupported format\0");
    }
    
    stream = stream->next;
  }

  audio_signal->buffer_size = buffer_size;
}

/**
 * ags_audio_signal_set_format:
 * @audio_signal: the #AgsAudioSignal
 * @format: the format
 *
 * Set format.
 *
 * Since: 0.7.45
 */
void
ags_audio_signal_set_format(AgsAudioSignal *audio_signal, guint format)
{
  GList *stream;

  stream = audio_signal->stream_beginning;

  while(stream != NULL){
    switch(format){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       audio_signal->buffer_size * sizeof(signed char));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       audio_signal->buffer_size * sizeof(signed short));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       audio_signal->buffer_size * sizeof(signed long));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       audio_signal->buffer_size * sizeof(signed long));
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	stream->data = (signed char *) realloc(stream->data,
					       audio_signal->buffer_size * sizeof(signed long long));
      }
      break;
    default:
      g_warning("ags_audio_signal_set_format() - unsupported format\0");
    }
    
    stream = stream->next;
  }

  audio_signal->format = format;
}

/**
 * ags_audio_signal_get_length_till_current:
 * @audio_signal: an #AgsAudioSignal
 *
 * Counts the buffers from stream_beginning upto stream_current.
 *
 * Returns: the counted length.
 *
 * Since: 0.4
 */
guint
ags_audio_signal_get_length_till_current(AgsAudioSignal *audio_signal)
{
  GList *list, *stop;
  guint length;

  if(audio_signal == NULL){
    return(0);
  }

  list = audio_signal->stream_beginning;
  length = 0;

  if(audio_signal->stream_current != NULL){
    stop = audio_signal->stream_current->next;
  }else{
    return(0);
  }

  while(list != stop){
    length++;
    list = list->next;
  }

  return(length);
}

/**
 * ags_audio_signal_add_stream:
 * @audio_signal: an #AgsAudioSignal
 *
 * Adds a buffer at the end of the stream.
 *
 * Since: 0.3
 */
void
ags_audio_signal_add_stream(AgsAudioSignal *audio_signal)
{
  GList *stream, *end_old;
  signed short *buffer;

  stream = g_list_alloc();
  buffer = ags_stream_alloc(audio_signal->buffer_size,
			    audio_signal->format);
  stream->data = buffer;

  if(audio_signal->stream_end != NULL){
    end_old = audio_signal->stream_end;

    stream->prev = end_old;
    end_old->next = stream;
  }else{
    audio_signal->stream_beginning = stream;
    audio_signal->stream_current = stream;
  }

  audio_signal->stream_end = stream;

  audio_signal->length += 1;
}

/**
 * ags_audio_signal_stream_resize:
 * @audio_signal: an #AgsAudioSignal to resize.
 * @length: a guint as the new length.
 *
 * Resizes an #AgsAudioSignal's stream but be carefull with shrinking.
 * This function may crash the application.
 *
 * Since: 0.3
 */
void
ags_audio_signal_stream_resize(AgsAudioSignal *audio_signal, guint length)
{
  guint i;

  if(audio_signal->length < length){
    GList *stream, *end_old;
    signed short *buffer;

    stream = NULL;

    for(i = audio_signal->length; i < length; i++){
      buffer = ags_stream_alloc(audio_signal->buffer_size,
				audio_signal->format);

      stream = g_list_prepend(stream,
			      buffer);
    }

    stream = g_list_reverse(stream);

    if(audio_signal->stream_end != NULL){
      end_old = audio_signal->stream_end;
      audio_signal->stream_end = g_list_last(stream);

      stream->prev = end_old;
      end_old->next = stream;
    }else{
      audio_signal->stream_beginning = stream;
      audio_signal->stream_end = g_list_last(stream);
      audio_signal->stream_current = stream;
    }
  }else if(audio_signal->length > length){
    GList *stream, *stream_end, *stream_next;
    gboolean check_current;

    stream = audio_signal->stream_beginning;
    check_current = TRUE;

    for(i = 0; i < length; i++){
      if(check_current && stream == audio_signal->stream_current){
	audio_signal->stream_current = NULL;
	check_current = FALSE;
      }

      stream = stream->next;
    }

    if(length != 0){
      stream_end = stream->prev;
      stream_end->next = NULL;
      audio_signal->stream_end = stream_end;
    }else{
      audio_signal->stream_beginning = NULL;
      audio_signal->stream_current = NULL;
      audio_signal->stream_end = NULL;
    }

    stream->prev = NULL;
    g_list_free_full(stream,
		     g_free);
  }

  audio_signal->length = length;
}

/**
 * ags_audio_signal_stream_safe_resize:
 * @audio_signal: an #AgsAudioSignal
 * @length: a guint
 *
 * Resizes an #AgsAudioSignal's stream but doesn't shrink more than the
 * current stream position.
 *
 * Since: 0.4
 */
void
ags_audio_signal_stream_safe_resize(AgsAudioSignal *audio_signal, guint length)
{
  guint length_till_current;

  length_till_current = ags_audio_signal_get_length_till_current(audio_signal);

  if(length_till_current < length){
    ags_audio_signal_stream_resize(audio_signal,
				   length);
  }else{
    ags_audio_signal_stream_resize(audio_signal,
				   length_till_current);
  }
}

void
ags_audio_signal_real_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size)
{
  GList *current, *old; 
  guint old_buffer_size;
  guint counter;
  guint i_old, i_current;
  
  old = audio_signal->stream_beginning;
  old_buffer_size = audio_signal->buffer_size;

  current = NULL;
  current = g_list_prepend(current,
			   ags_stream_alloc(buffer_size,
					    audio_signal->format));
  counter = 0;

  i_old = 0;
  i_current = 0;

  while(old != NULL){
    ((signed short *) current->data)[i_current] = ((signed short *) old->data)[i_old];
    
    if(i_current == buffer_size){
      current = g_list_prepend(current,
			       ags_stream_alloc(buffer_size,
						audio_signal->format));
      
      i_current = 0;
      
      counter++;
    }
    
    if(i_old == old_buffer_size){
      old = old->next;
      
      i_old = 0;
    }
    
    i_current++;
    i_old++;
  }
  
  memset(((signed short *) current->data), 0, (buffer_size - i_current) * sizeof(signed short));
  
  /*  */
  audio_signal->length = counter;
  audio_signal->last_frame = i_current;
  
  audio_signal->stream_end = current;
  audio_signal->stream_beginning = g_list_reverse(current);
}

/**
 * ags_audio_signal_realloc_buffer_size:
 * @audio_signal: an #AgsAudioSignal
 * @buffer_size: the buffer size
 *
 * Realloc the stream to the new buffer size. 
 *
 * Since: 0.4
 */
void
ags_audio_signal_realloc_buffer_size(AgsAudioSignal *audio_signal, guint buffer_size)
{
  g_return_if_fail(AGS_IS_AUDIO_SIGNAL(audio_signal));
  g_object_ref(G_OBJECT(audio_signal));
  g_signal_emit(G_OBJECT(audio_signal),
		audio_signal_signals[REALLOC_BUFFER_SIZE], 0,
		buffer_size);
  g_object_unref(G_OBJECT(audio_signal));
}

/**
 * ags_audio_signal_copy_double_buffer_to_buffer:
 * @destination: destination buffer
 * @dchannels: destination audio channels
 * @source: source buffer
 * @schannels: source audio channels
 * @size: frame count to copy
 *
 * Copy a buffer to an other buffer.
 *
 * Since: 0.4
 */
void
ags_audio_signal_copy_double_buffer_to_buffer(signed short *destination, guint dchannels,
					      double *source, guint schannels,
					      guint size)
{
  for(; 0<size; --size){
    *destination += *source;

    destination += dchannels;
    source += schannels;
  }
}

/**
 * ags_audio_signal_duplicate_stream:
 * @audio_signal: destination #AgsAudioSignal
 * @template: source #AgsAudioSignal
 * 
 * Copy stream of one audio signal to another
 *
 * Since: 0.3
 */
void
ags_audio_signal_duplicate_stream(AgsAudioSignal *audio_signal,
				  AgsAudioSignal *template)
{
  if(template == NULL){
    if(audio_signal == NULL){
      return;
    }else{
      ags_audio_signal_stream_resize(audio_signal,
				     0);
      return;
    }
  }

  if(template->stream_beginning == NULL){
    ags_audio_signal_stream_resize(audio_signal,
				   0);
  }else{
    GList *template_stream, *stream;

    guint word_size;
    guint copy_mode;

    audio_signal->buffer_size = template->buffer_size;
    ags_audio_signal_stream_resize(audio_signal,
				   template->length);

    stream = audio_signal->stream_beginning;
    template_stream = template->stream_beginning;

    copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(audio_signal->format),
						    ags_audio_buffer_util_format_from_soundcard(template->format));

    switch(audio_signal->format){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	word_size = sizeof(signed char);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	word_size = sizeof(signed short);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	//NOTE:JK: The 24-bit linear samples use 32-bit physical space
	word_size = sizeof(signed long);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	word_size = sizeof(signed long);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	word_size = sizeof(signed long long);
      }
      break;
    default:
      g_warning("ags_audio_signal_duplicate(): unsupported word size\0");
      return;
    }
    
    while(template_stream != NULL){
      ags_audio_buffer_util_copy_buffer_to_buffer(stream->data, 1, 0,
						  template_stream->data, 1, 0,
						  template->buffer_size, copy_mode);
      stream = stream->next;
      template_stream = template_stream->next;
    }
  }
}

/**
 * ags_audio_signal_get_template:
 * @audio_signal: a #GList containing #AgsAudioSignal
 *
 * Retrieve the template audio signal.
 *
 * Returns: the template #AgsAudioSignal
 *
 * Since: 0.3
 */
AgsAudioSignal*
ags_audio_signal_get_template(GList *audio_signal)
{
  GList *list;

  list = audio_signal;

  while(list != NULL){
    if((AGS_AUDIO_SIGNAL_TEMPLATE & (AGS_AUDIO_SIGNAL(list->data)->flags)) != 0)
      return((AgsAudioSignal *) list->data);

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_audio_signal_get_stream_current:
 * @audio_signal: a #GList containing #AgsAudioSignal
 * @recall_id: the matching #AgsRecallID
 * 
 * Retrieve next current stream of #AgsAudioSignal list.
 *
 * Returns: next #GList matching #AgsRecallID
 *
 * Since: 0.4
 */
GList*
ags_audio_signal_get_stream_current(GList *list_audio_signal,
				    GObject *recall_id)
{
  AgsAudioSignal *audio_signal;
  GList *list;

  list = list_audio_signal;

  while(list != NULL){
    audio_signal = AGS_AUDIO_SIGNAL(list->data);

    if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0){
      list = list->next;
      continue;
    }

    if(audio_signal->stream_current != NULL && audio_signal->recall_id == recall_id)
      return(list);

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_audio_signal_get_by_recall_id:
 * @audio_signal: a #GList containing #AgsAudioSignal
 * @recall_id: matching #AgsRecallID
 *
 * Retrieve next audio signal refering to @recall_id
 *
 * Returns: matching #AgsAudioSignal
 *
 * Since: 0.3
 */
GList*
ags_audio_signal_get_by_recall_id(GList *list_audio_signal,
				  GObject *recall_id)
{
  AgsAudioSignal *audio_signal;
  GList *list;

  list = list_audio_signal;

  while(list != NULL){
    audio_signal = AGS_AUDIO_SIGNAL(list->data);

    if((AGS_AUDIO_SIGNAL_TEMPLATE & (audio_signal->flags)) != 0){
      list = list->next;
      continue;
    }

    if(audio_signal->recall_id == recall_id)
      return(list);

    list = list->next;
  }

  return(NULL);
}

/**
 * ags_audio_signal_tile:
 * @audio_signal: an #AgsAudioSignal
 * @template: the source #AgsAudioSignal
 * @frame_count: new frame count
 *
 * Tile audio signal data.
 *
 * Since: 0.4
 */
void
ags_audio_signal_tile(AgsAudioSignal *audio_signal,
		      AgsAudioSignal *template,
		      guint frame_count)
{
  GObject *soundcard;
  GList *template_stream, *audio_signal_stream, *audio_signal_stream_end;
  signed short *template_buffer, *audio_signal_buffer;
  guint template_size;
  guint remaining_size;
  guint i, j, j_offcut;
  guint k, k_end;
  guint copy_mode;
  gboolean alloc_buffer;
  
  soundcard = audio_signal->soundcard;

  audio_signal_stream = NULL;
  template_stream = template->stream_beginning;

  if(template_stream == NULL){
    return;
  }

  template_size = (guint) (template->delay * template->buffer_size) +
    template->length * template->buffer_size +
    template->last_frame;
  
  copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(audio_signal->format),
						  ags_audio_buffer_util_format_from_soundcard(template->format));
  
  j = 0;
  k = 0;

  if(template->buffer_size > audio_signal->buffer_size){
    k_end = audio_signal->buffer_size;
  }else if(template->buffer_size == audio_signal->buffer_size){
    k_end = audio_signal->buffer_size;
  }else{
    k_end = template->buffer_size;
  }

  j_offcut = 0;
  alloc_buffer = TRUE;

  /* write buffers */
  for(i = 0; i < frame_count - template->buffer_size; i += audio_signal->buffer_size){
    /* alloc buffer and prepend */
    if(alloc_buffer){
      audio_signal_buffer = ags_stream_alloc(audio_signal->buffer_size,
					     audio_signal->format);
      audio_signal_stream = g_list_prepend(audio_signal_stream,
					   audio_signal_buffer);
    }

    /* get template buffer */
    template_buffer = (signed short *) template_stream->data;

    /* allocate and copy buffer */
    if(template_size < audio_signal->buffer_size){
      /* copy buffer */
      //FIXME:JK: 
      ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, 0,
						  template_buffer, 1, j_offcut,
						  template_size - j_offcut, copy_mode);
      //      ags_audio_signal_copy_buffer_to_buffer(audio_signal_buffer, 1,
      //				     &(template_buffer[j_offcut]), 1, template_size - j_offcut);

      for(j = template_size - j_offcut; j < audio_signal->buffer_size - template_size; j += template_size){
	//FIXME:JK:
	ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, 0,
						    template_buffer, 1, 0,
						    template_size, copy_mode);
	//	ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
	//				       template_buffer, 1, template_size);
      }

      j_offcut = audio_signal->buffer_size - j;
      //FIXME:JK:
      ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, j,
						  template_buffer, 1, j_offcut,
						  k_end - j_offcut, copy_mode);
      //      ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
      //				     template_buffer, 1, j_offcut);
    }else{
      /* deep copy */
      //FIXME:JK:
      ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, j,
						  template_buffer, 1, j_offcut,
						  k_end - j_offcut, copy_mode);
      //      ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
      //				     &(template_buffer[j_offcut]), 1, k_end - j_offcut);
      k += (k_end - j_offcut);

      /* iterate template stream */
      if(k_end + j_offcut == template->buffer_size){
	if(template_stream->next != NULL){
	  template_stream = template_stream->next;
	}else{
	  template_stream = template->stream_beginning;
	}
      }

      /* copy parameters */
      j = k % audio_signal->buffer_size;
	
      if(template->buffer_size < audio_signal->buffer_size){
	j_offcut = k % template->buffer_size;
      }else if(template->buffer_size == audio_signal->buffer_size){
	j_offcut = k % audio_signal->buffer_size;
      }else{
	j_offcut = k % audio_signal->buffer_size;
      }

      k_end = template->buffer_size - j_offcut;

      /* alloc audio signal */
      if(j == 0){
	alloc_buffer = TRUE;
      }else{
	alloc_buffer = FALSE;
      }
    }
  }
  
  /* write remaining buffer */
  remaining_size = frame_count - (i * audio_signal->buffer_size);

  if(remaining_size > k_end - j_offcut){
    ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, j,
						template_buffer, 1, j_offcut,
						k_end - j_offcut, copy_mode);
    //    ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
    //					   &(template_buffer[j_offcut]), 1, k_end - j_offcut);

    if(k_end + j_offcut == template->buffer_size){
      if(template_stream->next != NULL){
	template_stream = template_stream->next;
      }else{
	template_stream = template->stream_beginning;
      }
    }

    //FIXME:JK:
    ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, j,
						template_buffer, 1, j_offcut,
						remaining_size - (k_end - j_offcut), copy_mode);
    //    ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
    //					   &(template_buffer[j_offcut]), 1, remaining_size - (k_end - j_offcut));
  }else{
    //FIXME:JK:
    ags_audio_buffer_util_copy_buffer_to_buffer(audio_signal_buffer, 1, j,
						template_buffer, 1, j_offcut,
						remaining_size, copy_mode);
    //    ags_audio_signal_copy_buffer_to_buffer(&(audio_signal_buffer[j]), 1,
    //					   &(template_buffer[j_offcut]), 1, remaining_size);
  }

  /* reverse list */
  audio_signal_stream_end = audio_signal_stream;
  audio_signal_stream = g_list_reverse(audio_signal_stream);

  if(audio_signal->stream_beginning != NULL){
    g_list_free_full(audio_signal->stream_beginning,
		     g_free);
  }

  audio_signal->stream_beginning = audio_signal_stream;
  //  audio_signal->stream_current = audio_signal_stream;
  audio_signal->stream_end = audio_signal_stream_end;
}

/**
 * ags_audio_signal_scale:
 * @audio_signal: an #AgsAudioSignal
 * @template: the source #AgsAudioSignal
 * @length: new frame count
 *
 * Scale audio signal data.
 *
 * Since: 0.4
 */
void
ags_audio_signal_scale(AgsAudioSignal *audio_signal,
		       AgsAudioSignal *template,
		       guint length)
{
  GList *source, *destination, *stream_template;
  gpointer data;
  double scale_factor, morph_factor;
  guint offset;
  double step;
  guint i, j, j_stop;
  guint k, template_k;
  gboolean expand;

  auto void ags_audio_signal_scale_copy_8_bit(GList *source, GList *destination,
					      guint soffset, guint doffset,
					      guint dresolution);
  auto void ags_audio_signal_scale_copy_16_bit(GList *source, GList *destination,
					       guint soffset, guint doffset,
					       guint dresolution);
  auto void ags_audio_signal_scale_copy_24_bit(GList *source, GList *destination,
					       guint soffset, guint doffset,
					       guint dresolution);
  auto void ags_audio_signal_scale_copy_32_bit(GList *source, GList *destination,
					       guint soffset, guint doffset,
					       guint dresolution);
  auto void ags_audio_signal_scale_copy_64_bit(GList *source, GList *destination,
					       guint soffset, guint doffset,
					       guint dresolution);

  void ags_audio_signal_scale_copy_8_bit(GList *source, GList *destination,
					 guint soffset, guint doffset,
					 guint dresolution){
    gint8 *sbuffer;

    sbuffer = (gint8 *) source->data;

    switch(dresolution){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	gint8 *dbuffer;
	gdouble scale;

	dbuffer = (gint8 *) destination->data;

	scale = 1.0;

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	gint16 *dbuffer;
	gdouble scale;

	dbuffer = (gint16 *) destination->data;

	scale = exp2(8.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	unsigned char *dbuffer;
	gint16 value;
	gdouble scale;
	gint16 mask;

	dbuffer = (unsigned char *) destination->data;

	scale = exp2(16.0);

	value = scale * sbuffer[soffset];
	mask = 0xff;

	dbuffer[doffset * 3] = mask & value;
	dbuffer[doffset * 3 + 1] = (mask << 8) & value;
	
	if(sbuffer[soffset] < 0){
	  dbuffer[doffset * 3 + 1] &= (~0x80);
	  dbuffer[doffset * 3 + 2] = 0x80;
	}else{
	  dbuffer[doffset * 3 + 2] = 0;
	}
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	gint32 *dbuffer;
	gdouble scale;

	dbuffer = (gint32 *) destination->data;

	scale = exp2(24.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	gint64 *dbuffer;
	gdouble scale;

	dbuffer = (gint64 *) destination->data;

	scale = exp2(56.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    }
  }
  void ags_audio_signal_scale_copy_16_bit(GList *source, GList *destination,
					  guint soffset, guint doffset,
					  guint dresolution){
    gint16 *sbuffer;

    sbuffer = (gint16 *) source->data;

    switch(dresolution){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	gint8 *dbuffer;
	gdouble scale;

	dbuffer = (gint8 *) destination->data;

	scale = exp2(1.0 / 8.0);

	dbuffer[doffset] = (gint8) floor(scale * sbuffer[soffset]);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	gint16 *dbuffer;

	dbuffer = (gint16 *) destination->data;

	dbuffer[doffset] = sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	unsigned char *dbuffer;
	gint32 value;
	gdouble scale;
	gint32 mask;

	dbuffer = (unsigned char *) destination->data;

	scale = exp2(8.0);
	mask = 0xff;

	value = scale * sbuffer[soffset];
      
	dbuffer[doffset * 3] = mask & value;
	dbuffer[doffset * 3 + 1] = (mask << 8) & value;
	dbuffer[doffset * 3 + 2] = (mask << 16) & value;
	
	if(sbuffer[soffset] < 0){
	  dbuffer[doffset * 3 + 2] |= 0x80;
	}
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	gint32 *dbuffer;
	gdouble scale;

	dbuffer = (gint32 *) destination->data;

	scale = exp2(16.0);
	
	dbuffer[doffset] = sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	gint64 *dbuffer;
	gdouble scale;

	dbuffer = (gint64 *) destination->data;

	scale = exp2(48.0);
	
	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    }
  }
  void ags_audio_signal_scale_copy_24_bit(GList *source, GList *destination,
					  guint soffset, guint doffset,
					  guint dresolution){
    unsigned char *sbuffer;

    sbuffer = (char *) source->data;

    switch(dresolution){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	gint8 *dbuffer;
	gdouble scale;
	gint16 *mask;
	
	dbuffer = (gint8 *) destination->data;

	scale = exp2(1.0 / 16.0);

	dbuffer[doffset] = (gint8) round(scale * (double) sbuffer[soffset]);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	gint16 *dbuffer;
	gdouble scale;

	dbuffer = (gint16 *) destination->data;

	scale = exp2(1.0 / 8.0);

	dbuffer[doffset] = (gint16) round(scale * (double) sbuffer[soffset]);
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	unsigned char *dbuffer;
	gint16 mask;

	dbuffer = (unsigned char *) destination->data;

	dbuffer[doffset * 3] = sbuffer[soffset * 3];
	dbuffer[doffset * 3 + 1] = sbuffer[soffset * 3 + 1];
	dbuffer[doffset * 3 + 2] = sbuffer[soffset * 3 + 2];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	gint32 *dbuffer;
	gdouble scale;

	dbuffer = (gint32 *) destination->data;

	scale = exp2(8.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	gint64 *dbuffer;
	gdouble scale;

	dbuffer = (gint64 *) destination->data;

	scale = exp2(40.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    }
  }
  void ags_audio_signal_scale_copy_32_bit(GList *source, GList *destination,
					  guint soffset, guint doffset,
					  guint dresolution){
    gint32 *sbuffer;

    sbuffer = (gint32 *) source->data;

    switch(dresolution){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	gint8 *dbuffer;
	gdouble scale;

	dbuffer = (gint8 *) destination->data;

	scale = exp2(1.0 / 24.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	gint16 *dbuffer;
	gdouble scale;

	dbuffer = (gint16 *) destination->data;

	scale = exp2(1.0 / 16.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	unsigned char *dbuffer;
	gint32 value;
	gdouble scale;
	gint32 mask;

	dbuffer = (unsigned char *) destination->data;

	scale = exp2(1.0 / 8.0);
	mask = 0xff;

	value = scale * sbuffer[soffset];
      
	dbuffer[doffset * 3] = mask & value;
	dbuffer[doffset * 3 + 1] = (mask << 8) & value;
	dbuffer[doffset * 3 + 2] = (mask << 16) & value;
	
	if(sbuffer[soffset] < 0){
	  dbuffer[doffset * 3 + 2] |= 0x80;
	}
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	gint32 *dbuffer;

	dbuffer = (gint32 *) destination->data;

	dbuffer[doffset] = sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	gint64 *dbuffer;
	gdouble scale;

	dbuffer = (gint64 *) destination->data;

	scale = exp2(32.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    }
  }
  void ags_audio_signal_scale_copy_64_bit(GList *source, GList *destination,
					  guint soffset, guint doffset,
					  guint dresolution){
    gint64 *sbuffer;

    sbuffer = (gint64 *) source->data;

    switch(dresolution){
    case AGS_SOUNDCARD_SIGNED_8_BIT:
      {
	gint8 *dbuffer;
	gdouble scale;

	dbuffer = (gint8 *) destination->data;

	scale = exp2(1 / 56.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_16_BIT:
      {
	gint16 *dbuffer;
	gdouble scale;

	dbuffer = (gint16 *) destination->data;

	scale = exp2(1 / 48.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_24_BIT:
      {
	unsigned char *dbuffer;
	gint32 value;
	gdouble scale;
	gint32 mask;

	dbuffer = (unsigned char *) destination->data;

	scale = exp2(1.0 / 40.0);
	mask = 0xff;

	value = scale * sbuffer[soffset];
      
	dbuffer[doffset * 3] = mask & value;
	dbuffer[doffset * 3 + 1] = (mask << 8) & value;
	dbuffer[doffset * 3 + 2] = (mask << 16) & value;
	
	if(sbuffer[soffset] < 0){
	  dbuffer[doffset * 3 + 2] |= 0x80;
	}
      }
      break;
    case AGS_SOUNDCARD_SIGNED_32_BIT:
      {
	gint32 *dbuffer;
	gdouble scale;

	dbuffer = (gint32 *) destination->data;

	scale = exp2(1 / 32.0);

	dbuffer[doffset] = scale * sbuffer[soffset];
      }
      break;
    case AGS_SOUNDCARD_SIGNED_64_BIT:
      {
	gint64 *dbuffer;

	dbuffer = (gint64 *) destination->data;

	dbuffer[doffset] = sbuffer[soffset];
      }
      break;
    }
  }

  source = template->stream_beginning;

  if(template->samplerate < audio_signal->samplerate){
    expand = TRUE;
  }else{
    expand = FALSE;
  }

  scale_factor = 1.0 / template->length * length;
  morph_factor = 1.0 / template->format * audio_signal->format;

  /* prepare destination */
  ags_audio_signal_stream_resize(audio_signal, length);

  /* create audio data */
  //TODO:JK: fix me
  j_stop = audio_signal->format; // lcm(audio_signal->format, template->format);

  stream_template = NULL;

  offset = 0;
  step = 0.0;

  for(i = 0; i < template->length; i++){
    for(; j < j_stop; j++){

      if(offset == audio_signal->buffer_size && step >= morph_factor){
	break;
      }

      if(offset == 0){
	if(expand){
	  data = (gpointer) malloc(sizeof(audio_signal->buffer_size * morph_factor));
	}else{
	  data = (gpointer) malloc(sizeof(audio_signal->buffer_size / morph_factor));
	}

	stream_template = g_list_prepend(stream_template,
					 data);

	destination = stream_template;
      }

      switch(template->format){
      case AGS_SOUNDCARD_SIGNED_8_BIT:
	{
	  ags_audio_signal_scale_copy_8_bit(destination, source,
					    j, offset,
					    audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_16_BIT:
	{
	  ags_audio_signal_scale_copy_16_bit(destination, source,
					     j, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_24_BIT:
	{
	  ags_audio_signal_scale_copy_24_bit(destination, source,
					     j, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_32_BIT:
	{
	  ags_audio_signal_scale_copy_32_bit(destination, source,
					     j, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_64_BIT:
	{
	  ags_audio_signal_scale_copy_64_bit(destination, source,
					     j, offset,
					     audio_signal->format);
	}
	break;
      }

      step += (1 / morph_factor);

      if(step >= morph_factor){
	step = 0.0;
	offset++;
      }      
    }


    if(j == j_stop){
      j = 0;
    }

    if(offset == template->buffer_size && step == 0.0){
      offset = 0;
      source = source->next;
    }
  }

  stream_template = g_list_reverse(stream_template);

  /* morph */
  //TODO:JK: implement me

  /* scale */
  source = stream_template;
  destination = audio_signal->stream_beginning;

  offset = 0;
  k = 0;
  template_k = 0;

  while(destination != NULL){
    for(i = 0;
	i < audio_signal->buffer_size &&
	  k < audio_signal->buffer_size &&
	  template_k < template->buffer_size;
	i++, k++, template_k++){

      switch(audio_signal->format){
      case AGS_SOUNDCARD_SIGNED_8_BIT:
	{
	  ags_audio_signal_scale_copy_8_bit(destination, source,
					    i, offset,
					    audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_16_BIT:
	{
	  ags_audio_signal_scale_copy_16_bit(destination, source,
					     i, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_24_BIT:
	{
	  ags_audio_signal_scale_copy_24_bit(destination, source,
					     i, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_32_BIT:
	{
	  ags_audio_signal_scale_copy_32_bit(destination, source,
					     i, offset,
					     audio_signal->format);
	}
	break;
      case AGS_SOUNDCARD_SIGNED_64_BIT:
	{
	  ags_audio_signal_scale_copy_64_bit(destination, source,
					     i, offset,
					     audio_signal->format);
	}
	break;
      }

      offset = (guint) floor(morph_factor * (double) i);
    }

    if(k == audio_signal->buffer_size){
      destination = destination->next;
    }

    if(template_k == template->buffer_size){
      source = source->next;
    }
  }
}

/**
 * ags_audio_signal_feed:
 * @audio_signal: the #AgsAudioSignal
 * @template: the template #AgsAudioSignal
 * @frame_count: the new frame count
 * 
 * Feed audio signal to grow upto frame count.
 * 
 * Since: 1.0.0
 */
void
ags_audio_signal_feed(AgsAudioSignal *audio_signal,
		      AgsAudioSignal *template,
		      guint frame_count)
{
  GList *stream, *template_stream;

  guint old_length;
  guint old_last_frame;
  guint old_frame_count;
  gdouble delay;
  guint attack;
  guint loop_length;
  guint loop_frame_count;
  guint n_frames;
  guint copy_n_frames;
  guint nth_loop;
  guint i, j, k;
  guint copy_mode;

  if(audio_signal == NULL ||
     template == NULL){
    return;
  }
  
  old_length = audio_signal->length;
  old_last_frame = audio_signal->last_frame;
  old_frame_count = old_last_frame + (old_length * audio_signal->buffer_size) - audio_signal->first_frame;
  
  /* resize */
  if(template->loop_end > template->loop_start){
    loop_length = template->loop_end - template->loop_start;
    loop_frame_count = ((frame_count - template->loop_start) / loop_length) * template->buffer_size;

    ags_audio_signal_stream_resize(audio_signal,
				   (guint) ceil(frame_count / audio_signal->buffer_size) + 1);
    
  }else{
    ags_audio_signal_stream_resize(audio_signal,
				   (guint) ceil(frame_count / audio_signal->buffer_size) + 1);

    return;
  }

    audio_signal->last_frame = ((guint) (delay * audio_signal->buffer_size) + frame_count + attack) % audio_signal->buffer_size;

  if(template->length == 0){
    return;
  }

  delay = audio_signal->delay;
  attack = audio_signal->attack;

  /* generic copying */
  stream = g_list_nth(audio_signal->stream_beginning,
		      (guint) ((delay * audio_signal->buffer_size) + attack) / audio_signal->buffer_size);
  template_stream = template->stream_beginning;

  /* loop related copying */
  copy_mode = ags_audio_buffer_util_get_copy_mode(ags_audio_buffer_util_format_from_soundcard(audio_signal->format),
						  ags_audio_buffer_util_format_from_soundcard(template->format));
  
  for(i = 0, j = 0, k = attack, nth_loop = 0; i < frame_count;){    
    /* compute count of frames to copy */
    copy_n_frames = audio_signal->buffer_size;

    /* limit nth loop */
    if(i > template->loop_start &&
       i + copy_n_frames > template->loop_start + loop_length &&
       i + copy_n_frames < template->loop_start + loop_frame_count &&
       i + copy_n_frames >= template->loop_start + (nth_loop + 1) * loop_length){
      copy_n_frames = (template->loop_start + (nth_loop + 1) * loop_length) - i;
    }

    /* check boundaries */
    if((k % audio_signal->buffer_size) + copy_n_frames > audio_signal->buffer_size){
      copy_n_frames = audio_signal->buffer_size - (k % audio_signal->buffer_size);
    }

    if(j + copy_n_frames > audio_signal->buffer_size){
      copy_n_frames = audio_signal->buffer_size - j;
    }

    if(stream == NULL ||
       template_stream == NULL){
      break;
    }
    
    /* copy */
    ags_audio_buffer_util_copy_buffer_to_buffer(stream->data, 1, k % audio_signal->buffer_size,
						template_stream->data, 1, j,
						copy_n_frames, copy_mode);
    
    /* increment and iterate */
    if((i + copy_n_frames) % audio_signal->buffer_size == 0){
      stream = stream->next;
    }

    if(j + copy_n_frames == template->buffer_size){
      template_stream = template_stream->next;
    }
    
    if(template_stream == NULL ||
       (i > template->loop_start &&
	i + copy_n_frames > template->loop_start + loop_length &&
	i + copy_n_frames < template->loop_start + loop_frame_count &&
	i + copy_n_frames >= template->loop_start + (nth_loop + 1) * loop_length)){
      j = template->loop_start % template->buffer_size;
      template_stream = g_list_nth(template->stream_beginning,
				   floor(template->loop_start / template->buffer_size));

      nth_loop++;
    }else{
      j += copy_n_frames;
    }
    
    i += copy_n_frames;
    k += copy_n_frames;

    if(j == template->buffer_size){
      j = 0;
    }
  }
}

/**
 * ags_audio_signal_envelope:
 * @audio_signal: the #AgsAudioSignal
 * @attack: the attack
 * @decay: the decay
 * @sustain: the sustain
 * @release: the release
 * @ratio: the ratio
 *
 * Envelope audio signal.
 *
 * Since: 0.6.7
 */
void
ags_audio_signal_envelope(AgsAudioSignal *audio_signal,
			  gdouble attack,
			  gdouble decay,
			  gdouble sustain,
			  gdouble release,
			  gdouble ratio)
{
  GList *stream;
  
  signed short *buffer;
  gdouble volume;
  guint frame_count, current_frame_count;
  guint i, j, k;
  
  if(ratio == 0.0){
    return;
  }

  stream = audio_signal->stream_beginning;
  buffer = stream->data;
  
  frame_count = audio_signal->buffer_size * audio_signal->length - (audio_signal->buffer_size - audio_signal->last_frame);

  for(j = 0, k = 0; j < 4; j++){
    for(i = 0; ratio * (k + i) < (1.0 / ratio) * (frame_count / 4.0); i++, k++){
      if(k != 0 && k % audio_signal->buffer_size == 0){
	stream = stream->next;
	buffer = stream->data;
      }

      switch(j){
      case 0:
	{
	  volume = (1.0 / (k - i)) / (attack / (k - i));
	}
	break;
      case 1:
	{
	  volume = (1.0 / (k - i)) / (decay / (k - i));
	}
	break;
      case 2:
	{
	  volume = (1.0 / (k - i)) / (sustain / (k - i));
	}
	break;
      case 3:
	{
	  volume = (1.0 / (k - i)) / (release / (k - i));
	}
	break;
      }

      *buffer = (signed short) (volume * buffer[0]);
      buffer++;
    }
  }
}

/**
 * ags_audio_signal_is_active:
 * @audio_signal: the #GList-struct containing #AgsAudioSignal
 * @recall_id: the #AgsRecallID
 * 
 * Check if is active.
 * 
 * Returns: %TRUE if related audio signal to recall id is available, otherwise %FALSE
 * 
 * Since: 0.7.122.9
 */
gboolean
ags_audio_signal_is_active(GList *audio_signal,
			   GObject *recall_id)
{
  AgsAudioSignal *current;
  AgsRecyclingContext *recycling_context;
  
  if(recall_id == NULL ||
     AGS_RECALL_ID(recall_id)->recycling_context == NULL){
    return(FALSE);
  }
  
  recycling_context = AGS_RECALL_ID(recall_id)->recycling_context;
  
  while(audio_signal != NULL){
    current = AGS_AUDIO_SIGNAL(audio_signal->data);
    
    if(current->recall_id != NULL &&
       AGS_RECALL_ID(current->recall_id)->recycling_context == recycling_context){
      return(TRUE);
    }
    
    audio_signal = audio_signal->next;
  }
  
  return(FALSE);
}

/**
 * ags_audio_signal_new:
 * @soundcard: the assigned #AgsSoundcard
 * @recycling: the #AgsRecycling
 * @recall_id: the #AgsRecallID, it can be NULL if %AGS_AUDIO_SIGNAL_TEMPLATE is set
 *
 * Creates a #AgsAudioSignal, with defaults of @soundcard, linking @recycling tree
 * and refering to @recall_id.
 *
 * Returns: a new #AgsAudioSignal
 *
 * Since: 0.3
 */
AgsAudioSignal*
ags_audio_signal_new(GObject *soundcard,
		     GObject *recycling,
		     GObject *recall_id)
{
  AgsAudioSignal *audio_signal;

  audio_signal = (AgsAudioSignal *) g_object_new(AGS_TYPE_AUDIO_SIGNAL,
						 "soundcard\0", soundcard,
						 "recycling\0", recycling,
						 "recall-id\0", recall_id,
						 NULL);

  return(audio_signal);
}

/**
 * ags_audio_signal_new_with_length:
 * @soundcard: the assigned #AgsSoundcard
 * @recycling: the #AgsRecycling
 * @recall_id: the #AgsRecallID, it can be NULL if %AGS_AUDIO_SIGNAL_TEMPLATE is set
 * @length: audio data frame count
 *
 * Creates a #AgsAudioSignal, with defaults of @soundcard, linking @recycling tree
 * and refering to @recall_id.
 * The audio data is tiled to @length frame count.
 *
 * Returns: a new #AgsAudioSignal
 *
 * Since: 0.4
 */
AgsAudioSignal*
ags_audio_signal_new_with_length(GObject *soundcard,
				 GObject *recycling,
				 GObject *recall_id,
				 guint length)
{
  AgsAudioSignal *audio_signal, *template;

  audio_signal = (AgsAudioSignal *) g_object_new(AGS_TYPE_AUDIO_SIGNAL,
						 "soundcard\0", soundcard,
						 "recycling\0", recycling,
						 "recall-id\0", recall_id,
						 NULL);

  template = ags_audio_signal_get_template(AGS_RECYCLING(recycling)->audio_signal);

  if(template != NULL){
    ags_audio_signal_tile(audio_signal,
			  template,
			  length);
  }

  return(audio_signal);
}
