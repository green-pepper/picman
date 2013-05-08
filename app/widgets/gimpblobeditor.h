/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanblobeditor.h
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

#ifndef  __PICMAN_BLOB_EDITOR_H__
#define  __PICMAN_BLOB_EDITOR_H__


#define PICMAN_TYPE_BLOB_EDITOR            (picman_blob_editor_get_type ())
#define PICMAN_BLOB_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BLOB_EDITOR, PicmanBlobEditor))
#define PICMAN_BLOB_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BLOB_EDITOR, PicmanBlobEditorClass))
#define PICMAN_IS_BLOB_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BLOB_EDITOR))
#define PICMAN_IS_BLOB_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BLOB_EDITOR))
#define PICMAN_BLOB_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BLOB_EDITOR, PicmanBlobEditorClass))


typedef struct _PicmanBlobEditorClass PicmanBlobEditorClass;

struct _PicmanBlobEditor
{
  GtkDrawingArea       parent_instance;

  PicmanInkBlobType      type;
  gdouble              aspect;
  gdouble              angle;

  /*<  private  >*/
  gboolean             active;
};

struct _PicmanBlobEditorClass
{
  GtkDrawingAreaClass  parent_class;
};


GType       picman_blob_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_blob_editor_new      (PicmanInkBlobType  type,
                                       gdouble          aspect,
                                       gdouble          angle);


#endif  /*  __PICMAN_BLOB_EDITOR_H__  */
