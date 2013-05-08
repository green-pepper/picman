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

#ifndef __PICMAN_MAGNIFY_OPTIONS_H__
#define __PICMAN_MAGNIFY_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_MAGNIFY_OPTIONS            (picman_magnify_options_get_type ())
#define PICMAN_MAGNIFY_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MAGNIFY_OPTIONS, PicmanMagnifyOptions))
#define PICMAN_MAGNIFY_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MAGNIFY_OPTIONS, PicmanMagnifyOptionsClass))
#define PICMAN_IS_MAGNIFY_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MAGNIFY_OPTIONS))
#define PICMAN_IS_MAGNIFY_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MAGNIFY_OPTIONS))
#define PICMAN_MAGNIFY_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MAGNIFY_OPTIONS, PicmanMagnifyOptionsClass))


typedef struct _PicmanMagnifyOptions   PicmanMagnifyOptions;
typedef struct _PicmanToolOptionsClass PicmanMagnifyOptionsClass;

struct _PicmanMagnifyOptions
{
  PicmanToolOptions   parent_instance;

  gboolean          auto_resize;
  PicmanZoomType      zoom_type;
};


GType       picman_magnify_options_get_type (void) G_GNUC_CONST;

GtkWidget * picman_magnify_options_gui      (PicmanToolOptions *tool_options);


#endif  /*  __PICMAN_MAGNIFY_OPTIONS_H__  */
