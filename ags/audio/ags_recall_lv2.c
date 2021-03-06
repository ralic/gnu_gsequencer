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

#include <ags/audio/ags_recall_lv2.h>

#include <ags/util/ags_id_generator.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_plugin.h>
#include <ags/object/ags_soundcard.h>

#include <ags/plugin/ags_lv2_manager.h>
#include <ags/plugin/ags_lv2_plugin.h>
#include <ags/plugin/ags_lv2_conversion.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/audio/ags_port.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <libxml/tree.h>

void ags_recall_lv2_class_init(AgsRecallLv2Class *recall_lv2_class);
void ags_recall_lv2_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_lv2_plugin_interface_init(AgsPluginInterface *plugin);
void ags_recall_lv2_init(AgsRecallLv2 *recall_lv2);
void ags_recall_lv2_set_property(GObject *gobject,
				 guint prop_id,
				 const GValue *value,
				 GParamSpec *param_spec);
void ags_recall_lv2_get_property(GObject *gobject,
				 guint prop_id,
				 GValue *value,
				 GParamSpec *param_spec);
void ags_recall_lv2_connect(AgsConnectable *connectable);
void ags_recall_lv2_disconnect(AgsConnectable *connectable);
void ags_recall_lv2_set_ports(AgsPlugin *plugin, GList *port);
void ags_recall_lv2_dispose(GObject *gobject);
void ags_recall_lv2_finalize(GObject *gobject);

void ags_recall_lv2_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_recall_lv2_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

/**
 * SECTION:ags_recall_lv2
 * @short_description: The object interfacing with LV2
 * @title: AgsRecallLv2
 * @section_id:
 * @include: ags/audio/ags_recall_lv2.h
 *
 * #AgsRecallLv2 provides LV2 support.
 */

enum{
  PROP_0,
  PROP_TURTLE,
  PROP_FILENAME,
  PROP_EFFECT,
  PROP_URI,
  PROP_INDEX,
};

static gpointer ags_recall_lv2_parent_class = NULL;
static AgsConnectableInterface* ags_recall_lv2_parent_connectable_interface;
static AgsPluginInterface* ags_recall_lv2_parent_plugin_interface;

GType
ags_recall_lv2_get_type (void)
{
  static GType ags_type_recall_lv2 = 0;

  if(!ags_type_recall_lv2){
    static const GTypeInfo ags_recall_lv2_info = {
      sizeof (AgsRecallLv2Class),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_lv2_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallLv2),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_lv2_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_lv2_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_recall_lv2_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_lv2 = g_type_register_static(AGS_TYPE_RECALL_CHANNEL,
						 "AgsRecallLv2\0",
						 &ags_recall_lv2_info,
						 0);

    g_type_add_interface_static(ags_type_recall_lv2,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_recall_lv2,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_recall_lv2);
}

void
ags_recall_lv2_class_init(AgsRecallLv2Class *recall_lv2)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_recall_lv2_parent_class = g_type_class_peek_parent(recall_lv2);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_lv2;

  gobject->set_property = ags_recall_lv2_set_property;
  gobject->get_property = ags_recall_lv2_get_property;

  gobject->dispose = ags_recall_lv2_dispose;
  gobject->finalize = ags_recall_lv2_finalize;

  /* properties */
  /**
   * AgsRecallLv2:turtle:
   *
   * The assigned turtle.
   * 
   * Since: 0.7.0
   */
  param_spec = g_param_spec_object("turtle\0",
				   "turtle of recall lv2\0",
				   "The turtle which this recall lv2 is described by\0",
				   AGS_TYPE_TURTLE,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_TURTLE,
				  param_spec);

  /**
   * AgsRecallLv2:filename:
   *
   * The plugins filename.
   * 
   * Since: 0.7.0
   */
  param_spec =  g_param_spec_string("filename\0",
				    "the object file\0",
				    "The filename as string of object file\0",
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILENAME,
				  param_spec);

  /**
   * AgsRecallLv2:effect:
   *
   * The effect's name.
   * 
   * Since: 0.7.0
   */
  param_spec =  g_param_spec_string("effect\0",
				    "the effect\0",
				    "The effect's string representation\0",
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_EFFECT,
				  param_spec);

  /**
   * AgsRecallLv2:uri:
   *
   * The uri's name.
   * 
   * Since: 0.7.0
   */
  param_spec =  g_param_spec_string("uri\0",
				    "the uri\0",
				    "The uri's string representation\0",
				    NULL,
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_URI,
				  param_spec);

  /**
   * AgsRecallLv2:index:
   *
   * The uri's index.
   * 
   * Since: 0.7.0
   */
  param_spec =  g_param_spec_ulong("index\0",
				   "index of uri\0",
				   "The numerical index of uri\0",
				   0,
				   65535,
				   0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_INDEX,
				  param_spec);
}

void
ags_recall_lv2_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recall_lv2_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_lv2_connect;
  connectable->disconnect = ags_recall_lv2_disconnect;
}

void
ags_recall_lv2_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_recall_lv2_parent_plugin_interface = g_type_interface_peek_parent(plugin);

  plugin->read = ags_recall_lv2_read;
  plugin->write = ags_recall_lv2_write;
  plugin->set_ports = ags_recall_lv2_set_ports;
}

void
ags_recall_lv2_init(AgsRecallLv2 *recall_lv2)
{
  AGS_RECALL(recall_lv2)->name = "ags-lv2\0";
  AGS_RECALL(recall_lv2)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(recall_lv2)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(recall_lv2)->xml_type = "ags-recall-lv2\0";
  AGS_RECALL(recall_lv2)->port = NULL;

  recall_lv2->flags = 0;
  
  recall_lv2->turtle = NULL;

  recall_lv2->filename = NULL;
  recall_lv2->effect = NULL;
  recall_lv2->uri = NULL;
  recall_lv2->index = 0;

  recall_lv2->plugin = NULL;
  recall_lv2->plugin_descriptor = NULL;

  recall_lv2->input_port = NULL;
  recall_lv2->input_lines = 0;

  recall_lv2->output_port = NULL;
  recall_lv2->output_lines = 0;

  recall_lv2->event_port = 0;
  recall_lv2->atom_port = 0;

  recall_lv2->bank = 0;
  recall_lv2->program = 0;
}

void
ags_recall_lv2_set_property(GObject *gobject,
			    guint prop_id,
			    const GValue *value,
			    GParamSpec *param_spec)
{
  AgsRecallLv2 *recall_lv2;

  recall_lv2 = AGS_RECALL_LV2(gobject);

  switch(prop_id){
  case PROP_TURTLE:
    {
      AgsTurtle *turtle;

      turtle = (AgsTurtle *) g_value_get_object(value);

      if(recall_lv2->turtle == turtle){
	return;
      }

      if(recall_lv2->turtle != NULL){
	g_object_unref(recall_lv2->turtle);
      }

      if(turtle != NULL){
	g_object_ref(turtle);
      }

      recall_lv2->turtle = turtle;
    }
    break;
  case PROP_FILENAME:
    {
      gchar *filename;

      filename = g_value_get_string(value);

      if(filename == recall_lv2->filename){
	return;
      }

      if(recall_lv2->filename != NULL){
	g_free(recall_lv2->filename);
      }

      recall_lv2->filename = g_strdup(filename);
    }
    break;
  case PROP_EFFECT:
    {
      gchar *effect;
      
      effect = g_value_get_string(value);

      if(effect == recall_lv2->effect){
	return;
      }

      if(recall_lv2->effect != NULL){
	g_free(recall_lv2->effect);
      }

      recall_lv2->effect = g_strdup(effect);
    }
    break;
  case PROP_URI:
    {
      gchar *uri;
      
      uri = g_value_get_string(value);

      if(uri == recall_lv2->uri){
	return;
      }

      if(recall_lv2->uri != NULL){
	g_free(recall_lv2->uri);
      }

      recall_lv2->uri = g_strdup(uri);
    }
    break;
  case PROP_INDEX:
    {
      uint32_t index;
      
      index = g_value_get_ulong(value);

      if(index == recall_lv2->index){
	return;
      }

      recall_lv2->index = index;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_recall_lv2_get_property(GObject *gobject,
			    guint prop_id,
			    GValue *value,
			    GParamSpec *param_spec)
{
  AgsRecallLv2 *recall_lv2;

  recall_lv2 = AGS_RECALL_LV2(gobject);

  switch(prop_id){
  case PROP_TURTLE:
    {
      g_value_set_object(value, recall_lv2->turtle);
    }
    break;
  case PROP_FILENAME:
    {
      g_value_set_string(value, recall_lv2->filename);
    }
    break;
  case PROP_EFFECT:
    {
      g_value_set_string(value, recall_lv2->effect);
    }
    break;
  case PROP_URI:
    {
      g_value_set_string(value, recall_lv2->uri);
    }
    break;
  case PROP_INDEX:
    {
      g_value_set_ulong(value, recall_lv2->index);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_lv2_connect(AgsConnectable *connectable)
{
  AgsRecall *recall;
  
  recall = AGS_RECALL(connectable);
  
  if((AGS_RECALL_CONNECTED & (recall->flags)) != 0){
    return;
  }

  /* load automation */
  ags_recall_load_automation(recall,
			     g_list_copy(recall->port));

  /* call parent */
  ags_recall_lv2_parent_connectable_interface->connect(connectable);
}

void
ags_recall_lv2_disconnect(AgsConnectable *connectable)
{
  ags_recall_lv2_parent_connectable_interface->disconnect(connectable);
}

void
ags_recall_lv2_dispose(GObject *gobject)
{
  AgsRecallLv2 *recall_lv2;
  
  recall_lv2 = AGS_RECALL_LV2(gobject);

  /* turtle */
  if(recall_lv2->turtle != NULL){
    g_object_unref(recall_lv2->turtle);

    recall_lv2->turtle = NULL;
  }

  /* call parent */
  G_OBJECT_CLASS(ags_recall_lv2_parent_class)->dispose(gobject);
}

void
ags_recall_lv2_finalize(GObject *gobject)
{
  AgsRecallLv2 *recall_lv2;
  
  recall_lv2 = AGS_RECALL_LV2(gobject);

  /* turtle */
  if(recall_lv2->turtle != NULL){
    g_object_unref(recall_lv2->turtle);
  }

  /* filename, effect and uri */
  g_free(recall_lv2->filename);
  g_free(recall_lv2->effect);
  g_free(recall_lv2->uri);
  
  /* call parent */
  G_OBJECT_CLASS(ags_recall_lv2_parent_class)->finalize(gobject);
}

void
ags_recall_lv2_set_ports(AgsPlugin *plugin, GList *port)
{
  AgsRecallLv2 *recall_lv2;
  AgsPort *current;
  
  AgsLv2Plugin *lv2_plugin;
  
  GList *list;  
  GList *port_descriptor;
  
  unsigned long port_count;
  unsigned long i;

  recall_lv2 = AGS_RECALL_LV2(plugin);

  lv2_plugin = ags_lv2_manager_find_lv2_plugin(ags_lv2_manager_get_instance(),
					       recall_lv2->filename, recall_lv2->effect);

  port_descriptor = AGS_BASE_PLUGIN(lv2_plugin)->port;

  if(port_descriptor != NULL){
    port_count = g_list_length(port_descriptor);

    for(i = 0; i < port_count; i++){
      if((AGS_PORT_DESCRIPTOR_CONTROL & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	gchar *specifier;
	
	specifier = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name;

	if(specifier == NULL){
	  port_descriptor = port_descriptor->next;
	  
	  continue;
	}
	
	list = port;
	current = NULL;
	
	while(list != NULL){
	  if(!g_strcmp0(specifier,
			AGS_PORT(list->data)->specifier)){
	    current = list->data;
	    break;
	  }
	  
	  list = list->next;
	}

	if(current != NULL){
	  current->port_descriptor = port_descriptor->data;
	  ags_recall_lv2_load_conversion(recall_lv2,
					 (GObject *) current,
					 port_descriptor->data);
	
	  current->port_value.ags_port_float = (float) ags_conversion_convert(current->conversion,
									      g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->default_value),
									      FALSE);
	
#ifdef AGS_DEBUG
	  g_message("connecting port: %d/%d\0", i, port_count);      
#endif
	}
      }else if((AGS_PORT_DESCRIPTOR_AUDIO & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	if((AGS_PORT_DESCRIPTOR_INPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  if(recall_lv2->input_port == NULL){
	    recall_lv2->input_port = (uint32_t *) malloc(sizeof(uint32_t));
	    recall_lv2->input_port[0] = i;
	  }else{
	    recall_lv2->input_port = (uint32_t *) realloc(recall_lv2->input_port,
								  (recall_lv2->input_lines + 1) * sizeof(uint32_t));
	    recall_lv2->input_port[recall_lv2->input_lines] = i;
	  }

	  recall_lv2->input_lines += 1;
	}else if((AGS_PORT_DESCRIPTOR_OUTPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  if(recall_lv2->output_port == NULL){
	    recall_lv2->output_port = (uint32_t *) malloc(sizeof(uint32_t));
	    recall_lv2->output_port[0] = i;
	  }else{
	    recall_lv2->output_port = (uint32_t *) realloc(recall_lv2->output_port,
								   (recall_lv2->output_lines + 1) * sizeof(uint32_t));
	    recall_lv2->output_port[recall_lv2->output_lines] = i;
	  }

	  recall_lv2->output_lines += 1;
	}
      }

      port_descriptor = port_descriptor->next;
    }

    AGS_RECALL(recall_lv2)->port = g_list_reverse(port);
  }
}

void
ags_recall_lv2_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsRecallLv2 *gobject;
  AgsLv2Plugin *lv2_plugin;
  gchar *filename, *effect, *uri;
  uint32_t index;

  gobject = AGS_RECALL_LV2(plugin);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  filename = xmlGetProp(node,
			"filename\0");
  effect = xmlGetProp(node,
		      "effect\0");
  uri = xmlGetProp(node,
		   "uri\0");
  index = g_ascii_strtoull(xmlGetProp(node,
				      "index\0"),
			   NULL,
			   10);

  g_object_set(gobject,
	       "filename\0", filename,
	       "effect\0", effect,
	       "uri\0", uri,
	       "index\0", index,
	       NULL);

  ags_recall_lv2_load(gobject);
}

xmlNode*
ags_recall_lv2_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsRecallLv2 *recall_lv2;
  xmlNode *node;
  GList *list;
  gchar *id;

  recall_lv2 = AGS_RECALL_LV2(plugin);

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-recall-lv2\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "application-context\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", recall_lv2,
				   NULL));

  xmlNewProp(node,
	     "filename\0",
	     g_strdup(recall_lv2->filename));
  
  xmlNewProp(node,
	     "effect\0",
	     g_strdup(recall_lv2->effect));

  xmlNewProp(node,
	     "uri\0",
	     g_strdup(recall_lv2->uri));

  xmlNewProp(node,
	     "index\0",
	     g_strdup_printf("%d\0", recall_lv2->index));

  xmlAddChild(parent,
	      node);

  return(node);
}

/**
 * ags_recall_lv2_load:
 * @recall_lv2: an #AgsRecallLv2
 *
 * Set up LV2 handle.
 * 
 * Since: 0.7.0
 */
void
ags_recall_lv2_load(AgsRecallLv2 *recall_lv2)
{
  AgsLv2Plugin *lv2_plugin;

  void *plugin_so;
  LV2_Descriptor_Function lv2_descriptor;
  LV2_Descriptor *plugin_descriptor;

  if(recall_lv2 == NULL ||
     !AGS_RECALL_LV2(recall_lv2)){
    return;
  }
  
  /*  */
  recall_lv2->plugin = 
    lv2_plugin = ags_lv2_manager_find_lv2_plugin(ags_lv2_manager_get_instance(),
						 recall_lv2->filename, recall_lv2->effect);
  plugin_so = AGS_BASE_PLUGIN(lv2_plugin)->plugin_so;
  
  if(plugin_so != NULL){
    lv2_descriptor = (LV2_Descriptor_Function) dlsym(plugin_so,
						     "lv2_descriptor\0");

    if(dlerror() == NULL && lv2_descriptor){
      recall_lv2->plugin_descriptor = 
	plugin_descriptor = lv2_descriptor(recall_lv2->index);

      if((AGS_LV2_PLUGIN_NEEDS_WORKER & (lv2_plugin->flags)) != 0){
	recall_lv2->flags |= AGS_RECALL_LV2_HAS_WORKER;
      }
    }
  }
}

/**
 * ags_recall_lv2_load_ports:
 * @recall_lv2: an #AgsRecallLv2
 *
 * Set up LV2 ports.
 *
 * Returns: a #GList containing #AgsPort.
 * 
 * Since: 0.7.0
 */
GList*
ags_recall_lv2_load_ports(AgsRecallLv2 *recall_lv2)
{
  AgsPort *current;

  AgsLv2Plugin *lv2_plugin;

  GList *port;
  GList *port_descriptor;

  uint32_t port_count;
  uint32_t i;

  lv2_plugin = ags_lv2_manager_find_lv2_plugin(ags_lv2_manager_get_instance(),
					       recall_lv2->filename, recall_lv2->effect);
#ifdef AGS_DEBUG
  g_message("ports from ttl: %s\0", lv2_plugin->turtle->filename);
#endif
  
  port = NULL;
  port_descriptor = AGS_BASE_PLUGIN(lv2_plugin)->port;
  
  if(port_descriptor != NULL){
    port_count = g_list_length(port_descriptor);
    
    for(i = 0; i < port_count; i++){
#ifdef AGS_DEBUG
      g_message("Lv2 plugin port-index: %d\0", AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index);
#endif
      
      if((AGS_PORT_DESCRIPTOR_INPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	if((AGS_PORT_DESCRIPTOR_EVENT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  recall_lv2->flags |= AGS_RECALL_LV2_HAS_EVENT_PORT;
	  recall_lv2->event_port = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	}
      
	if((AGS_PORT_DESCRIPTOR_ATOM & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  recall_lv2->flags |= AGS_RECALL_LV2_HAS_ATOM_PORT;
	  recall_lv2->atom_port = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	}
      }
      
      if((AGS_PORT_DESCRIPTOR_CONTROL & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0 &&
	 ((AGS_PORT_DESCRIPTOR_INPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0 ||
	  (AGS_PORT_DESCRIPTOR_OUTPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0)){
	gchar *plugin_name;
	gchar *specifier;

	specifier = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_name;

	if(specifier == NULL){
	  port_descriptor = port_descriptor->next;
	  
	  continue;
	}
	
	plugin_name = g_strdup_printf("lv2-<%s>\0", lv2_plugin->uri);

	current = g_object_new(AGS_TYPE_PORT,
			       "plugin-name\0", plugin_name,
			       "specifier\0", specifier,
			       "control-port\0", g_strdup_printf("%u/%u\0",
								 i,
								 port_count),
			       "port-value-is-pointer\0", FALSE,
			       "port-value-type\0", G_TYPE_FLOAT,
			       NULL);
	g_object_ref(current);
	
	if((AGS_PORT_DESCRIPTOR_OUTPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  AGS_RECALL(recall_lv2)->flags |= AGS_RECALL_HAS_OUTPUT_PORT;
	  
	  current->flags |= AGS_PORT_IS_OUTPUT;
	}else{
	  if((AGS_PORT_DESCRIPTOR_INTEGER & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) == 0 &&
	     (AGS_PORT_DESCRIPTOR_TOGGLED & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) == 0 &&
	     AGS_PORT_DESCRIPTOR(port_descriptor->data)->scale_steps == -1){
	    current->flags |= AGS_PORT_INFINITE_RANGE;
	  }
	}
	
	current->port_descriptor = port_descriptor->data;
	ags_recall_lv2_load_conversion(recall_lv2,
				       (GObject *) current,
				       port_descriptor->data);

	current->port_value.ags_port_float = (float) ags_conversion_convert(current->conversion,
									    g_value_get_float(AGS_PORT_DESCRIPTOR(port_descriptor->data)->default_value),
									    FALSE);

#ifdef AGS_DEBUG
	g_message("connecting port: %s %d/%d\0", specifier, i, port_count);
#endif
	
	port = g_list_prepend(port,
			      current);
      }else if((AGS_PORT_DESCRIPTOR_AUDIO & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	if((AGS_PORT_DESCRIPTOR_INPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  if(recall_lv2->input_port == NULL){
	    recall_lv2->input_port = (uint32_t *) malloc(sizeof(uint32_t));
	    recall_lv2->input_port[0] = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	  }else{
	    recall_lv2->input_port = (uint32_t *) realloc(recall_lv2->input_port,
								  (recall_lv2->input_lines + 1) * sizeof(uint32_t));
	    recall_lv2->input_port[recall_lv2->input_lines] = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	  }
	  
	  recall_lv2->input_lines += 1;
	}else if((AGS_PORT_DESCRIPTOR_OUTPUT & (AGS_PORT_DESCRIPTOR(port_descriptor->data)->flags)) != 0){
	  if(recall_lv2->output_port == NULL){
	    recall_lv2->output_port = (uint32_t *) malloc(sizeof(uint32_t));
	    recall_lv2->output_port[0] = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	  }else{
	    recall_lv2->output_port = (uint32_t *) realloc(recall_lv2->output_port,
								   (recall_lv2->output_lines + 1) * sizeof(uint32_t));
	    recall_lv2->output_port[recall_lv2->output_lines] = AGS_PORT_DESCRIPTOR(port_descriptor->data)->port_index;
	  }
	  
	  recall_lv2->output_lines += 1;
	}
      }

      port_descriptor = port_descriptor->next;
    }
    
    AGS_RECALL(recall_lv2)->port = g_list_reverse(port);
  }
  
  return(g_list_copy(AGS_RECALL(recall_lv2)->port));
}

/**
 * ags_recall_lv2_load_conversion:
 * @recall_lv2: the #AgsRecallLv2
 * @port: an #AgsPort
 * @port_descriptor: the #AgsPortDescriptor-struct
 * 
 * Loads conversion object by using @port_descriptor and sets in on @port.
 * 
 * Since: 0.7.9
 */
void
ags_recall_lv2_load_conversion(AgsRecallLv2 *recall_lv2,
			       GObject *port,
			       gpointer port_descriptor)
{
  AgsLv2Conversion *lv2_conversion;

  if(recall_lv2 == NULL ||
     port == NULL ||
     port_descriptor == NULL){
    return;
  }

  lv2_conversion = NULL;
  
  if((AGS_PORT_DESCRIPTOR_LOGARITHMIC & (AGS_PORT_DESCRIPTOR(port_descriptor)->flags)) != 0){
    lv2_conversion = ags_lv2_conversion_new();
    g_object_set(port,
		 "conversion\0", lv2_conversion,
		 NULL);
    
    lv2_conversion->flags |= AGS_LV2_CONVERSION_LOGARITHMIC;
  }
}

/**
 * ags_recall_lv2_find:
 * @recall: a #GList containing #AgsRecall
 * @filename: plugin filename
 * @uri: uri's name
 *
 * Retrieve LV2 recall.
 *
 * Returns: Next match.
 * 
 * Since: 0.7.0
 */
GList*
ags_recall_lv2_find(GList *recall,
		    gchar *filename, gchar *uri)
{
  while(recall != NULL){
    if(AGS_IS_RECALL_LV2(recall->data)){
      if(!g_strcmp0(AGS_RECALL_LV2(recall->data)->filename,
		    filename) &&
	 !g_strcmp0(AGS_RECALL_LV2(recall->data)->uri,
		    uri)){
	return(recall);
      }
    }

    recall = recall->next;
  }

  return(NULL);
}

/**
 * ags_recall_lv2_new:
 * @source: the source
 * @turtle: the .ttl file
 * @filename: the LV2 plugin filename
 * @effect: the LV2 plugin effect
 * @uri: uri's name
 * @index: uri's index
 *
 * Creates a #AgsRecallLv2
 *
 * Returns: a new #AgsRecallLv2
 * 
 * Since: 0.7.0
 */
AgsRecallLv2*
ags_recall_lv2_new(AgsChannel *source,
		   AgsTurtle *turtle,
		   gchar *filename,
		   gchar *effect,
		   gchar *uri,
		   uint32_t index)
{
  GObject *soundcard;
  AgsRecallLv2 *recall_lv2;

  if(source != NULL){
    soundcard = AGS_AUDIO(source->audio)->soundcard;
  }else{
    soundcard = NULL;
  }

  recall_lv2 = (AgsRecallLv2 *) g_object_new(AGS_TYPE_RECALL_LV2,
					     "soundcard\0", soundcard,
					     "turtle\0", turtle,
					     "source\0", source,
					     "filename\0", filename,
					     "effect\0", effect,
					     "uri\0", uri,
					     "index\0", index,
					     NULL);

  return(recall_lv2);
}
