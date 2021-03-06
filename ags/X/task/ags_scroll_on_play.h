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

#ifndef __AGS_SCROLL_ON_PLAY_H__
#define __AGS_SCROLL_ON_PLAY_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/thread/ags_task.h>
#include <ags/object/ags_soundcard.h>

#define AGS_TYPE_SCROLL_ON_PLAY                (ags_scroll_on_play_get_type())
#define AGS_SCROLL_ON_PLAY(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SCROLL_ON_PLAY, AgsScrollOnPlay))
#define AGS_SCROLL_ON_PLAY_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SCROLL_ON_PLAY, AgsScrollOnPlayClass))
#define AGS_IS_SCROLL_ON_PLAY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SCROLL_ON_PLAY))
#define AGS_IS_SCROLL_ON_PLAY_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SCROLL_ON_PLAY))
#define AGS_SCROLL_ON_PLAY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SCROLL_ON_PLAY, AgsScrollOnPlayClass))

typedef struct _AgsScrollOnPlay AgsScrollOnPlay;
typedef struct _AgsScrollOnPlayClass AgsScrollOnPlayClass;

struct _AgsScrollOnPlay
{
  AgsTask task;

  gdouble step;
  
  GtkWidget *editor;
};

struct _AgsScrollOnPlayClass
{
  AgsTaskClass task;
};

GType ags_scroll_on_play_get_type();

AgsScrollOnPlay* ags_scroll_on_play_new(GtkWidget *editor, gdouble step);

#endif /*__AGS_SCROLL_ON_PLAY_H__*/
