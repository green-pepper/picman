/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2004  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
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

#if !defined (__PICMAN_THUMB_H_INSIDE__) && !defined (PICMAN_THUMB_COMPILATION)
#error "Only <libpicmanthumb/picmanthumb.h> can be included directly."
#endif

#ifndef __PICMAN_THUMBNAIL_H__
#define __PICMAN_THUMBNAIL_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_THUMBNAIL            (picman_thumbnail_get_type ())
#define PICMAN_THUMBNAIL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_THUMBNAIL, PicmanThumbnail))
#define PICMAN_THUMBNAIL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_THUMBNAIL, PicmanThumbnailClass))
#define PICMAN_IS_THUMBNAIL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_THUMBNAIL))
#define PICMAN_IS_THUMBNAIL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_THUMBNAIL))
#define PICMAN_THUMBNAIL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_THUMBNAIL, PicmanThumbnailClass))


typedef struct _PicmanThumbnailClass PicmanThumbnailClass;

/**
 * PicmanThumbnail:
 *
 * All members of #PicmanThumbnail are private and should only be accessed
 * using object properties.
 **/
struct _PicmanThumbnail
{
  GObject         parent_instance;

  /*< private >*/
  PicmanThumbState  image_state;
  gchar          *image_uri;
  gchar          *image_filename;
  gint64          image_filesize;
  gint64          image_mtime;
  gint            image_not_found_errno;
  gint            image_width;
  gint            image_height;
  gchar          *image_type;
  gint            image_num_layers;

  PicmanThumbState  thumb_state;
  PicmanThumbSize   thumb_size;
  gchar          *thumb_filename;
  gint64          thumb_filesize;
  gint64          thumb_mtime;

  gchar          *image_mimetype;

  gpointer        _reserved_2;
};

struct _PicmanThumbnailClass
{
  GObjectClass    parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType            picman_thumbnail_get_type         (void) G_GNUC_CONST;

PicmanThumbnail  * picman_thumbnail_new              (void);

void             picman_thumbnail_set_uri          (PicmanThumbnail  *thumbnail,
                                                  const gchar    *uri);
gboolean         picman_thumbnail_set_filename     (PicmanThumbnail  *thumbnail,
                                                  const gchar    *filename,
                                                  GError        **error);
gboolean         picman_thumbnail_set_from_thumb   (PicmanThumbnail  *thumbnail,
                                                  const gchar    *filename,
                                                  GError        **error);

PicmanThumbState   picman_thumbnail_peek_image       (PicmanThumbnail  *thumbnail);
PicmanThumbState   picman_thumbnail_peek_thumb       (PicmanThumbnail  *thumbnail,
                                                  PicmanThumbSize   size);

PicmanThumbState   picman_thumbnail_check_thumb      (PicmanThumbnail  *thumbnail,
                                                  PicmanThumbSize   size);

GdkPixbuf      * picman_thumbnail_load_thumb       (PicmanThumbnail  *thumbnail,
                                                  PicmanThumbSize   size,
                                                  GError        **error);

gboolean         picman_thumbnail_save_thumb       (PicmanThumbnail  *thumbnail,
                                                  GdkPixbuf      *pixbuf,
                                                  const gchar    *software,
                                                  GError        **error);
gboolean         picman_thumbnail_save_thumb_local (PicmanThumbnail  *thumbnail,
                                                  GdkPixbuf      *pixbuf,
                                                  const gchar    *software,
                                                  GError        **error);

gboolean         picman_thumbnail_save_failure     (PicmanThumbnail  *thumbnail,
                                                  const gchar    *software,
                                                  GError        **error);
void             picman_thumbnail_delete_failure   (PicmanThumbnail  *thumbnail);
void             picman_thumbnail_delete_others    (PicmanThumbnail  *thumbnail,
                                                  PicmanThumbSize   size);

gboolean         picman_thumbnail_has_failed       (PicmanThumbnail  *thumbnail);


G_END_DECLS

#endif /* __PICMAN_THUMBNAIL_H__ */
