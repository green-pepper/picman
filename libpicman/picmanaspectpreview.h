/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanaspectpreview.h
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

#if !defined (__PICMAN_UI_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picmanui.h> can be included directly."
#endif

#ifndef __PICMAN_ASPECT_PREVIEW_H__
#define __PICMAN_ASPECT_PREVIEW_H__

G_BEGIN_DECLS


/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_ASPECT_PREVIEW            (picman_aspect_preview_get_type ())
#define PICMAN_ASPECT_PREVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ASPECT_PREVIEW, PicmanAspectPreview))
#define PICMAN_ASPECT_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ASPECT_PREVIEW, PicmanAspectPreviewClass))
#define PICMAN_IS_ASPECT_PREVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ASPECT_PREVIEW))
#define PICMAN_IS_ASPECT_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ASPECT_PREVIEW))
#define PICMAN_ASPECT_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ASPECT_PREVIEW, PicmanAspectPreviewClass))


typedef struct _PicmanAspectPreviewClass  PicmanAspectPreviewClass;

struct _PicmanAspectPreview
{
  PicmanPreview   parent_instance;

  /*< private >*/
  PicmanDrawable *drawable;
};

struct _PicmanAspectPreviewClass
{
  PicmanPreviewClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_aspect_preview_get_type (void) G_GNUC_CONST;

GtkWidget * picman_aspect_preview_new      (PicmanDrawable *drawable,
                                          gboolean     *toggle);


G_END_DECLS

#endif /* __PICMAN_ASPECT_PREVIEW_H__ */
