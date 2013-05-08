/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixmap.h
 * Copyright (C) 2000 Michael Natterer <mitch@picman.org>
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

#ifndef PICMAN_DISABLE_DEPRECATED

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_PIXMAP_H__
#define __PICMAN_PIXMAP_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_PIXMAP            (picman_pixmap_get_type ())
#define PICMAN_PIXMAP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PIXMAP, PicmanPixmap))
#define PICMAN_PIXMAP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PIXMAP, PicmanPixmapClass))
#define PICMAN_IS_PIXMAP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PIXMAP))
#define PICMAN_IS_PIXMAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PIXMAP))
#define PICMAN_PIXMAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PIXMAP, PicmanPixmapClass))


typedef struct _PicmanPixmapClass  PicmanPixmapClass;

struct _PicmanPixmap
{
  GtkImage   parent_instance;

  gchar    **xpm_data;
};

struct _PicmanPixmapClass
{
  GtkImageClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_pixmap_get_type (void) G_GNUC_CONST;

GtkWidget * picman_pixmap_new      (gchar      **xpm_data);

void        picman_pixmap_set      (PicmanPixmap  *pixmap,
                                  gchar      **xpm_data);


G_END_DECLS

#endif /* __PICMAN_PIXMAP_H__ */

#endif /* PICMAN_DISABLE_DEPRECATED */

