/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfgbgview.h
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_FG_BG_VIEW_H__
#define __PICMAN_FG_BG_VIEW_H__


#define PICMAN_TYPE_FG_BG_VIEW            (picman_fg_bg_view_get_type ())
#define PICMAN_FG_BG_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FG_BG_VIEW, PicmanFgBgView))
#define PICMAN_FG_BG_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FG_BG_VIEW, PicmanFgBgViewClass))
#define PICMAN_IS_FG_BG_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FG_BG_VIEW))
#define PICMAN_IS_FG_BG_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FG_BG_VIEW))
#define PICMAN_FG_BG_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FG_BG_VIEW, PicmanFgBgViewClass))


typedef struct _PicmanFgBgViewClass PicmanFgBgViewClass;

struct _PicmanFgBgView
{
  GtkWidget    parent_instance;

  PicmanContext *context;
};

struct _PicmanFgBgViewClass
{
  GtkWidgetClass  parent_class;
};


GType       picman_fg_bg_view_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_fg_bg_view_new         (PicmanContext  *context);
void        picman_fg_bg_view_set_context (PicmanFgBgView *view,
                                         PicmanContext  *context);


#endif  /*  __PICMAN_FG_BG_VIEW_H__  */
