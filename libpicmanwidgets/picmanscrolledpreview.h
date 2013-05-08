/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanscrolledpreview.h
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

#ifndef __PICMAN_SCROLLED_PREVIEW_H__
#define __PICMAN_SCROLLED_PREVIEW_H__

#include "picmanpreview.h"

G_BEGIN_DECLS


/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_SCROLLED_PREVIEW            (picman_scrolled_preview_get_type ())
#define PICMAN_SCROLLED_PREVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SCROLLED_PREVIEW, PicmanScrolledPreview))
#define PICMAN_SCROLLED_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SCROLLED_PREVIEW, PicmanScrolledPreviewClass))
#define PICMAN_IS_SCROLLED_PREVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SCROLLED_PREVIEW))
#define PICMAN_IS_SCROLLED_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SCROLLED_PREVIEW))
#define PICMAN_SCROLLED_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SCROLLED_PREVIEW, PicmanScrolledPreviewClass))


typedef struct _PicmanScrolledPreviewClass  PicmanScrolledPreviewClass;

struct _PicmanScrolledPreview
{
  PicmanPreview   parent_instance;

  /*< protected >*/
  GtkWidget    *hscr;
  GtkWidget    *vscr;
  GtkWidget    *nav_icon;
  GtkWidget    *nav_popup;
  GdkCursor    *cursor_move;
  gpointer      nav_gc; /* unused */

  /*< private >*/
  gpointer      priv;
};

struct _PicmanScrolledPreviewClass
{
  PicmanPreviewClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType  picman_scrolled_preview_get_type  (void) G_GNUC_CONST;

void   picman_scrolled_preview_set_position (PicmanScrolledPreview *preview,
                                           gint                 x,
                                           gint                 y);
void   picman_scrolled_preview_set_policy   (PicmanScrolledPreview *preview,
                                           GtkPolicyType        hscrollbar_policy,
                                           GtkPolicyType        vscrollbar_policy);

/*  only for use from derived widgets  */
void   picman_scrolled_preview_freeze       (PicmanScrolledPreview *preview);
void   picman_scrolled_preview_thaw         (PicmanScrolledPreview *preview);


G_END_DECLS

#endif /* __PICMAN_SCROLLED_PREVIEW_H__ */
