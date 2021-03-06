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

#ifndef __AGS_SCALE_AREA_H__
#define __AGS_SCALE_AREA_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <cairo.h>

#define AGS_TYPE_SCALE_AREA                (ags_scale_area_get_type())
#define AGS_SCALE_AREA(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_SCALE_AREA, AgsScaleArea))
#define AGS_SCALE_AREA_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_SCALE_AREA, AgsScaleAreaClass))
#define AGS_IS_SCALE_AREA(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_SCALE_AREA))
#define AGS_IS_SCALE_AREA_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_SCALE_AREA))
#define AGS_SCALE_AREA_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS (obj, AGS_TYPE_SCALE_AREA, AgsScaleAreaClass))

#define AGS_SCALE_AREA_DEFAULT_HEIGHT (128)

typedef struct _AgsScaleArea AgsScaleArea;
typedef struct _AgsScaleAreaClass AgsScaleAreaClass;

struct _AgsScaleArea
{
  GObject gobject;

  guint y;
  guint height;

  guint font_size;
  
  GtkDrawingArea *drawing_area;

  gchar *control_name;

  gdouble lower;
  gdouble upper;

  gdouble steps;  
};

struct _AgsScaleAreaClass
{
  GObjectClass gobject;
};

GType ags_scale_area_get_type(void);

GList* ags_scale_area_find_specifier(GList *scale_area,
				     gchar *specifier);

void ags_scale_area_paint(AgsScaleArea *scale_area,
			  cairo_t *cr,
			  gdouble x_offset, gdouble y_offset);

AgsScaleArea* ags_scale_area_new(GtkDrawingArea *drawing_area,
				 gchar *control_name,
				 gdouble lower,
				 gdouble upper,
				 gdouble steps);

#endif /*__AGS_SCALE_AREA_H__*/
