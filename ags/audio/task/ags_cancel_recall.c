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

#include <ags/audio/task/ags_cancel_recall.h>

#include <ags/object/ags_connectable.h>

void ags_cancel_recall_class_init(AgsCancelRecallClass *cancel_recall);
void ags_cancel_recall_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_cancel_recall_init(AgsCancelRecall *cancel_recall);
void ags_cancel_recall_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec);
void ags_cancel_recall_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec);
void ags_cancel_recall_connect(AgsConnectable *connectable);
void ags_cancel_recall_disconnect(AgsConnectable *connectable);
void ags_cancel_recall_finalize(GObject *gobject);

void ags_cancel_recall_launch(AgsTask *task);

/**
 * SECTION:ags_cancel_recall
 * @short_description: cancel recall object
 * @title: AgsCancelRecall
 * @section_id:
 * @include: ags/audio/task/ags_cancel_recall.h
 *
 * The #AgsCancelRecall task cancels #AgsRecall playback.
 */

static gpointer ags_cancel_recall_parent_class = NULL;
static AgsConnectableInterface *ags_cancel_recall_parent_connectable_interface;

enum{
  PROP_0,
  PROP_RECALL,
  PROP_PLAYBACK,
};

GType
ags_cancel_recall_get_type()
{
  static GType ags_type_cancel_recall = 0;

  if(!ags_type_cancel_recall){
    static const GTypeInfo ags_cancel_recall_info = {
      sizeof (AgsCancelRecallClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_cancel_recall_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsCancelRecall),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_cancel_recall_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_cancel_recall_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_cancel_recall = g_type_register_static(AGS_TYPE_TASK,
						    "AgsCancelRecall\0",
						    &ags_cancel_recall_info,
						    0);
    
    g_type_add_interface_static(ags_type_cancel_recall,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_cancel_recall);
}

void
ags_cancel_recall_class_init(AgsCancelRecallClass *cancel_recall)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_cancel_recall_parent_class = g_type_class_peek_parent(cancel_recall);

  /* gobject */
  gobject = (GObjectClass *) cancel_recall;

  gobject->set_property = ags_cancel_recall_set_property;
  gobject->get_property = ags_cancel_recall_get_property;

  gobject->finalize = ags_cancel_recall_finalize;

  /* properties */
  /**
   * AgsCancelRecall:recall:
   *
   * The assigned #AgsRecall
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("recall\0",
				   "recall of cancel recall\0",
				   "The recall of cancel recall task\0",
				   AGS_TYPE_RECALL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECALL,
				  param_spec);

  /**
   * AgsCancelRecall:playback:
   *
   * The assigned #AgsPlayback
   * 
   * Since: 0.7.117
   */
  param_spec = g_param_spec_object("playback\0",
				   "playback of cancel recall\0",
				   "The playback of cancel recall task\0",
				   AGS_TYPE_PLAYBACK,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAYBACK,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) cancel_recall;

  task->launch = ags_cancel_recall_launch;
}

void
ags_cancel_recall_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_cancel_recall_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_cancel_recall_connect;
  connectable->disconnect = ags_cancel_recall_disconnect;
}

void
ags_cancel_recall_init(AgsCancelRecall *cancel_recall)
{
  cancel_recall->recall = NULL;

  cancel_recall->playback = NULL;
}

void
ags_cancel_recall_set_property(GObject *gobject,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *param_spec)
{
  AgsCancelRecall *cancel_recall;

  cancel_recall = AGS_CANCEL_RECALL(gobject);

  switch(prop_id){
  case PROP_RECALL:
    {
      AgsRecall *recall;

      recall = (AgsRecall *) g_value_get_object(value);

      if(cancel_recall->recall == (GObject *) recall){
	return;
      }

      if(cancel_recall->recall != NULL){
	g_object_unref(cancel_recall->recall);
      }

      if(recall != NULL){
	g_object_ref(recall);
      }

      cancel_recall->recall = (GObject *) recall;
    }
    break;
  case PROP_PLAYBACK:
    {
      AgsPlayback *playback;

      playback = (AgsPlayback *) g_value_get_object(value);

      if(cancel_recall->playback == (GObject *) playback){
	return;
      }

      if(cancel_recall->playback != NULL){
	g_object_unref(cancel_recall->playback);
      }

      if(playback != NULL){
	g_object_ref(playback);
      }

      cancel_recall->playback = (GObject *) playback;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_cancel_recall_get_property(GObject *gobject,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *param_spec)
{
  AgsCancelRecall *cancel_recall;

  cancel_recall = AGS_CANCEL_RECALL(gobject);

  switch(prop_id){
  case PROP_RECALL:
    {
      g_value_set_object(value, cancel_recall->recall);
    }
    break;
  case PROP_PLAYBACK:
    {
      g_value_set_object(value, cancel_recall->playback);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_cancel_recall_connect(AgsConnectable *connectable)
{
  ags_cancel_recall_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_cancel_recall_disconnect(AgsConnectable *connectable)
{
  ags_cancel_recall_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_cancel_recall_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_cancel_recall_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_cancel_recall_launch(AgsTask *task)
{
  AgsRecall *recall;
  AgsCancelRecall *cancel_recall;

  cancel_recall = AGS_CANCEL_RECALL(task);

  recall = cancel_recall->recall;
  recall->flags |= AGS_RECALL_REMOVE;

  /* cancel AgsRecall */
  ags_recall_cancel(recall);

  /* set remove flag */
  if(cancel_recall->playback != NULL){
    g_atomic_int_or(&(cancel_recall->playback->flags),
		    AGS_PLAYBACK_REMOVE);
  }
}

/**
 * ags_cancel_recall_new:
 * @recall: the #AgsRecall to cancel
 * @playback: the #AgsPlayback
 *
 * Creates an #AgsCancelRecall.
 *
 * Returns: an new #AgsCancelRecall.
 *
 * Since: 0.4
 */
AgsCancelRecall*
ags_cancel_recall_new(AgsRecall *recall,
		      AgsPlayback *playback)
{
  AgsCancelRecall *cancel_recall;

  cancel_recall = (AgsCancelRecall *) g_object_new(AGS_TYPE_CANCEL_RECALL,
						   NULL);

  cancel_recall->recall = recall;

  cancel_recall->playback = playback;

  return(cancel_recall);
}
