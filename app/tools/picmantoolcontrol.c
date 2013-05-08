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

#include "config.h"

#include <gtk/gtk.h>

#include "tools-types.h"

#include "picmantoolcontrol.h"


static void picman_tool_control_finalize (GObject *object);


G_DEFINE_TYPE (PicmanToolControl, picman_tool_control, PICMAN_TYPE_OBJECT)

#define parent_class picman_tool_control_parent_class


static void
picman_tool_control_class_init (PicmanToolControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_tool_control_finalize;
}

static void
picman_tool_control_init (PicmanToolControl *control)
{
  control->active                 = FALSE;
  control->paused_count           = 0;

  control->preserve               = TRUE;
  control->scroll_lock            = FALSE;
  control->handle_empty_image     = FALSE;

  control->dirty_mask             = PICMAN_DIRTY_NONE;
  control->motion_mode            = PICMAN_MOTION_MODE_EXACT;

  control->auto_snap_to           = TRUE;
  control->snap_offset_x          = 0;
  control->snap_offset_y          = 0;
  control->snap_width             = 0;
  control->snap_height            = 0;

  control->precision              = PICMAN_CURSOR_PRECISION_PIXEL_CENTER;

  control->toggled                = FALSE;

  control->wants_click            = FALSE;
  control->wants_double_click     = FALSE;
  control->wants_triple_click     = FALSE;
  control->wants_all_key_events   = FALSE;

  control->cursor                 = PICMAN_CURSOR_MOUSE;
  control->tool_cursor            = PICMAN_TOOL_CURSOR_NONE;
  control->cursor_modifier        = PICMAN_CURSOR_MODIFIER_NONE;

  control->toggle_cursor          = -1;
  control->toggle_tool_cursor     = -1;
  control->toggle_cursor_modifier = -1;

  control->action_value_1         = NULL;
  control->action_value_2         = NULL;
  control->action_value_3         = NULL;
  control->action_value_4         = NULL;

  control->action_object_1        = NULL;
  control->action_object_2        = NULL;
}

static void
picman_tool_control_finalize (GObject *object)
{
  PicmanToolControl *control = PICMAN_TOOL_CONTROL (object);

  g_slist_free (control->preserve_stack);

  g_free (control->action_value_1);
  g_free (control->action_value_2);
  g_free (control->action_value_3);
  g_free (control->action_value_4);
  g_free (control->action_object_1);
  g_free (control->action_object_2);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/*  public functions  */

void
picman_tool_control_activate (PicmanToolControl *control)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));
  g_return_if_fail (control->active == FALSE);

  control->active = TRUE;
}

void
picman_tool_control_halt (PicmanToolControl *control)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));
  g_return_if_fail (control->active == TRUE);

  control->active = FALSE;
}

gboolean
picman_tool_control_is_active (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->active;
}

void
picman_tool_control_pause (PicmanToolControl *control)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->paused_count++;
}

void
picman_tool_control_resume (PicmanToolControl *control)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));
  g_return_if_fail (control->paused_count > 0);

  control->paused_count--;
}

gboolean
picman_tool_control_is_paused (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->paused_count > 0;
}

void
picman_tool_control_set_preserve (PicmanToolControl *control,
                                gboolean         preserve)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->preserve = preserve ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_preserve (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->preserve;
}

void
picman_tool_control_push_preserve (PicmanToolControl *control,
                                 gboolean         preserve)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->preserve_stack =
    g_slist_prepend (control->preserve_stack,
                     GINT_TO_POINTER (control->preserve));

  control->preserve = preserve ? TRUE : FALSE;
}

void
picman_tool_control_pop_preserve (PicmanToolControl *control)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));
  g_return_if_fail (control->preserve_stack != NULL);

  control->preserve = GPOINTER_TO_INT (control->preserve_stack->data);

  control->preserve_stack = g_slist_delete_link (control->preserve_stack,
                                                 control->preserve_stack);
}

void
picman_tool_control_set_scroll_lock (PicmanToolControl *control,
                                   gboolean         scroll_lock)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->scroll_lock = scroll_lock ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_scroll_lock (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->scroll_lock;
}

void
picman_tool_control_set_handle_empty_image (PicmanToolControl *control,
                                          gboolean         handle_empty)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->handle_empty_image = handle_empty ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_handle_empty_image (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->handle_empty_image;
}

void
picman_tool_control_set_dirty_mask (PicmanToolControl *control,
                                  PicmanDirtyMask    dirty_mask)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->dirty_mask = dirty_mask;
}

PicmanDirtyMask
picman_tool_control_get_dirty_mask (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), PICMAN_DIRTY_NONE);

  return control->dirty_mask;
}

void
picman_tool_control_set_motion_mode (PicmanToolControl *control,
                                   PicmanMotionMode   motion_mode)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->motion_mode = motion_mode;
}

PicmanMotionMode
picman_tool_control_get_motion_mode (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), PICMAN_MOTION_MODE_EXACT);

  return control->motion_mode;
}

void
picman_tool_control_set_snap_to (PicmanToolControl *control,
                               gboolean         snap_to)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->auto_snap_to = snap_to ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_snap_to (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->auto_snap_to;
}

void
picman_tool_control_set_wants_click (PicmanToolControl *control,
                                   gboolean         wants_click)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->wants_click = wants_click ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_wants_click (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->wants_click;
}

void
picman_tool_control_set_wants_double_click (PicmanToolControl *control,
                                          gboolean         wants_double_click)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->wants_double_click = wants_double_click ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_wants_double_click (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->wants_double_click;
}

void
picman_tool_control_set_wants_triple_click (PicmanToolControl *control,
                                          gboolean         wants_triple_click)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->wants_triple_click = wants_triple_click ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_wants_triple_click (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->wants_triple_click;
}

void
picman_tool_control_set_wants_all_key_events (PicmanToolControl *control,
                                            gboolean         wants_key_events)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->wants_all_key_events = wants_key_events ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_wants_all_key_events (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->wants_all_key_events;
}

void
picman_tool_control_set_snap_offsets (PicmanToolControl *control,
                                    gint             offset_x,
                                    gint             offset_y,
                                    gint             width,
                                    gint             height)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->snap_offset_x = offset_x;
  control->snap_offset_y = offset_y;
  control->snap_width    = width;
  control->snap_height   = height;
}

void
picman_tool_control_get_snap_offsets (PicmanToolControl *control,
                                    gint            *offset_x,
                                    gint            *offset_y,
                                    gint            *width,
                                    gint            *height)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (offset_x) *offset_x = control->snap_offset_x;
  if (offset_y) *offset_y = control->snap_offset_y;
  if (width)    *width    = control->snap_width;
  if (height)   *height   = control->snap_height;
}

void
picman_tool_control_set_precision (PicmanToolControl     *control,
                                 PicmanCursorPrecision  precision)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->precision = precision;
}

PicmanCursorPrecision
picman_tool_control_get_precision (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control),
                        PICMAN_CURSOR_PRECISION_PIXEL_CENTER);

  return control->precision;
}

void
picman_tool_control_set_toggled (PicmanToolControl *control,
                               gboolean         toggled)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->toggled = toggled ? TRUE : FALSE;
}

gboolean
picman_tool_control_get_toggled (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  return control->toggled;
}

void
picman_tool_control_set_cursor (PicmanToolControl *control,
                              PicmanCursorType   cursor)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->cursor = cursor;
}

void
picman_tool_control_set_tool_cursor (PicmanToolControl    *control,
                                   PicmanToolCursorType  cursor)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->tool_cursor = cursor;
}

void
picman_tool_control_set_cursor_modifier (PicmanToolControl    *control,
                                       PicmanCursorModifier  modifier)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->cursor_modifier = modifier;
}

void
picman_tool_control_set_toggle_cursor (PicmanToolControl *control,
                                     PicmanCursorType   cursor)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->toggle_cursor = cursor;
}

void
picman_tool_control_set_toggle_tool_cursor (PicmanToolControl    *control,
                                          PicmanToolCursorType  cursor)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->toggle_tool_cursor = cursor;
}

void
picman_tool_control_set_toggle_cursor_modifier (PicmanToolControl    *control,
                                              PicmanCursorModifier  modifier)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  control->toggle_cursor_modifier = modifier;
}

PicmanCursorType
picman_tool_control_get_cursor (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  if (control->toggled && control->toggle_cursor != -1)
    return control->toggle_cursor;

  return control->cursor;
}

PicmanToolCursorType
picman_tool_control_get_tool_cursor (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  if (control->toggled && control->toggle_tool_cursor != -1)
    return control->toggle_tool_cursor;

  return control->tool_cursor;
}

PicmanCursorModifier
picman_tool_control_get_cursor_modifier (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), FALSE);

  if (control->toggled && control->toggle_cursor_modifier != -1)
    return control->toggle_cursor_modifier;

  return control->cursor_modifier;
}

void
picman_tool_control_set_action_value_1 (PicmanToolControl *control,
                                      const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_value_1)
    {
      g_free (control->action_value_1);
      control->action_value_1 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_value_1 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_value_1;
}

void
picman_tool_control_set_action_value_2 (PicmanToolControl *control,
                                      const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_value_2)
    {
      g_free (control->action_value_2);
      control->action_value_2 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_value_2 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_value_2;
}

void
picman_tool_control_set_action_value_3 (PicmanToolControl *control,
                                      const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_value_3)
    {
      g_free (control->action_value_3);
      control->action_value_3 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_value_3 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_value_3;
}

void
picman_tool_control_set_action_value_4 (PicmanToolControl *control,
                                      const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_value_4)
    {
      g_free (control->action_value_4);
      control->action_value_4 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_value_4 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_value_4;
}

void
picman_tool_control_set_action_object_1 (PicmanToolControl *control,
                                       const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_object_1)
    {
      g_free (control->action_object_1);
      control->action_object_1 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_object_1 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_object_1;
}

void
picman_tool_control_set_action_object_2 (PicmanToolControl *control,
                                       const gchar     *action)
{
  g_return_if_fail (PICMAN_IS_TOOL_CONTROL (control));

  if (action != control->action_object_2)
    {
      g_free (control->action_object_2);
      control->action_object_2 = g_strdup (action);
    }
}

const gchar *
picman_tool_control_get_action_object_2 (PicmanToolControl *control)
{
  g_return_val_if_fail (PICMAN_IS_TOOL_CONTROL (control), NULL);

  return control->action_object_2;
}
