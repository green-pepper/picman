/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancellrendererviewable.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CELL_RENDERER_VIEWABLE_H__
#define __PICMAN_CELL_RENDERER_VIEWABLE_H__


#define PICMAN_TYPE_CELL_RENDERER_VIEWABLE            (picman_cell_renderer_viewable_get_type ())
#define PICMAN_CELL_RENDERER_VIEWABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CELL_RENDERER_VIEWABLE, PicmanCellRendererViewable))
#define PICMAN_CELL_RENDERER_VIEWABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CELL_RENDERER_VIEWABLE, PicmanCellRendererViewableClass))
#define PICMAN_IS_CELL_RENDERER_VIEWABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CELL_RENDERER_VIEWABLE))
#define PICMAN_IS_CELL_RENDERER_VIEWABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CELL_RENDERER_VIEWABLE))
#define PICMAN_CELL_RENDERER_VIEWABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CELL_RENDERER_VIEWABLE, PicmanCellRendererViewableClass))


typedef struct _PicmanCellRendererViewableClass PicmanCellRendererViewableClass;

struct _PicmanCellRendererViewable
{
  GtkCellRenderer   parent_instance;

  PicmanViewRenderer *renderer;
};

struct _PicmanCellRendererViewableClass
{
  GtkCellRendererClass  parent_class;

  gboolean (* pre_clicked) (PicmanCellRendererViewable *cell,
                            const gchar              *path,
                            GdkModifierType           state);
  void     (* clicked)     (PicmanCellRendererViewable *cell,
                            const gchar              *path,
                            GdkModifierType           state);
};


GType             picman_cell_renderer_viewable_get_type    (void) G_GNUC_CONST;
GtkCellRenderer * picman_cell_renderer_viewable_new         (void);
gboolean          picman_cell_renderer_viewable_pre_clicked (PicmanCellRendererViewable *cell,
                                                           const gchar              *path,
                                                           GdkModifierType           state);
void              picman_cell_renderer_viewable_clicked     (PicmanCellRendererViewable *cell,
                                                           const gchar              *path,
                                                           GdkModifierType           state);


#endif /* __PICMAN_CELL_RENDERER_VIEWABLE_H__ */
