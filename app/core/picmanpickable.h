/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanpickable.h
 * Copyright (C) 2004  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PICKABLE_H__
#define __PICMAN_PICKABLE_H__


#define PICMAN_TYPE_PICKABLE               (picman_pickable_interface_get_type ())
#define PICMAN_IS_PICKABLE(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PICKABLE))
#define PICMAN_PICKABLE(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PICKABLE, PicmanPickable))
#define PICMAN_PICKABLE_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_PICKABLE, PicmanPickableInterface))


typedef struct _PicmanPickableInterface PicmanPickableInterface;

struct _PicmanPickableInterface
{
  GTypeInterface base_iface;

  /*  virtual functions  */
  void            (* flush)                 (PicmanPickable *pickable);
  PicmanImage     * (* get_image)             (PicmanPickable *pickable);
  const Babl    * (* get_format)            (PicmanPickable *pickable);
  const Babl    * (* get_format_with_alpha) (PicmanPickable *pickable);
  GeglBuffer    * (* get_buffer)            (PicmanPickable *pickable);
  gboolean        (* get_pixel_at)          (PicmanPickable *pickable,
                                             gint          x,
                                             gint          y,
                                             const Babl   *format,
                                             gpointer      pixel);
  gdouble         (* get_opacity_at)        (PicmanPickable *pickable,
                                             gint          x,
                                             gint          y);
};


GType           picman_pickable_interface_get_type    (void) G_GNUC_CONST;

void            picman_pickable_flush                 (PicmanPickable *pickable);
PicmanImage     * picman_pickable_get_image             (PicmanPickable *pickable);
const Babl    * picman_pickable_get_format            (PicmanPickable *pickable);
const Babl    * picman_pickable_get_format_with_alpha (PicmanPickable *pickable);
GeglBuffer    * picman_pickable_get_buffer            (PicmanPickable *pickable);
gboolean        picman_pickable_get_pixel_at          (PicmanPickable *pickable,
                                                     gint          x,
                                                     gint          y,
                                                     const Babl   *format,
                                                     gpointer      pixel);
gboolean        picman_pickable_get_color_at          (PicmanPickable *pickable,
                                                     gint          x,
                                                     gint          y,
                                                     PicmanRGB      *color);
gdouble         picman_pickable_get_opacity_at        (PicmanPickable *pickable,
                                                     gint          x,
                                                     gint          y);

gboolean        picman_pickable_pick_color            (PicmanPickable *pickable,
                                                     gint          x,
                                                     gint          y,
                                                     gboolean      sample_average,
                                                     gdouble       average_radius,
                                                     PicmanRGB      *color,
                                                     gint         *color_index);


#endif  /* __PICMAN_PICKABLE_H__ */
