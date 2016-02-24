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

#include <ags/plugin/ags_dssi_plugin.h>

void ags_dssi_plugin_class_init(AgsDssiPluginClass *dssi_plugin);
void ags_dssi_plugin_init (AgsDssiPlugin *dssi_plugin);
void ags_dssi_plugin_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_dssi_plugin_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_dssi_plugin_finalize(GObject *gobject);

gpointer ags_dssi_plugin_instantiate(AgsBasePlugin *base_plugin);
void ags_dssi_plugin_connect_port(AgsBasePlugin *base_plugin,
				  gpointer plugin_handle,
				  guint port_index,
				  gpointer data_location);
void ags_dssi_plugin_activate(AgsBasePlugin *base_plugin,
			      gpointer plugin_handle);
void ags_dssi_plugin_deactivate(AgsBasePlugin *base_plugin,
				gpointer plugin_handle);
void ags_dssi_plugin_load_plugin(AgsBasePlugin *base_plugin);

/**
 * SECTION:ags_dssi_plugin
 * @short_description: The dssi plugin class
 * @title: AgsDssiPlugin
 * @section_id:
 * @include: ags/object/ags_dssi_plugin.h
 *
 * The #AgsDssiPlugin loads/unloads a Dssi plugin.
 */

enum{
  PROP_0,
  PROP_PROGRAM,
};

static gpointer ags_dssi_plugin_parent_class = NULL;

GType
ags_dssi_plugin_get_type (void)
{
  static GType ags_type_dssi_plugin = 0;

  if(!ags_type_dssi_plugin){
    static const GTypeInfo ags_dssi_plugin_info = {
      sizeof (AgsDssiPluginClass),
      NULL, /* dssi_init */
      NULL, /* dssi_finalize */
      (GClassInitFunc) ags_dssi_plugin_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsDssiPlugin),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_dssi_plugin_init,
    };

    ags_type_dssi_plugin = g_type_register_static(AGS_TYPE_BASE_PLUGIN,
						  "AgsDssiPlugin\0",
						  &ags_dssi_plugin_info,
						  0);
  }

  return (ags_type_dssi_plugin);
}

void
ags_dssi_plugin_class_init(AgsDssiPluginClass *dssi_plugin)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_dssi_plugin_parent_class = g_type_class_peek_parent(dssi_plugin);

  /* GObjectClass */
  gobject = (GObjectClass *) dssi_plugin;

  gobject->set_property = ags_dssi_plugin_set_property;
  gobject->get_property = ags_dssi_plugin_get_property;

  gobject->finalize = ags_dssi_plugin_finalize;
}

void
ags_dssi_plugin_init(AgsDssiPlugin *dssi_plugin)
{
  dssi_plugin->flags = 0;
}

void
ags_dssi_plugin_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsDssiPlugin *dssi_plugin;

  dssi_plugin = AGS_DSSI_PLUGIN(gobject);

  switch(prop_id){
  case PROP_PROGRAM:
    {
      gchar *program;

      program = (gchar *) g_value_get_string(value);

      if(dssi_plugin->program == program){
	return;
      }
      
      if(dssi_plugin->program != NULL){
	g_free(dssi_plugin->program);
      }

      dssi_plugin->program = g_strdup(program);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_dssi_plugin_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsDssiPlugin *dssi_plugin;

  dssi_plugin = AGS_DSSI_PLUGIN(gobject);

  switch(prop_id){
  case PROP_PROGRAM:
    g_value_set_string(value, dssi_plugin->program);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_dssi_plugin_finalize(GObject *gobject)
{
  AgsDssiPlugin *dssi_plugin;

  dssi_plugin = AGS_DSSI_PLUGIN(gobject);
}

gpointer
ags_dssi_plugin_instantiate(AgsBasePlugin *base_plugin)
{
  //TODO:JK: implement me
}

void
ags_dssi_plugin_connect_port(AgsBasePlugin *base_plugin,
			     gpointer plugin_handle,
			     guint port_index,
			     gpointer data_location)
{
  //TODO:JK: implement me
}

void
ags_dssi_plugin_activate(AgsBasePlugin *base_plugin,
			 gpointer plugin_handle)
{
  //TODO:JK: implement me
}

void
ags_dssi_plugin_deactivate(AgsBasePlugin *base_plugin,
			   gpointer plugin_handle)
{
  //TODO:JK: implement me
}

void
ags_dssi_plugin_load_plugin(AgsBasePlugin *base_plugin)
{
}

/**
 * ags_dssi_plugin_new:
 * @filename: the plugin .so
 * @effect: the effect's string representation
 * @effect_index: the effect's index
 *
 * Creates an #AgsDssiPlugin
 *
 * Returns: a new #AgsDssiPlugin
 *
 * Since: 0.7.6
 */
AgsDssiPlugin*
ags_dssi_plugin_new(gchar *filename, gchar *effect, guint effect_index)
{
  AgsDssiPlugin *dssi_plugin;

  dssi_plugin = (AgsDssiPlugin *) g_object_new(AGS_TYPE_DSSI_PLUGIN,
					       "filename\0", filename,
					       "effect\0", effect,
					       "effect-index\0", effect_index,
					       NULL);

  return(dssi_plugin);
}
