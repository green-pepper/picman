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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"
#include "core/picmantoolinfo.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-cursor.h"
#include "display/picmanstatusbar.h"

#include "picmantool.h"
#include "picmantool-progress.h"
#include "picmantoolcontrol.h"

#include "picman-log.h"
#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_TOOL_INFO
};


static void       picman_tool_constructed         (GObject               *object);
static void       picman_tool_dispose             (GObject               *object);
static void       picman_tool_finalize            (GObject               *object);
static void       picman_tool_set_property        (GObject               *object,
                                                 guint                  property_id,
                                                 const GValue          *value,
                                                 GParamSpec            *pspec);
static void       picman_tool_get_property        (GObject               *object,
                                                 guint                  property_id,
                                                 GValue                *value,
                                                 GParamSpec            *pspec);

static gboolean   picman_tool_real_has_display    (PicmanTool              *tool,
                                                 PicmanDisplay           *display);
static PicmanDisplay * picman_tool_real_has_image   (PicmanTool              *tool,
                                                 PicmanImage             *image);
static gboolean   picman_tool_real_initialize     (PicmanTool              *tool,
                                                 PicmanDisplay           *display,
                                                 GError               **error);
static void       picman_tool_real_control        (PicmanTool              *tool,
                                                 PicmanToolAction         action,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_button_press   (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 guint32                time,
                                                 GdkModifierType        state,
                                                 PicmanButtonPressType    press_type,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_button_release (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 guint32                time,
                                                 GdkModifierType        state,
                                                 PicmanButtonReleaseType  release_type,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_motion         (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 guint32                time,
                                                 GdkModifierType        state,
                                                 PicmanDisplay           *display);
static gboolean   picman_tool_real_key_press      (PicmanTool              *tool,
                                                 GdkEventKey           *kevent,
                                                 PicmanDisplay           *display);
static gboolean   picman_tool_real_key_release    (PicmanTool              *tool,
                                                 GdkEventKey           *kevent,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_modifier_key   (PicmanTool              *tool,
                                                 GdkModifierType        key,
                                                 gboolean               press,
                                                 GdkModifierType        state,
                                                 PicmanDisplay           *display);
static void  picman_tool_real_active_modifier_key (PicmanTool              *tool,
                                                 GdkModifierType        key,
                                                 gboolean               press,
                                                 GdkModifierType        state,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_oper_update    (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 GdkModifierType        state,
                                                 gboolean               proximity,
                                                 PicmanDisplay           *display);
static void       picman_tool_real_cursor_update  (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 GdkModifierType        state,
                                                 PicmanDisplay           *display);
static PicmanUIManager * picman_tool_real_get_popup (PicmanTool              *tool,
                                                 const PicmanCoords      *coords,
                                                 GdkModifierType        state,
                                                 PicmanDisplay           *display,
                                                 const gchar          **ui_path);
static void       picman_tool_real_options_notify (PicmanTool              *tool,
                                                 PicmanToolOptions       *options,
                                                 const GParamSpec      *pspec);

static void       picman_tool_options_notify      (PicmanToolOptions       *options,
                                                 const GParamSpec      *pspec,
                                                 PicmanTool              *tool);
static void       picman_tool_clear_status        (PicmanTool              *tool);


G_DEFINE_TYPE_WITH_CODE (PicmanTool, picman_tool, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_tool_progress_iface_init))

#define parent_class picman_tool_parent_class

static gint global_tool_ID = 1;


static void
picman_tool_class_init (PicmanToolClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_tool_constructed;
  object_class->dispose      = picman_tool_dispose;
  object_class->finalize     = picman_tool_finalize;
  object_class->set_property = picman_tool_set_property;
  object_class->get_property = picman_tool_get_property;

  klass->has_display         = picman_tool_real_has_display;
  klass->has_image           = picman_tool_real_has_image;
  klass->initialize          = picman_tool_real_initialize;
  klass->control             = picman_tool_real_control;
  klass->button_press        = picman_tool_real_button_press;
  klass->button_release      = picman_tool_real_button_release;
  klass->motion              = picman_tool_real_motion;
  klass->key_press           = picman_tool_real_key_press;
  klass->key_release         = picman_tool_real_key_release;
  klass->modifier_key        = picman_tool_real_modifier_key;
  klass->active_modifier_key = picman_tool_real_active_modifier_key;
  klass->oper_update         = picman_tool_real_oper_update;
  klass->cursor_update       = picman_tool_real_cursor_update;
  klass->get_popup           = picman_tool_real_get_popup;
  klass->options_notify      = picman_tool_real_options_notify;

  g_object_class_install_property (object_class, PROP_TOOL_INFO,
                                   g_param_spec_object ("tool-info",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TOOL_INFO,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_tool_init (PicmanTool *tool)
{
  tool->tool_info             = NULL;
  tool->ID                    = global_tool_ID++;
  tool->control               = g_object_new (PICMAN_TYPE_TOOL_CONTROL, NULL);
  tool->display               = NULL;
  tool->drawable              = NULL;
  tool->focus_display         = NULL;
  tool->modifier_state        = 0;
  tool->active_modifier_state = 0;
  tool->button_press_state    = 0;
}

static void
picman_tool_constructed (GObject *object)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_TOOL_INFO (tool->tool_info));

  g_signal_connect_object (picman_tool_get_options (tool), "notify",
                           G_CALLBACK (picman_tool_options_notify),
                           tool, 0);
}

static void
picman_tool_dispose (GObject *object)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_tool_finalize (GObject *object)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  if (tool->tool_info)
    {
      g_object_unref (tool->tool_info);
      tool->tool_info = NULL;
    }

  if (tool->control)
    {
      g_object_unref (tool->control);
      tool->control = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_tool_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  switch (property_id)
    {
    case PROP_TOOL_INFO:
      tool->tool_info = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  switch (property_id)
    {
    case PROP_TOOL_INFO:
      g_value_set_object (value, tool->tool_info);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  standard member functions  */

static gboolean
picman_tool_real_has_display (PicmanTool    *tool,
                            PicmanDisplay *display)
{
  return (display == tool->display ||
          g_list_find (tool->status_displays, display));
}

static PicmanDisplay *
picman_tool_real_has_image (PicmanTool  *tool,
                          PicmanImage *image)
{
  if (tool->display)
    {
      if (image && picman_display_get_image (tool->display) == image)
        return tool->display;

      /*  NULL image means any display  */
      if (! image)
        return tool->display;
    }

  return NULL;
}

static gboolean
picman_tool_real_initialize (PicmanTool     *tool,
                           PicmanDisplay  *display,
                           GError      **error)
{
  return TRUE;
}

static void
picman_tool_real_control (PicmanTool       *tool,
                        PicmanToolAction  action,
                        PicmanDisplay    *display)
{
  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      tool->display = NULL;
      break;
    }
}

static void
picman_tool_real_button_press (PicmanTool            *tool,
                             const PicmanCoords    *coords,
                             guint32              time,
                             GdkModifierType      state,
                             PicmanButtonPressType  press_type,
                             PicmanDisplay         *display)
{
  if (press_type == PICMAN_BUTTON_PRESS_NORMAL)
    {
      PicmanImage *image = picman_display_get_image (display);

      tool->display  = display;
      tool->drawable = picman_image_get_active_drawable (image);

      picman_tool_control_activate (tool->control);
    }
}

static void
picman_tool_real_button_release (PicmanTool              *tool,
                               const PicmanCoords      *coords,
                               guint32                time,
                               GdkModifierType        state,
                               PicmanButtonReleaseType  release_type,
                               PicmanDisplay           *display)
{
  picman_tool_control_halt (tool->control);
}

static void
picman_tool_real_motion (PicmanTool         *tool,
                       const PicmanCoords *coords,
                       guint32           time,
                       GdkModifierType   state,
                       PicmanDisplay      *display)
{
}

static gboolean
picman_tool_real_key_press (PicmanTool    *tool,
                          GdkEventKey *kevent,
                          PicmanDisplay *display)
{
  return FALSE;
}

static gboolean
picman_tool_real_key_release (PicmanTool    *tool,
                            GdkEventKey *kevent,
                            PicmanDisplay *display)
{
  return FALSE;
}

static void
picman_tool_real_modifier_key (PicmanTool        *tool,
                             GdkModifierType  key,
                             gboolean         press,
                             GdkModifierType  state,
                             PicmanDisplay     *display)
{
}

static void
picman_tool_real_active_modifier_key (PicmanTool        *tool,
                                    GdkModifierType  key,
                                    gboolean         press,
                                    GdkModifierType  state,
                                    PicmanDisplay     *display)
{
}

static void
picman_tool_real_oper_update (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            GdkModifierType   state,
                            gboolean          proximity,
                            PicmanDisplay      *display)
{
}

static void
picman_tool_real_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  picman_tool_set_cursor (tool, display,
                        picman_tool_control_get_cursor (tool->control),
                        picman_tool_control_get_tool_cursor (tool->control),
                        picman_tool_control_get_cursor_modifier (tool->control));
}

static PicmanUIManager *
picman_tool_real_get_popup (PicmanTool         *tool,
                          const PicmanCoords *coords,
                          GdkModifierType   state,
                          PicmanDisplay      *display,
                          const gchar     **ui_path)
{
  *ui_path = NULL;

  return NULL;
}

static void
picman_tool_real_options_notify (PicmanTool         *tool,
                               PicmanToolOptions  *options,
                               const GParamSpec *pspec)
{
}


/*  public functions  */

PicmanToolOptions *
picman_tool_get_options (PicmanTool *tool)
{
  g_return_val_if_fail (PICMAN_IS_TOOL (tool), NULL);
  g_return_val_if_fail (PICMAN_IS_TOOL_INFO (tool->tool_info), NULL);

  return tool->tool_info->tool_options;
}

gboolean
picman_tool_has_display (PicmanTool    *tool,
                       PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_TOOL (tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);

  return PICMAN_TOOL_GET_CLASS (tool)->has_display (tool, display);
}

PicmanDisplay *
picman_tool_has_image (PicmanTool  *tool,
                     PicmanImage *image)
{
  PicmanDisplay *display;

  g_return_val_if_fail (PICMAN_IS_TOOL (tool), NULL);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), NULL);

  display = PICMAN_TOOL_GET_CLASS (tool)->has_image (tool, image);

  /*  check status displays last because they don't affect the tool
   *  itself (unlike tool->display or draw_tool->display)
   */
  if (! display && tool->status_displays)
    {
      GList *list;

      for (list = tool->status_displays; list; list = g_list_next (list))
        {
          PicmanDisplay *status_display = list->data;

          if (picman_display_get_image (status_display) == image)
            return status_display;
        }

      /*  NULL image means any display  */
      if (! image)
        return tool->status_displays->data;
    }

  return display;
}

gboolean
picman_tool_initialize (PicmanTool    *tool,
                      PicmanDisplay *display)
{
  GError *error = NULL;

  g_return_val_if_fail (PICMAN_IS_TOOL (tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);

  if (! PICMAN_TOOL_GET_CLASS (tool)->initialize (tool, display, &error))
    {
      if (error)
        {
          picman_tool_message_literal (tool, display, error->message);
          g_clear_error (&error);
        }

      return FALSE;
    }

  return TRUE;
}

void
picman_tool_control (PicmanTool       *tool,
                   PicmanToolAction  action,
                   PicmanDisplay    *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
      if (! picman_tool_control_is_paused (tool->control))
        PICMAN_TOOL_GET_CLASS (tool)->control (tool, action, display);

      picman_tool_control_pause (tool->control);
      break;

    case PICMAN_TOOL_ACTION_RESUME:
      if (picman_tool_control_is_paused (tool->control))
        {
          picman_tool_control_resume (tool->control);

          if (! picman_tool_control_is_paused (tool->control))
            PICMAN_TOOL_GET_CLASS (tool)->control (tool, action, display);
        }
      else
        {
          g_warning ("picman_tool_control: unable to RESUME tool with "
                     "tool->control->paused_count == 0");
        }
      break;

    case PICMAN_TOOL_ACTION_HALT:
      PICMAN_TOOL_GET_CLASS (tool)->control (tool, action, display);

      if (picman_tool_control_is_active (tool->control))
        picman_tool_control_halt (tool->control);

      picman_tool_clear_status (tool);
      break;
    }
}

void
picman_tool_button_press (PicmanTool            *tool,
                        const PicmanCoords    *coords,
                        guint32              time,
                        GdkModifierType      state,
                        PicmanButtonPressType  press_type,
                        PicmanDisplay         *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (coords != NULL);
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  PICMAN_TOOL_GET_CLASS (tool)->button_press (tool, coords, time, state,
                                            press_type, display);

  if (press_type == PICMAN_BUTTON_PRESS_NORMAL &&
      picman_tool_control_is_active (tool->control))
    {
      tool->button_press_state    = state;
      tool->active_modifier_state = state;

      if (picman_tool_control_get_wants_click (tool->control))
        {
          tool->in_click_distance   = TRUE;
          tool->got_motion_event    = FALSE;
          tool->button_press_coords = *coords;
          tool->button_press_time   = time;
        }
      else
        {
          tool->in_click_distance   = FALSE;
        }
    }
}

static gboolean
picman_tool_check_click_distance (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                guint32           time,
                                PicmanDisplay      *display)
{
  PicmanDisplayShell *shell;
  gint              double_click_time;
  gint              double_click_distance;

  if (! tool->in_click_distance)
    return FALSE;

  shell = picman_display_get_shell (display);

  g_object_get (gtk_widget_get_settings (GTK_WIDGET (shell)),
                "gtk-double-click-time",     &double_click_time,
                "gtk-double-click-distance", &double_click_distance,
                NULL);

  if ((time - tool->button_press_time) > double_click_time)
    {
      tool->in_click_distance = FALSE;
    }
  else
    {
      PicmanDisplayShell *shell = picman_display_get_shell (display);
      gdouble           dx;
      gdouble           dy;

      dx = SCALEX (shell, tool->button_press_coords.x - coords->x);
      dy = SCALEY (shell, tool->button_press_coords.y - coords->y);

      if ((SQR (dx) + SQR (dy)) > SQR (double_click_distance))
        {
          tool->in_click_distance = FALSE;
        }
    }

  return tool->in_click_distance;
}

void
picman_tool_button_release (PicmanTool         *tool,
                          const PicmanCoords *coords,
                          guint32           time,
                          GdkModifierType   state,
                          PicmanDisplay      *display)
{
  PicmanButtonReleaseType release_type = PICMAN_BUTTON_RELEASE_NORMAL;
  PicmanCoords            my_coords;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (coords != NULL);
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == TRUE);

  g_object_ref (tool);

  my_coords = *coords;

  if (state & GDK_BUTTON3_MASK)
    {
      release_type = PICMAN_BUTTON_RELEASE_CANCEL;
    }
  else if (picman_tool_control_get_wants_click (tool->control))
    {
      if (picman_tool_check_click_distance (tool, coords, time, display))
        {
          release_type = PICMAN_BUTTON_RELEASE_CLICK;
          my_coords    = tool->button_press_coords;

          /*  synthesize a motion event back to the recorded press
           *  coordinates
           */
          PICMAN_TOOL_GET_CLASS (tool)->motion (tool, &my_coords, time,
                                              state & GDK_BUTTON1_MASK,
                                              display);
        }
      else if (! tool->got_motion_event)
        {
          release_type = PICMAN_BUTTON_RELEASE_NO_MOTION;
        }
    }

  PICMAN_TOOL_GET_CLASS (tool)->button_release (tool, &my_coords, time, state,
                                              release_type, display);

  g_warn_if_fail (picman_tool_control_is_active (tool->control) == FALSE);

  if (tool->active_modifier_state != 0)
    {
      picman_tool_control_activate (tool->control);

      picman_tool_set_active_modifier_state (tool, 0, display);

      picman_tool_control_halt (tool->control);
    }

  tool->button_press_state = 0;

  g_object_unref (tool);
}

void
picman_tool_motion (PicmanTool         *tool,
                  const PicmanCoords *coords,
                  guint32           time,
                  GdkModifierType   state,
                  PicmanDisplay      *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (coords != NULL);
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == TRUE);

  tool->got_motion_event = TRUE;

  PICMAN_TOOL_GET_CLASS (tool)->motion (tool, coords, time, state, display);
}

void
picman_tool_set_focus_display (PicmanTool    *tool,
                             PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (display == NULL || PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == FALSE);

  PICMAN_LOG (TOOL_FOCUS, "tool: %p  focus_display: %p  tool->focus_display: %p",
            tool, display, tool->focus_display);

  if (display != tool->focus_display)
    {
      if (tool->focus_display)
        {
          if (tool->active_modifier_state != 0)
            {
              picman_tool_control_activate (tool->control);

              picman_tool_set_active_modifier_state (tool, 0, tool->focus_display);

              picman_tool_control_halt (tool->control);
            }

          if (tool->modifier_state != 0)
            picman_tool_set_modifier_state (tool, 0, tool->focus_display);
        }

      tool->focus_display = display;
    }
}

gboolean
picman_tool_key_press (PicmanTool    *tool,
                     GdkEventKey *kevent,
                     PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_TOOL (tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);
  g_return_val_if_fail (display == tool->focus_display, FALSE);
  g_return_val_if_fail (picman_tool_control_is_active (tool->control) == FALSE,
                        FALSE);

  return PICMAN_TOOL_GET_CLASS (tool)->key_press (tool, kevent, display);
}

gboolean
picman_tool_key_release (PicmanTool    *tool,
                       GdkEventKey *kevent,
                       PicmanDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_TOOL (tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);
  g_return_val_if_fail (display == tool->focus_display, FALSE);
  g_return_val_if_fail (picman_tool_control_is_active (tool->control) == FALSE,
                        FALSE);

  return PICMAN_TOOL_GET_CLASS (tool)->key_release (tool, kevent, display);
}

static void
picman_tool_modifier_key (PicmanTool        *tool,
                        GdkModifierType  key,
                        gboolean         press,
                        GdkModifierType  state,
                        PicmanDisplay     *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (display == tool->focus_display);

  PICMAN_TOOL_GET_CLASS (tool)->modifier_key (tool, key, press, state, display);
}

void
picman_tool_set_modifier_state (PicmanTool        *tool,
                              GdkModifierType  state,
                              PicmanDisplay     *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == FALSE);

  PICMAN_LOG (TOOL_FOCUS, "tool: %p  display: %p  tool->focus_display: %p",
            tool, display, tool->focus_display);

  g_return_if_fail (display == tool->focus_display);

  if ((tool->modifier_state & GDK_SHIFT_MASK) != (state & GDK_SHIFT_MASK))
    {
      picman_tool_modifier_key (tool, GDK_SHIFT_MASK,
                              (state & GDK_SHIFT_MASK) ? TRUE : FALSE, state,
                              display);
    }

  if ((tool->modifier_state & GDK_CONTROL_MASK) != (state & GDK_CONTROL_MASK))
    {
      picman_tool_modifier_key (tool, GDK_CONTROL_MASK,
                              (state & GDK_CONTROL_MASK) ? TRUE : FALSE, state,
                              display);
    }

  if ((tool->modifier_state & GDK_MOD1_MASK) != (state & GDK_MOD1_MASK))
    {
      picman_tool_modifier_key (tool, GDK_MOD1_MASK,
                              (state & GDK_MOD1_MASK) ? TRUE : FALSE, state,
                              display);
    }

  if ((tool->modifier_state & GDK_MOD2_MASK) != (state & GDK_MOD2_MASK))
    {
      picman_tool_modifier_key (tool, GDK_MOD2_MASK,
                              (state & GDK_MOD2_MASK) ? TRUE : FALSE, state,
                              display);
    }

  tool->modifier_state = state;
}

static void
picman_tool_active_modifier_key (PicmanTool        *tool,
                               GdkModifierType  key,
                               gboolean         press,
                               GdkModifierType  state,
                               PicmanDisplay     *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (display == tool->focus_display);

  PICMAN_TOOL_GET_CLASS (tool)->active_modifier_key (tool, key, press, state,
                                                   display);
}

void
picman_tool_set_active_modifier_state (PicmanTool        *tool,
                                     GdkModifierType  state,
                                     PicmanDisplay     *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == TRUE);

  PICMAN_LOG (TOOL_FOCUS, "tool: %p  display: %p  tool->focus_display: %p",
            tool, display, tool->focus_display);

  g_return_if_fail (display == tool->focus_display);

  if ((tool->active_modifier_state & GDK_SHIFT_MASK) !=
      (state & GDK_SHIFT_MASK))
    {
      gboolean press = state & GDK_SHIFT_MASK;

#ifdef DEBUG_ACTIVE_STATE
      g_printerr ("%s: SHIFT %s\n", G_STRFUNC,
                  press ? "pressed" : "released");
#endif

      if (! press && (tool->button_press_state & GDK_SHIFT_MASK))
        {
          tool->button_press_state &= ~GDK_SHIFT_MASK;
        }
      else
        {
          picman_tool_active_modifier_key (tool, GDK_SHIFT_MASK,
                                         press, state,
                                         display);
        }
    }

  if ((tool->active_modifier_state & GDK_CONTROL_MASK) !=
      (state & GDK_CONTROL_MASK))
    {
      gboolean press = state & GDK_CONTROL_MASK;

#ifdef DEBUG_ACTIVE_STATE
      g_printerr ("%s: CONTROL %s\n", G_STRFUNC,
                  press ? "pressed" : "released");
#endif

      if (! press && (tool->button_press_state & GDK_CONTROL_MASK))
        {
          tool->button_press_state &= ~GDK_CONTROL_MASK;
        }
      else
        {
          picman_tool_active_modifier_key (tool, GDK_CONTROL_MASK,
                                         press, state,
                                         display);
        }
    }

  if ((tool->active_modifier_state & GDK_MOD1_MASK) !=
      (state & GDK_MOD1_MASK))
    {
      gboolean press = state & GDK_MOD1_MASK;

#ifdef DEBUG_ACTIVE_STATE
      g_printerr ("%s: ALT %s\n", G_STRFUNC,
                  press ? "pressed" : "released");
#endif

      if (! press && (tool->button_press_state & GDK_MOD1_MASK))
        {
          tool->button_press_state &= ~GDK_MOD1_MASK;
        }
      else
        {
          picman_tool_active_modifier_key (tool, GDK_MOD1_MASK,
                                         press, state,
                                         display);
        }
    }

  if ((tool->active_modifier_state & GDK_MOD2_MASK) !=
      (state & GDK_MOD2_MASK))
    {
      gboolean press = state & GDK_MOD2_MASK;

#ifdef DEBUG_ACTIVE_STATE
      g_printerr ("%s: MOD2 %s\n", G_STRFUNC,
                  press ? "pressed" : "released");
#endif

      if (! press && (tool->button_press_state & GDK_MOD2_MASK))
        {
          tool->button_press_state &= ~GDK_MOD2_MASK;
        }
      else
        {
          picman_tool_active_modifier_key (tool, GDK_MOD2_MASK,
                                         press, state,
                                         display);
        }
    }

  tool->active_modifier_state = state;
}

void
picman_tool_oper_update (PicmanTool         *tool,
                       const PicmanCoords *coords,
                       GdkModifierType   state,
                       gboolean          proximity,
                       PicmanDisplay      *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (coords != NULL);
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == FALSE);

  PICMAN_TOOL_GET_CLASS (tool)->oper_update (tool, coords, state, proximity,
                                           display);

  if (G_UNLIKELY (picman_image_is_empty (picman_display_get_image (display)) &&
                  ! picman_tool_control_get_handle_empty_image (tool->control)))
    {
      picman_tool_replace_status (tool, display,
                                "%s",
                                _("Can't work on an empty image, "
                                  "add a layer first"));
    }
}

void
picman_tool_cursor_update (PicmanTool         *tool,
                         const PicmanCoords *coords,
                         GdkModifierType   state,
                         PicmanDisplay      *display)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (coords != NULL);
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_tool_control_is_active (tool->control) == FALSE);

  PICMAN_TOOL_GET_CLASS (tool)->cursor_update (tool, coords, state, display);
}

PicmanUIManager *
picman_tool_get_popup (PicmanTool         *tool,
                     const PicmanCoords *coords,
                     GdkModifierType   state,
                     PicmanDisplay      *display,
                     const gchar     **ui_path)
{
  g_return_val_if_fail (PICMAN_IS_TOOL (tool), NULL);
  g_return_val_if_fail (coords != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);
  g_return_val_if_fail (ui_path != NULL, NULL);

  return PICMAN_TOOL_GET_CLASS (tool)->get_popup (tool, coords, state, display,
                                                ui_path);
}

void
picman_tool_push_status (PicmanTool    *tool,
                       PicmanDisplay *display,
                       const gchar *format,
                       ...)
{
  PicmanDisplayShell *shell;
  const gchar      *stock_id;
  va_list           args;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (format != NULL);

  shell = picman_display_get_shell (display);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool->tool_info));

  va_start (args, format);

  picman_statusbar_push_valist (picman_display_shell_get_statusbar (shell),
                              G_OBJECT_TYPE_NAME (tool), stock_id,
                              format, args);

  va_end (args);

  tool->status_displays = g_list_remove (tool->status_displays, display);
  tool->status_displays = g_list_prepend (tool->status_displays, display);
}

void
picman_tool_push_status_coords (PicmanTool            *tool,
                              PicmanDisplay         *display,
                              PicmanCursorPrecision  precision,
                              const gchar         *title,
                              gdouble              x,
                              const gchar         *separator,
                              gdouble              y,
                              const gchar         *help)
{
  PicmanDisplayShell *shell;
  const gchar      *stock_id;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  shell = picman_display_get_shell (display);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool->tool_info));

  picman_statusbar_push_coords (picman_display_shell_get_statusbar (shell),
                              G_OBJECT_TYPE_NAME (tool), stock_id,
                              precision, title, x, separator, y,
                              help);

  tool->status_displays = g_list_remove (tool->status_displays, display);
  tool->status_displays = g_list_prepend (tool->status_displays, display);
}

void
picman_tool_push_status_length (PicmanTool            *tool,
                              PicmanDisplay         *display,
                              const gchar         *title,
                              PicmanOrientationType  axis,
                              gdouble              value,
                              const gchar         *help)
{
  PicmanDisplayShell *shell;
  const gchar      *stock_id;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  shell = picman_display_get_shell (display);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool->tool_info));

  picman_statusbar_push_length (picman_display_shell_get_statusbar (shell),
                              G_OBJECT_TYPE_NAME (tool), stock_id,
                              title, axis, value, help);

  tool->status_displays = g_list_remove (tool->status_displays, display);
  tool->status_displays = g_list_prepend (tool->status_displays, display);
}

void
picman_tool_replace_status (PicmanTool    *tool,
                          PicmanDisplay *display,
                          const gchar *format,
                          ...)
{
  PicmanDisplayShell *shell;
  const gchar      *stock_id;
  va_list           args;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (format != NULL);

  shell = picman_display_get_shell (display);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool->tool_info));

  va_start (args, format);

  picman_statusbar_replace_valist (picman_display_shell_get_statusbar (shell),
                                 G_OBJECT_TYPE_NAME (tool), stock_id,
                                 format, args);

  va_end (args);

  tool->status_displays = g_list_remove (tool->status_displays, display);
  tool->status_displays = g_list_prepend (tool->status_displays, display);
}

void
picman_tool_pop_status (PicmanTool    *tool,
                      PicmanDisplay *display)
{
  PicmanDisplayShell *shell;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  shell = picman_display_get_shell (display);

  picman_statusbar_pop (picman_display_shell_get_statusbar (shell),
                      G_OBJECT_TYPE_NAME (tool));

  tool->status_displays = g_list_remove (tool->status_displays, display);
}

void
picman_tool_message (PicmanTool    *tool,
                   PicmanDisplay *display,
                   const gchar *format,
                   ...)
{
  va_list args;

  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (format != NULL);

  va_start (args, format);

  picman_message_valist (display->picman, G_OBJECT (display),
                       PICMAN_MESSAGE_WARNING, format, args);

  va_end (args);
}

void
picman_tool_message_literal (PicmanTool    *tool,
			   PicmanDisplay *display,
			   const gchar *message)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (message != NULL);

  picman_message_literal (display->picman, G_OBJECT (display),
			PICMAN_MESSAGE_WARNING, message);
}

void
picman_tool_set_cursor (PicmanTool           *tool,
                      PicmanDisplay        *display,
                      PicmanCursorType      cursor,
                      PicmanToolCursorType  tool_cursor,
                      PicmanCursorModifier  modifier)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  picman_display_shell_set_cursor (picman_display_get_shell (display),
                                 cursor, tool_cursor, modifier);
}


/*  private functions  */

static void
picman_tool_options_notify (PicmanToolOptions  *options,
                          const GParamSpec *pspec,
                          PicmanTool         *tool)
{
  PICMAN_TOOL_GET_CLASS (tool)->options_notify (tool, options, pspec);
}

static void
picman_tool_clear_status (PicmanTool *tool)
{
  g_return_if_fail (PICMAN_IS_TOOL (tool));

  while (tool->status_displays)
    picman_tool_pop_status (tool, tool->status_displays->data);
}
