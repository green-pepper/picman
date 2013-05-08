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

#ifndef __PICMAN_COLOR_PANEL_H__
#define __PICMAN_COLOR_PANEL_H__


#define PICMAN_TYPE_COLOR_PANEL            (picman_color_panel_get_type ())
#define PICMAN_COLOR_PANEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_PANEL, PicmanColorPanel))
#define PICMAN_COLOR_PANEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_PANEL, PicmanColorPanelClass))
#define PICMAN_IS_COLOR_PANEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_PANEL))
#define PICMAN_IS_COLOR_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_PANEL))
#define PICMAN_COLOR_PANEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_PANEL, PicmanColorPanelClass))


typedef struct _PicmanColorPanelClass PicmanColorPanelClass;

struct _PicmanColorPanel
{
  PicmanColorButton  parent_instance;

  PicmanContext     *context;
  GtkWidget       *color_dialog;
};

struct _PicmanColorPanelClass
{
  PicmanColorButtonClass  parent_class;
};


GType       picman_color_panel_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_color_panel_new         (const gchar       *title,
                                          const PicmanRGB     *color,
                                          PicmanColorAreaType  type,
                                          gint               width,
                                          gint               height);

void        picman_color_panel_set_context (PicmanColorPanel    *panel,
                                          PicmanContext       *context);


#endif  /*  __PICMAN_COLOR_PANEL_H__  */
