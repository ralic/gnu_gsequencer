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

#ifndef __AGS_AUDIO_FILE_LINK_H__
#define __AGS_AUDIO_FILE_LINK_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/file/ags_file_link.h>

#include <ags/thread/ags_timestamp.h>

#define AGS_TYPE_AUDIO_FILE_LINK                (ags_audio_file_link_get_type())
#define AGS_AUDIO_FILE_LINK(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_AUDIO_FILE_LINK, AgsAudioFileLink))
#define AGS_AUDIO_FILE_LINK_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST(class, AGS_TYPE_AUDIO_FILE_LINK, AgsAudioFileLink))
#define AGS_IS_AUDIO_FILE_LINK(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_AUDIO_FILE_LINK))
#define AGS_IS_AUDIO_FILE_LINK_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_AUDIO_FILE_LINK))
#define AGS_AUDIO_FILE_LINK_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS(obj, AGS_TYPE_AUDIO_FILE_LINK, AgsAudioFileLinkClass))

typedef struct _AgsAudioFileLink AgsAudioFileLink;
typedef struct _AgsAudioFileLinkClass AgsAudioFileLinkClass;

struct _AgsAudioFileLink
{
  AgsFileLink file_link;

  gchar *preset;
  gchar *instrument;
  gchar *sample;
  
  guint audio_channel;
  
  AgsTimestamp *timestamp;
};

struct _AgsAudioFileLinkClass
{
  AgsFileLinkClass file_link;
};

GType ags_audio_file_link_get_type();

AgsAudioFileLink* ags_audio_file_link_new();

#endif /*__AGS_AUDIO_FILE_LINK_H__*/
