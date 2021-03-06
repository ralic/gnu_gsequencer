/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#include <ags/audio/ags_note.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/midi/ags_midi_buffer_util.h>

#include <stdlib.h>

void ags_note_class_init(AgsNoteClass *note);
void ags_note_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_note_init(AgsNote *note);
void ags_note_set_property(GObject *gobject,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *param_spec);
void ags_note_get_property(GObject *gobject,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *param_spec);
void ags_note_connect(AgsConnectable *connectable);
void ags_note_disconnect(AgsConnectable *connectable);
void ags_note_finalize(GObject *gobject);

/**
 * SECTION:ags_note
 * @short_description: Note class.
 * @title: AgsNote
 * @section_id:
 * @include: ags/audio/ags_note.h
 *
 * #AgsNote represents a tone.
 */

static gpointer ags_note_parent_class = NULL;

enum{
  PROP_0,
  PROP_X0,
  PROP_X1,
  PROP_Y,
  PROP_STREAM_DELAY,
  PROP_STREAM_ATTACK,
  PROP_ATTACK,
  PROP_DECAY,
  PROP_SUSTAIN,
  PROP_RELEASE,
  PROP_RATIO,
};

GType
ags_note_get_type()
{
  static GType ags_type_note = 0;

  if(!ags_type_note){
    static const GTypeInfo ags_note_info = {
      sizeof(AgsNoteClass),
      NULL,
      NULL,
      (GClassInitFunc) ags_note_class_init,
      NULL,
      NULL,
      sizeof(AgsNote),
      0,
      (GInstanceInitFunc) ags_note_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_note_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_note = g_type_register_static(G_TYPE_OBJECT,
					   "AgsNote\0",
					   &ags_note_info,
					   0);
    
    g_type_add_interface_static(ags_type_note,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_note);
}

void 
ags_note_class_init(AgsNoteClass *note)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_note_parent_class = g_type_class_peek_parent(note);

  gobject = (GObjectClass *) note;

  gobject->set_property = ags_note_set_property;
  gobject->get_property = ags_note_get_property;

  gobject->finalize = ags_note_finalize;

  /**
   * AgsNote:x0:
   *
   * Note offset x0.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("x0\0",
				 "offset x0\0",
				 "The first x offset\0",
				 0,
				 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_X0,
				  param_spec);

  /**
   * AgsNote:x1:
   *
   * Note offset x1.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("x1\0",
				 "offset x1\0",
				 "The last x offset\0",
				 0,
				 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_X1,
				  param_spec);

  /**
   * AgsNote:y:
   *
   * Note offset y.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("y\0",
				 "offset y\0",
				 "The y offset\0",
				 0,
				 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_Y,
				  param_spec);

  /**
   * AgsNote:stream-delay:
   *
   * The stream's delay.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_double("stream-delay\0",
				   "delay of stream\0",
				   "The delay of the stream\0",
				   0.0,
				   65535.0,
				   0.0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM_DELAY,
				  param_spec);

  /**
   * AgsNote:stream-attack:
   *
   * The stream's attack.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_uint("stream-attack\0",
				 "offset stream-attack\0",
				 "The first x offset\0",
				 0,
				 65535,
				 0,
				 G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM_ATTACK,
				  param_spec);

  /**
   * AgsNote:attack:
   *
   * Envelope attack.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_boxed("attack\0",
				  "envelope's attack\0",
				  "The envelope's attack\0",
				  AGS_TYPE_COMPLEX,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_ATTACK,
				  param_spec);

  /**
   * AgsNote:decay:
   *
   * Envelope decay.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_boxed("decay\0",
				  "envelope's decay\0",
				  "The envelope's decay\0",
				  AGS_TYPE_COMPLEX,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DECAY,
				  param_spec);

  /**
   * AgsNote:sustain:
   *
   * Envelope sustain.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_boxed("sustain\0",
				  "envelope's sustain\0",
				  "The envelope's sustain\0",
				  AGS_TYPE_COMPLEX,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SUSTAIN,
				  param_spec);

  /**
   * AgsNote:release:
   *
   * Envelope release.
   * 
   * Since: 0.7.2
   */
  param_spec = g_param_spec_boxed("release\0",
				  "envelope's release\0",
				  "The envelope's release\0",
				  AGS_TYPE_COMPLEX,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RELEASE,
				  param_spec);

  /**
   * AgsNote:ratio:
   *
   * Envelope ratio.
   * 
   * Since: 0.7.42
   */
  param_spec = g_param_spec_boxed("ratio\0",
				  "envelope's ratio\0",
				  "The envelope's ratio\0",
				  AGS_TYPE_COMPLEX,
				  G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RATIO,
				  param_spec);
}

void
ags_note_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_note_connect;
  connectable->disconnect = ags_note_disconnect;
}

void
ags_note_init(AgsNote *note)
{
  note->flags = 0;

  note->x[0] = 0;
  note->x[1] = 0;
  note->y = 0;

  note->stream_delay = 0.0;
  note->stream_attack = 0;
  
  ags_complex_set(&(note->attack),
		  1.0);
  ags_complex_set(&(note->decay),
		  1.0);
  ags_complex_set(&(note->sustain),
		  1.0);  
  ags_complex_set(&(note->release),
		  1.0);

  ags_complex_set(&(note->ratio),
		  1.0);
  
  note->name = NULL;
  note->frequency = 440.0;
}

void
ags_note_connect(AgsConnectable *connectable)
{
  AgsNote *note;

  note = AGS_NOTE(connectable);

  if((AGS_NOTE_CONNECTED & (note->flags)) != 0){
    return;
  }

  note->flags |= AGS_NOTE_CONNECTED;
}

void
ags_note_disconnect(AgsConnectable *connectable)
{
  AgsNote *note;

  note = AGS_NOTE(connectable);

  if((AGS_NOTE_CONNECTED & (note->flags)) == 0){
    return;
  }

  note->flags &= (~AGS_NOTE_CONNECTED);
}

void
ags_note_set_property(GObject *gobject,
		      guint prop_id,
		      const GValue *value,
		      GParamSpec *param_spec)
{
  AgsNote *note;

  note = AGS_NOTE(gobject);

  switch(prop_id){
  case PROP_X0:
    {
      note->x[0] = g_value_get_uint(value);
    }
    break;
  case PROP_X1:
    {
      note->x[1] = g_value_get_uint(value);
    }
    break;
  case PROP_Y:
    {
      note->y = g_value_get_uint(value);
    }
    break;
  case PROP_STREAM_DELAY:
    {
      note->stream_delay = g_value_get_double(value);
    }
    break;
  case PROP_STREAM_ATTACK:
    {
      note->stream_attack = g_value_get_uint(value);
    }
    break;
  case PROP_ATTACK:
    {
      AgsComplex *attack;

      attack = (AgsComplex *) g_value_get_boxed(value);

      ags_complex_set(&(note->attack),
		      ags_complex_get(attack));
    }
    break;
  case PROP_SUSTAIN:
    {
      AgsComplex *sustain;

      sustain = (AgsComplex *) g_value_get_boxed(value);

      ags_complex_set(&(note->sustain),
		      ags_complex_get(sustain));
    }
    break;
  case PROP_DECAY:
    {
      AgsComplex *decay;

      decay = (AgsComplex *) g_value_get_boxed(value);

      ags_complex_set(&(note->decay),
		      ags_complex_get(decay));
    }
    break;
  case PROP_RELEASE:
    {
      AgsComplex *release;

      release = (AgsComplex *) g_value_get_boxed(value);

      ags_complex_set(&(note->release),
		      ags_complex_get(release));
    }
    break;
  case PROP_RATIO:
    {
      AgsComplex *ratio;

      ratio = (AgsComplex *) g_value_get_boxed(value);

      ags_complex_set(&(note->ratio),
		      ags_complex_get(ratio));
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_note_get_property(GObject *gobject,
		      guint prop_id,
		      GValue *value,
		      GParamSpec *param_spec)
{
  AgsNote *note;

  note = AGS_NOTE(gobject);

  switch(prop_id){
  case PROP_X0:
    g_value_set_uint(value, note->x[0]);
    break;
  case PROP_X1:
    g_value_set_uint(value, note->x[1]);
    break;
  case PROP_Y:
    g_value_set_uint(value, note->y);
    break;
  case PROP_STREAM_DELAY:
    g_value_set_double(value, note->stream_delay);
    break;
  case PROP_STREAM_ATTACK:
    g_value_set_uint(value, note->stream_attack);
    break;
  case PROP_ATTACK:
    g_value_set_boxed(value, note->attack);
    break;
  case PROP_SUSTAIN:
    g_value_set_boxed(value, note->sustain);
    break;
  case PROP_DECAY:
    g_value_set_boxed(value, note->decay);
    break;
  case PROP_RELEASE:
    g_value_set_boxed(value, note->release);
    break;
  case PROP_RATIO:
    g_value_set_boxed(value, note->ratio);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_note_finalize(GObject *gobject)
{
  AgsNote *note;

  note = AGS_NOTE(gobject);

  /* name */
  if(note->name != NULL){
    free(note->name);
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_note_parent_class)->finalize(gobject);
}

/**
 * ags_note_find_prev:
 * @note: a #GList containing #AgsNote
 * @x0: x offset
 * @y:  y offset
 * 
 * Find prev note.
 *
 * Returns: the matching entry as #GList.
 *
 * Since: 0.7.2
 */
GList*
ags_note_find_prev(GList *note,
		   guint x0, guint y)
{
  GList *current_match;

  current_match = NULL;

  while(note != NULL){
    if(AGS_NOTE(note->data)->y == y){
      current_match = note;

      note = note->next;
      continue;
    }

    if(AGS_NOTE(note->data)->x[0] > x0){
      return(current_match);
    }

    note = note->next;
  }

  return(current_match);
}

/**
 * ags_note_find_next:
 * @note: a #GList containing #AgsNote
 * @x0: x offset
 * @y:  y offset
 * 
 * Find next note.
 *
 * Returns: the matching entry as #GList.
 *
 * Since: 0.7.2
 */
GList*
ags_note_find_next(GList *note,
		   guint x0, guint y)
{
  GList *current_match;

  note = g_list_last(note);
  current_match = NULL;

  while(note != NULL){
    if(AGS_NOTE(note->data)->y == y){
      current_match = note;

      note = note->prev;
      continue;
    }

    if(AGS_NOTE(note->data)->x[0] < x0){
      return(current_match);
    }

    note = note->prev;
  }

  return(current_match);
}

/**
 * ags_note_to_seq_event:
 * @note: the #AgsNote
 * @bpm: the bpm to use
 * @delay_factor: the segmentation delay factor
 * @buffer_length: the length of the returned buffer
 * 
 * Convert @note to raw MIDI and set the buffer length of returned bytes
 * in the array as @buffer_length.
 *
 * Returns: The sequencer raw midi as array.
 *
 * Since: 0.7.1
 */
unsigned char*
ags_note_to_raw_midi(AgsNote *note,
		     gdouble bpm, gdouble delay_factor,
		     guint *buffer_length)
{
  unsigned char *raw_midi;
  guint length;
  guint current_length;
  long delta_time;
  guint delta_time_length;
  unsigned char status;
  int channel;
  int key;
  int velocity;
  int pressure;
  gdouble ticks_per_beat;
  guint i, i_stop;
  guint j;
  guint k;
  
  if(note == NULL){
    if(buffer_length != NULL){
      *buffer_length = 0;
    }
    
    return(NULL);
  }

  length = 0;

  /* key-on */
  k = 0;
  
  /* delta-time */
  delta_time = note->x[0] / 16.0 / bpm * 60.0 / ((USEC_PER_SEC * bpm / 4.0) / (4.0 * bpm) / USEC_PER_SEC);
  delta_time_length = 
    current_length = ags_midi_buffer_util_get_varlength_size(delta_time);

  /* status and channel */
  channel = 0;
  status = (0x90 | (0x7f & channel));
  current_length++;

  /* note / key */
  key = (0x7f & (note->y));
  current_length++;

  /* velocity */
  velocity = (0x7f & (unsigned char) (128 * (ags_complex_get(&(note->attack)))));
  current_length++;

  /* prepare buffer */
  raw_midi = (unsigned char *) malloc(current_length * sizeof(unsigned char));
  length += current_length;

  ags_midi_buffer_util_put_varlength(raw_midi,
				     delta_time);
  k += delta_time_length;
  
  raw_midi[k] = status;
  raw_midi[k + 1] = key;
  raw_midi[k + 2] = velocity;

  k += 3;

  /* key-pressure */
  ticks_per_beat = AGS_NOTE_DEFAULT_TICKS_PER_QUARTER_NOTE / 4.0 / delay_factor;

  if(ticks_per_beat > 2.0){
    i_stop = (note->x[1] - note->x[0]) * (ticks_per_beat - 2.0);

    for(i = 1; i <= i_stop; i++){
      /* delta-time */
      delta_time = (note->x[0] + i + 1)  / 16.0 / bpm * 60.0 / ((USEC_PER_SEC * bpm / 4.0) / (4.0 * bpm) / USEC_PER_SEC);
      delta_time_length = 
	current_length = ags_midi_buffer_util_get_varlength_size(delta_time);

      /* status and channel */
      channel = 0;
      status = (0x90 | (0x7f & channel));
      current_length++;

      /* note / key */
      key = (0x7f & (note->y));
      current_length++;

      /* pressure */
      //TODO:JK: verify
      pressure = (0x7f & (unsigned char) (128 * (((ags_complex_get(&(note->decay)) / i) - (i * ags_complex_get(&(note->sustain)))))));
      current_length++;

      /* prepare buffer */
      raw_midi = (unsigned char *) realloc(raw_midi,
					   current_length * sizeof(unsigned char));
      length += current_length;

      ags_midi_buffer_util_put_varlength(raw_midi,
					 delta_time);
      k += delta_time_length;
  
      raw_midi[k] = status;
      raw_midi[k + 1] = key;
      raw_midi[k + 2] = pressure;

      k += 3;
    }
  }

  /* key-off */
  /* delta-time */
  delta_time = note->x[1] / 16.0 / bpm * 60.0 / ((USEC_PER_SEC * bpm / 4.0) / (4.0 * bpm) / USEC_PER_SEC);
  delta_time_length = 
    current_length = ags_midi_buffer_util_get_varlength_size(delta_time);

  /* status and channel */
  channel = 0;
  status = (0x90 | (0x7f & channel));
  current_length++;

  /* note / key */
  key = (0x7f & (note->y));
  current_length++;

  /* velocity */
  velocity = (0x7f & (unsigned char) (128 * (ags_complex_get(&(note->attack)))));
  current_length++;

  /* prepare buffer */
  raw_midi = (unsigned char *) realloc(raw_midi,
				       current_length * sizeof(unsigned char));
  length += current_length;

  ags_midi_buffer_util_put_varlength(raw_midi,
				     delta_time);
  k += delta_time_length;
  
  raw_midi[k] = status;
  raw_midi[k + 1] = key;
  raw_midi[k + 2] = velocity;

  /* return value */
  if(buffer_length != NULL){
    *buffer_length = length;
  }
  
  return(raw_midi);
}

/**
 * ags_note_to_seq_event:
 * @note: the #AgsNote
 * @bpm: the bpm to use
 * @delay_factor: the segmentation delay factor
 * @n_events: the count of events
 * 
 * Convert @note to ALSA sequencer events and set the number of events
 * in the array as @n_events.
 *
 * Returns: The sequencer events as array.
 *
 * Since: 0.7.1
 */
snd_seq_event_t*
ags_note_to_seq_event(AgsNote *note,
		      gdouble bpm, gdouble delay_factor,
		      guint *n_events)
{
  snd_seq_event_t *event;


  
  event = NULL;
  
  //TODO:JK: implement me

  return(event);
}

/**
 * ags_note_from_raw_midi:
 * @raw_midi: the data array
 * @bpm: the bpm to use
 * @delay_factor: the segmentation delay factor
 * @length: the length of the array
 *
 * Parse @raw_midi data and convert to #AgsNote.
 *
 * Returns: a #GList containing the notes
 *
 * Since: 0.7.1
 */
GList*
ags_note_from_raw_midi(unsigned char *raw_midi,
		       gdouble bpm, gdouble delay_factor,
		       guint length)
{
  GList *list;

  list = NULL;

  //TODO:JK: implement me
  
  return(list);
}

/**
 * ags_note_from_seq_event:
 * @event: ALSA sequencer events as array
 * @bpm: the bpm to use
 * @delay_factor: the segmentation delay factor
 * @n_events: the arrays length
 *
 * Convert ALSA sequencer data @event to #AgsNote.
 *
 * Returns: a #GList containing the notes
 *
 * Since: 0.7.1
 */
GList*
ags_note_from_seq_event(snd_seq_event_t *event,
			gdouble bpm, gdouble delay_factor,
			guint n_events)
{
  GList *list;

  list = NULL;

  //TODO:JK: implement me
  
  return(list);
}

/**
 * ags_note_duplicate:
 * @note: an #AgsNote
 * 
 * Duplicate a note.
 *
 * Returns: the duplicated #AgsNote.
 *
 * Since: 0.4
 */
AgsNote*
ags_note_duplicate(AgsNote *note)
{
  AgsNote *copy;

  if(note->x[0] == note->x[1]){
    return(NULL);
  }
  
  copy = ags_note_new();

  copy->flags = 0;

  if(note->x[0] < note->x[1]){
    copy->x[0] = note->x[0];
    copy->x[1] = note->x[1];
  }else{
    copy->x[0] = note->x[1];
    copy->x[1] = note->x[0];
  }
  
  copy->y = note->y;

  copy->stream_delay = note->stream_delay;
  copy->stream_attack = note->stream_attack;
  
  copy->attack[0] = note->attack[0];
  copy->attack[1] = note->attack[1];

  copy->decay[0] = note->decay[0];
  copy->decay[1] = note->decay[1];

  copy->sustain[0] = note->sustain[0];
  copy->sustain[1] = note->sustain[1];

  copy->release[0] = note->release[0];
  copy->release[1] = note->release[1];

  copy->ratio[0] = note->ratio[0];
  copy->ratio[1] = note->ratio[1];

  return(copy);
}

/**
 * ags_note_new:
 *
 * Creates an #AgsNote
 *
 * Returns: a new #AgsNote
 *
 * Since: 0.4
 */
AgsNote*
ags_note_new()
{
  AgsNote *note;

  note = (AgsNote *) g_object_new(AGS_TYPE_NOTE, NULL);

  return(note);
}

/**
 * ags_note_new:
 *
 * Creates an #AgsNote
 *
 * Returns: a new #AgsNote
 *
 * Since: 0.7.2
 */
AgsNote*
ags_note_new_with_offset(guint x0, guint x1,
			 guint y,
			 gdouble stream_delay, guint stream_attack)
{
  AgsNote *note;

  note = (AgsNote *) g_object_new(AGS_TYPE_NOTE,
				  "x0\0", x0,
				  "x1\0", x1,
				  "y\0", y,
				  "stream-delay\0", stream_delay,
				  "stream-attack\0", stream_attack,
				  NULL);

  return(note);
}

