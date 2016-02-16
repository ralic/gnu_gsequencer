/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2016 Joël Krähemann
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

#include <ags/audio/ags_audio_buffer_util.h>

signed char*
ags_audio_buffer_util_morph_s8(signed char *buffer, guint channels,
			       guint buffer_length,
			       guint morph_lookbehind,
			       AgsComplex *morph)
{
  //TODO:JK: implement me
}

signed short*
ags_audio_buffer_util_morph_s16(signed short *buffer, guint channels,
				guint buffer_length,
				guint morph_lookbehind,
				AgsComplex *morph)
{
  //TODO:JK: implement me
}

signed long*
ags_audio_buffer_util_morph_s24(signed long *buffer, guint channels,
				guint buffer_length,
				guint morph_lookbehind,
				AgsComplex *morph)
{
  //TODO:JK: implement me
}

signed long*
ags_audio_buffer_util_morph_s32(signed long *buffer, guint channels,
				guint buffer_length,
				guint morph_lookbehind,
				AgsComplex *morph)
{
  //TODO:JK: implement me
}

signed long long*
ags_audio_buffer_util_morph_s64(signed char *buffer, guint channels,
				guint buffer_length,
				guint morph_lookbehind,
				AgsComplex *morph)
{
  //TODO:JK: implement me
}

float*
ags_audio_buffer_util_morph_float(float *buffer, guint channels,
				  guint buffer_length,
				  guint morph_lookbehind,
				  AgsComplex *morph)
{
  //TODO:JK: implement me
}

double*
ags_audio_buffer_util_morph_double(double *buffer, guint channels,
				   guint buffer_length,
				   guint morph_lookbehind,
				   AgsComplex *morph)
{
  //TODO:JK: implement me
}

void*
ags_audio_buffer_util_morph(void *buffer, guint channels,
			    guint format,
			    guint buffer_length,
			    guint morph_lookbehind,
			    AgsComplex *morph)
{
  //TODO:JK: implement me
}

signed char*
ags_audio_buffer_util_resample_s8(signed char *buffer, guint channels,
				  guint samplerate,
				  guint buffer_length,
				  guint target_samplerate)
{
  //TODO:JK: implement me
}

signed short*
ags_audio_buffer_util_resample_s16(signed short *buffer, guint channels,
				   guint samplerate,
				   guint buffer_length,
				   guint target_samplerate)
{
  //TODO:JK: implement me
}

signed long*
ags_audio_buffer_util_resample_s24(signed long *buffer, guint channels,
				   guint samplerate,
				   guint buffer_length,
				   guint target_samplerate)
{
  //TODO:JK: implement me
}

signed long*
ags_audio_buffer_util_resample_s32(signed long *buffer, guint channels,
				   guint samplerate,
				   guint buffer_length,
				   guint target_samplerate)
{
  //TODO:JK: implement me
}

signed long long*
ags_audio_buffer_util_resample_s64(signed char *buffer, guint channels,
				   guint samplerate,
				   guint buffer_length,
				   guint target_samplerate)
{
  //TODO:JK: implement me
}

float*
ags_audio_buffer_util_resample_float(float *buffer, guint channels,
				     guint samplerate,
				     guint buffer_length,
				     guint target_samplerate)
{
  //TODO:JK: implement me
}

double*
ags_audio_buffer_util_resample_double(double *buffer, guint channels,
				      guint samplerate,
				      guint buffer_length,
				      guint target_samplerate)
{
  //TODO:JK: implement me
}

void*
ags_audio_buffer_util_resample(void *buffer, guint channels,
			       guint format,  guint samplerate,
			       guint buffer_length,
			       guint target_samplerate)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_s8(signed char *destination, guint dchannels,
				    signed char *source, guint schannels,
				    guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_s16(signed short *destination, guint dchannels,
				     signed char *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_s24(signed long *destination, guint dchannels,
				     signed char *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_s32(signed long *destination, guint dchannels,
				     signed char *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_s64(signed long long *destination, guint dchannels,
				     signed char *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_float(signed float *destination, guint dchannels,
				       signed char *source, guint schannels,
				       guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s8_to_double(signed double *destination, guint dchannels,
					signed char *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s16_to_s8(signed char *destination, guint dchannels,
				     signed short *source, guint schannels,
				     guint count)
{
  static const gdouble scale = 0.00387585070345;
  guint limit;
  guint i;

  i = 0;
  
  /* unrolled function */
  if(count > 8){
    limit = count - 8;
  
    for(; i < limit; i += 8){
      *destination = 0xff & ((signed short) ((*destination) + (scale * source[0])));
      destination[1 * dchannels] = 0xff & ((signed short) (destination[1 * dchannels] + (scale * source[1 * schannels])));
      destination[2 * dchannels] = 0xff & ((signed short) (destination[2 * dchannels] + (scale * source[2 * schannels])));
      destination[3 * dchannels] = 0xff & ((signed short) (destination[3 * dchannels] + (scale * source[3 * schannels])));
      destination[4 * dchannels] = 0xff & ((signed short) (destination[4 * dchannels] + (scale * source[4 * schannels])));
      destination[5 * dchannels] = 0xff & ((signed short) (destination[5 * dchannels] + (scale * source[5 * schannels])));
      destination[6 * dchannels] = 0xff & ((signed short) (destination[6 * dchannels] + (scale * source[6 * schannels])));
      destination[7 * dchannels] = 0xff & ((signed short) (destination[7 * dchannels] + (scale * source[7 * schannels])));

      destination += (8 * dchannels);
      source += (8 * schannels);
    }
  }

  for(; i < count; i++){
    *destination = 0xff & ((signed short) ((*destination) + (scale * source[0])));

    destination += dchannels;
    source += schannels;
  }
}

void
ags_audio_buffer_util_copy_s16_to_s16(signed short *destination, guint dchannels,
				      signed short *source, guint schannels,
				      guint count)
{
  guint limit;
  guint i;

  i = 0;
  
  /* unrolled function */
  if(count > 8){
    limit = count - 8;
  
    for(; i < limit; i += 8){
      *destination = 0xffff & ((signed long) ((*destination) + (*source)));
      destination[1 * dchannels] = 0xffff & ((signed long) (destination[1 * dchannels] + source[1 * schannels]));
      destination[2 * dchannels] = 0xffff & ((signed long) (destination[2 * dchannels] + source[2 * schannels]));
      destination[3 * dchannels] = 0xffff & ((signed long) (destination[3 * dchannels] + source[3 * schannels]));
      destination[4 * dchannels] = 0xffff & ((signed long) (destination[4 * dchannels] + source[4 * schannels]));
      destination[5 * dchannels] = 0xffff & ((signed long) (destination[5 * dchannels] + source[5 * schannels]));
      destination[6 * dchannels] = 0xffff & ((signed long) (destination[6 * dchannels] + source[6 * schannels]));
      destination[7 * dchannels] = 0xffff & ((signed long) (destination[7 * dchannels] + source[7 * schannels]));

      destination += (8 * dchannels);
      source += (8 * schannels);
    }
  }

  for(; i < count; i++){
    *destination = 0xffff & ((signed long) ((*destination) + (*source)));

    destination += dchannels;
    source += schannels;
  }
}

void
ags_audio_buffer_util_copy_s16_to_s24(signed long *destination, guint dchannels,
				      signed short *source, guint schannels,
				      guint count)
{
  static const gdouble scale = 256.00778222;
  guint limit;
  guint i;

  i = 0;
  
  /* unrolled function */
  if(count > 8){
    limit = count - 8;
  
    for(; i < limit; i += 8){
      *destination = 0xffffff & ((signed long) ((*destination) + (scale * source[0])));
      destination[1 * dchannels] = 0xffffff & ((signed long) (destination[1 * dchannels] + (signed long) (scale * source[1 * schannels])));
      destination[2 * dchannels] = 0xffffff & ((signed long) (destination[2 * dchannels] + (signed long) (scale * source[2 * schannels])));
      destination[3 * dchannels] = 0xffffff & ((signed long) (destination[3 * dchannels] + (signed long) (scale * source[3 * schannels])));
      destination[4 * dchannels] = 0xffffff & ((signed long) (destination[4 * dchannels] + (signed long) (scale * source[4 * schannels])));
      destination[5 * dchannels] = 0xffffff & ((signed long) (destination[5 * dchannels] + (signed long) (scale * source[5 * schannels])));
      destination[6 * dchannels] = 0xffffff & ((signed long) (destination[6 * dchannels] + (signed long) (scale * source[6 * schannels])));
      destination[7 * dchannels] = 0xffffff & ((signed long) (destination[7 * dchannels] + (signed long) (scale * source[7 * schannels])));

      destination += (8 * dchannels);
      source += (8 * schannels);
    }
  }

  for(; i < count; i++){
    *destination = 0xffffff & ((signed long) ((*destination) + (scale * source[0])));

    destination += dchannels;
    source += schannels;
  }
}

void
ags_audio_buffer_util_copy_s16_to_s32(signed long *destination, guint dchannels,
				      signed short *source, guint schannels,
				      guint count)
{
  static const gdouble scale = 65538.0000305;
  guint limit;
  guint i;

  i = 0;
  
  /* unrolled function */
  if(count > 8){
    limit = count - 8;
  
    for(; i < limit; i += 8){
      *destination = 0xffffffff & ((signed long) ((*destination) + (scale * source[0])));
      destination[1 * dchannels] = 0xffffffff & ((signed long long) (destination[1 * dchannels] + (signed long) (scale * source[1 * schannels])));
      destination[2 * dchannels] = 0xffffffff & ((signed long long) (destination[2 * dchannels] + (signed long) (scale * source[2 * schannels])));
      destination[3 * dchannels] = 0xffffffff & ((signed long long) (destination[3 * dchannels] + (signed long) (scale * source[3 * schannels])));
      destination[4 * dchannels] = 0xffffffff & ((signed long long) (destination[4 * dchannels] + (signed long) (scale * source[4 * schannels])));
      destination[5 * dchannels] = 0xffffffff & ((signed long long) (destination[5 * dchannels] + (signed long) (scale * source[5 * schannels])));
      destination[6 * dchannels] = 0xffffffff & ((signed long long) (destination[6 * dchannels] + (signed long) (scale * source[6 * schannels])));
      destination[7 * dchannels] = 0xffffffff & ((signed long long) (destination[7 * dchannels] + (signed long) (scale * source[7 * schannels])));

      destination += (8 * dchannels);
      source += (8 * schannels);
    }
  }

  for(; i < count; i++){
    *destination = 0xffffffff & ((signed long) ((*destination) + (scale * source[0])));

    destination += dchannels;
    source += schannels;
  }
}

void
ags_audio_buffer_util_copy_s16_to_s64(signed long long *destination, guint dchannels,
				      signed short *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s16_to_float(signed float *destination, guint dchannels,
					signed short *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s16_to_double(signed double *destination, guint dchannels,
					 signed short *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_s8(signed char *destination, guint dchannels,
				     signed long *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_s16(signed short *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_s24(signed long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_s32(signed long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_s64(signed long long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_float(signed float *destination, guint dchannels,
					signed long *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s24_to_double(signed double *destination, guint dchannels,
					 signed long *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_s8(signed char *destination, guint dchannels,
				     signed long *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_s16(signed short *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_s24(signed long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_s32(signed long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_s64(signed long long *destination, guint dchannels,
				      signed long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_float(signed float *destination, guint dchannels,
					signed long *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s32_to_double(signed double *destination, guint dchannels,
					 signed long *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_s8(signed char *destination, guint dchannels,
				     signed long long *source, guint schannels,
				     guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_s16(signed short *destination, guint dchannels,
				      signed long long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_s24(signed long *destination, guint dchannels,
				      signed long long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_s32(signed long *destination, guint dchannels,
				      signed long long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_s64(signed long long *destination, guint dchannels,
				      signed long long *source, guint schannels,
				      guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_float(signed float *destination, guint dchannels,
					signed long long *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_s64_to_double(signed double *destination, guint dchannels,
					 signed long long *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_s8(signed char *destination, guint dchannels,
				       signed float *source, guint schannels,
				       guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_s16(signed short *destination, guint dchannels,
					signed float *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_s24(signed long *destination, guint dchannels,
					signed float *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_s32(signed long *destination, guint dchannels,
					signed float *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_s64(signed long long *destination, guint dchannels,
					signed float *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_float(signed float *destination, guint dchannels,
					  signed float *source, guint schannels,
					  guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_float_to_double(signed double *destination, guint dchannels,
					   signed float *source, guint schannels,
					   guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_s8(signed char *destination, guint dchannels,
					signed double *source, guint schannels,
					guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_s16(signed short *destination, guint dchannels,
					 signed double *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_s24(signed long *destination, guint dchannels,
					 signed double *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_s32(signed long *destination, guint dchannels,
					 signed double *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_s64(signed long long *destination, guint dchannels,
					 signed double *source, guint schannels,
					 guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_float(signed float *destination, guint dchannels,
					   signed double *source, guint schannels,
					   guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_double_to_double(signed double *destination, guint dchannels,
					    signed double *source, guint schannels,
					    guint count)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_get_copy_mode(guint dformat, gboolean source_is_soundcard,
				    guint sformat, gboolean destination_is_soundcard)
{
  //TODO:JK: implement me
}

void
ags_audio_buffer_util_copy_buffer_to_buffer(void *destination, guint dchannels,
					    void *source, guint schannels,
					    guint count, guint mode)
{
  switch(mode){
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_S8:
    {
      ags_audio_buffer_util_copy_s8_to_s8((signed char *) destination, dchannels,
					  (signed char *) source, schannels,
					  count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_S16:
    {
      ags_audio_buffer_util_copy_s8_to_s16((signed short *) destination, dchannels,
					   (signed char *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_S24:
    {
      ags_audio_buffer_util_copy_s8_to_s24((signed long *) destination, dchannels,
					   (signed char *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_S32:
    {
      ags_audio_buffer_util_copy_s8_to_s32((signed long *) destination, dchannels,
					   (signed char *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_S64:
    {
      ags_audio_buffer_util_copy_s8_to_s64((signed long long *) destination, dchannels,
					   (signed char *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_s8_to_float((signed float *) destination, dchannels,
					     (signed char *) source, schannels,
					     count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S8_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_s8_to_double((signed double *) destination, dchannels,
					      (signed char *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_S8:
    {
      ags_audio_buffer_util_copy_s16_to_s8((signed char *) destination, dchannels,
					   (signed short *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_S16:
    {
      ags_audio_buffer_util_copy_s16_to_s16((signed short *) destination, dchannels,
					    (signed short *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_S24:
    {
      ags_audio_buffer_util_copy_s16_to_s24((signed long *) destination, dchannels,
					    (signed short *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_S32:
    {
      ags_audio_buffer_util_copy_s16_to_s32((signed long *) destination, dchannels,
					    (signed short *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_S64:
    {
      ags_audio_buffer_util_copy_s16_to_s64((signed long long *) destination, dchannels,
					    (signed short *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_s16_to_float((signed float *) destination, dchannels,
					      (signed short *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S16_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_s16_to_double((signed double *) destination, dchannels,
					       (signed short *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_S8:
    {
      ags_audio_buffer_util_copy_s24_to_s8((signed char *) destination, dchannels,
					   (signed long *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_S16:
    {
      ags_audio_buffer_util_copy_s24_to_s16((signed short *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_S24:
    {
      ags_audio_buffer_util_copy_s24_to_s24((signed long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_S32:
    {
      ags_audio_buffer_util_copy_s24_to_s32((signed long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_S64:
    {
      ags_audio_buffer_util_copy_s24_to_s64((signed long long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_s24_to_float((signed float *) destination, dchannels,
					      (signed long *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S24_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_s24_to_double((signed double *) destination, dchannels,
					       (signed long *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_S8:
    {
      ags_audio_buffer_util_copy_s32_to_s8((signed char *) destination, dchannels,
					   (signed long *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_S16:
    {
      ags_audio_buffer_util_copy_s32_to_s16((signed short *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_S24:
    {
      ags_audio_buffer_util_copy_s32_to_s24((signed long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_S32:
    {
      ags_audio_buffer_util_copy_s32_to_s32((signed long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_S64:
    {
      ags_audio_buffer_util_copy_s32_to_s64((signed long long *) destination, dchannels,
					    (signed long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_s32_to_s8((signed float *) destination, dchannels,
					   (signed long *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S32_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_s32_to_double((signed double *) destination, dchannels,
					       (signed long *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_S8:
    {
      ags_audio_buffer_util_copy_s64_to_s8((signed char *) destination, dchannels,
					   (signed long long *) source, schannels,
					   count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_S16:
    {
      ags_audio_buffer_util_copy_s64_to_s16((signed short *) destination, dchannels,
					    (signed long long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_S24:
    {
      ags_audio_buffer_util_copy_s64_to_s24((signed long *) destination, dchannels,
					    (signed long long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_S32:
    {
      ags_audio_buffer_util_copy_s64_to_s32((signed long *) destination, dchannels,
					    (signed long long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_S64:
    {
      ags_audio_buffer_util_copy_s64_to_s64((signed long long *) destination, dchannels,
					    (signed long long *) source, schannels,
					    count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_s64_to_float((signed float *) destination, dchannels,
					      (signed long long *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_S64_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_s64_to_double((signed double *) destination, dchannels,
					       (signed long long *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_S8:
    {
      ags_audio_buffer_util_copy_float_to_s8((signed char *) destination, dchannels,
					     (signed float *) source, schannels,
					     count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_S16:
    {
      ags_audio_buffer_util_copy_float_to_s16((signed short *) destination, dchannels,
					      (signed float *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_S24:
    {
      ags_audio_buffer_util_copy_float_to_s24((signed long *) destination, dchannels,
					      (signed float *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_S32:
    {
      ags_audio_buffer_util_copy_float_to_s32((signed long *) destination, dchannels,
					      (signed float *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_S64:
    {
      ags_audio_buffer_util_copy_float_to_s64((signed long long *) destination, dchannels,
					      (signed float *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_float_to_float((signed float *) destination, dchannels,
						(signed float *) source, schannels,
						count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_FLOAT_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_float_to_double((signed double *) destination, dchannels,
						 (signed float *) source, schannels,
						 count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_S8:
    {
      ags_audio_buffer_util_copy_double_to_s8((signed char *) destination, dchannels,
					      (signed double *) source, schannels,
					      count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_S16:
    {
      ags_audio_buffer_util_copy_double_to_s16((signed short *) destination, dchannels,
					       (signed double *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_S24:
    {
      ags_audio_buffer_util_copy_double_to_s24((signed long *) destination, dchannels,
					       (signed double *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_S32:
    {
      ags_audio_buffer_util_copy_double_to_s32((signed long *) destination, dchannels,
					       (signed double *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_S64:
    {
      ags_audio_buffer_util_copy_double_to_s64((signed long long *) destination, dchannels,
					       (signed double *) source, schannels,
					       count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_FLOAT:
    {
      ags_audio_buffer_util_copy_double_to_float((signed float *) destination, dchannels,
						 (signed double *) source, schannels,
						 count);
    }
    break;
  case AGS_AUDIO_BUFFER_UTIL_COPY_DOUBLE_TO_DOUBLE:
    {
      ags_audio_buffer_util_copy_double_to_double((signed double *) destination, dchannels,
						  (signed double *) source, schannels,
						  count);
    }
    break;
  default:
    {
      g_warning("ags_audio_buffer_util.c - unknown copy mode\0");
    }
  }
}