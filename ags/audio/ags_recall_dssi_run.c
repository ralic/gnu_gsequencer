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
#include <ags/audio/ags_recall_dssi_run.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_plugin.h>

#include <ags/plugin/ags_dssi_manager.h>

#include <ags/audio/ags_port.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void ags_recall_dssi_run_class_init(AgsRecallDssiRunClass *recall_dssi_run_class);
void ags_recall_dssi_run_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_recall_dssi_run_plugin_interface_init(AgsPluginInterface *plugin);
void ags_recall_dssi_run_init(AgsRecallDssiRun *recall_dssi_run);
void ags_recall_dssi_run_connect(AgsConnectable *connectable);
void ags_recall_dssi_run_disconnect(AgsConnectable *connectable);
void ags_recall_dssi_run_finalize(GObject *gobject);

void ags_recall_dssi_run_run_init_pre(AgsRecall *recall);
void ags_recall_dssi_run_run_pre(AgsRecall *recall);
void ags_recall_dssi_run_run_inter(AgsRecall *recall);
void ags_recall_dssi_run_run_post(AgsRecall *recall);

void ags_recall_dssi_run_load_ports(AgsRecallDssiRun *recall_dssi_run);

/**
 * SECTION:ags_recall_dssi_run
 * @Short_description: The object interfacing with DSSI
 * @Title: AgsRecallDssiRun
 *
 * #AgsRecallDssiRun provides DSSI support.
 */

static gpointer ags_recall_dssi_run_parent_class = NULL;
static AgsConnectableInterface* ags_recall_dssi_run_parent_connectable_interface;

GType
ags_recall_dssi_run_get_type (void)
{
  static GType ags_type_recall_dssi_run = 0;

  if(!ags_type_recall_dssi_run){
    static const GTypeInfo ags_recall_dssi_run_info = {
      sizeof (AgsRecallDssiRunClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_recall_dssi_run_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsRecallDssiRun),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_recall_dssi_run_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_recall_dssi_run_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_plugin_interface_info = {
      (GInterfaceInitFunc) ags_recall_dssi_run_plugin_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_recall_dssi_run = g_type_register_static(AGS_TYPE_RECALL_AUDIO_SIGNAL,
						      "AgsRecallDssiRun\0",
						      &ags_recall_dssi_run_info,
						      0);

    g_type_add_interface_static(ags_type_recall_dssi_run,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_recall_dssi_run,
				AGS_TYPE_PLUGIN,
				&ags_plugin_interface_info);
  }

  return(ags_type_recall_dssi_run);
}

void
ags_recall_dssi_run_class_init(AgsRecallDssiRunClass *recall_dssi_run)
{
  GObjectClass *gobject;
  AgsRecallClass *recall;
  GParamSpec *param_spec;

  ags_recall_dssi_run_parent_class = g_type_class_peek_parent(recall_dssi_run);

  /* GObjectClass */
  gobject = (GObjectClass *) recall_dssi_run;

  gobject->finalize = ags_recall_dssi_run_finalize;

  /* AgsRecallClass */
  recall = (AgsRecallClass *) recall_dssi_run;

  recall->run_init_pre = ags_recall_dssi_run_run_init_pre;
  recall->run_pre = ags_recall_dssi_run_run_pre;
  recall->run_inter = ags_recall_dssi_run_run_inter;
  recall->run_post = ags_recall_dssi_run_run_post;
}


void
ags_recall_dssi_run_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_recall_dssi_run_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_recall_dssi_run_connect;
  connectable->disconnect = ags_recall_dssi_run_disconnect;
}

void
ags_recall_dssi_run_plugin_interface_init(AgsPluginInterface *plugin)
{
  //TODO:JK: implement me
}

void
ags_recall_dssi_run_init(AgsRecallDssiRun *recall_dssi_run)
{
  recall_dssi_run->ladspa_handle = NULL;

  recall_dssi_run->input = NULL;
  recall_dssi_run->output = NULL;

  recall_dssi_run->delta_time = 0;
  
  recall_dssi_run->event_buffer = NULL;
  recall_dssi_run->event_count = NULL;

  recall_dssi_run->note = NULL;
}

void
ags_recall_dssi_run_connect(AgsConnectable *connectable)
{
  ags_recall_dssi_run_parent_connectable_interface->connect(connectable);
}

void
ags_recall_dssi_run_disconnect(AgsConnectable *connectable)
{
  ags_recall_dssi_run_parent_connectable_interface->disconnect(connectable);
}

void
ags_recall_dssi_run_finalize(GObject *gobject)
{
  AgsRecallDssi *recall_dssi;
  AgsRecallDssiRun *recall_dssi_run;
  unsigned long i;

  recall_dssi_run = AGS_RECALL_DSSI_RUN(gobject);

  free(recall_dssi_run->output);
  free(recall_dssi_run->input);

  /* call parent */
  G_OBJECT_CLASS(ags_recall_dssi_run_parent_class)->finalize(gobject);
}

void
ags_recall_dssi_run_run_init_pre(AgsRecall *recall)
{
  AgsRecallDssi *recall_dssi;
  AgsRecallDssiRun *recall_dssi_run;
  AgsAudioSignal *audio_signal;
  unsigned long samplerate;
  unsigned long buffer_size;
  unsigned long i, i_stop;

  /* call parent */
  AGS_RECALL_CLASS(ags_recall_dssi_run_parent_class)->run_init_pre(recall);

  recall_dssi_run = AGS_RECALL_DSSI_RUN(recall);
  recall_dssi = AGS_RECALL_DSSI(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->recall_channel);
  
  /* set up buffer */
  audio_signal = AGS_RECALL_AUDIO_SIGNAL(recall_dssi_run)->source;
  
  /* set up buffer */ 
  samplerate = audio_signal->samplerate;
  buffer_size = audio_signal->buffer_size;

  if(recall_dssi->input_lines > 0){
    recall_dssi_run->input = (LADSPA_Data *) malloc(recall_dssi->input_lines *
						    buffer_size *
						    sizeof(LADSPA_Data));
  }
  
  recall_dssi_run->output = (LADSPA_Data *) malloc(recall_dssi->output_lines *
						   buffer_size *
						   sizeof(LADSPA_Data));

  if(recall_dssi->input_lines < recall_dssi->output_lines){
    i_stop = recall_dssi->output_lines;
  }else{
    i_stop = recall_dssi->input_lines;
  }

  recall_dssi_run->ladspa_handle = (LADSPA_Handle *) malloc(i_stop *
							    sizeof(LADSPA_Handle));

  for(i = 0; i < i_stop; i++){
    /* instantiate dssi */
    recall_dssi_run->ladspa_handle[i] = (LADSPA_Handle *) recall_dssi->plugin_descriptor->LADSPA_Plugin->instantiate(recall_dssi->plugin_descriptor,
														     samplerate);

#ifdef AGS_DEBUG
    g_message("instantiate DSSI handle\0");
#endif

  }

  ags_recall_dssi_run_load_ports(recall_dssi_run);

  /* can't be done in ags_recall_dssi_run_run_init_inter since possebility of overlapping buffers */
  /* connect audio port */
  for(i = 0; i < recall_dssi->input_lines; i++){
    recall_dssi->plugin_descriptor->LADSPA_Plugin->connect_port(recall_dssi_run->ladspa_handle[i],
								recall_dssi->input_port[i],
								&(recall_dssi_run->input[i]));
  }
  
  for(i = 0; i < recall_dssi->output_lines; i++){
    recall_dssi->plugin_descriptor->LADSPA_Plugin->connect_port(recall_dssi_run->ladspa_handle[i],
								recall_dssi->output_port[i],
								&(recall_dssi_run->output[i]));
  }


  for(i = 0; i < i_stop; i++){
    if(recall_dssi->plugin_descriptor->select_program != NULL){
      recall_dssi->plugin_descriptor->select_program(recall_dssi_run->ladspa_handle[i],
						     recall_dssi->bank,
						     recall_dssi->program);
    }

    if(recall_dssi->plugin_descriptor->LADSPA_Plugin->activate != NULL){
      recall_dssi->plugin_descriptor->LADSPA_Plugin->activate(recall_dssi_run->ladspa_handle[i]);
    }
    
#ifdef AGS_DEBUG
    g_message("instantiate DSSI handle\0");
#endif

  }
}

void
ags_recall_dssi_run_run_pre(AgsRecall *recall)
{
  //empty
}

void
ags_recall_dssi_run_run_inter(AgsRecall *recall)
{
  AgsRecallDssi *recall_dssi;
  AgsRecallDssiRun *recall_dssi_run;
  AgsAudioSignal *audio_signal;

  snd_midi_event_t **event_buffer;
  unsigned long *event_count;
  
  unsigned long buffer_size;
  unsigned long i, i_stop;

  /* call parent */
  AGS_RECALL_CLASS(ags_recall_dssi_run_parent_class)->run_inter(recall);
  
  recall_dssi = AGS_RECALL_DSSI(AGS_RECALL_CHANNEL_RUN(recall->parent->parent)->recall_channel);
  recall_dssi_run = AGS_RECALL_DSSI_RUN(recall);

  /* set up buffer */
  audio_signal = AGS_RECALL_AUDIO_SIGNAL(recall_dssi_run)->source;
  buffer_size = audio_signal->buffer_size;

  g_message("%d\0", buffer_size);
  
  memset(recall_dssi_run->output, 0, recall_dssi->output_lines * buffer_size * sizeof(LADSPA_Data));
  memset(recall_dssi_run->input, 0, recall_dssi->input_lines * buffer_size * sizeof(LADSPA_Data));

  if(recall_dssi->input_lines < recall_dssi->output_lines){
    i_stop = recall_dssi->output_lines;
  }else{
    i_stop = recall_dssi->input_lines;
  }

  if(audio_signal->stream_current == NULL){
    for(i = 0; i < i_stop; i++){
      /* deactivate */
      //TODO:JK: fix-me
      if(recall_dssi->plugin_descriptor->LADSPA_Plugin->deactivate != NULL){
	recall_dssi->plugin_descriptor->LADSPA_Plugin->deactivate(recall_dssi_run->ladspa_handle[i]);
      }
      
      recall_dssi->plugin_descriptor->LADSPA_Plugin->cleanup(recall_dssi_run->ladspa_handle[i]);
    }

    ags_recall_done(recall);
    return;
  }

  if(recall_dssi_run->input != NULL){
    ags_recall_dssi_short_to_float(audio_signal->stream_current->data,
				   recall_dssi_run->input,
				   (guint) audio_signal->buffer_size, (guint) recall_dssi->input_lines);
  }
  
  /* process data */
  for(i = 0; i < i_stop; i++){
    if(recall_dssi->plugin_descriptor->run_synth != NULL){
      if(recall_dssi_run->event_buffer != NULL){
	event_buffer = recall_dssi_run->event_buffer;
	event_count = recall_dssi_run->event_count;
      
	while(*event_buffer != NULL){
	  recall_dssi->plugin_descriptor->run_synth(recall_dssi_run->ladspa_handle[i],
						    buffer_size,
						    *(event_buffer),
						    *(event_count));
	  
	  event_buffer++;
	  event_count++;
	}
      }
    }else if(recall_dssi->plugin_descriptor->LADSPA_Plugin->run != NULL){
      recall_dssi->plugin_descriptor->LADSPA_Plugin->run(recall_dssi_run->ladspa_handle[i],
							 buffer_size);
    }
  }

  /* copy data */
  memset((signed short *) audio_signal->stream_current->data,
	 0,
	 buffer_size * sizeof(signed short));
  ags_recall_dssi_float_to_short(recall_dssi_run->output,
				 audio_signal->stream_current->data,
				 (guint) audio_signal->buffer_size, (guint) recall_dssi->output_lines);
}

void
ags_recall_dssi_run_run_post(AgsRecall *recall)
{
  AgsRecallDssiRun *recall_dssi_run;

  recall_dssi_run = AGS_RECALL_DSSI_RUN(recall);
}

/**
 * ags_recall_dssi_run_load_ports:
 * @recall_dssi_run: an #AgsRecallDssiRun
 *
 * Set up DSSI ports.
 *
 * Since: 0.7.0
 */
void
ags_recall_dssi_run_load_ports(AgsRecallDssiRun *recall_dssi_run)
{
  AgsRecallDssi *recall_dssi;
  AgsDssiPlugin *dssi_plugin;
  AgsPort *current;
  GList *port;
  gchar *path;
  unsigned long port_count;
  unsigned long i, j, j_stop;

  DSSI_Descriptor *plugin_descriptor;
  LADSPA_PortDescriptor *port_descriptor;
  LADSPA_PortRangeHintDescriptor hint_descriptor;

  recall_dssi = AGS_RECALL_DSSI(AGS_RECALL_CHANNEL_RUN(AGS_RECALL(recall_dssi_run)->parent->parent)->recall_channel);

  port = AGS_RECALL(recall_dssi)->port;
  
  plugin_descriptor = recall_dssi->plugin_descriptor;

  port_count = plugin_descriptor->LADSPA_Plugin->PortCount;
  port_descriptor = plugin_descriptor->LADSPA_Plugin->PortDescriptors;

  if(recall_dssi->input_lines < recall_dssi->output_lines){
    j_stop = recall_dssi->output_lines;
  }else{
    j_stop = recall_dssi->input_lines;
  }

  for(i = 0; i < port_count; i++){
    if(LADSPA_IS_PORT_CONTROL(port_descriptor[i])){
      if(LADSPA_IS_PORT_INPUT(port_descriptor[i]) ||
	 LADSPA_IS_PORT_OUTPUT(port_descriptor[i])){
	GList *list;
	LADSPA_Data *port_data;
	gchar *plugin_name;
	gchar *specifier;

	hint_descriptor = plugin_descriptor->LADSPA_Plugin->PortRangeHints[i].HintDescriptor;
	specifier = plugin_descriptor->LADSPA_Plugin->PortNames[i];

	list = port;

	while(list != NULL){
	  current = port->data;

	  if(!g_strcmp0(specifier,
			current->specifier)){
	    break;
	  }

	  list = list->next;
	}

	for(j = 0; j < j_stop; j++){
	  g_message("connecting port[%d]: %d/%d\0", j, i, port_count);
	  port_data = (LADSPA_Data *) &(current->port_value.ags_port_ladspa);
	  recall_dssi->plugin_descriptor->LADSPA_Plugin->connect_port(recall_dssi_run->ladspa_handle[j],
								      i,
								      port_data);
	}
      }
    }
  }
}

/**
 * ags_recall_dssi_run_new:
 * @audio_signal: the source
 *
 * Creates a #AgsRecallDssiRun
 *
 * Returns: a new #AgsRecallDssiRun
 *
 * Since: 0.7.0
 */
AgsRecallDssiRun*
ags_recall_dssi_run_new(AgsAudioSignal *audio_signal)
{
  AgsRecallDssiRun *recall_dssi_run;

  recall_dssi_run = (AgsRecallDssiRun *) g_object_new(AGS_TYPE_RECALL_DSSI_RUN,
						      "source\0", audio_signal,
						      NULL);

  return(recall_dssi_run);
}
