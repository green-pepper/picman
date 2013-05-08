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

#ifndef __PICMAN_COLOR_BAR_H__
#define __PICMAN_COLOR_BAR_H__


#define PICMAN_TYPE_COLOR_BAR            (picman_color_bar_get_type ())
#define PICMAN_COLOR_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_BAR, PicmanColorBar))
#define PICMAN_COLOR_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_BAR, PicmanColorBarClass))
#define PICMAN_IS_COLOR_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_BAR))
#define PICMAN_IS_COLOR_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_BAR))
#define PICMAN_COLOR_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_BAR, PicmanColorBarClass))


typedef struct _PicmanColorBarClass  PicmanColorBarClass;

struct _PicmanColorBar
{
  GtkEventBox     parent_class;

  GtkOrientation  orientation;
  guchar          buf[3 * 256];
};

struct _PicmanColorBarClass
{
  GtkEventBoxClass  parent_class;
};


GType       picman_color_bar_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_color_bar_new         (GtkOrientation        orientation);

void        picman_color_bar_set_color   (PicmanColorBar         *bar,
                                        const PicmanRGB        *color);
void        picman_color_bar_set_channel (PicmanColorBar         *bar,
                                        PicmanHistogramChannel  channel);
void        picman_color_bar_set_buffers (PicmanColorBar         *bar,
                                        const guchar         *red,
                                        const guchar         *green,
                                        const guchar         *blue);


#endif  /*  __PICMAN_COLOR_BAR_H__  */
