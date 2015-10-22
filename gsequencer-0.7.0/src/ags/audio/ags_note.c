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

#include <ags/audio/ags_note.h>

#include <ags/object/ags_connectable.h>

#include <stdlib.h>

void ags_note_class_init(AgsNoteClass *note);
void ags_note_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_note_init(AgsNote *note);
void ags_note_connect(AgsConnectable *connectable);
void ags_note_disconnect(AgsConnectable *connectable);
void ags_note_finalize(GObject *object);

/**
 * SECTION:ags_note
 * @short_description: Note class.
 * @title: AgsNote
 * @section_id:
 * @include: ags/audio/ags_note.h
 *
 * #AgsNote represents a tone.
 */

static gpointer ags_note_parent_class = NULL;

GType
ags_note_get_type()
{
  static GType ags_type_note = 0;

  if(!ags_type_note){
    static const GTypeInfo ags_note_info = {
      sizeof(AgsNoteClass),
      NULL,
      NULL,
      (GClassInitFunc) ags_note_class_init,
      NULL,
      NULL,
      sizeof(AgsNote),
      0,
      (GInstanceInitFunc) ags_note_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_note_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_note = g_type_register_static(G_TYPE_OBJECT,
					   "AgsNote\0",
					   &ags_note_info,
					   0);
    
    g_type_add_interface_static(ags_type_note,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_note);
}

void 
ags_note_class_init(AgsNoteClass *note)
{
  GObjectClass *gobject;

  ags_note_parent_class = g_type_class_peek_parent(note);

  gobject = (GObjectClass *) note;

  gobject->finalize = ags_note_finalize;
}

void
ags_note_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->connect = ags_note_connect;
  connectable->disconnect = ags_note_disconnect;
}

void
ags_note_init(AgsNote *note)
{
  note->flags = 0;

  note->x[0] = 0;
  note->x[1] = 0;
  note->y = 0;

  note->name = NULL;
  note->frequency = 440.0;
}

void
ags_note_connect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_note_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

void
ags_note_finalize(GObject *gobject)
{
  /* empty */

  G_OBJECT_CLASS(ags_note_parent_class)->finalize(gobject);
}

/**
 * ags_note_duplicate:
 * @note: an #AgsNote
 * 
 * Duplicate a note.
 *
 * Returns: the duplicated #AgsNote.
 *
 * Since: 0.4
 */
AgsNote*
ags_note_duplicate(AgsNote *note)
{
  AgsNote *copy;

  copy = ags_note_new();

  copy->flags = 0;

  copy->x[0] = note->x[0];
  copy->x[1] = note->x[1];
  copy->y = note->y;

  return(copy);
}

/**
 * ags_note_new:
 *
 * Creates an #AgsNote
 *
 * Returns: a new #AgsNote
 *
 * Since: 0.4
 */
AgsNote*
ags_note_new()
{
  AgsNote *note;

  note = (AgsNote *) g_object_new(AGS_TYPE_NOTE, NULL);

  return(note);
}
