/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis and others
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

#ifndef __PICMAN_TOOL_CONTROL_H__
#define __PICMAN_TOOL_CONTROL_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_TOOL_CONTROL            (picman_tool_control_get_type ())
#define PICMAN_TOOL_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_CONTROL, PicmanToolControl))
#define PICMAN_TOOL_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_CONTROL, PicmanToolControlClass))
#define PICMAN_IS_TOOL_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_CONTROL))
#define PICMAN_IS_TOOL_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_CONTROL))
#define PICMAN_TOOL_CONTROL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_CONTROL, PicmanToolControlClass))


typedef struct _PicmanToolControlClass PicmanToolControlClass;


struct _PicmanToolControl
{
  PicmanObject           parent_instance;

  gboolean             active;             /*  state of tool activity          */
  gint                 paused_count;       /*  paused control count            */

  gboolean             preserve;           /*  Preserve this tool across       *
                                            *  drawable changes                */
  GSList              *preserve_stack;     /*  for push/pop preserve           */

  gboolean             scroll_lock;        /*  allow scrolling or not          */
  gboolean             handle_empty_image; /*  invoke the tool on images       *
                                            *  without active drawable         */
  PicmanDirtyMask        dirty_mask;         /*  if preserve is FALSE, cancel    *
                                            *  the tool on these events        */
  PicmanMotionMode       motion_mode;        /*  how to process motion events    *
                                            *  before they go to the tool      */
  gboolean             auto_snap_to;       /*  snap to guides automatically    */
  gint                 snap_offset_x;
  gint                 snap_offset_y;
  gint                 snap_width;
  gint                 snap_height;

  PicmanCursorPrecision  precision;

  gboolean             wants_click;        /*  wants click detection           */
  gboolean             wants_double_click;
  gboolean             wants_triple_click;
  gboolean             wants_all_key_events;

  gboolean             toggled;

  PicmanCursorType       cursor;
  PicmanToolCursorType   tool_cursor;
  PicmanCursorModifier   cursor_modifier;

  PicmanCursorType       toggle_cursor;
  PicmanToolCursorType   toggle_tool_cursor;
  PicmanCursorModifier   toggle_cursor_modifier;

  gchar               *action_value_1;
  gchar               *action_value_2;
  gchar               *action_value_3;
  gchar               *action_value_4;
  gchar               *action_object_1;
  gchar               *action_object_2;
};

struct _PicmanToolControlClass
{
  PicmanObjectClass parent_class;
};


GType          picman_tool_control_get_type         (void) G_GNUC_CONST;

void           picman_tool_control_activate         (PicmanToolControl *control);
void           picman_tool_control_halt             (PicmanToolControl *control);
gboolean       picman_tool_control_is_active        (PicmanToolControl *control);

void           picman_tool_control_pause            (PicmanToolControl *control);
void           picman_tool_control_resume           (PicmanToolControl *control);
gboolean       picman_tool_control_is_paused        (PicmanToolControl *control);

void           picman_tool_control_set_preserve     (PicmanToolControl *control,
                                                   gboolean         preserve);
gboolean       picman_tool_control_get_preserve     (PicmanToolControl *control);

void           picman_tool_control_push_preserve    (PicmanToolControl *control,
                                                   gboolean         preserve);
void           picman_tool_control_pop_preserve     (PicmanToolControl *control);

void           picman_tool_control_set_scroll_lock  (PicmanToolControl *control,
                                                   gboolean         scroll_lock);
gboolean       picman_tool_control_get_scroll_lock  (PicmanToolControl *control);

void     picman_tool_control_set_handle_empty_image (PicmanToolControl *control,
                                                   gboolean         handle_empty);
gboolean picman_tool_control_get_handle_empty_image (PicmanToolControl *control);

void           picman_tool_control_set_dirty_mask   (PicmanToolControl *control,
                                                   PicmanDirtyMask    dirty_mask);
PicmanDirtyMask  picman_tool_control_get_dirty_mask   (PicmanToolControl *control);

void           picman_tool_control_set_motion_mode  (PicmanToolControl *control,
                                                   PicmanMotionMode   motion_mode);
PicmanMotionMode picman_tool_control_get_motion_mode  (PicmanToolControl *control);

void           picman_tool_control_set_snap_to      (PicmanToolControl *control,
                                                   gboolean         snap_to);
gboolean       picman_tool_control_get_snap_to      (PicmanToolControl *control);

void           picman_tool_control_set_wants_click  (PicmanToolControl *control,
                                                   gboolean         wants_click);
gboolean       picman_tool_control_get_wants_click  (PicmanToolControl *control);

void           picman_tool_control_set_wants_double_click   (PicmanToolControl *control,
                                                           gboolean         wants_double_click);
gboolean       picman_tool_control_get_wants_double_click   (PicmanToolControl *control);

void           picman_tool_control_set_wants_triple_click   (PicmanToolControl *control,
                                                           gboolean         wants_double_click);
gboolean       picman_tool_control_get_wants_triple_click   (PicmanToolControl *control);

void           picman_tool_control_set_wants_all_key_events (PicmanToolControl *control,
                                                           gboolean         wants_key_events);
gboolean       picman_tool_control_get_wants_all_key_events (PicmanToolControl *control);

void           picman_tool_control_set_snap_offsets (PicmanToolControl *control,
                                                   gint             offset_x,
                                                   gint             offset_y,
                                                   gint             width,
                                                   gint             height);
void           picman_tool_control_get_snap_offsets (PicmanToolControl *control,
                                                   gint            *offset_x,
                                                   gint            *offset_y,
                                                   gint            *width,
                                                   gint            *height);

void           picman_tool_control_set_precision    (PicmanToolControl     *control,
                                                   PicmanCursorPrecision  precision);
PicmanCursorPrecision
               picman_tool_control_get_precision    (PicmanToolControl     *control);

void           picman_tool_control_set_toggled      (PicmanToolControl *control,
                                                   gboolean         toggled);
gboolean       picman_tool_control_get_toggled      (PicmanToolControl *control);

void picman_tool_control_set_cursor                 (PicmanToolControl    *control,
                                                   PicmanCursorType      cursor);
void picman_tool_control_set_tool_cursor            (PicmanToolControl    *control,
                                                   PicmanToolCursorType  cursor);
void picman_tool_control_set_cursor_modifier        (PicmanToolControl    *control,
                                                   PicmanCursorModifier  modifier);
void picman_tool_control_set_toggle_cursor          (PicmanToolControl    *control,
                                                   PicmanCursorType      cursor);
void picman_tool_control_set_toggle_tool_cursor     (PicmanToolControl    *control,
                                                   PicmanToolCursorType  cursor);
void picman_tool_control_set_toggle_cursor_modifier (PicmanToolControl    *control,
                                                   PicmanCursorModifier  modifier);

PicmanCursorType
              picman_tool_control_get_cursor          (PicmanToolControl *control);

PicmanToolCursorType
              picman_tool_control_get_tool_cursor     (PicmanToolControl *control);

PicmanCursorModifier
              picman_tool_control_get_cursor_modifier (PicmanToolControl *control);

void          picman_tool_control_set_action_value_1  (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_value_1  (PicmanToolControl *control);

void          picman_tool_control_set_action_value_2  (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_value_2  (PicmanToolControl *control);

void          picman_tool_control_set_action_value_3  (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_value_3  (PicmanToolControl *control);

void          picman_tool_control_set_action_value_4  (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_value_4  (PicmanToolControl *control);

void          picman_tool_control_set_action_object_1 (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_object_1 (PicmanToolControl *control);

void          picman_tool_control_set_action_object_2 (PicmanToolControl *control,
                                                     const gchar     *action);
const gchar * picman_tool_control_get_action_object_2 (PicmanToolControl *control);


#endif /* __PICMAN_TOOL_CONTROL_H__ */
