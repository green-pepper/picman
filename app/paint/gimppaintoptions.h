/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_PAINT_OPTIONS_H__
#define __PICMAN_PAINT_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_PAINT_OPTIONS_CONTEXT_MASK PICMAN_CONTEXT_FOREGROUND_MASK | \
                                        PICMAN_CONTEXT_BACKGROUND_MASK | \
                                        PICMAN_CONTEXT_OPACITY_MASK    | \
                                        PICMAN_CONTEXT_PAINT_MODE_MASK | \
                                        PICMAN_CONTEXT_BRUSH_MASK      | \
                                        PICMAN_CONTEXT_DYNAMICS_MASK   | \
                                        PICMAN_CONTEXT_PALETTE_MASK


typedef struct _PicmanJitterOptions   PicmanJitterOptions;
typedef struct _PicmanFadeOptions     PicmanFadeOptions;
typedef struct _PicmanGradientOptions PicmanGradientOptions;
typedef struct _PicmanSmoothingOptions PicmanSmoothingOptions;

struct _PicmanJitterOptions
{
  gboolean  use_jitter;
  gdouble   jitter_amount;
};

struct _PicmanFadeOptions
{
  gboolean        fade_reverse;
  gdouble         fade_length;
  PicmanUnit        fade_unit;
  PicmanRepeatMode  fade_repeat;
};

struct _PicmanGradientOptions
{
  gboolean        gradient_reverse;
  PicmanRepeatMode  gradient_repeat;
};

struct _PicmanSmoothingOptions
{
  gboolean use_smoothing;
  gint     smoothing_quality;
  gdouble  smoothing_factor;
};


#define PICMAN_TYPE_PAINT_OPTIONS            (picman_paint_options_get_type ())
#define PICMAN_PAINT_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINT_OPTIONS, PicmanPaintOptions))
#define PICMAN_PAINT_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINT_OPTIONS, PicmanPaintOptionsClass))
#define PICMAN_IS_PAINT_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINT_OPTIONS))
#define PICMAN_IS_PAINT_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINT_OPTIONS))
#define PICMAN_PAINT_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINT_OPTIONS, PicmanPaintOptionsClass))


typedef struct _PicmanPaintOptionsClass PicmanPaintOptionsClass;

struct _PicmanPaintOptions
{
  PicmanToolOptions           parent_instance;

  PicmanPaintInfo            *paint_info;

  gdouble                   brush_size;
  gdouble                   brush_angle;
  gdouble                   brush_aspect_ratio;

  PicmanPaintApplicationMode  application_mode;
  PicmanPaintApplicationMode  application_mode_save;

  gboolean                  hard;

  PicmanJitterOptions        *jitter_options;

  gboolean                  dynamics_expanded;
  PicmanFadeOptions          *fade_options;
  PicmanGradientOptions      *gradient_options;
  PicmanSmoothingOptions     *smoothing_options;

  PicmanViewType              brush_view_type;
  PicmanViewSize              brush_view_size;
  PicmanViewType              dynamics_view_type;
  PicmanViewSize              dynamics_view_size;
  PicmanViewType              pattern_view_type;
  PicmanViewSize              pattern_view_size;
  PicmanViewType              gradient_view_type;
  PicmanViewSize              gradient_view_size;
};

struct _PicmanPaintOptionsClass
{
  PicmanToolOptionsClass  parent_instance;
};


GType              picman_paint_options_get_type (void) G_GNUC_CONST;

PicmanPaintOptions * picman_paint_options_new      (PicmanPaintInfo    *paint_info);

gdouble            picman_paint_options_get_fade (PicmanPaintOptions *paint_options,
                                                PicmanImage        *image,
                                                gdouble           pixel_dist);

gdouble          picman_paint_options_get_jitter (PicmanPaintOptions *paint_options,
                                                PicmanImage        *image);

gboolean picman_paint_options_get_gradient_color (PicmanPaintOptions *paint_options,
                                                PicmanImage        *image,
                                                gdouble           grad_point,
                                                gdouble           pixel_dist,
                                                PicmanRGB          *color);

PicmanBrushApplicationMode
             picman_paint_options_get_brush_mode (PicmanPaintOptions *paint_options);

void    picman_paint_options_copy_brush_props    (PicmanPaintOptions *src,
                                                PicmanPaintOptions *dest);
void    picman_paint_options_copy_dynamics_props (PicmanPaintOptions *src,
                                                PicmanPaintOptions *dest);
void    picman_paint_options_copy_gradient_props (PicmanPaintOptions *src,
                                                PicmanPaintOptions *dest);


#endif  /*  __PICMAN_PAINT_OPTIONS_H__  */
