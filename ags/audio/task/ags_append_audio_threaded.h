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

#ifndef __AGS_APPEND_AUDIO_THREADED_H__
#define __AGS_APPEND_AUDIO_THREADED_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>
#include <ags/object/ags_soundcard.h>

#define AGS_TYPE_APPEND_AUDIO_THREADED                (ags_append_audio_threaded_get_type())
#define AGS_APPEND_AUDIO_THREADED(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_APPEND_AUDIO_THREADED, AgsAppendAudioThreaded))
#define AGS_APPEND_AUDIO_THREADED_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_APPEND_AUDIO_THREADED, AgsAppendAudioThreadedClass))
#define AGS_IS_APPEND_AUDIO_THREADED(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_APPEND_AUDIO_THREADED))
#define AGS_IS_APPEND_AUDIO_THREADED_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_APPEND_AUDIO_THREADED))
#define AGS_APPEND_AUDIO_THREADED_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_APPEND_AUDIO_THREADED, AgsAppendAudioThreadedClass))

typedef struct _AgsAppendAudioThreaded AgsAppendAudioThreaded;
typedef struct _AgsAppendAudioThreadedClass AgsAppendAudioThreadedClass;

struct _AgsAppendAudioThreaded
{
  AgsTask task;

  GObject *audio_loop;
  GObject *playback;
};

struct _AgsAppendAudioThreadedClass
{
  AgsTaskClass task;
};

GType ags_append_audio_threaded_get_type();

AgsAppendAudioThreaded* ags_append_audio_threaded_new(GObject *audio_loop,
						      GObject *playback);

#endif /*__AGS_APPEND_AUDIO_THREADED_H__*/
