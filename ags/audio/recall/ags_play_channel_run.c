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

#include <ags/audio/recall/ags_play_channel_run.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_dynamic_connectable.h>
#include <ags/object/ags_plugin.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/task/ags_cancel_recall.h>

#include <ags/audio/recall/ags_play_channel.h>
#include <ags/audio/recall/ags_play_recycling.h>
#include <ags/audio/recall/ags_play_audio_signal.h>
#include <ags/audio/recall/ags_stream_channel.h>
#include <ags/audio/recall/ags_stream_recycling.h>
#include <ags/audio/recall/ags_stream_audio_signal.h>

#include <ags/audio/task/ags_cancel_channel.h>

#include <stdlib.h>
#include <stdio.h>

void ags_play_channel_run_class_init(AgsPlayChannelRunClass *play_channel_run);
void ags_play_channel_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_play_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable);
void ags_play_channel_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_play_channel_run_init(AgsPlayChannelRun *play_channel_run);
void ags_play_channel_run_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_play_channel_run_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_play_channel_run_connect(AgsConnectable *connectable);
void ags_play_channel_run_disconnect(AgsConnectable *connectable);
void ags_play_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable);
void ags_play_channel_run_dispose(GObject *gobject);
void ags_play_channel_run_finalize(GObject *gobject);

void ags_play_channel_run_run_init_inter(AgsRecall *recall);
void ags_play_channel_run_run_pre(AgsRecall *recall);
void ags_play_channel_run_run_post(AgsRecall *recall);
void ags_play_channel_run_remove(AgsRecall *recall);
void ags_play_channel_run_cancel(AgsRecall *recall);
void ags_play_channel_run_resolve_dependencies(AgsRecall *recall);
AgsRecall* ags_play_channel_run_duplicate(AgsRecall *recall,
					  AgsRecallID *recall_id,
					  guint *n_params, GParameter *parameter);

void ags_play_channel_run_stream_audio_signal_done_callback(AgsRecall *recall,
							    AgsPlayChannelRun *play_channel_run);
void ags_play_channel_run_stop(AgsPlayChannelRun *play_channel_run);

/**
 * SECTION:ags_play_channel_run
 * @short_description: plays channel
 * @title: AgsPlayChannel
 * @section_id:
 * @include: ags/audio/recall/ags_play_channel.h
 *
 * The #AgsPlayChannel class plays the channel.
 */

enum{
  PROP_0,
  PROP_STREAM_CHANNEL_RUN,
};

static gpointer ags_play_channel_run_parent_class = NULL;
static AgsConnectableInterface *ags_play_channel_run_parent_connectable_interface;
static AgsDynamicConnectableInterface *ags_play_channel_run_parent_dynamic_connectable_interface;
static AgsPluginInterface *ags_play_channel_run_parent_plugin_interface;

static const gchar *ags_play_channel_run_plugin_name = "ags-play\0";

GType
ags_play_channel_run_get_type()
{
  static GType ags_type_play_channel_run = 0;

  if(!ags_type_play_channel_run){
    static const GTypeInfo ags_play_channel_run_info = {
      sizeof (AgsPlayChannelRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_play_channel_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsPlayChannelRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_play_channel_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_channel_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_dynamic_connectable_interface_info = {
      (GInterfaceInitFunc) ags_play_channel_run_dynamic_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_play_channel_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };    

    ags_type_play_channel_run = g_type_register_static(AGS_TYPE_RECALL_CHANNEL_RUN,
						       "AgsPlayChannelRun\0",
						       &ags_play_channel_run_info,
						       0);

    g_type_add_interface_static(ags_type_play_channel_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_channel_run,
				AGS_TYPE_DYNAMIC_CONNECTABLE,
				&ags_dynamic_connectable_interface_info);

    g_type_add_interface_static(ags_type_play_channel_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_play_channel_run);
}

void
ags_play_channel_run_class_init(AgsPlayChannelRunClass *play_channel_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_play_channel_run_parent_class = g_type_class_peek_parent(play_channel_run);

  /* GObjectClass */
  gobject = (GObjectClass *) play_channel_run;

  gobject->set_property = ags_play_channel_run_set_property;
  gobject->get_property = ags_play_channel_run_get_property;

  gobject->dispose = ags_play_channel_run_dispose;
  gobject->finalize = ags_play_channel_run_finalize;

  /* properties */
  /**
   * AgsPlayChannelRun:stream-channel-run:
   * 
   * The stream channel run dependency.
   * 
   * Since: 0.7.122.7
   */
  param_spec = g_param_spec_object("stream-channel-run\0",
				   "assigned AgsStreamChannelRun\0",
				   "the assigned AgsStreamChannelRun\0",
				   AGS_TYPE_STREAM_CHANNEL_RUN,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_STREAM_CHANNEL_RUN,
				  param_spec);

  /* AgsRecallClass */
  recall = (AgsRecallClass *) play_channel_run;

  recall->run_init_inter = ags_play_channel_run_run_init_inter;
  recall->run_pre = ags_play_channel_run_run_pre;
  recall->run_post = ags_play_channel_run_run_post;
  recall->remove = ags_play_channel_run_remove;
  recall->cancel = ags_play_channel_run_cancel;
  recall->resolve_dependencies = ags_play_channel_run_resolve_dependencies;
  recall->duplicate = ags_play_channel_run_duplicate;
}

void
ags_play_channel_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_play_channel_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_play_channel_run_connect;
  connectable->disconnect = ags_play_channel_run_disconnect;
}

void
ags_play_channel_run_dynamic_connectable_interface_init(AgsDynamicConnectableInterface *dynamic_connectable)
{
  ags_play_channel_run_parent_dynamic_connectable_interface = g_type_interface_peek_parent(dynamic_connectable);

  dynamic_connectable->connect_dynamic = ags_play_channel_run_connect_dynamic;
  dynamic_connectable->disconnect_dynamic = ags_play_channel_run_disconnect_dynamic;
}

void
ags_play_channel_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_play_channel_run_parent_plugin_interface = g_type_interface_peek_parent(plugin);
}

void
ags_play_channel_run_init(AgsPlayChannelRun *play_channel_run)
{
  AGS_RECALL(play_channel_run)->name = "ags-play\0";
  AGS_RECALL(play_channel_run)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(play_channel_run)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(play_channel_run)->xml_type = "ags-play-channel-run\0";
  AGS_RECALL(play_channel_run)->port = NULL;

  AGS_RECALL(play_channel_run)->flags |= (AGS_RECALL_INPUT_ORIENTATED);
  AGS_RECALL(play_channel_run)->child_type = AGS_TYPE_PLAY_RECYCLING;

  play_channel_run->flags = 0;

  play_channel_run->stream_channel_run = NULL;
}

void
ags_play_channel_run_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsPlayChannelRun *play_channel_run;

  play_channel_run = AGS_PLAY_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_STREAM_CHANNEL_RUN:
    {
      AgsStreamChannelRun *stream_channel_run;
      gboolean is_template;

      stream_channel_run = (AgsStreamChannelRun *) g_value_get_object(value);

      if(stream_channel_run == play_channel_run->stream_channel_run)
	return;

      if(stream_channel_run != NULL &&
	 (AGS_RECALL_TEMPLATE & (AGS_RECALL(stream_channel_run)->flags)) != 0){
	is_template = TRUE;
      }else{
	is_template = FALSE;
      }

      if(play_channel_run->stream_channel_run != NULL){
	if(is_template){
	  ags_recall_remove_dependency(AGS_RECALL(play_channel_run),
				       (AgsRecall *) play_channel_run->stream_channel_run);
	}

	g_object_unref(G_OBJECT(play_channel_run->stream_channel_run));
      }

      if(stream_channel_run != NULL){
	g_object_ref(G_OBJECT(stream_channel_run));

	if(is_template){
	  ags_recall_add_dependency(AGS_RECALL(play_channel_run),
				    ags_recall_dependency_new((GObject *) stream_channel_run));
	}
      }

      play_channel_run->stream_channel_run = stream_channel_run;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_play_channel_run_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsPlayChannelRun *play_channel_run;

  play_channel_run = AGS_PLAY_CHANNEL_RUN(gobject);

  switch(prop_id){
  case PROP_STREAM_CHANNEL_RUN:
    {
      g_value_set_object(value, G_OBJECT(play_channel_run->stream_channel_run));
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_play_channel_run_dispose(GObject *gobject)
{
  AgsPlayChannelRun *play_channel_run;

  play_channel_run = AGS_PLAY_CHANNEL_RUN(gobject);

  /* stream channel run */
  if(play_channel_run->stream_channel_run != NULL){
    g_object_unref(G_OBJECT(play_channel_run->stream_channel_run));

    play_channel_run->stream_channel_run = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_channel_run_parent_class)->dispose(gobject);
}

void
ags_play_channel_run_finalize(GObject *gobject)
{
  AgsPlayChannelRun *play_channel_run;

  play_channel_run = AGS_PLAY_CHANNEL_RUN(gobject);

  ags_play_channel_run_stop((AgsPlayChannelRun *) gobject);

  /* stream channel run */
  if(play_channel_run->stream_channel_run != NULL){
    g_object_unref(G_OBJECT(play_channel_run->stream_channel_run));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_play_channel_run_parent_class)->finalize(gobject);
}

void
ags_play_channel_run_connect(AgsConnectable *connectable)
{
  /* call parent */
  ags_play_channel_run_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_play_channel_run_disconnect(AgsConnectable *connectable)
{
  /* call parent */
  ags_play_channel_run_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_play_channel_run_connect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_play_channel_run_parent_dynamic_connectable_interface->connect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_play_channel_run_disconnect_dynamic(AgsDynamicConnectable *dynamic_connectable)
{
  /* call parent */
  ags_play_channel_run_parent_dynamic_connectable_interface->disconnect_dynamic(dynamic_connectable);

  /* empty */
}

void
ags_play_channel_run_run_init_inter(AgsRecall *recall)
{
  AGS_PLAY_CHANNEL_RUN(recall)->flags |= AGS_PLAY_CHANNEL_RUN_INITIAL_RUN;

  /* call parent */
  AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->run_init_inter(recall);
}

void
ags_play_channel_run_run_pre(AgsRecall *recall)
{
  /* empty */
  
  /* call parent */
  AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->run_pre(recall);
}

void
ags_play_channel_run_run_post(AgsRecall *recall)
{
  AgsStreamChannelRun *stream_channel_run;
  AgsChannel *source;
  GList *list;
  GList *recall_recycling_list, *recall_audio_signal_list;
  gboolean found;
  
  AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->run_post(recall);

  /* connect done */
  source = AGS_RECALL_CHANNEL_RUN(recall)->source;
  found = FALSE;

  list = source->play;

  while((list = ags_recall_find_type_with_recycling_context(list,
							    AGS_TYPE_STREAM_CHANNEL_RUN,
							    (GObject *) recall->recall_id->recycling_context)) != NULL){
    stream_channel_run = AGS_STREAM_CHANNEL_RUN(list->data);

    if((AGS_RECALL_TEMPLATE & (AGS_RECALL(stream_channel_run)->flags)) != 0){
      list = list->next;

      continue;
    }
    
    recall_recycling_list = AGS_RECALL(stream_channel_run)->children;

    while(recall_recycling_list != NULL){
      recall_audio_signal_list = AGS_RECALL(recall_recycling_list->data)->children;
      
      while(recall_audio_signal_list != NULL &&
	    (AGS_RECALL_DONE & (AGS_RECALL(recall_audio_signal_list->data)->flags)) == 0 &&
	    (AGS_RECALL_TEMPLATE & (AGS_RECALL(recall_audio_signal_list->data)->flags)) == 0){
	found = TRUE;
	break;
	
	recall_audio_signal_list = recall_audio_signal_list->next;
      }

      recall_recycling_list = recall_recycling_list->next;
    }

    list = list->next;
  }
  
  if(!found){
    ags_play_channel_run_stop((AgsPlayChannelRun *) recall);
  }
}

void
ags_play_channel_run_remove(AgsRecall *recall)
{
  /* empty */
  AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->remove(recall);
}

void 
ags_play_channel_run_cancel(AgsRecall *recall)
{  
  AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->cancel(recall);

  /* empty */
}

void
ags_play_channel_run_resolve_dependencies(AgsRecall *recall)
{
  AgsRecall *template;
  AgsRecallID *recall_id;
  AgsRecallDependency *recall_dependency;
  
  AgsStreamChannelRun *stream_channel_run;

  GList *list;

  guint i, i_stop;

  template = AGS_RECALL(ags_recall_find_template(AGS_RECALL_CONTAINER(recall->container)->recall_channel_run)->data);

  list = template->dependencies;
  recall_id = recall->recall_id;

  stream_channel_run = NULL;

  i_stop = 1;

  for(i = 0; i < i_stop && list != NULL;){
    recall_dependency = AGS_RECALL_DEPENDENCY(list->data);

    if(AGS_IS_STREAM_CHANNEL_RUN(recall_dependency->dependency)){
      stream_channel_run = (AgsStreamChannelRun *) ags_recall_dependency_resolve(recall_dependency, recall_id);

      i++;
    }

    list = list->next;
  }

  g_object_set(G_OBJECT(recall),
	       "stream_channel_run\0", stream_channel_run,
	       NULL);
}

AgsRecall*
ags_play_channel_run_duplicate(AgsRecall *recall,
			       AgsRecallID *recall_id,
			       guint *n_params, GParameter *parameter)
{
  AgsPlayChannelRun *copy;
  
  copy = (AgsPlayChannelRun *) AGS_RECALL_CLASS(ags_play_channel_run_parent_class)->duplicate(recall,
											      recall_id,
											      n_params, parameter);

  return((AgsRecall *) copy);
}

void
ags_play_channel_run_stream_audio_signal_done_callback(AgsRecall *recall,
						       AgsPlayChannelRun *play_channel_run)
{  
  ags_play_channel_run_stop(play_channel_run);
}

void
ags_play_channel_run_stop(AgsPlayChannelRun *play_channel_run)
{
  GObject *soundcard;
  AgsChannel *channel;
  AgsCancelChannel *cancel_channel;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsThread *async_queue;

  AgsApplicationContext *application_context;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *soundcard_mutex;
  
  channel = AGS_RECALL_CHANNEL_RUN(play_channel_run)->source;

  soundcard = AGS_AUDIO(channel->audio)->soundcard;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* lookup soundcard mutex */
  pthread_mutex_lock(application_mutex);
 
  soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					  soundcard);
  
  pthread_mutex_unlock(application_mutex);
  
  /* get application_context */
  pthread_mutex_lock(soundcard_mutex);
  
  application_context = ags_soundcard_get_application_context(AGS_SOUNDCARD(soundcard));

  pthread_mutex_unlock(soundcard_mutex);
  
  /* get main loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* get async queue */
  async_queue = (AgsThread *) ags_thread_find_type(main_loop,
						   AGS_TYPE_TASK_THREAD);

  /* create append task */
  cancel_channel = ags_cancel_channel_new(channel,
					  AGS_PLAYBACK(channel->playback)->recall_id[0],
					  channel->playback);
  
  /* append AgsCancelAudio */
  ags_task_thread_append_task((AgsTaskThread *) async_queue,
			      (AgsTask *) cancel_channel);
}

/**
 * ags_play_channel_run_new:
 * @stream_channel_run: the #AgsStreamChannelRun as dependency
 *
 * Creates an #AgsPlayChannelRun
 *
 * Returns: a new #AgsPlayChannelRun
 *
 * Since: 0.4
 */
AgsPlayChannelRun*
ags_play_channel_run_new(AgsStreamChannelRun *stream_channel_run)
{
  AgsPlayChannelRun *play_channel_run;

  play_channel_run = (AgsPlayChannelRun *) g_object_new(AGS_TYPE_PLAY_CHANNEL_RUN,
							"stream-channel-run\0", stream_channel_run,
							NULL);
  
  return(play_channel_run);
}
