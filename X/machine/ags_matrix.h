#ifndef __AGS_MATRIX_H__
#define __AGS_MATRIX_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "../ags_machine.h"

#include "../../audio/recall/ags_delay_shared_audio.h"
#include "../../audio/recall/ags_copy_pattern_shared_audio.h"
#include "../../audio/recall/ags_copy_pattern_shared_audio_run.h"

#define AGS_TYPE_MATRIX                (ags_matrix_get_type())
#define AGS_MATRIX(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_MATRIX, AgsMatrix))
#define AGS_MATRIX_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_MATRIX, AgsMatrixClass))
#define AGS_IS_MATRIX(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), AGS_TYPE_MATRIX))
#define AGS_IS_MATRIX_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE((class), AGS_TYPE_MATRIX))
#define AGS_MATRIX_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj), AGS_TYPE_MATRIX, AgsMatrixClass))

typedef struct _AgsMatrix AgsMatrix;
typedef struct _AgsMatrixClass AgsMatrixClass;

struct _AgsMatrix
{
  AgsMachine machine;

  guint flags;

  AgsDelaySharedAudio *delay_shared_audio;

  AgsCopyPatternSharedAudio *copy_pattern_shared_audio;
  AgsCopyPatternSharedAudioRun *copy_pattern_shared_audio_run;

  GtkTable *table;

  GtkToggleButton *run;
  GtkToggleButton *selected;
  GtkToggleButton *index[9];

  GtkDrawingArea *drawing_area;

  GtkAdjustment *adjustment;

  GtkSpinButton *length_spin;
  GtkOptionMenu *tic;
  GtkCheckButton *loop_button;
};

struct _AgsMatrixClass
{
  AgsMachineClass machine;
};

void ags_matrix_draw_gutter(AgsMatrix *matrix);
void ags_matrix_draw_matrix(AgsMatrix *matrix);
void ags_matrix_redraw_gutter_point(AgsMatrix *matrix, AgsChannel *channel, guint j, guint i);
void ags_matrix_highlight_gutter_point(AgsMatrix *matrix, guint j, guint i);
void ags_matrix_unpaint_gutter_point(AgsMatrix *matrix, guint j, guint i);

AgsMatrix* ags_matrix_new();

#endif /*__AGS_MATRIX_H__*/
