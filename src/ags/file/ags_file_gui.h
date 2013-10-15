/* AGS - Advanced GTK Sequencer
 * Copyright (C) 2013 Joël Krähemann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __AGS_FILE_GUI_H__
#define __AGS_FILE_GUI_H__

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <libxml/tree.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_menu_bar.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_editor.h>
#include <ags/X/ags_navigation.h>
#include <ags/X/ags_machine_editor.h>
#include <ags/X/ags_pad_editor.h>
#include <ags/X/ags_line_editor.h>
#include <ags/X/ags_link_editor.h>
#include <ags/X/ags_line_member_editor.h>

#include <ags/X/editor/ags_toolbar.h>
#include <ags/X/editor/ags_notebook.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_oscillator.h>
#include <ags/X/machine/ags_ffplayer.h>

/* AgsWindow */
void ags_file_read_window(AgsFile *file, xmlNode *node, AgsWindow **window);
xmlNode* ags_file_write_window(AgsFile *file, xmlNode *parent, AgsWindow *window);

/* AgsMenuBar */
void ags_file_read_menu_bar(AgsFile *file, xmlNode *node, AgsMenuBar **menu_bar);
xmlNode* ags_file_write_menu_bar(AgsFile *file, xmlNode *parent, AgsMenuBar *menu_bar);

/* AgsMachineCounter */
void ags_file_read_machine_counter(AgsFile *file, xmlNode *node, AgsMachineCounter **machine_counter);
xmlNode* ags_file_write_machine_counter(AgsFile *file, xmlNode *parent, AgsMachineCounter *machine_counter);

void ags_file_read_machine_counter_list(AgsFile *file, xmlNode *node, GList **machine_counter);
xmlNode* ags_file_write_machine_counter_list(AgsFile *file, xmlNode *parent, GList *machine_counter);

/* AgsMachine */
void ags_file_read_machine(AgsFile *file, xmlNode *node, AgsMachine **machine);
xmlNode* ags_file_write_machine(AgsFile *file, xmlNode *parent, AgsMachine *machine);

void ags_file_read_machine_list(AgsFile *file, xmlNode *node, GList **machine);
xmlNode* ags_file_write_machine_list(AgsFile *file, xmlNode *parent, GList *machine);

void ags_file_read_panel(AgsFile *file, xmlNode *node, AgsMachine *panel);
xmlNode* ags_file_write_panel(AgsFile *file, xmlNode *parent, AgsMachine *panel);

void ags_file_read_mixer(AgsFile *file, xmlNode *node, AgsMachine *mixer);
xmlNode* ags_file_write_mixer(AgsFile *file, xmlNode *parent, AgsMachine *mixer);

void ags_file_read_drum(AgsFile *file, xmlNode *node, AgsMachine *drum);
xmlNode* ags_file_write_drum(AgsFile *file, xmlNode *parent, AgsMachine *drum);

void ags_file_read_matrix(AgsFile *file, xmlNode *node, AgsMachine *matrix);
xmlNode* ags_file_write_matrix(AgsFile *file, xmlNode *parent, AgsMachine *matrix);

void ags_file_read_synth(AgsFile *file, xmlNode *node, AgsMachine *synth);
xmlNode* ags_file_write_synth(AgsFile *file, xmlNode *parent, AgsMachine *synth);

void ags_file_read_ffplayer(AgsFile *file, xmlNode *node, AgsMachine *ffplayer);
xmlNode* ags_file_write_ffplayer(AgsFile *file, xmlNode *parent, AgsMachine *ffplayer);

/* AgsPad */
void ags_file_read_pad(AgsFile *file, xmlNode *node, AgsPad **pad);
xmlNode* ags_file_write_pad(AgsFile *file, xmlNode *parent, AgsPad *pad);

void ags_file_read_pad_list(AgsFile *file, xmlNode *node, GList **pad);
xmlNode* ags_file_write_pad_list(AgsFile *file, xmlNode *parent, GList *pad);

void ags_file_read_mixer_input_pad(AgsFile *file, xmlNode *node, AgsPad *mixer_input_pad);
xmlNode* ags_file_write_mixer_input_pad(AgsFile *file, xmlNode *parent, AgsPad *mixer_input_pad);

void ags_file_read_mixer_output_pad(AgsFile *file, xmlNode *node, AgsPad *mixer_output_pad);
xmlNode* ags_file_write_mixer_output_pad(AgsFile *file, xmlNode *parent, AgsPad *mixer_output_pad);

void ags_file_read_drum_input_pad(AgsFile *file, xmlNode *node, AgsPad *drum_input_pad);
xmlNode* ags_file_write_drum_input_pad(AgsFile *file, xmlNode *parent, AgsPad *drum_input_pad);

void ags_file_read_drum_output_pad(AgsFile *file, xmlNode *node, AgsPad *drum_output_pad);
xmlNode* ags_file_write_drum_output_pad(AgsFile *file, xmlNode *parent, AgsPad *drum_output_pad);

/* AgsLine */
void ags_file_read_line(AgsFile *file, xmlNode *node, AgsLine **line);
xmlNode* ags_file_write_line(AgsFile *file, xmlNode *parent, AgsLine *line);

void ags_file_read_line_list(AgsFile *file, xmlNode *node, GList **line);
xmlNode* ags_file_write_line_list(AgsFile *file, xmlNode *parent, GList *line);

void ags_file_read_drum_input_line(AgsFile *file, xmlNode *node, AgsLine *drum_output_line);
xmlNode* ags_file_write_drum_input_line(AgsFile *file, xmlNode *parent, AgsLine *drum_output_line);

void ags_file_read_drum_output_line(AgsFile *file, xmlNode *node, AgsLine *drum_output_line);
xmlNode* ags_file_write_drum_output_line(AgsFile *file, xmlNode *parent, AgsLine *drum_output_line);

void ags_file_read_oscillator(AgsFile *file, xmlNode *node, AgsOscillator *oscillator);
xmlNode* ags_file_write_oscillator(AgsFile *file, xmlNode *parent, AgsOscillator *oscillator);

/* AgsLineMember */
void ags_file_read_line_member(AgsFile *file, xmlNode *node, AgsLineMember **line_member);
xmlNode* ags_file_write_line_member(AgsFile *file, xmlNode *parent, AgsLineMember *line_member);

void ags_file_read_line_member_list(AgsFile *file, xmlNode *node, GList **line_member);
xmlNode* ags_file_write_line_member_list(AgsFile *file, xmlNode *parent, GList *line_member);

/* AgsMachineEditor */
//TODO:JK: dialogs are missing for now

/* AgsEditor */
void ags_file_read_editor(AgsFile *file, xmlNode *node, AgsEditor **editor);
xmlNode* ags_file_write_editor(AgsFile *file, xmlNode *parent, AgsEditor *editor);

/* AgsToolbar */
void ags_file_read_toolbar(AgsFile *file, xmlNode *node, AgsToolbar **toolbar);
xmlNode* ags_file_write_toolbar(AgsFile *file, xmlNode *parent, AgsToolbar *toolbar);

/* AgsNotebook */
void ags_file_read_notebook(AgsFile *file, xmlNode *node, AgsNotebook **notebook);
xmlNode* ags_file_write_notebook(AgsFile *file, xmlNode *parent, AgsNotebook *notebook);

/* AgsNavigation */
void ags_file_read_navigation(AgsFile *file, xmlNode *node, AgsNavigation **navigation);
xmlNode* ags_file_write_navigation(AgsFile *file, xmlNode *parent, AgsNavigation *navigation);

#endif /*__AGS_FILE_GUI_H__*/