/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpreview.h
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

#ifndef __PICMAN_PREVIEW_H__
#define __PICMAN_PREVIEW_H__

G_BEGIN_DECLS


/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_PREVIEW            (picman_preview_get_type ())
#define PICMAN_PREVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PREVIEW, PicmanPreview))
#define PICMAN_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PREVIEW, PicmanPreviewClass))
#define PICMAN_IS_PREVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PREVIEW))
#define PICMAN_IS_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PREVIEW))
#define PICMAN_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PREVIEW, PicmanPreviewClass))


typedef struct _PicmanPreviewClass  PicmanPreviewClass;

struct _PicmanPreview
{
  GtkBox        parent_instance;

  gboolean      update_preview;

  /*< protected >*/
  GtkWidget    *area;
  GtkWidget    *table;
  GtkWidget    *frame;
  GtkWidget    *toggle;
  GdkCursor    *cursor_busy;
  GdkCursor    *default_cursor;

  /*< private >*/
  gint          xoff, yoff;
  gint          xmin, xmax, ymin, ymax;
  gint          width, height;

  guint         timeout_id;
};

struct _PicmanPreviewClass
{
  GtkBoxClass  parent_class;

  /* virtual methods */
  void   (* draw)        (PicmanPreview     *preview);
  void   (* draw_thumb)  (PicmanPreview     *preview,
                          PicmanPreviewArea *area,
                          gint             width,
                          gint             height);
  void   (* draw_buffer) (PicmanPreview     *preview,
                          const guchar    *buffer,
                          gint             rowstride);
  void   (* set_cursor)  (PicmanPreview     *preview);

  /* signal */
  void   (* invalidated) (PicmanPreview     *preview);

  /* virtual methods */
  void   (* transform)   (PicmanPreview     *preview,
                          gint             src_x,
                          gint             src_y,
                          gint            *dest_x,
                          gint            *dest_y);
  void   (* untransform) (PicmanPreview     *preview,
                          gint             src_x,
                          gint             src_y,
                          gint            *dest_x,
                          gint            *dest_y);

  /* Padding for future expansion */
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_preview_get_type           (void) G_GNUC_CONST;

void        picman_preview_set_update         (PicmanPreview  *preview,
                                             gboolean      update);
gboolean    picman_preview_get_update         (PicmanPreview  *preview);

void        picman_preview_set_bounds         (PicmanPreview  *preview,
                                             gint          xmin,
                                             gint          ymin,
                                             gint          xmax,
                                             gint          ymax);

void        picman_preview_get_position       (PicmanPreview  *preview,
                                             gint         *x,
                                             gint         *y);
void        picman_preview_get_size           (PicmanPreview  *preview,
                                             gint         *width,
                                             gint         *height);

void        picman_preview_transform          (PicmanPreview *preview,
                                             gint         src_x,
                                             gint         src_y,
                                             gint        *dest_x,
                                             gint        *dest_y);
void        picman_preview_untransform        (PicmanPreview *preview,
                                             gint         src_x,
                                             gint         src_y,
                                             gint        *dest_x,
                                             gint        *dest_y);

GtkWidget * picman_preview_get_area           (PicmanPreview  *preview);

void        picman_preview_draw               (PicmanPreview  *preview);
void        picman_preview_draw_buffer        (PicmanPreview  *preview,
                                             const guchar *buffer,
                                             gint          rowstride);

void        picman_preview_invalidate         (PicmanPreview  *preview);

void        picman_preview_set_default_cursor (PicmanPreview  *preview,
                                             GdkCursor    *cursor);

GtkWidget * picman_preview_get_controls       (PicmanPreview  *preview);


G_END_DECLS

#endif /* __PICMAN_PREVIEW_H__ */
