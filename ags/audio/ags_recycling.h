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

#ifndef __AGS_RECYCLING_H__
#define __AGS_RECYCLING_H__

#include <glib-object.h>

#include <ags/audio/ags_audio_signal.h>

#define AGS_TYPE_RECYCLING                (ags_recycling_get_type())
#define AGS_RECYCLING(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECYCLING, AgsRecycling))
#define AGS_RECYCLING_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECYCLING, AgsRecyclingClass))
#define AGS_IS_RECYCLING(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECYCLING))
#define AGS_IS_RECYCLING_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECYCLING))
#define AGS_RECYCLING_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_RECYCLING, AgsRecyclingClass))

typedef struct _AgsRecycling AgsRecycling;
typedef struct _AgsRecyclingClass AgsRecyclingClass;

typedef enum{
  AGS_RECYCLING_CONNECTED       = 1,
  AGS_RECYCLING_MUTED           = 1 <<  1,
}AgsRecyclingFlags;

struct _AgsRecycling
{
  GObject object;

  guint flags;

  gboolean parent_locked;
  
  GObject *channel;
  GObject *soundcard;

  AgsRecycling *parent;

  AgsRecycling *next;
  AgsRecycling *prev;

  GList *audio_signal;
};

struct _AgsRecyclingClass
{
  GObjectClass object;

  void (*add_audio_signal)(AgsRecycling *recycling,
			   AgsAudioSignal *audio_signal);
  void (*remove_audio_signal)(AgsRecycling *recycling,
			      AgsAudioSignal *audio_signal);
};

GType ags_recycling_get_type(void);

void ags_recycling_set_soundcard(AgsRecycling *recycling, GObject *soundcard);

void ags_recycling_add_audio_signal(AgsRecycling *recycling,
				    AgsAudioSignal *audio_signal);
void ags_recycling_remove_audio_signal(AgsRecycling *recycling,
				       AgsAudioSignal *audio_signal);

void ags_recycling_create_audio_signal_with_defaults(AgsRecycling *recycling,
						     AgsAudioSignal *audio_signal,
						     gdouble delay, guint attack);
void ags_recycling_create_audio_signal_with_frame_count(AgsRecycling *recycling,
							AgsAudioSignal *audio_signal,
							guint frame_count,
							gdouble delay, guint attack);

gint ags_recycling_position(AgsRecycling *start_region, AgsRecycling *end_region,
			    AgsRecycling *recycling);
AgsRecycling* ags_recycling_find_next_channel(AgsRecycling *start_region, AgsRecycling *end_region,
					      GObject *prev_channel);

gboolean ags_recycling_is_active(AgsRecycling *start_region, AgsRecycling *end_region,
				 GObject *recall_id);

AgsRecycling* ags_recycling_new(GObject *soundcard);

#endif /*__AGS_RECYCLING_H__*/
