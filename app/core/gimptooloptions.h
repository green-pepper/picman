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

#ifndef __PICMAN_TOOL_OPTIONS_H__
#define __PICMAN_TOOL_OPTIONS_H__


#include "picmancontext.h"


#define PICMAN_TYPE_TOOL_OPTIONS            (picman_tool_options_get_type ())
#define PICMAN_TOOL_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_OPTIONS, PicmanToolOptions))
#define PICMAN_TOOL_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_OPTIONS, PicmanToolOptionsClass))
#define PICMAN_IS_TOOL_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_OPTIONS))
#define PICMAN_IS_TOOL_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_OPTIONS))
#define PICMAN_TOOL_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_OPTIONS, PicmanToolOptionsClass))


typedef struct _PicmanToolOptionsClass PicmanToolOptionsClass;

struct _PicmanToolOptions
{
  PicmanContext   parent_instance;

  PicmanToolInfo *tool_info;
};

struct _PicmanToolOptionsClass
{
  PicmanContextClass parent_class;

  void (* reset) (PicmanToolOptions *tool_options);
};


GType      picman_tool_options_get_type      (void) G_GNUC_CONST;

void       picman_tool_options_reset         (PicmanToolOptions  *tool_options);

gboolean   picman_tool_options_serialize     (PicmanToolOptions   *tool_options,
                                            GError           **error);
gboolean   picman_tool_options_deserialize   (PicmanToolOptions   *tool_options,
                                            GError           **error);

gboolean   picman_tool_options_delete        (PicmanToolOptions   *tool_options,
                                            GError           **error);
void       picman_tool_options_create_folder (void);


#endif  /*  __PICMAN_TOOL_OPTIONS_H__  */
