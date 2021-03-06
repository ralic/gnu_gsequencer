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

#include <ags/audio/task/recall/ags_apply_sequencer_length.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_tactable.h>

#include <ags/object/ags_soundcard.h>
#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall.h>

#include <math.h>

void ags_apply_sequencer_length_class_init(AgsApplySequencerLengthClass *apply_sequencer_length);
void ags_apply_sequencer_length_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_apply_sequencer_length_init(AgsApplySequencerLength *apply_sequencer_length);
void ags_apply_sequencer_length_set_property(GObject *gobject,
					     guint prop_id,
					     const GValue *value,
					     GParamSpec *param_spec);
void ags_apply_sequencer_length_get_property(GObject *gobject,
					     guint prop_id,
					     GValue *value,
					     GParamSpec *param_spec);
void ags_apply_sequencer_length_connect(AgsConnectable *connectable);
void ags_apply_sequencer_length_disconnect(AgsConnectable *connectable);
void ags_apply_sequencer_length_finalize(GObject *gobject);

void ags_apply_sequencer_length_recall(AgsApplySequencerLength *apply_sequencer_length, AgsRecall *recall);
void ags_apply_sequencer_length_channel(AgsApplySequencerLength *apply_sequencer_length, AgsChannel *channel);
void ags_apply_sequencer_length_audio(AgsApplySequencerLength *apply_sequencer_length, AgsAudio *audio);

void ags_apply_sequencer_length_launch(AgsTask *task);

/**
 * SECTION:ags_apply_sequencer_length
 * @short_description: apply sequencer length to delay audio
 * @title: AgsApplySequencerLength
 * @section_id:
 * @include: ags/audio/task/recall/ags_apply_sequencer_length.h
 *
 * The #AgsApplySequencerLength task applys sequencer length to #AgsDelayAudio.
 */

enum{
  PROP_0,
  PROP_SCOPE,
  PROP_SEQUENCER_LENGTH,
};

static gpointer ags_apply_sequencer_length_parent_class = NULL;
static AgsConnectableInterface *ags_apply_sequencer_length_parent_connectable_interface;

GType
ags_apply_sequencer_length_get_type()
{
  static GType ags_type_apply_sequencer_length = 0;

  if(!ags_type_apply_sequencer_length){
    static const GTypeInfo ags_apply_sequencer_length_info = {
      sizeof (AgsApplySequencerLengthClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_apply_sequencer_length_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsApplySequencerLength),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_apply_sequencer_length_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_apply_sequencer_length_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_apply_sequencer_length = g_type_register_static(AGS_TYPE_TASK,
							     "AgsApplySequencerLength\0",
							     &ags_apply_sequencer_length_info,
							     0);
    
    g_type_add_interface_static(ags_type_apply_sequencer_length,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_apply_sequencer_length);
}

void
ags_apply_sequencer_length_class_init(AgsApplySequencerLengthClass *apply_sequencer_length)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_apply_sequencer_length_parent_class = g_type_class_peek_parent(apply_sequencer_length);

  /* GObjectClass */
  gobject = (GObjectClass *) apply_sequencer_length;

  gobject->set_property = ags_apply_sequencer_length_set_property;
  gobject->get_property = ags_apply_sequencer_length_get_property;

  gobject->finalize = ags_apply_sequencer_length_finalize;

  /* properties */
  /**
   * AgsApplySequencerLength:scope:
   *
   * The assigned #GObject as scope.
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("scope\0",
				   "scope of set buffer size\0",
				   "The scope of set buffer size\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SCOPE,
				  param_spec);

  /**
   * AgsApplySequencerLength:sequencer-length:
   *
   * The sequencer length to apply to scope.
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_double("sequencer-length\0",
				   "sequencer length\0",
				   "The sequence length to apply\0",
				   0,
				   G_MAXDOUBLE,
				   0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SEQUENCER_LENGTH,
				  param_spec);

  /* AgsTaskClass */
  task = (AgsTaskClass *) apply_sequencer_length;
  
  task->launch = ags_apply_sequencer_length_launch;
}

void
ags_apply_sequencer_length_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_apply_sequencer_length_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_apply_sequencer_length_connect;
  connectable->disconnect = ags_apply_sequencer_length_disconnect;
}

void
ags_apply_sequencer_length_init(AgsApplySequencerLength *apply_sequencer_length)
{
  apply_sequencer_length->scope = NULL;
  apply_sequencer_length->length = 0.0;
}

void
ags_apply_sequencer_length_set_property(GObject *gobject,
					guint prop_id,
					const GValue *value,
					GParamSpec *param_spec)
{
  AgsApplySequencerLength *apply_sequencer_length;

  apply_sequencer_length = AGS_APPLY_SEQUENCER_LENGTH(gobject);

  switch(prop_id){
  case PROP_SCOPE:
    {
      GObject *scope;

      scope = (GObject *) g_value_get_object(value);

      if(apply_sequencer_length->scope == (GObject *) scope){
	return;
      }

      if(apply_sequencer_length->scope != NULL){
	g_object_unref(apply_sequencer_length->scope);
      }

      if(scope != NULL){
	g_object_ref(scope);
      }

      apply_sequencer_length->scope = (GObject *) scope;
    }
    break;
  case PROP_SEQUENCER_LENGTH:
    {
      apply_sequencer_length->length = g_value_get_double(value);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_apply_sequencer_length_get_property(GObject *gobject,
					guint prop_id,
					GValue *value,
					GParamSpec *param_spec)
{
  AgsApplySequencerLength *apply_sequencer_length;

  apply_sequencer_length = AGS_APPLY_SEQUENCER_LENGTH(gobject);

  switch(prop_id){
  case PROP_SCOPE:
    {
      g_value_set_object(value, apply_sequencer_length->scope);
    }
    break;
  case PROP_SEQUENCER_LENGTH:
    {
      g_value_set_double(value, apply_sequencer_length->length);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_apply_sequencer_length_connect(AgsConnectable *connectable)
{
  ags_apply_sequencer_length_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_apply_sequencer_length_disconnect(AgsConnectable *connectable)
{
  ags_apply_sequencer_length_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_apply_sequencer_length_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_apply_sequencer_length_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_apply_sequencer_length_launch(AgsTask *task)
{
  AgsApplySequencerLength *apply_sequencer_length;

  apply_sequencer_length = AGS_APPLY_SEQUENCER_LENGTH(task);

  if(AGS_IS_AUDIO(apply_sequencer_length->scope)){
    AgsAudio *audio;

    audio = AGS_AUDIO(apply_sequencer_length->scope);

    ags_apply_sequencer_length_audio(apply_sequencer_length, audio);
  }else if(AGS_IS_CHANNEL(apply_sequencer_length->scope)){
    AgsChannel *channel;

    channel = AGS_CHANNEL(apply_sequencer_length->scope);

    ags_apply_sequencer_length_channel(apply_sequencer_length, channel);
  }else if(AGS_IS_RECALL(apply_sequencer_length->scope)){
    AgsRecall *recall;

    recall = AGS_RECALL(apply_sequencer_length->scope);

    ags_apply_sequencer_length_recall(apply_sequencer_length, recall);
  }else{
    g_warning("AgsApplySequencerLength: Not supported scope\0");
  }
}

void
ags_apply_sequencer_length_recall(AgsApplySequencerLength *apply_sequencer_length, AgsRecall *recall)
{
  if(AGS_IS_TACTABLE(recall)){
    ags_tactable_change_sequencer_duration(AGS_TACTABLE(recall), apply_sequencer_length->length);
  }
}

void
ags_apply_sequencer_length_channel(AgsApplySequencerLength *apply_sequencer_length, AgsChannel *channel)
{
  GList *list;

  list = channel->play;

  while(list != NULL){
    ags_apply_sequencer_length_recall(apply_sequencer_length, AGS_RECALL(list->data));

    list = list->next;
  }

  list = channel->recall;

  while(list != NULL){
    ags_apply_sequencer_length_recall(apply_sequencer_length, AGS_RECALL(list->data));

    list = list->next;
  }
}

void
ags_apply_sequencer_length_audio(AgsApplySequencerLength *apply_sequencer_length, AgsAudio *audio)
{
  AgsChannel *channel;
  GList *list;

  /* AgsRecall */
  list = audio->play;

  while(list != NULL){
    ags_apply_sequencer_length_recall(apply_sequencer_length, AGS_RECALL(list->data));

    list = list->next;
  }

  list = audio->recall;

  while(list != NULL){
    ags_apply_sequencer_length_recall(apply_sequencer_length, AGS_RECALL(list->data));

    list = list->next;
  }

  /* AgsChannel */
  channel = audio->output;

  while(channel != NULL){
    ags_apply_sequencer_length_channel(apply_sequencer_length, channel);

    channel = channel->next;
  }

  channel = audio->input;

  while(channel != NULL){
    ags_apply_sequencer_length_channel(apply_sequencer_length, channel);

    channel = channel->next;
  }
}

/**
 * ags_apply_sequencer_length_new:
 * @scope: the #GObject
 * @length: the sequencer length to apply
 *
 * Creates an #AgsApplySequencerLength.
 *
 * Returns: an new #AgsApplySequencerLength.
 *
 * Since: 0.4
 */
AgsApplySequencerLength*
ags_apply_sequencer_length_new(GObject *scope,
			       gdouble length)
{
  AgsApplySequencerLength *apply_sequencer_length;

  apply_sequencer_length = (AgsApplySequencerLength *) g_object_new(AGS_TYPE_APPLY_SEQUENCER_LENGTH,
								    NULL);

  apply_sequencer_length->scope = scope;
  apply_sequencer_length->length = length;

  return(apply_sequencer_length);
}
