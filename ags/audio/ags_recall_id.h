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

#ifndef __AGS_RECALL_ID_H__
#define __AGS_RECALL_ID_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_recycling.h>
#include <ags/audio/ags_recycling_context.h>

#define AGS_TYPE_RECALL_ID                (ags_recall_id_get_type ())
#define AGS_RECALL_ID(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), AGS_TYPE_RECALL_ID, AgsRecallID))
#define AGS_RECALL_ID_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST ((class), AGS_TYPE_RECALL_ID, AgsRecallIDClass))
#define AGS_IS_RECALL_ID(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_RECALL_ID))
#define AGS_IS_RECALL_ID_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_RECALL_ID))
#define AGS_RECALL_ID_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_RECALL_ID, AgsRecallIDClass))

typedef struct _AgsRecallID AgsRecallID;
typedef struct _AgsRecallIDClass AgsRecallIDClass;

typedef enum{
  AGS_RECALL_ID_CONNECTED         = 1,
  AGS_RECALL_ID_PLAYBACK          = 1 <<  1,
  AGS_RECALL_ID_SEQUENCER         = 1 <<  2,
  AGS_RECALL_ID_NOTATION          = 1 <<  3,
  AGS_RECALL_ID_DUPLICATE         = 1 <<  4,
  AGS_RECALL_ID_RESOLVE           = 1 <<  5,
  AGS_RECALL_ID_INIT_PRE          = 1 <<  6,
  AGS_RECALL_ID_INIT_INTER        = 1 <<  7,
  AGS_RECALL_ID_INIT_POST         = 1 <<  8,
  AGS_RECALL_ID_PRE               = 1 <<  9,
  AGS_RECALL_ID_INTER             = 1 << 10,
  AGS_RECALL_ID_POST              = 1 << 11,
  AGS_RECALL_ID_CANCEL            = 1 << 12,
}AgsRecallIDFlags;

struct _AgsRecallID
{
  GObject object;
  
  guint flags;

  GObject *recycling;
  AgsRecyclingContext *recycling_context;
};

struct _AgsRecallIDClass
{
  GObjectClass object;
};

GType ags_recall_id_get_type(void);

gboolean ags_recall_id_get_run_stage(AgsRecallID *id, gint stage);
void ags_recall_id_set_run_stage(AgsRecallID *recall_id, gint stage);
void ags_recall_id_unset_run_stage(AgsRecallID *recall_id, gint stage);

AgsRecallID* ags_recall_id_find_recycling_context(GList *recall_id_list,
						  AgsRecyclingContext *recycling_context);
AgsRecallID* ags_recall_id_find_parent_recycling_context(GList *recall_id_list,
							 AgsRecyclingContext *parent_recycling_context);

AgsRecallID* ags_recall_id_new(AgsRecycling *recycling);

#endif /*__AGS_RECALL_ID_H__*/
