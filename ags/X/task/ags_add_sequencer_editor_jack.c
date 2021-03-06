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

#include <ags/X/task/ags_add_sequencer_editor_jack.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>

#include <ags/thread/ags_mutex_manager.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>

#include <ags/X/thread/ags_gui_thread.h>

void ags_add_sequencer_editor_jack_class_init(AgsAddSequencerEditorJackClass *add_sequencer_editor_jack);
void ags_add_sequencer_editor_jack_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_add_sequencer_editor_jack_init(AgsAddSequencerEditorJack *add_sequencer_editor_jack);
void ags_add_sequencer_editor_jack_connect(AgsConnectable *connectable);
void ags_add_sequencer_editor_jack_disconnect(AgsConnectable *connectable);
void ags_add_sequencer_editor_jack_finalize(GObject *gobject);

void ags_add_sequencer_editor_jack_launch(AgsTask *task);

/**
 * SECTION:ags_add_sequencer_editor_jack
 * @short_description: add line_member object to line
 * @title: AgsAddSequencerEditorJack
 * @section_id:
 * @include: ags/X/task/ags_add_sequencer_editor_jack.h
 *
 * The #AgsAddSequencerEditorJack task addspacks #AgsLineMember to #AgsLine.
 */

static gpointer ags_add_sequencer_editor_jack_parent_class = NULL;
static AgsConnectableInterface *ags_add_sequencer_editor_jack_parent_connectable_interface;

GType
ags_add_sequencer_editor_jack_get_type()
{
  static GType ags_type_add_sequencer_editor_jack = 0;

  if(!ags_type_add_sequencer_editor_jack){
    static const GTypeInfo ags_add_sequencer_editor_jack_info = {
      sizeof (AgsAddSequencerEditorJackClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_add_sequencer_editor_jack_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsAddSequencerEditorJack),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_add_sequencer_editor_jack_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_add_sequencer_editor_jack_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_add_sequencer_editor_jack = g_type_register_static(AGS_TYPE_TASK,
								"AgsAddSequencerEditorJack\0",
								&ags_add_sequencer_editor_jack_info,
								0);

    g_type_add_interface_static(ags_type_add_sequencer_editor_jack,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_add_sequencer_editor_jack);
}

void
ags_add_sequencer_editor_jack_class_init(AgsAddSequencerEditorJackClass *add_sequencer_editor_jack)
{
  GObjectClass *gobject;
  AgsTaskClass *task;

  ags_add_sequencer_editor_jack_parent_class = g_type_class_peek_parent(add_sequencer_editor_jack);

  /* gobject */
  gobject = (GObjectClass *) add_sequencer_editor_jack;

  gobject->finalize = ags_add_sequencer_editor_jack_finalize;

  /* task */
  task = (AgsTaskClass *) add_sequencer_editor_jack;

  task->launch = ags_add_sequencer_editor_jack_launch;
}

void
ags_add_sequencer_editor_jack_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_add_sequencer_editor_jack_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_add_sequencer_editor_jack_connect;
  connectable->disconnect = ags_add_sequencer_editor_jack_disconnect;
}

void
ags_add_sequencer_editor_jack_init(AgsAddSequencerEditorJack *add_sequencer_editor_jack)
{
  add_sequencer_editor_jack->sequencer_editor = NULL;
}

void
ags_add_sequencer_editor_jack_connect(AgsConnectable *connectable)
{
  ags_add_sequencer_editor_jack_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_add_sequencer_editor_jack_disconnect(AgsConnectable *connectable)
{
  ags_add_sequencer_editor_jack_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_add_sequencer_editor_jack_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_add_sequencer_editor_jack_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_add_sequencer_editor_jack_launch(AgsTask *task)
{
  AgsWindow *window;

  AgsGuiThread *gui_thread;

  AgsMutexManager *mutex_manager;
  AgsThread *main_loop;

  AgsApplicationContext *application_context;
  
  pthread_mutex_t *application_mutex;
  
  /* lock gdk threads */
  gdk_threads_enter();

  window = (AgsWindow *) gtk_widget_get_toplevel((GtkWidget *) AGS_ADD_SEQUENCER_EDITOR_JACK(task)->sequencer_editor);

  application_context = (AgsApplicationContext *) window->application_context;

  mutex_manager = ags_mutex_manager_get_instance();
  application_mutex = ags_mutex_manager_get_application_mutex(mutex_manager);

  /* get audio loop */
  pthread_mutex_lock(application_mutex);

  main_loop = (AgsThread *) application_context->main_loop;
  
  pthread_mutex_unlock(application_mutex);

  /* get task thread */
  gui_thread = (AgsGuiThread *) ags_thread_find_type((AgsThread *) main_loop,
						      AGS_TYPE_GUI_THREAD);
  /*  */
  gdk_threads_enter();

  ags_sequencer_editor_add_jack(AGS_ADD_SEQUENCER_EDITOR_JACK(task)->sequencer_editor,
				NULL);

  gdk_threads_leave();

  /* unlock gdk threads */
  gdk_threads_leave();
}

/**
 * ags_add_sequencer_editor_jack_new:
 * @sequencer_editor: the #AgsAddSequencerEditor
 *
 * Creates an #AgsAddSequencerEditorJack.
 *
 * Returns: an new #AgsAddSequencerEditorJack.
 *
 * Since: 0.7.111
 */
AgsAddSequencerEditorJack*
ags_add_sequencer_editor_jack_new(AgsSequencerEditor *sequencer_editor)
{
  AgsAddSequencerEditorJack *add_sequencer_editor_jack;

  add_sequencer_editor_jack = (AgsAddSequencerEditorJack *) g_object_new(AGS_TYPE_ADD_SEQUENCER_EDITOR_JACK,
									 NULL);

  add_sequencer_editor_jack->sequencer_editor = sequencer_editor;

  return(add_sequencer_editor_jack);
}
