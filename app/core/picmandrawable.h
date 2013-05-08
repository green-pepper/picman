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

#ifndef __PICMAN_DRAWABLE_H__
#define __PICMAN_DRAWABLE_H__


#include "picmanitem.h"


#define PICMAN_TYPE_DRAWABLE            (picman_drawable_get_type ())
#define PICMAN_DRAWABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DRAWABLE, PicmanDrawable))
#define PICMAN_DRAWABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DRAWABLE, PicmanDrawableClass))
#define PICMAN_IS_DRAWABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DRAWABLE))
#define PICMAN_IS_DRAWABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DRAWABLE))
#define PICMAN_DRAWABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DRAWABLE, PicmanDrawableClass))


typedef struct _PicmanDrawablePrivate PicmanDrawablePrivate;
typedef struct _PicmanDrawableClass   PicmanDrawableClass;

struct _PicmanDrawable
{
  PicmanItem             parent_instance;

  PicmanDrawablePrivate *private;
};

struct _PicmanDrawableClass
{
  PicmanItemClass  parent_class;

  /*  signals  */
  void          (* update)                (PicmanDrawable         *drawable,
                                           gint                  x,
                                           gint                  y,
                                           gint                  width,
                                           gint                  height);
  void          (* alpha_changed)         (PicmanDrawable         *drawable);

  /*  virtual functions  */
  gint64        (* estimate_memsize)      (const PicmanDrawable   *drawable,
                                           gint                  width,
                                           gint                  height);
  void          (* invalidate_boundary)   (PicmanDrawable         *drawable);
  void          (* get_active_components) (const PicmanDrawable   *drawable,
                                           gboolean             *active);
  PicmanComponentMask (* get_active_mask)   (const PicmanDrawable   *drawable);
  void          (* convert_type)          (PicmanDrawable         *drawable,
                                           PicmanImage            *dest_image,
                                           const Babl           *new_format,
                                           PicmanImageBaseType     new_base_type,
                                           PicmanPrecision         new_precision,
                                           gint                  layer_dither_type,
                                           gint                  mask_dither_type,
                                           gboolean              push_undo);
  void          (* apply_buffer)          (PicmanDrawable         *drawable,
                                           GeglBuffer           *buffer,
                                           const GeglRectangle  *buffer_region,
                                           gboolean              push_undo,
                                           const gchar          *undo_desc,
                                           gdouble               opacity,
                                           PicmanLayerModeEffects  mode,
                                           GeglBuffer           *base_buffer,
                                           gint                  base_x,
                                           gint                  base_y);
  void          (* replace_buffer)        (PicmanDrawable         *drawable,
                                           GeglBuffer           *buffer,
                                           const GeglRectangle  *buffer_region,
                                           gboolean              push_undo,
                                           const gchar          *undo_desc,
                                           gdouble               opacity,
                                           GeglBuffer           *mask,
                                           const GeglRectangle  *mask_region,
                                           gint                  x,
                                           gint                  y);
  GeglBuffer  * (* get_buffer)            (PicmanDrawable         *drawable);
  void          (* set_buffer)            (PicmanDrawable         *drawable,
                                           gboolean              push_undo,
                                           const gchar          *undo_desc,
                                           GeglBuffer           *buffer,
                                           gint                  offset_x,
                                           gint                  offset_y);
  void          (* push_undo)             (PicmanDrawable         *drawable,
                                           const gchar          *undo_desc,
                                           GeglBuffer           *buffer,
                                           gint                  x,
                                           gint                  y,
                                           gint                  width,
                                           gint                  height);
  void          (* swap_pixels)           (PicmanDrawable         *drawable,
                                           GeglBuffer           *buffer,
                                           gint                  x,
                                           gint                  y);
};


GType           picman_drawable_get_type           (void) G_GNUC_CONST;

PicmanDrawable  * picman_drawable_new                (GType               type,
                                                  PicmanImage          *image,
                                                  const gchar        *name,
                                                  gint                offset_x,
                                                  gint                offset_y,
                                                  gint                width,
                                                  gint                height,
                                                  const Babl         *format);

gint64          picman_drawable_estimate_memsize   (const PicmanDrawable *drawable,
                                                  gint                width,
                                                  gint                height);

void            picman_drawable_update             (PicmanDrawable       *drawable,
                                                  gint                x,
                                                  gint                y,
                                                  gint                width,
                                                  gint                height);
void            picman_drawable_alpha_changed      (PicmanDrawable       *drawable);

void           picman_drawable_invalidate_boundary (PicmanDrawable       *drawable);
void         picman_drawable_get_active_components (const PicmanDrawable *drawable,
                                                  gboolean           *active);
PicmanComponentMask picman_drawable_get_active_mask  (const PicmanDrawable *drawable);

void            picman_drawable_convert_type       (PicmanDrawable       *drawable,
                                                  PicmanImage          *dest_image,
                                                  PicmanImageBaseType   new_base_type,
                                                  PicmanPrecision       new_precision,
                                                  gint                layer_dither_type,
                                                  gint                mask_dither_type,
                                                  gboolean            push_undo);

void            picman_drawable_apply_buffer       (PicmanDrawable        *drawable,
                                                  GeglBuffer          *buffer,
                                                  const GeglRectangle *buffer_rect,
                                                  gboolean             push_undo,
                                                  const gchar         *undo_desc,
                                                  gdouble              opacity,
                                                  PicmanLayerModeEffects mode,
                                                  GeglBuffer          *base_buffer,
                                                  gint                 base_x,
                                                  gint                 base_y);
void            picman_drawable_replace_buffer     (PicmanDrawable        *drawable,
                                                  GeglBuffer          *buffer,
                                                  const GeglRectangle *buffer_region,
                                                  gboolean             push_undo,
                                                  const gchar         *undo_desc,
                                                  gdouble              opacity,
                                                  GeglBuffer          *mask,
                                                  const GeglRectangle *mask_region,
                                                  gint                 x,
                                                  gint                 y);

GeglBuffer    * picman_drawable_get_buffer         (PicmanDrawable       *drawable);
void            picman_drawable_set_buffer         (PicmanDrawable       *drawable,
                                                  gboolean            push_undo,
                                                  const gchar        *undo_desc,
                                                  GeglBuffer         *buffer);
void            picman_drawable_set_buffer_full    (PicmanDrawable       *drawable,
                                                  gboolean            push_undo,
                                                  const gchar        *undo_desc,
                                                  GeglBuffer         *buffer,
                                                  gint                offset_x,
                                                  gint                offset_y);

GeglNode      * picman_drawable_get_source_node    (PicmanDrawable       *drawable);
GeglNode      * picman_drawable_get_mode_node      (PicmanDrawable       *drawable);

void            picman_drawable_swap_pixels        (PicmanDrawable       *drawable,
                                                  GeglBuffer         *buffer,
                                                  gint                x,
                                                  gint                y);

void            picman_drawable_push_undo          (PicmanDrawable       *drawable,
                                                  const gchar        *undo_desc,
                                                  GeglBuffer         *buffer,
                                                  gint                x,
                                                  gint                y,
                                                  gint                width,
                                                  gint                height);

void            picman_drawable_fill               (PicmanDrawable       *drawable,
                                                  const PicmanRGB      *color,
                                                  const PicmanPattern  *pattern);
void            picman_drawable_fill_by_type       (PicmanDrawable       *drawable,
                                                  PicmanContext        *context,
                                                  PicmanFillType        fill_type);

const Babl    * picman_drawable_get_format         (const PicmanDrawable *drawable);
const Babl    * picman_drawable_get_format_with_alpha
                                                 (const PicmanDrawable *drawable);
const Babl    * picman_drawable_get_format_without_alpha
                                                 (const PicmanDrawable *drawable);
gboolean        picman_drawable_get_linear         (const PicmanDrawable *drawable);
gboolean        picman_drawable_has_alpha          (const PicmanDrawable *drawable);
PicmanImageBaseType picman_drawable_get_base_type    (const PicmanDrawable *drawable);
PicmanPrecision   picman_drawable_get_precision      (const PicmanDrawable *drawable);
gboolean        picman_drawable_is_rgb             (const PicmanDrawable *drawable);
gboolean        picman_drawable_is_gray            (const PicmanDrawable *drawable);
gboolean        picman_drawable_is_indexed         (const PicmanDrawable *drawable);

const guchar  * picman_drawable_get_colormap       (const PicmanDrawable *drawable);

PicmanLayer    * picman_drawable_get_floating_sel    (const PicmanDrawable *drawable);
void           picman_drawable_attach_floating_sel (PicmanDrawable       *drawable,
                                                  PicmanLayer          *floating_sel);
void           picman_drawable_detach_floating_sel (PicmanDrawable       *drawable);
PicmanFilter *
           picman_drawable_get_floating_sel_filter (PicmanDrawable       *drawable);


#endif /* __PICMAN_DRAWABLE_H__ */
