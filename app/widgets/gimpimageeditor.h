/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_IMAGE_EDITOR_H__
#define __PICMAN_IMAGE_EDITOR_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_IMAGE_EDITOR            (picman_image_editor_get_type ())
#define PICMAN_IMAGE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_EDITOR, PicmanImageEditor))
#define PICMAN_IMAGE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_EDITOR, PicmanImageEditorClass))
#define PICMAN_IS_IMAGE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_EDITOR))
#define PICMAN_IS_IMAGE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_EDITOR))
#define PICMAN_IMAGE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_EDITOR, PicmanImageEditorClass))


typedef struct _PicmanImageEditorClass PicmanImageEditorClass;

struct _PicmanImageEditor
{
  PicmanEditor   parent_instance;

  PicmanContext *context;
  PicmanImage   *image;
};

struct _PicmanImageEditorClass
{
  PicmanEditorClass  parent_class;

  /*  virtual function  */
  void (* set_image) (PicmanImageEditor *editor,
                      PicmanImage       *image);
};


GType       picman_image_editor_get_type  (void) G_GNUC_CONST;

void        picman_image_editor_set_image (PicmanImageEditor *editor,
                                         PicmanImage       *image);
PicmanImage * picman_image_editor_get_image (PicmanImageEditor *editor);


#endif /* __PICMAN_IMAGE_EDITOR_H__ */
