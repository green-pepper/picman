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

#ifndef __PICMAN_VECTOR_OPTIONS_H__
#define __PICMAN_VECTOR_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_VECTOR_OPTIONS            (picman_vector_options_get_type ())
#define PICMAN_VECTOR_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VECTOR_OPTIONS, PicmanVectorOptions))
#define PICMAN_VECTOR_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VECTOR_OPTIONS, PicmanVectorOptionsClass))
#define PICMAN_IS_VECTOR_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VECTOR_OPTIONS))
#define PICMAN_IS_VECTOR_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VECTOR_OPTIONS))
#define PICMAN_VECTOR_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VECTOR_OPTIONS, PicmanVectorOptionsClass))


typedef struct _PicmanVectorOptions    PicmanVectorOptions;
typedef struct _PicmanToolOptionsClass PicmanVectorOptionsClass;

struct _PicmanVectorOptions
{
  PicmanToolOptions  parent_instance;

  PicmanVectorMode   edit_mode;
  gboolean         polygonal;

  /*  options gui  */
  GtkWidget       *to_selection_button;
  GtkWidget       *stroke_button;
};


GType       picman_vector_options_get_type (void) G_GNUC_CONST;

GtkWidget * picman_vector_options_gui      (PicmanToolOptions *tool_options);


#endif  /*  __PICMAN_VECTOR_OPTIONS_H__  */
