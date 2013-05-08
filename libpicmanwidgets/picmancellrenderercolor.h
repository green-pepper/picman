/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancellrenderercolor.h
 * Copyright (C) 2004  Sven Neuman <sven1@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_CELL_RENDERER_COLOR_H__
#define __PICMAN_CELL_RENDERER_COLOR_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_CELL_RENDERER_COLOR            (picman_cell_renderer_color_get_type ())
#define PICMAN_CELL_RENDERER_COLOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CELL_RENDERER_COLOR, PicmanCellRendererColor))
#define PICMAN_CELL_RENDERER_COLOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CELL_RENDERER_COLOR, PicmanCellRendererColorClass))
#define PICMAN_IS_CELL_RENDERER_COLOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CELL_RENDERER_COLOR))
#define PICMAN_IS_CELL_RENDERER_COLOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CELL_RENDERER_COLOR))
#define PICMAN_CELL_RENDERER_COLOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CELL_RENDERER_COLOR, PicmanCellRendererColorClass))


typedef struct _PicmanCellRendererColorClass PicmanCellRendererColorClass;

struct _PicmanCellRendererColor
{
  GtkCellRenderer       parent_instance;

  PicmanRGB               color;
  gboolean              opaque;
  GtkIconSize           size;
  gint                  border;
};

struct _PicmanCellRendererColorClass
{
  GtkCellRendererClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType             picman_cell_renderer_color_get_type (void) G_GNUC_CONST;

GtkCellRenderer * picman_cell_renderer_color_new      (void);


G_END_DECLS

#endif /* __PICMAN_CELL_RENDERER_COLOR_H__ */
