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

#ifndef __AGS_PLAY_AUDIO_FILE_H__
#define __AGS_PLAY_AUDIO_FILE_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall.h>

#include <ags/audio/file/ags_audio_file.h>
#include <ags/object/ags_soundcard.h>

#define AGS_TYPE_PLAY_AUDIO_FILE                (ags_play_audio_file_get_type())
#define AGS_PLAY_AUDIO_FILE(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PLAY_AUDIO_FILE, AgsPlayAudioFile))
#define AGS_PLAY_AUDIO_FILE_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AgsPlayAudioFileClass))
#define AGS_IS_PLAY_AUDIO_FILE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PLAY_AUDIO_FILE))
#define AGS_IS_PLAY_AUDIO_FILE_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PLAY_AUDIO_FILE))
#define AGS_PLAY_AUDIO_FILE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_PLAY_AUDIO_FILE, AgsPlayAudioFileClass))

typedef struct _AgsPlayAudioFile AgsPlayAudioFile;
typedef struct _AgsPlayAudioFileClass AgsPlayAudioFileClass;

struct _AgsPlayAudioFile
{
  AgsRecall recall;

  AgsAudioFile *audio_file;
  guint current_frame;

  GObject *soundcard;
};

struct _AgsPlayAudioFileClass
{
  AgsRecallClass recall;
};

GType ags_play_audio_file_get_type();

AgsPlayAudioFile* ags_play_audio_file_new(AgsAudioFile *audio_file,
					  GObject *soundcard);

#endif /*__AGS_PLAY_AUDIO_FILE_H__*/
