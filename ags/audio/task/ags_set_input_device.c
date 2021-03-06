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

#include <ags/audio/task/ags_set_input_device.h>

#include <ags/object/ags_connectable.h>
#include <ags/object/ags_sequencer.h>

#include <ags/audio/ags_midiin.h>

void ags_set_input_device_class_init(AgsSetInputDeviceClass *set_input_device);
void ags_set_input_device_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_set_input_device_init(AgsSetInputDevice *set_input_device);
void ags_set_input_device_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_set_input_device_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_set_input_device_connect(AgsConnectable *connectable);
void ags_set_input_device_disconnect(AgsConnectable *connectable);
void ags_set_input_device_finalize(GObject *gobject);

void ags_set_input_device_launch(AgsTask *task);

/**
 * SECTION:ags_set_input_device
 * @short_description: resets sequencer
 * @title: AgsSetInputDevice
 * @section_id:
 * @include: ags/audio/task/ags_set_input_device.h
 *
 * The #AgsSetInputDevice task resets sequencer of #AgsSequencer.
 */

static gpointer ags_set_input_device_parent_class = NULL;
static AgsConnectableInterface *ags_set_input_device_parent_connectable_interface;

enum{
  PROP_0,
  PROP_SEQUENCER,
  PROP_DEVICE,
};

GType
ags_set_input_device_get_type()
{
  static GType ags_type_set_input_device = 0;

  if(!ags_type_set_input_device){
    static const GTypeInfo ags_set_input_device_info = {
      sizeof (AgsSetInputDeviceClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_set_input_device_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSetInputDevice),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_set_input_device_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_set_input_device_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_set_input_device = g_type_register_static(AGS_TYPE_TASK,
						       "AgsSetInputDevice\0",
						       &ags_set_input_device_info,
						       0);
    
    g_type_add_interface_static(ags_type_set_input_device,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }
  
  return (ags_type_set_input_device);
}

void
ags_set_input_device_class_init(AgsSetInputDeviceClass *set_input_device)
{
  GObjectClass *gobject;
  AgsTaskClass *task;
  GParamSpec *param_spec;

  ags_set_input_device_parent_class = g_type_class_peek_parent(set_input_device);

  /* gobject */
  gobject = (GObjectClass *) set_input_device;

  gobject->set_property = ags_set_input_device_set_property;
  gobject->get_property = ags_set_input_device_get_property;

  gobject->finalize = ags_set_input_device_finalize;

  /* properties */
  /**
   * AgsSetInputDevice:sequencer:
   *
   * The assigned #AgsSequencer instance.
   * 
   * Since: 0.7.111
   */
  param_spec = g_param_spec_object("sequencer\0",
				   "sequencer of set audio channels\0",
				   "The sequencer of set audio channels\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SEQUENCER,
				  param_spec);

  /**
   * AgsSetInputDevice:device:
   *
   * The sequencer indentifier
   * 
   * Since: 0.7.111
   */
  param_spec = g_param_spec_string("device\0",
				   "device identifier\0",
				   "The device identifier to set\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_DEVICE,
				  param_spec);

  /* task */
  task = (AgsTaskClass *) set_input_device;

  task->launch = ags_set_input_device_launch;
}

void
ags_set_input_device_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_set_input_device_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_set_input_device_connect;
  connectable->disconnect = ags_set_input_device_disconnect;
}

void
ags_set_input_device_init(AgsSetInputDevice *set_input_device)
{
  set_input_device->sequencer = NULL;
  set_input_device->device = NULL;
}

void
ags_set_input_device_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsSetInputDevice *set_input_device;

  set_input_device = AGS_SET_INPUT_DEVICE(gobject);

  switch(prop_id){
  case PROP_SEQUENCER:
    {
      GObject *sequencer;

      sequencer = (GObject *) g_value_get_object(value);

      if(set_input_device->sequencer == (GObject *) sequencer){
	return;
      }

      if(set_input_device->sequencer != NULL){
	g_object_unref(set_input_device->sequencer);
      }

      if(sequencer != NULL){
	g_object_ref(sequencer);
      }

      set_input_device->sequencer = (GObject *) sequencer;
    }
    break;
  case PROP_DEVICE:
    {
      char *device;

      device = (char *) g_value_get_string(value);

      if(device == set_input_device->device){
	return;
      }

      if(set_input_device->device != NULL){
	g_free(set_input_device->device);
      }

      set_input_device->device = g_strdup(device);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_set_input_device_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsSetInputDevice *set_input_device;

  set_input_device = AGS_SET_INPUT_DEVICE(gobject);

  switch(prop_id){
  case PROP_SEQUENCER:
    {
      g_value_set_object(value, set_input_device->sequencer);
    }
    break;
  case PROP_DEVICE:
    {
      g_value_set_string(value, set_input_device->device);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_set_input_device_connect(AgsConnectable *connectable)
{
  ags_set_input_device_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_set_input_device_disconnect(AgsConnectable *connectable)
{
  ags_set_input_device_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

void
ags_set_input_device_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_set_input_device_parent_class)->finalize(gobject);

  /* empty */
}

void
ags_set_input_device_launch(AgsTask *task)
{
  GObject *sequencer;
  AgsSetInputDevice *set_input_device;
  char *device;

  set_input_device = AGS_SET_INPUT_DEVICE(task);

  sequencer = set_input_device->sequencer;
  device = set_input_device->device;

  if(AGS_IS_MIDIIN(sequencer) &&
     (AGS_MIDIIN_ALSA & (AGS_MIDIIN(sequencer)->flags)) != 0){
    if(index(device, ',') == NULL){
      gchar *tmp;
      
      tmp = g_strdup_printf("%s,0\0",
			    device);
      
      g_free(device);
      device = tmp;
    }
  }
  
  /* perform task */
  ags_sequencer_set_device(AGS_SEQUENCER(sequencer),
			   device);
}

/**
 * ags_set_input_device_new:
 * @sequencer: the #AgsSequencer to reset
 * @device: the new sequencer
 *
 * Creates an #AgsSetInputDevice.
 *
 * Returns: an new #AgsSetInputDevice.
 *
 * Since: 0.7.111
 */
AgsSetInputDevice*
ags_set_input_device_new(GObject *sequencer,
			 char *device)
{
  AgsSetInputDevice *set_input_device;

  set_input_device = (AgsSetInputDevice *) g_object_new(AGS_TYPE_SET_INPUT_DEVICE,
							NULL);

  set_input_device->sequencer = sequencer;
  set_input_device->device = device;

  return(set_input_device);
}
