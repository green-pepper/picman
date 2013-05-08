/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef __PICMAN_PREVIEW_AREA_H__
#define __PICMAN_PREVIEW_AREA_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_PREVIEW_AREA            (picman_preview_area_get_type ())
#define PICMAN_PREVIEW_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PREVIEW_AREA, PicmanPreviewArea))
#define PICMAN_PREVIEW_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PREVIEW_AREA, PicmanPreviewAreaClass))
#define PICMAN_IS_PREVIEW_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PREVIEW_AREA))
#define PICMAN_IS_PREVIEW_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PREVIEW_AREA))
#define PICMAN_PREVIEW_AREA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PREVIEW_AREA, PicmanPreviewArea))


typedef struct _PicmanPreviewAreaClass  PicmanPreviewAreaClass;

struct _PicmanPreviewArea
{
  GtkDrawingArea   parent_instance;

  PicmanCheckSize    check_size;
  PicmanCheckType    check_type;
  gint             width;
  gint             height;
  gint             rowstride;
  gint             offset_x;
  gint             offset_y;
  gint             max_width;
  gint             max_height;
  guchar          *buf;
  guchar          *colormap;
};

struct _PicmanPreviewAreaClass
{
  GtkDrawingAreaClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_preview_area_get_type       (void) G_GNUC_CONST;

GtkWidget * picman_preview_area_new            (void);

void        picman_preview_area_draw           (PicmanPreviewArea *area,
                                              gint             x,
                                              gint             y,
                                              gint             width,
                                              gint             height,
                                              PicmanImageType    type,
                                              const guchar    *buf,
                                              gint             rowstride);
void        picman_preview_area_blend          (PicmanPreviewArea *area,
                                              gint             x,
                                              gint             y,
                                              gint             width,
                                              gint             height,
                                              PicmanImageType    type,
                                              const guchar    *buf1,
                                              gint             rowstride1,
                                              const guchar    *buf2,
                                              gint             rowstride2,
                                              guchar           opacity);
void        picman_preview_area_mask           (PicmanPreviewArea *area,
                                              gint             x,
                                              gint             y,
                                              gint             width,
                                              gint             height,
                                              PicmanImageType    type,
                                              const guchar    *buf1,
                                              gint             rowstride1,
                                              const guchar    *buf2,
                                              gint             rowstride2,
                                              const guchar    *mask,
                                              gint             rowstride_mask);
void        picman_preview_area_fill           (PicmanPreviewArea *area,
                                              gint             x,
                                              gint             y,
                                              gint             width,
                                              gint             height,
                                              guchar           red,
                                              guchar           green,
                                              guchar           blue);

void        picman_preview_area_set_offsets    (PicmanPreviewArea *area,
                                              gint             x,
                                              gint             y);

void        picman_preview_area_set_colormap   (PicmanPreviewArea *area,
                                              const guchar    *colormap,
                                              gint             num_colors);

void        picman_preview_area_set_max_size   (PicmanPreviewArea *area,
                                              gint             width,
                                              gint             height);

void        picman_preview_area_menu_popup     (PicmanPreviewArea *area,
                                              GdkEventButton  *event);


G_END_DECLS

#endif /* __PICMAN_PREVIEW_AREA_H__ */
