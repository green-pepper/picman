/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancellrendererdashes.h
 * Copyright (C) 2005 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CELL_RENDERER_DASHES_H__
#define __PICMAN_CELL_RENDERER_DASHES_H__


#define PICMAN_TYPE_CELL_RENDERER_DASHES            (picman_cell_renderer_dashes_get_type ())
#define PICMAN_CELL_RENDERER_DASHES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CELL_RENDERER_DASHES, PicmanCellRendererDashes))
#define PICMAN_CELL_RENDERER_DASHES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CELL_RENDERER_DASHES, PicmanCellRendererDashesClass))
#define PICMAN_IS_CELL_RENDERER_DASHES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CELL_RENDERER_DASHES))
#define PICMAN_IS_CELL_RENDERER_DASHES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CELL_RENDERER_DASHES))
#define PICMAN_CELL_RENDERER_DASHES_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CELL_RENDERER_DASHES, PicmanCellRendererDashesClass))


typedef struct _PicmanCellRendererDashesClass PicmanCellRendererDashesClass;

struct _PicmanCellRendererDashes
{
  GtkCellRenderer   parent_instance;

  gboolean         *segments;
};

struct _PicmanCellRendererDashesClass
{
  GtkCellRendererClass  parent_class;
};


GType             picman_cell_renderer_dashes_get_type (void) G_GNUC_CONST;

GtkCellRenderer * picman_cell_renderer_dashes_new      (void);


#endif /* __PICMAN_CELL_RENDERER_DASHES_H__ */
