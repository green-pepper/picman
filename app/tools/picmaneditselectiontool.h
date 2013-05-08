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

#ifndef __PICMAN_EDIT_SELECTION_TOOL_H__
#define __PICMAN_EDIT_SELECTION_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_EDIT_SELECTION_TOOL            (picman_edit_selection_tool_get_type ())
#define PICMAN_EDIT_SELECTION_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_EDIT_SELECTION_TOOL, PicmanEditSelectionTool))
#define PICMAN_EDIT_SELECTION_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_EDIT_SELECTION_TOOL, PicmanEditSelectionToolClass))
#define PICMAN_IS_EDIT_SELECTION_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_EDIT_SELECTION_TOOL))
#define PICMAN_IS_EDIT_SELECTION_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_EDIT_SELECTION_TOOL))


GType      picman_edit_selection_tool_get_type  (void) G_GNUC_CONST;

void       picman_edit_selection_tool_start     (PicmanTool          *parent_tool,
                                               PicmanDisplay       *display,
                                               const PicmanCoords  *coords,
                                               PicmanTranslateMode  edit_mode,
                                               gboolean           propagate_release);

gboolean   picman_edit_selection_tool_key_press (PicmanTool          *tool,
                                               GdkEventKey       *kevent,
                                               PicmanDisplay       *display);
gboolean   picman_edit_selection_tool_translate (PicmanTool          *tool,
                                               GdkEventKey       *kevent,
                                               PicmanTransformType  translate_type,
                                               PicmanDisplay       *display);


#endif  /*  __PICMAN_EDIT_SELECTION_TOOL_H__  */
