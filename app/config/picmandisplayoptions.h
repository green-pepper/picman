/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanDisplayOptions
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_DISPLAY_OPTIONS_H__
#define __PICMAN_DISPLAY_OPTIONS_H__


#define PICMAN_TYPE_DISPLAY_OPTIONS            (picman_display_options_get_type ())
#define PICMAN_TYPE_DISPLAY_OPTIONS_FULLSCREEN (picman_display_options_fullscreen_get_type ())
#define PICMAN_TYPE_DISPLAY_OPTIONS_NO_IMAGE   (picman_display_options_no_image_get_type ())

#define PICMAN_DISPLAY_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DISPLAY_OPTIONS, PicmanDisplayOptions))
#define PICMAN_DISPLAY_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DISPLAY_OPTIONS, PicmanDisplayOptionsClass))
#define PICMAN_IS_DISPLAY_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DISPLAY_OPTIONS))
#define PICMAN_IS_DISPLAY_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DISPLAY_OPTIONS))
#define PICMAN_DISPLAY_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DISPLAY_OPTIONS, PicmanDisplayOptionsClass))


typedef struct _PicmanDisplayOptionsClass PicmanDisplayOptionsClass;

struct _PicmanDisplayOptions
{
  GObject                parent_instance;

  /*  PicmanImageWindow options  */
  gboolean               show_menubar;
  gboolean               show_statusbar;

  /*  PicmanDisplayShell options  */
  gboolean               show_rulers;
  gboolean               show_scrollbars;

  /*  PicmanCanvas options  */
  gboolean               show_selection;
  gboolean               show_layer_boundary;
  gboolean               show_guides;
  gboolean               show_grid;
  gboolean               show_sample_points;
  PicmanCanvasPaddingMode  padding_mode;
  PicmanRGB                padding_color;
  gboolean               padding_mode_set;
};

struct _PicmanDisplayOptionsClass
{
  GObjectClass           parent_class;
};


GType  picman_display_options_get_type            (void) G_GNUC_CONST;
GType  picman_display_options_fullscreen_get_type (void) G_GNUC_CONST;
GType  picman_display_options_no_image_get_type   (void) G_GNUC_CONST;


#endif /* __PICMAN_DISPLAY_OPTIONS_H__ */
