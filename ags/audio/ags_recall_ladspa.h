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

#ifndef __AGS_RECALL_LADSPA_H__
#define __AGS_RECALL_LADSPA_H__

#include <glib.h>
#include <glib-object.h>

#include <ladspa.h>

#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_channel_run.h>
#include <ags/audio/ags_channel.h>

#define AGS_TYPE_RECALL_LADSPA                (ags_recall_ladspa_get_type())
#define AGS_RECALL_LADSPA(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_RECALL_LADSPA, AgsRecallLadspa))
#define AGS_RECALL_LADSPA_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_RECALL_LADSPA, AgsRecallLadspaClass))
#define AGS_IS_RECALL_LADSPA(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_RECALL_LADSPA))
#define AGS_IS_RECALL_LADSPA_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_RECALL_LADSPA))
#define AGS_RECALL_LADSPA_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_RECALL_LADSPA, AgsRecallLadspaClass))

typedef struct _AgsRecallLadspa AgsRecallLadspa;
typedef struct _AgsRecallLadspaClass AgsRecallLadspaClass;

struct _AgsRecallLadspa
{
  AgsRecallChannel recall_channel;

  gchar *filename;
  gchar *effect;
  unsigned long index;

  LADSPA_Descriptor *plugin_descriptor;

  unsigned long *input_port;
  unsigned long input_lines;

  unsigned long *output_port;
  unsigned long output_lines;
};

struct _AgsRecallLadspaClass
{
  AgsRecallChannelClass recall_channel;
};

GType ags_recall_ladspa_get_type();

void ags_recall_ladspa_load(AgsRecallLadspa *recall_ladspa);
GList* ags_recall_ladspa_load_ports(AgsRecallLadspa *recall_ladspa);
void ags_recall_ladspa_load_conversion(AgsRecallLadspa *recall_ladspa,
				       GObject *port,
				       gpointer port_descriptor);

GList* ags_recall_ladspa_find(GList *recall,
			      gchar *filename, gchar *effect);

AgsRecallLadspa* ags_recall_ladspa_new(AgsChannel *source,
				       gchar *filename,
				       gchar *effect,
				       unsigned long index);

#endif /*__AGS_RECALL_LADSPA_H__*/
