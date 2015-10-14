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
#include <ags/main.h>

#include <ags/object/ags_connectable.h>

#include <ags/object/ags_main_loop.h>

#include <ags/plugin/ags_ladspa_manager.h>

#include <ags/file/ags_file.h>

#include <ags/thread/ags_audio_loop.h>
#include <ags/thread/ags_task_thread.h>
#include <ags/thread/ags_devout_thread.h>
#include <ags/thread/ags_export_thread.h>
#include <ags/thread/ags_audio_thread.h>
#include <ags/thread/ags_channel_thread.h>
#include <ags/thread/ags_gui_thread.h>
#include <ags/thread/ags_autosave_thread.h>
#include <ags/thread/ags_single_thread.h>

#include <ags/object/ags_config.h>

#include <sys/mman.h>

#include <gtk/gtk.h>

#include <stdlib.h>
#include <libintl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <mcheck.h>
#include <signal.h>
#include <time.h>

#include <gdk/gdk.h>

#include <sys/types.h>
#include <pwd.h>

#include "config.h"

void ags_signal_handler(int signr);
void ags_signal_handler_timer(int sig, siginfo_t *si, void *uc);
static void ags_signal_cleanup();

extern void ags_thread_resume_handler(int sig);
extern void ags_thread_suspend_handler(int sig);

static sigset_t ags_wait_mask;
static sigset_t ags_timer_mask;

struct sigaction ags_sigact;
struct sigaction ags_sigact_timer;

struct sigevent ags_sev_timer;
struct itimerspec its;

void
ags_signal_handler(int signr)
{
  if(signr == SIGINT){
    //TODO:JK: do backup
    
    exit(-1);
  }else{
    sigemptyset(&(ags_sigact.sa_mask));

    //    if(signr == AGS_ASYNC_QUEUE_SIGNAL_HIGH){
      // pthread_yield();
    //    }
  }
}

void
ags_signal_handler_timer(int sig, siginfo_t *si, void *uc)
{
  pthread_mutex_lock(application_context->main_loop->timer_mutex);

  g_atomic_int_set(&(application_context->main_loop->timer_expired),
		   TRUE);
  
  if(application_context->main_loop->timer_wait){
    pthread_cond_signal(application_context->main_loop->timer_cond);
  }
  
  pthread_mutex_unlock(application_context->main_loop->timer_mutex);

  //  g_message("sig\0");
  //  signal(sig, SIG_IGN);
}

static void
ags_signal_cleanup()
{
  sigemptyset(&(ags_sigact.sa_mask));
}

int
main(int argc, char **argv)
{
  AgsDevout *devout;
  AgsWindow *window;
  AgsThread *gui_thread;
  AgsThread *async_queue;
  AgsThread *devout_thread;
  AgsThread *export_thread;
  
  GFile *autosave_file;
  struct sched_param param;
  struct rlimit rl;
  timer_t timerid;
  gchar *filename, *autosave_filename;

  struct passwd *pw;
  uid_t uid;
  gchar *wdir, *config_file;
  int result;
  gboolean single_thread = FALSE;
  guint i;

  const char *error;
  const rlim_t kStackSize = 256L * 1024L * 1024L;   // min stack size = 128 Mb

  pthread_mutexattr_t attr;
  
  //  mtrace();
  atexit(ags_signal_cleanup);

  result = getrlimit(RLIMIT_STACK, &rl);

  /* set stack size 64M */
  if(result == 0){
    if(rl.rlim_cur < kStackSize){
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);

      if(result != 0){
	//TODO:JK
      }
    }
  }

  /* Ignore interactive and job-control signals.  */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);

  ags_sigact.sa_handler = ags_signal_handler;
  sigemptyset(&ags_sigact.sa_mask);
  ags_sigact.sa_flags = 0;
  sigaction(SIGINT, &ags_sigact, (struct sigaction *) NULL);
  sigaction(SA_RESTART, &ags_sigact, (struct sigaction *) NULL);

  /**/
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr,
			    PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&ags_application_mutex,
		     &attr);

#ifdef AGS_USE_TIMER
  /* create timer */
  ags_sigact_timer.sa_flags = SA_SIGINFO;
  ags_sigact_timer.sa_sigaction = ags_signal_handler_timer;
  sigemptyset(&ags_sigact_timer.sa_mask);
  
  if(sigaction(SIGRTMIN, &ags_sigact_timer, NULL) == -1){
    perror("sigaction\0");
    exit(EXIT_FAILURE);
  }
  
  /* Block timer signal temporarily */
  sigemptyset(&ags_timer_mask);
  sigaddset(&ags_timer_mask, SIGRTMIN);
  
  if(sigprocmask(SIG_SETMASK, &ags_timer_mask, NULL) == -1){
    perror("sigprocmask\0");
    exit(EXIT_FAILURE);
  }

  /* Create the timer */
  ags_sev_timer.sigev_notify = SIGEV_SIGNAL;
  ags_sev_timer.sigev_signo = SIGRTMIN;
  ags_sev_timer.sigev_value.sival_ptr = &timerid;
  
  if(timer_create(CLOCK_MONOTONIC, &ags_sev_timer, &timerid) == -1){
    perror("timer_create\0");
    exit(EXIT_FAILURE);
  }
#endif

  /* parse gtkrc */
  uid = getuid();
  pw = getpwuid(uid);
  
  gtk_rc_parse(g_strdup_printf("%s/%s/ags.rc",
			       pw->pw_dir,
			       AGS_DEFAULT_DIRECTORY));
  
  /**/
  LIBXML_TEST_VERSION;

  g_thread_init(NULL);
  gdk_threads_init();

  gtk_init(&argc, &argv);
  ipatch_init();

  ao_initialize();

  filename = NULL;

  for(i = 0; i < argc; i++){
    if(!strncmp(argv[i], "--help\0", 7)){
      printf("GSequencer is an audio sequencer and notation editor\n\n\0");

      printf("Usage:\n\t%s\n\t%s\n\t%s\n\t%s\n\n",
	     "Report bugs to <jkraehemann@gmail.com>\n\0",
	     "--filename file     open file\0",
	     "--single-thread     run in single thread mode\0",     
	     "--help              display this help and exit\0",
	     "--version           output version information and exit\0");
      
      exit(0);
    }else if(!strncmp(argv[i], "--version\0", 10)){
      printf("GSequencer 0.4.2\n\n\0");
      
      printf("%s\n%s\n%s\n\n\0",
	     "Copyright (C) 2005-2015 Joël Krähemann\0",
	     "This is free software; see the source for copying conditions.  There is NO\0",
	     "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\0");
      
      printf("Written by Joël Krähemann\n\0");
      exit(0);
    }else if(!strncmp(argv[i], "--single-thread\0", 16)){
      single_thread = TRUE;
    }else if(!strncmp(argv[i], "--filename\0", 11)){
      filename = argv[i + 1];
      i++;
    }
  }

  config = ags_config_new();
  
  wdir = g_strdup_printf("%s/%s\0",
			 pw->pw_dir,
			 AGS_DEFAULT_DIRECTORY);

  config_file = g_strdup_printf("%s/%s\0",
				wdir,
				AGS_DEFAULT_CONFIG);

  ags_config_load_from_file(config,
			    config_file);

  g_free(wdir);
  g_free(config_file);

  if(filename != NULL){
    AgsFile *file;

    file = g_object_new(AGS_TYPE_FILE,
			"filename\0", filename,
			NULL);
    ags_file_open(file);
    ags_file_read(file);

    application_context = AGS_MAIN(file->application_context);
    ags_file_close(file);

    ags_thread_start(application_context->main_loop);

    /* complete thread pool */
    application_context->thread_pool->parent = AGS_THREAD(application_context->main_loop);
    ags_thread_pool_start(application_context->thread_pool);

#ifdef AGS_USE_TIMER
    /* Start the timer */
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = NSEC_PER_SEC / AGS_THREAD_MAX_PRECISION;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if(timer_settime(timerid, 0, &its, NULL) == -1){
      perror("timer_settime\0");
      exit(EXIT_FAILURE);
    
    }
    
    if(sigprocmask(SIG_UNBLOCK, &ags_timer_mask, NULL) == -1){
      perror("sigprocmask\0");
      exit(EXIT_FAILURE);
    }
#endif

    gui_thread = ags_thread_find_type(application_context->main_loop,
				      AGS_TYPE_GUI_THREAD);
    
#ifdef _USE_PTH
    pth_join(gui_thread->thread,
	     NULL);
#else
    pthread_join(*(gui_thread->thread),
		 NULL);
#endif
  }else{
    application_context = application_context_new();

    if(single_thread){
      application_context->flags = AGS_MAIN_SINGLE_THREAD;
    }

    /* Declare ourself as a real time task */
    param.sched_priority = AGS_PRIORITY;

    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
      perror("sched_setscheduler failed\0");
    }

    mlockall(MCL_CURRENT | MCL_FUTURE);

    if((AGS_MAIN_SINGLE_THREAD & (application_context->flags)) == 0){
      //      GdkFrameClock *frame_clock;

#ifdef AGS_WITH_XMLRPC_C
      AbyssInit(&error);

      xmlrpc_env_init(&(application_context->env));
#endif /* AGS_WITH_XMLRPC_C */

      /* AgsDevout */
      devout = ags_devout_new((GObject *) application_context);
      application_context_add_devout(application_context,
			  devout);

      /*  */
      g_object_set(G_OBJECT(application_context->autosave_thread),
		   "devout\0", devout,
		   NULL);

      /* AgsWindow */
      application_context->window =
	window = ags_window_new((GObject *) application_context);
      g_object_set(G_OBJECT(window),
		   "devout\0", devout,
		   NULL);
      g_object_ref(G_OBJECT(window));

      gtk_window_set_default_size((GtkWindow *) window, 500, 500);
      gtk_paned_set_position((GtkPaned *) window->paned, 300);

      ags_connectable_connect(window);
      gtk_widget_show_all((GtkWidget *) window);

      /* AgsServer */
      application_context->server = ags_server_new((GObject *) application_context);

      /* AgsMainLoop */
      application_context->main_loop = (AgsThread *) ags_audio_loop_new((GObject *) devout, (GObject *) application_context);
      application_context->thread_pool->parent = application_context->main_loop;
      g_object_ref(G_OBJECT(application_context->main_loop));
      ags_connectable_connect(AGS_CONNECTABLE(G_OBJECT(application_context->main_loop)));

      /* AgsTaskThread */
      async_queue = (AgsThread *) ags_task_thread_new(devout);
      AGS_TASK_THREAD(async_queue)->thread_pool = application_context->thread_pool;
      ags_main_loop_set_async_queue(AGS_MAIN_LOOP(application_context->main_loop),
				    async_queue);
      ags_thread_add_child_extended(application_context->main_loop,
				    async_queue,
				    TRUE, TRUE);

      /* AgsGuiThread */
      gui_thread = (AgsThread *) ags_gui_thread_new();
      ags_thread_add_child_extended(application_context->main_loop,
				    gui_thread,
				    TRUE, TRUE);

      /* AgsDevoutThread */
      devout_thread = (AgsThread *) ags_devout_thread_new(devout);
      ags_thread_add_child_extended(application_context->main_loop,
				    devout_thread,
				    TRUE, TRUE);

      /* AgsExportThread */
      export_thread = (AgsThread *) ags_export_thread_new(devout, NULL);
      ags_thread_add_child_extended(application_context->main_loop,
				    export_thread,
				    TRUE, TRUE);

      /* start thread tree */
      ags_thread_start(application_context->main_loop);
      ags_thread_start(gui_thread);

      /* wait thread */
      pthread_mutex_lock(AGS_THREAD(application_context->main_loop)->start_mutex);

      g_atomic_int_set(&(AGS_THREAD(application_context->main_loop)->start_wait),
		       TRUE);
	
      if(g_atomic_int_get(&(AGS_THREAD(application_context->main_loop)->start_wait)) == TRUE &&
	 g_atomic_int_get(&(AGS_THREAD(application_context->main_loop)->start_done)) == FALSE){
	while(g_atomic_int_get(&(AGS_THREAD(application_context->main_loop)->start_wait)) == TRUE &&
	      g_atomic_int_get(&(AGS_THREAD(application_context->main_loop)->start_done)) == FALSE){
	  pthread_cond_wait(AGS_THREAD(application_context->main_loop)->start_cond,
			    AGS_THREAD(application_context->main_loop)->start_mutex);
	}
      }
	
      pthread_mutex_unlock(AGS_THREAD(application_context->main_loop)->start_mutex);

      /* complete thread pool */
      ags_thread_pool_start(application_context->thread_pool);
    }else{
      AgsSingleThread *single_thread;

      devout = ags_devout_new((GObject *) application_context);
      application_context_add_devout(application_context,
			  devout);

      g_object_set(G_OBJECT(application_context->autosave_thread),
		   "devout\0", devout,
		   NULL);

      /* threads */
      single_thread = ags_single_thread_new((GObject *) devout);

      /* AgsWindow */
      application_context->window = 
	window = ags_window_new((GObject *) application_context);
      g_object_set(G_OBJECT(window),
		   "devout\0", devout,
		   NULL);
      gtk_window_set_default_size((GtkWindow *) window, 500, 500);
      gtk_paned_set_position((GtkPaned *) window->paned, 300);

      ags_connectable_connect(window);
      gtk_widget_show_all((GtkWidget *) window);

      /* AgsMainLoop */
      application_context->main_loop = AGS_MAIN_LOOP(ags_audio_loop_new((GObject *) devout, (GObject *) application_context));
      g_object_ref(G_OBJECT(application_context->main_loop));

      /* complete thread pool */
      application_context->thread_pool->parent = AGS_THREAD(application_context->main_loop);
      ags_thread_pool_start(application_context->thread_pool);

      /* start thread tree */
      ags_thread_start((AgsThread *) single_thread);
    }

#ifdef AGS_USE_TIMER
    /* Start the timer */
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = NSEC_PER_SEC / AGS_THREAD_MAX_PRECISION; // / AGS_AUDIO_LOOP_DEFAULT_JIFFIE;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if(timer_settime(timerid, 0, &its, NULL) == -1){
      perror("timer_settime\0");
      exit(EXIT_FAILURE);
    
    }
    
    if(sigprocmask(SIG_UNBLOCK, &ags_timer_mask, NULL) == -1){
      perror("sigprocmask\0");
      exit(EXIT_FAILURE);
    }
#endif

    gui_thread = ags_thread_find_type(application_context->main_loop,
				      AGS_TYPE_GUI_THREAD);

    if(!single_thread){
      /* join gui thread */
#ifdef _USE_PTH
      pth_join(gui_thread->thread,
	       NULL);
#else
      pthread_join(*(gui_thread->thread),
		   NULL);
#endif
    }
  }

  /* free managers */
  if(ags_ladspa_manager != NULL){
    g_object_unref(ags_ladspa_manager_get_instance());
  }

  uid = getuid();
  pw = getpwuid(uid);
  
  autosave_filename = g_strdup_printf("%s/%s/%d-%s\0",
				      pw->pw_dir,
				      AGS_DEFAULT_DIRECTORY,
				      getpid(),
				      AGS_AUTOSAVE_THREAD_DEFAULT_FILENAME);
  
  autosave_file = g_file_new_for_path(autosave_filename);

  if(g_file_query_exists(autosave_file,
			 NULL)){
    g_file_delete(autosave_file,
		  NULL,
		  NULL);
  }

  g_free(autosave_filename);
  g_object_unref(autosave_file);

  //  muntrace();

  return(0);
}
