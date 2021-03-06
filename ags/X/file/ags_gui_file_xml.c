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

#include <ags/X/file/ags_gui_file_xml.h>

#include <ags/object/ags_application_context.h>

#include <ags/plugin/ags_plugin_factory.h>

#include <ags/object/ags_soundcard.h>
#include <ags/object/ags_plugin.h>

#include <ags/util/ags_id_generator.h>

#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>
#include <ags/file/ags_file_lookup.h>
#include <ags/file/ags_file_launch.h>
#include <ags/file/ags_file_util.h>

#include <ags/audio/ags_channel.h>
#include <ags/audio/ags_output.h>
#include <ags/audio/ags_input.h>
#include <ags/audio/ags_recall.h>
#include <ags/audio/ags_recall_container.h>

#include <ags/audio/recall/ags_peak_channel.h>
#include <ags/audio/recall/ags_peak_channel_run.h>

#include <ags/widget/ags_container.h>

#include <ags/X/ags_machine_callbacks.h>
#include <ags/X/ags_line_callbacks.h>
#include <ags/X/ags_effect_bridge_callbacks.h>
#include <ags/X/ags_effect_bulk_callbacks.h>

#include <ags/X/machine/ags_drum_input_line_callbacks.h>

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>

#include <math.h>

#define AGS_FILE_READ_EDITOR_PARAMETER_NAME "ags-file-read-editor-parameter-name\0"

void ags_file_read_window_resolve_soundcard(AgsFileLookup *file_lookup,
					    AgsWindow *window);
void ags_file_write_window_resolve_soundcard(AgsFileLookup *file_lookup,
					     AgsWindow *window);

void ags_file_read_machine_resolve_machine_editor(AgsFileLookup *file_lookup,
						  AgsMachine *machine);
void ags_file_write_machine_resolve_machine_editor(AgsFileLookup *file_lookup,
						   AgsMachine *machine);
void ags_file_write_machine_resolve_audio(AgsFileLookup *file_lookup,
					  AgsMachine *machine);
void ags_file_read_machine_resolve_rename_dialog(AgsFileLookup *file_lookup,
						 AgsMachine *machine);
void ags_file_write_machine_resolve_rename_dialog(AgsFileLookup *file_lookup,
						  AgsMachine *machine);

void ags_file_read_pad_resolve_channel(AgsFileLookup *file_lookup,
				       AgsPad *pad);

void ags_file_read_line_resolve_channel(AgsFileLookup *file_lookup,
					AgsLine *line);

void ags_file_read_line_member_resolve_port(AgsFileLookup *file_lookup,
					    AgsLineMember *line_member);
void ags_file_write_line_member_resolve_port(AgsFileLookup *file_lookup,
					     AgsLineMember *line_member);

void ags_file_write_effect_bridge_resolve_audio(AgsFileLookup *file_lookup,
						AgsEffectBridge *effect_bridge);

void ags_file_write_effect_bulk_resolve_audio(AgsFileLookup *file_lookup,
					      AgsEffectBulk *effect_bulk);

void ags_file_read_bulk_member_resolve_port(AgsFileLookup *file_lookup,
					    AgsBulkMember *bulk_member);

void ags_file_read_effect_pad_resolve_channel(AgsFileLookup *file_lookup,
					      AgsPad *pad);

void ags_file_read_effect_line_resolve_channel(AgsFileLookup *file_lookup,
					       AgsLine *line);

void ags_file_read_editor_resolve_parameter(AgsFileLookup *file_lookup,
					    AgsEditor *editor);
void ags_file_read_editor_launch(AgsFileLaunch *file_launch,
				 AgsEditor *editor);

void ags_file_read_machine_selector_resolve_parameter(AgsFileLookup *file_lookup,
						      AgsMachineSelector *machine_selector);

void ags_file_read_navigation_resolve_soundcard(AgsFileLookup *file_lookup,
					     AgsNavigation *navigation);

void ags_file_write_automation_area_resolve_audio(AgsFileLookup *file_lookup,
						    AgsAutomationArea *automation_area);
xmlNode* ags_file_read_automation_area_resolve_audio(AgsFileLookup *file_lookup,
						     AgsAutomationArea *automation_area);

void
ags_file_read_widget(AgsFile *file, xmlNode *node, GtkWidget *widget)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_widget(AgsFile *file, xmlNode *parent, GtkWidget *widget)
{
  //TODO:JK: implement me
}

void
ags_file_read_window(AgsFile *file, xmlNode *node, AgsWindow **window)
{
  AgsWindow *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  xmlChar *prop, *content;
  xmlChar *str;
  
  if(*window == NULL){
    gobject = g_object_new(AGS_TYPE_WINDOW,
			   NULL);
    *window = gobject;
  }else{
    gobject = *window;
  }

  g_object_set(G_OBJECT(gobject),
	       "application-context\0", file->application_context,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  str = xmlGetProp(node, "name\0");
  
  if(str != NULL){
    gobject->name = (gchar *) g_strdup(str);
  }else{
    gobject->name = (gchar *) g_strdup(file->filename);
  }
  
  /* soundcard */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_window_resolve_soundcard), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){

    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-menu-bar\0",
		     11)){
	ags_file_read_menu_bar(file,
			       child,
			       &(gobject->menu_bar));
      }else if(!xmlStrncmp(child->name,
			   "ags-machine-counter-list\0",
			   24)){
	ags_file_read_machine_counter_list(file,
					   child,
					   &(gobject->machine_counter));
      }else if(!xmlStrncmp(child->name,
			   "ags-machine-list\0",
			   16)){
	GList *list;

	list = NULL;

	ags_file_read_machine_list(file,
				   child,
				   &list);

	while(list != NULL){
	  gtk_box_pack_start((GtkBox *) gobject->machines,
			     GTK_WIDGET(list->data),
			     FALSE, FALSE, 0);

	  list = list->next;
	}
      }else if(!xmlStrncmp(child->name,
			   "ags-editor\0",
			   10)){
	ags_file_read_editor(file,
			     child,
			     &(gobject->editor));
      }else if(!xmlStrncmp(child->name,
			   "ags-navigation\0",
			   14)){
	ags_file_read_navigation(file,
				 child,
				 &(gobject->navigation));
      }
    }

    child = child->next;
  }
}

void
ags_file_read_window_resolve_soundcard(AgsFileLookup *file_lookup,
				    AgsWindow *window)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "soundcard\0");

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(id_ref != NULL){
    g_object_set(G_OBJECT(window),
		 "soundcard\0", (AgsWindow *) id_ref->ref,
		 NULL);
  }
}

xmlNode*
ags_file_write_window(AgsFile *file, xmlNode *parent, AgsWindow *window)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-window\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", window,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", window->flags));

  xmlNewProp(node,
	     "name\0",
	     g_strdup(window->name));

  /* soundcard */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", window,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_write_window_resolve_soundcard), window);

  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_file_write_menu_bar(file,
			  node,
			  window->menu_bar);

  ags_file_write_machine_list(file,
			      node,
			      gtk_container_get_children(GTK_CONTAINER(window->machines)));

  ags_file_write_editor(file,
			node,
			window->editor);

  ags_file_write_navigation(file,
			    node,
			    window->navigation);
}

void
ags_file_write_window_resolve_soundcard(AgsFileLookup *file_lookup,
				     AgsWindow *window)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, window->soundcard);

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "soundcard\0",
	     g_strdup_printf("xpath=//ags-soundcard[@id='%s']\0", id));
}

void
ags_file_read_menu_bar(AgsFile *file, xmlNode *node, AgsMenuBar **menu_bar)
{
  AgsMenuBar *gobject;
  xmlNode *child;
  xmlChar *prop, *content;

  if(*menu_bar == NULL){
    gobject = g_object_new(AGS_TYPE_MENU_BAR,
			   NULL);
    *menu_bar = gobject;
  }else{
    gobject = *menu_bar;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));
}

xmlNode*
ags_file_write_menu_bar(AgsFile *file, xmlNode *parent, AgsMenuBar *menu_bar)
{
  xmlNode *node, *child;
  gchar *id;
  guint i;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-menu-bar\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", menu_bar,
				   NULL));

  xmlAddChild(parent,
	      node);
}

void
ags_file_read_machine_counter(AgsFile *file, xmlNode *node, AgsMachineCounter **machine_counter)
{
  AgsMachineCounter *ptr;
  xmlNode *child;
  xmlChar *prop, *content;

  if(*machine_counter == NULL){
    ptr = ags_machine_counter_alloc(NULL, NULL,
				    G_TYPE_NONE, 0);
    *machine_counter = ptr;
  }else{
    ptr = *machine_counter;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", ptr,
				   NULL));

  ptr->version = xmlGetProp(node,
			    AGS_FILE_VERSION_PROP);

  ptr->build_id = xmlGetProp(node,
			     AGS_FILE_BUILD_ID_PROP);

  ptr->machine_type = g_type_from_name(xmlGetProp(node,
						  AGS_FILE_TYPE_PROP));

  ptr->counter = g_ascii_strtoull(xmlGetProp(node,
					     "counter\0"),
				  NULL,
				  10);
}

xmlNode*
ags_file_write_machine_counter(AgsFile *file, xmlNode *parent, AgsMachineCounter *machine_counter)
{
  xmlNode *node, *child;
  gchar *id;
  guint i;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-machine-counter\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", machine_counter,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     machine_counter->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     machine_counter->build_id);

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     g_type_name(machine_counter->machine_type));

  xmlNewProp(node,
	     "counter\0",
	     g_strdup_printf("%d\0", machine_counter->counter));

  xmlAddChild(parent,
	      node);
}

void
ags_file_read_machine_counter_list(AgsFile *file, xmlNode *node, GList **machine_counter)
{
  AgsMachineCounter *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    current = NULL;
    ags_file_read_machine_counter(file, child, &current);

    list = g_list_prepend(list, current);

    child = child->next;
  }

  list = g_list_reverse(list);
  *machine_counter = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_machine_counter_list(AgsFile *file, xmlNode *parent, GList *machine_counter)
{
  AgsMachineCounter *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-machine-counter-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = machine_counter;

  while(list != NULL){
    ags_file_write_machine_counter(file, node, AGS_MACHINE_COUNTER(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_machine(AgsFile *file, xmlNode *node, AgsMachine **machine)
{
  AgsMachine *gobject;
  AgsFileLookup *file_lookup;
  GType machine_type;
  xmlNode *child;

  if(*machine == NULL){
    machine_type = g_type_from_name(xmlGetProp(node,
					       AGS_FILE_TYPE_PROP));

    gobject = (AgsMachine *) g_object_new(machine_type,
					  NULL);
    *machine = gobject;
  }else{
    gobject = *machine;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);
  gobject->flags |= AGS_MACHINE_PREMAPPED_RECALL;
  gobject->flags &= (~AGS_MACHINE_CONNECTED);

  gobject->file_input_flags = (guint) g_ascii_strtoull(xmlGetProp(node, "file-input-flags\0"),
						       NULL,
						       16);

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_machine_resolve_audio), gobject);

  /* machine-editor */
  //TODO:JK: uncomment me
  //  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
  //					       "file\0", file,
  //					       "node\0", node,
  //					       "reference\0", gobject,
  //					       NULL);
  //  ags_file_add_lookup(file, (GObject *) file_lookup);
  //  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
  //		   G_CALLBACK(ags_file_read_machine_resolve_machine_editor), gobject);

  /* rename-dialog */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_machine_resolve_rename_dialog), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-pad-list\0",
			   12)){
	GList *pad, *list;

	pad = NULL;

	ags_file_read_pad_list(file,
			       child,
			       &pad);

	if(!xmlStrncmp(xmlGetProp(child,
				  AGS_FILE_SCOPE_PROP),
		       "output\0",
		       6)){
	  if(!GTK_IS_BOX(gobject->output)){
	    ags_container_add_all(gobject->output,
				  pad);
	  }else{
	    list = pad;

	    while(list != NULL){
	      gtk_box_pack_start(GTK_BOX(gobject->output),
				 GTK_WIDGET(list->data),
				 FALSE, FALSE,
				 0);

	      list = list->next;
	    }
	  }
	}else{
	  if(!GTK_IS_BOX(gobject->input)){
	    ags_container_add_all(gobject->input,
				  pad);
	  }else{
	    list = pad;

	    while(list != NULL){
	      gtk_box_pack_start(GTK_BOX(gobject->input),
				 GTK_WIDGET(list->data),
				 FALSE, FALSE,
				 0);

	      list = list->next;
	    }
	  }
	}

	g_list_free(pad);
      }else if(!xmlStrncmp(child->name,
			   "ags-effect-bridge\0",
			   18)){
	ags_file_read_effect_bridge(file,
				    child,
				    (AgsEffectBridge **) &(gobject->bridge));	
      }
    }

    child = child->next;
  }
}

void
ags_file_read_machine_resolve_audio(AgsFileLookup *file_lookup,
				    AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  gchar *str;
  
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "audio\0");
  g_message("xpath = %s\0", xpath);
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);
  
  g_object_set(G_OBJECT(machine),
	       "audio\0", (AgsAudio *) id_ref->ref,
	       NULL);

  AGS_AUDIO(id_ref->ref)->machine = (GObject *) machine;

  g_signal_connect_after(G_OBJECT(machine->audio), "set_audio_channels\0",
			 G_CALLBACK(ags_machine_set_audio_channels_callback), machine);

  g_signal_connect_after(G_OBJECT(machine->audio), "set_pads\0",
			 G_CALLBACK(ags_machine_set_pads_callback), machine);

  str = xmlGetProp(file_lookup->node,
		   AGS_FILE_NAME_PROP);

  if(str != NULL){
    machine->name = g_strdup(str);
  }
}

void
ags_file_read_machine_resolve_machine_editor(AgsFileLookup *file_lookup,
					     AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "machine-editor\0");

  if(xpath == NULL){
    return;
  }

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  //TODO:JK: use g_object_set
  machine->properties = (GtkDialog *) id_ref->ref;
}

void
ags_file_read_machine_resolve_rename_dialog(AgsFileLookup *file_lookup,
					    AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "rename-dialog\0");

  if(xpath == NULL){
    return;
  }

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  //TODO:JK: use g_object_set
  machine->rename = (GtkDialog *) id_ref->ref;
}

xmlNode*
ags_file_write_machine(AgsFile *file, xmlNode *parent, AgsMachine *machine)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-machine\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", machine,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(machine));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     machine->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     machine->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~(AGS_MACHINE_BLOCK_PLAY | AGS_MACHINE_BLOCK_STOP)) & (machine->flags))));

  xmlNewProp(node,
	     "file-input-flags\0",
	     g_strdup_printf("%x\0", machine->file_input_flags));

  xmlNewProp(node,
	     AGS_FILE_NAME_PROP,
	     machine->name);

  xmlAddChild(parent,
	      node);  

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", machine,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_write_machine_resolve_audio), machine);

  /* machine-editor */
  //TODO:JK: uncomment me
  //  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
  //					       "file\0", file,
  //					       "node\0", node,
  //					       "reference\0", machine,
  //					       NULL);
  //  ags_file_add_lookup(file, (GObject *) file_lookup);
  //  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
  //		   G_CALLBACK(ags_file_write_machine_resolve_machine_editor), machine);

  /* rename-dialog */
  //TODO:JK: uncomment me
  //  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
  //					       "file\0", file,
  //					       "node\0", node,
  //					       "reference\0", machine,
  //					       NULL);
  //  ags_file_add_lookup(file, (GObject *) file_lookup);
  //  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
  //		   G_CALLBACK(ags_file_write_machine_resolve_rename_dialog), machine);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(machine));

  if(machine->output != NULL){
    GList *list;
    
    list = gtk_container_get_children(machine->output);
    child = ags_file_write_pad_list(file,
				    node,
				    list);
    g_list_free(list);
    xmlNewProp(child,
	       AGS_FILE_SCOPE_PROP,
	       "output\0");
  }
  
  if(machine->input != NULL){
    GList *list;

    list = gtk_container_get_children(machine->input);
    child = ags_file_write_pad_list(file,
				    node,
				    list);
    g_list_free(list);
    xmlNewProp(child,
	       AGS_FILE_SCOPE_PROP,
	       "input\0");
  }

  if(machine->bridge != NULL){
    child = ags_file_write_effect_bridge(file,
					 node,
					 (AgsEffectBridge *) machine->bridge);
  }

  return(node);
}

void
ags_file_write_machine_resolve_machine_editor(AgsFileLookup *file_lookup,
					      AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, machine->properties);

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "machine-editor\0",
	     g_strdup_printf("xpath=//ags-machine-editor[@id='%s']\0", id));
}

void
ags_file_write_machine_resolve_rename_dialog(AgsFileLookup *file_lookup,
					     AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, machine->rename);

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "rename-dialog\0",
	     g_strdup_printf("xpath=//ags-dialog[@id='%s']\0", id));
}

void
ags_file_write_machine_resolve_audio(AgsFileLookup *file_lookup,
				     AgsMachine *machine)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, AGS_AUDIO(machine->audio));

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "audio\0",
	     g_strdup_printf("xpath=//ags-audio[@id='%s']\0", id));
}

void
ags_file_read_machine_list(AgsFile *file, xmlNode *node, GList **machine)
{
  AgsMachine *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-machine\0",
		     12)){
	current = NULL;
	ags_file_read_machine(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *machine = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_machine_list(AgsFile *file, xmlNode *parent, GList *machine)
{
  AgsMachine *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-machine-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = machine;

  while(list != NULL){
    ags_file_write_machine(file, node, AGS_MACHINE(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_pad(AgsFile *file, xmlNode *node, AgsPad **pad)
{
  AgsPad *gobject;
  
  AgsFileLookup *file_lookup;

  xmlNode *child;

  GType pad_type;
  
  if(*pad == NULL){
    pad_type = g_type_from_name(xmlGetProp(node,
					   AGS_FILE_TYPE_PROP));
    
    gobject = (AgsPad *) g_object_new(pad_type,
				      NULL);
    *pad = gobject;
  }else{
    gobject = *pad;
  }

  if(gobject == NULL)
    return;
  
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);
  gobject->flags &= (~AGS_PAD_CONNECTED);

  /* channel */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_pad_resolve_channel), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-line-list\0",
			   13)){
	xmlNode *line_node;
	GList *start, *list;

	list = NULL;

	ags_file_read_line_list(file,
				child,
				&list);
	start = list;

	/* add line to pad */
	line_node = child->children;

	while(line_node != NULL){
	  if(line_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(line_node->name,
			   "ags-line\0",
			   8)){
	      guint x, y;
	      guint width, height;

	      g_object_set(G_OBJECT(list->data),
			   "pad\0", gobject,
			   NULL);

	      x = g_ascii_strtoull(xmlGetProp(line_node,
					      "left-attach\0"),
				   NULL,
				   10);

	      y = g_ascii_strtoull(xmlGetProp(line_node,
					      "top-attach\0"),
				   NULL,
				   10);

	      width = g_ascii_strtoull(xmlGetProp(line_node,
						  "right-attach\0"),
				   NULL,
				   10) - x;

	      height = g_ascii_strtoull(xmlGetProp(line_node,
						   "bottom-attach\0"),
				   NULL,
				   10) - y;

	      ags_expander_set_add(gobject->expander_set,
				   GTK_WIDGET(list->data),
				   x, y,
				   width, height);

	      list = list->next;
	    }
	  }

	  line_node = line_node->next;
	}

	g_list_free(start);
      }
    }

    child = child->next;
  }
}

void
ags_file_read_pad_resolve_channel(AgsFileLookup *file_lookup,
				  AgsPad *pad)
{
  AgsFile *file;
  AgsMachine *machine;
  AgsFileIdRef *id_ref;
  xmlNode *node, *audio_node, *channel_node;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  GList *list;
  xmlChar *xpath;
  guint position;
  guint nth, audio_channel;
  guint i, j;
  gboolean is_output;
  
  file = file_lookup->file;

  machine = (AgsMachine *) gtk_widget_get_ancestor(GTK_WIDGET(pad),
						   AGS_TYPE_MACHINE);

  if(machine->output != NULL){
    list = gtk_container_get_children(machine->output);

    if(list != NULL &&
       g_list_find(list,
		   pad) != NULL){
      is_output = TRUE;
    }else{
      is_output = FALSE;
    }
  }else{
    is_output = FALSE;
  }

  node = file_lookup->node;

  /* retrieve position */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent,
  //			 xpath_context);
  xpath_context->node = node->parent;

  xpath_object = xmlXPathEval("./ags-pad\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }
  nth = j;

  /*  */
  position = nth * machine->audio->audio_channels;

  /*  */
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, machine->audio);
  audio_node = NULL;

  if(id_ref != NULL){
    audio_node = id_ref->node;
  }

  /*  */
  xpath = g_strdup_printf("(./ags-channel-list/ags-channel)/%s\0",
			  (is_output ? "ags-output\0": "ags-input\0"));

  xpath_context = xmlXPathNewContext(file->doc);
  xpath_context->node = audio_node;
  xpath_object = xmlXPathEval(xpath,
			      xpath_context);

  /*  */
  if(xpath_object != NULL && xpath_object->nodesetval != NULL){
    AgsFileIdRef *file_id_ref;
    xmlNode *channel_node;

    for(i = 0, j = 0; j < position && i < xpath_object->nodesetval->nodeMax; i++){
      if(xpath_object->nodesetval->nodeTab[i] != NULL && xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
	j++;
      }
    }

    channel_node = xpath_object->nodesetval->nodeTab[i];

    file_id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_node(file,
								channel_node);
    g_object_set(G_OBJECT(pad),
		 "channel\0", AGS_CHANNEL(file_id_ref->ref),
		 NULL);
  }else{
    g_message("no xpath match: %s\0",
	      xpath);
  }
}

xmlNode*
ags_file_write_pad(AgsFile *file, xmlNode *parent, AgsPad *pad)
{
  AgsExpanderSetChild *expander_set_child;

  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *line_node;

  GList *line, *line_start;

  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-pad\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", pad,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(pad));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     pad->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     pad->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_PAD_CONNECTED) & (pad->flags))));

  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(pad));

  line_start = 
    line = gtk_container_get_children(GTK_CONTAINER(pad->expander_set));

  child = ags_file_write_line_list(file,
				   node,
				   line);

  line_node = child->children;

  while(line != NULL){
    expander_set_child = ags_expander_set_child_find(pad->expander_set,
						     line->data);

    xmlNewProp(line_node,
	       "left-attach\0",
	       g_strdup_printf("%d\0", expander_set_child->x));

    xmlNewProp(line_node,
	       "top-attach\0",
	       g_strdup_printf("%d\0", expander_set_child->y));

    xmlNewProp(line_node,
	       "right-attach\0",
	       g_strdup_printf("%d\0", expander_set_child->x + expander_set_child->width));

    xmlNewProp(line_node,
	       "bottom-attach\0",
	       g_strdup_printf("%d\0", expander_set_child->y + expander_set_child->height));
    
    line = line->next;
    line_node = line_node->next;
  }

  g_list_free(line_start);
}

void
ags_file_read_pad_list(AgsFile *file, xmlNode *node, GList **pad)
{
  AgsPad *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-pad\0",
		     8)){
	current = NULL;
	ags_file_read_pad(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *pad = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_pad_list(AgsFile *file, xmlNode *parent, GList *pad)
{
  AgsPad *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-pad-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = pad;

  while(list != NULL){
    ags_file_write_pad(file, node, AGS_PAD(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_line(AgsFile *file, xmlNode *node, AgsLine **line)
{
  AgsLine *gobject;

  AgsFileLookup *file_lookup;

  xmlNode *child;

  GType line_type;

  if(*line == NULL){
    line_type = g_type_from_name(xmlGetProp(node,
					    AGS_FILE_TYPE_PROP));
    
    gobject = (AgsLine *) g_object_new(line_type,
				       NULL);
    *line = gobject;
  }else{
    gobject = *line;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);
  gobject->flags |= AGS_LINE_PREMAPPED_RECALL;
  gobject->flags &= (~AGS_LINE_CONNECTED);

  /* channel */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_line_resolve_channel), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-line-member-list\0",
			   20)){
	xmlNode *line_member_node;
	GList *start, *list;

	list = NULL;

	ags_file_read_line_member_list(file,
				       child,
				       &list);
	start = list;
	
	/* remove default line members */
	gtk_widget_destroy(GTK_WIDGET(gobject->expander));

	gobject->expander = ags_expander_new(1, 1);
	gtk_table_set_row_spacings(gobject->expander->table,
				   2);
	gtk_table_set_col_spacings(gobject->expander->table,
				   2);
	gtk_box_pack_start(GTK_BOX(gobject),
			   GTK_WIDGET(gobject->expander),
			   TRUE, TRUE,
			   0);

	/* add line member to line */
	line_member_node = child->children;

	while(line_member_node != NULL){
	  if(line_member_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(child->name,
			   "ags-line-member\0",
			   15)){
	      guint x, y;
	      guint width, height;

	      /* pack */
	      x = g_ascii_strtoull(xmlGetProp(line_member_node,
					      "left-attach\0"),
				   NULL,
				   10);

	      y = g_ascii_strtoull(xmlGetProp(line_member_node,
					      "top-attach\0"),
				   NULL,
				   10);

	      width = g_ascii_strtoull(xmlGetProp(line_member_node,
						  "right-attach\0"),
				       NULL,
				       10) - x;

	      height = g_ascii_strtoull(xmlGetProp(line_member_node,
						   "bottom-attach\0"),
					NULL,
					10) - y;

	      ags_expander_add(gobject->expander,
			       GTK_WIDGET(list->data),
			       x, y,
			       width, height);	      
	      
	      /* iterate */
	      list = list->next;
	    }
	  }
	  
	  line_member_node = line_member_node->next;
	}

	g_list_free(start);
      }
    }

    child = child->next;
  }
}

void
ags_file_read_line_resolve_channel(AgsFileLookup *file_lookup,
				   AgsLine *line)
{
  AgsFile *file;
  AgsMachine *machine;
  AgsFileIdRef *id_ref;
  xmlNode *pad_node, *node, *audio_node, *channel_node;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  GList *list;
  xmlChar *xpath;
  guint position;
  guint pad, audio_channel;
  guint i, j;
  gboolean is_output;

  file = file_lookup->file;

  machine = (AgsMachine *) gtk_widget_get_ancestor(GTK_WIDGET(line),
						   AGS_TYPE_MACHINE);

  if(machine->output != NULL){
    AgsPad *pad;

    pad = (AgsPad *) gtk_widget_get_ancestor((GtkWidget *) line,
					     AGS_TYPE_PAD);
    list = gtk_container_get_children(machine->output);

    if(list != NULL &&
       g_list_find(list,
		   pad) != NULL){
      is_output = TRUE;
    }else{
      is_output = FALSE;
    }
  }else{
    is_output = FALSE;
  }

  node = file_lookup->node;
  pad_node = node->parent->parent;

  /* retrieve position - pad */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent->parent->parent,
  //			 xpath_context);
  xpath_context->node = pad_node->parent;

  xpath_object = xmlXPathEval("./ags-pad\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != pad_node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }
  pad = j;

  /* retrieve position - line */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent,
  //			 xpath_context);
  xpath_context->node = node->parent;

  xpath_object = xmlXPathEval("./ags-line\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }

  audio_channel = j;

  /*  */
  position = pad * machine->audio->audio_channels + (machine->audio->audio_channels - audio_channel - 1);

  /*  */
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, machine->audio);
  audio_node = NULL;

  if(id_ref != NULL){
    audio_node = id_ref->node;
  }

  /*  */
  xpath = g_strdup_printf("(./ags-channel-list/ags-channel)/%s\0",
			  ((is_output) ? "ags-output\0": "ags-input\0"));

  xpath_context = xmlXPathNewContext(file->doc);
  xpath_context->node = audio_node;
  xpath_object = xmlXPathEval(xpath,
			      xpath_context);

  /*  */
  if(xpath_object != NULL && xpath_object->nodesetval != NULL){
    AgsFileIdRef *file_id_ref;
    xmlNode *channel_node;

    for(i = 0, j = 0; j < position && i < xpath_object->nodesetval->nodeMax; i++){
      if(xpath_object->nodesetval->nodeTab[i] != NULL && xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
	j++;
      }
    }

    channel_node = xpath_object->nodesetval->nodeTab[i];

    file_id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_node(file,
								channel_node);

    g_object_set(G_OBJECT(line),
		 "channel\0", AGS_CHANNEL(file_id_ref->ref),
		 NULL);
  }else{
    g_message("no xpath match: %s\0",
	      xpath);
  }
}

xmlNode*
ags_file_write_line(AgsFile *file, xmlNode *parent, AgsLine *line)
{
  AgsExpanderChild *expander_child;

  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *line_member_node;

  GList *line_member, *line_member_start;

  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-line\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", line,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(line));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     line->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     line->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_LINE_CONNECTED)&(line->flags))));

  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(line));

  line_member_start = 
    line_member = gtk_container_get_children(GTK_CONTAINER(line->expander->table));

  child = ags_file_write_line_member_list(file,
					  node,
					  line_member);

  line_member_node = child->children;

  while(line_member != NULL){
    if(AGS_IS_LINE_MEMBER(line_member->data)){
      expander_child = ags_expander_child_find(line->expander,
					       line_member->data);

      xmlNewProp(line_member_node,
		 "left-attach\0",
		 g_strdup_printf("%d\0", expander_child->x));

      xmlNewProp(line_member_node,
		 "top-attach\0",
		 g_strdup_printf("%d\0", expander_child->y));

      xmlNewProp(line_member_node,
		 "right-attach\0",
		 g_strdup_printf("%d\0", expander_child->x + expander_child->width));

      xmlNewProp(line_member_node,
		 "bottom-attach\0",
		 g_strdup_printf("%d\0", expander_child->y + expander_child->height));

      line_member_node = line_member_node->next;
    }

    line_member = line_member->next;
  }

  g_list_free(line_member_start);
}

void
ags_file_read_line_list(AgsFile *file, xmlNode *node, GList **line)
{
  AgsLine *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-line\0",
		     9)){
	current = NULL;
	ags_file_read_line(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *line = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_line_list(AgsFile *file, xmlNode *parent, GList *line)
{
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-line-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = line;

  while(list != NULL){
    ags_file_write_line(file, node, AGS_LINE(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_line_member(AgsFile *file, xmlNode *node, AgsLineMember **line_member)
{
  AgsLineMember *gobject;
  GtkAdjustment *adjustment;
  GtkWidget *child_widget;

  AgsFileLookup *file_lookup;

  xmlNode *child;

  xmlChar *prop, *content;
  gchar *widget_type;
  gchar *label;
  gchar *task_type;
  guint width, height;

  if(*line_member == NULL){
    gobject = g_object_new(AGS_TYPE_LINE_MEMBER,
			   NULL);
    *line_member = gobject;
  }else{
    gobject = *line_member;

    if(!AGS_IS_LINE_MEMBER(gobject)){
      return;
    }
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  widget_type = (gchar *) xmlGetProp(node, "widget-type\0");
  g_object_set(gobject,
	       "widget-type\0", g_type_from_name(widget_type),
	       NULL);
  child_widget = (GtkWidget *) gtk_bin_get_child(GTK_BIN(gobject));

  /* label */
  label = (gchar *) xmlGetProp(node, "label\0");

  if(label != NULL){
    g_object_set(G_OBJECT(gobject),
		 "widget-label\0", label,
		 NULL);
  }

  /* size */
  width = (guint) g_ascii_strtoull(xmlGetProp(node, "width\0"),
				   NULL,
				   10);

  height = (guint) g_ascii_strtoull(xmlGetProp(node, "height\0"),
				    NULL,
				    10);

  gtk_widget_set_size_request(child_widget,
			      width, height);

  /* check misc */
  if(GTK_IS_MISC(child_widget)){
    gfloat xalign, yalign;
    guint xpad, ypad;

    xalign = (gfloat) g_ascii_strtod(xmlGetProp(node, "xalign\0"),
				     NULL);

    yalign = (gfloat) g_ascii_strtod(xmlGetProp(node, "yalign\0"),
				     NULL);

    xpad = (guint) g_ascii_strtoull(xmlGetProp(node, "xpad\0"),
				    NULL,
				    10);
    
    ypad = (guint) g_ascii_strtoull(xmlGetProp(node, "ypad\0"),
				    NULL,
				    10);
  }

  /* check adjustment and toggle types */
  adjustment = NULL;

  if(GTK_IS_TOGGLE_BUTTON(child_widget)){
    if(!xmlStrncmp(AGS_FILE_TRUE,
		   xmlGetProp(node, "value\0"),
		   5)){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child_widget),
				   TRUE);
    }
  }else if(AGS_IS_DIAL(child_widget)){
    AgsDial *dial;
    
    dial = (AgsDial *) child_widget;
    adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
    g_object_set(child_widget,
		 "adjustment\0", adjustment,
		 NULL);
    gtk_widget_set_size_request((GtkWidget *) dial,
				2 * dial->radius + 2 * dial->outline_strength + dial->button_width + 1,
				2 * dial->radius + 2 * dial->outline_strength + 1);
  }else if(GTK_IS_SPIN_BUTTON(child_widget)){
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(child_widget),
			       3);
    adjustment = GTK_SPIN_BUTTON(child_widget)->adjustment;
  }else if(GTK_IS_RANGE(child_widget)){
    gtk_range_set_round_digits(GTK_RANGE(child_widget),
			       3);
    adjustment = GTK_RANGE(child_widget)->adjustment;
    
    if(!xmlStrncmp(AGS_FILE_TRUE,
		   xmlGetProp(node, "inverted\0"),
		   9)){
      gtk_range_set_inverted(GTK_RANGE(child_widget),
			     TRUE);
    }
  }else if(AGS_IS_VINDICATOR(child_widget)){
    adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10.0, 1.0, 1.0, 10.0);
    g_object_set(child_widget,
		 "adjustment\0", adjustment,
		 NULL);
    gtk_widget_queue_draw(child_widget);
  }
  
  //TODO:JK: implement more types

  if(adjustment != NULL){
    gdouble upper, lower;
    gdouble step, page;
    gdouble value;

    step = (gdouble) g_ascii_strtod(xmlGetProp(node, "step\0"),
				    NULL);
    gtk_adjustment_set_step_increment(adjustment,
				      step);
    
    lower = (gdouble) g_ascii_strtod(xmlGetProp(node, "lower\0"),
				     NULL);
    gtk_adjustment_set_lower(adjustment,
			     lower);

    upper = (gdouble) g_ascii_strtod(xmlGetProp(node, "upper\0"),
				     NULL);
    gtk_adjustment_set_upper(adjustment,
			     upper);
    
    //    page = (gdouble) g_ascii_strtod(xmlGetProp(node, "page\0"),
    //				    NULL);
    //    gtk_adjustment_set_page_size(adjustment,
    //				 page);
    
    value = (gdouble) g_ascii_strtod(xmlGetProp(node, "value\0"),
				     NULL);
    gtk_adjustment_set_value(adjustment,
			     value);
  }
  
  /* flags */
  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  if((task_type = xmlGetProp(node, "task-type\0")) != NULL){
    gobject->task_type = g_type_from_name(task_type);
  }
  
  /* port */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect_after(G_OBJECT(file_lookup), "resolve\0",
			 G_CALLBACK(ags_file_read_line_member_resolve_port), gobject);
}

void
ags_file_read_line_member_resolve_port(AgsFileLookup *file_lookup,
				       AgsLineMember *line_member)
{
  GtkWidget *child_widget;
  AgsFileIdRef *id_ref;
  gchar *xpath;

  /* play port */
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "port\0");

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(id_ref == NULL){
    g_warning("couldn't find port\0");
  }else{
    g_object_set(G_OBJECT(line_member),
		 "port\0", (AgsPort *) id_ref->ref,
		 NULL);
  }

  /* recall port */
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "recall-port\0");

  if(xpath != NULL){
    id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

    if(id_ref == NULL){
      g_warning("couldn't find port\0");
    }else{
      g_object_set(G_OBJECT(line_member),
		   "recall-port\0", (AgsPort *) id_ref->ref,
		   NULL);
    }
  }
}

xmlNode*
ags_file_write_line_member(AgsFile *file, xmlNode *parent, AgsLineMember *line_member)
{
  AgsFileLookup *file_lookup;
  GtkWidget *child_widget;

  GtkAdjustment *adjustment;
  xmlNode *node;

  gchar *id;
  gchar *label;

  guint control_width, control_height;
  
  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-line-member\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", line_member,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", line_member->flags));

  xmlNewProp(node,
	     "widget-type\0",
	     g_strdup_printf("%s\0", g_type_name(line_member->widget_type)));

  child_widget = gtk_bin_get_child(GTK_BIN(line_member));
  
  label = NULL;
  g_object_get(G_OBJECT(line_member),
	       "widget-label\0", &label,
	       NULL);

  if(label != NULL){
    xmlNewProp(node,
	       "label\0",
	       g_strdup_printf("%s\0", label));
  }

  gtk_widget_get_size_request(child_widget,
			      &control_width, &control_height);

  xmlNewProp(node,
	     "width\0",
	     g_strdup_printf("%d\0", control_width));
  
  xmlNewProp(node,
	     "height\0",
	     g_strdup_printf("%d\0", control_height));
  
  if(line_member->task_type != G_TYPE_NONE){
    xmlNewProp(node,
	       AGS_FILE_TYPE_PROP,
	       g_strdup_printf("%s\0", g_type_name(line_member->task_type)));
  }

  /*  */
  if(GTK_IS_MISC(child_widget)){
    gfloat xalign, yalign;
    gint xpad, ypad;
    
    gtk_misc_get_alignment(GTK_MISC(child_widget),
			   &xalign, &yalign);
    xmlNewProp(node,
	       "xalign\0",
	       g_strdup_printf("%f\0", xalign));
    xmlNewProp(node,
	       "yalign\0",
	       g_strdup_printf("%f\0", yalign));
    
    gtk_misc_get_padding(GTK_MISC(child_widget),
			 &xpad, &ypad);
    xmlNewProp(node,
	       "xpad\0",
	       g_strdup_printf("%d\0", xpad));
    xmlNewProp(node,
	       "ypad\0",
	       g_strdup_printf("%d\0", ypad));
    
  }

  /*  */
  adjustment = NULL;

  /*  */
  if(GTK_IS_TOGGLE_BUTTON(child_widget)){
    xmlNewProp(node,
	       "value\0",
	       g_strdup_printf("%s\0", ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(child_widget))) ?
					AGS_FILE_TRUE:
					AGS_FILE_FALSE)));
  }else if(AGS_IS_DIAL(child_widget)){
    adjustment = AGS_DIAL(child_widget)->adjustment;

    //TODO:JK: improve dial widget work-around
    
  }else if(GTK_IS_SPIN_BUTTON(child_widget)){
    adjustment = GTK_SPIN_BUTTON(child_widget)->adjustment;
  }else if(GTK_IS_RANGE(child_widget)){
    adjustment = GTK_RANGE(child_widget)->adjustment;

    xmlNewProp(node,
	       "inverted\0",
	       g_strdup_printf("%s\0", (gtk_range_get_inverted(GTK_RANGE(child_widget)) ?
					AGS_FILE_TRUE :
					AGS_FILE_FALSE)));
  }else if(AGS_IS_INDICATOR(child_widget)){
    adjustment = AGS_INDICATOR(child_widget)->adjustment;
  }

  if(adjustment != NULL){
    gdouble upper, lower;
    gdouble page, step;
    gdouble value;

    xmlNewProp(node,
	       "upper\0",
	       g_strdup_printf("%.8f\0", adjustment->upper));
    xmlNewProp(node,
	       "lower\0",
	       g_strdup_printf("%.8f\0", adjustment->lower));
    
    xmlNewProp(node,
	       "page\0",
	       g_strdup_printf("%.8f\0", adjustment->page_size));
    xmlNewProp(node,
	       "step\0",
	       g_strdup_printf("%.8f\0", adjustment->step_increment));
    
    xmlNewProp(node,
	       "value\0",
	       g_strdup_printf("%.8f\0", adjustment->value));
  }

  /* port */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", line_member,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect_after(G_OBJECT(file_lookup), "resolve\0",
			 G_CALLBACK(ags_file_write_line_member_resolve_port), line_member);

  xmlAddChild(parent,
	      node);
}

void
ags_file_write_line_member_resolve_port(AgsFileLookup *file_lookup,
					AgsLineMember *line_member)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  /* play port */
  if(line_member->port != NULL){
    id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, line_member->port);

    if(id_ref != NULL){
      id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

      xmlNewProp(file_lookup->node,
		 "port\0",
		 g_strdup_printf("xpath=//*[@id='%s']\0", id));
    }
  }
  
  /* recall port */
  if(line_member->recall_port != NULL){
    id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, line_member->recall_port);

    if(id_ref != NULL){
      id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

      xmlNewProp(file_lookup->node,
		 "recall-port\0",
		 g_strdup_printf("xpath=//*[@id='%s']\0", id));
    }
  }
}

void
ags_file_read_line_member_list(AgsFile *file, xmlNode *node, GList **line_member)
{
  AgsLineMember *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-line-member\0",
		     16)){
	current = NULL;
	ags_file_read_line_member(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *line_member = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_line_member_list(AgsFile *file, xmlNode *parent, GList *line_member)
{
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-line-member-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = line_member;

  while(list != NULL){
    if(AGS_IS_LINE_MEMBER(list->data)){
      ags_file_write_line_member(file, node, AGS_LINE_MEMBER(list->data));
    }

    list = list->next;
  }

  return(node);
}

void
ags_file_read_effect_bridge(AgsFile *file, xmlNode *node, AgsEffectBridge **effect_bridge)
{
  AgsEffectBridge *gobject;

  AgsFileLookup *file_lookup;

  GType effect_bridge_type;

  xmlNode *child;
  
  if(*effect_bridge == NULL){
    effect_bridge_type = g_type_from_name(xmlGetProp(node,
						     AGS_FILE_TYPE_PROP));

    gobject = (AgsEffectBridge *) g_object_new(effect_bridge_type,
					       NULL);
    *effect_bridge = gobject;
  }else{
    gobject = *effect_bridge;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  gobject->flags |= AGS_EFFECT_BRIDGE_PREMAPPED_RECALL;
  gobject->flags &= (~AGS_EFFECT_BRIDGE_CONNECTED);

  gobject->name = g_strdup(xmlGetProp(node,
				      AGS_FILE_NAME_PROP));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_effect_bridge_resolve_audio), gobject);

  /* bulk */
  gobject->bulk_output_type = g_type_from_name(xmlGetProp(node,
							  "bulk-output-type\0"));
  
  gobject->bulk_input_type = g_type_from_name(xmlGetProp(node,
							 "bulk-input-type\0"));

  /* pad and line */
  gobject->output_pad_type = g_type_from_name(xmlGetProp(node,
							 "output-pad-type\0"));
  gobject->output_line_type = g_type_from_name(xmlGetProp(node,
							  "output-line-type\0"));
  
  gobject->input_pad_type = g_type_from_name(xmlGetProp(node,
							"input-pad-type\0"));
  gobject->input_line_type = g_type_from_name(xmlGetProp(node,
							 "input-line-type\0"));
  
  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-effect-bulk\0",
			   16)){
	if(!xmlStrncmp(xmlGetProp(child,
				  AGS_FILE_SCOPE_PROP),
		       "output\0",
		       6)){
	  ags_file_read_effect_bulk(file,
				    child,
				    (AgsEffectBulk **) &(gobject->bulk_output));
	}else{
	  ags_file_read_effect_bulk(file,
				    child,
				    (AgsEffectBulk **) &(gobject->bulk_input));
	}
      }else if(!xmlStrncmp(child->name,
			   "ags-effect-pad-list\0",
			   20)){
	GList *effect_pad, *list;

	effect_pad = NULL;

	ags_file_read_effect_pad_list(file,
				      child,
				      &effect_pad);
	
	if(!xmlStrncmp(xmlGetProp(child,
				  AGS_FILE_SCOPE_PROP),
		       "output\0",
		       7)){
	  if(!GTK_IS_BOX(gobject->output)){
	    ags_container_add_all((GtkContainer *) gobject->output,
				  effect_pad);
	  }else{
	    list = effect_pad;

	    while(list != NULL){
	      gtk_box_pack_start(GTK_BOX(gobject->output),
				 GTK_WIDGET(list->data),
				 FALSE, FALSE,
				 0);

	      list = list->next;
	    }
	  }
	}else{
	  if(!GTK_IS_BOX(gobject->input)){
	    ags_container_add_all((GtkContainer *) gobject->input,
				  effect_pad);
	  }else{
	    list = effect_pad;

	    while(list != NULL){
	      gtk_box_pack_start(GTK_BOX(gobject->input),
				 GTK_WIDGET(list->data),
				 FALSE, FALSE,
				 0);

	      list = list->next;
	    }
	  }
	}

	g_list_free(effect_pad);
      }
    }
    
    child = child->next;
  }
}

xmlNode*
ags_file_write_effect_bridge(AgsFile *file, xmlNode *parent, AgsEffectBridge *effect_bridge)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-bridge\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", effect_bridge,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", (effect_bridge->flags)));
  
  xmlNewProp(node,
	     AGS_FILE_NAME_PROP,
	     effect_bridge->name);

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(effect_bridge));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     effect_bridge->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     effect_bridge->build_id);

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", effect_bridge,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_write_effect_bridge_resolve_audio), effect_bridge);

  /* bulk */
  xmlNewProp(node,
	     "bulk-output-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->bulk_output_type)));

  xmlNewProp(node,
	     "bulk-input-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->bulk_input_type)));

  /* pad and list */
  xmlNewProp(node,
	     "output-pad-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->output_pad_type)));

  xmlNewProp(node,
	     "output-line-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->output_line_type)));

  xmlNewProp(node,
	     "input-pad-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->input_pad_type)));

  xmlNewProp(node,
	     "input-line-type\0",
	     g_strdup_printf("%s\0", g_type_name(effect_bridge->input_line_type)));

  /* add to parent */
  xmlAddChild(parent,
	      node);  

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(effect_bridge));

  if(effect_bridge->bulk_output != NULL){
    child = ags_file_write_effect_bulk(file,
				       node,
				       (AgsEffectBulk *) effect_bridge->bulk_output);
    xmlNewProp(child,
	       AGS_FILE_SCOPE_PROP,
	       "output\0");
  }

  if(effect_bridge->output != NULL){
    GList *list;

    list = gtk_container_get_children((GtkContainer *) effect_bridge->output);
    child = ags_file_write_effect_pad_list(file,
					   node,
					   list);
    g_list_free(list);
  }

  if(effect_bridge->bulk_input != NULL){
    child = ags_file_write_effect_bulk(file,
				       node,
				       (AgsEffectBulk *) effect_bridge->bulk_input);
  }
  
  if(effect_bridge->input != NULL){
    GList *list;

    list = gtk_container_get_children((GtkContainer *) effect_bridge->input);
    child = ags_file_write_effect_pad_list(file,
					   node,
					   list);
    g_list_free(list);

    xmlNewProp(child,
	       AGS_FILE_SCOPE_PROP,
	       "input\0");
  }
  
  return(node);
}

void
ags_file_write_effect_bridge_resolve_audio(AgsFileLookup *file_lookup,
					   AgsEffectBridge *effect_bridge)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, AGS_AUDIO(effect_bridge->audio));

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "audio\0",
	     g_strdup_printf("xpath=//ags-audio[@id='%s']\0", id));
}

void
ags_file_read_effect_bridge_resolve_audio(AgsFileLookup *file_lookup,
					  AgsEffectBridge *effect_bridge)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  gchar *str;
  
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "audio\0");
  g_message("xpath = %s\0", xpath);
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);
  
  g_object_set(G_OBJECT(effect_bridge),
	       "audio\0", (AgsAudio *) id_ref->ref,
	       NULL);

  g_signal_connect_after(G_OBJECT(effect_bridge->audio), "set_audio_channels\0",
			 G_CALLBACK(ags_effect_bridge_set_audio_channels_callback), effect_bridge);

  g_signal_connect_after(G_OBJECT(effect_bridge->audio), "set_pads\0",
			 G_CALLBACK(ags_effect_bridge_set_pads_callback), effect_bridge);

  str = xmlGetProp(file_lookup->node,
		   AGS_FILE_NAME_PROP);

  if(str != NULL){
    effect_bridge->name = g_strdup(str);
  }
}

void
ags_file_read_effect_bridge_list(AgsFile *file, xmlNode *node, GList **effect_bridge)
{
  AgsEffectBridge *current;

  GList *list;

  xmlNode *child;

  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-effect-bridge\0",
		     12)){
	current = NULL;
	ags_file_read_effect_bridge(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_bridge = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_effect_bridge_list(AgsFile *file, xmlNode *parent, GList *effect_bridge)
{
  AgsEffectBridge *current;
  
  xmlNode *node;
  
  GList *list;
  
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-bridge-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = effect_bridge;

  while(list != NULL){
    ags_file_write_effect_bridge(file, node, AGS_EFFECT_BRIDGE(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_effect_bulk(AgsFile *file, xmlNode *node, AgsEffectBulk **effect_bulk)
{
  AgsEffectBulk *gobject;

  AgsFileLookup *file_lookup;

  GType effect_bulk_type;
  
  xmlNode *child;

  if(*effect_bulk == NULL){
    effect_bulk_type = g_type_from_name(xmlGetProp(node,
						   AGS_FILE_TYPE_PROP));
    
    gobject = (AgsEffectBulk *) g_object_new(effect_bulk_type,
					       NULL);
    *effect_bulk = gobject;
  }else{
    gobject = *effect_bulk;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  gobject->flags |= AGS_EFFECT_BULK_PREMAPPED_RECALL;
  gobject->flags &= (~AGS_EFFECT_BULK_CONNECTED);

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));
  
  /* scope */
  gobject->channel_type = g_type_from_name(xmlGetProp(node,
						      "channel-type"));
  
  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_effect_bulk_resolve_audio), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-bulk-member-list\0",
			   21)){
	xmlNode *bulk_member_node;
	
	GList *bulk_member, *bulk_member_start;

	bulk_member = NULL;

	ags_file_read_bulk_member_list(file,
				       child,
				       &bulk_member);
	bulk_member_start = bulk_member;
	
	/* add bulk member to effect bulk */
	bulk_member_node = child->children;

	while(bulk_member_node != NULL){
	  if(bulk_member_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(child->name,
			   "ags-bulk-member\0",
			   15)){
	      guint x0, y0;
	      guint x1, y1;

	      /* pack */
	      x0 = g_ascii_strtoull(xmlGetProp(bulk_member_node,
					       "left-attach\0"),
				    NULL,
				    10);

	      y0 = g_ascii_strtoull(xmlGetProp(bulk_member_node,
					       "top-attach\0"),
				    NULL,
				    10);

	      x1 = g_ascii_strtoull(xmlGetProp(bulk_member_node,
					       "right-attach\0"),
				    NULL,
				    10);

	      y1 = g_ascii_strtoull(xmlGetProp(bulk_member_node,
					       "bottom-attach\0"),
				    NULL,
				    10);

	      gtk_table_attach(gobject->table,
			       GTK_WIDGET(bulk_member->data),
			       x0, x1,
			       y0, y1,
			       GTK_FILL, GTK_FILL,
			       0, 0);

	      /* iterate */
	      bulk_member = bulk_member->next;
	    }
	  }
	  
	  bulk_member_node = bulk_member_node->next;
	}

	g_list_free(bulk_member_start);
      }
    }
    
    child = child->next;
  }
}

xmlNode*
ags_file_write_effect_bulk(AgsFile *file, xmlNode *parent, AgsEffectBulk *effect_bulk)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-bulk\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", effect_bulk,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", (effect_bulk->flags)));
  
  xmlNewProp(node,
	     AGS_FILE_NAME_PROP,
	     effect_bulk->name);

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     effect_bulk->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     effect_bulk->build_id);

  /* scope */
  if(g_type_is_a(effect_bulk->channel_type,
		 AGS_TYPE_OUTPUT)){
    xmlNewProp(node,
	       AGS_FILE_SCOPE_PROP,
	       "output\0");
  }else{
    xmlNewProp(node,
	       AGS_FILE_SCOPE_PROP,
	       "input\0");
  }

  xmlNewProp(node,
	     "channel-type\0",
	     g_type_name(effect_bulk->channel_type));
  
  xmlAddChild(parent,
	      node);  

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", effect_bulk,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_write_effect_bulk_resolve_audio), effect_bulk);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(effect_bulk));

  if(effect_bulk->table != NULL){
    GList *list;

    list = gtk_container_get_children((GtkContainer *) effect_bulk->table);
    child = ags_file_write_bulk_member_list(file,
					    node,
					    list);
    g_list_free(list);
  }
  
  return(node);
}

void
ags_file_read_effect_bulk_list(AgsFile *file, xmlNode *node, GList **effect_bulk)
{
  AgsEffectBulk *current;

  GList *list;

  xmlNode *child;

  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-effect-bulk\0",
		     12)){
	current = NULL;
	ags_file_read_effect_bulk(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_bulk = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_effect_bulk_list(AgsFile *file, xmlNode *parent, GList *effect_bulk)
{
  AgsEffectBulk *current;
  
  xmlNode *node;
  
  GList *list;
  
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-bulk-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = effect_bulk;

  while(list != NULL){
    ags_file_write_effect_bulk(file, node, AGS_EFFECT_BULK(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_effect_bulk_resolve_audio(AgsFileLookup *file_lookup,
					AgsEffectBulk *effect_bulk)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  gchar *str;
  
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "audio\0");
  g_message("xpath = %s\0", xpath);
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);
  
  g_object_set(G_OBJECT(effect_bulk),
	       "audio\0", (AgsAudio *) id_ref->ref,
	       NULL);

  g_signal_connect_after(G_OBJECT(effect_bulk->audio), "set_audio_channels\0",
			 G_CALLBACK(ags_effect_bulk_set_audio_channels_callback), effect_bulk);

  g_signal_connect_after(G_OBJECT(effect_bulk->audio), "set_pads\0",
			 G_CALLBACK(ags_effect_bulk_set_pads_callback), effect_bulk);

  str = xmlGetProp(file_lookup->node,
		   AGS_FILE_NAME_PROP);

  if(str != NULL){
    effect_bulk->name = g_strdup(str);
  }
}

void
ags_file_write_effect_bulk_resolve_audio(AgsFileLookup *file_lookup,
					 AgsEffectBulk *effect_bulk)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, AGS_AUDIO(effect_bulk->audio));

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "audio\0",
	     g_strdup_printf("xpath=//ags-audio[@id='%s']\0", id));
}

void
ags_file_read_bulk_member(AgsFile *file, xmlNode *node, AgsBulkMember **bulk_member)
{
  AgsBulkMember *gobject;
  GtkAdjustment *adjustment;
  GtkWidget *child_widget;

  xmlNode *child;

  xmlChar *prop, *content;
  gchar *widget_type;
  gchar *label;
  gchar *task_type;
  guint width, height;

  if(*bulk_member == NULL){
    gobject = g_object_new(AGS_TYPE_BULK_MEMBER,
			   NULL);
    *bulk_member = gobject;
  }else{
    gobject = *bulk_member;

    if(!AGS_IS_BULK_MEMBER(gobject)){
      return;
    }
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  widget_type = (gchar *) xmlGetProp(node, "widget-type\0");
  g_object_set(gobject,
	       "widget-type\0", g_type_from_name(widget_type),
	       NULL);
  child_widget = (GtkWidget *) gtk_bin_get_child(GTK_BIN(gobject));

  /* label */
  label = (gchar *) xmlGetProp(node, "label\0");

  if(label != NULL){
    g_object_set(G_OBJECT(gobject),
		 "widget-label\0", label,
		 NULL);
  }

  /* filename */
  g_object_set(G_OBJECT(gobject),
	       "filename\0", (gchar *) xmlGetProp(node, "filename\0"),
	       NULL);

  /* effect */
  g_object_set(G_OBJECT(gobject),
	       "effect\0", (gchar *) xmlGetProp(node, "effect\0"),
	       NULL);

  /* plugin-name */
  g_object_set(G_OBJECT(gobject),
	       "plugin-name\0", (gchar *) xmlGetProp(node, "plugin-name\0"),
	       NULL);

  /* specifier */
  g_object_set(G_OBJECT(gobject),
	       "specifier\0", (gchar *) xmlGetProp(node, "specifier\0"),
	       NULL);

  /* size */
  width = (guint) g_ascii_strtoull(xmlGetProp(node, "width\0"),
				   NULL,
				   10);

  height = (guint) g_ascii_strtoull(xmlGetProp(node, "height\0"),
				    NULL,
				    10);

  gtk_widget_set_size_request(child_widget,
			      width, height);

  /* check misc */
  if(GTK_IS_MISC(child_widget)){
    gfloat xalign, yalign;
    guint xpad, ypad;

    xalign = (gfloat) g_ascii_strtod(xmlGetProp(node, "xalign\0"),
				     NULL);

    yalign = (gfloat) g_ascii_strtod(xmlGetProp(node, "yalign\0"),
				     NULL);

    xpad = (guint) g_ascii_strtoull(xmlGetProp(node, "xpad\0"),
				    NULL,
				    10);
    
    ypad = (guint) g_ascii_strtoull(xmlGetProp(node, "ypad\0"),
				    NULL,
				    10);
  }

  /* check adjustment and toggle types */
  adjustment = NULL;

  if(GTK_IS_TOGGLE_BUTTON(child_widget)){
    if(!xmlStrncmp(AGS_FILE_TRUE,
		   xmlGetProp(node, "value\0"),
		   5)){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child_widget),
				   TRUE);
    }
  }else if(AGS_IS_DIAL(child_widget)){
    AgsDial *dial;
    
    dial = (AgsDial *) child_widget;
    adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
    g_object_set(child_widget,
		 "adjustment\0", adjustment,
		 NULL);
    gtk_widget_set_size_request((GtkWidget *) dial,
				2 * dial->radius + 2 * dial->outline_strength + dial->button_width + 1,
				2 * dial->radius + 2 * dial->outline_strength + 1);
  }else if(GTK_IS_SPIN_BUTTON(child_widget)){
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(child_widget),
			       3);
    adjustment = GTK_SPIN_BUTTON(child_widget)->adjustment;
  }else if(GTK_IS_RANGE(child_widget)){
    gtk_range_set_round_digits(GTK_RANGE(child_widget),
			       3);
    adjustment = GTK_RANGE(child_widget)->adjustment;
    
    if(!xmlStrncmp(AGS_FILE_TRUE,
		   xmlGetProp(node, "inverted\0"),
		   9)){
      gtk_range_set_inverted(GTK_RANGE(child_widget),
			     TRUE);
    }
  }else if(AGS_IS_VINDICATOR(child_widget)){
    adjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10.0, 1.0, 1.0, 10.0);
    g_object_set(child_widget,
		 "adjustment\0", adjustment,
		 NULL);
    gtk_widget_queue_draw(child_widget);
  }
  
  //TODO:JK: implement more types

  if(adjustment != NULL){
    gdouble upper, lower;
    gdouble step, page;
    gdouble value;

    step = (gdouble) g_ascii_strtod(xmlGetProp(node, "step\0"),
				    NULL);
    gtk_adjustment_set_step_increment(adjustment,
				      step);
    
    lower = (gdouble) g_ascii_strtod(xmlGetProp(node, "lower\0"),
				     NULL);
    gtk_adjustment_set_lower(adjustment,
			     lower);

    upper = (gdouble) g_ascii_strtod(xmlGetProp(node, "upper\0"),
				     NULL);
    gtk_adjustment_set_upper(adjustment,
			     upper);
    
    //    page = (gdouble) g_ascii_strtod(xmlGetProp(node, "page\0"),
    //				    NULL);
    //    gtk_adjustment_set_page_size(adjustment,
    //				 page);
    
    value = (gdouble) g_ascii_strtod(xmlGetProp(node, "value\0"),
				     NULL);
    gtk_adjustment_set_value(adjustment,
			     value);
  }
  
  /* flags */
  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  if((task_type = xmlGetProp(node, "task-type\0")) != NULL){
    gobject->task_type = g_type_from_name(task_type);
  }
    
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    "ags-parameter\0")){
	AgsFileLookup *file_lookup;
	
	GParameter *parameter;

	gint n_params;
	guint i;

	parameter = NULL;
	n_params = 0;
	
	ags_file_util_read_parameter(file,
				     child, NULL,
				     &parameter, &n_params, NULL);

	for(i = 0; i < n_params; i++){
	  file_lookup = (AgsFileLookup *) ags_file_lookup_find_by_reference(file->lookup,
									    &(parameter[i].value));
	  g_signal_connect_after(G_OBJECT(file_lookup), "resolve\0",
				 G_CALLBACK(ags_file_read_bulk_member_resolve_port), gobject);
	}
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_bulk_member(AgsFile *file, xmlNode *parent, AgsBulkMember *bulk_member)
{
  GtkWidget *child_widget;

  GtkAdjustment *adjustment;

  xmlNode *node;
  GParameter *parameter;
  GList *list;
  
  gchar *id;
  gchar *label;

  guint width, height;
  guint left_attach, right_attach;
  guint top_attach, bottom_attach;
  guint i, i_stop;
  
  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-bulk-member\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", bulk_member,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", bulk_member->flags));

  xmlNewProp(node,
	     "widget-type\0",
	     g_strdup_printf("%s\0", g_type_name(bulk_member->widget_type)));

  child_widget = gtk_bin_get_child(GTK_BIN(bulk_member));
  
  label = NULL;
  g_object_get(G_OBJECT(child_widget),
	       "label\0", &label,
	       NULL);

  if(label != NULL){
    xmlNewProp(node,
	       "label\0",
	       g_strdup_printf("%s\0", label));
  }

  xmlNewProp(node,
	     "filename\0",
	     g_strdup_printf("%s\0", bulk_member->filename));

  xmlNewProp(node,
	     "effect\0",
	     g_strdup_printf("%s\0", bulk_member->effect));

  xmlNewProp(node,
	     "plugin-name\0",
	     g_strdup_printf("%s\0", bulk_member->plugin_name));

  xmlNewProp(node,
	     "specifier\0",
	     g_strdup_printf("%s\0", bulk_member->specifier));

  gtk_widget_get_size_request(child_widget,
			      &width, &height);
  
  xmlNewProp(node,
	     "width\0",
	     g_strdup_printf("%d\0", width));
  
  xmlNewProp(node,
	     "height\0",
	     g_strdup_printf("%d\0", height));

  gtk_container_child_get((GtkContainer *) GTK_WIDGET(bulk_member)->parent,
			  (GtkWidget *) bulk_member,
			  "left-attach\0", &left_attach,
			  "right-attach\0", &right_attach,
			  "top-attach\0", &top_attach,
			  "bottom-attach\0", &bottom_attach,
			  NULL);

  xmlNewProp(node,
	     "left-attach\0",
	     g_strdup_printf("%d\0", left_attach));

  xmlNewProp(node,
	     "right-attach\0",
	     g_strdup_printf("%d\0", right_attach));

  xmlNewProp(node,
	     "top-attach\0",
	     g_strdup_printf("%d\0", top_attach));

  xmlNewProp(node,
	     "bottom-attach\0",
	     g_strdup_printf("%d\0", bottom_attach));
  
  if(bulk_member->task_type != G_TYPE_NONE){
    xmlNewProp(node,
	       AGS_FILE_TYPE_PROP,
	       g_strdup_printf("%s\0", g_type_name(bulk_member->task_type)));
  }

  /*  */
  if(GTK_IS_MISC(child_widget)){
    gfloat xalign, yalign;
    gint xpad, ypad;
    
    gtk_misc_get_alignment(GTK_MISC(child_widget),
			   &xalign, &yalign);
    xmlNewProp(node,
	       "xalign\0",
	       g_strdup_printf("%f\0", xalign));
    xmlNewProp(node,
	       "yalign\0",
	       g_strdup_printf("%f\0", yalign));
    
    gtk_misc_get_padding(GTK_MISC(child_widget),
			 &xpad, &ypad);
    xmlNewProp(node,
	       "xpad\0",
	       g_strdup_printf("%d\0", xpad));
    xmlNewProp(node,
	       "ypad\0",
	       g_strdup_printf("%d\0", ypad));
    
  }

  /*  */
  adjustment = NULL;

  /*  */
  if(GTK_IS_TOGGLE_BUTTON(child_widget)){
    xmlNewProp(node,
	       "value\0",
	       g_strdup_printf("%s\0", ((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(child_widget))) ?
					AGS_FILE_TRUE:
					AGS_FILE_FALSE)));
  }else if(AGS_IS_DIAL(child_widget)){
    adjustment = AGS_DIAL(child_widget)->adjustment;

    //TODO:JK: improve dial widget work-around
    
  }else if(GTK_IS_SPIN_BUTTON(child_widget)){
    adjustment = GTK_SPIN_BUTTON(child_widget)->adjustment;
  }else if(GTK_IS_RANGE(child_widget)){
    adjustment = GTK_RANGE(child_widget)->adjustment;

    xmlNewProp(node,
	       "inverted\0",
	       g_strdup_printf("%s\0", (gtk_range_get_inverted(GTK_RANGE(child_widget)) ?
					AGS_FILE_TRUE :
					AGS_FILE_FALSE)));
  }else if(AGS_IS_INDICATOR(child_widget)){
    adjustment = AGS_INDICATOR(child_widget)->adjustment;
  }

  if(adjustment != NULL){
    gdouble upper, lower;
    gdouble page, step;
    gdouble value;

    xmlNewProp(node,
	       "upper\0",
	       g_strdup_printf("%.8f\0", adjustment->upper));
    xmlNewProp(node,
	       "lower\0",
	       g_strdup_printf("%.8f\0", adjustment->lower));
    
    xmlNewProp(node,
	       "page\0",
	       g_strdup_printf("%.8f\0", adjustment->page_size));
    xmlNewProp(node,
	       "step\0",
	       g_strdup_printf("%.8f\0", adjustment->step_increment));
    
    xmlNewProp(node,
	       "value\0",
	       g_strdup_printf("%.8f\0", adjustment->value));
  }

  /* bulk port */
  list = bulk_member->bulk_port;
  i_stop = g_list_length(list);

  parameter = (GParameter *) g_new(GParameter,
				   i_stop);
  
  for(i = 0; i < i_stop; i++){
    parameter[i].name = "bulk-port\0";
    
    memset(&(parameter[i].value), 0, sizeof(GValue));
    g_value_init(&(parameter[i].value),
		 G_TYPE_OBJECT);
    g_value_set_object(&(parameter[i].value),
		       list->data);

    list = list->next;
  }

  ags_file_util_write_parameter(file,
				node,
				ags_id_generator_create_uuid(),
				parameter, i_stop);
  
  /* bulk port */
  list = bulk_member->recall_bulk_port;
  i_stop = g_list_length(list);

  parameter = (GParameter *) g_new(GParameter,
				   i_stop);
  
  for(i = 0; i < i_stop; i++){
    parameter[i].name = "recall-bulk-port\0";
    
    memset(&(parameter[i].value), 0, sizeof(GValue));
    g_value_init(&(parameter[i].value),
		 G_TYPE_OBJECT);
    g_value_set_object(&(parameter[i].value),
		       list->data);

    list = list->next;
  }
  
  ags_file_util_write_parameter(file,
				node,
				ags_id_generator_create_uuid(),
				parameter, i_stop);
  
  xmlAddChild(parent,
	      node);
}

void
ags_file_read_bulk_member_resolve_port(AgsFileLookup *file_lookup,
				       AgsBulkMember *bulk_member)
{
  AgsFileIdRef *id_ref;

  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;

  gchar *xpath;
  gchar *str;
  
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "link\0");
  g_message("ags_file_read_bulk_member_resolve_port - xpath = %s\0", xpath);
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(!g_ascii_strncasecmp(xmlGetProp(file_lookup->node->parent,
				     AGS_FILE_NAME_PROP),
			  "bulk-port\0",
			  10)){
    g_object_set(G_OBJECT(bulk_member),
		 "bulk-port\0", (AgsAudio *) id_ref->ref,
		 NULL);
  }else{
    g_object_set(G_OBJECT(bulk_member),
		 "recall-bulk-port\0", (AgsAudio *) id_ref->ref,
		 NULL);
  }
}

void
ags_file_read_bulk_member_list(AgsFile *file, xmlNode *node, GList **bulk_member)
{
  AgsBulkMember *current;

  GList *list;

  xmlNode *child;

  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-bulk-member\0",
		     12)){
	current = NULL;
	ags_file_read_bulk_member(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *bulk_member = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_bulk_member_list(AgsFile *file, xmlNode *parent, GList *bulk_member)
{
  AgsBulkMember *current;
  
  xmlNode *node;
  
  GList *list;
  
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-bulk-member-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = bulk_member;

  while(list != NULL){
    ags_file_write_bulk_member(file, node, AGS_BULK_MEMBER(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_effect_pad(AgsFile *file, xmlNode *node, AgsEffectPad **effect_pad)
{
  AgsEffectPad *gobject;
  
  AgsFileLookup *file_lookup;

  xmlNode *child;

  GType effect_pad_type;
  
  if(*effect_pad == NULL){
    effect_pad_type = g_type_from_name(xmlGetProp(node,
						  AGS_FILE_TYPE_PROP));
    
    gobject = (AgsEffectPad *) g_object_new(effect_pad_type,
					    NULL);
    *effect_pad = gobject;
  }else{
    gobject = *effect_pad;
  }

  if(gobject == NULL){
    return;
  }
  
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);
  gobject->flags &= (~AGS_EFFECT_PAD_CONNECTED);

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  /* channel */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_effect_pad_resolve_channel), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(ags_plugin_get_xml_type(AGS_PLUGIN(gobject)) != NULL &&
	 !xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-effect-line-list\0",
			   20)){
	xmlNode *line_node;
	GList *start, *list;

	list = NULL;

	ags_file_read_effect_line_list(file,
				       child,
				       &list);
	start = list;

	/* add line to effect_pad */
	line_node = child->children;

	while(line_node != NULL && list != NULL){
	  if(line_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(line_node->name,
			   "ags-effect-line\0",
			   15)){
	      guint x0, x1;
	      guint y0, y1;

	      g_object_set(G_OBJECT(list->data),
			   "effect-pad\0", gobject,
			   NULL);

	      x0 = g_ascii_strtoull(xmlGetProp(line_node,
					       "left-attach\0"),
				    NULL,
				    10);

	      y0 = g_ascii_strtoull(xmlGetProp(line_node,
					       "top-attach\0"),
				    NULL,
				    10);

	      x1 = g_ascii_strtoull(xmlGetProp(line_node,
					       "right-attach\0"),
				    NULL,
				    10);

	      y1 = g_ascii_strtoull(xmlGetProp(line_node,
					       "bottom-attach\0"),
				    NULL,
				    10);

	      gtk_table_attach(gobject->table,
			       GTK_WIDGET(list->data),
			       x0, x1,
			       y0, y1,
			       GTK_FILL, GTK_FILL,
			       0, 0);	      
	      

	      list = list->next;
	    }
	  }

	  line_node = line_node->next;
	}

	g_list_free(start);
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_effect_pad(AgsFile *file, xmlNode *parent, AgsEffectPad *effect_pad)
{
  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *effect_line_node;

  GList *effect_line, *effect_line_start;

  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-effect-pad\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", effect_pad,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(effect_pad));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     effect_pad->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     effect_pad->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_EFFECT_PAD_CONNECTED) & (effect_pad->flags))));

  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(effect_pad));

  effect_line_start = 
    effect_line = gtk_container_get_children(GTK_CONTAINER(effect_pad->table));

  child = ags_file_write_effect_line_list(file,
					  node,
					  effect_line);

  effect_line_node = child->children;

  while(effect_line != NULL){
    guint x0, x1;
    guint y0, y1;

    gtk_container_child_get((GtkContainer *) effect_pad->table,
			    (GtkWidget *) effect_line->data,
			    "left-attach", &x0,
			    "right-attach", &x1,
			    "top-attach", &y0,
			    "bottom-attach", &y1,
			    NULL);    

    xmlNewProp(effect_line_node,
	       "left-attach\0",
	       g_strdup_printf("%d\0", x0));

    xmlNewProp(effect_line_node,
	       "top-attach\0",
	       g_strdup_printf("%d\0", y0));

    xmlNewProp(effect_line_node,
	       "right-attach\0",
	       g_strdup_printf("%d\0", x1));

    xmlNewProp(effect_line_node,
	       "bottom-attach\0",
	       g_strdup_printf("%d\0", y1));
    
    effect_line = effect_line->next;
    effect_line_node = effect_line_node->next;
  }

  g_list_free(effect_line_start);
}

void
ags_file_read_effect_pad_resolve_channel(AgsFileLookup *file_lookup,
					 AgsPad *pad)
{
  AgsFile *file;
  AgsEffectBridge *effect_bridge;
  AgsFileIdRef *id_ref;
  xmlNode *node, *audio_node, *channel_node;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  GList *list;
  xmlChar *xpath;
  guint position;
  guint nth, audio_channel;
  guint i, j;
  gboolean is_output;
  
  file = file_lookup->file;

  effect_bridge = (AgsEffectBridge *) gtk_widget_get_ancestor(GTK_WIDGET(pad),
							      AGS_TYPE_EFFECT_BRIDGE);

  if(effect_bridge->output != NULL){
    list = gtk_container_get_children((GtkContainer *) effect_bridge->output);

    if(list != NULL &&
       g_list_find(list,
		   pad) != NULL){
      is_output = TRUE;
    }else{
      is_output = FALSE;
    }
  }else{
    is_output = FALSE;
  }

  node = file_lookup->node;

  /* retrieve position */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent,
  //			 xpath_context);
  xpath_context->node = node->parent;

  xpath_object = xmlXPathEval("./ags-effect-pad\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }
  nth = j;

  /*  */
  position = nth * effect_bridge->audio->audio_channels;

  /*  */
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, effect_bridge->audio);
  audio_node = NULL;

  if(id_ref != NULL){
    audio_node = id_ref->node;
  }

  /*  */
  xpath = g_strdup_printf("(./ags-channel-list/ags-channel)/%s\0",
			  (is_output ? "ags-output\0": "ags-input\0"));

  xpath_context = xmlXPathNewContext(file->doc);
  xpath_context->node = audio_node;
  xpath_object = xmlXPathEval(xpath,
			      xpath_context);

  /*  */
  if(xpath_object != NULL && xpath_object->nodesetval != NULL){
    AgsFileIdRef *file_id_ref;
    xmlNode *channel_node;

    for(i = 0, j = 0; j < position && i < xpath_object->nodesetval->nodeMax; i++){
      if(xpath_object->nodesetval->nodeTab[i] != NULL && xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
	j++;
      }
    }

    channel_node = xpath_object->nodesetval->nodeTab[i];

    file_id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_node(file,
								channel_node);
    g_object_set(G_OBJECT(pad),
		 "channel\0", AGS_CHANNEL(file_id_ref->ref),
		 NULL);
  }else{
    g_message("no xpath match: %s\0",
	      xpath);
  }
}

void
ags_file_read_effect_pad_list(AgsFile *file, xmlNode *node, GList **effect_pad)
{
  AgsEffectPad *current;

  GList *list;

  xmlNode *child;

  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-effect-pad\0",
		     12)){
	current = NULL;
	ags_file_read_effect_pad(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_pad = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_effect_pad_list(AgsFile *file, xmlNode *parent, GList *effect_pad)
{
  AgsEffectPad *current;
  
  xmlNode *node;
  
  GList *list;
  
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-pad-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = effect_pad;

  while(list != NULL){
    ags_file_write_effect_pad(file, node, AGS_EFFECT_PAD(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_effect_line(AgsFile *file, xmlNode *node, AgsEffectLine **effect_line)
{
  AgsEffectLine *gobject;

  AgsFileLookup *file_lookup;

  xmlNode *child;

  GType effect_line_type;

  if(*effect_line == NULL){
    effect_line_type = g_type_from_name(xmlGetProp(node,
						   AGS_FILE_TYPE_PROP));
    
    gobject = (AgsEffectLine *) g_object_new(effect_line_type,
					     NULL);
    *effect_line = gobject;
  }else{
    gobject = *effect_line;
  }

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);
  gobject->flags |= AGS_EFFECT_LINE_PREMAPPED_RECALL;
  gobject->flags &= (~AGS_EFFECT_LINE_CONNECTED);

  /* channel */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_effect_line_resolve_channel), gobject);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrcmp(child->name,
		    ags_plugin_get_xml_type(AGS_PLUGIN(gobject)))){
	ags_plugin_read(file,
			child,
			AGS_PLUGIN(gobject));
      }else if(!xmlStrncmp(child->name,
			   "ags-line-member-list\0",
			   20)){
	xmlNode *line_member_node;
	GList *start, *list;

	list = NULL;

	ags_file_read_line_member_list(file,
				       child,
				       &list);
	start = list;
	
	/* remove default line members */
	gtk_widget_destroy(GTK_WIDGET(gobject->table));

	gobject->table = (GtkTable *) gtk_table_new(1, 1,
						    FALSE);
	gtk_table_set_row_spacings(gobject->table,
				   2);
	gtk_table_set_col_spacings(gobject->table,
				   2);
	gtk_box_pack_start(GTK_BOX(gobject),
			   GTK_WIDGET(gobject->table),
			   TRUE, TRUE,
			   0);

	/* add line member to line */
	line_member_node = child->children;

	while(line_member_node != NULL){
	  if(line_member_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(child->name,
			   "ags-line-member\0",
			   15)){
	      guint x0, x1;
	      guint y0, y1;

	      /* pack */
	      x0 = g_ascii_strtoull(xmlGetProp(line_member_node,
					       "left-attach\0"),
				    NULL,
				    10);

	      y0 = g_ascii_strtoull(xmlGetProp(line_member_node,
					       "top-attach\0"),
				    NULL,
				    10);

	      x1 = g_ascii_strtoull(xmlGetProp(line_member_node,
					       "right-attach\0"),
				    NULL,
				    10);

	      y1 = g_ascii_strtoull(xmlGetProp(line_member_node,
					       "bottom-attach\0"),
				    NULL,
				    10);

	      gtk_table_attach(gobject->table,
			       GTK_WIDGET(list->data),
			       x0, x1,
			       y0, y1,
			       GTK_FILL, GTK_FILL,
			       0, 0);	      
	      
	      /* iterate */
	      list = list->next;
	    }
	  }
	  
	  line_member_node = line_member_node->next;
	}

	g_list_free(start);
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_effect_line(AgsFile *file, xmlNode *parent, AgsEffectLine *effect_line)
{
  AgsFileLookup *file_lookup;

  xmlNode *node, *child;
  xmlNode *line_member_node;

  GList *line_child;
  GList *line_member, *line_member_start;

  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-effect-line\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", effect_line,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_TYPE_PROP,
	     G_OBJECT_TYPE_NAME(effect_line));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     effect_line->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     effect_line->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", ((~AGS_EFFECT_LINE_CONNECTED)&(effect_line->flags))));

  xmlAddChild(parent,
	      node);

  /* child elements */
  ags_plugin_write(file,
  		   node,
  		   AGS_PLUGIN(effect_line));

  line_member_start = 
    line_member = gtk_container_get_children(GTK_CONTAINER(effect_line->table));

  child = ags_file_write_line_member_list(file,
					  node,
					  line_member);

  line_member_node = child->children;

  while(line_member != NULL){
    if(AGS_IS_LINE_MEMBER(line_member->data)){
      GList *list;
      guint x0, x1;
      guint y0, y1;
      
      list = gtk_container_get_children((GtkContainer *) effect_line->table);
      line_child = g_list_find(list,
			       line_member->data)->data;
      g_list_free(list);

      gtk_container_child_get((GtkContainer *) effect_line->table,
			      (GtkWidget *) line_child->data,
			      "left-attach", &x0,
			      "right-attach", &x1,
			      "top-attach", &y0,
			      "bottom-attach", &y1,
			      NULL);
      
      xmlNewProp(line_member_node,
		 "left-attach\0",
		 g_strdup_printf("%d\0", x0));

      xmlNewProp(line_member_node,
		 "top-attach\0",
		 g_strdup_printf("%d\0", y0));

      xmlNewProp(line_member_node,
		 "right-attach\0",
		 g_strdup_printf("%d\0", x1));

      xmlNewProp(line_member_node,
		 "bottom-attach\0",
		 g_strdup_printf("%d\0", y1));

      line_member_node = line_member_node->next;
    }

    line_member = line_member->next;
  }

  g_list_free(line_member_start);
}

void
ags_file_read_effect_line_resolve_channel(AgsFileLookup *file_lookup,
					  AgsLine *line)
{
  AgsFile *file;
  AgsEffectBridge *effect_bridge;
  AgsFileIdRef *id_ref;
  xmlNode *pad_node, *node, *audio_node, *channel_node;
  xmlXPathContext *xpath_context;
  xmlXPathObject *xpath_object;
  GList *list;
  xmlChar *xpath;
  guint position;
  guint pad, audio_channel;
  guint i, j;
  gboolean is_output;

  file = file_lookup->file;

  effect_bridge = (AgsEffectBridge *) gtk_widget_get_ancestor(GTK_WIDGET(line),
							      AGS_TYPE_EFFECT_BRIDGE);

  if(effect_bridge->output != NULL){
    AgsPad *pad;

    pad = (AgsPad *) gtk_widget_get_ancestor((GtkWidget *) line,
					     AGS_TYPE_PAD);
    list = gtk_container_get_children((GtkContainer *) effect_bridge->output);

    if(list != NULL &&
       g_list_find(list,
		   pad) != NULL){
      is_output = TRUE;
    }else{
      is_output = FALSE;
    }
  }else{
    is_output = FALSE;
  }

  node = file_lookup->node;
  pad_node = node->parent->parent;

  /* retrieve position - pad */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent->parent->parent,
  //			 xpath_context);
  xpath_context->node = pad_node->parent;

  xpath_object = xmlXPathEval("./ags-effect-pad\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != pad_node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }
  pad = j;

  /* retrieve position - line */
  xpath_context = xmlXPathNewContext(file->doc);
  //  xmlXPathSetContextNode(node->parent,
  //			 xpath_context);
  xpath_context->node = node->parent;

  xpath_object = xmlXPathEval("./ags-effect-line\0",
			      xpath_context);

  for(i = 0, j = 0; xpath_object->nodesetval->nodeTab[i] != node && i < xpath_object->nodesetval->nodeMax; i++){
    if(xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
      j++;
    }
  }

  audio_channel = j;

  /*  */
  position = pad * effect_bridge->audio->audio_channels + (effect_bridge->audio->audio_channels - audio_channel - 1);

  /*  */
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, effect_bridge->audio);
  audio_node = NULL;

  if(id_ref != NULL){
    audio_node = id_ref->node;
  }

  /*  */
  xpath = g_strdup_printf("(./ags-channel-list/ags-channel)/%s\0",
			  ((is_output) ? "ags-output\0": "ags-input\0"));

  xpath_context = xmlXPathNewContext(file->doc);
  xpath_context->node = audio_node;
  xpath_object = xmlXPathEval(xpath,
			      xpath_context);

  /*  */
  if(xpath_object != NULL && xpath_object->nodesetval != NULL){
    AgsFileIdRef *file_id_ref;
    xmlNode *channel_node;

    for(i = 0, j = 0; j < position && i < xpath_object->nodesetval->nodeMax; i++){
      if(xpath_object->nodesetval->nodeTab[i] != NULL && xpath_object->nodesetval->nodeTab[i]->type == XML_ELEMENT_NODE){
	j++;
      }
    }

    channel_node = xpath_object->nodesetval->nodeTab[i];

    file_id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_node(file,
								channel_node);

    g_object_set(G_OBJECT(line),
		 "channel\0", AGS_CHANNEL(file_id_ref->ref),
		 NULL);
  }else{
    g_message("no xpath match: %s\0",
	      xpath);
  }
}

void
ags_file_read_effect_line_list(AgsFile *file, xmlNode *node, GList **effect_line)
{
  AgsEffectLine *current;

  GList *list;

  xmlNode *child;

  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-effect-line\0",
		     12)){
	current = NULL;
	ags_file_read_effect_line(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_line = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_effect_line_list(AgsFile *file, xmlNode *parent, GList *effect_line)
{
  AgsEffectLine *current;
  
  xmlNode *node;
  
  GList *list;
  
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-effect-line-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = effect_line;

  while(list != NULL){
    ags_file_write_effect_line(file, node, AGS_EFFECT_LINE(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_dialog(AgsFile *file, xmlNode *node, GtkDialog **dialog)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_dialog(AgsFile *file, xmlNode *parent, GtkDialog *dialog)
{
  //TODO:JK: implement me
}

void
ags_file_read_dialog_list(AgsFile *file, xmlNode *node, GList **dialog)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_dialog_list(AgsFile *file, xmlNode *parent, GList *dialog)
{
  //TODO:JK: implement me
}

void
ags_file_read_machine_editor(AgsFile *file, xmlNode *node, AgsMachineEditor **machine_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_machine_editor(AgsFile *file, xmlNode *parent, AgsMachineEditor *machine_editor)
{
  xmlNode *node;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-machine-editor\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", machine_editor,
				   NULL));

  //TODO:JK: implement me

  xmlAddChild(parent,
	      node);  
}

void
ags_file_read_machine_editor_list(AgsFile *file, xmlNode *node, GList **machine_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_machine_editor_list(AgsFile *file, xmlNode *parent, GList *machine_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_pad_editor(AgsFile *file, xmlNode *node, AgsPadEditor **pad_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_pad_editor(AgsFile *file, xmlNode *parent, AgsPadEditor *pad_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_line_editor(AgsFile *file, xmlNode *node, AgsLineEditor **line_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_line_editor(AgsFile *file, xmlNode *parent, AgsLineEditor *line_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_link_editor(AgsFile *file, xmlNode *node, AgsLinkEditor **link_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_link_editor(AgsFile *file, xmlNode *parent, AgsLinkEditor *link_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_line_member_editor(AgsFile *file, xmlNode *node, AgsLineMemberEditor **line_member_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_line_member_editor(AgsFile *file, xmlNode *parent, AgsLineMemberEditor *line_member_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_link_collection_editor(AgsFile *file, xmlNode *node, AgsLinkCollectionEditor **link_collection_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_link_collection_editor(AgsFile *file, xmlNode *parent, AgsLinkCollectionEditor *link_collection_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_resize_editor(AgsFile *file, xmlNode *node, AgsResizeEditor **resize_editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_resize_editor(AgsFile *file, xmlNode *parent, AgsResizeEditor *resize_editor)
{
  //TODO:JK: implement me
}

void
ags_file_read_editor(AgsFile *file, xmlNode *node, AgsEditor **editor)
{
  AgsEditor *gobject;
  AgsFileLaunch *file_launch;
  xmlNode *child;

  if(*editor == NULL){
    gobject = (AgsEditor *) g_object_new(AGS_TYPE_EDITOR,
					 NULL);
    *editor = gobject;
  }else{
    gobject = *editor;
  }
  
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->version = g_strdup(xmlGetProp(node,
					 AGS_FILE_VERSION_PROP));

  gobject->build_id = g_strdup(xmlGetProp(node,
					  AGS_FILE_BUILD_ID_PROP));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-toolbar\0",
		     12)){
	ags_file_read_toolbar(file,
			      child,
			      &(gobject->toolbar));
      }else if(!xmlStrncmp(child->name,
			   "ags-machine-selector\0",
			   11)){
	ags_file_read_machine_selector(file,
				       child,
				       &(gobject->machine_selector));
      }else if(!xmlStrncmp(child->name,
			   "ags-notebook\0",
			   13)){
	ags_file_read_notebook(file,
			       child,
			       &(gobject->current_notebook));
      }
    }

    child = child->next;
  }

  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start\0",
		   G_CALLBACK(ags_file_read_editor_launch), gobject);
  ags_file_add_launch(file,
		      (GObject *) file_launch);
}

void
ags_file_read_editor_resolve_parameter(AgsFileLookup *file_lookup,
				       AgsEditor *editor)
{
  gchar *name;
  GValue *value;

  name = g_object_get_data(G_OBJECT(file_lookup),
			   AGS_FILE_READ_EDITOR_PARAMETER_NAME);
  value = file_lookup->ref;

  g_object_set_property(G_OBJECT(editor),
			name,
			value);
}

void
ags_file_read_editor_launch(AgsFileLaunch *file_launch,
			    AgsEditor *editor)
{
  AgsMachine *machine;
  GList *list;
  double tact_factor, zoom_factor;
  double tact;
  guint history;
  guint tabs, pads;
  guint i;

  machine = editor->selected_machine;

  if(machine == NULL){
    return;
  }

  /* set tabs */
  tabs = machine->audio->audio_channels;

  for(i = 0; i < tabs; i++){
    ags_notebook_add_tab(editor->current_notebook);
  }

  list = editor->current_notebook->tabs;

  while(list != NULL){
    gtk_toggle_button_set_active(AGS_NOTEBOOK_TAB(list->data)->toggle,
				 TRUE);

    list = list->next;
  }

  /* set zoom */
  zoom_factor = 0.25;

  tact_factor = exp2(8.0 - (double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom));
  tact = exp2((double) gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom) - 4.0);

  if(AGS_IS_NOTE_EDIT(editor->current_edit_widget)){
    /* reset note edit */
    history = gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom);

    editor->toolbar->zoom_history = history;

    AGS_NOTE_EDIT(editor->current_edit_widget)->flags |= AGS_NOTE_EDIT_RESETING_HORIZONTALLY;
    ags_note_edit_reset_horizontally(AGS_NOTE_EDIT(editor->current_edit_widget), AGS_NOTE_EDIT_RESET_HSCROLLBAR |
				     AGS_NOTE_EDIT_RESET_WIDTH);
    AGS_NOTE_EDIT(editor->current_edit_widget)->flags &= (~AGS_NOTE_EDIT_RESETING_HORIZONTALLY);

    /* reset ruler */
    AGS_NOTE_EDIT(editor->current_edit_widget)->ruler->factor = tact_factor;
    AGS_NOTE_EDIT(editor->current_edit_widget)->ruler->precision = tact;
    AGS_NOTE_EDIT(editor->current_edit_widget)->ruler->scale_precision = 1.0 / tact;

    gtk_widget_queue_draw((GtkWidget *) AGS_NOTE_EDIT(editor->current_edit_widget)->ruler);
    gtk_widget_queue_draw((GtkWidget *) AGS_NOTE_EDIT(editor->current_edit_widget));
  }else if(AGS_IS_PATTERN_EDIT(editor->current_edit_widget)){
    /* reset pattern edit */
    history = gtk_combo_box_get_active((GtkComboBox *) editor->toolbar->zoom);

    editor->toolbar->zoom_history = history;

    AGS_PATTERN_EDIT(editor->current_edit_widget)->flags |= AGS_PATTERN_EDIT_RESETING_HORIZONTALLY;
    ags_pattern_edit_reset_horizontally(AGS_PATTERN_EDIT(editor->current_edit_widget), AGS_PATTERN_EDIT_RESET_HSCROLLBAR |
					AGS_PATTERN_EDIT_RESET_WIDTH);
    AGS_PATTERN_EDIT(editor->current_edit_widget)->flags &= (~AGS_PATTERN_EDIT_RESETING_HORIZONTALLY);

    /* reset ruler */
    AGS_PATTERN_EDIT(editor->current_edit_widget)->ruler->factor = tact_factor;
    AGS_PATTERN_EDIT(editor->current_edit_widget)->ruler->precision = tact;
    AGS_PATTERN_EDIT(editor->current_edit_widget)->ruler->scale_precision = 1.0 / tact;

    gtk_widget_queue_draw((GtkWidget *) AGS_PATTERN_EDIT(editor->current_edit_widget)->ruler);
    gtk_widget_queue_draw((GtkWidget *) AGS_PATTERN_EDIT(editor->current_edit_widget));
  }
}

xmlNode*
ags_file_write_editor(AgsFile *file, xmlNode *parent, AgsEditor *editor)
{
  AgsFileIdRef *id_ref;
  xmlNode *node;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-editor\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", editor,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     editor->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     editor->build_id);

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", editor->flags));

  xmlAddChild(parent,
	      node);  

  /* child elements */
  ags_file_write_machine_selector(file, node, editor->machine_selector);
  ags_file_write_toolbar(file, node, editor->toolbar);
  ags_file_write_notebook(file, node, editor->current_notebook);
}

void
ags_file_read_toolbar(AgsFile *file, xmlNode *node, AgsToolbar **toolbar)
{
  AgsToolbar *gobject;
  GtkTreeIter iter;
  GtkTreeModel *model;
  xmlNode *child;
  gchar *value;
  gchar *str;

  if(*toolbar == NULL){
    gobject = (AgsToolbar *) g_object_new(AGS_TYPE_TOOLBAR,
					 NULL);
    *toolbar = gobject;
  }else{
    gobject = *toolbar;
  }
  
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  str = xmlGetProp(node,
		   "edit-mode\0");

  if(!g_strcmp0("position\0",
		str)){
    gtk_button_clicked((GtkButton *) gobject->position);
  }else if(!g_strcmp0("edit\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->edit);
  }else if(!g_strcmp0("clear\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->clear);
  }else if(!g_strcmp0("select\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->select);
  }

  /* zoom */
  str = xmlGetProp(node,
		   "zoom\0");

  model = gtk_combo_box_get_model((GtkComboBox *) gobject->zoom);

  if(gtk_tree_model_get_iter_first(model, &iter)){
    do{
      gtk_tree_model_get(model, &iter,
			 0, &value,
			 -1);

      if(!g_strcmp0(str,
		    value)){
	break;
      }
    }while(gtk_tree_model_iter_next(model,
				    &iter));

    gtk_combo_box_set_active_iter((GtkComboBox *) gobject->zoom,
				  &iter);

    gobject->zoom_history = gtk_combo_box_get_active((GtkComboBox *) gobject->zoom);
  }

  /* mode */
  str = xmlGetProp(node,
		   "mode\0");
  
  model = gtk_combo_box_get_model((GtkComboBox *) gobject->mode);
  
  if(gtk_tree_model_get_iter_first(model, &iter)){
    do{
      gtk_tree_model_get(model, &iter,
			 0, &value,
			 -1);

      if(!g_strcmp0(str,
		    value)){
	break;
      }
    }while(gtk_tree_model_iter_next(GTK_TREE_MODEL(model),
				    &iter));

    gtk_combo_box_set_active_iter((GtkComboBox *) gobject->mode,
				  &iter);
  }
}

xmlNode*
ags_file_write_toolbar(AgsFile *file, xmlNode *parent, AgsToolbar *toolbar)
{
  AgsFileIdRef *id_ref;
  xmlNode *node;
  GList *list;
  guint n_properties, n_params;
  gchar *id;
  gint i;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-toolbar\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", toolbar,
				   NULL));

  if(toolbar->selected_edit_mode == toolbar->position){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("position\0"));
  }else if(toolbar->selected_edit_mode == toolbar->edit){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("edit\0"));
  }else if(toolbar->selected_edit_mode == toolbar->clear){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("clear\0"));
  }else if(toolbar->selected_edit_mode == toolbar->select){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("select\0"));
  }

  xmlNewProp(node,
	     "zoom\0",
	     g_strdup_printf("%s\0", gtk_combo_box_text_get_active_text(toolbar->zoom)));

  xmlNewProp(node,
	     "mode\0",
	     g_strdup_printf("%s\0", gtk_combo_box_text_get_active_text(toolbar->mode)));

  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_file_read_machine_selector(AgsFile *file, xmlNode *node, AgsMachineSelector **machine_selector)
{
  AgsMachineSelector *gobject;
  GParameter *parameter;
  xmlNode *child;
  guint n_params;

  if(*machine_selector == NULL){
    gobject = g_object_new(AGS_TYPE_MACHINE_SELECTOR,
			   NULL);

    *machine_selector = gobject;
  }else{
    gobject = *machine_selector;
  }

  /* child elements */
  child = node->children;

  parameter = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-parameter\0",
		     14)){
	AgsFileLookup *file_lookup;
	xmlNode *value_node;
	GList *list;

	ags_file_util_read_parameter(file,
				     child,
				     NULL,
				     &parameter,
				     &n_params,
				     NULL);

	value_node = child->children;

	while(value_node != NULL){
	  if(value_node->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(value_node->name,
			   "ags-value\0",
			   10)){
	      list = ags_file_lookup_find_by_node(file->lookup,
						  value_node);
	  
	      if(list != NULL){
		file_lookup = AGS_FILE_LOOKUP(list->data);
		g_signal_connect_after(G_OBJECT(file_lookup), "resolve\0",
				       G_CALLBACK(ags_file_read_machine_selector_resolve_parameter), gobject);
	      }
	    }
	  }

	  value_node = value_node->next;
	}
      }
    }

    child = child->next;
  }
}

void
ags_file_read_machine_selector_resolve_parameter(AgsFileLookup *file_lookup,
						 AgsMachineSelector *machine_selector)
{
  AgsEditor *editor;
  GObject *gobject;
  GValue *value;

  value = file_lookup->ref;

  if(G_VALUE_HOLDS(value, G_TYPE_OBJECT)){
    AgsMachineRadioButton *machine_radio_button;
    GList *list;
    
    gobject = g_value_get_object(value);

    if(gobject == NULL){
      return;
    }

    editor = (AgsEditor *) gtk_widget_get_ancestor((GtkWidget *) machine_selector,
						   AGS_TYPE_EDITOR);

    list = gtk_container_get_children((GtkContainer *) machine_selector);
    
    machine_radio_button = g_object_new(AGS_TYPE_MACHINE_RADIO_BUTTON,
					NULL);
    gtk_box_pack_start(GTK_BOX(machine_selector),
		       (GtkWidget *) machine_radio_button,
		       FALSE, FALSE,
		       0);
    if(list->next != NULL){
      g_object_set(machine_radio_button,
		   "group\0", list->next->data,
		   "machine\0", gobject,
		   NULL);
    }else{
      g_object_set(machine_radio_button,
		   "machine\0", gobject,
		   NULL);
    }

    g_list_free(list);
  }
}

xmlNode*
ags_file_write_machine_selector(AgsFile *file, xmlNode *parent, AgsMachineSelector *machine_selector)
{
  xmlNode *node;
  GParameter *parameter;
  GList *list;
  gchar *id;
  gint n_params;

  auto GParameter* ags_file_write_machine_selector_parameter(GList *list, GParameter *parameter, gchar *prop, gint *n_params);

  GParameter* ags_file_write_machine_selector_parameter(GList *list, GParameter *parameter, gchar *prop, gint *n_params){
    gint i;

    if(n_params == NULL){
      i = 0;
    }else{
      i = *n_params;
    }

    while(list != NULL){
      if(!AGS_IS_MACHINE_RADIO_BUTTON(list->data)){
	list = list->next;
	continue;
      }

      if(parameter == NULL){
	parameter = (GParameter *) malloc(sizeof(GParameter));
      }else{
	parameter = (GParameter *) realloc(parameter,
					   (i + 1) * sizeof(GParameter));
      }

      parameter[i].name = prop;

      memset(&(parameter[i].value), 0, sizeof(GValue));
      g_value_init(&(parameter[i].value), G_TYPE_OBJECT);
      g_value_set_object(&(parameter[i].value),
			 G_OBJECT(AGS_MACHINE_RADIO_BUTTON(list->data)->machine));

      list = list->next;
      i++;
    }

    if(n_params != NULL){
      *n_params = i;
    }

    return(parameter);
  }

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-machine-selector\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);
 
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", machine_selector,
				   NULL));

  xmlAddChild(parent,
	      node);

  /* child elements */
  parameter = NULL;
  n_params = 0;

  list = gtk_container_get_children(GTK_CONTAINER(machine_selector));
  list = list->next;
  parameter = ags_file_write_machine_selector_parameter(list, parameter, "machine\0", &n_params);

  ags_file_util_write_parameter(file,
				node,
				ags_id_generator_create_uuid(),
				parameter, n_params);

  return(node);
}

void
ags_file_read_notebook(AgsFile *file, xmlNode *node, AgsNotebook **notebook)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_notebook(AgsFile *file, xmlNode *parent, AgsNotebook *notebook)
{
  //TODO:JK: implement me
}

void
ags_file_read_notebook_tab_list(AgsFile *file, xmlNode *node, GList **notebook_tab_list)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_notebook_tab_list(AgsFile *file, xmlNode *parent, GList *notebook_tab_list)
{
  //TODO:JK: implement me
}

void
ags_file_read_notebook_tab(AgsFile *file, xmlNode *node, AgsNotebookTab **notebook_tab)
{
  //TODO:JK: implement me
}

xmlNode*
ags_file_write_notebook_tab(AgsFile *file, xmlNode *parent, AgsNotebookTab *notebook_tab)
{
  //TODO:JK: implement me
}

void
ags_file_read_navigation(AgsFile *file, xmlNode *node, AgsNavigation **navigation)
{
  AgsNavigation *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  xmlChar *str;

  if(*navigation == NULL){
    gobject = g_object_new(AGS_TYPE_NAVIGATION,
			   NULL);

    *navigation = gobject;
  }else{
    gobject = *navigation;
  }

  str = xmlGetProp(node,
		   "expanded\0");

  if(!xmlStrncmp(str,
		 AGS_FILE_TRUE,
		 5)){
    gtk_toggle_button_set_active((GtkToggleButton *) gobject->expander,
				 TRUE);
  }

  str = xmlGetProp(node,
		   "bpm\0");
  gtk_spin_button_set_value(gobject->bpm,
			    g_strtod(str,
				     NULL));

  str = xmlGetProp(node,
		   "loop\0");

  if(!xmlStrncmp(str,
		 AGS_FILE_TRUE,
		 5)){
    gtk_toggle_button_set_active((GtkToggleButton *) gobject->loop,
				 TRUE);
  }

  str = xmlGetProp(node,
		   "position\0");
  gtk_spin_button_set_value(gobject->position_tact,
			    g_strtod(str,
				     NULL));

  str = xmlGetProp(node,
		   "loop-left\0");
  gtk_spin_button_set_value(gobject->loop_left_tact,
			    g_strtod(str,
				     NULL));

  str = xmlGetProp(node,
		   "loop-right\0");
  gtk_spin_button_set_value(gobject->loop_right_tact,
			    g_strtod(str,
				     NULL));

  /* soundcard */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_navigation_resolve_soundcard), gobject);
}

void
ags_file_read_navigation_resolve_soundcard(AgsFileLookup *file_lookup,
					AgsNavigation *navigation)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;

  xpath = "xpath=//ags-soundcard";
  
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);

  if(id_ref != NULL){
    g_object_set(G_OBJECT(navigation),
		 "soundcard\0", id_ref->ref,
		 NULL);
    gtk_spin_button_set_value(navigation->bpm,
			      ags_soundcard_get_bpm(AGS_SOUNDCARD(id_ref->ref)));
  }
}

xmlNode*
ags_file_write_navigation(AgsFile *file, xmlNode *parent, AgsNavigation *navigation)
{
  xmlNode *node;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-navigation\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", navigation,
				   NULL));

  xmlNewProp(node,
	     "expanded\0",
	     g_strdup_printf("%s\0", ((gtk_toggle_button_get_active((GtkToggleButton *) navigation->expander)) ? AGS_FILE_TRUE: AGS_FILE_FALSE)));
  
  xmlNewProp(node,
	     "bpm\0",
	     g_strdup_printf("%.f\0", gtk_spin_button_get_value(navigation->bpm)));
  
  xmlNewProp(node,
	     "loop\0",
	     g_strdup_printf("%s\0", ((gtk_toggle_button_get_active((GtkToggleButton *) navigation->loop)) ? AGS_FILE_TRUE: AGS_FILE_FALSE)));
 
  xmlNewProp(node,
	     "position\0",
	     g_strdup_printf("%.3f\0", gtk_spin_button_get_value(navigation->position_tact)));

  xmlNewProp(node,
	     "loop-left\0",
	     g_strdup_printf("%.3f\0", gtk_spin_button_get_value(navigation->loop_left_tact)));

  xmlNewProp(node,
	     "loop-right\0",
	     g_strdup_printf("%.3f\0", gtk_spin_button_get_value(navigation->loop_right_tact)));

  xmlAddChild(parent,
	      node);  
}

void
ags_file_read_automation_window(AgsFile *file, xmlNode *node, AgsAutomationWindow **automation_window)
{
  AgsAutomationWindow *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  
  if(*automation_window == NULL){
    gobject = g_object_new(AGS_TYPE_AUTOMATION_WINDOW,
			   NULL);
    *automation_window = gobject;
  }else{
    gobject = *automation_window;
  }

  g_object_set(G_OBJECT(gobject),
	       "application-context\0", file->application_context,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  /* child elements */
  child = node->children;

  while(child != NULL){

    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-edit\0",
		     20)){
	ags_file_read_automation_editor(file,
					child,
					&(gobject->automation_editor));
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_automation_window(AgsFile *file, xmlNode *parent, AgsAutomationWindow *automation_window)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-automation-window\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", automation_window,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", automation_window->flags));

  /* child elements */
  ags_file_write_automation_editor(file,
				   node,
				   automation_window->automation_editor);
}

void
ags_file_read_automation_window_list(AgsFile *file, xmlNode *node, GList **automation_window_list)
{
  AgsAutomationWindow *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-window\0",
		     12)){
	current = NULL;
	ags_file_read_automation_window(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation_window_list = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_automation_window_list(AgsFile *file, xmlNode *parent, GList *automation_window_list)
{
  AgsAutomationWindow *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-automation-window-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = automation_window_list;

  while(list != NULL){
    ags_file_write_automation_window(file, node, AGS_AUTOMATION_WINDOW(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_automation_editor(AgsFile *file, xmlNode *node, AgsAutomationEditor **automation_editor)
{
  AgsAutomationEditor *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  
  if(*automation_editor == NULL){
    gobject = g_object_new(AGS_TYPE_AUTOMATION_EDITOR,
			   NULL);
    *automation_editor = gobject;
  }else{
    gobject = *automation_editor;
  }

  g_object_set(G_OBJECT(gobject),
	       "application-context\0", file->application_context,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  /* child elements */
  child = node->children;

  while(child != NULL){

    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-machine-selector\0",
		     21)){
	ags_file_read_machine_selector(file,
				       child,
				       &(gobject->machine_selector));
      }else if(!xmlStrncmp(child->name,
			   "ags-automation-toolbar\0",
			   23)){
	ags_file_read_automation_toolbar(file,
					 child,
					 &(gobject->automation_toolbar));
      }else if(!xmlStrncmp(child->name,
			   "ags-automation-edit\0",
			   20)){
	//TODO:JK: implement me
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_automation_editor(AgsFile *file, xmlNode *parent, AgsAutomationEditor *automation_editor)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-automation-editor\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", automation_editor,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", automation_editor->flags));

  /* child elements */
  ags_file_write_machine_selector(file,
				  node,
				  automation_editor->machine_selector);

  ags_file_write_automation_toolbar(file,
				    node,
				    automation_editor->automation_toolbar);
  //TODO:JK: implement me
}

void
ags_file_read_automation_editor_list(AgsFile *file, xmlNode *node, GList **automation_editor_list)
{
  AgsAutomationEditor *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-editor\0",
		     12)){
	current = NULL;
	ags_file_read_automation_editor(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation_editor_list = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_automation_editor_list(AgsFile *file, xmlNode *parent, GList *automation_editor_list)
{
  AgsAutomationEditor *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-automation-editor-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = automation_editor_list;

  while(list != NULL){
    ags_file_write_automation_editor(file, node, AGS_AUTOMATION_EDITOR(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_automation_edit(AgsFile *file, xmlNode *node, AgsAutomationEdit **automation_edit)
{
  AgsAutomationEdit *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  
  if(*automation_edit == NULL){
    gobject = g_object_new(AGS_TYPE_AUTOMATION_EDIT,
			   NULL);
    *automation_edit = gobject;
  }else{
    gobject = *automation_edit;
  }

  g_object_set(G_OBJECT(gobject),
	       "application-context\0", file->application_context,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  if(!xmlStrncmp(xmlGetProp(node,
			    AGS_FILE_SCOPE_PROP),
		 "audio\0",
		 6)){
    gobject->scope = AGS_TYPE_AUDIO;
  }else if(!xmlStrncmp(xmlGetProp(node,
				  AGS_FILE_SCOPE_PROP),
		       "output\0",
		       6)){
    gobject->scope = AGS_TYPE_OUTPUT;
  }else{
    gobject->scope = AGS_TYPE_INPUT;
  }

  gobject->map_width = g_ascii_strtoull(xmlGetProp(node,
						   "map-width"),
					NULL,
					10);
  gobject->map_height = g_ascii_strtoull(xmlGetProp(node,
						    "map-height"),
					 NULL,
					 10);

  gobject->edit_x = g_ascii_strtoull(xmlGetProp(node,
						"edit-x"),
				     NULL,
				     10);
  gobject->edit_y = g_ascii_strtoull(xmlGetProp(node,
						"edit-y"),
				     NULL,
				     10);
  
  /* child elements */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-area-list\0",
		     25)){
	ags_file_read_automation_area_list(file,
					   child,
					   &(gobject->automation_area));
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_file_write_automation_edit(AgsFile *file, xmlNode *parent, AgsAutomationEdit *automation_edit)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-automation-edit\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", automation_edit,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", automation_edit->flags));

  if(g_type_is_a(automation_edit->scope,
		 AGS_TYPE_AUDIO)){
    xmlNewProp(node,
	       AGS_FILE_SCOPE_PROP,
	       g_strdup("audio\0"));
  }else if(g_type_is_a(automation_edit->scope,
		       AGS_TYPE_OUTPUT)){
    xmlNewProp(node,
	       AGS_FILE_SCOPE_PROP,
	       g_strdup("output\0"));
  }else{
    xmlNewProp(node,
	       AGS_FILE_SCOPE_PROP,
	       g_strdup("input\0"));
  }

  xmlNewProp(node,
	     "map-width\0",
	     g_strdup_printf("%d\0", automation_edit->map_width));
  xmlNewProp(node,
	     "map-height\0",
	     g_strdup_printf("%d\0", automation_edit->map_height));

  xmlNewProp(node,
	     "edit-x\0",
	     g_strdup_printf("%d\0", automation_edit->edit_x));
  xmlNewProp(node,
	     "edit-y\0",
	     g_strdup_printf("%d\0", automation_edit->edit_y));
  
  /* child elements */
  ags_file_write_automation_area_list(file,
				      node,
				      automation_edit->automation_area);
}

void
ags_file_read_automation_edit_list(AgsFile *file, xmlNode *node, GList **automation_edit_list)
{
  AgsAutomationEdit *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-edit\0",
		     12)){
	current = NULL;
	ags_file_read_automation_edit(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation_edit_list = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_automation_edit_list(AgsFile *file, xmlNode *parent, GList *automation_edit_list)
{
  AgsAutomationEdit *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-automation-edit-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = automation_edit_list;

  while(list != NULL){
    ags_file_write_automation_edit(file, node, AGS_AUTOMATION_EDIT(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_automation_toolbar(AgsFile *file, xmlNode *node, AgsAutomationToolbar **automation_toolbar)
{
  AgsAutomationToolbar *gobject;
  GtkTreeIter iter;
  GtkTreeModel *model;
  xmlNode *child;
  gchar *value;
  gchar *str;

  if(*automation_toolbar == NULL){
    gobject = (AgsAutomationToolbar *) g_object_new(AGS_TYPE_AUTOMATION_TOOLBAR,
						    NULL);
    *automation_toolbar = gobject;
  }else{
    gobject = *automation_toolbar;
  }
  
  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  str = xmlGetProp(node,
		   "edit-mode\0");

  if(!g_strcmp0("position\0",
		str)){
    gtk_button_clicked((GtkButton *) gobject->position);
  }else if(!g_strcmp0("edit\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->edit);
  }else if(!g_strcmp0("clear\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->clear);
  }else if(!g_strcmp0("select\0",
		      str)){
    gtk_button_clicked((GtkButton *) gobject->select);
  }

  /* zoom */
  str = xmlGetProp(node,
		   "zoom\0");

  model = gtk_combo_box_get_model((GtkComboBox *) gobject->zoom);

  if(gtk_tree_model_get_iter_first(model, &iter)){
    do{
      gtk_tree_model_get(model, &iter,
			 0, &value,
			 -1);

      if(!g_strcmp0(str,
		    value)){
	break;
      }
    }while(gtk_tree_model_iter_next(model,
				    &iter));

    gtk_combo_box_set_active_iter((GtkComboBox *) gobject->zoom,
				  &iter);

    gobject->zoom_history = gtk_combo_box_get_active(GTK_COMBO_BOX(gobject->zoom));
  }
}

xmlNode*
ags_file_write_automation_toolbar(AgsFile *file, xmlNode *parent, AgsAutomationToolbar *automation_toolbar)
{
  AgsFileIdRef *id_ref;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-automation-toolbar\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", automation_toolbar,
				   NULL));

  if(automation_toolbar->selected_edit_mode == automation_toolbar->position){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("position\0"));
  }else if(automation_toolbar->selected_edit_mode == automation_toolbar->edit){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("edit\0"));
  }else if(automation_toolbar->selected_edit_mode == automation_toolbar->clear){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("clear\0"));
  }else if(automation_toolbar->selected_edit_mode == automation_toolbar->select){
    xmlNewProp(node,
	       "edit-mode\0",
	       g_strdup("select\0"));
  }

  xmlNewProp(node,
	     "zoom\0",
	     g_strdup_printf("%s\0", gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(automation_toolbar->zoom))));

  xmlAddChild(parent,
	      node);  

  return(node);
}

void
ags_file_read_automation_toolbar_list(AgsFile *file, xmlNode *node, GList **automation_toolbar_list)
{
  AgsAutomationToolbar *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-toolbar\0",
		     12)){
	current = NULL;
	ags_file_read_automation_toolbar(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation_toolbar_list = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_automation_toolbar_list(AgsFile *file, xmlNode *parent, GList *automation_toolbar_list)
{
  AgsAutomationToolbar *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-automation-toolbar-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = automation_toolbar_list;

  while(list != NULL){
    ags_file_write_automation_toolbar(file, node, AGS_AUTOMATION_TOOLBAR(list->data));

    list = list->next;
  }

  return(node);
}

void
ags_file_read_automation_area(AgsFile *file, xmlNode *node, AgsAutomationArea **automation_area)
{
  AgsAutomationArea *gobject;
  AgsFileLookup *file_lookup;
  xmlNode *child;
  
  if(*automation_area == NULL){
    gobject = g_object_new(AGS_TYPE_AUTOMATION_AREA,
			   NULL);
    *automation_area = gobject;
  }else{
    gobject = *automation_area;
  }

  g_object_set(G_OBJECT(gobject),
	       "application-context\0", file->application_context,
	       NULL);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  gobject->flags = (guint) g_ascii_strtoull(xmlGetProp(node, AGS_FILE_FLAGS_PROP),
					    NULL,
					    16);

  gobject->y = (guint) g_ascii_strtoull(xmlGetProp(node, "y\0"),
					NULL,
					10);
  gobject->height = (guint) g_ascii_strtoull(xmlGetProp(node, "height\0"),
					     NULL,
					     10);

  gobject->channel_type = g_type_from_name(xmlGetProp(node, "channel-type\0"));

  gobject->filename = g_strdup(xmlGetProp(node, "filename\0"));
  gobject->effect = g_strdup(xmlGetProp(node, "effect\0"));
  gobject->control_specifier = g_strdup(xmlGetProp(node, "control-specifier\0"));
  
  gobject->control_name = g_strdup(xmlGetProp(node, "control-name\0"));

  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", gobject,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_read_automation_area_resolve_audio), gobject);
}

xmlNode*
ags_file_write_automation_area(AgsFile *file, xmlNode *parent, AgsAutomationArea *automation_area)
{
  AgsFileLookup *file_lookup;
  xmlNode *node, *child;
  gchar *id;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-automation-area\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", automation_area,
				   NULL));

  xmlNewProp(node,
	     AGS_FILE_FLAGS_PROP,
	     g_strdup_printf("%x\0", automation_area->flags));

  xmlNewProp(node,
	     "channel-type\0",
	     g_strdup(g_type_name(automation_area->channel_type)));

  xmlNewProp(node,
	     "filename\0",
	     g_strdup(automation_area->filename));
  xmlNewProp(node,
	     "effect\0",
	     g_strdup(automation_area->effect));
  xmlNewProp(node,
	     "control-specifier\0",
	     g_strdup(automation_area->control_specifier));

  xmlNewProp(node,
	     "control-name\0",
	     g_strdup(automation_area->control_name));
  
  /* audio */
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file\0", file,
					       "node\0", node,
					       "reference\0", automation_area,
					       NULL);
  ags_file_add_lookup(file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve\0",
		   G_CALLBACK(ags_file_write_automation_area_resolve_audio), automation_area);
}

void
ags_file_write_automation_area_resolve_audio(AgsFileLookup *file_lookup,
					     AgsAutomationArea *automation_area)
{
  AgsFileIdRef *id_ref;
  gchar *id;

  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_reference(file_lookup->file, AGS_AUDIO(automation_area->audio));

  id = xmlGetProp(id_ref->node, AGS_FILE_ID_PROP);

  xmlNewProp(file_lookup->node,
	     "audio\0",
	     g_strdup_printf("xpath=//ags-audio[@id='%s']\0", id));
}

xmlNode*
ags_file_read_automation_area_resolve_audio(AgsFileLookup *file_lookup,
					    AgsAutomationArea *automation_area)
{
  AgsFileIdRef *id_ref;
  gchar *xpath;
  
  xpath = (gchar *) xmlGetProp(file_lookup->node,
			       "audio\0");
  g_message("xpath = %s\0", xpath);
  id_ref = (AgsFileIdRef *) ags_file_find_id_ref_by_xpath(file_lookup->file, xpath);
  
  g_object_set(G_OBJECT(automation_area),
	       "audio\0", (AgsAudio *) id_ref->ref,
	       NULL);
}

void
ags_file_read_automation_area_list(AgsFile *file, xmlNode *node, GList **automation_area_list)
{
  AgsAutomationArea *current;
  GList *list;
  xmlNode *child;
  xmlChar *id;

  id = xmlGetProp(node, AGS_FILE_ID_PROP);

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-automation-area\0",
		     12)){
	current = NULL;
	ags_file_read_automation_area(file, child, &current);

	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation_area_list = list;

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));
}

xmlNode*
ags_file_write_automation_area_list(AgsFile *file, xmlNode *parent, GList *automation_area_list)
{
  AgsAutomationArea *current;
  xmlNode *node;
  GList *list;
  gchar *id;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-automation-area-list\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", list,
				   NULL));

  xmlAddChild(parent,
	      node);

  list = automation_area_list;

  while(list != NULL){
    ags_file_write_automation_area(file, node, AGS_AUTOMATION_AREA(list->data));

    list = list->next;
  }

  return(node);
}

