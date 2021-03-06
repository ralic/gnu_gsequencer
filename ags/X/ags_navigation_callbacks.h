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

#ifndef __AGS_NAVIGATION_CALLBACKS_H__
#define __AGS_NAVIGATION_CALLBACKS_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <ags/X/ags_navigation.h>

void ags_navigation_parent_set_callback(GtkWidget *widget, GtkObject *old_parent,
					gpointer data);

void ags_navigation_expander_callback(GtkWidget *widget,
				      AgsNavigation *navigation);

void ags_navigation_bpm_callback(GtkWidget *widget,
				 AgsNavigation *navigation);

void ags_navigation_rewind_callback(GtkWidget *widget,
				    AgsNavigation *navigation);
void ags_navigation_prev_callback(GtkWidget *widget,
				  AgsNavigation *navigation);
void ags_navigation_play_callback(GtkWidget *widget,
				  AgsNavigation *navigation);
void ags_navigation_stop_callback(GtkWidget *widget,
				  AgsNavigation *navigation);
void ags_navigation_next_callback(GtkWidget *widget,
				  AgsNavigation *navigation);
void ags_navigation_forward_callback(GtkWidget *widget,
				     AgsNavigation *navigation);

void ags_navigation_loop_callback(GtkWidget *widget,
				  AgsNavigation *navigation);

void ags_navigation_position_tact_callback(GtkWidget *widget,
					   AgsNavigation *navigation);

void ags_navigation_duration_tact_callback(GtkWidget *widget,
					   AgsNavigation *navigation);

void ags_navigation_loop_left_tact_callback(GtkWidget *widget,
					    AgsNavigation *navigation);

void ags_navigation_loop_right_tact_callback(GtkWidget *widget,
					     AgsNavigation *navigation);

void ags_navigation_raster_callback(GtkWidget *widget,
				    AgsNavigation *navigation);

void ags_navigation_soundcard_stop_callback(GObject *soundcard,
					    AgsNavigation *navigation);

#endif /*__AGS_NAVIGATION_CALLBACKS_H__*/
