/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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

#ifndef __AGS_PREPARE_AUDIO_SIGNAL_H__
#define __AGS_PREPARE_AUDIO_SIGNAL_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recall_audio_signal.h>

#define AGS_TYPE_PREPARE_AUDIO_SIGNAL                (ags_prepare_audio_signal_get_type())
#define AGS_PREPARE_AUDIO_SIGNAL(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_PREPARE_AUDIO_SIGNAL, AgsPrepareAudioSignal))
#define AGS_PREPARE_AUDIO_SIGNAL_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_PREPARE_AUDIO_SIGNAL, AgsPrepareAudioSignalClass))
#define AGS_IS_PREPARE_AUDIO_SIGNAL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_PREPARE_AUDIO_SIGNAL))
#define AGS_IS_PREPARE_AUDIO_SIGNAL_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_PREPARE_AUDIO_SIGNAL))
#define AGS_PREPARE_AUDIO_SIGNAL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_PREPARE_AUDIO_SIGNAL, AgsPrepareAudioSignalClass))

typedef struct _AgsPrepareAudioSignal AgsPrepareAudioSignal;
typedef struct _AgsPrepareAudioSignalClass AgsPrepareAudioSignalClass;

struct _AgsPrepareAudioSignal
{
  AgsRecallAudioSignal recall_audio_signal;
};

struct _AgsPrepareAudioSignalClass
{
  AgsRecallAudioSignalClass recall_audio_signal;
};

GType ags_prepare_audio_signal_get_type();

AgsPrepareAudioSignal* ags_prepare_audio_signal_new(AgsAudioSignal *audio_signal);

#endif /*__AGS_PREPARE_AUDIO_SIGNAL_H__*/