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

#ifndef __PICMAN_HISTOGRAM_EDITOR_H__
#define __PICMAN_HISTOGRAM_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_HISTOGRAM_EDITOR            (picman_histogram_editor_get_type ())
#define PICMAN_HISTOGRAM_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HISTOGRAM_EDITOR, PicmanHistogramEditor))
#define PICMAN_HISTOGRAM_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HISTOGRAM_EDITOR, PicmanHistogramEditorClass))
#define PICMAN_IS_HISTOGRAM_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HISTOGRAM_EDITOR))
#define PICMAN_IS_HISTOGRAM_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HISTOGRAM_EDITOR))
#define PICMAN_HISTOGRAM_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HISTOGRAM_EDITOR, PicmanHistogramEditorClass))


typedef struct _PicmanHistogramEditorClass PicmanHistogramEditorClass;

struct _PicmanHistogramEditor
{
  PicmanImageEditor       parent_instance;

  PicmanDrawable         *drawable;
  PicmanHistogram        *histogram;
  PicmanHistogram        *bg_histogram;

  guint                 idle_id;
  gboolean              valid;

  GtkWidget            *menu;
  GtkWidget            *box;
  GtkWidget            *labels[6];
};

struct _PicmanHistogramEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_histogram_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_histogram_editor_new      (void);


#endif /* __PICMAN_HISTOGRAM_EDITOR_H__ */
