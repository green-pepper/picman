/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancursorview.h
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CURSOR_VIEW_H__
#define __PICMAN_CURSOR_VIEW_H__


#include "widgets/picmaneditor.h"


#define PICMAN_TYPE_CURSOR_VIEW            (picman_cursor_view_get_type ())
#define PICMAN_CURSOR_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CURSOR_VIEW, PicmanCursorView))
#define PICMAN_CURSOR_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CURSOR_VIEW, PicmanCursorViewClass))
#define PICMAN_IS_CURSOR_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CURSOR_VIEW))
#define PICMAN_IS_CURSOR_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CURSOR_VIEW))
#define PICMAN_CURSOR_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CURSOR_VIEW, PicmanCursorViewClass))


typedef struct _PicmanCursorViewClass PicmanCursorViewClass;
typedef struct _PicmanCursorViewPriv  PicmanCursorViewPriv;

struct _PicmanCursorView
{
  PicmanEditor          parent_instance;

  PicmanCursorViewPriv *priv;
};

struct _PicmanCursorViewClass
{
  PicmanEditorClass  parent_class;
};


GType       picman_cursor_view_get_type          (void) G_GNUC_CONST;

GtkWidget * picman_cursor_view_new               (PicmanMenuFactory *menu_factory);

void        picman_cursor_view_set_sample_merged (PicmanCursorView  *view,
                                                gboolean         sample_merged);
gboolean    picman_cursor_view_get_sample_merged (PicmanCursorView  *view);

void        picman_cursor_view_update_cursor     (PicmanCursorView  *view,
                                                PicmanImage       *image,
                                                PicmanUnit         shell_unit,
                                                gdouble          x,
                                                gdouble          y);
void        picman_cursor_view_clear_cursor      (PicmanCursorView  *view);


#endif /* __PICMAN_CURSOR_VIEW_H__ */
