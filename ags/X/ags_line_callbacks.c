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

#include <ags/X/ags_line_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_soundcard.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/plugin/ags_base_plugin.h>

#include <ags/audio/ags_playback.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_recall_audio_run.h>
#include <ags/audio/ags_recall_channel.h>
#include <ags/audio/ags_recall_id.h>
#include <ags/audio/ags_port.h>
#include <ags/audio/ags_recycling_context.h>

#include <ags/audio/thread/ags_audio_loop.h>

#include <ags/audio/recall/ags_peak_channel.h>
#include <ags/audio/recall/ags_volume_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel.h>
#include <ags/audio/recall/ags_copy_pattern_channel_run.h>

#include <ags/widget/ags_led.h>
#include <ags/widget/ags_indicator.h>
#include <ags/widget/ags_vindicator.h>
#include <ags/widget/ags_hindicator.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_listing_editor.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_machine_editor.h>
#include <ags/X/ags_pad_editor.h>
#include <ags/X/ags_line_editor.h>
#include <ags/X/ags_line_member_editor.h>
#include <ags/X/ags_plugin_browser.h>
#include <ags/X/ags_ladspa_browser.h>
#include <ags/X/ags_dssi_browser.h>
#include <ags/X/ags_lv2_browser.h>

#include <ags/X/thread/ags_gui_thread.h>

int
ags_line_parent_set_callback(GtkWidget *widget, GtkObject *old_parent, AgsLine *line)
{
  if(old_parent == NULL){
    //    gtk_widget_show_all(GTK_WIDGET(line));
  }
}

void
ags_line_remove_recall_callback(AgsRecall *recall, AgsLine *line)
{
  if(recall->recall_id != NULL && recall->recall_id->recycling_context->parent != NULL){
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(line->channel->audio), (GObject *) recall, FALSE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(line->channel), (GObject *) recall, FALSE);
    }
  }else{
    if(AGS_IS_RECALL_AUDIO(recall) || AGS_RECALL_AUDIO_RUN(recall)){
      ags_audio_remove_recall(AGS_AUDIO(line->channel->audio), (GObject *) recall, TRUE);
    }else{
      ags_channel_remove_recall(AGS_CHANNEL(line->channel), (GObject *) recall, TRUE);
    }
  }
}

void
ags_line_group_clicked_callback(GtkWidget *widget, AgsLine *line)
{
  AgsPad *pad;
  AgsLine *current;

  GtkContainer *container;

  GList *list, *list_start;

  pad = (AgsPad *) gtk_widget_get_ancestor(GTK_WIDGET(line),
					   AGS_TYPE_PAD);

  container = (GtkContainer *) pad->expander_set;

  list_start =
    list = gtk_container_get_children(container);

  if(gtk_toggle_button_get_active(line->group)){
    ags_line_group_changed(line);

    while(list != NULL){
      current = AGS_LINE(list->data);

      if(!gtk_toggle_button_get_active(current->group)){
	g_list_free(list_start);

	return;
      }

      list = list->next;
    }

    gtk_toggle_button_set_active(pad->group, TRUE);
  }else{
    if(g_list_length(list) > 1){
      if(gtk_toggle_button_get_active(pad->group)){
	gtk_toggle_button_set_active(pad->group, FALSE);
      }

      while(list != NULL){
	current = AGS_LINE(list->data);

	if(gtk_toggle_button_get_active(current->group)){
	  ags_line_group_changed(line);
	  g_list_free(list_start);

	  return;
	}

	list = list->next;
      } 
    }

    gtk_toggle_button_set_active(line->group, TRUE);
  }

  g_list_free(list_start);

  return;
}

void
ags_line_add_effect_callback(AgsChannel *channel,
			     gchar *filename,
			     gchar *effect,
			     AgsLine *line)
{
  AgsWindow *window;
  AgsMachine *machine;
  AgsMachineEditor *machine_editor;
  AgsLineMemberEditor *line_member_editor;
  AgsPluginBrowser *plugin_browser;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;
  
  GList *pad_editor, *pad_editor_start;
  GList *line_editor, *line_editor_start;
  GList *control_type_name;
  
  pthread_mutex_t *application_mutex;

  /* lock gdk threads */
  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) line);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsAudioLoop *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);

  /*  */

  /* get machine and machine editor */
  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) line,
						   AGS_TYPE_MACHINE);
  machine_editor = (AgsMachineEditor *) machine->properties;

  /* get control type */
  control_type_name = NULL;  

  pad_editor_start = NULL;
  line_editor_start = NULL;
  
  if(machine_editor != NULL){
    pad_editor_start = 
      pad_editor = gtk_container_get_children((GtkContainer *) machine_editor->input_editor->child);
    pad_editor = g_list_nth(pad_editor,
			    channel->pad);
    
    if(pad_editor != NULL){
      line_editor_start =
	line_editor = gtk_container_get_children((GtkContainer *) AGS_PAD_EDITOR(pad_editor->data)->line_editor);
      line_editor = g_list_nth(line_editor,
			       channel->audio_channel);
    }else{
      line_editor = NULL;
    }

    if(line_editor != NULL){
      line_member_editor = AGS_LINE_EDITOR(line_editor->data)->member_editor;

      plugin_browser = line_member_editor->plugin_browser;

      if(plugin_browser != NULL &&
	 plugin_browser->active_browser != NULL){
	GList *description, *description_start;
	GList *port_control, *port_control_start;

	gchar *controls;

	/* get plugin browser */
	description_start = NULL;
	port_control_start = NULL;
	
	if(AGS_IS_LADSPA_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_LADSPA_BROWSER(plugin_browser->active_browser)->description);
	}else if(AGS_IS_DSSI_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_DSSI_BROWSER(plugin_browser->active_browser)->description);
	}else if(AGS_IS_LV2_BROWSER(plugin_browser->active_browser)){
	  description_start = 
	    description = gtk_container_get_children((GtkContainer *) AGS_LV2_BROWSER(plugin_browser->active_browser)->description);
	}else{
	  g_message("ags_line_callbacks.c unsupported plugin browser\0");
	}

	/* get port description */
	if(description != NULL){
	  description = g_list_last(description);
	  
	  port_control_start =
	    port_control = gtk_container_get_children(GTK_CONTAINER(description->data));
	  
	  if(port_control != NULL){
	    while(port_control != NULL){
	      controls = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(port_control->data));

	      if(!g_ascii_strncasecmp(controls,
				      "led\0",
				      4)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsLed\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "vertical indicator\0",
					    19)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsVIndicator\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "horizontal indicator\0",
					    19)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsHIndicator\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "spin button\0",
					    12)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkSpinButton\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "dial\0",
					    5)){
		control_type_name = g_list_prepend(control_type_name,
						   "AgsDial\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "vertical scale\0",
					    15)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkVScale\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "horizontal scale\0",
					    17)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkHScale\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "check-button\0",
					    13)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkCheckButton\0");
	      }else if(!g_ascii_strncasecmp(controls,
					    "toggle button\0",
					    14)){
		control_type_name = g_list_prepend(control_type_name,
						   "GtkToggleButton\0");
	      }
	      
	      port_control = port_control->next;
	      port_control = port_control->next;
	    }
	  }

	  /* free lists */
	  g_list_free(description_start);
	  g_list_free(port_control_start);
	}
      }
      
      //      line_member_editor->plugin_browser;
    }
  }else{
    control_type_name = NULL;
  }

  /* add effect */
  ags_line_add_effect(line,
		      control_type_name,
		      filename,
		      effect);

  /* free lists */
  g_list_free(pad_editor_start);
  g_list_free(line_editor_start);
}

void
ags_line_remove_effect_callback(AgsChannel *channel,
				guint nth,
				AgsLine *line)
{
  AgsWindow *window;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;

  pthread_mutex_t *application_mutex;

  gdk_threads_enter();

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) line);

  application_context = (AgsApplicationContext *) window->application_context;

  gdk_threads_leave();

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsAudioLoop *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);

  /*  */
  ags_line_remove_effect(line,
			 nth);
}

void
ags_line_volume_callback(GtkRange *range,
			 AgsLine *line)
{
  AgsVolumeChannel *volume_channel;
  GList *list;
  GValue value = {0,};

  g_value_init(&value, G_TYPE_DOUBLE);
  g_value_set_double(&value, gtk_range_get_value(range));

  list = line->channel->play;

  while((list = ags_recall_find_type(list, AGS_TYPE_VOLUME_CHANNEL)) != NULL){
    volume_channel = AGS_VOLUME_CHANNEL(list->data);
    ags_port_safe_write(volume_channel->volume,
			&value);

    list = list->next;
  }

  list = line->channel->recall;

  while((list = ags_recall_find_type(list, AGS_TYPE_VOLUME_CHANNEL)) != NULL){
    volume_channel = AGS_VOLUME_CHANNEL(list->data);
    ags_port_safe_write(volume_channel->volume,
			&value);

    list = list->next;
  }
}

void
ags_line_channel_done_callback(AgsChannel *source, AgsRecallID *recall_id,
			       AgsLine *line)
{
  AgsChannel *channel;
  AgsPlayback *playback;
  AgsChannel *next_pad;
  AgsRecallID *match_recall_id;

  AgsMutexManager *mutex_manager;

  gboolean all_done;

  pthread_mutex_t *application_mutex;
  pthread_mutex_t *channel_mutex;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  gdk_threads_enter();
  
  /* retrieve channel */
  channel = AGS_PAD(AGS_LINE(line)->pad)->channel;

  /* retrieve channel mutex */
  pthread_mutex_lock(application_mutex);

  channel_mutex = ags_mutex_manager_lookup(mutex_manager,
					   (GObject *) channel);

  pthread_mutex_unlock(application_mutex);

  /* get next pad */
  pthread_mutex_lock(channel_mutex);

  next_pad = channel->next_pad;

  pthread_mutex_unlock(channel_mutex);

  all_done = TRUE;

  while(channel != next_pad){
    pthread_mutex_lock(channel_mutex);

    playback = AGS_PLAYBACK(channel->playback);
    match_recall_id = playback->recall_id[0];
		
    pthread_mutex_unlock(channel_mutex);

    /* check if pending */
    if(match_recall_id != NULL){
      all_done = FALSE;
      break;
    }

    /* iterate */
    pthread_mutex_lock(channel_mutex);

    channel = channel->next;

    pthread_mutex_unlock(channel_mutex);
  }

  /* toggle play button if all playback done */
  if(all_done){
    AgsPad *pad;

    pad = AGS_PAD(AGS_LINE(line)->pad);

    if(pad->play != NULL){
      gtk_toggle_button_set_active(pad->play, FALSE);
    }
  }

  gdk_threads_leave();
}
