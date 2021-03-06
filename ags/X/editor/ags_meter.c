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

#include <ags/X/editor/ags_meter.h>
#include <ags/X/editor/ags_meter_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/X/ags_editor.h>

#include <ags/X/editor/ags_note_edit.h>
#include <ags/X/editor/ags_pattern_edit.h>

#include <math.h>

void ags_meter_class_init(AgsMeterClass *meter);
void ags_meter_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_meter_init(AgsMeter *meter);
void ags_meter_connect(AgsConnectable *connectable);
void ags_meter_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_meter
 * @short_description: piano widget
 * @title: AgsMeter
 * @section_id:
 * @include: ags/X/editor/ags_meter.h
 *
 * The #AgsMeter draws you a piano.
 */

GtkStyle *meter_style;

GType
ags_meter_get_type(void)
{
  static GType ags_type_meter = 0;

  if (!ags_type_meter){
    static const GTypeInfo ags_meter_info = {
      sizeof (AgsMeterClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_meter_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMeter),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_meter_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_meter_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_meter = g_type_register_static(GTK_TYPE_DRAWING_AREA,
					    "AgsMeter\0", &ags_meter_info,
					    0);
    
    g_type_add_interface_static(ags_type_meter,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return (ags_type_meter);
}

void
ags_meter_class_init(AgsMeterClass *meter)
{
}

void
ags_meter_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_meter_connect;
  connectable->disconnect = ags_meter_disconnect;
}

void
ags_meter_init(AgsMeter *meter)
{
  GtkWidget *widget;

  widget = (GtkWidget *) meter;
  gtk_widget_set_style(widget, meter_style);
  gtk_widget_set_size_request(widget, 60, -1);
  gtk_widget_set_events (GTK_WIDGET (meter), GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
			 | GDK_BUTTON_RELEASE_MASK
			 );
}

void
ags_meter_connect(AgsConnectable *connectable)
{
  AgsMeter *meter;

  meter = AGS_METER(connectable);

  g_signal_connect((GObject *) meter, "expose_event\0",
  		   G_CALLBACK(ags_meter_expose_event), (gpointer) meter);

  g_signal_connect((GObject *) meter, "configure_event\0",
  		   G_CALLBACK(ags_meter_configure_event), (gpointer) meter);
}

void
ags_meter_disconnect(AgsConnectable *connectable)
{
}

void
ags_meter_paint(AgsMeter *meter)
{
  AgsEditor *editor;
  GtkWidget *widget;
  cairo_t *cr;
  gchar *base_note;
  guint bitmap;
  guint y[2];
  guint i, i_stop, j, j0;
  guint border_top;
  guint control_height;

  widget = (GtkWidget *) meter;

  if(!gtk_widget_get_visible(widget) ||
     !gtk_widget_is_drawable(widget) ||
     !gtk_widget_get_mapped(widget) ||
     !gtk_widget_get_realized(widget)){
    return;
  }
  
  editor = (AgsEditor *) gtk_widget_get_ancestor(widget, AGS_TYPE_EDITOR);

  border_top = 24; // see ags_ruler.c

  bitmap = 0x52a52a; // description of the keyboard

  if(AGS_IS_NOTE_EDIT(editor->current_edit_widget)){
    control_height = AGS_NOTE_EDIT(editor->current_edit_widget)->control_height;
    
    j = (guint) ceil(GTK_RANGE(AGS_NOTE_EDIT(editor->current_edit_widget)->vscrollbar)->adjustment->value / (double) control_height);

    y[0] = (guint) round(GTK_RANGE(AGS_NOTE_EDIT(editor->current_edit_widget)->vscrollbar)->adjustment->value) % control_height;
  }else if(AGS_IS_PATTERN_EDIT(editor->current_edit_widget)){
    control_height = AGS_PATTERN_EDIT(editor->current_edit_widget)->control_height;
    
    j = (guint) ceil(GTK_RANGE(AGS_PATTERN_EDIT(editor->current_edit_widget)->vscrollbar)->adjustment->value / (double) control_height);

    y[0] = (guint) round(GTK_RANGE(AGS_PATTERN_EDIT(editor->current_edit_widget)->vscrollbar)->adjustment->value) % control_height;
  }else{
    return;
  }

  base_note = AGS_NOTATION(editor->selected_machine->audio->notation->data)->base_note;

  j = j % 12;
  
  /* apply base note */
  if(!g_ascii_strncasecmp(base_note,
		      "A\0",
		      2)){
    j = (11 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 11) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "A#\0",
		      3)){
    j = (10 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 10) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "H\0",
		      2)){
    j = (9 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 9) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "C\0",
		      2)){
    j = (8 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 8) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "C#\0",
		      3)){
    j = (7 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 7) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "D\0",
		      2)){
    j = (6 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 6) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "D#\0",
		      3)){
    j = (5 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 5) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "E\0",
		      2)){
    j = (4 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 4) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "F\0",
		      2)){
    j = (3 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 3) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "F#\0",
		      3)){
    j = (2 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads * 2) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "G\0",
		      2)){
    j = (1 + j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + (editor->selected_machine->audio->input_pads) + 4;
  }else if(!g_ascii_strncasecmp(base_note,
		      "G#\0",
		      3)){
    j = (j) + (editor->selected_machine->audio->input_pads % 12) * (j * editor->selected_machine->audio->input_pads) + 4;
  }

  j = j % 12;
  
  if(y[0] != 0){
    y[0] = control_height - y[0];
  }

  y[1] = ((guint) (widget->allocation.height - border_top) - y[0]) % control_height;
  i_stop = ((widget->allocation.height - border_top) - y[0] - y[1]) / control_height;

  cr = gdk_cairo_create(widget->window);

  if(cr == NULL){
    return;
  }
  
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_rectangle(cr, 0.0, (double) border_top, (double) widget->allocation.width, (double) widget->allocation.height - border_top);
  cairo_fill(cr);

  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb(cr, 0.68, 0.68, 0.68);

  if(y[0] != 0){
    j0 = (j != 0) ? j -1: 11;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

      cairo_rectangle(cr, 0.0, border_top, 2.0 / 3.0 * (double) widget->allocation.width, (double) y[0]);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.68, 0.68, 0.68);

      if(y[0] > control_height / 2){
	cairo_move_to(cr, 2.0 / 3.0 * (double) widget->allocation.width, (double) (y[0] - control_height / 2) + border_top);
	cairo_line_to(cr, (double) widget->allocation.width, (double) (y[0] - control_height / 2) + border_top);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) widget->allocation.width, (double) y[0] + border_top);
      cairo_line_to(cr, (double) widget->allocation.width, (double) y[0] + border_top);
      cairo_stroke(cr);
    }else{
      if(((1 << (j0 + 1)) & bitmap) == 0){
	cairo_move_to(cr, 0.0, (double) y[0] + border_top);
	cairo_line_to(cr, (double) widget->allocation.width, (double) y[0] + border_top);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) widget->allocation.width,  (double) y[0] + border_top);
      cairo_line_to(cr, (double) widget->allocation.width, (double) y[0] + border_top);
      cairo_stroke(cr);
    }
  }

  for(i = 0; i < i_stop; i++){
    if(((1 << j) & bitmap) != 0){
      // draw semi tone key
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

      cairo_rectangle(cr, 0.0, (double) (i * control_height + y[0] + border_top), 2.0 / 3.0 * (double) widget->allocation.width, (double) control_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.68, 0.68, 0.68);

      cairo_move_to(cr, 2.0 / 3.0 * (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top + control_height / 2));
      cairo_line_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top + control_height / 2));
      cairo_stroke(cr);

      cairo_move_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top));
      cairo_line_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top + control_height));
      cairo_stroke(cr);
    }else{
      // no semi tone key
      if(((1 << (j + 1)) & bitmap) == 0){
	cairo_move_to(cr, 0.0, (double) (i * control_height + y[0] + border_top + control_height));
	cairo_line_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top + control_height));
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top));
      cairo_line_to(cr, (double) widget->allocation.width, (double) (i * control_height + y[0] + border_top + control_height));
      cairo_stroke(cr);
    }

    if(j == 11)
      j = 0;
    else
      j++;
  }

  if(y[1] != 0){
    j0 = j;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);

      cairo_rectangle(cr, 0.0, (double) (widget->allocation.height - y[1]), 2.0 / 3.0 * (double) widget->allocation.width, (double) y[1]);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.68, 0.68, 0.68);

      if(y[1] > control_height / 2){
	cairo_move_to(cr, 2.0 / 3.0 * (double) widget->allocation.width, (double) (widget->allocation.height - y[1] + control_height / 2));
	cairo_line_to(cr, (double) widget->allocation.width, (double) ((widget->allocation.height) - y[1] + control_height / 2));
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) widget->allocation.width, (double) (widget->allocation.height - y[1]));
      cairo_line_to(cr, (double) widget->allocation.width, (double) widget->allocation.height);
      cairo_stroke(cr);
    }else{
      cairo_move_to(cr, (double) widget->allocation.width, (double) (widget->allocation.height - y[1]));
      cairo_line_to(cr, (double) widget->allocation.width, (double) widget->allocation.height);
      cairo_stroke(cr);
    }
  }
}

/**
 * ags_meter_new:
 *
 * Create a new #AgsMeter.
 *
 * Returns: a new #AgsMeter
 *
 * Since: 0.4
 */
AgsMeter*
ags_meter_new()
{
  AgsMeter *meter;

  meter = (AgsMeter *) g_object_new(AGS_TYPE_METER, NULL);

  return(meter);
}
