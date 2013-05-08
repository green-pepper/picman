/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_ISCISSORS_OPTIONS_H__
#define __PICMAN_ISCISSORS_OPTIONS_H__


#include "picmanselectionoptions.h"


#define PICMAN_TYPE_ISCISSORS_OPTIONS            (picman_iscissors_options_get_type ())
#define PICMAN_ISCISSORS_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ISCISSORS_OPTIONS, PicmanIscissorsOptions))
#define PICMAN_ISCISSORS_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ISCISSORS_OPTIONS, PicmanIscissorsOptionsClass))
#define PICMAN_IS_ISCISSORS_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ISCISSORS_OPTIONS))
#define PICMAN_IS_ISCISSORS_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ISCISSORS_OPTIONS))
#define PICMAN_ISCISSORS_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ISCISSORS_OPTIONS, PicmanIscissorsOptionsClass))


typedef struct _PicmanIscissorsOptions PicmanIscissorsOptions;
typedef struct _PicmanToolOptionsClass PicmanIscissorsOptionsClass;

struct _PicmanIscissorsOptions
{
  PicmanSelectionOptions  parent_instance;

  gboolean              interactive;
};


GType       picman_iscissors_options_get_type (void) G_GNUC_CONST;

GtkWidget * picman_iscissors_options_gui      (PicmanToolOptions *tool_options);


#endif  /*  __PICMAN_ISCISSORS_OPTIONS_H__  */
