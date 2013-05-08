/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextTool
 * Copyright (C) 2002-2010  Sven Neumann <sven@picman.org>
 *                          Daniel Eddeland <danedde@svn.gnome.org>
 *                          Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEXT_TOOL_EDITOR_H__
#define __PICMAN_TEXT_TOOL_EDITOR_H__


void       picman_text_tool_editor_init            (PicmanTextTool        *text_tool);
void       picman_text_tool_editor_finalize        (PicmanTextTool        *text_tool);

void       picman_text_tool_editor_start           (PicmanTextTool        *text_tool);
void       picman_text_tool_editor_position        (PicmanTextTool        *text_tool);
void       picman_text_tool_editor_halt            (PicmanTextTool        *text_tool);

void       picman_text_tool_editor_button_press    (PicmanTextTool        *text_tool,
                                                  gdouble              x,
                                                  gdouble              y,
                                                  PicmanButtonPressType  press_type);
void       picman_text_tool_editor_button_release  (PicmanTextTool        *text_tool);
void       picman_text_tool_editor_motion          (PicmanTextTool        *text_tool,
                                                  gdouble              x,
                                                  gdouble              y);
gboolean   picman_text_tool_editor_key_press       (PicmanTextTool        *text_tool,
                                                  GdkEventKey         *kevent);
gboolean   picman_text_tool_editor_key_release     (PicmanTextTool        *text_tool,
                                                  GdkEventKey         *kevent);

void       picman_text_tool_reset_im_context       (PicmanTextTool        *text_tool);

void       picman_text_tool_editor_get_cursor_rect (PicmanTextTool        *text_tool,
                                                  gboolean             overwrite,
                                                  PangoRectangle      *cursor_rect);


#endif /* __PICMAN_TEXT_TOOL_EDITOR_H__ */
