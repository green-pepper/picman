/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasitem.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CANVAS_ITEM_H__
#define __PICMAN_CANVAS_ITEM_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_CANVAS_ITEM            (picman_canvas_item_get_type ())
#define PICMAN_CANVAS_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_ITEM, PicmanCanvasItem))
#define PICMAN_CANVAS_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_ITEM, PicmanCanvasItemClass))
#define PICMAN_IS_CANVAS_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_ITEM))
#define PICMAN_IS_CANVAS_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_ITEM))
#define PICMAN_CANVAS_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_ITEM, PicmanCanvasItemClass))


typedef struct _PicmanCanvasItemClass PicmanCanvasItemClass;

struct _PicmanCanvasItem
{
  PicmanObject       parent_instance;
};

struct _PicmanCanvasItemClass
{
  PicmanObjectClass  parent_class;

  /*  signals  */
  void             (* update)      (PicmanCanvasItem   *item,
                                    cairo_region_t   *region);

  /*  virtual functions  */
  void             (* draw)        (PicmanCanvasItem   *item,
                                    cairo_t          *cr);
  cairo_region_t * (* get_extents) (PicmanCanvasItem   *item);

  void             (* stroke)      (PicmanCanvasItem   *item,
                                    cairo_t          *cr);
  void             (* fill)        (PicmanCanvasItem   *item,
                                    cairo_t          *cr);

  gboolean         (* hit)         (PicmanCanvasItem   *item,
                                    gdouble           x,
                                    gdouble           y);
};


GType            picman_canvas_item_get_type         (void) G_GNUC_CONST;

PicmanDisplayShell * picman_canvas_item_get_shell      (PicmanCanvasItem   *item);
PicmanImage      * picman_canvas_item_get_image        (PicmanCanvasItem   *item);
GtkWidget      * picman_canvas_item_get_canvas       (PicmanCanvasItem   *item);

void             picman_canvas_item_draw             (PicmanCanvasItem   *item,
                                                    cairo_t          *cr);
cairo_region_t * picman_canvas_item_get_extents      (PicmanCanvasItem   *item);

gboolean         picman_canvas_item_hit              (PicmanCanvasItem   *item,
                                                    gdouble           x,
                                                    gdouble           y);

void             picman_canvas_item_set_visible      (PicmanCanvasItem   *item,
                                                    gboolean          visible);
gboolean         picman_canvas_item_get_visible      (PicmanCanvasItem   *item);

void             picman_canvas_item_set_line_cap     (PicmanCanvasItem   *item,
                                                    cairo_line_cap_t  line_cap);

void             picman_canvas_item_set_highlight    (PicmanCanvasItem   *item,
                                                    gboolean          highlight);
gboolean         picman_canvas_item_get_highlight    (PicmanCanvasItem   *item);

void             picman_canvas_item_begin_change     (PicmanCanvasItem   *item);
void             picman_canvas_item_end_change       (PicmanCanvasItem   *item);

void             picman_canvas_item_suspend_stroking (PicmanCanvasItem   *item);
void             picman_canvas_item_resume_stroking  (PicmanCanvasItem   *item);

void             picman_canvas_item_suspend_filling  (PicmanCanvasItem   *item);
void             picman_canvas_item_resume_filling   (PicmanCanvasItem   *item);

void             picman_canvas_item_transform        (PicmanCanvasItem   *item,
                                                    cairo_t          *cr);
void             picman_canvas_item_transform_xy     (PicmanCanvasItem   *item,
                                                    gdouble           x,
                                                    gdouble           y,
                                                    gint             *tx,
                                                    gint             *ty);
void             picman_canvas_item_transform_xy_f   (PicmanCanvasItem   *item,
                                                    gdouble           x,
                                                    gdouble           y,
                                                    gdouble          *tx,
                                                    gdouble          *ty);


/*  protected  */

void             _picman_canvas_item_update          (PicmanCanvasItem   *item,
                                                    cairo_region_t   *region);
gboolean         _picman_canvas_item_needs_update    (PicmanCanvasItem   *item);
void             _picman_canvas_item_stroke          (PicmanCanvasItem   *item,
                                                    cairo_t          *cr);
void             _picman_canvas_item_fill            (PicmanCanvasItem   *item,
                                                    cairo_t          *cr);


#endif /* __PICMAN_CANVAS_ITEM_H__ */
