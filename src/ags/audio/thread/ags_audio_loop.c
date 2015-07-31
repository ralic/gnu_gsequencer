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

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_main_loop.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_concurrency_provider.h>
#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_recall.h>

#include <ags/audio/thread/ags_audio_thread.h>
#include <ags/audio/thread/ags_channel_thread.h>

void ags_audio_loop_class_init(AgsAudioLoopClass *audio_loop);
void ags_audio_loop_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_audio_loop_main_loop_interface_init(AgsMainLoopInterface *main_loop);
void ags_audio_loop_init(AgsAudioLoop *audio_loop);
void ags_audio_loop_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_audio_loop_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_audio_loop_connect(AgsConnectable *connectable);
void ags_audio_loop_disconnect(AgsConnectable *connectable);
void ags_audio_loop_set_application_context(AgsMainLoop *main_loop, AgsApplicationContext *application_context);
AgsApplicationContext* ags_audio_loop_get_application_context(AgsMainLoop *main_loop);
void ags_audio_loop_set_async_queue(AgsMainLoop *main_loop, GObject *async_queue);
GObject* ags_audio_loop_get_async_queue(AgsMainLoop *main_loop);
void ags_audio_loop_set_tic(AgsMainLoop *main_loop, guint tic);
guint ags_audio_loop_get_tic(AgsMainLoop *main_loop);
void ags_audio_loop_set_last_sync(AgsMainLoop *main_loop, guint last_sync);
guint ags_audio_loop_get_last_sync(AgsMainLoop *main_loop);
void ags_audio_loop_finalize(GObject *gobject);

void ags_audio_loop_start(AgsThread *thread);
void ags_audio_loop_run(AgsThread *thread);

void ags_audio_loop_play_recall(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_channel(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_channel_super_threaded(AgsAudioLoop *audio_loop, AgsPlayback *playback);
void ags_audio_loop_sync_channel_super_threaded(AgsAudioLoop *audio_loop, AgsPlayback *playback);
void ags_audio_loop_play_audio(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_audio_super_threaded(AgsAudioLoop *audio_loop, AgsPlaybackDomain *playback_domain);
void ags_audio_loop_sync_audio_super_threaded(AgsAudioLoop *audio_loop, AgsPlaybackDomain *playback_domain);

/**
 * SECTION:ags_audio_loop
 * @short_description: audio loop
 * @title: AgsAudioLoop
 * @section_id:
 * @include: ags/thread/ags_audio_loop.h
 *
 * The #AgsAudioLoop is suitable as #AgsMainLoop and does
 * audio processing.
 */

enum{
  STOPPED_ALL,
  LAST_SIGNAL,
};

enum{
  PROP_0,
  PROP_APPLICATION_CONTEXT,
  PROP_APPLICATION_MUTEX,
  PROP_SOUNDCARD,
  PROP_PLAY_RECALL,
  PROP_PLAY_CHANNEL,
  PROP_PLAY_AUDIO,
};

static gpointer ags_audio_loop_parent_class = NULL;
static AgsConnectableInterface *ags_audio_loop_parent_connectable_interface;
static guint audio_loop_signals[LAST_SIGNAL];

GType
ags_audio_loop_get_type()
{
  static GType ags_type_audio_loop = 0;

  if(!ags_type_audio_loop){
    static const GTypeInfo ags_audio_loop_info = {
      sizeof (AgsAudioLoopClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_audio_loop_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAudioLoop),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_audio_loop_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_audio_loop_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_main_loop_interface_info = {
      (GInterfaceInitFunc) ags_audio_loop_main_loop_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_audio_loop = g_type_register_static(AGS_TYPE_THREAD,
						 "AgsAudioLoop\0",
						 &ags_audio_loop_info,
						 0);
    
    g_type_add_interface_static(ags_type_audio_loop,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_audio_loop,
				AGS_TYPE_MAIN_LOOP,
				&ags_main_loop_interface_info);
  }
  
  return (ags_type_audio_loop);
}

void
ags_audio_loop_class_init(AgsAudioLoopClass *audio_loop)
{
  GObjectClass *gobject;
  AgsThreadClass *thread;
  GParamSpec *param_spec;

  ags_audio_loop_parent_class = g_type_class_peek_parent(audio_loop);

  /* GObject */
  gobject = (GObjectClass *) audio_loop;

  gobject->set_property = ags_audio_loop_set_property;
  gobject->get_property = ags_audio_loop_get_property;

  gobject->finalize = ags_audio_loop_finalize;

  /* properties */
  /**
   * AgsAudioLoop:application-context:
   *
   * The assigned #AgsApplicationContext.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("application-context\0",
				   "task thread to run\0",
				   "The task thread to run\0",
				   AGS_TYPE_APPLICATION_CONTEXT,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /**
   * AgsAudioLoop:application-mutex:
   *
   * The assigned #AgsApplicationMutex.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_pointer("application-mutex\0",
				    "task thread to run\0",
				    "The task thread to run\0",
				    G_PARAM_READABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_MUTEX,
				  param_spec);

  /**
   * AgsAudioLoop:soundcard:
   *
   * The assigned #AgsSoundcard.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("soundcard\0",
				   "soundcard assigned to\0",
				   "The AgsSoundcard it is assigned to.\0",
				   G_TYPE_OBJECT,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);

  /**
   * AgsAudioLoop:play-recall:
   *
   * An #AgsRecall to add for playback.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("play_recall\0",
				   "recall to run\0",
				   "A recall to run\0",
				   AGS_TYPE_RECALL,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAY_RECALL,
				  param_spec);

  /**
   * AgsAudioLoop:play-channel:
   *
   * An #AgsChannel to add for playback.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("play_channel\0",
				   "channel to run\0",
				   "A channel to run\0",
				   AGS_TYPE_CHANNEL,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAY_CHANNEL,
				  param_spec);

  /**
   * AgsAudioLoop:play-audio:
   *
   * An #AgsAudio to add for playback.
   * 
   * Since: 0.4
   */
  param_spec = g_param_spec_object("play_audio\0",
				   "audio to run\0",
				   "A audio to run\0",
				   AGS_TYPE_AUDIO,
				   G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_PLAY_AUDIO,
				  param_spec);

  /* AgsThread */
  thread = (AgsThreadClass *) audio_loop;
  
  thread->start = ags_audio_loop_start;
  thread->run = ags_audio_loop_run;

  /* AgsAudioLoop */
  audio_loop->stopped_all = NULL;

  
  /* signals */
  /**
   * AgsAudioLoop::stopped-all:
   * @audio_loop: the object that stopped
   *
   * The ::stopped-all signal notifies about stopped playback..
   */
  audio_loop_signals[STOPPED_ALL] =
    g_signal_new("stopped-all\0",
		 G_TYPE_FROM_CLASS(audio_loop),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsAudioLoopClass, stopped_all),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

void
ags_audio_loop_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_audio_loop_parent_connectable_interface = g_type_interface_peek_parent(connectable);
  
  connectable->connect = ags_audio_loop_connect;
  connectable->disconnect = ags_audio_loop_disconnect;
}

void
ags_audio_loop_main_loop_interface_init(AgsMainLoopInterface *main_loop)
{
  main_loop->set_application_context = ags_audio_loop_set_application_context;
  main_loop->get_application_context = ags_audio_loop_get_application_context;
  main_loop->set_async_queue = ags_audio_loop_set_async_queue;
  main_loop->get_async_queue = ags_audio_loop_get_async_queue;
  main_loop->set_tic = ags_audio_loop_set_tic;
  main_loop->get_tic = ags_audio_loop_get_tic;
  main_loop->set_last_sync = ags_audio_loop_set_last_sync;
  main_loop->get_last_sync = ags_audio_loop_get_last_sync;
}

void
ags_audio_loop_init(AgsAudioLoop *audio_loop)
{
  AgsThread *thread;

  thread = (AgsThread *) audio_loop;

  //  thread->flags |= AGS_THREAD_WAIT_FOR_CHILDREN;

  thread->freq = AGS_AUDIO_LOOP_DEFAULT_JIFFIE;

  audio_loop->flags = 0;

  g_atomic_int_set(&(audio_loop->tic), 0);
  g_atomic_int_set(&(audio_loop->last_sync), 0);

  /* AgsApplicationContext */
  audio_loop->application_context = NULL;
  audio_loop->application_mutex = NULL;
  audio_loop->async_queue = NULL;
  
  audio_loop->soundcard = NULL;

  /* recall mutex */
  audio_loop->recall_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(audio_loop->recall_mutex, NULL);

  /* recall related lists */
  audio_loop->play_recall_ref = 0;
  audio_loop->play_recall = NULL;

  audio_loop->play_channel_ref = 0;
  audio_loop->play_channel = NULL;

  audio_loop->play_audio_ref = 0;
  audio_loop->play_audio = NULL;

  audio_loop->play_notation_ref = 0;
  audio_loop->play_notation = NULL;
}

void
ags_audio_loop_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      AgsApplicationContext *application_context;

      application_context = (AgsApplicationContext *) g_value_get_object(value);

      if(audio_loop->application_context != NULL){
	g_object_unref(G_OBJECT(audio_loop->application_context));
      }

      if(application_context != NULL){
	g_object_ref(G_OBJECT(application_context));

	audio_loop->application_mutex = application_context->mutex;
      }

      audio_loop->application_context = application_context;
    }
    break;
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = (GObject *) g_value_get_object(value);

      if(audio_loop->soundcard != NULL){
	g_object_unref(G_OBJECT(audio_loop->soundcard));
      }

      if(soundcard != NULL){
	g_object_ref(G_OBJECT(soundcard));
      }

      audio_loop->soundcard = G_OBJECT(soundcard);
    }
    break;
  case PROP_PLAY_RECALL:
    {
      AgsRecall *recall;

      recall = (AgsRecall *) g_value_get_pointer(value);

      if(recall != NULL){
	audio_loop->play_recall = g_list_prepend(audio_loop->play_recall,
						 recall);
      }
    }
    break;
  case PROP_PLAY_CHANNEL:
    {
      AgsChannel *channel;

      channel = (AgsChannel *) g_value_get_pointer(value);

      if(channel != NULL){
	audio_loop->play_channel = g_list_prepend(audio_loop->play_channel,
						  channel);
      }
    }
    break;
  case PROP_PLAY_AUDIO:
    {
      AgsAudio *audio;

      audio = (AgsAudio *) g_value_get_pointer(value);

      if(audio != NULL){
	audio_loop->play_audio = g_list_prepend(audio_loop->play_audio,
						audio);
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_loop_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(gobject);

  switch(prop_id){
  case PROP_APPLICATION_CONTEXT:
    {
      g_value_set_object(value, audio_loop->application_context);
    }
    break;
  case PROP_APPLICATION_MUTEX:
    {
      g_value_set_pointer(value, audio_loop->application_mutex);
    }
    break;
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, G_OBJECT(audio_loop->soundcard));
    }
    break;
  case PROP_PLAY_RECALL:
    {
      g_value_set_pointer(value, audio_loop->play_recall);
    }
    break;
  case PROP_PLAY_CHANNEL:
    {
      g_value_set_pointer(value, audio_loop->play_channel);
    }
    break;
  case PROP_PLAY_AUDIO:
    {
      g_value_set_pointer(value, audio_loop->play_audio);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_audio_loop_connect(AgsConnectable *connectable)
{
  ags_audio_loop_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_audio_loop_disconnect(AgsConnectable *connectable)
{
  ags_audio_loop_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_audio_loop_set_application_context(AgsMainLoop *main_loop, AgsApplicationContext *application_context)
{
  AGS_AUDIO_LOOP(main_loop)->application_context = application_context;
}

AgsApplicationContext*
ags_audio_loop_get_application_context(AgsMainLoop *main_loop)
{
  return(AGS_AUDIO_LOOP(main_loop)->application_context);
}

void
ags_audio_loop_set_async_queue(AgsMainLoop *main_loop, GObject *async_queue)
{
  AGS_AUDIO_LOOP(main_loop)->async_queue = async_queue;
}

GObject*
ags_audio_loop_get_async_queue(AgsMainLoop *main_loop)
{
  return(AGS_AUDIO_LOOP(main_loop)->async_queue);
}

void
ags_audio_loop_set_tic(AgsMainLoop *main_loop, guint tic)
{
  g_atomic_int_set(&(AGS_AUDIO_LOOP(main_loop)->tic),
		   tic);
}

guint
ags_audio_loop_get_tic(AgsMainLoop *main_loop)
{
  return(g_atomic_int_get(&(AGS_AUDIO_LOOP(main_loop)->tic)));
}

void
ags_audio_loop_set_last_sync(AgsMainLoop *main_loop, guint last_sync)
{
  g_atomic_int_set(&(AGS_AUDIO_LOOP(main_loop)->last_sync),
		   last_sync);
}

guint
ags_audio_loop_get_last_sync(AgsMainLoop *main_loop)
{
  gint val;

  val = g_atomic_int_get(&(AGS_AUDIO_LOOP(main_loop)->last_sync));

  return(val);
}

void
ags_audio_loop_finalize(GObject *gobject)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(gobject);

  /* free AgsPlayback lists */
  ags_list_free_and_free_link(audio_loop->play_recall);
  ags_list_free_and_free_link(audio_loop->play_channel);
  ags_list_free_and_free_link(audio_loop->play_audio);

  if(audio_loop->soundcard != NULL){
    g_object_unref(G_OBJECT(audio_loop->soundcard));
  }

  /* call parent */
  G_OBJECT_CLASS(ags_audio_loop_parent_class)->finalize(gobject);
}

void
ags_audio_loop_start(AgsThread *thread)
{
  AgsAudioLoop *audio_loop;

  AgsApplicationContext *application_context;
  
  audio_loop = AGS_AUDIO_LOOP(thread);

  application_context = audio_loop->application_context;

  if((AGS_THREAD_SINGLE_LOOP & (thread->flags)) == 0){    
    /*  */
    AGS_THREAD_CLASS(ags_audio_loop_parent_class)->start(thread);
  }
}

void
ags_audio_loop_run(AgsThread *thread)
{
  AgsAudioLoop *audio_loop;
  GObject *soundcard;

  guint val;
  
  pthread_mutex_t *application_mutex;

  audio_loop = AGS_AUDIO_LOOP(thread);

  soundcard = G_OBJECT(audio_loop->soundcard);

  pthread_mutex_lock(audio_loop->recall_mutex);

  /* play recall */
  if((AGS_AUDIO_LOOP_PLAY_RECALL & (audio_loop->flags)) != 0){
    ags_audio_loop_play_recall(audio_loop);

    if(audio_loop->play_recall_ref == 0){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_RECALL);
    }
  }

  /* play channel */
  if((AGS_AUDIO_LOOP_PLAY_CHANNEL & (audio_loop->flags)) != 0){
    ags_audio_loop_play_channel(audio_loop);

    if(audio_loop->play_channel_ref == 0){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_CHANNEL);
    }
  }

  /* play audio */
  if((AGS_AUDIO_LOOP_PLAY_AUDIO & (audio_loop->flags)) != 0){
    ags_audio_loop_play_audio(audio_loop);

    if(audio_loop->play_audio_ref == 0){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_AUDIO);
    }
  }

  /* decide if we stop */
  if(audio_loop->play_recall_ref == 0 &&
     audio_loop->play_channel_ref == 0 &&
     audio_loop->play_audio_ref == 0 &&
     audio_loop->play_notation_ref == 0){
    ags_audio_loop_stopped_all(audio_loop);
  }

  pthread_mutex_unlock(audio_loop->recall_mutex);
}

/**
 * ags_audio_loop_play_recall:
 * @audio_loop: an #AgsAudioLoop
 *
 * Runs all recalls assigned with @audio_loop. You may want to use
 * #AgsAppendRecall task to add an #AgsRecall.
 *
 * Since: 0.4
 */
void
ags_audio_loop_play_recall(AgsAudioLoop *audio_loop)
{
  AgsPlayback *playback;
  AgsRecall *recall;
  //  AgsRecallID *recall_id;
  GList *list, *list_next;
  guint stage;

  audio_loop->flags |= AGS_AUDIO_LOOP_PLAYING_RECALL;
  stage = 0;

 ags_audio_loop_play_recall0:

  list = audio_loop->play_recall;

  if(list == NULL){
    if((AGS_AUDIO_LOOP_PLAY_RECALL_TERMINATING & (audio_loop->flags)) != 0){
      audio_loop->flags &= (~(AGS_AUDIO_LOOP_PLAY_RECALL |
			      AGS_AUDIO_LOOP_PLAY_RECALL_TERMINATING));
    }else{
      audio_loop->flags |= AGS_AUDIO_LOOP_PLAY_RECALL_TERMINATING;
    }
  }

  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_RECALL_TERMINATING);

  while(list != NULL){
    playback = AGS_PLAYBACK(list->data);
    recall = AGS_RECALL(playback->source);

    //    recall_id = playback->recall_id;
    list_next = list->next;

    if((AGS_RECALL_REMOVE & (recall->flags)) == 0){
      if((AGS_RECALL_HIDE & (recall->flags)) == 0){
	if(stage == 0){
	  ags_recall_run_pre(recall);
	}else if(stage == 1){
	  ags_recall_run_inter(recall);
	}else{
	  ags_recall_run_post(recall);
	}
      }
    }else{
      ags_audio_loop_remove_recall(audio_loop, playback);
      g_object_unref(recall);
      free(playback);
    }

    list = list_next;
  }

  if(stage == 0){
    stage = 1;
    goto ags_audio_loop_play_recall0;
  }else if(stage == 1){
    stage = 2;
    goto ags_audio_loop_play_recall0;
  }

  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAYING_RECALL);
}

/**
 * ags_audio_loop_play_channel:
 * @audio_loop: an #AgsAudioLoop
 *
 * Runs all recalls descending recursively and ascending till next 
 * #AgsRecycling around prior added #AgsChannel with #AgsAppendChannel
 * task.
 *
 * Since: 0.4
 */
void
ags_audio_loop_play_channel(AgsAudioLoop *audio_loop)
{
  AgsPlayback *playback;
  AgsChannel *channel;

  AgsMutexManager *mutex_manager;

  GList *list_play, *list_next_play;
  gint stage;

  pthread_mutex_t *soundcard_mutex, *audio_mutex;

  if(audio_loop->play_channel == NULL){
    if((AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING & (audio_loop->flags)) != 0){
      audio_loop->flags &= (~(AGS_AUDIO_LOOP_PLAY_CHANNEL |
			      AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING));
    }else{
      audio_loop->flags |= AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING;
    }
  }

  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING);

  /* entry point */
  audio_loop->flags |= AGS_AUDIO_LOOP_PLAYING_CHANNEL;

  /* run the 3 stages */
  list_play = audio_loop->play_channel;
  
  while(list_play != NULL){
    list_next_play = list_play->next;
    
    playback = (AgsPlayback *) list_play->data;
    channel = AGS_CHANNEL(playback->source);
    
    /*  */
    pthread_mutex_lock(audio_loop->application_mutex);
    
    soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					    (GObject *) AGS_AUDIO(channel->audio)->soundcard);
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel->audio);
      
    pthread_mutex_unlock(audio_loop->application_mutex);
      
    //      pthread_mutex_lock(soundcard_mutex);
    //      pthread_mutex_lock(audio_mutex);
      
    /*  */
    if((AGS_PLAYBACK_REMOVE & (g_atomic_int_get(&(playback->flags)))) == 0){
      if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
	/* super threaded */
	ags_audio_loop_play_channel_super_threaded(audio_loop, playback);
      }else{
	gboolean remove_play;

	/* not super threaded */
	remove_play = TRUE;
	    
	for(stage = 0; stage < 3; stage++){


	  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel, playback->recall_id[0], stage);
	  }

	  if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel, playback->recall_id[1], stage);
	  }

	  if((AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) != 0){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel, playback->recall_id[2], stage);
	  }
	}

	if(remove_play){
	  ags_audio_loop_remove_channel(audio_loop, channel);
	}	  
      }
    }

    //      pthread_mutex_unlock(audio_mutex);
    //      pthread_mutex_unlock(soundcard_mutex);

    list_play = list_next_play;
  }

  /* sync the 3 stages */
  list_play = audio_loop->play_channel;
  
  while(list_play != NULL){
    list_next_play = list_play->next;
    
    playback = (AgsPlayback *) list_play->data;
    channel = AGS_CHANNEL(playback->source);

    if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
      /* super threaded */
      ags_audio_loop_sync_channel_super_threaded(audio_loop, playback);
    }

    list_play = list_next_play;
  }
}

void
ags_audio_loop_play_channel_super_threaded(AgsAudioLoop *audio_loop, AgsPlayback *playback)
{
  gboolean do_playback, do_sequencer, do_notation;
	
  /* super threaded audio level */
  do_playback = FALSE;
  do_sequencer = FALSE;
  do_notation = FALSE;
	
  /* playback */
  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
    do_playback = TRUE;

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) == 0){
      guint val;

      ags_thread_start(playback->channel_thread[0]);

      /* wait child */
      pthread_mutex_lock(playback->channel_thread[0]->start_mutex);

      val = g_atomic_int_get(&(playback->channel_thread[0]->flags));

      if((AGS_THREAD_INITIAL_RUN & val) != 0){
	while((AGS_THREAD_INITIAL_RUN & val) != 0){
	  pthread_cond_wait(playback->channel_thread[0]->start_cond,
			    playback->channel_thread[0]->start_mutex);

	  val = g_atomic_int_get(&(playback->channel_thread[0]->flags));
	}
      }

      pthread_mutex_unlock(playback->channel_thread[0]->start_mutex);
    }

    /* wakeup wait */
    pthread_mutex_lock(AGS_CHANNEL_THREAD(playback->channel_thread[0])->wakeup_mutex);

    g_atomic_int_or(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags),
		    AGS_CHANNEL_THREAD_WAKEUP);
	    
    if((AGS_CHANNEL_THREAD_WAITING & (g_atomic_int_get(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags)))) != 0){
      pthread_cond_signal(AGS_CHANNEL_THREAD(playback->channel_thread[0])->wakeup_cond);
    }
	    
    pthread_mutex_unlock(AGS_CHANNEL_THREAD(playback->channel_thread[0])->wakeup_mutex);
  }
}

void
ags_audio_loop_sync_channel_super_threaded(AgsAudioLoop *audio_loop, AgsPlayback *playback)
{
  gboolean do_playback;
  
  do_playback = FALSE;
	
  /* playback */
  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
    do_playback = TRUE;

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) != 0 &&
       (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_0 & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_1 & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_2 & (g_atomic_int_get(&(playback->channel_thread[0]->flags)))) == 0){

      pthread_mutex_lock(AGS_CHANNEL_THREAD(playback->channel_thread[0])->done_mutex);

      if((AGS_CHANNEL_THREAD_DONE & (g_atomic_int_get(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags)))) == 0){
	g_atomic_int_or(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags),
			AGS_CHANNEL_THREAD_NOTIFY);
    
	while((AGS_CHANNEL_THREAD_DONE & (g_atomic_int_get(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags)))) == 0){
	  pthread_cond_wait(AGS_CHANNEL_THREAD(playback->channel_thread[0])->done_cond,
			    AGS_CHANNEL_THREAD(playback->channel_thread[0])->done_mutex);
	}
      }

      g_atomic_int_and(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags),
		       (~AGS_CHANNEL_THREAD_NOTIFY));
      g_atomic_int_and(&(AGS_CHANNEL_THREAD(playback->channel_thread[0])->flags),
		       (~AGS_CHANNEL_THREAD_DONE));
      
      pthread_mutex_unlock(AGS_CHANNEL_THREAD(playback->channel_thread[0])->done_mutex);
    }
  }

  /*  */
  if(do_playback && playback->recall_id[0] == NULL){
    ags_thread_stop(playback->channel_thread[0]);
    g_atomic_int_and(&(playback->flags),
		     (~AGS_PLAYBACK_PLAYBACK));
  }

  /* remove from playback */
  if(!do_playback){
    ags_thread_stop(playback->channel_thread[0]);
    ags_audio_loop_remove_channel(audio_loop,
				  playback->source);
  }
}

/**
 * ags_audio_loop_play_audio:
 * @audio_loop: an #AgsAudioLoop
 *
 * Like ags_audio_loop_play_channel() except that it runs all channels within
 * #AgsAudio.
 *
 * Since: 0.4
 */
void
ags_audio_loop_play_audio(AgsAudioLoop *audio_loop)
{
  AgsPlaybackDomain *play_domain;
  AgsPlayback *playback;
  AgsAudio *audio;
  AgsChannel *output;

  AgsMutexManager *mutex_manager;

  GList *list_play_domain, *list_next_play_domain;
  GList *list_play;
  gint stage;
  gboolean remove_domain;
  
  pthread_mutex_t *soundcard_mutex, *audio_mutex;

  if(audio_loop->play_audio == NULL){
    if((AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING & (audio_loop->flags)) != 0){
      audio_loop->flags &= (~(AGS_AUDIO_LOOP_PLAY_AUDIO |
			      AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING));
    }else{
      audio_loop->flags |= AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING;
    }
  }

  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING);

  /*  */
  pthread_mutex_lock(audio_loop->application_mutex);

  mutex_manager = ags_mutex_manager_get_instance();

  pthread_mutex_unlock(audio_loop->application_mutex);

  /* entry point */
  audio_loop->flags |= AGS_AUDIO_LOOP_PLAYING_AUDIO;

  list_play_domain = audio_loop->play_audio;

  while(list_play_domain != NULL){
    list_next_play_domain = list_play_domain->next;
    
    play_domain = (AgsPlaybackDomain *) list_play_domain->data;
    audio = AGS_AUDIO(play_domain->domain);
    
    /*  */
    pthread_mutex_lock(audio_loop->application_mutex);
    
    soundcard_mutex = ags_mutex_manager_lookup(mutex_manager,
					    (GObject *) audio->soundcard);
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) audio);
    
    pthread_mutex_unlock(audio_loop->application_mutex);

    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(play_domain->flags)))) != 0){
      ags_audio_loop_play_audio_super_threaded(audio_loop, play_domain);
    }else{
      /*  */
      output = audio->output;

      while(output != NULL){
	playback = output->playback; // ags_playback_find_source(play_domain->playback,
	//			   (GObject *) output);
      
	if(playback == NULL){
	  output = output->next;
	  continue;
	}
            
	if((AGS_PLAYBACK_SUPER_THREADED_RECYCLING & (g_atomic_int_get(&(playback->flags)))) != 0){
	  //TODO:JK: implement me

	  /* super threaded recycling level */
	  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
	    playback->iterator_thread[0]->flags |= AGS_ITERATOR_THREAD_DONE;
	    pthread_cond_signal(&(playback->iterator_thread[0]->tic_cond));
	  }

	  if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0){
	    playback->iterator_thread[1]->flags |= AGS_ITERATOR_THREAD_DONE;
	    pthread_cond_signal(&(playback->iterator_thread[1]->tic_cond));
	  }

	    if((AGS_PLAYBACK_SEQUENCER & (playback->flags)) != 0){
	      ags_channel_recursive_play(output, playback->recall_id[1], stage);
	    }

	  /* not super threaded */
	  pthread_mutex_lock(audio_mutex);
	  pthread_mutex_lock(soundcard_mutex);    

	  remove_domain = TRUE;
	
	  /* run the 3 stages */
	  for(stage = 0; stage < 3; stage++){
	    if((AGS_PLAYBACK_REMOVE & (g_atomic_int_get(&(playback->flags)))) == 0){
	      if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[0], stage);
	      }

	      if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[1], stage);
	      }

	      if((AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) != 0){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[2], stage);
	      }
	    }
	  }

	  /* check for removal */
	  if(remove_domain){
	    AgsChannel *channel;

	    audio_loop->play_audio_ref = audio_loop->play_audio_ref - 1;
	    audio_loop->play_audio = g_list_remove(audio_loop->play_audio, (gpointer) play_domain);

	    channel = audio->output;
	
	    while(channel != NULL){
	      playback = AGS_PLAYBACK(channel->playback);	  
	      g_atomic_int_and(&(playback->flags),
			       (~(AGS_PLAYBACK_REMOVE | AGS_PLAYBACK_DONE)));

	      //	  ags_audio_done(audio);

	      //TODO:JK: verify g_object_unref() missing
	      playback->recall_id[0] = NULL;
	      playback->recall_id[1] = NULL;
	      playback->recall_id[2] = NULL;

	      channel = channel->next;
	    }
	  }
		  
	  pthread_mutex_unlock(soundcard_mutex);
	  pthread_mutex_unlock(audio_mutex);
	}

	output = output->next;
      }
    }
    
    /* iterate */
    list_play_domain = list_next_play_domain;
  }
  
  /* sync - wait the 3 stages */
  list_play_domain = audio_loop->play_audio;

  while(list_play_domain != NULL){
    list_next_play_domain = list_play_domain->next;
        
    play_domain = (AgsPlaybackDomain *) list_play_domain->data;
    audio = AGS_AUDIO(play_domain->domain);
    
    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(play_domain->flags)))) != 0){
      ags_audio_loop_sync_audio_super_threaded(audio_loop, play_domain);
    }else{
      /*  */
      output = audio->output;

      while(output != NULL){
	playback = output->playback; // ags_playback_find_source(play_domain->playback,

	if(playback == NULL){
	  output = output->next;
	  continue;
	}      

	if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
	  ags_audio_loop_sync_channel_super_threaded(audio_loop, playback);
	}else if((AGS_PLAYBACK_SUPER_THREADED_RECYCLING & (g_atomic_int_get(&(playback->flags)))) != 0){
	  //TODO:JK: implement me
	}

	output = output->next;
      }
    }
    
    /* iterate */
    list_play_domain = list_next_play_domain;
  }
}
 
/**
 * ags_audio_loop_play_audio_super_threaded:
 * @audio_loop: the #AgsAudioLoop
 * @playback_domain: an #AgsPlaybackDomain
 *
 * Play audio super-threaded.
 *
 * Since: 0.4.2
 */
void
ags_audio_loop_play_audio_super_threaded(AgsAudioLoop *audio_loop, AgsPlaybackDomain *playback_domain)
{
  gboolean do_sequencer, do_notation;
	
  /* super threaded audio level */
  do_sequencer = FALSE;
  do_notation = FALSE;	

  /* sequencer */
  if((AGS_PLAYBACK_DOMAIN_SEQUENCER & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    do_sequencer = TRUE;

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0){
      guint val;
	  
      ags_thread_start(playback_domain->audio_thread[1]);
  
      /* wait child */
      pthread_mutex_lock(playback_domain->audio_thread[1]->start_mutex);

      val = g_atomic_int_get(&(playback_domain->audio_thread[1]->flags));

      if((AGS_THREAD_INITIAL_RUN & val) != 0){
	while((AGS_THREAD_INITIAL_RUN & val) != 0){
	  pthread_cond_wait(playback_domain->audio_thread[1]->start_cond,
			    playback_domain->audio_thread[1]->start_mutex);

	  val = g_atomic_int_get(&(playback_domain->audio_thread[1]->flags));
	}
      }

      pthread_mutex_unlock(playback_domain->audio_thread[1]->start_mutex);
    }

    /* wakeup wait */
    pthread_mutex_lock(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->wakeup_mutex);

    g_atomic_int_or(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags),
		    AGS_AUDIO_THREAD_WAKEUP);
	    
    if((AGS_AUDIO_THREAD_WAITING & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags)))) != 0){
      pthread_cond_signal(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->wakeup_cond);
    }
	    
    pthread_mutex_unlock(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->wakeup_mutex);
  }

  /* notation */
  if((AGS_PLAYBACK_DOMAIN_NOTATION & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    do_notation = TRUE;

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0){
      guint val;

      ags_thread_start(playback_domain->audio_thread[2]);

      /* wait child */
      pthread_mutex_lock(playback_domain->audio_thread[2]->start_mutex);

      val = g_atomic_int_get(&(playback_domain->audio_thread[2]->flags));

      if((AGS_THREAD_INITIAL_RUN & val) != 0){
	while((AGS_THREAD_INITIAL_RUN & val) != 0){
	  pthread_cond_wait(playback_domain->audio_thread[2]->start_cond,
			    playback_domain->audio_thread[2]->start_mutex);

	  val = g_atomic_int_get(&(playback_domain->audio_thread[2]->flags));
	}
      }

      pthread_mutex_unlock(playback_domain->audio_thread[2]->start_mutex);
    }

    /* wakeup wait */
    pthread_mutex_lock(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->wakeup_mutex);
	  
    g_atomic_int_or(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags),
		    AGS_AUDIO_THREAD_WAKEUP);
	  
    if((AGS_AUDIO_THREAD_WAITING & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags)))) != 0){
      pthread_cond_signal(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->wakeup_cond);
    }
	  
    pthread_mutex_unlock(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->wakeup_mutex);
  }
}

/**
 * ags_audio_loop_sync_audio_super_threaded:
 * @audio_loop: the #AgsAudioLoop
 * @playback_domain: an #AgsPlaybackDomain
 *
 * Sync audio super-threaded.
 *
 * Since: 0.4.2
 */
void
ags_audio_loop_sync_audio_super_threaded(AgsAudioLoop *audio_loop, AgsPlaybackDomain *playback_domain)
{
  GList *playback;
  gboolean do_sequencer, do_notation;
  gboolean found_sequencer, found_notation;
  
  do_sequencer = FALSE;
  do_notation = FALSE;
  
  /* sequencer */
  if((AGS_PLAYBACK_DOMAIN_SEQUENCER & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    do_sequencer = TRUE;

    pthread_mutex_lock(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->done_mutex);

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) != 0 &&
       (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_0 & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_1 & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_2 & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0){

      if((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags)))) == 0){
	g_atomic_int_or(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags),
			AGS_AUDIO_THREAD_NOTIFY);
    
	while((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags)))) == 0){
	  pthread_cond_wait(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->done_cond,
			    AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->done_mutex);
	}
      }
    }
    
    g_atomic_int_and(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags),
		     (~AGS_AUDIO_THREAD_NOTIFY));
    g_atomic_int_and(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->flags),
		     (~AGS_AUDIO_THREAD_DONE));
      	  
    pthread_mutex_unlock(AGS_AUDIO_THREAD(playback_domain->audio_thread[1])->done_mutex);
  }

  /* notation */
  if((AGS_PLAYBACK_DOMAIN_NOTATION & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    do_notation = TRUE;

    pthread_mutex_lock(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->done_mutex);

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) != 0 &&
       (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_0 & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_1 & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0 &&
       (AGS_THREAD_WAIT_2 & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0){
      
      if((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags)))) == 0){
	g_atomic_int_or(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags),
			AGS_AUDIO_THREAD_NOTIFY);
    
	while((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags)))) == 0){
	  pthread_cond_wait(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->done_cond,
			    AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->done_mutex);
	}
      }
    }
      
    g_atomic_int_and(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags),
		     (~AGS_AUDIO_THREAD_NOTIFY));
    g_atomic_int_and(&(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->flags),
		     (~AGS_AUDIO_THREAD_DONE));
      
    pthread_mutex_unlock(AGS_AUDIO_THREAD(playback_domain->audio_thread[2])->done_mutex);
  }

  /* check if flags are still valid */
  found_sequencer = FALSE;
  found_notation = FALSE;
	
  playback = playback_domain->playback;

  while(playback != NULL){
    if(do_sequencer && AGS_PLAYBACK(playback->data)->recall_id[1] != NULL){
      found_sequencer = TRUE;
    }
    
    if(do_notation && AGS_PLAYBACK(playback->data)->recall_id[2] != NULL){
      found_notation = TRUE;
    }

    playback = playback->next;
  }

  if(do_sequencer && !found_sequencer){
    ags_thread_stop(playback_domain->audio_thread[1]);
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_SEQUENCER));
  }

  if(do_notation && !found_notation){
    ags_thread_stop(playback_domain->audio_thread[2]);
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_NOTATION));
  }

  /* remove domain from playback */
  if(!found_sequencer && !found_notation){
    ags_audio_loop_remove_audio(audio_loop,
				playback_domain->domain);
  }
}

/**
 * ags_audio_loop_add_audio:
 * @audio_loop: the #AgsAudioLoop
 * @audio: an #AgsAudio
 *
 * Add audio for playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_add_audio(AgsAudioLoop *audio_loop, GObject *audio)
{
  if(g_list_find(audio_loop->play_audio,
		 AGS_AUDIO(audio)->playback_domain) == NULL){
    audio_loop->play_audio_ref = audio_loop->play_audio_ref + 1;
    g_object_ref(G_OBJECT(audio));

    audio_loop->play_audio = g_list_prepend(audio_loop->play_audio,
					    AGS_AUDIO(audio)->playback_domain);
  }
}

/**
 * ags_audio_loop_remove_audio:
 * @audio_loop: the #AgsAudioLoop
 * @audio: an #AgsAudio
 *
 * Remove audio of playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_remove_audio(AgsAudioLoop *audio_loop, GObject *audio)
{  
  if(g_list_find(audio_loop->play_audio,
		 AGS_AUDIO(audio)->playback_domain) != NULL){
    audio_loop->play_audio = g_list_remove(audio_loop->play_audio,
					   AGS_AUDIO(audio)->playback_domain);
    audio_loop->play_audio_ref = audio_loop->play_audio_ref - 1;
    
    g_object_unref(audio);
  }
}

/**
 * ags_audio_loop_add_channel:
 * @audio_loop: the #AgsAudioLoop
 * @channel: an #AgsChannel
 *
 * Add channel for playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_add_channel(AgsAudioLoop *audio_loop, GObject *channel)
{
  if(g_list_find(audio_loop->play_channel,
		 AGS_CHANNEL(channel)->playback) == NULL){
    g_object_ref(G_OBJECT(channel));
    audio_loop->play_channel = g_list_prepend(audio_loop->play_channel,
					      AGS_CHANNEL(channel)->playback);

    audio_loop->play_channel_ref = audio_loop->play_channel_ref + 1;
  }
}

/**
 * ags_audio_loop_remove_channel:
 * @audio_loop: the #AgsAudioLoop
 * @channel: an #AgsChannel
 *
 * Remove channel of playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_remove_channel(AgsAudioLoop *audio_loop, GObject *channel)
{
  if(g_list_find(audio_loop->play_channel,
		 AGS_CHANNEL(channel)->playback) != NULL){
    audio_loop->play_channel = g_list_remove(audio_loop->play_channel,
					     AGS_CHANNEL(channel)->playback);
    audio_loop->play_channel_ref = audio_loop->play_channel_ref - 1;

    g_object_unref(channel);
  }
}

/**
 * ags_audio_loop_add_recall:
 * @audio_loop: the #AgsAudioLoop
 * @recall: an #AgsRecall
 *
 * Add recall for playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_add_recall(AgsAudioLoop *audio_loop, GObject *playback)
{
  if(g_list_find(audio_loop->play_recall,
		 playback) == NULL){
    AGS_PLAYBACK(playback)->flags &= (~AGS_PLAYBACK_REMOVE);
    audio_loop->play_recall = g_list_append(audio_loop->play_recall,
					    playback);
    audio_loop->play_recall_ref += 1;
  }
}

/**
 * ags_audio_loop_remove_recall:
 * @audio_loop: the #AgsAudioLoop
 * @recall: an #AgsRecall
 *
 * Remove recall of playback.
 *
 * Since: 0.4
 */
void
ags_audio_loop_remove_recall(AgsAudioLoop *audio_loop, GObject *playback)
{
  if(g_list_find(audio_loop->play_recall,
		 playback) != NULL){
    audio_loop->play_recall = g_list_remove(audio_loop->play_recall,
					    playback);
    audio_loop->play_recall_ref = audio_loop->play_recall_ref - 1;
  }
}

/**
 * ags_audio_loop_new:
 * @soundcard: the #GObject implementing #AgsSoundcard
 * @application_context: the #AgsMain
 *
 * Create a new #AgsAudioLoop.
 *
 * Returns: the new #AgsAudioLoop
 *
 * Since: 0.4
 */
AgsAudioLoop*
ags_audio_loop_new(GObject *soundcard, GObject *application_context)
{
  AgsAudioLoop *audio_loop;

  audio_loop = (AgsAudioLoop *) g_object_new(AGS_TYPE_AUDIO_LOOP,
					     "application-context\0", application_context,
					     "soundcard\0", soundcard,
					     NULL);

  return(audio_loop);
}
