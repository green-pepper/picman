/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorscale.h
 * Copyright (C) 2002  Sven Neumann <sven@picman.org>
 *                     Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_SCALE_H__
#define __PICMAN_COLOR_SCALE_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_COLOR_SCALE            (picman_color_scale_get_type ())
#define PICMAN_COLOR_SCALE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_SCALE, PicmanColorScale))
#define PICMAN_COLOR_SCALE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_SCALE, PicmanColorScaleClass))
#define PICMAN_IS_COLOR_SCALE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_SCALE))
#define PICMAN_IS_COLOR_SCALE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_SCALE))
#define PICMAN_COLOR_SCALE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_SCALE, PicmanColorScaleClass))


typedef struct _PicmanColorScaleClass  PicmanColorScaleClass;

struct _PicmanColorScale
{
  GtkScale                  parent_instance;

  /*< private >*/
  PicmanColorSelectorChannel  channel;
  PicmanRGB                   rgb;
  PicmanHSV                   hsv;

  guchar                   *buf;
  guint                     width;
  guint                     height;
  guint                     rowstride;

  gboolean                  needs_render;
};

struct _PicmanColorScaleClass
{
  GtkScaleClass             parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_scale_get_type    (void) G_GNUC_CONST;
GtkWidget * picman_color_scale_new         (GtkOrientation            orientation,
                                          PicmanColorSelectorChannel  channel);

void        picman_color_scale_set_channel (PicmanColorScale           *scale,
                                          PicmanColorSelectorChannel  channel);
void        picman_color_scale_set_color   (PicmanColorScale           *scale,
                                          const PicmanRGB            *rgb,
                                          const PicmanHSV            *hsv);


G_END_DECLS

#endif /* __PICMAN_COLOR_SCALE_H__ */
