#include "ags_ffplayer.h"
#include "ags_ffplayer_callbacks.h"

#include "../../audio/ags_audio.h"
#include "../../audio/ags_input.h"
#include "../../audio/ags_output.h"

#include <math.h>

GType ags_ffplayer_get_type(void);
void ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer);
void ags_ffplayer_init(AgsFFPlayer *ffplayer);
void ags_ffplayer_connect(AgsFFPlayer *ffplayer);
void ags_ffplayer_destroy(GtkObject *object);
void ags_ffplayer_show(GtkWidget *widget);

void ags_ffplayer_set_audio_channels(AgsAudio *audio,
				     guint audio_channels, guint audio_channels_old,
				     gpointer data);
void ags_ffplayer_set_pads(AgsAudio *audio, GType type,
			   guint pads, guint pads_old,
			   gpointer data);

void ags_ffplayer_paint(AgsFFPlayer *ffplayer);

extern void ags_file_read_ffplayer(AgsFile *file, AgsMachine *machine);
extern void ags_file_write_ffplayer(AgsFile *file, AgsMachine *machine);

GtkStyle *ffplayer_style;

GType
ags_ffplayer_get_type(void)
{
  static GType ffplayer_type = 0;

  if (!ffplayer_type){
    static const GtkTypeInfo ffplayer_info = {
      "AgsFFPlayer\0",
      sizeof(AgsFFPlayer), /* base_init */
      sizeof(AgsFFPlayerClass), /* base_finalize */
      (GtkClassInitFunc) ags_ffplayer_class_init,
      (GtkObjectInitFunc) ags_ffplayer_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      (GtkClassInitFunc) NULL,
    };

    ffplayer_type = gtk_type_unique (AGS_TYPE_MACHINE, &ffplayer_info);
  }

  return (ffplayer_type);
}

void
ags_ffplayer_class_init(AgsFFPlayerClass *ffplayer)
{
  AgsMachineClass *machine = (AgsMachineClass *) ffplayer;

  //  machine->read_file = ags_file_read_ffplayer;
  //  machine->write_file = ags_file_write_ffplayer;
}

void
ags_ffplayer_init(AgsFFPlayer *ffplayer)
{
  GtkTable *table;
  GtkHScrollbar *hscrollbar;
  GtkVBox *vbox;
  PangoAttrList *attr_list;
  PangoAttribute *attr;

  g_signal_connect_after((GObject *) ffplayer, "parent_set\0",
			 G_CALLBACK(ags_ffplayer_parent_set_callback), (gpointer) ffplayer);

  ffplayer->machine.audio->flags |= (AGS_AUDIO_INPUT_HAS_RECYCLING |
				     AGS_AUDIO_INPUT_TAKES_FILE |
				     AGS_AUDIO_SYNC |
				     AGS_AUDIO_HAS_NOTATION);

  table = (GtkTable *) gtk_table_new(1, 2, FALSE);
  gtk_container_add((GtkContainer*) (gtk_container_get_children((GtkContainer *) ffplayer))->data, (GtkWidget *) table);

  ffplayer->open = (GtkButton *) gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_table_attach(table, (GtkWidget *) ffplayer->open,
		   0, 1,
		   0, 1,
		   0, 0,
		   0, 0);

  vbox = (GtkVBox *) gtk_vbox_new(FALSE, 2);
  gtk_table_attach(table, (GtkWidget *) vbox,
		   1, 2,
		   0, 1,
		   GTK_FILL, GTK_FILL,
		   0, 0);

  ffplayer->control_width = 12;
  ffplayer->control_height = 40;

  ffplayer->drawing_area = (GtkDrawingArea *) gtk_drawing_area_new();
  gtk_widget_set_size_request((GtkWidget *) ffplayer->drawing_area, 16 * ffplayer->control_width, ffplayer->control_width * 8 + ffplayer->control_height);
  gtk_widget_set_style((GtkWidget *) ffplayer->drawing_area, ffplayer_style);
  gtk_widget_set_events ((GtkWidget *) ffplayer->drawing_area,
                         GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_POINTER_MOTION_MASK
                         | GDK_POINTER_MOTION_HINT_MASK);
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) ffplayer->drawing_area, FALSE, FALSE, 0);

  ffplayer->hadjustment = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 76 * ffplayer->control_width - GTK_WIDGET(ffplayer->drawing_area)->allocation.width, 1.0, (double) ffplayer->control_width, (double) (16 * ffplayer->control_width));
  hscrollbar = (GtkHScrollbar *) gtk_hscrollbar_new(ffplayer->hadjustment);
  gtk_widget_set_style((GtkWidget *) hscrollbar, ffplayer_style);
  gtk_box_pack_start((GtkBox *) vbox, (GtkWidget *) hscrollbar, FALSE, FALSE, 0);
}

void
ags_ffplayer_connect(AgsFFPlayer *ffplayer)
{
  ags_machine_connect((AgsMachine *) ffplayer);

  g_signal_connect((GObject *) ffplayer, "destroy\0",
		   G_CALLBACK(ags_ffplayer_destroy_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->open, "clicked\0",
		   G_CALLBACK(ags_ffplayer_open_clicked_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->drawing_area, "expose_event\0",
                   G_CALLBACK(ags_ffplayer_drawing_area_expose_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->drawing_area, "button_press_event\0",
                   G_CALLBACK(ags_ffplayer_drawing_area_button_press_callback), (gpointer) ffplayer);

  g_signal_connect((GObject *) ffplayer->hadjustment, "value_changed\0",
		   G_CALLBACK(ags_ffplayer_hscrollbar_value_changed), (gpointer) ffplayer);

  /* AgsAudio */
  g_signal_connect(G_OBJECT(ffplayer->machine.audio), "set_audio_channels\0",
		   G_CALLBACK(ags_ffplayer_set_audio_channels), NULL);

  g_signal_connect(G_OBJECT(ffplayer->machine.audio), "set_pads\0",
		   G_CALLBACK(ags_ffplayer_set_pads), NULL);
}

void
ags_ffplayer_destroy(GtkObject *object)
{
}

void
ags_ffplayer_show(GtkWidget *widget)
{
}

void
ags_ffplayer_set_audio_channels(AgsAudio *audio,
				guint audio_channels, guint audio_channels_old,
				gpointer data)
{
  /* empty */
}

void
ags_ffplayer_set_pads(AgsAudio *audio, GType type,
		      guint pads, guint pads_old,
		      gpointer data)
{
  /* empty */
}

void
ags_ffplayer_paint(AgsFFPlayer *ffplayer)
{
  GtkWidget *widget;
  cairo_t *cr;
  double semi_key_height;
  guint bitmap;
  guint x[2];
  guint i, i_stop, j, j0;

  widget = (GtkWidget *) ffplayer->drawing_area;

  semi_key_height = 2.0 / 3.0 * (double) ffplayer->control_height;
  bitmap = 0x52a52a; // description of the keyboard

  j = (guint) ceil(ffplayer->hadjustment->value / (double) ffplayer->control_width);
  j = j % 12;

  x[0] = (guint) round(ffplayer->hadjustment->value) % ffplayer->control_width;

  if(x[0] != 0){
    x[0] = ffplayer->control_width - x[0];
  }

  x[1] = ((guint) widget->allocation.width - x[0]) % ffplayer->control_width;
  i_stop = (widget->allocation.width - x[0] - x[1]) / ffplayer->control_width;

  cr = gdk_cairo_create(widget->window);

  cairo_set_source_rgb(cr, 0.015625, 0.03125, 0.21875);
  cairo_rectangle(cr, 0.0, 0.0, (double) widget->allocation.width, (double) widget->allocation.height);
  cairo_fill(cr);

  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

  if(x[0] != 0){
    j0 = (j != 0) ? j -1: 11;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, 0.0, 0.0, x[0], (double) semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      if(x[0] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (x[0] - ffplayer->control_width / 2),  semi_key_height);
	cairo_line_to(cr, (double) (x[0] - ffplayer->control_width / 2), (double) ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      if(((1 << (j0 + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) x[0], 0.0);
	cairo_line_to(cr, (double) x[0], ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, 0.0, ffplayer->control_height);
      cairo_line_to(cr, (double) x[0], ffplayer->control_height);
      cairo_stroke(cr);
    }
  }

  for(i = 0; i < i_stop; i++){
    if(((1 << j) & bitmap) != 0){
      // draw semi tone key
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, (double) (i * ffplayer->control_width + x[0]), 0.0, (double) ffplayer->control_width, semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), semi_key_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width / 2), ffplayer->control_height);
      cairo_stroke(cr);

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      // no semi tone key
      if(((1 << (j + 1)) & bitmap) == 0){
	cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), 0.0);
	cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (i * ffplayer->control_width + x[0]), ffplayer->control_height);
      cairo_line_to(cr, (double) (i * ffplayer->control_width + x[0] + ffplayer->control_width), ffplayer->control_height);
      cairo_stroke(cr);
    }

    if(j == 11)
      j = 0;
    else
      j++;
  }

  if(x[1] != 0){
    j0 = j;

    if(((1 << j0) & bitmap) != 0){
      cairo_set_source_rgb(cr, 0.5, 0.5, 1.0);

      cairo_rectangle(cr, (double) (widget->allocation.width - x[1]), 0.0, (double) x[1], semi_key_height);
      cairo_fill(cr); 	

      cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);

      if(x[1] > ffplayer->control_width / 2){
	cairo_move_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), semi_key_height);
	cairo_line_to(cr, (double) (widget->allocation.width - x[1] + ffplayer->control_width / 2), ffplayer->control_height);
	cairo_stroke(cr);
      }

      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }else{
      cairo_move_to(cr, (double) (widget->allocation.width - x[1]), ffplayer->control_height);
      cairo_line_to(cr, (double) widget->allocation.width, ffplayer->control_height);
      cairo_stroke(cr);
    }
  }
}

AgsFFPlayer*
ags_ffplayer_new()
{
  AgsFFPlayer *ffplayer;

  ffplayer = (AgsFFPlayer *) g_object_new(AGS_TYPE_FFPLAYER, NULL);

  return(ffplayer);
}
