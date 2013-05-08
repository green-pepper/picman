/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorarea.h
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
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

/* This provides a color preview area. The preview
 * can handle transparency by showing the checkerboard and
 * handles drag'n'drop.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_AREA_H__
#define __PICMAN_COLOR_AREA_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_COLOR_AREA            (picman_color_area_get_type ())
#define PICMAN_COLOR_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_AREA, PicmanColorArea))
#define PICMAN_COLOR_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_AREA, PicmanColorAreaClass))
#define PICMAN_IS_COLOR_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_AREA))
#define PICMAN_IS_COLOR_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_AREA))
#define PICMAN_COLOR_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_AREA, PicmanColorAreaClass))


typedef struct _PicmanColorAreaClass  PicmanColorAreaClass;

struct _PicmanColorArea
{
  GtkDrawingArea       parent_instance;

  /*< private >*/
  guchar              *buf;
  guint                width;
  guint                height;
  guint                rowstride;

  PicmanColorAreaType    type;
  PicmanRGB              color;
  guint                draw_border  : 1;
  guint                needs_render : 1;
};

struct _PicmanColorAreaClass
{
  GtkDrawingAreaClass  parent_class;

  void (* color_changed) (PicmanColorArea *area);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_area_get_type        (void) G_GNUC_CONST;

GtkWidget * picman_color_area_new             (const PicmanRGB     *color,
                                             PicmanColorAreaType  type,
                                             GdkModifierType    drag_mask);

void        picman_color_area_set_color       (PicmanColorArea     *area,
                                             const PicmanRGB     *color);
void        picman_color_area_get_color       (PicmanColorArea     *area,
                                             PicmanRGB           *color);
gboolean    picman_color_area_has_alpha       (PicmanColorArea     *area);
void        picman_color_area_set_type        (PicmanColorArea     *area,
                                             PicmanColorAreaType  type);
void        picman_color_area_set_draw_border (PicmanColorArea     *area,
                                             gboolean           draw_border);


G_END_DECLS

#endif /* __PICMAN_COLOR_AREA_H__ */
