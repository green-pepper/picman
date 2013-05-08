/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandocumentview.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DOCUMENT_VIEW_H__
#define __PICMAN_DOCUMENT_VIEW_H__


#include "picmancontainereditor.h"


#define PICMAN_TYPE_DOCUMENT_VIEW            (picman_document_view_get_type ())
#define PICMAN_DOCUMENT_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCUMENT_VIEW, PicmanDocumentView))
#define PICMAN_DOCUMENT_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCUMENT_VIEW, PicmanDocumentViewClass))
#define PICMAN_IS_DOCUMENT_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCUMENT_VIEW))
#define PICMAN_IS_DOCUMENT_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCUMENT_VIEW))
#define PICMAN_DOCUMENT_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCUMENT_VIEW, PicmanDocumentViewClass))


typedef struct _PicmanDocumentViewClass  PicmanDocumentViewClass;

struct _PicmanDocumentView
{
  PicmanContainerEditor  parent_instance;

  GtkWidget           *open_button;
  GtkWidget           *remove_button;
  GtkWidget           *refresh_button;
};

struct _PicmanDocumentViewClass
{
  PicmanContainerEditorClass  parent_class;
};


GType       picman_document_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_document_view_new      (PicmanViewType     view_type,
                                         PicmanContainer   *container,
                                         PicmanContext     *context,
                                         gint             view_size,
                                         gint             view_border_width,
                                         PicmanMenuFactory *menu_factory);


#endif  /*  __PICMAN_DOCUMENT_VIEW_H__  */
