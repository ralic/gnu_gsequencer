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

#ifndef __AGS_LV2_MANAGER_H__
#define __AGS_LV2_MANAGER_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/plugin/ags_lv2_plugin.h>

#include <lv2.h>

#include <math.h>
#include <stdlib.h>

#define AGS_TYPE_LV2_MANAGER                (ags_lv2_manager_get_type())
#define AGS_LV2_MANAGER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_LV2_MANAGER, AgsLv2Manager))
#define AGS_LV2_MANAGER_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_LV2_MANAGER, AgsLv2ManagerClass))
#define AGS_IS_LV2_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_LV2_MANAGER))
#define AGS_IS_LV2_MANAGER_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_LV2_MANAGER))
#define AGS_LV2_MANAGER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_LV2_MANAGER, AgsLv2ManagerClass))

#define AGS_LV2_MANAGER_DEFAULT_LOCALE "en-gb\0"

typedef struct _AgsLv2Manager AgsLv2Manager;
typedef struct _AgsLv2ManagerClass AgsLv2ManagerClass;

struct _AgsLv2Manager
{
  GObject object;

  gchar *locale;

  GList *lv2_plugin_blacklist;
  GList *lv2_plugin;
};

struct _AgsLv2ManagerClass
{
  GObjectClass object;
};

GType ags_lv2_manager_get_type(void);

gchar** ags_lv2_manager_get_filenames(AgsLv2Manager *lv2_manager);
AgsLv2Plugin* ags_lv2_manager_find_lv2_plugin(AgsLv2Manager *lv2_manager,
					      gchar *filename,
					      gchar *effect);

void ags_lv2_manager_load_blacklist(AgsLv2Manager *lv2_manager,
				    gchar *blacklist_filename);

void ags_lv2_manager_load_file(AgsLv2Manager *lv2_manager,
			       AgsTurtle *manifest,
			       AgsTurtle *turtle,
			       gchar *lv2_path,
			       gchar *filename);
void ags_lv2_manager_load_preset(AgsLv2Manager *lv2_manager,
				 AgsLv2Plugin *lv2_plugin,
				 AgsTurtle *preset);
void ags_lv2_manager_load_default_directory(AgsLv2Manager *lv2_manager);

/*  */
AgsLv2Manager* ags_lv2_manager_get_instance();

AgsLv2Manager* ags_lv2_manager_new(gchar *locale);

#endif /*__AGS_LV2_MANAGER_H__*/
