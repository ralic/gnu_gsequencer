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

#ifndef __AGS_SET_AUDIO_CHANNELS_H__
#define __AGS_SET_AUDIO_CHANNELS_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/thread/ags_task.h>

#include <ags/object/ags_soundcard.h>

#define AGS_TYPE_SET_AUDIO_CHANNELS                (ags_set_audio_channels_get_type())
#define AGS_SET_AUDIO_CHANNELS(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SET_AUDIO_CHANNELS, AgsSetAudioChannels))
#define AGS_SET_AUDIO_CHANNELS_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SET_AUDIO_CHANNELS, AgsSetAudioChannelsClass))
#define AGS_IS_SET_AUDIO_CHANNELS(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_SET_AUDIO_CHANNELS))
#define AGS_IS_SET_AUDIO_CHANNELS_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_SET_AUDIO_CHANNELS))
#define AGS_SET_AUDIO_CHANNELS_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_SET_AUDIO_CHANNELS, AgsSetAudioChannelsClass))

typedef struct _AgsSetAudioChannels AgsSetAudioChannels;
typedef struct _AgsSetAudioChannelsClass AgsSetAudioChannelsClass;

struct _AgsSetAudioChannels
{
  AgsTask task;

  GObject *soundcard;
  guint audio_channels;
};

struct _AgsSetAudioChannelsClass
{
  AgsTaskClass task;
};

GType ags_set_audio_channels_get_type();

AgsSetAudioChannels* ags_set_audio_channels_new(GObject *soundcard, guint audio_channels);

#endif /*__AGS_SET_AUDIO_CHANNELS_H__*/
