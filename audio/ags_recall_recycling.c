/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ags/audio/ags_recall_recycling.h>

#include <ags/lib/ags_list.h>
#include <ags/lib/ags_parameter.h>

#include <ags/object/ags_marshal.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_run_connectable.h>

void ags_recall_recycling_class_init(AgsRecallRecyclingClass *recall_recycling);
void ags_recall_recyclingconnectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_recycling_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable);
void ags_recall_recycling_init(AgsRecallRecycling *recall_recycling);
void ags_recall_recycling_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_recall_recycling_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_recall_recycling_connect(AgsConnectable *connectable);
void ags_recall_recycling_disconnect(AgsConnectable *connectable);
void ags_recall_recycling_run_connect(AgsRunConnectable *run_connectable);
void ags_recall_recycling_run_disconnect(AgsRunConnectable *run_connectable);
void ags_recall_recycling_finalize(GObject *gobject);

AgsRecall* ags_recall_recycling_duplicate(AgsRecall *recall,
					  AgsRecallID *recall_id,
					  guint *n_params, GParameter *parameter);


void ags_recall_recycling_duplicate_list(GList *list,
					 GList **target);

enum{
  CREATED_RECALL_AUDIO_SIGNAL,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_DESTINATION,
  PROP_SOURCE,
  PROP_CHILD_DESTINATION,
  PROP_CHILD_SOURCE,
};

static gpointer ags_recall_recycling_parent_class = NULL;
static AgsConnectableInterface* ags_recall_recycling_parent_connectable_interface;
static AgsRunConnectableInterface *ags_recall_recycling_parent_run_connectable_interface;

static guint recall_recycling_signals[LAST_SIGNAL];

GType
ags_recall_recycling_get_type()
{
  static GType ags_type_recall_recycling = 0;

  if(!ags_type_recall_recycling){
    static const GTypeInfo ags_recall_recycling_info = {
      sizeof (AgsRecallRecyclingClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_recycling_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallRecycling),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_recycling_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_recyclingconnectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_run_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_recycling_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_recycling = g_type_register_static(AGS_TYPE_RECALL,
						       "AgsRecallRecycling\0",
						       &ags_recall_recycling_info,
						       0);
    
    g_type_add_interface_static(ags_type_recall_recycling,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
    
    g_type_add_interface_static(ags_type_recall_recycling,
				AGS_TYPE_RUN_CONNECTABLE,
				&ags_run_connectable_interface_info);
  }

  return(ags_type_recall_recycling);
}

void
ags_recall_recycling_class_init(AgsRecallRecyclingClass *recall_recycling)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_recall_recycling_parent_class = g_type_class_peek_parent(recall_recycling);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_recycling;

  gobject->set_property = ags_recall_recycling_set_property;
  gobject->get_property = ags_recall_recycling_get_property;

  gobject->finalize = ags_recall_recycling_finalize;

  /* properties */
  param_spec = g_param_spec_object("destination\0",
				   "AgsRecycling destination of this recall\0",
				   "The AgsRecycling destination of this recall\0",
				   AGS_TYPE_RECYCLING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DESTINATION,
				  param_spec);

  param_spec = g_param_spec_object("source\0",
				   "AgsRecycling source of this recall\0",
				   "The AgsRecycling source of this recall\0",
				   AGS_TYPE_RECYCLING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOURCE,
				  param_spec);

  param_spec = g_param_spec_object("child_destination\0",
				   "AgsAudioSignal of this recall\0",
				   "The AgsRecallChannel which this recall needs\0",
				   AGS_TYPE_AUDIO_SIGNAL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHILD_DESTINATION,
				  param_spec);

  param_spec = g_param_spec_object("child_source\0",
				   "AgsAudioSignal of this recall\0",
				   "The AgsRecallChannel which this recall needs\0",
				   AGS_TYPE_AUDIO_SIGNAL,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_CHILD_DESTINATION,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) recall_recycling;

  recall->duplicate = ags_recall_recycling_duplicate;

  /* AgsRecallRecyclingClass */
  recall_recycling->created_recall_audio_signal = ags_recall_recycling_created_recall_audio_signal;

  /* signals */
  recall_channel_run_signals[CREATED_RECALL_AUDIO_SIGNAL] =
    g_signal_new("created_recall_audio_signal\0",
		 G_TYPE_FROM_CLASS (recall_recycling),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (AgsRecallChannelRunClass, created_recall_audio_signal),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__OBJECT_OBJECT,
		 G_TYPE_ULONG, 0);
}

void
ags_recall_recyclingconnectable_interface_init(AgsConnectableInterface *connectable)
{
  AgsConnectableInterface *ags_recall_recycling_connectable_parent_interface;

  ags_recall_recycling_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_recycling_connect;
  connectable->disconnect = ags_recall_recycling_disconnect;
}

void
ags_recall_recycling_run_connectable_interface_init(AgsRunConnectableInterface *run_connectable)
{
  ags_recall_recycling_parent_run_connectable_interface = g_type_interface_peek_parent(run_connectable);

  run_connectable->connect = ags_recall_recycling_run_connect;
  run_connectable->disconnect = ags_recall_recycling_run_disconnect;
}

void
ags_recall_recycling_init(AgsRecallRecycling *recall_recycling)
{
  recall_recycling->destination = NULL;
  recall_recycling->source = NULL;

  recall_recycling->child_destination = NULL;
  recall_child->child_source = NULL;
}

void
ags_recall_recycling_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsRecallRecycling *recall_recycling;

  recall_recycling = AGS_RECALL_RECYCLING(gobject);

  switch(prop_id){
  case PROP_DESTINATION:
    {
      AgsRecycling *destination;

      destination = (AgsRecycling *) g_value_get_object(value);

      if(recall_recycling->destination == destination)
	return;

      if(recall_recycling->destination != NULL){
	g_object_unref(G_OBJECT(recall_recycling->destination));
      }

      if(destination != NULL){
	g_object_ref(G_OBJECT(destination));
      }

      recall_recycling->destination = destination;
    }
    break;
  case PROP_SOURCE:
    {
      AgsRecycling *source;

      source = (AgsRecycling *) g_value_get_object(value);

      if(recall_recycling->source == source)
	return;

      if(recall_recycling->source != NULL){
	g_object_unref(G_OBJECT(recall_recycling->source));
      }

      if(source != NULL){
	g_object_ref(G_OBJECT(source));
      }

      recall_recycling->source = source;
    }
    break;
  case PROP_CHILD_DESTINATION:
    {
      AgsAudioSignal *child_destination;

      child_destination = (AgsRecycling *) g_value_get_object(value);

      if(recall_recycling->child_destination == child_destination)
	return;

      if(recall_recycling->child_destination != NULL){
	g_object_unref(G_OBJECT(recall_recycling->child_destination));
      }

      if(child_destination != NULL){
	g_object_ref(G_OBJECT(child_destination));
      }

      recall_recycling->child_destination = child_destination;
    }
    break;
  case PROP_CHILD_SOURCE:
    {
      AgsAudioSignal *child_source;

      child_source = (AgsRecycling *) g_value_get_object(value);

      if(recall_recycling->child_source == child_source)
	return;

      if(recall_recycling->child_source != NULL){
	g_object_unref(G_OBJECT(recall_recycling->child_source));
      }

      if(source != NULL){
	g_object_ref(G_OBJECT(child_source));

	recall_recycling->child_source = g_list_prepend(recall_recycling->child_source,
							child_source);
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_recall_recycling_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsRecallRecycling *recall_recycling;

  recall_recycling = AGS_RECALL_RECYCLING(gobject);

  switch(prop_id){
  case PROP_DESTINATION:
    {
      g_value_set_object(value, recall_recycling->destination);
    }
    break;
  case PROP_SOURCE:
    {
      g_value_set_object(value, recall_recycling->source);
    }
    break;
  case PROP_CHILD_DESTINATION:
    {
      g_value_set_object(value, recall_recycling->child_destination);
    }
    break;
  };
}

void
ags_recall_recycling_finalize(GObject *gobject)
{
  AgsRecallRecycling *recall_recycling;

  recall_recycling = AGS_RECALL_RECYCLING(gobject);

  if(recall_recycling->destination != NULL)
    g_object_unref(G_OBJECT(recall_recycling->destination));

  if(recall_recycling->source != NULL)
    g_object_unref(G_OBJECT(recall_recycling->source));

  if(recall_recycling->child_destination != NULL)
    g_object_unref(G_OBJECT(recall_recycling->child_destination));

  if(recall_recycling->child_source != NULL)
    ags_list_free_and_unref_link(recall_recycling->child_source);

  /* call parent */
  G_OBJECT_CLASS(ags_recall_recycling_parent_class)->finalize(gobject);
}

void
ags_recall_recycling_connect(AgsConnectable *connectable)
{
  ags_recall_recycling_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_recall_recycling_disconnect(AgsConnectable *connectable)
{
  ags_recall_recycling_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_recall_recycling_run_connect(AgsConnectable *connectable)
{
  ags_recall_recycling_parent_run_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_recall_recycling_run_disconnect(AgsConnectable *connectable)
{
  ags_recall_recycling_parent_run_connectable_interface->disconnect(connectable);

  /* empty */
}

AgsRecall*
ags_recall_recycling_duplicate(AgsRecall *recall,
				 AgsRecallID *recall_id,
				 guint *n_params, GParameter *parameter)
{
  AgsRecallRecycling *recall_recycling, *copy;

  recall_recycling = AGS_RECALL_RECYCLING(recall);

  parameter = ags_parameter_grow(G_OBJECT_TYPE(recall),
				 parameter, n_params,
				 "channel\0", recall_recycling->channel,
				 NULL);
  copy = AGS_RECALL_RECYCLING(AGS_RECALL_CLASS(ags_recall_recycling_parent_class)->duplicate(recall,
												 recall_id,
												 n_params, parameter));

  return((AgsRecall *) copy);
}

GList*
ags_recall_recycling_get_child_source(AgsRecallRecycling *recall_recycling)
{
  GList *child_source;

  child_source = NULL;

  g_list_foreach(recall_recycling->child_source,
		 G_FUNC(ags_list_dupcliate_list),
		 &child_source);

  child_source = g_list_reverse(child_source);

  return(child_source);
}

AgsRecallRecycling*
ags_recall_recycling_new()
{
  AgsRecallRecycling *recall_recycling;

  recall_recycling = (AgsRecallRecycling *) g_object_new(AGS_TYPE_RECALL_RECYCLING,
							 NULL);

  return(recall_recycling);
}
