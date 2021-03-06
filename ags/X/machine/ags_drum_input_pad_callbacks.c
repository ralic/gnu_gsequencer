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

#include <ags/X/machine/ags_drum_input_pad_callbacks.h>
#include <ags/X/machine/ags_drum_input_line_callbacks.h>
#include <ags/X/machine/ags_drum.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>
#include <ags/thread/ags_task_thread.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_audio_signal.h>
#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_pattern.h>
#include <ags/audio/ags_recall.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/audio/task/ags_start_soundcard.h>
#include <ags/audio/task/ags_init_channel.h>
#include <ags/audio/task/ags_append_channel.h>
#include <ags/audio/task/ags_append_recall.h>
#include <ags/audio/task/ags_add_audio_signal.h>
#include <ags/audio/task/ags_open_single_file.h>
#include <ags/audio/task/ags_cancel_channel.h>

#include <ags/audio/recall/ags_play_channel_run.h>
#include <ags/audio/recall/ags_play_audio_signal.h>
#include <ags/audio/recall/ags_stream_audio_signal.h>

#include <ags/audio/file/ags_audio_file.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_line_callbacks.h>

#include <math.h>

void ags_drum_input_pad_open_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *pad);
void ags_drum_input_pad_open_play_done(AgsRecall *recall, AgsDrumInputPad *drum_input_pad);
void ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *pad);

void ags_drum_input_pad_init_channel_launch_callback(AgsTask *task, AgsDrumInputPad *drum_input_pad);

#define AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME "AgsDrumInputPadOpenAudioFileName\0"
#define AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON "AgsDrumInputPadOpenSpinButton\0"

void
ags_drum_input_pad_open_callback(GtkWidget *widget, AgsDrumInputPad *drum_input_pad)
{
  GtkFileChooserDialog *file_chooser;
  GtkHBox *hbox;
  GtkLabel *label;
  GtkSpinButton *spin_button;
  GtkToggleButton *play;

  if(drum_input_pad->file_chooser != NULL)
    return;

  drum_input_pad->file_chooser =
    file_chooser = (GtkFileChooserDialog *) gtk_file_chooser_dialog_new (g_strdup("Open File\0"),
									 (GtkWindow *) gtk_widget_get_toplevel((GtkWidget *) drum_input_pad),
									 GTK_FILE_CHOOSER_ACTION_OPEN,
									 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									 NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),
				       FALSE);
  g_object_set_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE), NULL);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME, NULL);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE, 0);
  gtk_file_chooser_set_extra_widget((GtkFileChooser *) file_chooser,
				    (GtkWidget *) hbox);
  
  label = (GtkLabel *) gtk_label_new(g_strdup("channel: \0"));
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) label,
		     FALSE, FALSE,
		     0);

  spin_button = (GtkSpinButton *) gtk_spin_button_new_with_range(0.0, AGS_AUDIO(AGS_PAD(drum_input_pad)->channel->audio)->audio_channels - 1, 1.0);
  g_object_set_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON, spin_button);
  gtk_box_pack_start((GtkBox *) hbox,
		     (GtkWidget *) spin_button,
		     FALSE, FALSE,
		     0);

  if(drum_input_pad->pad.group->active)
    gtk_widget_set_sensitive((GtkWidget *) spin_button,
			     FALSE);

  //  play = (GtkToggleButton *) g_object_new(GTK_TYPE_TOGGLE_BUTTON,
  //					  "label\0", GTK_STOCK_MEDIA_PLAY,
  //					  "use-stock\0", TRUE,
  //					  "use-underline\0", TRUE,
  //					  NULL);
  gtk_box_pack_start((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, FALSE, FALSE, 0);
  gtk_box_reorder_child((GtkBox *) GTK_DIALOG(file_chooser)->action_area, (GtkWidget *) play, 0);

  gtk_widget_show_all((GtkWidget *) file_chooser);

  g_signal_connect((GObject *) file_chooser, "response\0",
		   G_CALLBACK(ags_drum_input_pad_open_response_callback), (gpointer) drum_input_pad);
  //  g_signal_connect((GObject *) play, "toggled\0",
  //		   G_CALLBACK(ags_drum_input_pad_open_play_callback), (gpointer) drum_input_pad);
}

void
ags_drum_input_pad_open_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  AgsWindow *window;
  GtkFileChooserDialog *file_chooser;
  
  GObject *soundcard;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsTaskThread *task_thread;
  
  AgsApplicationContext *application_context;
  
  GList *list, *tasks;
  gchar *name0, *name1;

  pthread_mutex_t *application_mutex;
  
  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad,
						 AGS_TYPE_WINDOW);
  
  file_chooser = drum_input_pad->file_chooser;
  name0 = (gchar *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
  name1 = g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME);

  soundcard = AGS_AUDIO(AGS_PAD(drum_input_pad)->channel->audio)->soundcard;

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);
  
  /* get main loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* find task thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type(main_loop,
						       AGS_TYPE_TASK_THREAD);
		  
  if(toggle_button->active){
    AgsPlayback *playback;

    AgsPlayAudioSignal *play_audio_signal;
    AgsStreamAudioSignal *stream_audio_signal;

    AgsStartSoundcard *start_soundcard;
    AgsAppendRecall *append_recall;

    AgsAudioFile *audio_file;

    GList *audio_signal;
    
    guint pcm_channels;
    guint i;

    drum_input_pad->flags &= (~AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE);

    /* AgsAudioFile */
    if(!g_strcmp0(name0, name1)){
      audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
    }else{
      if(name1 != NULL){
	audio_file = g_object_get_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE));
	g_object_unref(G_OBJECT(audio_file));
      }

      ags_soundcard_get_presets(AGS_SOUNDCARD(soundcard),
				&pcm_channels,
				NULL,
				NULL,
				NULL);
      
      audio_file = ags_audio_file_new(name0,
				      soundcard,
				      0, pcm_channels);
      g_object_set_data((GObject *) file_chooser, (char *) g_type_name(AGS_TYPE_AUDIO_FILE), audio_file);

      ags_audio_file_open(audio_file);
      ags_audio_file_read_audio_signal(audio_file);
      g_message("ags_drum_input_pad_open_play:\0");
    }

    /* task */
    audio_signal = audio_file->audio_signal;

    tasks = NULL;
    i = 0;

    while(audio_signal != NULL){
      /* AgsPlayAudioSignal recall */
      play_audio_signal = ags_play_audio_signal_new(AGS_AUDIO_SIGNAL(audio_signal->data),
						    soundcard,
						    i);
      g_object_ref(play_audio_signal);
      AGS_AUDIO_SIGNAL(audio_signal->data)->flags &= (~AGS_AUDIO_SIGNAL_TEMPLATE);
      AGS_AUDIO_SIGNAL(audio_signal->data)->stream_current = AGS_AUDIO_SIGNAL(audio_signal->data)->stream_beginning;
      drum_input_pad->pad_open_play_ref++;

      drum_input_pad->pad_open_recalls = g_list_prepend(drum_input_pad->pad_open_recalls,
							play_audio_signal);
      g_signal_connect(G_OBJECT(play_audio_signal), "done\0",
		       G_CALLBACK(ags_drum_input_pad_open_play_done), drum_input_pad);

      /* AgsAppendRecall */
      playback = ags_playback_new();

      playback->flags = AGS_PLAYBACK_PLAYBACK;
      playback->source = G_OBJECT(play_audio_signal);

      append_recall = ags_append_recall_new(G_OBJECT(application_context->main_loop),
					    (GObject *) playback);
    
      tasks = g_list_prepend(tasks, append_recall);
      
      /* AgsStreamAudioSignal recall */
      stream_audio_signal = ags_stream_audio_signal_new(AGS_AUDIO_SIGNAL(audio_signal->data));
      g_object_ref(stream_audio_signal);
      drum_input_pad->pad_open_play_ref++;

      drum_input_pad->pad_open_recalls = g_list_prepend(drum_input_pad->pad_open_recalls,
							stream_audio_signal);
      g_signal_connect_after(G_OBJECT(stream_audio_signal), "done\0",
			     G_CALLBACK(ags_drum_input_pad_open_play_done), drum_input_pad);

      /* AgsAppendRecall */
      playback = ags_playback_new();

      playback->flags = AGS_PLAYBACK_PLAYBACK;
      playback->source = G_OBJECT(stream_audio_signal);

      append_recall = ags_append_recall_new(G_OBJECT(application_context->main_loop),
					    (GObject *) playback);
    
      tasks = g_list_prepend(tasks, append_recall);

      /* next */
      audio_signal = audio_signal->next;
      i++;
    }

    /*  */
    tasks = g_list_reverse(tasks);

    /* create start task */
    start_soundcard = ags_start_soundcard_new(application_context);
    tasks = g_list_prepend(tasks, start_soundcard);

    ags_task_thread_append_tasks(task_thread,
				 tasks);
  }else{
    if((AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE & (drum_input_pad->flags)) == 0){
      GList *list;

      list = drum_input_pad->pad_open_recalls;

      while(list != NULL){
	ags_recall_cancel(AGS_RECALL(list->data));

	list = list->next;
      }
    }else{
      drum_input_pad->flags &= (~AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE);
    }
  }
}

void
ags_drum_input_pad_open_play_done(AgsRecall *recall, AgsDrumInputPad *drum_input_pad)
{
  drum_input_pad->pad_open_play_ref--;

  //  ags_recycling_remove_audio_signal(AGS_RECALL_AUDIO_SIGNAL(recall)->source->recycling,
  //				    AGS_RECALL_AUDIO_SIGNAL(recall)->source);

  if(drum_input_pad->pad_open_play_ref == 0){
    GtkToggleButton *toggle_button;
    //    AgsCancelRecall *cancel_recall;
    GList *list;

    list = drum_input_pad->pad_open_recalls;

    while(list != NULL){
      AGS_RECALL(list->data)->flags |= AGS_RECALL_REMOVE;
    
      list = list->next;
    }
    
    g_list_free(drum_input_pad->pad_open_recalls);
    drum_input_pad->pad_open_recalls = NULL;

    list = gtk_container_get_children((GtkContainer *) GTK_DIALOG(drum_input_pad->file_chooser)->action_area);
    toggle_button = (GtkToggleButton *) list->data;

    drum_input_pad->flags |= AGS_DRUM_INPUT_PAD_OPEN_PLAY_DONE;
    gtk_toggle_button_set_active(toggle_button, FALSE);

    g_list_free(list);
  }
}

void
ags_drum_input_pad_open_response_callback(GtkWidget *widget, gint response, AgsDrumInputPad *drum_input_pad)
{
  AgsWindow *window;
  AgsDrum *drum;

  GtkFileChooserDialog *file_chooser;
  GtkSpinButton *spin_button;

  AgsAudioFile *audio_file;

  AgsOpenSingleFile *open_single_file;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;
  AgsTaskThread *task_thread;
  
  AgsApplicationContext *application_context;
  
  char *name0, *name1;

  pthread_mutex_t *application_mutex;

  drum = (AgsDrum *) gtk_widget_get_ancestor(GTK_WIDGET(drum_input_pad),
					     AGS_TYPE_DRUM);

  window = (AgsWindow *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad,
						 AGS_TYPE_WINDOW);

  file_chooser = drum_input_pad->file_chooser;

  
  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get main loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;

  pthread_mutex_unlock(application_mutex);

  /* find task thread */
  task_thread = (AgsTaskThread *) ags_thread_find_type(main_loop,
						       AGS_TYPE_TASK_THREAD);

  if(response == GTK_RESPONSE_ACCEPT){
    name0 = gtk_file_chooser_get_filename((GtkFileChooser *) file_chooser);
    name1 = (char *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_AUDIO_FILE_NAME);

    spin_button = (GtkSpinButton *) g_object_get_data((GObject *) file_chooser, AGS_DRUM_INPUT_PAD_OPEN_SPIN_BUTTON);

    /* open audio file and read audio signal */
    if(g_strcmp0(name0, name1)){
      if(name1 != NULL){
	audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));
	g_object_unref(G_OBJECT(audio_file));
      }
    }

    /* task */
    if(AGS_PAD(drum_input_pad)->group->active){
      open_single_file = ags_open_single_file_new(AGS_PAD(drum_input_pad)->channel,
						  AGS_AUDIO(AGS_MACHINE(drum)->audio)->soundcard,
						  name0,
						  0, AGS_AUDIO(AGS_MACHINE(drum)->audio)->audio_channels);
    }else{
      AgsLine *line;
      GList *list;
      
      list = gtk_container_get_children(GTK_CONTAINER(AGS_PAD(drum_input_pad)->expander_set));
      line = AGS_LINE(ags_line_find_next_grouped(list)->data);

      open_single_file = ags_open_single_file_new(line->channel,
						  AGS_AUDIO(AGS_MACHINE(drum)->audio)->soundcard,
						  name0,
						  (guint) spin_button->adjustment->value, 1);

      g_list_free(list);
    }

    ags_task_thread_append_task(task_thread,
				AGS_TASK(open_single_file));

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }else if(response == GTK_RESPONSE_CANCEL){
    audio_file = (AgsAudioFile *) g_object_get_data((GObject *) file_chooser, g_type_name(AGS_TYPE_AUDIO_FILE));

    if(audio_file != NULL){
      g_object_unref(G_OBJECT(audio_file));
    }

    gtk_widget_destroy((GtkWidget *) file_chooser);
  }

  drum_input_pad->file_chooser = NULL;
}

void
ags_drum_input_pad_play_callback(GtkToggleButton *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  ags_pad_play(AGS_PAD(drum_input_pad));
}

void
ags_drum_input_pad_edit_callback(GtkWidget *toggle_button, AgsDrumInputPad *drum_input_pad)
{
  GtkToggleButton *toggle;
  AgsDrum *drum;

  drum = (AgsDrum *) gtk_widget_get_ancestor((GtkWidget *) drum_input_pad,
					     AGS_TYPE_DRUM);

  if(drum->selected_edit_button != NULL){
    if(GTK_TOGGLE_BUTTON(toggle_button) != drum->selected_edit_button){
      /* unset old */
      toggle = drum->selected_edit_button;

      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, FALSE);

      /* apply new */
      drum->selected_edit_button = (GtkToggleButton *) toggle_button;
      drum->selected_pad = (AgsDrumInputPad *) gtk_widget_get_ancestor((GtkWidget *) toggle_button,
								       AGS_TYPE_DRUM_INPUT_PAD);

      AGS_MACHINE(drum)->selected_input_pad = (GtkWidget *) drum->selected_pad;
      
      ags_pattern_box_set_pattern(drum->pattern_box);
    }else{
      /* chain up */
      toggle = drum->selected_edit_button;
      
      drum->selected_edit_button = NULL;
      gtk_toggle_button_set_active((GtkToggleButton *) toggle, TRUE);

      /* reset */
      drum->selected_edit_button = toggle;
    }
  }
}
