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

#include <ags/audio/ags_recall_id.h>

#include <ags/object/ags_connectable.h>

#include <stdlib.h>
#include <stdio.h>

void ags_recall_id_class_init(AgsRecallIDClass *recall_id);
void ags_recall_id_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_id_init(AgsRecallID *recall_id);
void ags_recall_id_set_property(GObject *gobject,
				guint prop_id,
				const GValue *value,
				GParamSpec *param_spec);
void ags_recall_id_get_property(GObject *gobject,
				guint prop_id,
				GValue *value,
				GParamSpec *param_spec);
void ags_recall_id_connect(AgsConnectable *connectable);
void ags_recall_id_disconnect(AgsConnectable *connectable);
void ags_recall_id_dispose(GObject *gobject);
void ags_recall_id_finalize(GObject *gobject);

/**
 * SECTION:ags_recall_id
 * @short_description: The object specifies run context.
 * @title: AgsRecallID
 * @section_id:
 * @include: ags/audio/ags_recall_id.h
 *
 * #AgsRecallID acts as dynamic context identifier.
 */

enum{
  PROP_0,
  PROP_RECYCLING,
  PROP_RECYCLING_CONTEXT,
};

static gpointer ags_recall_id_parent_class = NULL;

GType
ags_recall_id_get_type(void)
{
  static GType ags_type_recall_id = 0;

  if(!ags_type_recall_id){
    static const GTypeInfo ags_recall_id_info = {
      sizeof (AgsRecallIDClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_id_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallID),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_id_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_id_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_id = g_type_register_static(G_TYPE_OBJECT,
						"AgsRecallID\0",
						&ags_recall_id_info,
						0);

    g_type_add_interface_static(ags_type_recall_id,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_recall_id);
}

void
ags_recall_id_class_init(AgsRecallIDClass *recall_id)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_recall_id_parent_class = g_type_class_peek_parent(recall_id);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_id;

  gobject->set_property = ags_recall_id_set_property;
  gobject->get_property = ags_recall_id_get_property;

  gobject->dispose = ags_recall_id_dispose;
  gobject->finalize = ags_recall_id_finalize;

  /* properties */
  /**
   * AgsRecallID:recycling:
   *
   * The assigned #AgsRecycling.
   * 
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("recycling\0",
				   "assigned recycling\0",
				   "The recycling it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECYCLING,
				  param_spec);

  /**
   * AgsRecallID:recycling-context:
   *
   * The dynamic run context belonging to.
   * 
   * Since: 0.4.2
   */
  param_spec = g_param_spec_object("recycling-context\0",
				   "assigned recycling context\0",
				   "The recycling context it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_RECYCLING_CONTEXT,
				  param_spec);
}

void
ags_recall_id_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_recall_id_connect;
  connectable->disconnect = ags_recall_id_disconnect;
}

void
ags_recall_id_init(AgsRecallID *recall_id)
{
  recall_id->flags = 0;

  recall_id->recycling = NULL;
  recall_id->recycling_context = NULL;
}

void
ags_recall_id_set_property(GObject *gobject,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *param_spec)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(gobject);

  switch(prop_id){
  case PROP_RECYCLING:
    {
      AgsRecycling *recycling;

      recycling = g_value_get_object(value);

      if(recall_id->recycling == (GObject *) recycling){
	return;
      }

      if(recall_id->recycling != NULL){
	g_object_unref(recycling);
      }

      if(recycling != NULL){
	g_object_ref(recycling);
      }

      recall_id->recycling = (GObject *) recycling;
    }
    break;
  case PROP_RECYCLING_CONTEXT:
    {
      AgsRecyclingContext *recycling_context;

      recycling_context = g_value_get_object(value);

      if(recall_id->recycling_context == recycling_context){
	return;
      }

      if(recall_id->recycling_context != NULL){
	g_object_unref(recall_id->recycling_context);
      }

      if(recycling_context != NULL){
	g_object_ref(recycling_context);
      }

      recall_id->recycling_context = recycling_context;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_id_get_property(GObject *gobject,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *param_spec)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(gobject);

  switch(prop_id){
  case PROP_RECYCLING:
    {
      g_value_set_object(value, recall_id->recycling);
    }
    break;
  case PROP_RECYCLING_CONTEXT:
    {
      g_value_set_object(value, recall_id->recycling_context);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_id_connect(AgsConnectable *connectable)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(connectable);

  if((AGS_RECALL_ID_CONNECTED & (recall_id->flags)) != 0){
    return;
  }

  recall_id->flags |= AGS_RECALL_ID_CONNECTED;
}

void
ags_recall_id_disconnect(AgsConnectable *connectable)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(connectable);


  if((AGS_RECALL_ID_CONNECTED & (recall_id->flags)) == 0){
    return;
  }

  recall_id->flags &= (~AGS_RECALL_ID_CONNECTED);
}

void
ags_recall_id_dispose(GObject *gobject)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(gobject);

  if(recall_id->recycling != NULL){
    g_object_unref(recall_id->recycling);

    recall_id->recycling = NULL;
  }
  
  if(recall_id->recycling_context != NULL){
    g_object_unref(recall_id->recycling_context);

    recall_id->recycling_context = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_recall_id_parent_class)->dispose(gobject);
}

void
ags_recall_id_finalize(GObject *gobject)
{
  AgsRecallID *recall_id;

  recall_id = AGS_RECALL_ID(gobject);

  if(recall_id->recycling != NULL){
    g_object_unref(recall_id->recycling);
  }
  
  if(recall_id->recycling_context != NULL){
    g_object_unref(recall_id->recycling_context);
  }

  /* call parent */
  G_OBJECT_CLASS(ags_recall_id_parent_class)->finalize(gobject);
}

/**
 * ags_recall_id_get_run_stage:
 * @id: the #AgsRecallID to check
 * @stage: the current run stage to check against
 *
 * Check if a run stage already has been passed for current run. This
 * function is intended to handle AGS_AUDIO_ASYNC correctly.
 *
 * Returns: %TRUE if the stage isn't run yet otherwise %FALSE
 * 
 * Since: 0.3
 */
gboolean
ags_recall_id_get_run_stage(AgsRecallID *id, gint stage)
{
  switch(stage){
  case 0:
    {
      if((AGS_RECALL_ID_PRE & (id->flags)) == 0){
	return(TRUE);
      }
    }
    break;
  case 1:
    {
      if((AGS_RECALL_ID_INTER & (id->flags)) == 0){
	return(TRUE);
      }
    }
    break;
  case 2:
    {
      if((AGS_RECALL_ID_POST & (id->flags)) == 0){
	return(TRUE);
      }
    }
    break;
  }

  return(FALSE);
}

/**
 * ags_recall_id_set_run_stage:
 * @recall_id: the #AgsRecallID which has been passed
 * @stage: the run stage the networked channels are in
 *
 * Marks the run stage to be passed for audio channel.
 * 
 * Since: 0.3
 */
void
ags_recall_id_set_run_stage(AgsRecallID *recall_id, gint stage)
{
  guint i;

  if(stage == 0){
    recall_id->flags |= AGS_RECALL_ID_PRE;
  }else if(stage == 1){
    recall_id->flags |= AGS_RECALL_ID_INTER;
  }else{
    recall_id->flags |= AGS_RECALL_ID_POST;
  }
}

/**
 * ags_recall_id_unset_run_stage:
 * @recall_id: the #AgsRecallID which has been passed
 * @stage: the run stage the networked channels are in
 *
 * Unmarks the run stage to be passed for audio channel.
 * 
 * Since: 0.3
 */
void
ags_recall_id_unset_run_stage(AgsRecallID *recall_id, gint stage)
{
  if(stage == 0){
    recall_id->flags &= (~AGS_RECALL_ID_PRE);
  }else if(stage == 1){
    recall_id->flags &= (~AGS_RECALL_ID_INTER);
  }else{
    recall_id->flags &= (~AGS_RECALL_ID_POST);
  }
}

/**
 * ags_recall_id_add:
 * @recall_id_list: the #GList the new #AgsRecallID should be added
 * @recall_id: the #AgsRecallID to add
 *
 * Adds an #AgsRecallID with given properties to the passed #GList.
 *
 * Returns: the newly allocated #GList which is the new start of the #GList, too.
 * 
 * Since: 0.4
 */
GList*
ags_recall_id_add(GList *recall_id_list,
		  AgsRecallID *recall_id)
{
  GList *list;

  list = g_list_prepend(recall_id_list,
			(gpointer) recall_id);

  return(list);
}

/**
 * ags_recall_id_find_recycling_context:
 * @recall_id_list: a #GList containing #AgsRecallID
 * @recycling_context: the #AgsRecyclingContext to match
 *
 * Retrieve recall id by recycling context.
 *
 * Returns: Matching recall id.
 * 
 * Since: 0.4.2
 */
AgsRecallID*
ags_recall_id_find_recycling_context(GList *recall_id_list,
				       AgsRecyclingContext *recycling_context)
{
  AgsRecallID *recall_id;

  while(recall_id_list != NULL){
    recall_id = AGS_RECALL_ID(recall_id_list->data);

    if(recall_id->recycling_context == recycling_context){
      return(recall_id);
    }

    recall_id_list = recall_id_list->next;
  }

  return(NULL);
}

/**
 * ags_recall_id_find_parent_recycling_context:
 * @recall_id_list: a #GList containing #AgsRecallID
 * @parent_recycling_context: the #AgsRecyclingContext to match
 *
 * Retrieve recall id by recycling context.
 *
 * Returns: Matching recall id.
 * 
 * Since: 0.4.2
 */
AgsRecallID*
ags_recall_id_find_parent_recycling_context(GList *recall_id_list,
					      AgsRecyclingContext *parent_recycling_context)
{
  AgsRecallID *recall_id;

  while(recall_id_list != NULL){
    recall_id = AGS_RECALL_ID(recall_id_list->data);

    if(recall_id->recycling_context != NULL &&
       recall_id->recycling_context->parent == parent_recycling_context){
      return(recall_id);
    }

    recall_id_list = recall_id_list->next;
  }

  return(NULL);
}

/**
 * ags_recall_id_new:
 * @recycling: the assigned #AgsRecycling
 *
 * Creates a #AgsRecallID, assigned to @recycling
 *
 * Returns: a new #AgsRecallID
 * 
 * Since: 0.3
 */
AgsRecallID*
ags_recall_id_new(AgsRecycling *recycling)
{
  AgsRecallID *recall_id;

  recall_id = (AgsRecallID *) g_object_new(AGS_TYPE_RECALL_ID, NULL);

  return(recall_id);
}
