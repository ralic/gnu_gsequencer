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

#include <ags/X/ags_export_soundcard.h>
#include <ags/X/ags_export_soundcard_callbacks.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_soundcard.h>

void ags_export_soundcard_class_init(AgsExportSoundcardClass *export_soundcard);
void ags_export_soundcard_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_export_soundcard_init(AgsExportSoundcard *export_soundcard);
void ags_export_soundcard_set_property(GObject *gobject,
				       guint prop_id,
				       const GValue *value,
				       GParamSpec *param_spec);
void ags_export_soundcard_get_property(GObject *gobject,
				       guint prop_id,
				       GValue *value,
				       GParamSpec *param_spec);
void ags_export_soundcard_finalize(GObject *gobject);
void ags_export_soundcard_connect(AgsConnectable *connectable);
void ags_export_soundcard_disconnect(AgsConnectable *connectable);
void ags_export_soundcard_show(GtkWidget *widget);
gboolean ags_export_soundcard_delete_event(GtkWidget *widget, GdkEventAny *event);

/**
 * SECTION:ags_export_soundcard
 * @short_description: The export dialog
 * @title: AgsExportSoundcard
 * @section_id:
 * @include: ags/X/ags_export_soundcard.h
 *
 * #AgsExportSoundcard lets you export to audio files.
 */

enum{
  PROP_0,
  PROP_SOUNDCARD,
};

static gpointer ags_export_soundcard_parent_class = NULL;

GType
ags_export_soundcard_get_type()
{
  static GType ags_type_export_soundcard = 0;

  if(!ags_type_export_soundcard){
    static const GTypeInfo ags_export_soundcard_info = {
      sizeof (AgsExportSoundcardClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_export_soundcard_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsExportSoundcard),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_export_soundcard_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_export_soundcard_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_export_soundcard = g_type_register_static(GTK_TYPE_VBOX,
						       "AgsExportSoundcard\0", &ags_export_soundcard_info,
						       0);
    
    g_type_add_interface_static(ags_type_export_soundcard,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);
  }

  return(ags_type_export_soundcard);
}

void
ags_export_soundcard_class_init(AgsExportSoundcardClass *export_soundcard)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_export_soundcard_parent_class = g_type_class_peek_parent(export_soundcard);

  /* GObjectClass */
  gobject = (GObjectClass *) export_soundcard;

  gobject->set_property = ags_export_soundcard_set_property;
  gobject->get_property = ags_export_soundcard_get_property;

  gobject->finalize = ags_export_soundcard_finalize;

  /* properties */
  /**
   * AgsExportSoundcard:soundcard:
   *
   * The assigned #AgsSoundcard acting as default sink.
   * 
   * Since: 0.7.119
   */
  param_spec = g_param_spec_object("soundcard\0",
				   "assigned soundcard\0",
				   "The soundcard it is assigned with\0",
				   G_TYPE_OBJECT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_SOUNDCARD,
				  param_spec);
}

void
ags_export_soundcard_connectable_interface_init(AgsConnectableInterface *connectable)
{
  connectable->is_ready = NULL;
  connectable->is_connected = NULL;
  connectable->connect = ags_export_soundcard_connect;
  connectable->disconnect = ags_export_soundcard_disconnect;
}

void
ags_export_soundcard_init(AgsExportSoundcard *export_soundcard)
{
  GtkHBox *hbox;
  GtkTable *table;
  GtkLabel *label;

  export_soundcard->flags = 0;

  /* table */
  table = (GtkTable *) gtk_table_new(5, 2,
				     FALSE);
  gtk_box_pack_start(GTK_BOX(export_soundcard),
		     GTK_WIDGET(table),
		     FALSE, FALSE,
		     0);

  /* filename */
  label = (GtkLabel *) gtk_label_new("file\0");
  g_object_set(G_OBJECT(label),
	       "xalign\0", 0.0,
	       NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   0, 1,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  hbox = (GtkHBox *) gtk_hbox_new(FALSE,
				  0);
  gtk_table_attach(table,
		   GTK_WIDGET(hbox),
		   1, 2,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  export_soundcard->filename = (GtkEntry *) gtk_entry_new();
  gtk_entry_set_text(export_soundcard->filename,
		     "out.wav\0");
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(export_soundcard->filename),
		     TRUE, TRUE,
		     0);

  export_soundcard->file_chooser_button = (GtkButton *) gtk_button_new_with_label("open\0");
  gtk_box_pack_start(GTK_BOX(hbox),
		     GTK_WIDGET(export_soundcard->file_chooser_button),
		     TRUE, TRUE,
		     0);

  /* output format */
  label = (GtkLabel *) gtk_label_new("output format\0");
  g_object_set(G_OBJECT(label),
	       "xalign\0", 0.0,
	       NULL);
  gtk_table_attach(table,
		   GTK_WIDGET(label),
		   0, 1,
		   4, 5,
		   GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND,
		   0, 0);

  export_soundcard->output_format = (GtkComboBoxText *) gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(export_soundcard->output_format,
				 "WAV\0");
  gtk_combo_box_set_active((GtkComboBox *) export_soundcard->output_format,
			   0);
  gtk_table_attach(table,
		   GTK_WIDGET(export_soundcard->output_format),
		   1, 2,
		   4, 5,
		   GTK_FILL, GTK_FILL,
		   0, 0);
}

void
ags_export_soundcard_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec)
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = AGS_EXPORT_SOUNDCARD(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      GObject *soundcard;

      soundcard = g_value_get_object(value);

      if(export_soundcard->soundcard == soundcard){
	return;
      }

      if(soundcard != NULL){
	g_object_ref(soundcard);
      }
      
      export_soundcard->soundcard = soundcard;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_export_soundcard_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec)
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = AGS_EXPORT_SOUNDCARD(gobject);

  switch(prop_id){
  case PROP_SOUNDCARD:
    {
      g_value_set_object(value, export_soundcard->soundcard);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_export_soundcard_connect(AgsConnectable *connectable)
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = AGS_EXPORT_SOUNDCARD(connectable);

  if((AGS_EXPORT_SOUNDCARD_CONNECTED & (export_soundcard->flags)) != 0){
    return;
  }

  export_soundcard->flags |= AGS_EXPORT_SOUNDCARD_CONNECTED;
  
  g_signal_connect_after(G_OBJECT(export_soundcard->file_chooser_button), "clicked\0",
			 G_CALLBACK(ags_export_soundcard_file_chooser_button_callback), export_soundcard);
}

void
ags_export_soundcard_disconnect(AgsConnectable *connectable)
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = AGS_EXPORT_SOUNDCARD(connectable);

  if((AGS_EXPORT_SOUNDCARD_CONNECTED & (export_soundcard->flags)) == 0){
    return;
  }

  export_soundcard->flags &= (~AGS_EXPORT_SOUNDCARD_CONNECTED);
  
  g_object_disconnect(G_OBJECT(export_soundcard->file_chooser_button),
		      "clicked\0",
		      G_CALLBACK(ags_export_soundcard_file_chooser_button_callback),
		      export_soundcard,
		      NULL);
}

void
ags_export_soundcard_finalize(GObject *gobject)
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = (AgsExportSoundcard *) gobject;

  if(export_soundcard->soundcard != NULL){
    g_object_unref(export_soundcard->soundcard);
  }
  
  G_OBJECT_CLASS(ags_export_soundcard_parent_class)->finalize(gobject);
}

/**
 * ags_export_soundcard_new:
 * 
 * Instantiate an #AgsExportSoundcard
 * 
 * Returns: an #AgsExportSoundcard
 * 
 * Since: 0.7.119
 */
AgsExportSoundcard*
ags_export_soundcard_new()
{
  AgsExportSoundcard *export_soundcard;

  export_soundcard = (AgsExportSoundcard *) g_object_new(AGS_TYPE_EXPORT_SOUNDCARD,
							 NULL);

  return(export_soundcard);
}
