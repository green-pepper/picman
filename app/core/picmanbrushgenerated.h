/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * brush_generated module Copyright 1998 Jay Cox <jaycox@earthlink.net>
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

#ifndef __PICMAN_BRUSH_GENERATED_H__
#define __PICMAN_BRUSH_GENERATED_H__


#include "picmanbrush.h"


#define PICMAN_TYPE_BRUSH_GENERATED            (picman_brush_generated_get_type ())
#define PICMAN_BRUSH_GENERATED(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_GENERATED, PicmanBrushGenerated))
#define PICMAN_BRUSH_GENERATED_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_GENERATED, PicmanBrushGeneratedClass))
#define PICMAN_IS_BRUSH_GENERATED(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_GENERATED))
#define PICMAN_IS_BRUSH_GENERATED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_GENERATED))
#define PICMAN_BRUSH_GENERATED_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_GENERATED, PicmanBrushGeneratedClass))


typedef struct _PicmanBrushGeneratedClass PicmanBrushGeneratedClass;

struct _PicmanBrushGenerated
{
  PicmanBrush               parent_instance;

  PicmanBrushGeneratedShape shape;
  gfloat                  radius;
  gint                    spikes;       /* 2 - 20     */
  gfloat                  hardness;     /* 0.0 - 1.0  */
  gfloat                  aspect_ratio; /* y/x        */
  gfloat                  angle;        /* in degrees */
};

struct _PicmanBrushGeneratedClass
{
  PicmanBrushClass  parent_class;
};


GType       picman_brush_generated_get_type     (void) G_GNUC_CONST;

PicmanData  * picman_brush_generated_new          (const gchar             *name,
                                               PicmanBrushGeneratedShape  shape,
                                               gfloat                   radius,
                                               gint                     spikes,
                                               gfloat                   hardness,
                                               gfloat                   aspect_ratio,
                                               gfloat                   angle);

PicmanBrushGeneratedShape
        picman_brush_generated_set_shape        (PicmanBrushGenerated      *brush,
                                               PicmanBrushGeneratedShape  shape);
gfloat  picman_brush_generated_set_radius       (PicmanBrushGenerated      *brush,
                                               gfloat                   radius);
gint    picman_brush_generated_set_spikes       (PicmanBrushGenerated      *brush,
                                               gint                     spikes);
gfloat  picman_brush_generated_set_hardness     (PicmanBrushGenerated      *brush,
                                               gfloat                   hardness);
gfloat  picman_brush_generated_set_aspect_ratio (PicmanBrushGenerated      *brush,
                                               gfloat                   ratio);
gfloat  picman_brush_generated_set_angle        (PicmanBrushGenerated      *brush,
                                               gfloat                   angle);

PicmanBrushGeneratedShape
        picman_brush_generated_get_shape        (const PicmanBrushGenerated *brush);
gfloat  picman_brush_generated_get_radius       (const PicmanBrushGenerated *brush);
gint    picman_brush_generated_get_spikes       (const PicmanBrushGenerated *brush);
gfloat  picman_brush_generated_get_hardness     (const PicmanBrushGenerated *brush);
gfloat  picman_brush_generated_get_aspect_ratio (const PicmanBrushGenerated *brush);
gfloat  picman_brush_generated_get_angle        (const PicmanBrushGenerated *brush);


#endif  /*  __PICMAN_BRUSH_GENERATED_H__  */
