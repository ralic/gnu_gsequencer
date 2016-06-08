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

#include <ags/X/file/ags_simple_file.h>

#include <ags/util/ags_id_generator.h>

#include <ags/object/ags_application_context.h>
#include <ags/object/ags_connectable.h>
#include <ags/object/ags_marshal.h>

#include <ags/file/ags_file_lookup.h>
#include <ags/file/ags_file_id_ref.h>
#include <ags/file/ags_file_launch.h>

#include <ags/X/ags_window.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>
#include <ags/X/ags_effect_pad.h>
#include <ags/X/ags_effect_line.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_ffplayer.h>
#include <ags/X/machine/ags_ladspa_bridge.h>
#include <ags/X/machine/ags_dssi_bridge.h>
#include <ags/X/machine/ags_lv2_bridge.h>

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

void ags_simple_file_class_init(AgsSimpleFileClass *simple_file);
void ags_simple_file_init(AgsSimpleFile *simple_file);
void ags_simple_file_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_simple_file_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_simple_file_finalize(GObject *gobject);

void ags_simple_file_real_open(AgsSimpleFile *simple_file,
			       GError **error);
void ags_simple_file_real_open_from_data(AgsSimpleFile *simple_file,
					 gchar *data, guint length,
					 GError **error);
void ags_simple_file_real_rw_open(AgsSimpleFile *simple_file,
				  gboolean create,
				  GError **error);

void ags_simple_file_real_write(AgsSimpleFile *simple_file);
void ags_simple_file_real_write_resolve(AgsSimpleFile *simple_file);

void ags_simple_file_real_read(AgsSimpleFile *simple_file);
void ags_simple_file_real_read_resolve(AgsSimpleFile *simple_file);
void ags_simple_file_real_read_start(AgsSimpleFile *simple_file);

void ags_simple_file_read_config(AgsFile *file, xmlNode *node, AgsConfig **config);
void ags_simple_file_read_property_list(AgsFile *file, xmlNode *node, GList **property);
void ags_simple_file_read_property(AgsFile *file, xmlNode *node, GParameter **property);
void ags_simple_file_read_window(AgsFile *file, xmlNode *node, AgsWindow **window);
void ags_simple_file_read_machine_list(AgsFile *file, xmlNode *node, GList **machine);
void ags_simple_file_read_machine(AgsFile *file, xmlNode *node, AgsMachine **machine);
void ags_simple_file_read_pad_list(AgsFile *file, xmlNode *node, GList **pad);
void ags_simple_file_read_pad(AgsFile *file, xmlNode *node, AgsPad **pad);
void ags_simple_file_read_line_list(AgsFile *file, xmlNode *node, GList **line);
void ags_simple_file_read_line(AgsFile *file, xmlNode *node, AgsLine **line);
void ags_simple_file_read_editor(AgsFile *file, xmlNode *node, AgsEditor **editor);
void ags_simple_file_read_automation_editor(AgsFile *file, xmlNode *node, AgsAutomationEditor **automation_editor);

xmlNode* ags_simple_file_write_window(AgsFile *file, xmlNode *parent, AgsConfig *config);
xmlNode* ags_simple_file_write_property_list(AgsFile *file, xmlNode *parent, GList *property);
xmlNode* ags_simple_file_write_property(AgsFile *file, xmlNode *parent, GParameter *property);
xmlNode* ags_simple_file_write_window(AgsFile *file, xmlNode *parent, AgsWindow *window);
xmlNode* ags_simple_file_write_machine_list(AgsFile *file, xmlNode *parent, GList *machine);
xmlNode* ags_simple_file_write_machine(AgsFile *file, xmlNode *parent, AgsMachine *machine);
xmlNode* ags_simple_file_write_pad_list(AgsFile *file, xmlNode *parent, GList *pad);
xmlNode* ags_simple_file_write_pad(AgsFile *file, xmlNode *parent, AgsPad *pad);
xmlNode* ags_simple_file_write_line_list(AgsFile *file, xmlNode *parent, GList *line);
xmlNode* ags_simple_file_write_line(AgsFile *file, xmlNode *parent, AgsLine *line);
xmlNode* ags_simple_file_write_editor(AgsFile *file, xmlNode *parent, AgsEditor *editor);
xmlNode* ags_simple_file_write_automation_editor(AgsFile *file, xmlNode *parent, AgsAutomationEditor *automation_editor);

/**
 * SECTION:ags_file
 * @short_description: read/write XML file
 * @title: AgsFile
 * @section_id:
 * @include: ags/file/ags_file.h
 *
 * The #AgsFile is an object to read or write files using XML. It
 * is the persisting layer of Advanced Gtk+ Sequencer.
 */

enum{
  PROP_0,
  PROP_FILENAME,
  PROP_ENCODING,
  PROP_AUDIO_FORMAT,
  PROP_AUDIO_ENCODING,
  PROP_XML_DOC,
  PROP_APPLICATION_CONTEXT,
};

enum{
  OPEN,
  OPEN_FROM_DATA,
  RW_OPEN,
  WRITE,
  WRITE_RESOLVE,
  READ,
  READ_RESOLVE,
  READ_START,
  LAST_SIGNAL,
};

static gpointer ags_simple_file_parent_class = NULL;
static guint simple_file_signals[LAST_SIGNAL] = { 0 };

GType
ags_simple_file_get_type(void)
{
  static GType ags_type_simple_file = 0;

  if(!ags_type_simple_file){
    static const GTypeInfo ags_simple_file_info = {
      sizeof (AgsSimpleFileClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_simple_file_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSimpleFile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_simple_file_init,
    };

    ags_type_simple_file = g_type_register_static(G_TYPE_OBJECT,
						  "AgsSimpleFile\0",
						  &ags_simple_file_info,
						  0);
  }

  return(ags_type_simple_file);
}

void
ags_simple_file_class_init(AgsSimpleFileClass *simple_file)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_simple_file_parent_class = g_type_class_peek_parent(simple_file);

  /* GObjectClass */
  gobject = (GObjectClass *) simple_file;

  gobject->get_property = ags_simple_file_get_property;
  gobject->set_property = ags_simple_file_set_property;

  gobject->finalize = ags_simple_file_finalize;

  /* properties */
  /**
   * AgsSimpleFile:filename:
   *
   * The assigned filename to open and read from.
   *
   * Since: w0.7.25
   */
  param_spec = g_param_spec_string("filename\0",
				   "filename to read or write\0",
				   "The filename to read or write to.\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILENAME,
				  param_spec);

  /**
   * AgsSimpleFile:encoding:
   *
   * The charset encoding to use.
   *
   * Since: 0.7.25
   */
  param_spec = g_param_spec_string("encoding\0",
				   "encoding to use\0",
				   "The encoding of the XML document.\0",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_ENCODING,
				  param_spec);

  /**
   * AgsSimpleFile:audio-format:
   *
   * The format of embedded audio data.
   *
   * Since: 0.7.25
   */
  param_spec = g_param_spec_string("audio-format\0",
				   "audio format to use\0",
				   "The audio format used to embedded audio.\0",
				   AGS_SIMPLE_FILE_DEFAULT_AUDIO_FORMAT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_FORMAT,
				  param_spec);

  /**
   * AgsSimpleFile:audio-encoding:
   *
   * The encoding to use for embedding audio data.
   *
   * Since: 0.7.25
   */
  param_spec = g_param_spec_string("audio-encoding\0",
				   "audio encoding to use\0",
				   "The audio encoding used to embedded audio.\0",
				   AGS_SIMPLE_FILE_DEFAULT_AUDIO_ENCODING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_ENCODING,
				  param_spec);

  /**
   * AgsSimpleFile:xml-doc:
   *
   * The assigned xml-doc.
   * 
   * Since: 0.7.25
   */
  param_spec = g_param_spec_pointer("xml-doc\0",
				    "xml document of file\0",
				    "The xml document assigned with file\0",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_XML_DOC,
				  param_spec);

  /**
   * AgsSimpleFile:application-context:
   *
   * The application context assigned with.
   *
   * Since: 0.7.25
   */
  param_spec = g_param_spec_object("application-context\0",
				   "application context of file\0",
				   "The application context to write to file.\0",
				   AGS_TYPE_APPLICATION_CONTEXT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /* AgsSimpleFileClass */
  simple_file->open = ags_simple_file_real_open;
  simple_file->rw_open = ags_simple_file_real_rw_open;
  simple_file->open_from_data = ags_simple_file_real_open_from_data;

  simple_file->write = ags_simple_file_real_write;
  simple_file->write_resolve = ags_simple_file_real_write_resolve;
  simple_file->read = ags_simple_file_real_read;
  simple_file->read_resolve = ags_simple_file_real_read_resolve;
  simple_file->read_start = ags_simple_file_real_read_start;

  /* signals */
  /**
   * AgsSimpleFile::open:
   * @simple_file: the #AgsSimpleFile
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file with appropriate filename.
   *
   * Since: 0.7.25
   */
  simple_file_signals[OPEN] =
    g_signal_new("open\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, open),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__POINTER,
		 G_TYPE_NONE, 1,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::open-from-data:
   * @simple_file: the #AgsSimpleFile
   * @buffer: the buffer containing the file
   * @length: the buffer length
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file from a buffer containing the file.
   *
   * Since: 0.7.25
   */
  simple_file_signals[OPEN_FROM_DATA] =
    g_signal_new("open-from-data\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, open_from_data),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__STRING_UINT_POINTER,
		 G_TYPE_NONE, 3,
		 G_TYPE_STRING,
		 G_TYPE_UINT,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::open-from-data:
   * @simple_file: the #AgsSimpleFile
   * @create: if %TRUE the file will be created if not exists
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file in read-write mode.
   *
   * Since: 0.7.25
   */
  simple_file_signals[RW_OPEN] =
    g_signal_new("rw-open\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, rw_open),
		 NULL, NULL,
		 g_cclosure_user_marshal_VOID__BOOLEAN_POINTER,
		 G_TYPE_NONE, 2,
		 G_TYPE_BOOLEAN,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::write:
   * @simple_file: the #AgsSimpleFile
   * 
   * Write XML Document to disk.
   *
   * Since: 0.7.25
   */
  simple_file_signals[WRITE] =
    g_signal_new("write\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, write),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::write-resolve:
   * @simple_file: the #AgsSimpleFile
   *
   * Resolve references and generate thus XPath expressions just
   * before writing to disk.
   *
   * Since: 0.7.25
   */
  simple_file_signals[WRITE_RESOLVE] =
    g_signal_new("write_resolve\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, write_resolve),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read:
   * @simple_file: the #AgsSimpleFile
   *
   * Read a XML document from disk with specified simple_filename.
   * 
   * Since: 0.7.25
   */
  simple_file_signals[READ] =
    g_signal_new("read\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read-resolve:
   * @simple_file: the #AgsSimpleFile
   *
   * Resolve XPath expressions to their counterpart the newly created
   * instances refering to.
   * 
   * Since: 0.7.25
   */
  simple_file_signals[READ_RESOLVE] =
    g_signal_new("read_resolve\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read_resolve),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read-start:
   * @simple_file: the #AgsSimpleFile
   *
   * Hook after reading XML document to update or start the application.
   * 
   * Since: 0.7.25
   */
  simple_file_signals[READ_START] =
    g_signal_new("read_start\0",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read_start),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

GQuark
ags_simple_file_error_quark()
{
  return(g_quark_from_static_string("ags-simple-file-error-quark\0"));
}

void
ags_simple_file_init(AgsSimpleFile *simple_file)
{
  simple_file->flags = 0;

  simple_file->out = NULL;
  simple_file->buffer = NULL;

  simple_file->filename = NULL;
  simple_file->encoding = AGS_SIMPLE_FILE_DEFAULT_ENCODING;
  simple_file->dtd = AGS_SIMPLE_FILE_DEFAULT_DTD;

  simple_file->audio_format = AGS_SIMPLE_FILE_DEFAULT_AUDIO_FORMAT;
  simple_file->audio_encoding = AGS_SIMPLE_FILE_DEFAULT_AUDIO_ENCODING;

  simple_file->doc = NULL;

  simple_file->id_refs = NULL;
  simple_file->lookup = NULL;
  simple_file->launch = NULL;

  simple_file->application_context = NULL;
}

void
ags_simple_file_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsSimpleFile *simple_file;

  simple_file = AGS_SIMPLE_FILE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      gchar *filename;

      filename = g_value_get_string(value);

      if(simple_file->filename == filename){
	return;
      }

      if(simple_file->filename != NULL){
	g_free(simple_file->filename);
      }
      
      simple_file->filename = g_strdup(filename);
    }
    break;
  case PROP_ENCODING:
    {
      gchar *encoding;

      encoding = g_value_get_string(value);

      simple_file->encoding = encoding;
    }
    break;
  case PROP_AUDIO_FORMAT:
    {
      gchar *audio_format;

      audio_format = g_value_get_string(value);

      simple_file->audio_format = audio_format;
    }
    break;
  case PROP_AUDIO_ENCODING:
    {
      gchar *audio_encoding;

      audio_encoding = g_value_get_string(value);

      simple_file->audio_encoding = audio_encoding;
    }
    break;
  case PROP_XML_DOC:
    {
      xmlDoc *doc;

      doc = (xmlDoc *) g_value_get_pointer(value);
      
      simple_file->doc = doc;
    }
    break;
  case PROP_APPLICATION_CONTEXT:
    {
      GObject *application_context;

      application_context = g_value_get_object(value);

      if(simple_file->application_context == application_context){
	return;
      }

      if(simple_file->application_context != NULL){
	g_object_unref(simple_file->application_context);
      }

      if(application_context != NULL){
	g_object_ref(application_context);
      }

      simple_file->application_context = application_context;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_simple_file_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsSimpleFile *simple_file;

  simple_file = AGS_SIMPLE_FILE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      g_value_set_string(value, simple_file->filename);
    }
    break;
  case PROP_ENCODING:
    {
      g_value_set_string(value, simple_file->encoding);
    }
    break;
  case PROP_AUDIO_FORMAT:
    {
      g_value_set_string(value, simple_file->audio_format);
    }
    break;
  case PROP_AUDIO_ENCODING:
    {
      g_value_set_string(value, simple_file->audio_encoding);
    }
    break;
  case PROP_XML_DOC:
    {
      g_value_set_pointer(value, simple_file->doc);
    }
    break;
  case PROP_APPLICATION_CONTEXT:
    {
      g_value_set_object(value, simple_file->application_context);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_simple_file_finalize(GObject *gobject)
{
  //TODO:JK: implement me
}

gchar*
ags_simple_file_str2md5(gchar *content, guint content_length)
{
  GChecksum *checksum;
  gchar *str;

  str = g_compute_checksum_for_string(G_CHECKSUM_MD5,
				      content,
				      content_length);

  return(str);
}

void
ags_simple_file_add_id_ref(AgsSimpleFile *simple_file, GObject *id_ref)
{
  if(simple_file == NULL ||
     id_ref == NULL){
    return;
  }
  
  g_object_ref(id_ref);
  simple_file->id_refs = g_list_prepend(simple_file->id_ref,
					id_ref);
}

GObject*
ags_simple_file_find_id_ref_by_node(AgsSimpleFile *simple_file, xmlNode *node)
{
  AgsFileIdRef *file_id_ref;
  GList *list;

  if(simple_file == NULL ||
     node == NULL){
    return(NULL);
  }
  
  list = file->id_ref;

  while(list != NULL){
    file_id_ref = AGS_FILE_ID_REF(list->data);

    if(file_id_ref->node == node){
      return((GObject *) file_id_ref);
    }

    list = list->next;
  }

  return(NULL);
}

GList*
ags_simple_file_find_id_ref_by_xpath(AgsSimpleFile *simple_file, gchar *xpath)
{
  AgsFileIdRef *file_id_ref;

  xmlXPathContext *xpath_context; 
  xmlXPathObject *xpath_object;
  xmlNode **node;

  GList *list;
  
  guint i;

  if(simple_file == NULL || xpath == NULL || !g_str_has_prefix(xpath, "xpath=\0")){
    g_message("invalid xpath: %s", xpath);

    return(NULL);
  }

  xpath = &(xpath[6]);

  /* Create xpath evaluation context */
  xpath_context = xmlXPathNewContext(simple_file->doc);

  if(xpath_context == NULL) {
    g_warning("Error: unable to create new XPath context\0");

    return(NULL);
  }

  /* Evaluate xpath expression */
  xpath_object = xmlXPathEval(xpath, xpath_context);

  if(xpath_object == NULL) {
    g_warning("Error: unable to evaluate xpath expression \"%s\"\0", xpath);
    xmlXPathFreeContext(xpath_context); 

    return(NULL);
  }

  node = xpath_object->nodesetval->nodeTab;

  list = NULL;
  
  for(i = 0; i < xpath_object->nodesetval->nodeNr; i++){
    if(node[i]->type == XML_ELEMENT_NODE){
      list = g_list_prepend(list,
			    ags_simple_file_find_id_ref_by_node(simple_file,
								node[i])));
  }
}

if(list == NULL){
  g_message("no xpath match: %s\0", xpath);
 }else{
  list = g_list_reverse(list);
 }
  
return(list);
}

GObject*
ags_simple_file_find_id_ref_by_reference(AgsSimpleFile *simple_file, gpointer ref)
{
  AgsFileIdRef *file_id_ref;
  
  GList *list;
  GList *ref_list;
  
  if(simple_file == NULL || ref == NULL){
    return(NULL);
  }

  list = simple_file->id_refs;
  ref_list = NULL;
  
  while(list != NULL){
    file_id_ref = AGS_FILE_ID_REF(list->data);

    if(file_id_ref->ref == ref){
      ref_list = g_list_prepend(ref_list,
				file_id_ref);
    }

    list = list->next;
  }

  return(ref_list);
}

void
ags_simple_file_add_lookup(AgsSimpleFile *simple_file, GObject *file_lookup)
{
  if(simple_file == NULL || file_lookup == NULL){
    return;
  }

  g_object_ref(G_OBJECT(file_lookup));

  simple_file->lookup = g_list_prepend(simple_file->lookup,
				       file_lookup);
}

void
ags_simple_file_add_launch(AgsSimpleFile *simple_file, GObject *file_launch)
{
  if(simple_file == NULL || file_launch == NULL){
    return;
  }

  g_object_ref(G_OBJECT(file_launch));

  simple_file->launch = g_list_prepend(simple_file->launch,
				       file_launch);
}

void
ags_simple_file_real_open(AgsSimpleFile *simple_file,
			  GError **error)
{
  if(simple_file == NULL){
    return;
  }

  /* parse the file and get the DOM */
  simple_file->doc = xmlReadFile(simple_file->filename, NULL, 0);

  if(simple_file->doc == NULL){
    g_warning("ags_simple_file.c - failed to read XML document %s\0", simple_file->filename);

    if(error != NULL){
      g_set_error(error,
		  AGS_SIMPLE_FILE_ERROR,
		  AGS_SIMPLE_FILE_ERROR_PARSER_FAILURE,
		  "unable to parse document: %s\n\0",
		  simple_file->filename);
    }
  }else{
    /*Get the root element node */
    simple_file->root_node = xmlDocGetRootElement(simple_file->doc);
  }
}

void
ags_simple_file_open(AgsSimpleFile *simple_file,
		     GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[OPEN], 0,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_open_from_data(AgsSimpleFile *simple_file,
				    gchar *data, guint length,
				    GError **error)
{
  if(simple_file == NULL){
    return;
  }

  simple_file->doc = xmlReadMemory(data, length, simple_file->filename, NULL, 0);

  if(simple_file->doc == NULL){
    g_warning("ags_simple_file.c - failed to read XML document %s\0", simple_file->filename);

    if(error != NULL){
      g_set_error(error,
		  AGS_SIMPLE_FILE_ERROR,
		  AGS_SIMPLE_FILE_ERROR_PARSER_FAILURE,
		  "unable to parse document from data: %s\n\0",
		  simple_file->filename);
    }
  }else{
    /*Get the root element node */
    simple_file->root_node = xmlDocGetRootElement(simple_file->doc);
  }
}

void
ags_simple_file_open_from_data(AgsSimpleFile *simple_file,
			       gchar *data, guint length,
			       GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[OPEN_FROM_DATA], 0,
		data, length,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_rw_open(AgsSimpleFile *simple_file,
			     gboolean create,
			     GError **error)
{
  if(simple_file == NULL){
    return;
  }

  simple_file->out = fopen(simple_file->filename, "w+\0");

  simple_file->doc = xmlNewDoc("1.0\0");
  simple_file->root_node = xmlNewNode(NULL, "ags-simple-file\0");
  xmlDocSetRootElement(simple_file->doc, simple_file->root_node);
}

void
ags_simple_file_rw_open(AgsSimpleFile *simple_file,
			gboolean create,
			GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[RW_OPEN], 0,
		create,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_open_filename(AgsSimpleFile *simple_file,
			      gchar *filename)
{
  GError *error;
  
  if(simple_file == NULL){
    return;
  }

  if(simple_file->filename != NULL){
    ags_simple_file_close(simple_file);
  }

  error = NULL;
  g_object_set(simple_file,
	       "filename\0", filename,
	       NULL);
  ags_simple_file_open(simple_file,
		       &error);

  if(error != NULL){
    g_warning(error->message);
  }
}

void
ags_simple_file_close(AgsSimpleFile *simple_file)
{
  if(simple_file == NULL){
    return;
  }

  if(simple_file->out != NULL){
    fclose(simple_file->out);
  }
  
  /*free the document */
  xmlFreeDoc(simple_file->doc);

  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();

  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();

  simple_file->filename = NULL;
}

void
ags_simple_file_real_write(AgsSimpleFile *simple_file)
{
  AgsApplicationContext *application_context;
  AgsConfig *config;
  
  xmlNode *node, *child;
  
  gchar *id;

  if(simple_file == NULL ||
     simple_file->root_node == NULL){
    return;
  }

  application_context = simple_file->application_context;
  config = ags_config_get_instance();
  
  id = ags_id_generator_create_uuid();

  node = simple_file->root_node;
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->build_id);

  /* add to parent */
  ags_simple_file_write_config(simple_file,
			       node,
			       config);  
  
  ags_simple_file_write_window(simple_file,
			       node,
			       AGS_XORG_APPLICATION_CONTEXT(application_context)->window);

  /* resolve */
  ags_simple_file_write_resolve(simple_file);

  /* 
   * Dumping document to file
   */
  //  xmlSaveFormatFileEnc(simple_file->filename, simple_file->doc, "UTF-8\0", 1);
  xmlDocDumpFormatMemoryEnc(simple_file->doc, &(simple_file->buffer), &size, simple_file->encoding, TRUE);

  fwrite(simple_file->buffer, size, sizeof(xmlChar), simple_file->out);
  fflush(simple_file->out);
}


void
ags_simple_file_write(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[WRITE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_write_resolve(AgsSimpleFile *simple_file)
{
  GList *list;
  
  simple_file->lookup = g_list_prepend(simple_file->lookup,
				       NULL);
  list = simple_file->lookup;

  while(list != NULL){
    ags_simple_file_lookup_resolve(AGS_FILE_LOOKUP(list->data));

    list = list->next;
  }
}

void
ags_simple_file_write_resolve(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[WRITE_RESOLVE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read(AgsSimpleFile *simple_file)
{
  AgsApplicationContext *application_context;

  xmlNode *root_node, *child;

  root_node = simple_file->root_node;
  
  /* child elements */
  child = root_node->children;
  application_context = simple_file->application_context;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp("ags-sf-config\0",
		     child->name,
		     13)){
	ags_simple_file_read_config(simple_file,
				    child,
				    (GObject **) &(application_context->config));
      }else if(!xmlStrncmp("ags-window\0",
			   child->name,
			   11)){
	ags_simple_file_read_window(simple_file,
				    child,
				    (AgsWindow **) &(AGS_XORG_APPLICATION_CONTEXT(application_context)->window));
      }
    }

    child = child->next;
  }

  /* resolve */
  ags_simple_file_read_resolve(simple_file);
  
  g_message("XML simple file resolved\0");

  ags_connectable_connect(AGS_CONNECTABLE(application_context));

  g_message("XML simple file connected\0");

  /* start */
  ags_simple_file_read_start(simple_file);
}

void
ags_simple_file_read(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read_resolve(AgsSimpleFile *simple_file)
{
  GList *list;

  simple_file->lookup = g_list_prepend(simple_file->lookup,
				       NULL);
  list = g_list_reverse(simple_file->lookup);
  
  while(list != NULL){
    ags_simple_file_lookup_resolve(AGS_SIMPLE_FILE_LOOKUP(list->data));

    list = list->next;
  }
}

void
ags_simple_file_read_resolve(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ_RESOLVE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read_start(AgsSimpleFile *simple_file)
{
  GList *list;

  list = g_list_reverse(simple_file->launch);

  while(list != NULL){
    ags_simple_file_launch_start(AGS_SIMPLE_FILE_LAUNCH(list->data));

    list = list->next;
  }
}

void
ags_simple_file_read_start(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ_START], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_read_config(AgsSimpleFile *simple_file, xmlNode *node, GObject **ags_config)
{
  AgsConfig *gobject;

  gchar *id;

  char *buffer;
  gsize buffer_length;

  gobject = config;
  gobject->version = xmlGetProp(node,
				AGS_FILE_VERSION_PROP);

  gobject->build_id = xmlGetProp(node,
				 AGS_FILE_BUILD_ID_PROP);

  buffer = xmlNodeGetContent(node);
  buffer_length = xmlStrlen(buffer);

  ags_config_load_from_data(gobject,
			    buffer, buffer_length);
}

void
ags_simple_file_read_property_list(AgsFile *file, xmlNode *node, GList **property)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_property(AgsFile *file, xmlNode *node, GParameter **property)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_window(AgsFile *file, xmlNode *node, AgsWindow **window)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_machine_list(AgsFile *file, xmlNode *node, GList **machine)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_machine(AgsFile *file, xmlNode *node, AgsMachine **machine)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_pad_list(AgsFile *file, xmlNode *node, GList **pad)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_pad(AgsFile *file, xmlNode *node, AgsPad **pad)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_line_list(AgsFile *file, xmlNode *node, GList **line)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_line(AgsFile *file, xmlNode *node, AgsLine **line)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_editor(AgsFile *file, xmlNode *node, AgsEditor **editor)
{
  //TODO:JK: implement me
}

void
ags_simple_file_read_automation_editor(AgsFile *file, xmlNode *node, AgsAutomationEditor **automation_editor)
{
  //TODO:JK: implement me
}


void
ags_simple_file_write_config(AgsSimpleFile *simple_file, xmlNode *parent, GObject *ags_config)
{
  xmlNode *node;
  xmlNode *cdata;

  gchar *id;
  char *buffer;
  gsize buffer_length;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-config\0");

  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context\0", simple_file->application_context,
					  "file\0", simple_file,
					  "node\0", node,
					  "xpath\0", g_strdup_printf("xpath=//*[@id='%s']\0", id),
					  "reference\0", config,
					  NULL));

  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_CONFIG(config)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_CONFIG(config)->build_id);

  xmlAddChild(parent,
	      node);

  /* cdata */
  ags_config_to_data(config,
		     &buffer,
		     &buffer_length);

  cdata = xmlNewCDataBlock(simple_file->doc,
			   buffer,
			   buffer_length);

  xmlAddChild(node,
	      cdata);
}

xmlNode*
ags_simple_file_write_window(AgsFile *file, xmlNode *parent, AgsConfig *config)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_property_list(AgsFile *file, xmlNode *parent, GList *property)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_property(AgsFile *file, xmlNode *parent, GParameter *property)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_window(AgsFile *file, xmlNode *parent, AgsWindow *window)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_machine_list(AgsFile *file, xmlNode *parent, GList *machine)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_machine(AgsFile *file, xmlNode *parent, AgsMachine *machine)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_pad_list(AgsFile *file, xmlNode *parent, GList *pad)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_pad(AgsFile *file, xmlNode *parent, AgsPad *pad)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_line_list(AgsFile *file, xmlNode *parent, GList *line)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_line(AgsFile *file, xmlNode *parent, AgsLine *line)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_editor(AgsFile *file, xmlNode *parent, AgsEditor *editor)
{
  //TODO:JK: implement me
}

xmlNode*
ags_simple_file_write_automation_editor(AgsFile *file, xmlNode *parent, AgsAutomationEditor *automation_editor)
{
  //TODO:JK: implement me
}

AgsSimpleFile*
ags_simple_file_new()
{
  AgsSimpleFile *simple_file;

  simple_file = (AgsSimpleFile *) g_object_new(AGS_TYPE_SIMPLE_FILE,
					       NULL);

  return(simple_file);
}

