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

#include <ags/audio/ags_recall_dssi.h>

#include <ags/object/ags_connectable.h>

#include <ags/util/ags_id_generator.h>

#include <ags/plugin/ags_dssi_manager.h>

#include <ags/object/ags_plugin.h>

#include <ags/file/ags_file.h>
#include <ags/file/ags_file_stock.h>
#include <ags/file/ags_file_id_ref.h>

#include <ags/object/ags_config.h>
#include <ags/object/ags_soundcard.h>
#include <ags/audio/ags_port.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <libxml/tree.h>

void ags_recall_dssi_class_init(AgsRecallDssiClass *recall_dssi_class);
void ags_recall_dssi_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_dssi_plugin_interface_init(AgsPluginInterface *plugin);
void ags_recall_dssi_init(AgsRecallDssi *recall_dssi);
void ags_recall_dssi_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_recall_dssi_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_recall_dssi_connect(AgsConnectable *connectable);
void ags_recall_dssi_disconnect(AgsConnectable *connectable);
void ags_recall_dssi_set_ports(AgsPlugin *plugin, GList *port);
void ags_recall_dssi_finalize(GObject *gobject);

void ags_recall_dssi_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin);
xmlNode* ags_recall_dssi_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin);

void ags_recall_dssi_load(AgsRecallDssi *recall_dssi);

/**
 * SECTION:ags_recall_dssi
 * @short_description: The object interfacing with DSSI
 * @title: AgsRecallDssi
 * @section_id:
 * @include: ags/audio/ags_recall_dssi.h
 *
 * #AgsRecallDssi provides DSSI support.
 */

enum{
  PROP_0,
  PROP_FILENAME,
  PROP_EFFECT,
  PROP_INDEX,
};

static gpointer ags_recall_dssi_parent_class = NULL;
static AgsConnectableInterface* ags_recall_dssi_parent_connectable_interface;
static AgsPluginInterface* ags_recall_dssi_parent_plugin_interface;

GType
ags_recall_dssi_get_type (void)
{
  static GType ags_type_recall_dssi = 0;

  if(!ags_type_recall_dssi){
    static const GTypeInfo ags_recall_dssi_info = {
      sizeof (AgsRecallDssiClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_dssi_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallDssi),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_dssi_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_dssi_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_recall_dssi_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_dssi = g_type_register_static(AGS_TYPE_RECALL_CHANNEL,
						  "AgsRecallDssi\0",
						  &ags_recall_dssi_info,
						  0);

    g_type_add_interface_static(ags_type_recall_dssi,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_recall_dssi,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_recall_dssi);
}

void
ags_recall_dssi_class_init(AgsRecallDssiClass *recall_dssi)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_recall_dssi_parent_class = g_type_class_peek_parent(recall_dssi);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_dssi;

  gobject->set_property = ags_recall_dssi_set_property;
  gobject->get_property = ags_recall_dssi_get_property;

  gobject->finalize = ags_recall_dssi_finalize;

  /* properties */
  /**
   * AgsRecallDssi:filename:
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
   * AgsRecallDssi:recycling:
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
   * AgsRecallDssi:recycling:
   *
   * The effect's index.
   * 
   * Since: 0.7.0
   */
  param_spec =  g_param_spec_ulong("index\0",
				   "index of effect\0",
				   "The numerical index of effect\0",
				   0,
				   65535,
				   0,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_INDEX,
				  param_spec);
}


void
ags_recall_dssi_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recall_dssi_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_dssi_connect;
  connectable->disconnect = ags_recall_dssi_disconnect;
}

void
ags_recall_dssi_plugin_interface_init(AgsPluginInterface *plugin)
{
  ags_recall_dssi_parent_plugin_interface = g_type_interface_peek_parent(plugin);

  plugin->read = ags_recall_dssi_read;
  plugin->write = ags_recall_dssi_write;
  plugin->set_ports = ags_recall_dssi_set_ports;
}

void
ags_recall_dssi_init(AgsRecallDssi *recall_dssi)
{
  AGS_RECALL(recall_dssi)->name = "ags-dssi\0";
  AGS_RECALL(recall_dssi)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(recall_dssi)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(recall_dssi)->xml_type = "ags-recall-dssi\0";
  AGS_RECALL(recall_dssi)->port = NULL;

  recall_dssi->filename = NULL;
  recall_dssi->effect = NULL;
  recall_dssi->index = 0;

  recall_dssi->plugin_descriptor = NULL;

  recall_dssi->input_port = NULL;
  recall_dssi->input_lines = 0;

  recall_dssi->output_port = NULL;
  recall_dssi->output_lines = 0;
}

void
ags_recall_dssi_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsRecallDssi *recall_dssi;

  recall_dssi = AGS_RECALL_DSSI(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      GObject *soundcard;
      gchar *filename;

      filename = g_value_get_string(value);

      if(filename == recall_dssi->filename){
	return;
      }

      if(recall_dssi->filename != NULL){
	g_free(recall_dssi->filename);
      }

      recall_dssi->filename = g_strdup(filename);
    }
    break;
  case PROP_EFFECT:
    {
      gchar *effect;
      
      effect = g_value_get_string(value);

      if(effect == recall_dssi->effect){
	return;
      }

      recall_dssi->effect = g_strdup(effect);
    }
    break;
  case PROP_INDEX:
    {
      unsigned long index;
      
      index = g_value_get_ulong(value);

      if(index == recall_dssi->index){
	return;
      }

      recall_dssi->index = index;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  };
}

void
ags_recall_dssi_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsRecallDssi *recall_dssi;

  recall_dssi = AGS_RECALL_DSSI(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      g_value_set_string(value, recall_dssi->filename);
    }
    break;
  case PROP_EFFECT:
    {
      g_value_set_string(value, recall_dssi->effect);
    }
    break;
  case PROP_INDEX:
    {
      g_value_set_uint(value, recall_dssi->index);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_recall_dssi_connect(AgsConnectable *connectable)
{
  ags_recall_dssi_parent_connectable_interface->connect(connectable);
}

void
ags_recall_dssi_disconnect(AgsConnectable *connectable)
{
  ags_recall_dssi_parent_connectable_interface->disconnect(connectable);
}

void
ags_recall_dssi_set_ports(AgsPlugin *plugin, GList *port)
{
  AgsRecallDssi *recall_dssi;
  AgsDssiPlugin *dssi_plugin;
  AgsPort *current;
  GList *list;
  gchar *path;
  unsigned long port_count;
  unsigned long i;

  void *plugin_so;
  LADSPA_Descriptor_Function dssi_descriptor;
  LADSPA_Descriptor *plugin_descriptor;
  LADSPA_PortDescriptor *port_descriptor;
  LADSPA_PortRangeHintDescriptor hint_descriptor;

  recall_dssi = AGS_RECALL_DSSI(plugin);

  ags_dssi_manager_load_file(recall_dssi->filename);
  dssi_plugin = ags_dssi_manager_find_dssi_plugin(recall_dssi->filename);
  
  plugin_so = dssi_plugin->plugin_so;

  if(plugin_so){
    dssi_descriptor = (LADSPA_Descriptor_Function) dlsym(plugin_so,
						       "dssi_descriptor\0");

    if(dlerror() == NULL && dssi_descriptor){
      recall_dssi->plugin_descriptor = 
	plugin_descriptor = dssi_descriptor(recall_dssi->index);

      port_count = plugin_descriptor->PortCount;
      port_descriptor = plugin_descriptor->PortDescriptors;

      for(i = 0; i < port_count; i++){
	if(LADSPA_IS_PORT_CONTROL(port_descriptor[i])){
	  if(LADSPA_IS_PORT_INPUT(port_descriptor[i]) ||
	     LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	    gchar *plugin_name;
	    gchar *specifier;

	    hint_descriptor = plugin_descriptor->PortRangeHints[i].HintDescriptor;

	    plugin_name = g_strdup_printf("dssi-%lu\0", plugin_descriptor->UniqueID);
	    specifier = g_strdup(plugin_descriptor->PortNames[i]);

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
	    
	    current->port_value.ags_port_float = plugin_descriptor->PortRangeHints[i].LowerBound;

	    g_message("connecting port: %d/%d\0", i, port_count);
	  }
	}else if(LADSPA_IS_PORT_AUDIO(port_descriptor[i])){
	  if(LADSPA_IS_PORT_INPUT(port_descriptor[i])){
	    if(recall_dssi->input_port == NULL){
	      recall_dssi->input_port = (unsigned long *) malloc(sizeof(unsigned long));
	      recall_dssi->input_port[0] = i;
	    }else{
	      recall_dssi->input_port = (unsigned long *) realloc(recall_dssi->input_port,
								  (recall_dssi->input_lines + 1) * sizeof(unsigned long));
	      recall_dssi->input_port[recall_dssi->input_lines] = i;
	    }

	    recall_dssi->input_lines += 1;
	  }else if(LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	    if(recall_dssi->output_port == NULL){
	      recall_dssi->output_port = (unsigned long *) malloc(sizeof(unsigned long));
	      recall_dssi->output_port[0] = i;
	    }else{
	      recall_dssi->output_port = (unsigned long *) realloc(recall_dssi->output_port,
								   (recall_dssi->output_lines + 1) * sizeof(unsigned long));
	      recall_dssi->output_port[recall_dssi->output_lines] = i;
	    }

	    recall_dssi->output_lines += 1;
	  }
	}
      }

      AGS_RECALL(recall_dssi)->port = g_list_reverse(port);
    }
  }
}

void
ags_recall_dssi_finalize(GObject *gobject)
{
  AgsRecallDssi *recall_dssi;
  
  recall_dssi = AGS_RECALL_DSSI(gobject);

  /* call parent */
  G_OBJECT_CLASS(ags_recall_dssi_parent_class)->finalize(gobject);
}

void
ags_recall_dssi_read(AgsFile *file, xmlNode *node, AgsPlugin *plugin)
{
  AgsRecallDssi *gobject;
  AgsDssiPlugin *dssi_plugin;
  gchar *filename, *effect;
  unsigned long index;

  gobject = AGS_RECALL_DSSI(plugin);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", xmlGetProp(node, AGS_FILE_ID_PROP)),
				   "reference\0", gobject,
				   NULL));

  filename = xmlGetProp(node,
			"filename\0");
  effect = xmlGetProp(node,
		      "effect\0");
  index = g_ascii_strtoull(xmlGetProp(node,
				      "index\0"),
			   NULL,
			   10);

  g_object_set(gobject,
	       "filename\0", filename,
	       "effect\0", effect,
	       "index\0", index,
	       NULL);

  ags_recall_dssi_load(gobject);
}

xmlNode*
ags_recall_dssi_write(AgsFile *file, xmlNode *parent, AgsPlugin *plugin)
{
  AgsRecallDssi *recall_dssi;
  xmlNode *node;
  gchar *id;

  recall_dssi = AGS_RECALL_DSSI(plugin);

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-recall-dssi\0");
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  ags_file_add_id_ref(file,
		      g_object_new(AGS_TYPE_FILE_ID_REF,
				   "main\0", file->application_context,
				   "file\0", file,
				   "node\0", node,
				   "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
				   "reference\0", recall_dssi,
				   NULL));

  xmlNewProp(node,
	     "filename\0",
	     g_strdup(recall_dssi->filename));

  xmlNewProp(node,
	     "effect\0",
	     g_strdup(recall_dssi->effect));

  xmlNewProp(node,
	     "index\0",
	     g_strdup_printf("%d\0", recall_dssi->index));

  xmlAddChild(parent,
	      node);

  return(node);
}

/**
 * ags_recall_dssi_load:
 * @recall_dssi: an #AgsRecallDssi
 *
 * Set up DSSI handle.
 * 
 * Since: 0.7.0
 */
void
ags_recall_dssi_load(AgsRecallDssi *recall_dssi)
{
  AgsDssiPlugin *dssi_plugin;

  void *plugin_so;
  LADSPA_Descriptor_Function dssi_descriptor;
  LADSPA_Descriptor *plugin_descriptor;

  /*  */
  ags_dssi_manager_load_file(recall_dssi->filename);
  dssi_plugin = ags_dssi_manager_find_dssi_plugin(recall_dssi->filename);
  
  plugin_so = dssi_plugin->plugin_so;

  if(plugin_so){
    dssi_descriptor = (LADSPA_Descriptor_Function) dlsym(plugin_so,
						       "dssi_descriptor\0");

    if(dlerror() == NULL && dssi_descriptor){
      recall_dssi->plugin_descriptor = 
	plugin_descriptor = dssi_descriptor(recall_dssi->index);
    }
  }
}

/**
 * ags_recall_dssi_load_ports:
 * @recall_dssi: an #AgsRecallDssi
 *
 * Set up DSSI ports.
 *
 * Returns: a #GList containing #AgsPort.
 * 
 * Since: 0.7.0
 */
GList*
ags_recall_dssi_load_ports(AgsRecallDssi *recall_dssi)
{
  AgsDssiPlugin *dssi_plugin;
  AgsPort *current;
  GList *port;
  gchar *path;
  unsigned long port_count;
  unsigned long i;

  void *plugin_so;
  LADSPA_Descriptor_Function dssi_descriptor;
  LADSPA_Descriptor *plugin_descriptor;
  LADSPA_PortDescriptor *port_descriptor;
  LADSPA_PortRangeHintDescriptor hint_descriptor;

  ags_dssi_manager_load_file(recall_dssi->filename);
  dssi_plugin = ags_dssi_manager_find_dssi_plugin(recall_dssi->filename);
  port = NULL;
  
  plugin_so = dssi_plugin->plugin_so;

  if(plugin_so){
    dssi_descriptor = (LADSPA_Descriptor_Function) dlsym(plugin_so,
						       "dssi_descriptor\0");

    if(dlerror() == NULL && dssi_descriptor){
      recall_dssi->plugin_descriptor = 
	plugin_descriptor = dssi_descriptor(recall_dssi->index);

      port_count = plugin_descriptor->PortCount;
      port_descriptor = plugin_descriptor->PortDescriptors;

      for(i = 0; i < port_count; i++){
	if(LADSPA_IS_PORT_CONTROL(port_descriptor[i])){
	  if(LADSPA_IS_PORT_INPUT(port_descriptor[i]) ||
	     LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	    gchar *plugin_name;
	    gchar *specifier;

	    hint_descriptor = plugin_descriptor->PortRangeHints[i].HintDescriptor;

	    plugin_name = g_strdup_printf("dssi-%lu\0", plugin_descriptor->UniqueID);
	    specifier = g_strdup(plugin_descriptor->PortNames[i]);

	    current = g_object_new(AGS_TYPE_PORT,
				   "plugin-name\0", plugin_name,
				   "specifier\0", specifier,
				   "control-port\0", g_strdup_printf("%d/%d\0",
								     i,
								     port_count),
				   "port-value-is-pointer\0", FALSE,
				   "port-value-type\0", G_TYPE_FLOAT,
				   NULL);
	    current->port_value.ags_port_float = plugin_descriptor->PortRangeHints[i].LowerBound;

	    g_message("connecting port: %d/%d\0", i, port_count);

	    port = g_list_prepend(port,
				  current);
	  }
	}else if(LADSPA_IS_PORT_AUDIO(port_descriptor[i])){
	  if(LADSPA_IS_PORT_INPUT(port_descriptor[i])){
	    if(recall_dssi->input_port == NULL){
	      recall_dssi->input_port = (unsigned long *) malloc(sizeof(unsigned long));
	      recall_dssi->input_port[0] = i;
	    }else{
	      recall_dssi->input_port = (unsigned long *) realloc(recall_dssi->input_port,
								  (recall_dssi->input_lines + 1) * sizeof(unsigned long));
	      recall_dssi->input_port[recall_dssi->input_lines] = i;
	    }

	    recall_dssi->input_lines += 1;
	  }else if(LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	    if(recall_dssi->output_port == NULL){
	      recall_dssi->output_port = (unsigned long *) malloc(sizeof(unsigned long));
	      recall_dssi->output_port[0] = i;
	    }else{
	      recall_dssi->output_port = (unsigned long *) realloc(recall_dssi->output_port,
								   (recall_dssi->output_lines + 1) * sizeof(unsigned long));
	      recall_dssi->output_port[recall_dssi->output_lines] = i;
	    }

	    recall_dssi->output_lines += 1;
	  }
	}
      }

      AGS_RECALL(recall_dssi)->port = g_list_reverse(port);
    }
  }

  return(AGS_RECALL(recall_dssi)->port);
}

/**
 * ags_recall_dssi_short_to_float:
 * @buffer: source
 * @destination: destination
 *
 * Convert data type.
 * 
 * Since: 0.7.0
 */
void
ags_recall_dssi_short_to_float(signed short *buffer,
			       LADSPA_Data *destination,
			       guint buffer_size, guint lines)
{
  LADSPA_Data *new_buffer;
  guint i;

  new_buffer = destination;

  for(i = 0; i < buffer_size; i++){
    new_buffer[lines * i] = (LADSPA_Data) (buffer[i] * (1.0f / 32767.5f));
  }
}

/**
 * ags_recall_dssi_float_to_short:
 * @buffer: source
 * @destination: destination
 * @buffer_size: buffer_size
 *
 * Convert data type.
 * 
 * Since: 0.7.0
 */
void
ags_recall_dssi_float_to_short(LADSPA_Data *buffer,
			       signed short *destination,
			       guint buffer_size, guint lines)
{
  signed short *new_buffer;
  guint i;

  new_buffer = destination;

  for(i = 0; i < buffer_size; i++){
    new_buffer[i] = (signed short) (buffer[lines * i] * 32767.5f);
  }
}

/**
 * ags_recall_dssi_find:
 * @recall: a #GList containing #AgsRecall
 * @filename: plugin filename
 * @effect: effect's name
 *
 * Retrieve DSSI recall.
 *
 * Returns: Next match.
 * 
 * Since: 0.7.0
 */
GList*
ags_recall_dssi_find(GList *recall,
		       gchar *filename, gchar *effect)
{
  while(recall != NULL){
    if(AGS_IS_RECALL_DSSI(recall->data)){
      if(!g_strcmp0(AGS_RECALL_DSSI(recall->data)->filename,
		    filename) &&
	 !g_strcmp0(AGS_RECALL_DSSI(recall->data)->effect,
		    effect)){
	return(recall);
      }
    }

    recall = recall->next;
  }

  return(NULL);
}

/**
 * ags_recall_dssi_new:
 * @source: the source
 * @filename: the DSSI plugin filename
 * @effect: effect's name
 * @index: effect's index
 *
 * Creates a #AgsRecallDssi
 *
 * Returns: a new #AgsRecallDssi
 * 
 * Since: 0.7.0
 */
AgsRecallDssi*
ags_recall_dssi_new(AgsChannel *source,
		    gchar *filename,
		    gchar *effect,
		    unsigned long index)
{
  GObject *soundcard;
  AgsRecallDssi *recall_dssi;

  if(source != NULL){
    soundcard = (GObject *) source->soundcard;
  }else{
    soundcard = NULL;
  }

  recall_dssi = (AgsRecallDssi *) g_object_new(AGS_TYPE_RECALL_DSSI,
					       "soundcard\0", soundcard,
					       "source\0", source,
					       "filename\0", filename,
					       "effect\0", effect,
					       "index\0", index,
					       NULL);

  return(recall_dssi);
}