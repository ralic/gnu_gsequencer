/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2015 Joël Krähemann
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

#include <ags/X/task/ags_add_sequencer_editor_jack.h>

#include <ags/object/ags_connectable.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_channel.h>

#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>

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
  ags_sequencer_editor_add_jack(AGS_ADD_SEQUENCER_EDITOR_JACK(task)->sequencer_editor,
				NULL);
}

/**
 * ags_add_sequencer_editor_jack_new:
 * @sequencer_editor: the #AgsAddSequencerEditor
 *
 * Creates an #AgsAddSequencerEditorJack.
 *
 * Returns: an new #AgsAddSequencerEditorJack.
 *
 * Since: 1.0.0
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