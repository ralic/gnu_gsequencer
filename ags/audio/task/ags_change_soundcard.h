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

#ifndef __AGS_CHANGE_SOUNDCARD_H__
#define __AGS_CHANGE_SOUNDCARD_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/object/ags_application_context.h>

#include <ags/thread/ags_task.h>

#define AGS_TYPE_CHANGE_SOUNDCARD                (ags_change_soundcard_get_type())
#define AGS_CHANGE_SOUNDCARD(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_CHANGE_SOUNDCARD, AgsChangeSoundcard))
#define AGS_CHANGE_SOUNDCARD_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_CHANGE_SOUNDCARD, AgsChangeSoundcardClass))
#define AGS_IS_CHANGE_SOUNDCARD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_CHANGE_SOUNDCARD))
#define AGS_IS_CHANGE_SOUNDCARD_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_CHANGE_SOUNDCARD))
#define AGS_CHANGE_SOUNDCARD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_CHANGE_SOUNDCARD, AgsChangeSoundcardClass))

typedef struct _AgsChangeSoundcard AgsChangeSoundcard;
typedef struct _AgsChangeSoundcardClass AgsChangeSoundcardClass;

struct _AgsChangeSoundcard
{
  AgsTask task;
  
  AgsApplicationContext *application_context;

  GObject *new_soundcard;
  GObject *old_soundcard;
};

struct _AgsChangeSoundcardClass
{
  AgsTaskClass task;
};

GType ags_change_soundcard_get_type();

AgsChangeSoundcard* ags_change_soundcard_new(AgsApplicationContext *application_context,
					     GObject *new_soundcard,
					     GObject *old_soundcard);

#endif /*__AGS_CHANGE_SOUNDCARD_H__*/
