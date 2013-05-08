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

#ifndef __PICMAN_LAYER_H__
#define __PICMAN_LAYER_H__


#include "picmandrawable.h"


#define PICMAN_TYPE_LAYER            (picman_layer_get_type ())
#define PICMAN_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LAYER, PicmanLayer))
#define PICMAN_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LAYER, PicmanLayerClass))
#define PICMAN_IS_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LAYER))
#define PICMAN_IS_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LAYER))
#define PICMAN_LAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LAYER, PicmanLayerClass))


typedef struct _PicmanLayerClass PicmanLayerClass;

struct _PicmanLayer
{
  PicmanDrawable          parent_instance;

  gdouble               opacity;          /*  layer opacity              */
  PicmanLayerModeEffects  mode;             /*  layer combination mode     */
  gboolean              lock_alpha;       /*  lock the alpha channel     */

  PicmanLayerMask        *mask;             /*  possible layer mask        */
  gboolean              apply_mask;       /*  controls mask application  */
  gboolean              edit_mask;        /*  edit mask or layer?        */
  gboolean              show_mask;        /*  show mask or layer?        */

  GeglNode             *layer_offset_node;
  GeglNode             *mask_offset_node;

  /*  Floating selections  */
  struct
  {
    PicmanDrawable *drawable;           /*  floating sel is attached to    */
    gboolean      boundary_known;     /*  is the current boundary valid  */
    PicmanBoundSeg *segs;               /*  boundary of floating sel       */
    gint          num_segs;           /*  number of segs in boundary     */
  } fs;
};

struct _PicmanLayerClass
{
  PicmanDrawableClass  parent_class;

  void (* opacity_changed)    (PicmanLayer *layer);
  void (* mode_changed)       (PicmanLayer *layer);
  void (* lock_alpha_changed) (PicmanLayer *layer);
  void (* mask_changed)       (PicmanLayer *layer);
  void (* apply_mask_changed) (PicmanLayer *layer);
  void (* edit_mask_changed)  (PicmanLayer *layer);
  void (* show_mask_changed)  (PicmanLayer *layer);
};


/*  function declarations  */

GType           picman_layer_get_type            (void) G_GNUC_CONST;

PicmanLayer     * picman_layer_new                 (PicmanImage            *image,
                                                gint                  width,
                                                gint                  height,
                                                const Babl           *format,
                                                const gchar          *name,
                                                gdouble               opacity,
                                                PicmanLayerModeEffects  mode);

PicmanLayer     * picman_layer_new_from_buffer     (GeglBuffer           *buffer,
                                                PicmanImage            *dest_image,
                                                const Babl           *format,
                                                const gchar          *name,
                                                gdouble               opacity,
                                                PicmanLayerModeEffects  mode);
PicmanLayer     * picman_layer_new_from_pixbuf     (GdkPixbuf            *pixbuf,
                                                PicmanImage            *dest_image,
                                                const Babl           *format,
                                                const gchar          *name,
                                                gdouble               opacity,
                                                PicmanLayerModeEffects  mode);

PicmanLayer     * picman_layer_get_parent          (PicmanLayer            *layer);

PicmanLayerMask * picman_layer_get_mask            (const PicmanLayer      *layer);
PicmanLayerMask * picman_layer_create_mask         (const PicmanLayer      *layer,
                                                PicmanAddMaskType       mask_type,
                                                PicmanChannel          *channel);
PicmanLayerMask * picman_layer_add_mask            (PicmanLayer            *layer,
                                                PicmanLayerMask        *mask,
                                                gboolean              push_undo,
                                                GError              **error);
void            picman_layer_apply_mask          (PicmanLayer            *layer,
                                                PicmanMaskApplyMode     mode,
                                                gboolean              push_undo);

void            picman_layer_set_apply_mask      (PicmanLayer           *layer,
                                                gboolean             apply,
                                                gboolean             push_undo);
gboolean        picman_layer_get_apply_mask      (const PicmanLayer     *layer);

void            picman_layer_set_edit_mask       (PicmanLayer           *layer,
                                                gboolean             edit);
gboolean        picman_layer_get_edit_mask       (const PicmanLayer     *layer);

void            picman_layer_set_show_mask       (PicmanLayer           *layer,
                                                gboolean             show,
                                                gboolean             push_undo);
gboolean        picman_layer_get_show_mask       (const PicmanLayer     *layer);

void            picman_layer_add_alpha           (PicmanLayer            *layer);
void            picman_layer_flatten             (PicmanLayer            *layer,
                                                PicmanContext          *context);

void            picman_layer_resize_to_image     (PicmanLayer            *layer,
                                                PicmanContext          *context);

PicmanDrawable * picman_layer_get_floating_sel_drawable (const PicmanLayer *layer);
void           picman_layer_set_floating_sel_drawable (PicmanLayer       *layer,
                                                     PicmanDrawable    *drawable);
gboolean        picman_layer_is_floating_sel     (const PicmanLayer      *layer);

void            picman_layer_set_opacity         (PicmanLayer            *layer,
                                                gdouble               opacity,
                                                gboolean              push_undo);
gdouble         picman_layer_get_opacity         (const PicmanLayer      *layer);

void            picman_layer_set_mode            (PicmanLayer            *layer,
                                                PicmanLayerModeEffects  mode,
                                                gboolean              push_undo);
PicmanLayerModeEffects picman_layer_get_mode       (const PicmanLayer      *layer);

void            picman_layer_set_lock_alpha      (PicmanLayer            *layer,
                                                gboolean              lock_alpha,
                                                gboolean              push_undo);
gboolean        picman_layer_get_lock_alpha      (const PicmanLayer      *layer);
gboolean        picman_layer_can_lock_alpha      (const PicmanLayer      *layer);


#endif /* __PICMAN_LAYER_H__ */
