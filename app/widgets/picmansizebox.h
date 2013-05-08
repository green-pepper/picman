/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansizebox.h
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_SIZE_BOX_H__
#define __PICMAN_SIZE_BOX_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_SIZE_BOX            (picman_size_box_get_type ())
#define PICMAN_SIZE_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SIZE_BOX, PicmanSizeBox))
#define PICMAN_SIZE_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SIZE_BOX, PicmanSizeBoxClass))
#define PICMAN_IS_SIZE_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SIZE_BOX))
#define PICMAN_IS_SIZE_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SIZE_BOX))
#define PICMAN_SIZE_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SIZE_BOX, PicmanSizeBoxClass))


typedef struct _PicmanSizeBoxClass  PicmanSizeBoxClass;

struct _PicmanSizeBox
{
  GtkBox        parent_instance;

  GtkSizeGroup *size_group;

  gint          width;
  gint          height;
  PicmanUnit      unit;
  gdouble       xresolution;
  gdouble       yresolution;
  PicmanUnit      resolution_unit;

  gboolean      edit_resolution;
};

struct _PicmanSizeBoxClass
{
  GtkBoxClass  parent_class;
};


GType       picman_size_box_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __PICMAN_SIZE_BOX_H__ */
