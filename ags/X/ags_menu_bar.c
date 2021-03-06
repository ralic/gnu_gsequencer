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

#include <ags/X/ags_menu_bar.h>
#include <ags/X/ags_menu_bar_callbacks.h>

#include <ags/object/ags_connectable.h>

#include <ags/plugin/ags_base_plugin.h>
#include <ags/plugin/ags_ladspa_manager.h>
#include <ags/plugin/ags_dssi_manager.h>
#include <ags/plugin/ags_lv2_manager.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <ladspa.h>
#include <lv2.h>
#include <dssi.h>

void ags_menu_bar_class_init(AgsMenuBarClass *menu_bar);
void ags_menu_bar_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_menu_bar_init(AgsMenuBar *menu_bar);
void ags_menu_bar_set_property(GObject *gobject,
			       guint prop_id,
			       const GValue *value,
			       GParamSpec *param_spec);
void ags_menu_bar_get_property(GObject *gobject,
			       guint prop_id,
			       GValue *value,
			       GParamSpec *param_spec);
void ags_menu_bar_connect(AgsConnectable *connectable);
void ags_menu_bar_disconnect(AgsConnectable *connectable);

/**
 * SECTION:ags_menu_bar
 * @short_description: the menu bar.
 * @title: AgsMenuBar
 * @section_id:
 * @include: ags/X/ags_menu_bar.h
 *
 * #AgsMenuBar is a composite widget to be used as application's menu bar.
 */

GType
ags_menu_bar_get_type(void)
{
  static GType ags_type_menu_bar = 0;

  if(!ags_type_menu_bar){
    static const GTypeInfo ags_menu_bar_info = {
      sizeof (AgsMenuBarClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_menu_bar_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsMenuBar),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_menu_bar_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_menu_bar_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_menu_bar = g_type_register_static(GTK_TYPE_MENU_BAR,
					       "AgsMenuBar\0", &ags_menu_bar_info,
					       0);
    
    g_type_add_interface_static(ags_type_menu_bar,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_menu_bar);
}

void
ags_menu_bar_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_menu_bar_connect;
  connectable->disconnect = ags_menu_bar_disconnect;
}

void
ags_menu_bar_class_init(AgsMenuBarClass *menu_bar)
{
}

void
ags_menu_bar_init(AgsMenuBar *menu_bar)
{
  GtkImageMenuItem *item;
  GtkAccelGroup *accel_group;
  
  accel_group = gtk_accel_group_new();
  
  /* File */
  item = (GtkImageMenuItem *) gtk_menu_item_new_with_mnemonic("_File\0");
  gtk_menu_shell_append((GtkMenuShell*) menu_bar, (GtkWidget*) item);

  menu_bar->file = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) menu_bar->file);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file, (GtkWidget*) item);

  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file,
			(GtkWidget*) gtk_separator_menu_item_new());

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("export\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file, (GtkWidget*) item);

  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file,
			(GtkWidget*) gtk_separator_menu_item_new());

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->file, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_EDIT, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar, (GtkWidget*) item);

  /* Edit */
  menu_bar->edit = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) menu_bar->edit);

  //  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO, accel_group);
  //  gtk_menu_shell_append((GtkMenuShell*) menu, (GtkWidget*) item);

  //  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO, accel_group);
  //  gtk_menu_shell_append((GtkMenuShell*) menu, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->edit, (GtkWidget*) item);

  menu_bar->add = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) menu_bar->add);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Panel\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Mixer\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Drum\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Matrix\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Synth\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("FPlayer\0"));
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  /* bridge */
  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("LADSPA\0"));
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) ags_ladspa_bridge_menu_new());
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("DSSI\0"));
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) ags_dssi_bridge_menu_new());
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Lv2\0"));
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) ags_lv2_bridge_menu_new());
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  /* live */
  menu_bar->live = (GtkMenu *) gtk_menu_new();
  
  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label("live!\0");
  gtk_menu_item_set_submenu((GtkMenuItem*) item, menu_bar->live);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->add, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("DSSI\0"));
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) ags_live_dssi_bridge_menu_new());
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->live, (GtkWidget*) item);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Lv2\0"));
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) ags_live_lv2_bridge_menu_new());
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->live, (GtkWidget*) item);  

  /* edit */
  item = (GtkImageMenuItem *) gtk_image_menu_item_new_with_label(g_strdup("Automation\0"));
  //  gtk_widget_set_sensitive(item,
  //			   FALSE);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->edit, (GtkWidget*) item);

  /*  */
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->edit,
			(GtkWidget*) gtk_separator_menu_item_new());

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->edit, (GtkWidget*) item);

  /* Help */
  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar, (GtkWidget*) item);

  menu_bar->help = (GtkMenu *) gtk_menu_new();
  gtk_menu_item_set_submenu((GtkMenuItem*) item, (GtkWidget*) menu_bar->help);

  item = (GtkImageMenuItem *) gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel_group);
  gtk_menu_shell_append((GtkMenuShell*) menu_bar->help, (GtkWidget*) item);
}

void
ags_menu_bar_connect(AgsConnectable *connectable)
{
  AgsMenuBar *menu_bar;
  GList *list0, *list1, *list2, *list3, *list4;
  GList *list1_start, *list2_start, *list3_start, *list4_start;

  menu_bar = AGS_MENU_BAR(connectable);

  /* File */
  list0 = GTK_MENU_SHELL(menu_bar)->children;

  list1_start = 
    list1 = gtk_container_get_children ((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list0->data));

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_open_callback), (gpointer) menu_bar);
  list1 = list1->next;

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_save_callback), (gpointer) menu_bar);
  list1 = list1->next;

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_save_as_callback), menu_bar);
  list1 = list1->next->next;

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_export_callback), menu_bar);
  list1 = list1->next->next;

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_quit_callback), (gpointer) menu_bar);

  g_list_free(list1_start);

  /* Edit */
  list0 = list0->next;
  list1_start = 
    list1 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list0->data));

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_callback), (gpointer) menu_bar);

  list2_start = 
    list2 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list1->data));
  list1 = list1->next;

  /* machines */
  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_panel_callback), (gpointer) menu_bar);
  list2 = list2->next;

  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_mixer_callback), (gpointer) menu_bar);
  list2 = list2->next;

  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_drum_callback), (gpointer) menu_bar);
  list2 = list2->next;

  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_matrix_callback), (gpointer) menu_bar);
  list2 = list2->next;

  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_synth_callback), (gpointer) menu_bar);
  list2 = list2->next;

  g_signal_connect (G_OBJECT (list2->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_add_ffplayer_callback), (gpointer) menu_bar);
  list2 = list2->next;

  /* ladspa */
  list3_start = 
    list3 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list2->data));
  list2 = list2->next;

  while(list3 != NULL){
    g_signal_connect(G_OBJECT(list3->data), "activate\0",
		     G_CALLBACK(ags_menu_bar_add_ladspa_bridge_callback), (gpointer) menu_bar);
    
    list3 = list3->next;
  }
  
  g_list_free(list3_start);
  
  /* dssi */
  list3_start = 
    list3 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list2->data));
  list2 = list2->next;

  while(list3 != NULL){
    g_signal_connect(G_OBJECT(list3->data), "activate\0",
		     G_CALLBACK(ags_menu_bar_add_dssi_bridge_callback), (gpointer) menu_bar);

    list3 = list3->next;
  }
  
  g_list_free(list3_start);
  
  /* lv2 */
  list3_start = 
    list3 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list2->data));
  list2 = list2->next;
  
  while(list3 != NULL){
    g_signal_connect(G_OBJECT(list3->data), "activate\0",
		     G_CALLBACK(ags_menu_bar_add_lv2_bridge_callback), (gpointer) menu_bar);
    
    list3 = list3->next;
  }

  g_list_free(list3_start);

  /* live! */
  list3 =
    list3_start = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list2->data));
  
  /* dssi */
  list4_start = 
    list4 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list3->data));
  list3 = list3->next;

  while(list4 != NULL){
    g_signal_connect(G_OBJECT(list4->data), "activate\0",
		     G_CALLBACK(ags_menu_bar_add_live_dssi_bridge_callback), (gpointer) menu_bar);

    list4 = list4->next;
  }
  
  g_list_free(list4_start);
  
  /* lv2 */
  list4_start = 
    list4 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list3->data));
  //  list3 = list3->next;
  
  while(list4 != NULL){
    g_signal_connect(G_OBJECT(list4->data), "activate\0",
		     G_CALLBACK(ags_menu_bar_add_live_lv2_bridge_callback), (gpointer) menu_bar);
    
    list4 = list4->next;
  }

  g_list_free(list4_start);
  g_list_free(list3_start);
  g_list_free(list2_start);

  /* automation and preferences */
  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_automation_callback), (gpointer) menu_bar);
  list1 = list1->next;
  list1 = list1->next;

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_preferences_callback), (gpointer) menu_bar);

  g_list_free(list1_start);

  /* Help */
  list0 = list0->next;
  list1_start = 
    list1 = gtk_container_get_children((GtkContainer *) gtk_menu_item_get_submenu((GtkMenuItem *) list0->data));

  g_signal_connect (G_OBJECT (list1->data), "activate\0",
                    G_CALLBACK (ags_menu_bar_about_callback), (gpointer) menu_bar);

  g_list_free(list1_start);
}

void
ags_menu_bar_disconnect(AgsConnectable *connectable)
{
  /* empty */
}

GtkMenu*
ags_zoom_menu_new()
{
  GtkMenu *menu;
  GtkMenuItem *item;
  GtkLabel *label;

  menu = (GtkMenu *) gtk_menu_new();

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("16:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("8:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("4:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("2:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);


  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:2\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:4\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:8\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:16\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  return(menu);
}

GtkMenu*
ags_tact_menu_new()
{
  GtkMenu *menu;
  GtkMenuItem *item;
  GtkLabel *label;

  menu = (GtkMenu *) gtk_menu_new();

  //TODO:JK: uncomment me if tact implemented
  /*
  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("16/1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("8/1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("4/1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("2/1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);


  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1:1\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);


  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1/2\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1/4\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1/8\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);

  item = (GtkMenuItem *) gtk_menu_item_new();
  label = (GtkLabel *) gtk_label_new(g_strdup("1/16\0"));
  gtk_container_add((GtkContainer *) item, (GtkWidget *) label);
  gtk_menu_shell_append((GtkMenuShell *) menu, (GtkWidget *) item);
  */

  return(menu);
}

/**
 * ags_zoom_combo_box_new:
 *
 * Creates an #GtkComboBox to select zoom.
 *
 * Returns: a new #GtkComboBox
 *
 * Since: 0.4
 */
GtkComboBox*
ags_zoom_combo_box_new()
{
  GtkComboBoxText *combo_box;

  combo_box = (GtkComboBoxText *) gtk_combo_box_text_new();

  //  gtk_combo_box_text_append_text(combo_box,
  //				 "16:1\0");
  //  gtk_combo_box_text_append_text(combo_box,
  //				 "8:1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "4:1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "2:1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1:1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1:2\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1:4\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1:8\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1:16\0");

  return((GtkComboBox *) combo_box);
}

/**
 * ags_zoom_combo_box_new:
 *
 * Creates an #GtkComboBox to select tact
 *
 * Returns: a new #GtkComboBox
 *
 * Since: 0.4
 */
GtkComboBox*
ags_tact_combo_box_new()
{
  GtkComboBoxText *combo_box;

  combo_box = (GtkComboBoxText *) gtk_combo_box_text_new();

  gtk_combo_box_text_append_text(combo_box,
				 "16/1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "8/1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "4/1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "2/1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1/1\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1/2\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1/4\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1/8\0");
  gtk_combo_box_text_append_text(combo_box,
				 "1/16\0");

  return((GtkComboBox *) combo_box);
}

GtkMenu*
ags_ladspa_bridge_menu_new()
{
  GtkMenu *menu;
  GtkImageMenuItem *item;

  AgsLadspaManager *ladspa_manager;

  GList *list, *start;
  
  menu = (GtkMenu *) gtk_menu_new();

  ladspa_manager = ags_ladspa_manager_get_instance();

  start = 
    list = ags_base_plugin_sort(ladspa_manager->ladspa_plugin);

  while(list != NULL){
    item = (GtkImageMenuItem *) gtk_menu_item_new_with_label(AGS_BASE_PLUGIN(list->data)->effect);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_FILENAME_KEY, AGS_BASE_PLUGIN(list->data)->filename);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_EFFECT_KEY, AGS_BASE_PLUGIN(list->data)->effect);
    gtk_menu_shell_append((GtkMenuShell *) menu,
			  (GtkWidget *) item);

    list = list->next;
  }

  g_list_free(start);
  
  return(menu);
}

GtkMenu*
ags_dssi_bridge_menu_new()
{
  GtkMenu *menu;
  GtkImageMenuItem *item;

  AgsDssiManager *dssi_manager;

  GList *list, *start;
  
  menu = (GtkMenu *) gtk_menu_new();

  dssi_manager = ags_dssi_manager_get_instance();

  start = 
    list = ags_base_plugin_sort(dssi_manager->dssi_plugin);

  while(list != NULL){
    item = (GtkImageMenuItem *) gtk_menu_item_new_with_label(AGS_BASE_PLUGIN(list->data)->effect);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_FILENAME_KEY, AGS_BASE_PLUGIN(list->data)->filename);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_EFFECT_KEY, AGS_BASE_PLUGIN(list->data)->effect);
    gtk_menu_shell_append((GtkMenuShell *) menu,
			  (GtkWidget *) item);

    list = list->next;
  }

  g_list_free(start);
  
  return(menu);
}

GtkMenu*
ags_lv2_bridge_menu_new()
{
  GtkMenu *menu;
  GtkImageMenuItem *item;

  AgsLv2Manager *lv2_manager;

  GList *list, *start;
  
  menu = (GtkMenu *) gtk_menu_new();

  lv2_manager = ags_lv2_manager_get_instance();

  start = 
    list = ags_base_plugin_sort(lv2_manager->lv2_plugin);

  while(list != NULL){
    item = (GtkImageMenuItem *) gtk_menu_item_new_with_label(AGS_BASE_PLUGIN(list->data)->effect);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_FILENAME_KEY, AGS_BASE_PLUGIN(list->data)->filename);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_EFFECT_KEY, AGS_BASE_PLUGIN(list->data)->effect);
    
    gtk_menu_shell_append((GtkMenuShell *) menu,
			  (GtkWidget *) item);

    list = list->next;
  }

  g_list_free(start);
  
  return(menu);
}

GtkMenu*
ags_live_dssi_bridge_menu_new()
{
  GtkMenu *menu;
  GtkImageMenuItem *item;

  AgsDssiManager *dssi_manager;

  GList *list, *start;
  
  menu = (GtkMenu *) gtk_menu_new();

  dssi_manager = ags_dssi_manager_get_instance();

  start = 
    list = ags_base_plugin_sort(dssi_manager->dssi_plugin);

  while(list != NULL){
    item = (GtkImageMenuItem *) gtk_menu_item_new_with_label(AGS_BASE_PLUGIN(list->data)->effect);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_FILENAME_KEY, AGS_BASE_PLUGIN(list->data)->filename);
    g_object_set_data((GObject *) item,
		      AGS_MENU_ITEM_EFFECT_KEY, AGS_BASE_PLUGIN(list->data)->effect);
    gtk_menu_shell_append((GtkMenuShell *) menu,
			  (GtkWidget *) item);

    list = list->next;
  }

  g_list_free(start);
  
  return(menu);
}

GtkMenu*
ags_live_lv2_bridge_menu_new()
{
  GtkMenu *menu;
  GtkImageMenuItem *item;

  AgsLv2Manager *lv2_manager;

  GList *list, *start;
  
  menu = (GtkMenu *) gtk_menu_new();

  lv2_manager = ags_lv2_manager_get_instance();

  start = 
    list = ags_base_plugin_sort(lv2_manager->lv2_plugin);

  while(list != NULL){
    if((AGS_LV2_PLUGIN_IS_SYNTHESIZER & (AGS_LV2_PLUGIN(list->data)->flags)) != 0){
      item = (GtkImageMenuItem *) gtk_menu_item_new_with_label(AGS_BASE_PLUGIN(list->data)->effect);
      g_object_set_data((GObject *) item,
			AGS_MENU_ITEM_FILENAME_KEY, AGS_BASE_PLUGIN(list->data)->filename);
      g_object_set_data((GObject *) item,
			AGS_MENU_ITEM_EFFECT_KEY, AGS_BASE_PLUGIN(list->data)->effect);
      g_message("%s %s\0", AGS_BASE_PLUGIN(list->data)->filename, AGS_BASE_PLUGIN(list->data)->effect);
    
      gtk_menu_shell_append((GtkMenuShell *) menu,
			    (GtkWidget *) item);
    }
    
    list = list->next;
  }
  
  return(menu);
}

/**
 * ags_menu_bar_new:
 *
 * Creates an #AgsMenuBar
 *
 * Returns: a new #AgsMenuBar
 *
 * Since: 0.3
 */
AgsMenuBar*
ags_menu_bar_new()
{
  AgsMenuBar *menu_bar;

  menu_bar = (AgsMenuBar *) g_object_new(AGS_TYPE_MENU_BAR,
					 NULL);

  return(menu_bar);
}
