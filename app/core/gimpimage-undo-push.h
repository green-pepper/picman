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

#ifndef __PICMAN_IMAGE_UNDO_PUSH_H__
#define __PICMAN_IMAGE_UNDO_PUSH_H__


/*  image undos  */

PicmanUndo * picman_image_undo_push_image_type          (PicmanImage     *image,
                                                     const gchar   *undo_desc);
PicmanUndo * picman_image_undo_push_image_precision     (PicmanImage     *image,
                                                     const gchar   *undo_desc);
PicmanUndo * picman_image_undo_push_image_size          (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     gint           previous_origin_x,
                                                     gint           previous_origin_y,
                                                     gint           previous_width,
                                                     gint           prevoius_height);
PicmanUndo * picman_image_undo_push_image_resolution    (PicmanImage     *image,
                                                     const gchar   *undo_desc);
PicmanUndo * picman_image_undo_push_image_grid          (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanGrid      *grid);
PicmanUndo * picman_image_undo_push_image_colormap      (PicmanImage     *image,
                                                     const gchar   *undo_desc);
PicmanUndo * picman_image_undo_push_image_parasite      (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     const PicmanParasite *parasite);
PicmanUndo * picman_image_undo_push_image_parasite_remove (PicmanImage   *image,
                                                     const gchar   *undo_desc,
                                                     const gchar   *name);


/*  guide & sample point undos  */

PicmanUndo * picman_image_undo_push_guide               (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanGuide     *guide);
PicmanUndo * picman_image_undo_push_sample_point        (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanSamplePoint *sample_point);


/*  drawable undos  */

PicmanUndo * picman_image_undo_push_drawable            (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanDrawable  *drawable,
                                                     GeglBuffer    *buffer,
                                                     gint           x,
                                                     gint           y);
PicmanUndo * picman_image_undo_push_drawable_mod        (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanDrawable  *drawable,
                                                     gboolean       copy_buffer);


/*  mask undos  */

PicmanUndo * picman_image_undo_push_mask                (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanChannel   *mask);
PicmanUndo * picman_image_undo_push_mask_precision      (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanChannel   *mask);


/*  item undos  */

PicmanUndo * picman_image_undo_push_item_reorder        (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_rename         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_displace       (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_visibility     (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_linked         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_lock_content   (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_lock_position  (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item);
PicmanUndo * picman_image_undo_push_item_parasite       (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item,
                                                     const PicmanParasite *parasite);
PicmanUndo * picman_image_undo_push_item_parasite_remove(PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanItem      *item,
                                                     const gchar   *name);


/*  layer undos  */

PicmanUndo * picman_image_undo_push_layer_add           (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer,
                                                     PicmanLayer     *prev_layer);
PicmanUndo * picman_image_undo_push_layer_remove        (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer,
                                                     PicmanLayer     *prev_parent,
                                                     gint           prev_position,
                                                     PicmanLayer     *prev_layer);
PicmanUndo * picman_image_undo_push_layer_mode          (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer);
PicmanUndo * picman_image_undo_push_layer_opacity       (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer);
PicmanUndo * picman_image_undo_push_layer_lock_alpha    (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer);


/*  group layer undos  */

PicmanUndo * picman_image_undo_push_group_layer_suspend (PicmanImage      *image,
                                                     const gchar    *undo_desc,
                                                     PicmanGroupLayer *group);
PicmanUndo * picman_image_undo_push_group_layer_resume  (PicmanImage      *image,
                                                     const gchar    *undo_desc,
                                                     PicmanGroupLayer *group);
PicmanUndo * picman_image_undo_push_group_layer_convert (PicmanImage      *image,
                                                     const gchar    *undo_desc,
                                                     PicmanGroupLayer *group);


/*  text layer undos  */

PicmanUndo * picman_image_undo_push_text_layer          (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanTextLayer *layer,
                                                     const GParamSpec *pspec);
PicmanUndo * picman_image_undo_push_text_layer_modified (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanTextLayer *layer);
PicmanUndo * picman_image_undo_push_text_layer_convert  (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanTextLayer *layer);


/*  layer mask undos  */

PicmanUndo * picman_image_undo_push_layer_mask_add      (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer,
                                                     PicmanLayerMask *mask);
PicmanUndo * picman_image_undo_push_layer_mask_remove   (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer,
                                                     PicmanLayerMask *mask);
PicmanUndo * picman_image_undo_push_layer_mask_apply    (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer);
PicmanUndo * picman_image_undo_push_layer_mask_show     (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *layer);


/*  channel undos  */

PicmanUndo * picman_image_undo_push_channel_add         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanChannel   *channel,
                                                     PicmanChannel   *prev_channel);
PicmanUndo * picman_image_undo_push_channel_remove      (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanChannel   *channel,
                                                     PicmanChannel   *prev_parent,
                                                     gint           prev_position,
                                                     PicmanChannel   *prev_channel);
PicmanUndo * picman_image_undo_push_channel_color       (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanChannel   *channel);


/*  vectors undos  */

PicmanUndo * picman_image_undo_push_vectors_add         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanVectors   *vectors,
                                                     PicmanVectors   *prev_vectors);
PicmanUndo * picman_image_undo_push_vectors_remove      (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanVectors   *vectors,
                                                     PicmanVectors   *prev_parent,
                                                     gint           prev_position,
                                                     PicmanVectors   *prev_vectors);
PicmanUndo * picman_image_undo_push_vectors_mod         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanVectors   *vectors);


/*  floating selection undos  */

PicmanUndo * picman_image_undo_push_fs_to_layer         (PicmanImage     *image,
                                                     const gchar   *undo_desc,
                                                     PicmanLayer     *floating_layer);


/*  EEK undo  */

PicmanUndo * picman_image_undo_push_cantundo            (PicmanImage     *image,
                                                     const gchar   *undo_desc);


#endif  /* __PICMAN_IMAGE_UNDO_PUSH_H__ */
