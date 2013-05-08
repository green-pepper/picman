/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandataeditor.h
 * Copyright (C) 2002-2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DATA_EDITOR_H__
#define __PICMAN_DATA_EDITOR_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_DATA_EDITOR            (picman_data_editor_get_type ())
#define PICMAN_DATA_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DATA_EDITOR, PicmanDataEditor))
#define PICMAN_DATA_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DATA_EDITOR, PicmanDataEditorClass))
#define PICMAN_IS_DATA_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DATA_EDITOR))
#define PICMAN_IS_DATA_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DATA_EDITOR))
#define PICMAN_DATA_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DATA_EDITOR, PicmanDataEditorClass))


typedef struct _PicmanDataEditorClass PicmanDataEditorClass;

struct _PicmanDataEditor
{
  PicmanEditor       parent_instance;

  PicmanDataFactory *data_factory;
  PicmanContext     *context;
  gboolean         edit_active;

  PicmanData        *data;
  gboolean         data_editable;

  GtkWidget       *name_entry;

  GtkWidget       *view; /* filled by subclasses */

  GtkWidget       *save_button;
  GtkWidget       *revert_button;
};

struct _PicmanDataEditorClass
{
  PicmanEditorClass  parent_class;

  /*  virtual functions  */
  void (* set_data) (PicmanDataEditor *editor,
                     PicmanData       *data);

  const gchar *title;
};


GType       picman_data_editor_get_type        (void) G_GNUC_CONST;

void        picman_data_editor_set_data        (PicmanDataEditor *editor,
                                              PicmanData       *data);
PicmanData  * picman_data_editor_get_data        (PicmanDataEditor *editor);

void        picman_data_editor_set_edit_active (PicmanDataEditor *editor,
                                              gboolean        edit_active);
gboolean    picman_data_editor_get_edit_active (PicmanDataEditor *editor);


#endif  /*  __PICMAN_DATA_EDITOR_H__  */
