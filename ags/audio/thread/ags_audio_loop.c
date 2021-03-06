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

#include <ags/object/ags_config.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_main_loop.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_polling_thread.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_audio_run.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_playback_domain.h>
#include <ags/audio/ags_playback.h>

#include <ags/audio/thread/ags_soundcard_thread.h>
#include <ags/audio/thread/ags_sequencer_thread.h>
#include <ags/audio/thread/ags_export_thread.h>
#include <ags/audio/thread/ags_audio_thread.h>
#include <ags/audio/thread/ags_channel_thread.h>
#include <ags/audio/thread/ags_recycling_thread.h>
#include <ags/audio/thread/ags_iterator_thread.h>

#include <fontconfig/fontconfig.h>

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
pthread_mutex_t* ags_audio_loop_get_tree_lock(AgsMainLoop *main_loop);
void ags_audio_loop_set_async_queue(AgsMainLoop *main_loop, GObject *async_queue);
GObject* ags_audio_loop_get_async_queue(AgsMainLoop *main_loop);
void ags_audio_loop_set_tic(AgsMainLoop *main_loop, guint tic);
guint ags_audio_loop_get_tic(AgsMainLoop *main_loop);
void ags_audio_loop_set_last_sync(AgsMainLoop *main_loop, guint last_sync);
guint ags_audio_loop_get_last_sync(AgsMainLoop *main_loop);
gboolean ags_audio_loop_monitor(AgsMainLoop *main_loop,
				guint time_cycle, guint *time_spent);
void ags_audio_loop_change_frequency(AgsMainLoop *main_loop,
				     gdouble frequency);
void ags_audio_loop_dispose(GObject *gobject);
void ags_audio_loop_finalize(GObject *gobject);

void ags_audio_loop_start(AgsThread *thread);
void ags_audio_loop_run(AgsThread *thread);

void ags_audio_loop_notify_frequency(GObject *gobject,
				     GParamSpec *pspec,
				     gpointer user_data);

void* ags_audio_loop_timing_thread(void *ptr);
void ags_audio_loop_play_recall(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_channel(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_channel_super_threaded(AgsAudioLoop *audio_loop,
						AgsPlayback *playback);
void ags_audio_loop_sync_channel_super_threaded(AgsAudioLoop *audio_loop,
						AgsPlayback *playback);
void ags_audio_loop_play_audio(AgsAudioLoop *audio_loop);
void ags_audio_loop_play_audio_super_threaded(AgsAudioLoop *audio_loop,
					      AgsPlaybackDomain *playback_domain);
void ags_audio_loop_sync_audio_super_threaded(AgsAudioLoop *audio_loop,
					      AgsPlaybackDomain *playback_domain);

/**
 * SECTION:ags_audio_loop
 * @short_description: audio loop
 * @title: AgsAudioLoop
 * @section_id:
 * @include: ags/audio/thread/ags_audio_loop.h
 *
 * The #AgsAudioLoop is suitable as #AgsMainLoop and does
 * audio processing.
 */

enum{
  PROP_0,
  PROP_APPLICATION_CONTEXT,
  PROP_SOUNDCARD,
  PROP_PLAY_RECALL,
  PROP_PLAY_CHANNEL,
  PROP_PLAY_AUDIO,
};

static gpointer ags_audio_loop_parent_class = NULL;
static AgsConnectableInterface *ags_audio_loop_parent_connectable_interface;

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

  gobject->dispose = ags_audio_loop_dispose;
  gobject->finalize = ags_audio_loop_finalize;

  /* properties */
  /**
   * AgsDevout:application-context:
   *
   * The assigned #AgsApplicationContext
   * 
   * Since: 0.7.122.7
   */
  param_spec = g_param_spec_object("application-context\0",
				   "the application context object\0",
				   "The application context object\0",
				   AGS_TYPE_APPLICATION_CONTEXT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /**
   * AgsAudioLoop:soundcard:
   *
   * The assigned #AgsAudio.
   * 
   * Since: 0.7.0
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
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("play-recall\0",
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
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("play-channel\0",
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
   * Since: 0.4.0
   */
  param_spec = g_param_spec_object("play-audio\0",
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
  main_loop->get_tree_lock = ags_audio_loop_get_tree_lock;
  main_loop->set_async_queue = ags_audio_loop_set_async_queue;
  main_loop->get_async_queue = ags_audio_loop_get_async_queue;
  main_loop->set_tic = ags_audio_loop_set_tic;
  main_loop->get_tic = ags_audio_loop_get_tic;
  main_loop->set_last_sync = ags_audio_loop_set_last_sync;
  main_loop->get_last_sync = ags_audio_loop_get_last_sync;
  main_loop->interrupt = NULL;
  main_loop->monitor = ags_audio_loop_monitor;
  main_loop->change_frequency = ags_audio_loop_change_frequency;
}

void
ags_audio_loop_init(AgsAudioLoop *audio_loop)
{
  AgsMutexManager *mutex_manager;
  AgsThread *thread;

  AgsConfig *config;
  
  gchar *str0, *str1;

  pthread_mutex_t *application_mutex;

  /* calculate frequency */
  thread = (AgsThread *) audio_loop;

  g_signal_connect_after(thread, "notify::frequency\0",
			 G_CALLBACK(ags_audio_loop_notify_frequency), NULL);
  
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  //  thread->flags |= AGS_THREAD_WAIT_FOR_CHILDREN;
  pthread_mutex_lock(application_mutex);

  config = ags_config_get_instance();
  
  str0 = ags_config_get_value(config,
			      AGS_CONFIG_SOUNDCARD,
			      "samplerate\0");

  if(str0 == NULL){
    str0 = ags_config_get_value(config,
				AGS_CONFIG_SOUNDCARD_0,
				"samplerate\0");
  }
  
  str1 = ags_config_get_value(config,
			      AGS_CONFIG_SOUNDCARD,
			      "buffer-size\0");

  if(str1 == NULL){
    str1 = ags_config_get_value(config,
				AGS_CONFIG_SOUNDCARD_0,
				"buffer-size\0");
  }

  pthread_mutex_unlock(application_mutex);

  if(str0 == NULL || str1 == NULL){
    thread->freq = AGS_AUDIO_LOOP_DEFAULT_JIFFIE;
  }else{
    guint samplerate;
    guint buffer_size;

    samplerate = g_ascii_strtoull(str0,
				  NULL,
				  10);
    buffer_size = g_ascii_strtoull(str1,
				   NULL,
				   10);

    thread->freq = ceil((gdouble) samplerate / (gdouble) buffer_size) + AGS_SOUNDCARD_DEFAULT_OVERCLOCK;
  }

  g_free(str0);
  g_free(str1);
  
  audio_loop->flags = 0;

  g_atomic_int_set(&(audio_loop->tic), 0);
  g_atomic_int_set(&(audio_loop->last_sync), 0);

  audio_loop->time_cycle = NSEC_PER_SEC / thread->freq;
  
  audio_loop->application_context = NULL;
  audio_loop->soundcard = NULL;

  audio_loop->async_queue = NULL;
    
  /* tree lock mutex */
  pthread_mutexattr_init(&(audio_loop->tree_lock_mutexattr));
  pthread_mutexattr_settype(&(audio_loop->tree_lock_mutexattr), PTHREAD_MUTEX_RECURSIVE);

  audio_loop->tree_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(audio_loop->tree_lock, &(audio_loop->tree_lock_mutexattr));
  
  /* recall mutex */
  audio_loop->recall_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(audio_loop->recall_mutex, NULL);

  /* event main loop */
  g_cond_init(&(audio_loop->cond));
  g_mutex_init(&(audio_loop->mutex));

  audio_loop->main_context = g_main_context_new();

  audio_loop->cached_poll_array_size = 0;
  audio_loop->cached_poll_array = NULL;

  /* timing thread */
  audio_loop->timing_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(audio_loop->timing_mutex, NULL);

  audio_loop->timing_cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(audio_loop->timing_cond, NULL);

  audio_loop->timing_thread = (pthread_t *) malloc(sizeof(pthread_t));

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

      if(audio_loop->application_context == (GObject *) application_context){
	return;
      }

      if(audio_loop->application_context != NULL){
	g_object_unref(G_OBJECT(audio_loop->application_context));
      }

      if(application_context != NULL){
	g_object_ref(G_OBJECT(application_context));
      }
      
      audio_loop->application_context = (GObject *) application_context;
    }
    break;
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = g_value_get_object(value);

      if(audio_loop->soundcard != NULL){
	g_object_unref(audio_loop->soundcard);
      }

      if(soundcard != NULL){
	g_object_ref(soundcard);
      }

      audio_loop->soundcard = soundcard;
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
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, audio_loop->soundcard);
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

pthread_mutex_t*
ags_audio_loop_get_tree_lock(AgsMainLoop *main_loop)
{
  return(AGS_AUDIO_LOOP(main_loop)->tree_lock);
}

void
ags_audio_loop_set_async_queue(AgsMainLoop *main_loop, GObject *async_queue)
{
  if(AGS_AUDIO_LOOP(main_loop)->async_queue == async_queue){
    return;
  }

  if(AGS_AUDIO_LOOP(main_loop)->async_queue != NULL){
    g_object_unref(AGS_AUDIO_LOOP(main_loop)->async_queue);
  }
  
  if(async_queue != NULL){
    g_object_ref(async_queue);
  }
  
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

gboolean
ags_audio_loop_monitor(AgsMainLoop *main_loop,
		       guint time_cycle, guint *time_spent)
{
  if(g_atomic_int_get(&(AGS_AUDIO_LOOP(main_loop)->time_spent)) == 0){
    return(TRUE);
  }else{
    return(FALSE);
  }
}

void
ags_audio_loop_change_frequency(AgsMainLoop *main_loop,
				gdouble frequency)
{
  AgsThread *audio_loop, *thread;

  audio_loop = AGS_THREAD(main_loop);
  
  g_object_set(audio_loop,
	       "frequency\0", frequency,
	       NULL);

  /* reset soundcard thread */
  thread = audio_loop;

  while(ags_thread_find_type(thread, AGS_TYPE_SOUNDCARD_THREAD) != NULL){
    g_object_set(thread,
		 "frequency\0", frequency,
		 NULL);

    thread = g_atomic_pointer_get(&(thread->next));
  }

  /* reset sequencer thread */
  thread = audio_loop;

  while(ags_thread_find_type(thread, AGS_TYPE_SEQUENCER_THREAD) != NULL){
    g_object_set(thread,
		 "frequency\0", frequency,
		 NULL);

    thread = g_atomic_pointer_get(&(thread->next));
  }

  /* reset export thread */
  thread = audio_loop;

  while(ags_thread_find_type(thread, AGS_TYPE_EXPORT_THREAD) != NULL){
    g_object_set(thread,
		 "frequency\0", frequency,
		 NULL);

    thread = g_atomic_pointer_get(&(thread->next));
  }

  /* reset audio thread */
  thread = audio_loop;

  while(ags_thread_find_type(thread, AGS_TYPE_AUDIO_THREAD) != NULL){
    g_object_set(thread,
		 "frequency\0", frequency,
		 NULL);

    thread = g_atomic_pointer_get(&(thread->next));
  }

  /* reset channel thread */
  thread = audio_loop;

  while(ags_thread_find_type(thread, AGS_TYPE_CHANNEL_THREAD) != NULL){
    g_object_set(thread,
		 "frequency\0", frequency,
		 NULL);

    thread = g_atomic_pointer_get(&(thread->next));
  }
}

void
ags_audio_loop_dispose(GObject *gobject)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(gobject);

  /* application context */
  if(audio_loop->application_context != NULL){
    g_object_unref(audio_loop->application_context);

    audio_loop->application_context = NULL;
  }

  /* soundcard */
  if(audio_loop->soundcard != NULL){
    g_object_unref(audio_loop->soundcard);

    audio_loop->soundcard = NULL;
  }

  /* async queue */
  if(audio_loop->async_queue != NULL){
    g_object_unref(audio_loop->async_queue);

    audio_loop->async_queue = NULL;
  }

  /* unref AgsPlayback lists */
  if(audio_loop->play_recall != NULL){
    g_list_free_full(audio_loop->play_recall,
		     g_object_unref);
    
    audio_loop->play_recall = NULL;
  }

  if(audio_loop->play_channel != NULL){
    g_list_free_full(audio_loop->play_channel,
		     g_object_unref);
    
    audio_loop->play_channel = NULL;
  }

  if(audio_loop->play_audio != NULL){
    g_list_free_full(audio_loop->play_audio,
		     g_object_unref);
    
    audio_loop->play_audio = NULL;
  }
  
  /* call parent */
  G_OBJECT_CLASS(ags_audio_loop_parent_class)->dispose(gobject);
}

void
ags_audio_loop_finalize(GObject *gobject)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(gobject);

  /* application context */
  if(audio_loop->application_context != NULL){
    g_object_unref(audio_loop->application_context);
  }

  /* soundcard */
  if(audio_loop->soundcard != NULL){
    g_object_unref(audio_loop->soundcard);
  }

  /* async queue */
  if(audio_loop->async_queue != NULL){
    g_object_unref(audio_loop->async_queue);
  }

  /* tree lock and recall mutex */
  pthread_mutex_destroy(audio_loop->tree_lock);
  free(audio_loop->tree_lock);

  pthread_mutex_destroy(audio_loop->recall_mutex);
  free(audio_loop->recall_mutex);

  /* timing mutex and cond */
  pthread_mutex_destroy(audio_loop->timing_mutex);
  free(audio_loop->timing_mutex);

  pthread_cond_destroy(audio_loop->timing_cond);
  free(audio_loop->timing_cond);

  //FIXME:JK: destroy timing thread
  
  /* unref AgsPlayback lists */
  g_list_free_full(audio_loop->play_recall,
		   g_object_unref);
  
  g_list_free_full(audio_loop->play_channel,
		   g_object_unref);
  
  g_list_free_full(audio_loop->play_audio,
		   g_object_unref);

  /* call parent */
  G_OBJECT_CLASS(ags_audio_loop_parent_class)->finalize(gobject);
}

void
ags_audio_loop_start(AgsThread *thread)
{
  AgsAudioLoop *audio_loop;

  audio_loop = AGS_AUDIO_LOOP(thread);

  if((AGS_THREAD_SINGLE_LOOP & (g_atomic_int_get(&(thread->flags)))) == 0){
    /*  */
    AGS_THREAD_CLASS(ags_audio_loop_parent_class)->start(thread);

    //    pthread_create(audio_loop->timing_thread, NULL,
    //		   ags_audio_loop_timing_thread, audio_loop);
  }
}

void
ags_audio_loop_run(AgsThread *thread)
{
  AgsAudioLoop *audio_loop;

  AgsPollingThread *polling_thread;
  
  GMainContext *main_context;

  GPollFD *fds = NULL;  

  gint max_priority;
  gint timeout;
  gint nfds, allocated_nfds;
  gboolean some_ready;
  
  guint val;

  static gboolean initialized = FALSE;
  
  audio_loop = AGS_AUDIO_LOOP(thread);
  polling_thread = (AgsPollingThread *) ags_thread_find_type(thread,
							     AGS_TYPE_POLLING_THREAD);
  
  main_context = audio_loop->main_context;

  /* real-time setup */
  if((AGS_THREAD_RT_SETUP & (g_atomic_int_get(&(thread->flags)))) == 0){
    struct sched_param param;
    
    /* Declare ourself as a real time task */
    param.sched_priority = AGS_RT_PRIORITY;
      
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
      perror("sched_setscheduler failed\0");
    }

    g_atomic_int_or(&(thread->flags),
		    AGS_THREAD_RT_SETUP);

    g_main_context_push_thread_default(main_context);
  }

  /* reset polling thread */
  if(polling_thread != NULL){
    pthread_mutex_lock(polling_thread->fd_mutex);
      
    g_atomic_int_and(&(polling_thread->flags),
		     (~AGS_POLLING_THREAD_OMIT));
    
    pthread_mutex_unlock(polling_thread->fd_mutex);
  }
  
  /* wake-up timing thread */
  pthread_mutex_lock(audio_loop->timing_mutex);
  
  g_atomic_int_set(&(audio_loop->time_spent),
		   0);

  g_atomic_int_or(&(audio_loop->timing_flags),
		  AGS_AUDIO_LOOP_TIMING_WAKEUP);
  pthread_cond_signal(audio_loop->timing_cond);

  pthread_mutex_unlock(audio_loop->timing_mutex);

  /*  */
  pthread_mutex_lock(audio_loop->recall_mutex);
  
  /* play recall */
  if((AGS_AUDIO_LOOP_PLAY_RECALL & (audio_loop->flags)) != 0){
    ags_audio_loop_play_recall(audio_loop);

    if(audio_loop->play_recall_ref == 0 &&
       g_atomic_pointer_get(&(thread->start_queue)) == NULL){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_RECALL);
    }
  }

  /* play channel */
  if((AGS_AUDIO_LOOP_PLAY_CHANNEL & (audio_loop->flags)) != 0){
    ags_audio_loop_play_channel(audio_loop);

    if(audio_loop->play_channel_ref == 0 &&
       g_atomic_pointer_get(&(thread->start_queue)) == NULL){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_CHANNEL);
    }
  }

  /* play audio */
  if((AGS_AUDIO_LOOP_PLAY_AUDIO & (audio_loop->flags)) != 0){
    ags_audio_loop_play_audio(audio_loop);

    if(audio_loop->play_audio_ref == 0 &&
       g_atomic_pointer_get(&(thread->start_queue)) == NULL){
      audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_AUDIO);
    }
  }
  
  pthread_mutex_unlock(audio_loop->recall_mutex);

  /* decide if we stop */
  if(audio_loop->play_recall_ref == 0 &&
     audio_loop->play_channel_ref == 0 &&
     audio_loop->play_audio_ref == 0 &&
     audio_loop->play_notation_ref == 0){
    AgsThread *soundcard_thread;
    AgsThread *sequencer_thread;
    AgsThread *export_thread;

    /* soundcard thread */
    soundcard_thread = thread;

    while((soundcard_thread = ags_thread_find_type(soundcard_thread,
						   AGS_TYPE_SOUNDCARD_THREAD)) != NULL){
      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(soundcard_thread->flags)))) != 0){
	ags_thread_stop(soundcard_thread);
      }

      soundcard_thread = g_atomic_pointer_get(&(soundcard_thread->next));
    }

    /* sequencer thread */
    sequencer_thread = thread;

    while((sequencer_thread = ags_thread_find_type(sequencer_thread,
						   AGS_TYPE_SEQUENCER_THREAD)) != NULL){
      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(sequencer_thread->flags)))) != 0){
	ags_thread_stop(sequencer_thread);
      }

      sequencer_thread = g_atomic_pointer_get(&(sequencer_thread->next));
    }

    /* export thread */
    export_thread = thread;

    while((export_thread = ags_thread_find_type(export_thread,
						AGS_TYPE_EXPORT_THREAD)) != NULL){
      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(export_thread->flags)))) != 0){
	ags_thread_stop(export_thread);
      }

      export_thread = g_atomic_pointer_get(&(export_thread->next));
    }
  }
}

void
ags_audio_loop_notify_frequency(GObject *gobject,
				GParamSpec *pspec,
				gpointer user_data)
{
  AGS_AUDIO_LOOP(gobject)->time_cycle = NSEC_PER_SEC / AGS_THREAD(gobject)->freq;
}

void*
ags_audio_loop_timing_thread(void *ptr)
{
  AgsAudioLoop *audio_loop;

  AgsThread *thread;

  struct timespec idle;

  guint time_spent;
  
  pthread_mutex_t *timing_mutex;
  pthread_cond_t *timing_cond;
  
  audio_loop = AGS_AUDIO_LOOP(ptr);
  thread = (AgsThread *) audio_loop;

  timing_mutex = audio_loop->timing_mutex;
  timing_cond = audio_loop->timing_cond;
  
  while((AGS_THREAD_RUNNING & (g_atomic_int_get(&(thread->flags)))) != 0){
    pthread_mutex_lock(timing_mutex);

    if((AGS_AUDIO_LOOP_TIMING_WAKEUP & (g_atomic_int_get(&(audio_loop->timing_flags)))) == 0){
      g_atomic_int_or(&(audio_loop->timing_flags),
		      AGS_AUDIO_LOOP_TIMING_WAITING);
      
      while((AGS_AUDIO_LOOP_TIMING_WAKEUP & (g_atomic_int_get(&(audio_loop->timing_flags)))) == 0 &&
	    (AGS_AUDIO_LOOP_TIMING_WAITING & (g_atomic_int_get(&(audio_loop->timing_flags)))) != 0){
	pthread_cond_wait(timing_cond,
			  timing_mutex);
      }

      g_atomic_int_and(&(audio_loop->timing_flags),
		       ~(AGS_AUDIO_LOOP_TIMING_WAITING |
			 AGS_AUDIO_LOOP_TIMING_WAKEUP)); 
    }
    
    pthread_mutex_unlock(timing_mutex);

    idle.tv_sec = 0;
    idle.tv_nsec = audio_loop->time_cycle - 4000; //NOTE:JK: 4 usec tolerance
    
    nanosleep(&idle,
	      NULL);

    g_atomic_int_set(&(audio_loop->time_spent),
		     audio_loop->time_cycle);
    //    ags_main_loop_interrupt(AGS_MAIN_LOOP(thread),
    //			    AGS_THREAD_SUSPEND_SIG,
    //			    audio_loop->time_cycle, &time_spent);
    
    //    g_message("inter\0");    
  }
  
  pthread_exit(NULL);
}

/**
 * ags_audio_loop_play_recall:
 * @audio_loop: an #AgsAudioLoop
 *
 * Runs all recalls assigned with @audio_loop. You may want to use
 * #AgsAppendRecall task to add an #AgsRecall.
 *
 * Since: 0.4.0
 */
void
ags_audio_loop_play_recall(AgsAudioLoop *audio_loop)
{
  AgsPlayback *playback;
  AgsRecall *recall;

  GList *list, *list_next;

  guint stage;
  guint recall_flags;
  
  /* set flags */
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

  /*  */
  while(list != NULL){
    list_next = list->next;

    /* get playback fields */
    playback = AGS_PLAYBACK(list->data);
    recall = (AgsRecall *) playback->source;

    /* recall flags */
    //FIXME:JK: thread-unsafe
    recall_flags = recall->flags;
    
    /* initialize run or remove */
    if((AGS_RECALL_REMOVE & (recall_flags)) == 0){
      if((AGS_RECALL_HIDE & (recall_flags)) == 0){
	if(stage == 0){
	  ags_recall_run_pre(recall);
	}else if(stage == 1){
	  ags_recall_run_inter(recall);
	}else{
	  ags_recall_run_post(recall);
	}
      }
    }else{
      ags_audio_loop_remove_recall(audio_loop,
				   playback);
    }    

    /* iterate */
    list = list_next;
  }

  /* 3 stages */
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
 * Since: 0.4.0
 */
void
ags_audio_loop_play_channel(AgsAudioLoop *audio_loop)
{
  AgsPlayback *playback;
  AgsChannel *channel;

  AgsMutexManager *mutex_manager;

  GList *list_play, *list_next_play;

  gint stage;

  if(audio_loop->play_channel == NULL){
    if((AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING & (audio_loop->flags)) != 0){
      audio_loop->flags &= (~(AGS_AUDIO_LOOP_PLAY_CHANNEL |
			      AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING));
    }else{
      audio_loop->flags |= AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING;
    }
  }

  /* entry point */
  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_CHANNEL_TERMINATING);
  audio_loop->flags |= AGS_AUDIO_LOOP_PLAYING_CHANNEL;

  /* run the 3 stages */
  list_play = audio_loop->play_channel;
  
  while(list_play != NULL){
    list_next_play = list_play->next;
    
    playback = (AgsPlayback *) list_play->data;
    channel = (AgsChannel *) playback->source;
    
    /* play */
    if((AGS_PLAYBACK_REMOVE & (g_atomic_int_get(&(playback->flags)))) == 0){
      if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
	/* super threaded */
	ags_audio_loop_play_channel_super_threaded(audio_loop,
						   playback);
      }else{
	gboolean remove_play;

	/* not super threaded */
	remove_play = TRUE;
	    
	for(stage = 0; stage < 3; stage++){


	  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0 &&
	     playback->recall_id[0] != NULL){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel,
				       playback->recall_id[0],
				       stage);
	  }

	  if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0 &&
	     playback->recall_id[1] != NULL){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel,
				       playback->recall_id[1],
				       stage);
	  }

	  if((AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) != 0 &&
	     playback->recall_id[2] != NULL){
	    remove_play = FALSE;
	    ags_channel_recursive_play(channel,
				       playback->recall_id[2],
				       stage);
	  }
	}

	if(remove_play){
	  ags_audio_loop_remove_channel(audio_loop,
					(GObject *) channel);
	}	  
      }
    }

    /* iterate */
    list_play = list_next_play;
  }

  /* sync the 3 stages */
  list_play = audio_loop->play_channel;
  
  while(list_play != NULL){
    list_next_play = list_play->next;
    
    playback = (AgsPlayback *) list_play->data;

    /* sync */
    if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
      /* super threaded */
      ags_audio_loop_sync_channel_super_threaded(audio_loop,
						 playback);
    }

    /* iterate */
    list_play = list_next_play;
  }
}

void
ags_audio_loop_play_channel_super_threaded(AgsAudioLoop *audio_loop,
					   AgsPlayback *playback)
{
  AgsChannelThread *channel_thread;
  
  AgsThread *thread;
  
  /* playback */
  thread = playback->channel_thread[0];
  channel_thread = (AgsChannelThread *) thread;
  
  if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(thread->flags)))) != 0 &&
     (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(thread->flags)))) == 0){
    /* wakeup wait */
    pthread_mutex_lock(channel_thread->wakeup_mutex);

    g_atomic_int_and(&(channel_thread->flags),
		     (~AGS_CHANNEL_THREAD_WAIT));
	    
    if((AGS_CHANNEL_THREAD_DONE & (g_atomic_int_get(&(channel_thread->flags)))) == 0){
      pthread_cond_signal(channel_thread->wakeup_cond);
    }
	    
    pthread_mutex_unlock(channel_thread->wakeup_mutex);
  }
}

void
ags_audio_loop_sync_channel_super_threaded(AgsAudioLoop *audio_loop,
					   AgsPlayback *playback)
{
  AgsChannelThread *channel_thread;

  AgsThread *thread;

  gboolean do_playback;
  
  /* playback */
  thread = playback->channel_thread[0];
  channel_thread = (AgsChannelThread *) thread;
  
  do_playback = FALSE;
	
  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
    do_playback = TRUE;

    pthread_mutex_lock(channel_thread->done_mutex);
  
    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(thread->flags)))) != 0 &&
       (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(thread->flags)))) == 0 &&
       (AGS_THREAD_INITIAL_SYNC & (g_atomic_int_get(&(thread->flags)))) == 0){
      
      if((AGS_CHANNEL_THREAD_WAIT_SYNC & (g_atomic_int_get(&(channel_thread->flags)))) != 0){
	g_atomic_int_and(&(channel_thread->flags),
			 (~AGS_CHANNEL_THREAD_DONE_SYNC));
	
	while((AGS_CHANNEL_THREAD_DONE_SYNC & (g_atomic_int_get(&(channel_thread->flags)))) == 0 &&
	      (AGS_CHANNEL_THREAD_WAIT_SYNC & (g_atomic_int_get(&(channel_thread->flags)))) != 0){
	  pthread_cond_wait(channel_thread->done_cond,
			    channel_thread->done_mutex);
	}
      }
    }

    g_atomic_int_or(&(channel_thread->flags),
		    (AGS_CHANNEL_THREAD_WAIT_SYNC |
		     AGS_CHANNEL_THREAD_DONE_SYNC));

    pthread_mutex_unlock(channel_thread->done_mutex);
  }

  /*  */
  if(do_playback && playback->recall_id[0] == NULL){
    ags_thread_stop(thread);
    g_atomic_int_and(&(playback->flags),
		     (~AGS_PLAYBACK_PLAYBACK));
  }

  /* remove from playback */
  if(!do_playback){
    ags_thread_stop(thread);
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
 * Since: 0.4.0
 */
void
ags_audio_loop_play_audio(AgsAudioLoop *audio_loop)
{
  AgsAudio *audio;
  AgsChannel *output;
  AgsPlaybackDomain *playback_domain;
  AgsPlayback *playback;

  AgsMutexManager *mutex_manager;

  GObject *soundcard;
  
  GList *list_play_domain, *list_next_play_domain;
  GList *list_play;

  gint stage;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *audio_mutex;
  pthread_mutex_t *output_mutex;

  if(audio_loop->play_audio == NULL){
    if((AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING & (audio_loop->flags)) != 0){
      audio_loop->flags &= (~(AGS_AUDIO_LOOP_PLAY_AUDIO |
			      AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING));
    }else{
      audio_loop->flags |= AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING;
    }
  }

  audio_loop->flags &= (~AGS_AUDIO_LOOP_PLAY_AUDIO_TERMINATING);
  audio_loop->flags |= AGS_AUDIO_LOOP_PLAYING_AUDIO;

  /* get mutex manager and application mutex */
  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* playing */
  list_play_domain = audio_loop->play_audio;

  while(list_play_domain != NULL){
    list_next_play_domain = list_play_domain->next;
    
    playback_domain = (AgsPlaybackDomain *) list_play_domain->data;
    audio = (AgsAudio *) playback_domain->domain;
    
    /* get audio mutex */
    pthread_mutex_lock(application_mutex);
    
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) audio);
    
    pthread_mutex_unlock(application_mutex);

    /* get audio fields */
    pthread_mutex_lock(audio_mutex);

    output = audio->output;

    pthread_mutex_unlock(audio_mutex);

    /* play */
    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
      ags_audio_loop_play_audio_super_threaded(audio_loop,
					       playback_domain);
    }else{
      while(output != NULL){
	/* get output mutex */
	pthread_mutex_lock(application_mutex);
    
	output_mutex = ags_mutex_manager_lookup(mutex_manager,
						(GObject *) output);
    
	pthread_mutex_unlock(application_mutex);

	/* get playback */
	pthread_mutex_lock(output_mutex);
	
	playback = (AgsPlayback *) output->playback; // ags_playback_find_source(playback_domain->playback,
	//			   (GObject *) output);
      
	if(playback == NULL){
	  output = output->next;
	  
	  pthread_mutex_unlock(output_mutex);
	  
	  continue;
	}
	
	pthread_mutex_unlock(output_mutex);

	/* play */
	if((AGS_PLAYBACK_SUPER_THREADED_RECYCLING & (g_atomic_int_get(&(playback->flags)))) != 0){
	  //TODO:JK: implement me

	  /* super threaded recycling level */
	  if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0){
	    AGS_ITERATOR_THREAD(playback->iterator_thread[0])->flags |= AGS_ITERATOR_THREAD_DONE;
	    pthread_cond_signal(AGS_ITERATOR_THREAD(playback->iterator_thread[0])->tic_cond);
	  }

	  if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0){
	    AGS_ITERATOR_THREAD(playback->iterator_thread[1])->flags |= AGS_ITERATOR_THREAD_DONE;
	    pthread_cond_signal(AGS_ITERATOR_THREAD(playback->iterator_thread[1])->tic_cond);
	  }

	  if((AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) != 0){
	    AGS_ITERATOR_THREAD(playback->iterator_thread[2])->flags |= AGS_ITERATOR_THREAD_DONE;
	    pthread_cond_signal(AGS_ITERATOR_THREAD(playback->iterator_thread[2])->tic_cond);
	  }
	}else{
	  gboolean remove_domain;

	  /* not super threaded */
	  //	  pthread_mutex_lock(audio_mutex);
	  //	  pthread_mutex_lock(soundcard_mutex);    

	  remove_domain = TRUE;
	
	  /* run the 3 stages */
	  for(stage = 0; stage < 3; stage++){
	    if((AGS_PLAYBACK_REMOVE & (g_atomic_int_get(&(playback->flags)))) == 0){
	      if((AGS_PLAYBACK_PLAYBACK & (g_atomic_int_get(&(playback->flags)))) != 0 &&
		 playback->recall_id[0] != NULL){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[0], stage);
	      }

	      if((AGS_PLAYBACK_SEQUENCER & (g_atomic_int_get(&(playback->flags)))) != 0 &&
		 playback->recall_id[1] != NULL){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[1], stage);
	      }

	      if((AGS_PLAYBACK_NOTATION & (g_atomic_int_get(&(playback->flags)))) != 0 &&
		 playback->recall_id[2] != NULL){
		remove_domain = FALSE;
		ags_channel_recursive_play(output, playback->recall_id[2], stage);
	      }
	    }
	  }

	  /* check for removal */
	  if(remove_domain){
	    AgsChannel *channel;

	    ags_audio_loop_remove_audio(audio_loop,
					(GObject *) audio);
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
		  
	  //	  pthread_mutex_unlock(soundcard_mutex);
	  //	  pthread_mutex_unlock(audio_mutex);
	}

	/* iterate */
	pthread_mutex_lock(output_mutex);

	output = output->next;

	pthread_mutex_unlock(output_mutex);
      }
    }
    
    /* iterate */
    list_play_domain = list_next_play_domain;
  }
  
  /* sync - wait the 3 stages */
  list_play_domain = audio_loop->play_audio;

  while(list_play_domain != NULL){
    list_next_play_domain = list_play_domain->next;
        
    playback_domain = (AgsPlaybackDomain *) list_play_domain->data;

    /* get audio mutex */
    pthread_mutex_lock(application_mutex);
    
    audio_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) audio);
    
    pthread_mutex_unlock(application_mutex);

    /* get audio fields */
    pthread_mutex_lock(audio_mutex);

    output = audio->output;

    pthread_mutex_unlock(audio_mutex);

    /* sync */
    if((AGS_PLAYBACK_DOMAIN_SUPER_THREADED_AUDIO & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
      ags_audio_loop_sync_audio_super_threaded(audio_loop,
					       playback_domain);
    }else{
      while(output != NULL){
	/* get output mutex */
	pthread_mutex_lock(application_mutex);
    
	output_mutex = ags_mutex_manager_lookup(mutex_manager,
						(GObject *) output);
    
	pthread_mutex_unlock(application_mutex);

	/* get playback */
	pthread_mutex_lock(output_mutex);
	
	playback = (AgsPlayback *) output->playback; // ags_playback_find_source(playback_domain->playback,
	//			   (GObject *) output);
      
	if(playback == NULL){
	  output = output->next;
	  
	  pthread_mutex_unlock(output_mutex);
	  
	  continue;
	}
	
	pthread_mutex_unlock(output_mutex);

	/* sync */
	if((AGS_PLAYBACK_SUPER_THREADED_CHANNEL & (g_atomic_int_get(&(playback->flags)))) != 0){
	  ags_audio_loop_sync_channel_super_threaded(audio_loop,
						     playback);
	}else if((AGS_PLAYBACK_SUPER_THREADED_RECYCLING & (g_atomic_int_get(&(playback->flags)))) != 0){
	  //TODO:JK: implement me
	}

	/* iterate */
	pthread_mutex_lock(output_mutex);

	output = output->next;

	pthread_mutex_unlock(output_mutex);
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
  /* sequencer */
  if((AGS_PLAYBACK_DOMAIN_SEQUENCER & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    AgsAudioThread *audio_thread;

    AgsThread *thread;

    thread = playback_domain->audio_thread[1];

    if(thread != NULL){
      audio_thread = (AgsAudioThread *) thread;

      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(thread->flags)))) != 0 &&
	 (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(thread->flags)))) == 0){
	/* wakeup wait */
	pthread_mutex_lock(audio_thread->wakeup_mutex);
      
	g_atomic_int_and(&(audio_thread->flags),
			 (~AGS_AUDIO_THREAD_WAIT));
      
	if((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(audio_thread->flags)))) == 0){
	  pthread_cond_signal(audio_thread->wakeup_cond);
	}
      
	pthread_mutex_unlock(audio_thread->wakeup_mutex);
      }
    }
  }

  /* notation */
  if((AGS_PLAYBACK_DOMAIN_NOTATION & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    AgsAudioThread *audio_thread;

    AgsThread *thread;

    thread = playback_domain->audio_thread[2];

    if(thread != NULL){
      audio_thread = (AgsAudioThread *) thread;

      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(thread->flags)))) != 0 &&
	 (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(thread->flags)))) == 0){
	/* wakeup wait */
	pthread_mutex_lock(audio_thread->wakeup_mutex);
	  
	g_atomic_int_and(&(audio_thread->flags),
			 (~AGS_AUDIO_THREAD_WAIT));
	  
	if((AGS_AUDIO_THREAD_DONE & (g_atomic_int_get(&(audio_thread->flags)))) == 0){
	  pthread_cond_signal(audio_thread->wakeup_cond);
	}
	  
	pthread_mutex_unlock(audio_thread->wakeup_mutex);
      }
    }
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
  gboolean initial_sequencer, initial_notation;
  
  do_sequencer = FALSE;
  do_notation = FALSE;

  initial_sequencer = FALSE;
  initial_notation = FALSE;
  
  /* sequencer */
  if((AGS_PLAYBACK_DOMAIN_SEQUENCER & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    AgsAudioThread *audio_thread;

    audio_thread = AGS_AUDIO_THREAD(playback_domain->audio_thread[1]);
    do_sequencer = TRUE;

    pthread_mutex_lock(audio_thread->done_mutex);

    if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) != 0 &&
       (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(playback_domain->audio_thread[1]->flags)))) == 0){

      
      if((AGS_AUDIO_THREAD_WAIT_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) != 0){
	g_atomic_int_and(&(audio_thread->flags),
			 (~AGS_AUDIO_THREAD_DONE_SYNC));
	
      	while((AGS_AUDIO_THREAD_DONE_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) == 0 &&
	      (AGS_AUDIO_THREAD_WAIT_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) != 0){
	  pthread_cond_wait(audio_thread->done_cond,
			    audio_thread->done_mutex);
	}
      }
    }else{
      initial_sequencer = TRUE;
    }
    
    g_atomic_int_or(&(audio_thread->flags),
		    (AGS_AUDIO_THREAD_WAIT_SYNC |
		     AGS_AUDIO_THREAD_DONE_SYNC));
      	  
    pthread_mutex_unlock(audio_thread->done_mutex);
  }

  /* notation */
  if((AGS_PLAYBACK_DOMAIN_NOTATION & (g_atomic_int_get(&(playback_domain->flags)))) != 0){
    AgsAudioThread *audio_thread;

    audio_thread = AGS_AUDIO_THREAD(playback_domain->audio_thread[2]);

    if(audio_thread != NULL){
      do_notation = TRUE;

      pthread_mutex_lock(audio_thread->done_mutex);

      if((AGS_THREAD_RUNNING & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) != 0 &&
	 (AGS_THREAD_INITIAL_RUN & (g_atomic_int_get(&(playback_domain->audio_thread[2]->flags)))) == 0){

	if((AGS_AUDIO_THREAD_WAIT_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) != 0){

	  g_atomic_int_and(&(audio_thread->flags),
			   (~AGS_AUDIO_THREAD_DONE_SYNC));
	
	  while((AGS_AUDIO_THREAD_DONE_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) == 0 &&
		(AGS_AUDIO_THREAD_WAIT_SYNC & (g_atomic_int_get(&(audio_thread->flags)))) != 0){
	    pthread_cond_wait(audio_thread->done_cond,
			      audio_thread->done_mutex);
	  }
	}
      }else{
	initial_notation = TRUE;
      }
      
      g_atomic_int_or(&(audio_thread->flags),
		      (AGS_AUDIO_THREAD_WAIT_SYNC |
		       AGS_AUDIO_THREAD_DONE_SYNC));
    
      pthread_mutex_unlock(audio_thread->done_mutex);
    }
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

  if(!found_sequencer &&
     !initial_sequencer){
    ags_thread_stop(playback_domain->audio_thread[1]);
    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_SEQUENCER));
  }

  if(!found_notation &&
     !initial_notation){
    ags_thread_stop(playback_domain->audio_thread[2]);

    g_atomic_int_and(&(playback_domain->flags),
		     (~AGS_PLAYBACK_DOMAIN_NOTATION));
  }

  /* remove domain from playback */
  if(!found_sequencer && !found_notation &&
     !initial_sequencer && !initial_notation){
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
 * Since: 0.4.0
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
 * Since: 0.4.0
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
 * Since: 0.4.0
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
 * Since: 0.4.0
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
 * @playback: an #AgsRecall
 *
 * Add recall for playback.
 *
 * Since: 0.4.0
 */
void
ags_audio_loop_add_recall(AgsAudioLoop *audio_loop, gpointer playback)
{
  if(g_list_find(audio_loop->play_recall,
		 playback) == NULL){
    /* append to GObject */
    AGS_PLAYBACK(playback)->flags &= (~AGS_PLAYBACK_REMOVE);
    audio_loop->play_recall = g_list_append(audio_loop->play_recall,
					    playback);
    audio_loop->play_recall_ref += 1;
  }
}

/**
 * ags_audio_loop_remove_recall:
 * @audio_loop: the #AgsAudioLoop
 * @playback: an #AgsRecall
 *
 * Remove recall of playback.
 *
 * Since: 0.4.0
 */
void
ags_audio_loop_remove_recall(AgsAudioLoop *audio_loop, gpointer playback)
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
 * @soundcard: the #GObject
 * @application_context: the #AgsMain
 *
 * Create a new #AgsAudioLoop.
 *
 * Returns: the new #AgsAudioLoop
 *
 * Since: 0.4.0
 */
AgsAudioLoop*
ags_audio_loop_new(GObject *soundcard, GObject *application_context)
{
  AgsAudioLoop *audio_loop;

  audio_loop = (AgsAudioLoop *) g_object_new(AGS_TYPE_AUDIO_LOOP,
					     "soundcard\0", soundcard,
					     NULL);

  if(application_context != NULL){
    g_object_ref(G_OBJECT(application_context));
    audio_loop->application_context = application_context;
  }

  return(audio_loop);
}
