/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolornotebook.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * based on color_notebook module
 * Copyright (C) 1998 Austin Donnelly <austin@greenend.org.uk>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_NOTEBOOK_H__
#define __PICMAN_COLOR_NOTEBOOK_H__

#include <libpicmanwidgets/picmancolorselector.h>

G_BEGIN_DECLS


#define PICMAN_TYPE_COLOR_NOTEBOOK            (picman_color_notebook_get_type ())
#define PICMAN_COLOR_NOTEBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_NOTEBOOK, PicmanColorNotebook))
#define PICMAN_COLOR_NOTEBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_NOTEBOOK, PicmanColorNotebookClass))
#define PICMAN_IS_COLOR_NOTEBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_NOTEBOOK))
#define PICMAN_IS_COLOR_NOTEBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_NOTEBOOK))
#define PICMAN_COLOR_NOTEBOOK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_NOTEBOOK, PicmanColorNotebookClass))


typedef struct _PicmanColorNotebookClass PicmanColorNotebookClass;

struct _PicmanColorNotebook
{
  PicmanColorSelector  parent_instance;

  GtkWidget         *notebook;

  GList             *selectors;
  PicmanColorSelector *cur_page;
};

struct _PicmanColorNotebookClass
{
  PicmanColorSelectorClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_notebook_get_type     (void) G_GNUC_CONST;

GtkWidget * picman_color_notebook_set_has_page (PicmanColorNotebook *notebook,
                                              GType              page_type,
                                              gboolean           has_page);


G_END_DECLS

#endif /* __PICMAN_COLOR_NOTEBOOK_H__ */
