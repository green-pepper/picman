/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanstrokeoptions.h
 * Copyright (C) 2003 Simon Budig
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

#ifndef __PICMAN_STROKE_OPTIONS_H__
#define __PICMAN_STROKE_OPTIONS_H__


#include "picmanfilloptions.h"


#define PICMAN_TYPE_STROKE_OPTIONS            (picman_stroke_options_get_type ())
#define PICMAN_STROKE_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STROKE_OPTIONS, PicmanStrokeOptions))
#define PICMAN_STROKE_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STROKE_OPTIONS, PicmanStrokeOptionsClass))
#define PICMAN_IS_STROKE_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STROKE_OPTIONS))
#define PICMAN_IS_STROKE_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_STROKE_OPTIONS))
#define PICMAN_STROKE_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_STROKE_OPTIONS, PicmanStrokeOptionsClass))


typedef struct _PicmanStrokeOptionsClass PicmanStrokeOptionsClass;

struct _PicmanStrokeOptions
{
  PicmanFillOptions  parent_instance;
};

struct _PicmanStrokeOptionsClass
{
  PicmanFillOptionsClass  parent_class;

  void (* dash_info_changed) (PicmanStrokeOptions *stroke_options,
                              PicmanDashPreset     preset);
};


GType               picman_stroke_options_get_type             (void) G_GNUC_CONST;

PicmanStrokeOptions * picman_stroke_options_new                  (Picman              *picman,
                                                              PicmanContext       *context,
                                                              gboolean           use_context_color);

PicmanStrokeMethod    picman_stroke_options_get_method           (PicmanStrokeOptions *options);

gdouble             picman_stroke_options_get_width            (PicmanStrokeOptions *options);
PicmanUnit            picman_stroke_options_get_unit             (PicmanStrokeOptions *options);
PicmanCapStyle        picman_stroke_options_get_cap_style        (PicmanStrokeOptions *options);
PicmanJoinStyle       picman_stroke_options_get_join_style       (PicmanStrokeOptions *options);
gdouble             picman_stroke_options_get_miter_limit      (PicmanStrokeOptions *options);
gdouble             picman_stroke_options_get_dash_offset      (PicmanStrokeOptions *options);
GArray            * picman_stroke_options_get_dash_info        (PicmanStrokeOptions *options);

PicmanPaintOptions  * picman_stroke_options_get_paint_options    (PicmanStrokeOptions *options);
gboolean            picman_stroke_options_get_emulate_dynamics (PicmanStrokeOptions *options);

void                picman_stroke_options_take_dash_pattern    (PicmanStrokeOptions *options,
                                                              PicmanDashPreset     preset,
                                                              GArray            *pattern);

void                picman_stroke_options_prepare              (PicmanStrokeOptions *options,
                                                              PicmanContext       *context,
                                                              gboolean           use_default_values);
void                picman_stroke_options_finish               (PicmanStrokeOptions *options);


#endif /* __PICMAN_STROKE_OPTIONS_H__ */
