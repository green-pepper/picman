/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansamplepointeditor.h
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

#ifndef __PICMAN_SAMPLE_POINT_EDITOR_H__
#define __PICMAN_SAMPLE_POINT_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_SAMPLE_POINT_EDITOR            (picman_sample_point_editor_get_type ())
#define PICMAN_SAMPLE_POINT_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SAMPLE_POINT_EDITOR, PicmanSamplePointEditor))
#define PICMAN_SAMPLE_POINT_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SAMPLE_POINT_EDITOR, PicmanSamplePointEditorClass))
#define PICMAN_IS_SAMPLE_POINT_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SAMPLE_POINT_EDITOR))
#define PICMAN_IS_SAMPLE_POINT_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SAMPLE_POINT_EDITOR))
#define PICMAN_SAMPLE_POINT_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SAMPLE_POINT_EDITOR, PicmanSamplePointEditorClass))


typedef struct _PicmanSamplePointEditorClass PicmanSamplePointEditorClass;

struct _PicmanSamplePointEditor
{
  PicmanImageEditor  parent_instance;

  GtkWidget       *table;
  GtkWidget       *color_frames[4];

  gboolean         dirty[4];
  guint            dirty_idle_id;

  gboolean         sample_merged;
};

struct _PicmanSamplePointEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_sample_point_editor_get_type          (void) G_GNUC_CONST;

GtkWidget * picman_sample_point_editor_new               (PicmanMenuFactory *menu_factory);

void        picman_sample_point_editor_set_sample_merged (PicmanSamplePointEditor *editor,
                                                        gboolean               sample_merged);
gboolean    picman_sample_point_editor_get_sample_merged (PicmanSamplePointEditor *editor);


#endif /* __PICMAN_SAMPLE_POINT_EDITOR_H__ */
