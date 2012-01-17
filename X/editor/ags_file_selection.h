/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2005-2011 Joël Krähemann
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

#ifndef __AGS_FILE_SELECTION_H__
#define __AGS_FILE_SELECTION_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/audio/ags_audio.h>

#include <ags/audio/file/ags_audio_file.h>

#define AGS_TYPE_FILE_SELECTION                (ags_file_selection_get_type())
#define AGS_FILE_SELECTION(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_FILE_SELECTION, AgsFileSelection))
#define AGS_FILE_SELECTION_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_FILE_SELECTION, AgsFileSelectionClass))
#define AGS_IS_FILE_SELECTION(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_FILE_SELECTION))
#define AGS_IS_FILE_SELECTION_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_FILE_SELECTION))
#define AGS_FILE_SELECTION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_FILE_SELECTION, AgsFileSelectionClass))

typedef struct _AgsFileSelection AgsFileSelection;
typedef struct _AgsFileSelectionClass AgsFileSelectionClass;
typedef struct _AgsFileSelectionEntry AgsFileSelectionEntry;

struct _AgsFileSelection
{
  GtkVBox vbox;

  GList *entry;

  GtkLabel *chosed;
  GtkLabel *selected;
};

struct _AgsFileSelectionClass
{
  GtkVBoxClass vbox;

  void (*completed)(AgsFileSelection *file_selection);
};

GType ags_file_selection_get_type(void);

void ags_file_selection_completed(AgsFileSelection *file_selection);

AgsFileSelection* ags_file_selection_new(GObject *devout);

#endif /*__AGS_FILE_SELECTION_H__*/
