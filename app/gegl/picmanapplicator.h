/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanapplicator.h
 * Copyright (C) 2012-2013 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_APPLICATOR_H__
#define __PICMAN_APPLICATOR_H__


#define PICMAN_TYPE_APPLICATOR            (picman_applicator_get_type ())
#define PICMAN_APPLICATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_APPLICATOR, PicmanApplicator))
#define PICMAN_APPLICATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_APPLICATOR, PicmanApplicatorClass))
#define PICMAN_IS_APPLICATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_APPLICATOR))
#define PICMAN_IS_APPLICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_APPLICATOR))
#define PICMAN_APPLICATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_APPLICATOR, PicmanApplicatorClass))


typedef struct _PicmanApplicatorClass PicmanApplicatorClass;

struct _PicmanApplicator
{
  GObject               parent_instance;

  GeglNode             *node;
  GeglNode             *input_node;
  GeglNode             *aux_node;
  GeglNode             *output_node;

  GeglBuffer           *apply_buffer;
  GeglNode             *apply_src_node;

  gint                  apply_offset_x;
  gint                  apply_offset_y;
  GeglNode             *apply_offset_node;

  gdouble               opacity;
  PicmanLayerModeEffects  paint_mode;
  gboolean              linear;
  GeglNode             *mode_node;

  PicmanComponentMask     affect;
  GeglNode             *affect_node;

  GeglBuffer           *src_buffer;
  GeglNode             *src_node;

  GeglBuffer           *dest_buffer;
  GeglNode             *dest_node;

  GeglBuffer           *mask_buffer;
  GeglNode             *mask_node;

  gint                  mask_offset_x;
  gint                  mask_offset_y;
  GeglNode             *mask_offset_node;
};

struct _PicmanApplicatorClass
{
  GObjectClass  parent_class;
};


GType        picman_applicator_get_type         (void) G_GNUC_CONST;

PicmanApplicator * picman_applicator_new          (GeglNode             *parent,
                                               gboolean              linear);

void         picman_applicator_set_src_buffer   (PicmanApplicator       *applicator,
                                               GeglBuffer           *dest_buffer);
void         picman_applicator_set_dest_buffer  (PicmanApplicator       *applicator,
                                               GeglBuffer           *dest_buffer);

void         picman_applicator_set_mask_buffer  (PicmanApplicator       *applicator,
                                               GeglBuffer           *mask_buffer);
void         picman_applicator_set_mask_offset  (PicmanApplicator       *applicator,
                                               gint                  mask_offset_x,
                                               gint                  mask_offset_y);

void         picman_applicator_set_apply_buffer (PicmanApplicator       *applicator,
                                               GeglBuffer           *apply_buffer);
void         picman_applicator_set_apply_offset (PicmanApplicator       *applicator,
                                               gint                  apply_offset_x,
                                               gint                  apply_offset_y);

void         picman_applicator_set_mode         (PicmanApplicator       *applicator,
                                               gdouble               opacity,
                                               PicmanLayerModeEffects  paint_mode);
void         picman_applicator_set_affect       (PicmanApplicator       *applicator,
                                               PicmanComponentMask     affect);

void         picman_applicator_blit             (PicmanApplicator       *applicator,
                                               const GeglRectangle  *rect);

GeglBuffer * picman_applicator_dup_apply_buffer (PicmanApplicator       *applicator,
                                               const GeglRectangle  *rect);


#endif  /*  __PICMAN_APPLICATOR_H__  */
