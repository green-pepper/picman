/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanprojectable.h
 * Copyright (C) 2008  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PROJECTABLE_H__
#define __PICMAN_PROJECTABLE_H__


#define PICMAN_TYPE_PROJECTABLE               (picman_projectable_interface_get_type ())
#define PICMAN_IS_PROJECTABLE(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PROJECTABLE))
#define PICMAN_PROJECTABLE(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PROJECTABLE, PicmanProjectable))
#define PICMAN_PROJECTABLE_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_PROJECTABLE, PicmanProjectableInterface))


typedef struct _PicmanProjectableInterface PicmanProjectableInterface;

struct _PicmanProjectableInterface
{
  GTypeInterface base_iface;

  /*  signals  */
  void         (* invalidate)         (PicmanProjectable *projectable,
                                       gint             x,
                                       gint             y,
                                       gint             width,
                                       gint             height);
  void         (* flush)              (PicmanProjectable *projectable,
                                       gboolean         invalidate_preview);
  void         (* structure_changed)  (PicmanProjectable *projectable);

  /*  virtual functions  */
  PicmanImage  * (* get_image)          (PicmanProjectable *projectable);
  const Babl * (* get_format)         (PicmanProjectable *projectable);
  void         (* get_offset)         (PicmanProjectable *projectable,
                                       gint            *x,
                                       gint            *y);
  void         (* get_size)           (PicmanProjectable *projectable,
                                       gint            *width,
                                       gint            *height);
  GeglNode   * (* get_graph)          (PicmanProjectable *projectable);
  void         (* invalidate_preview) (PicmanProjectable *projectable);
};


GType        picman_projectable_interface_get_type (void) G_GNUC_CONST;

void         picman_projectable_invalidate         (PicmanProjectable *projectable,
                                                  gint             x,
                                                  gint             y,
                                                  gint             width,
                                                  gint             height);
void         picman_projectable_flush              (PicmanProjectable *projectable,
                                                  gboolean         preview_invalidated);
void         picman_projectable_structure_changed  (PicmanProjectable *projectable);

PicmanImage  * picman_projectable_get_image          (PicmanProjectable *projectable);
const Babl * picman_projectable_get_format         (PicmanProjectable *projectable);
void         picman_projectable_get_offset         (PicmanProjectable *projectable,
                                                  gint            *x,
                                                  gint            *y);
void         picman_projectable_get_size           (PicmanProjectable *projectable,
                                                  gint            *width,
                                                  gint            *height);
GeglNode   * picman_projectable_get_graph          (PicmanProjectable *projectable);
void         picman_projectable_invalidate_preview (PicmanProjectable *projectable);


#endif  /* __PICMAN_PROJECTABLE_H__ */
