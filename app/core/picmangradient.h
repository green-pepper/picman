/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_GRADIENT_H__
#define __PICMAN_GRADIENT_H__


#include "picmandata.h"


#define PICMAN_GRADIENT_DEFAULT_SAMPLE_SIZE 40


struct _PicmanGradientSegment
{
  gdouble                  left, middle, right;

  PicmanGradientColor        left_color_type;
  PicmanRGB                  left_color;
  PicmanGradientColor        right_color_type;
  PicmanRGB                  right_color;

  PicmanGradientSegmentType  type;          /*  Segment's blending function  */
  PicmanGradientSegmentColor color;         /*  Segment's coloring type      */

  PicmanGradientSegment     *prev;
  PicmanGradientSegment     *next;
};


#define PICMAN_TYPE_GRADIENT            (picman_gradient_get_type ())
#define PICMAN_GRADIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GRADIENT, PicmanGradient))
#define PICMAN_GRADIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GRADIENT, PicmanGradientClass))
#define PICMAN_IS_GRADIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GRADIENT))
#define PICMAN_IS_GRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GRADIENT))
#define PICMAN_GRADIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GRADIENT, PicmanGradientClass))


typedef struct _PicmanGradientClass PicmanGradientClass;

struct _PicmanGradient
{
  PicmanData             parent_instance;

  PicmanGradientSegment *segments;
};

struct _PicmanGradientClass
{
  PicmanDataClass  parent_class;
};


GType                 picman_gradient_get_type       (void) G_GNUC_CONST;

PicmanData            * picman_gradient_new            (PicmanContext   *context,
                                                    const gchar   *name);
PicmanData            * picman_gradient_get_standard   (PicmanContext   *context);

PicmanGradientSegment * picman_gradient_get_color_at   (PicmanGradient        *gradient,
                                                    PicmanContext         *context,
                                                    PicmanGradientSegment *seg,
                                                    gdouble              pos,
                                                    gboolean             reverse,
                                                    PicmanRGB             *color);
PicmanGradientSegment * picman_gradient_get_segment_at (PicmanGradient  *grad,
                                                    gdouble        pos);

gboolean          picman_gradient_has_fg_bg_segments (PicmanGradient  *gradient);
PicmanGradient    * picman_gradient_flatten            (PicmanGradient  *gradient,
                                                    PicmanContext   *context);


/*  gradient segment functions  */

PicmanGradientSegment * picman_gradient_segment_new       (void);
PicmanGradientSegment * picman_gradient_segment_get_last  (PicmanGradientSegment *seg);
PicmanGradientSegment * picman_gradient_segment_get_first (PicmanGradientSegment *seg);
PicmanGradientSegment * picman_gradient_segment_get_nth   (PicmanGradientSegment *seg,
                                                       gint                 index);

void                  picman_gradient_segment_free      (PicmanGradientSegment *seg);
void                  picman_gradient_segments_free     (PicmanGradientSegment *seg);

void    picman_gradient_segment_split_midpoint  (PicmanGradient         *gradient,
                                               PicmanContext          *context,
                                               PicmanGradientSegment  *lseg,
                                               PicmanGradientSegment **newl,
                                               PicmanGradientSegment **newr);
void    picman_gradient_segment_split_uniform   (PicmanGradient         *gradient,
                                               PicmanContext          *context,
                                               PicmanGradientSegment  *lseg,
                                               gint                  parts,
                                               PicmanGradientSegment **newl,
                                               PicmanGradientSegment **newr);

/* Colors Setting/Getting Routines */
void    picman_gradient_segment_get_left_color  (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               PicmanRGB              *color);

void    picman_gradient_segment_set_left_color  (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               const PicmanRGB        *color);


void    picman_gradient_segment_get_right_color (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               PicmanRGB              *color);

void    picman_gradient_segment_set_right_color (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               const PicmanRGB        *color);


PicmanGradientColor
picman_gradient_segment_get_left_color_type     (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);

void
picman_gradient_segment_set_left_color_type     (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               PicmanGradientColor     color_type);


PicmanGradientColor
picman_gradient_segment_get_right_color_type    (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);

void
picman_gradient_segment_set_right_color_type    (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               PicmanGradientColor     color_type);


/* Position Setting/Getting Routines */
/* (Setters return the position after it was set) */
gdouble picman_gradient_segment_get_left_pos    (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);
gdouble picman_gradient_segment_set_left_pos    (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               gdouble               pos);

gdouble picman_gradient_segment_get_right_pos   (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);
gdouble picman_gradient_segment_set_right_pos   (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               gdouble               pos);

gdouble picman_gradient_segment_get_middle_pos  (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);
gdouble picman_gradient_segment_set_middle_pos  (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg,
                                               gdouble               pos);

/* Getting/Setting the Blending Function/Coloring Type */
PicmanGradientSegmentType
picman_gradient_segment_get_blending_function   (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);
PicmanGradientSegmentColor
picman_gradient_segment_get_coloring_type       (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *seg);

/*
 * If the second segment is NULL, these functions will process
 * until the end of the string.
 * */
void    picman_gradient_segment_range_compress  (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *range_l,
                                               PicmanGradientSegment  *range_r,
                                               gdouble               new_l,
                                               gdouble               new_r);
void    picman_gradient_segment_range_blend     (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *lseg,
                                               PicmanGradientSegment  *rseg,
                                               const PicmanRGB        *rgb1,
                                               const PicmanRGB        *rgb2,
                                               gboolean              blend_colors,
                                               gboolean              blend_opacity);

void    picman_gradient_segment_range_set_blending_function
                                              (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               PicmanGradientSegmentType new_type);

void    picman_gradient_segment_range_set_coloring_type
                                              (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               PicmanGradientSegmentColor new_color);

void    picman_gradient_segment_range_flip      (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               PicmanGradientSegment **final_start_seg,
                                               PicmanGradientSegment **final_end_seg);

void    picman_gradient_segment_range_replicate (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               gint                  replicate_times,
                                               PicmanGradientSegment **final_start_seg,
                                               PicmanGradientSegment **final_end_seg);

void    picman_gradient_segment_range_split_midpoint
                                              (PicmanGradient         *gradient,
                                               PicmanContext          *context,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               PicmanGradientSegment **final_start_seg,
                                               PicmanGradientSegment **final_end_seg);

void    picman_gradient_segment_range_split_uniform
                                              (PicmanGradient         *gradient,
                                               PicmanContext          *context,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               gint                  parts,
                                               PicmanGradientSegment **final_start_seg,
                                               PicmanGradientSegment **final_end_seg);

void    picman_gradient_segment_range_delete    (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg,
                                               PicmanGradientSegment **final_start_seg,
                                               PicmanGradientSegment **final_end_seg);

void    picman_gradient_segment_range_recenter_handles
                                              (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg);
void    picman_gradient_segment_range_redistribute_handles
                                              (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *start_seg,
                                               PicmanGradientSegment  *end_seg);

gdouble picman_gradient_segment_range_move      (PicmanGradient         *gradient,
                                               PicmanGradientSegment  *range_l,
                                               PicmanGradientSegment  *range_r,
                                               gdouble               delta,
                                               gboolean              control_compress);


#endif /* __PICMAN_GRADIENT_H__ */
