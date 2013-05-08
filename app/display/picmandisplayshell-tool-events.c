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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"
#include "tools/tools-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmanimage.h"

#include "widgets/picmancontrollers.h"
#include "widgets/picmancontrollerkeyboard.h"
#include "widgets/picmancontrollermouse.h"
#include "widgets/picmancontrollerwheel.h"
#include "widgets/picmandeviceinfo.h"
#include "widgets/picmandeviceinfo-coords.h"
#include "widgets/picmandevicemanager.h"
#include "widgets/picmandevices.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "tools/picmanimagemaptool.h"
#include "tools/picmanmovetool.h"
#include "tools/picmanpainttool.h"
#include "tools/picmantoolcontrol.h"
#include "tools/tool_manager.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-autoscroll.h"
#include "picmandisplayshell-cursor.h"
#include "picmandisplayshell-grab.h"
#include "picmandisplayshell-layer-select.h"
#include "picmandisplayshell-rotate.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-tool-events.h"
#include "picmandisplayshell-transform.h"
#include "picmanimagewindow.h"
#include "picmanmotionbuffer.h"

#include "picman-log.h"


/*  local function prototypes  */

static GdkModifierType
                  picman_display_shell_key_to_state             (gint               key);
static GdkModifierType
                  picman_display_shell_button_to_state          (gint               button);

static void       picman_display_shell_proximity_in             (PicmanDisplayShell  *shell);
static void       picman_display_shell_proximity_out            (PicmanDisplayShell  *shell);

static void       picman_display_shell_check_device_cursor      (PicmanDisplayShell  *shell);

static void       picman_display_shell_start_scrolling          (PicmanDisplayShell  *shell,
                                                               const GdkEvent    *event,
                                                               GdkModifierType    state,
                                                               gint               x,
                                                               gint               y);
static void       picman_display_shell_stop_scrolling           (PicmanDisplayShell  *shell,
                                                               const GdkEvent    *event);

static void       picman_display_shell_space_pressed            (PicmanDisplayShell  *shell,
                                                               const GdkEvent    *event);
static void       picman_display_shell_space_released           (PicmanDisplayShell  *shell,
                                                               const GdkEvent    *event,
                                                               const PicmanCoords  *image_coords);

static gboolean   picman_display_shell_tab_pressed              (PicmanDisplayShell  *shell,
                                                               const GdkEventKey *event);

static void       picman_display_shell_update_focus             (PicmanDisplayShell  *shell,
                                                               gboolean           focus_in,
                                                               const PicmanCoords  *image_coords,
                                                               GdkModifierType    state);
static void       picman_display_shell_update_cursor            (PicmanDisplayShell  *shell,
                                                               const PicmanCoords  *display_coords,
                                                               const PicmanCoords  *image_coords,
                                                               GdkModifierType    state,
                                                               gboolean           update_software_cursor);

static gboolean   picman_display_shell_initialize_tool          (PicmanDisplayShell  *shell,
                                                               const PicmanCoords  *image_coords,
                                                               GdkModifierType    state);

static void       picman_display_shell_get_event_coords         (PicmanDisplayShell  *shell,
                                                               const GdkEvent    *event,
                                                               PicmanCoords        *display_coords,
                                                               GdkModifierType   *state,
                                                               guint32           *time);
static void       picman_display_shell_untransform_event_coords (PicmanDisplayShell  *shell,
                                                               const PicmanCoords  *display_coords,
                                                               PicmanCoords        *image_coords,
                                                               gboolean          *update_software_cursor);

static GdkEvent * picman_display_shell_compress_motion          (PicmanDisplayShell  *shell);


/*  public functions  */

gboolean
picman_display_shell_events (GtkWidget        *widget,
                           GdkEvent         *event,
                           PicmanDisplayShell *shell)
{
  Picman     *picman;
  gboolean  set_display = FALSE;

  /*  are we in destruction?  */
  if (! shell->display || ! picman_display_get_shell (shell->display))
    return TRUE;

  picman = picman_display_get_picman (shell->display);

  switch (event->type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      {
        GdkEventKey *kevent = (GdkEventKey *) event;

        if (picman->busy)
          return TRUE;

        /*  do not process any key events while BUTTON1 is down. We do this
         *  so tools keep the modifier state they were in when BUTTON1 was
         *  pressed and to prevent accelerators from being invoked.
         */
        if (kevent->state & GDK_BUTTON1_MASK)
          {
            if (kevent->keyval == GDK_KEY_Shift_L   ||
                kevent->keyval == GDK_KEY_Shift_R   ||
                kevent->keyval == GDK_KEY_Control_L ||
                kevent->keyval == GDK_KEY_Control_R ||
                kevent->keyval == GDK_KEY_Alt_L     ||
                kevent->keyval == GDK_KEY_Alt_R     ||
                kevent->keyval == GDK_KEY_Meta_L    ||
                kevent->keyval == GDK_KEY_Meta_R)
              {
                break;
              }

            if (event->type == GDK_KEY_PRESS)
              {
                if ((kevent->keyval == GDK_KEY_space ||
                     kevent->keyval == GDK_KEY_KP_Space) && shell->space_release_pending)
                  {
                    shell->space_pressed         = TRUE;
                    shell->space_release_pending = FALSE;
                  }
              }
            else
              {
                if ((kevent->keyval == GDK_KEY_space ||
                     kevent->keyval == GDK_KEY_KP_Space) && shell->space_pressed)
                  {
                    shell->space_pressed         = FALSE;
                    shell->space_release_pending = TRUE;
                  }
              }

            return TRUE;
          }

        switch (kevent->keyval)
          {
          case GDK_KEY_Left:      case GDK_KEY_Right:
          case GDK_KEY_Up:        case GDK_KEY_Down:
          case GDK_KEY_space:
          case GDK_KEY_KP_Space:
          case GDK_KEY_Tab:
          case GDK_KEY_ISO_Left_Tab:
          case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
          case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
          case GDK_KEY_Control_L: case GDK_KEY_Control_R:
          case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
          case GDK_KEY_Return:
          case GDK_KEY_KP_Enter:
          case GDK_KEY_ISO_Enter:
          case GDK_KEY_BackSpace:
          case GDK_KEY_Escape:
            break;

          default:
            if (shell->space_pressed || shell->scrolling)
              return TRUE;
            break;
          }

        set_display = TRUE;
        break;
      }

    case GDK_BUTTON_PRESS:
    case GDK_SCROLL:
      set_display = TRUE;
      break;

    case GDK_FOCUS_CHANGE:
      {
        GdkEventFocus *fevent = (GdkEventFocus *) event;

        if (fevent->in && shell->display->config->activate_on_focus)
          set_display = TRUE;
      }
      break;

    default:
      break;
    }

  /*  Setting the context's display automatically sets the image, too  */
  if (set_display)
    picman_context_set_display (picman_get_user_context (picman), shell->display);

  return FALSE;
}

static gboolean
picman_display_shell_canvas_no_image_events (GtkWidget        *canvas,
                                           GdkEvent         *event,
                                           PicmanDisplayShell *shell)
{
  switch (event->type)
    {
    case GDK_2BUTTON_PRESS:
      {
        GdkEventButton  *bevent = (GdkEventButton *) event;
        if (bevent->button == 1)
          {
            PicmanImageWindow *window  = picman_display_shell_get_window (shell);
            PicmanUIManager   *manager = picman_image_window_get_ui_manager (window);

            picman_ui_manager_activate_action (manager, "file", "file-open");
          }
        return TRUE;
      }
      break;

    case GDK_BUTTON_PRESS:
      if (gdk_event_triggers_context_menu (event))
        {
          picman_ui_manager_ui_popup (shell->popup_manager,
                                    "/dummy-menubar/image-popup",
                                    GTK_WIDGET (shell),
                                    NULL, NULL, NULL, NULL);
          return TRUE;
        }
      break;

    case GDK_KEY_PRESS:
      {
        GdkEventKey *kevent = (GdkEventKey *) event;

        if (kevent->keyval == GDK_KEY_Tab ||
            kevent->keyval == GDK_KEY_ISO_Left_Tab)
          {
            return picman_display_shell_tab_pressed (shell, kevent);
          }
      }
      break;

    default:
      break;
    }

  return FALSE;
}

gboolean
picman_display_shell_canvas_tool_events (GtkWidget        *canvas,
                                       GdkEvent         *event,
                                       PicmanDisplayShell *shell)
{
  PicmanDisplay     *display;
  PicmanImage       *image;
  Picman            *picman;
  PicmanCoords       display_coords;
  PicmanCoords       image_coords;
  GdkModifierType  state;
  guint32          time;
  gboolean         device_changed   = FALSE;
  gboolean         return_val       = FALSE;
  gboolean         update_sw_cursor = FALSE;

  g_return_val_if_fail (gtk_widget_get_realized (canvas), FALSE);

  /*  are we in destruction?  */
  if (! shell->display || ! picman_display_get_shell (shell->display))
    return TRUE;

  /*  set the active display before doing any other canvas event processing  */
  if (picman_display_shell_events (canvas, event, shell))
    return TRUE;

  /*  ignore events on overlays, which are the canvas' children
   */
  if (gtk_widget_is_ancestor (gtk_get_event_widget (event), shell->canvas))
    {
      return FALSE;
    }

  display = shell->display;
  picman    = picman_display_get_picman (display);
  image   = picman_display_get_image (display);

  if (! image)
    return picman_display_shell_canvas_no_image_events (canvas, event, shell);

  PICMAN_LOG (TOOL_EVENTS, "event (display %p): %s",
            display, picman_print_event (event));

  /*  Find out what device the event occurred upon  */
  if (! picman->busy && picman_devices_check_change (picman, event))
    {
      picman_display_shell_check_device_cursor (shell);
      device_changed = TRUE;
    }

  picman_display_shell_get_event_coords (shell, event,
                                       &display_coords,
                                       &state, &time);
  picman_display_shell_untransform_event_coords (shell,
                                               &display_coords, &image_coords,
                                               &update_sw_cursor);

  /*  If the device (and maybe the tool) has changed, update the new
   *  tool's state
   */
  if (device_changed && gtk_widget_has_focus (canvas))
    {
      picman_display_shell_update_focus (shell, TRUE,
                                       &image_coords, state);
    }

  switch (event->type)
    {
    case GDK_ENTER_NOTIFY:
      {
        GdkEventCrossing *cevent = (GdkEventCrossing *) event;

        if (cevent->mode != GDK_CROSSING_NORMAL)
          return TRUE;

        /*  ignore enter notify while we have a grab  */
        if (shell->pointer_grabbed)
          return TRUE;

        picman_display_shell_proximity_in (shell);
        update_sw_cursor = TRUE;

        tool_manager_oper_update_active (picman,
                                         &image_coords, state,
                                         shell->proximity,
                                         display);
      }
      break;

    case GDK_LEAVE_NOTIFY:
      {
        GdkEventCrossing *cevent = (GdkEventCrossing *) event;

        if (cevent->mode != GDK_CROSSING_NORMAL)
          return TRUE;

        /*  ignore leave notify while we have a grab  */
        if (shell->pointer_grabbed)
          return TRUE;

        picman_display_shell_proximity_out (shell);

        tool_manager_oper_update_active (picman,
                                         &image_coords, state,
                                         shell->proximity,
                                         display);
      }
      break;

    case GDK_PROXIMITY_IN:
      picman_display_shell_proximity_in (shell);

      tool_manager_oper_update_active (picman,
                                       &image_coords, state,
                                       shell->proximity,
                                       display);
      break;

    case GDK_PROXIMITY_OUT:
      picman_display_shell_proximity_out (shell);

      tool_manager_oper_update_active (picman,
                                       &image_coords, state,
                                       shell->proximity,
                                       display);
      break;

    case GDK_FOCUS_CHANGE:
      {
        GdkEventFocus *fevent = (GdkEventFocus *) event;

        if (fevent->in)
          {
            if (G_UNLIKELY (! gtk_widget_has_focus (canvas)))
              g_warning ("%s: FOCUS_IN but canvas has no focus", G_STRFUNC);

            /*  ignore focus changes while we have a grab  */
            if (shell->pointer_grabbed)
              return TRUE;

            /*   press modifier keys when the canvas gets the focus  */
            picman_display_shell_update_focus (shell, TRUE,
                                             &image_coords, state);
          }
        else
          {
            if (G_UNLIKELY (gtk_widget_has_focus (canvas)))
              g_warning ("%s: FOCUS_OUT but canvas has focus", G_STRFUNC);

            /*  ignore focus changes while we have a grab  */
            if (shell->pointer_grabbed)
              return TRUE;

            /*  release modifier keys when the canvas loses the focus  */
            picman_display_shell_update_focus (shell, FALSE,
                                             &image_coords, 0);
          }
      }
      break;

    case GDK_BUTTON_PRESS:
      {
        GdkEventButton  *bevent = (GdkEventButton *) event;
        GdkModifierType  button_state;

        /*  ignore new mouse events  */
        if (picman->busy || shell->scrolling || shell->pointer_grabbed)
          return TRUE;

        button_state = picman_display_shell_button_to_state (bevent->button);

        state |= button_state;

        /* ignore new buttons while another button is down */
        if (((state & (GDK_BUTTON1_MASK)) && (state & (GDK_BUTTON2_MASK |
                                                       GDK_BUTTON3_MASK))) ||
            ((state & (GDK_BUTTON2_MASK)) && (state & (GDK_BUTTON1_MASK |
                                                       GDK_BUTTON3_MASK))) ||
            ((state & (GDK_BUTTON3_MASK)) && (state & (GDK_BUTTON1_MASK |
                                                       GDK_BUTTON2_MASK))))
          return TRUE;

        /*  focus the widget if it isn't; if the toplevel window
         *  already has focus, this will generate a FOCUS_IN on the
         *  canvas immediately, therefore we do this before logging
         *  the BUTTON_PRESS.
         */
        if (! gtk_widget_has_focus (canvas))
          gtk_widget_grab_focus (canvas);

        /*  if the toplevel window didn't have focus, the above
         *  gtk_widget_grab_focus() didn't set the canvas' HAS_FOCUS
         *  flags, and didn't trigger a FOCUS_IN, but the tool needs
         *  to be set up correctly regardless, so simply do the
         *  same things here, it's safe to do them redundantly.
         */
        picman_display_shell_update_focus (shell, TRUE,
                                         &image_coords, state);
        picman_display_shell_update_cursor (shell, &display_coords,
                                          &image_coords, state & ~button_state,
                                          FALSE);

        if (gdk_event_triggers_context_menu (event))
          {
            PicmanUIManager *ui_manager;
            const gchar   *ui_path;

            ui_manager = tool_manager_get_popup_active (picman,
                                                        &image_coords, state,
                                                        display,
                                                        &ui_path);

            if (ui_manager)
              {
                picman_ui_manager_ui_popup (ui_manager,
                                          ui_path,
                                          GTK_WIDGET (shell),
                                          NULL, NULL, NULL, NULL);
              }
            else
              {
                picman_ui_manager_ui_popup (shell->popup_manager,
                                          "/dummy-menubar/image-popup",
                                          GTK_WIDGET (shell),
                                          NULL, NULL, NULL, NULL);
              }
          }
        else if (bevent->button == 1)
          {
            if (! picman_display_shell_pointer_grab (shell, NULL, 0))
              return TRUE;

            if (! shell->space_pressed && ! shell->space_release_pending)
              if (! picman_display_shell_keyboard_grab (shell, event))
                {
                  picman_display_shell_pointer_ungrab (shell, NULL);
                  return TRUE;
                }

            if (picman_display_shell_initialize_tool (shell,
                                                    &image_coords, state))
              {
                PicmanCoords last_motion;

                /* Use the last evaluated dynamic axes instead of the
                 * button_press event's ones because the click is
                 * usually at the same spot as the last motion event
                 * which would give us bogus dynamics.
                 */
                picman_motion_buffer_begin_stroke (shell->motion_buffer, time,
                                                 &last_motion);

                last_motion.x        = image_coords.x;
                last_motion.y        = image_coords.y;
                last_motion.pressure = image_coords.pressure;
                last_motion.xtilt    = image_coords.xtilt;
                last_motion.ytilt    = image_coords.ytilt;
                last_motion.wheel    = image_coords.wheel;

                image_coords = last_motion;

                tool_manager_button_press_active (picman,
                                                  &image_coords,
                                                  time, state,
                                                  PICMAN_BUTTON_PRESS_NORMAL,
                                                  display);
              }
          }
        else if (bevent->button == 2)
          {
            picman_display_shell_start_scrolling (shell, NULL, state,
                                                bevent->x, bevent->y);
          }

        return_val = TRUE;
      }
      break;

    case GDK_2BUTTON_PRESS:
      {
        GdkEventButton *bevent = (GdkEventButton *) event;
        PicmanTool       *active_tool;

        if (picman->busy)
          return TRUE;

        active_tool = tool_manager_get_active (picman);

        if (bevent->button == 1                                &&
            active_tool                                        &&
            picman_tool_control_is_active (active_tool->control) &&
            picman_tool_control_get_wants_double_click (active_tool->control))
          {
            tool_manager_button_press_active (picman,
                                              &image_coords,
                                              time, state,
                                              PICMAN_BUTTON_PRESS_DOUBLE,
                                              display);
          }

        /*  don't update the cursor again on double click  */
        return TRUE;
      }
      break;

    case GDK_3BUTTON_PRESS:
      {
        GdkEventButton *bevent = (GdkEventButton *) event;
        PicmanTool       *active_tool;

        if (picman->busy)
          return TRUE;

        active_tool = tool_manager_get_active (picman);

        if (bevent->button == 1                                &&
            active_tool                                        &&
            picman_tool_control_is_active (active_tool->control) &&
            picman_tool_control_get_wants_triple_click (active_tool->control))
          {
            tool_manager_button_press_active (picman,
                                              &image_coords,
                                              time, state,
                                              PICMAN_BUTTON_PRESS_TRIPLE,
                                              display);
          }

        /*  don't update the cursor again on triple click  */
        return TRUE;
      }
      break;

    case GDK_BUTTON_RELEASE:
      {
        GdkEventButton *bevent = (GdkEventButton *) event;
        PicmanTool       *active_tool;

        picman_display_shell_autoscroll_stop (shell);

        if (picman->busy)
          return TRUE;

        active_tool = tool_manager_get_active (picman);

        state &= ~picman_display_shell_key_to_state (bevent->button);

        if (bevent->button == 1)
          {
            if (! shell->pointer_grabbed || shell->scrolling)
              return TRUE;

            if (! shell->space_pressed && ! shell->space_release_pending)
              picman_display_shell_keyboard_ungrab (shell, event);

            if (active_tool &&
                (! picman_image_is_empty (image) ||
                 picman_tool_control_get_handle_empty_image (active_tool->control)))
              {
                picman_motion_buffer_end_stroke (shell->motion_buffer);

                if (picman_tool_control_is_active (active_tool->control))
                  {
                    tool_manager_button_release_active (picman,
                                                        &image_coords,
                                                        time, state,
                                                        display);
                  }
              }

            /*  update the tool's modifier state because it didn't get
             *  key events while BUTTON1 was down
             */
            if (gtk_widget_has_focus (canvas))
              picman_display_shell_update_focus (shell, TRUE,
                                               &image_coords, state);
            else
              picman_display_shell_update_focus (shell, FALSE,
                                               &image_coords, 0);

            picman_display_shell_pointer_ungrab (shell, NULL);

            if (shell->space_release_pending)
              picman_display_shell_space_released (shell, event, &image_coords);
          }
        else if (bevent->button == 2)
          {
            if (shell->scrolling)
              picman_display_shell_stop_scrolling (shell, NULL);
          }
        else if (bevent->button == 3)
          {
            /* nop */
          }
        else
          {
            GdkEventButton *bevent = (GdkEventButton *) event;
            PicmanController *mouse;

            mouse = picman_controllers_get_mouse (picman);

            if (!(shell->scrolling || shell->pointer_grabbed) &&
                mouse && picman_controller_mouse_button (PICMAN_CONTROLLER_MOUSE (mouse),
                                                       bevent))
              return TRUE;
          }

        return_val = TRUE;
      }
      break;

    case GDK_SCROLL:
      {
        GdkEventScroll     *sevent = (GdkEventScroll *) event;
        PicmanController     *wheel;

        wheel = picman_controllers_get_wheel (picman);

        if (! wheel ||
            ! picman_controller_wheel_scroll (PICMAN_CONTROLLER_WHEEL (wheel),
                                            sevent))
          {
            GdkScrollDirection  direction = sevent->direction;

            if (state & picman_get_toggle_behavior_mask ())
              {
                switch (direction)
                  {
                  case GDK_SCROLL_UP:
                    picman_display_shell_scale (shell,
                                              PICMAN_ZOOM_IN,
                                              0.0,
                                              PICMAN_ZOOM_FOCUS_BEST_GUESS);
                    break;

                  case GDK_SCROLL_DOWN:
                    picman_display_shell_scale (shell,
                                              PICMAN_ZOOM_OUT,
                                              0.0,
                                              PICMAN_ZOOM_FOCUS_BEST_GUESS);
                    break;

                  default:
                    break;
                  }
              }
            else
              {
                GtkAdjustment *adj = NULL;
                gdouble        value;

                if (state & GDK_SHIFT_MASK)
                  switch (direction)
                    {
                    case GDK_SCROLL_UP:    direction = GDK_SCROLL_LEFT;  break;
                    case GDK_SCROLL_DOWN:  direction = GDK_SCROLL_RIGHT; break;
                    case GDK_SCROLL_LEFT:  direction = GDK_SCROLL_UP;    break;
                    case GDK_SCROLL_RIGHT: direction = GDK_SCROLL_DOWN;  break;
                    }

                switch (direction)
                  {
                  case GDK_SCROLL_LEFT:
                  case GDK_SCROLL_RIGHT:
                    adj = shell->hsbdata;
                    break;

                  case GDK_SCROLL_UP:
                  case GDK_SCROLL_DOWN:
                    adj = shell->vsbdata;
                    break;
                  }

                value = (gtk_adjustment_get_value (adj) +
                         ((direction == GDK_SCROLL_UP ||
                           direction == GDK_SCROLL_LEFT) ?
                          -gtk_adjustment_get_page_increment (adj) / 2 :
                          gtk_adjustment_get_page_increment (adj) / 2));
                value = CLAMP (value,
                               gtk_adjustment_get_lower (adj),
                               gtk_adjustment_get_upper (adj) -
                               gtk_adjustment_get_page_size (adj));

                gtk_adjustment_set_value (adj, value);
              }
          }

        picman_display_shell_untransform_event_coords (shell,
                                                     &display_coords,
                                                     &image_coords,
                                                     &update_sw_cursor);

        tool_manager_oper_update_active (picman,
                                         &image_coords, state,
                                         shell->proximity,
                                         display);

        return_val = TRUE;
      }
      break;

    case GDK_MOTION_NOTIFY:
      {
        GdkEventMotion *mevent            = (GdkEventMotion *) event;
        GdkEvent       *compressed_motion = NULL;
        PicmanMotionMode  motion_mode       = PICMAN_MOTION_MODE_EXACT;
        PicmanTool       *active_tool;

        if (picman->busy)
          return TRUE;

        active_tool = tool_manager_get_active (picman);

        if (active_tool)
          motion_mode = picman_tool_control_get_motion_mode (active_tool->control);

        if (shell->scrolling ||
            motion_mode == PICMAN_MOTION_MODE_COMPRESS)
          {
            compressed_motion = picman_display_shell_compress_motion (shell);

            if (compressed_motion && ! shell->scrolling)
              {
                picman_display_shell_get_event_coords (shell,
                                                     compressed_motion,
                                                     &display_coords,
                                                     &state, &time);
                picman_display_shell_untransform_event_coords (shell,
                                                             &display_coords,
                                                             &image_coords,
                                                             NULL);
              }
          }

        /*  call proximity_in() here because the pointer might already
         *  be in proximity when the canvas starts to receive events,
         *  like when a new image has been created into an empty
         *  display
         */
        picman_display_shell_proximity_in (shell);
        update_sw_cursor = TRUE;

        if (shell->scrolling)
          {
            const gint x = (compressed_motion
                            ? ((GdkEventMotion *) compressed_motion)->x
                            : mevent->x);
            const gint y = (compressed_motion
                            ? ((GdkEventMotion *) compressed_motion)->y
                            : mevent->y);

            if (shell->rotating)
              {
                gboolean constrain = (state & GDK_CONTROL_MASK) ? TRUE : FALSE;

                picman_display_shell_rotate_drag (shell,
                                                shell->scroll_last_x,
                                                shell->scroll_last_y,
                                                x,
                                                y,
                                                constrain);
              }
            else
              {
                picman_display_shell_scroll (shell,
                                           shell->scroll_last_x - x,
                                           shell->scroll_last_y - y);

              }

            shell->scroll_last_x = x;
            shell->scroll_last_y = y;
          }
        else if (state & GDK_BUTTON1_MASK)
          {
            if (active_tool                                        &&
                picman_tool_control_is_active (active_tool->control) &&
                (! picman_image_is_empty (image) ||
                 picman_tool_control_get_handle_empty_image (active_tool->control)))
              {
                GdkTimeCoord **history_events;
                gint           n_history_events;
                guint32        last_motion_time;

                /*  if the first mouse button is down, check for automatic
                 *  scrolling...
                 */
                if ((mevent->x < 0                 ||
                     mevent->y < 0                 ||
                     mevent->x > shell->disp_width ||
                     mevent->y > shell->disp_height) &&
                    ! picman_tool_control_get_scroll_lock (active_tool->control))
                  {
                    picman_display_shell_autoscroll_start (shell, state, mevent);
                  }

                /* gdk_device_get_history() has several quirks. First
                 * is that events with borderline timestamps at both
                 * ends are included. Because of that we need to add 1
                 * to lower border. The second is due to poor X event
                 * resolution. We need to do -1 to ensure that the
                 * amount of events between timestamps is final or
                 * risk losing some.
                 */
                last_motion_time =
                  picman_motion_buffer_get_last_motion_time (shell->motion_buffer);

                if (motion_mode == PICMAN_MOTION_MODE_EXACT     &&
                    shell->display->config->use_event_history &&
                    gdk_device_get_history (mevent->device, mevent->window,
                                            last_motion_time + 1,
                                            mevent->time - 1,
                                            &history_events,
                                            &n_history_events))
                  {
                    PicmanDeviceInfo *device;
                    gint            i;

                    device = picman_device_info_get_by_device (mevent->device);

                    for (i = 0; i < n_history_events; i++)
                      {
                        picman_device_info_get_time_coords (device,
                                                          history_events[i],
                                                          &display_coords);

                        picman_display_shell_untransform_event_coords (shell,
                                                                     &display_coords,
                                                                     &image_coords,
                                                                     NULL);

                        /* Early removal of useless events saves CPU time.
                         */
                        if (picman_motion_buffer_motion_event (shell->motion_buffer,
                                                             &image_coords,
                                                             history_events[i]->time,
                                                             shell->scale_x,
                                                             shell->scale_y,
                                                             TRUE))
                          {
                            picman_motion_buffer_request_stroke (shell->motion_buffer,
                                                               state,
                                                               history_events[i]->time);
                          }
                      }

                    gdk_device_free_history (history_events, n_history_events);
                  }
                else
                  {
                    gboolean event_fill = (motion_mode == PICMAN_MOTION_MODE_EXACT);

                    /* Early removal of useless events saves CPU time.
                     */
                    if (picman_motion_buffer_motion_event (shell->motion_buffer,
                                                         &image_coords,
                                                         time,
                                                         shell->scale_x,
                                                         shell->scale_y,
                                                         event_fill))
                      {
                        picman_motion_buffer_request_stroke (shell->motion_buffer,
                                                           state,
                                                           time);
                      }
                  }
              }
          }

        if (! (state &
               (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)))
          {
            /* Early removal of useless events saves CPU time.
             * Pass event_fill = FALSE since we are only hovering.
             */
            if (picman_motion_buffer_motion_event (shell->motion_buffer,
                                                 &image_coords,
                                                 time,
                                                 shell->scale_x,
                                                 shell->scale_y,
                                                 FALSE))
              {
                picman_motion_buffer_request_hover (shell->motion_buffer,
                                                  state,
                                                  shell->proximity);
              }
          }

        if (compressed_motion)
          gdk_event_free (compressed_motion);

        return_val = TRUE;
      }
      break;

    case GDK_KEY_PRESS:
      {
        GdkEventKey *kevent = (GdkEventKey *) event;
        PicmanTool    *active_tool;

        active_tool = tool_manager_get_active (picman);

        if (state & GDK_BUTTON1_MASK)
          {
            switch (kevent->keyval)
              {
              case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
              case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
              case GDK_KEY_Control_L: case GDK_KEY_Control_R:
              case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
                {
                  GdkModifierType key;

                  key = picman_display_shell_key_to_state (kevent->keyval);
                  state |= key;

                  if (active_tool                                        &&
                      picman_tool_control_is_active (active_tool->control) &&
                      ! picman_image_is_empty (image))
                    {
                      tool_manager_active_modifier_state_active (picman, state,
                                                                 display);
                    }
                }
                break;
              }
          }
        else
          {
            gboolean arrow_key = FALSE;

            tool_manager_focus_display_active (picman, display);

            if (picman_tool_control_get_wants_all_key_events (active_tool->control))
              {
                if (tool_manager_key_press_active (picman, kevent, display))
                  {
                    /* FIXME: need to do some of the stuff below, like
                     * calling oper_update()
                     */

                    return TRUE;
                  }
              }

            switch (kevent->keyval)
              {
              case GDK_KEY_Left:
              case GDK_KEY_Right:
              case GDK_KEY_Up:
              case GDK_KEY_Down:
                arrow_key = TRUE;

              case GDK_KEY_Return:
              case GDK_KEY_KP_Enter:
              case GDK_KEY_ISO_Enter:
              case GDK_KEY_BackSpace:
              case GDK_KEY_Escape:
                if (! picman_image_is_empty (image))
                  return_val = tool_manager_key_press_active (picman,
                                                              kevent,
                                                              display);

                if (! return_val)
                  {
                    PicmanController *keyboard = picman_controllers_get_keyboard (picman);

                    if (keyboard)
                      return_val =
                        picman_controller_keyboard_key_press (PICMAN_CONTROLLER_KEYBOARD (keyboard),
                                                            kevent);
                  }

                /* always swallow arrow keys, we don't want focus keynav */
                if (! return_val)
                  return_val = arrow_key;
                break;

              case GDK_KEY_space:
              case GDK_KEY_KP_Space:
                picman_display_shell_space_pressed (shell, event);
                return_val = TRUE;
                break;

              case GDK_KEY_Tab:
              case GDK_KEY_ISO_Left_Tab:
                picman_display_shell_tab_pressed (shell, kevent);
                return_val = TRUE;
                break;

                /*  Update the state based on modifiers being pressed  */
              case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
              case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
              case GDK_KEY_Control_L: case GDK_KEY_Control_R:
              case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
                {
                  GdkModifierType key;

                  key = picman_display_shell_key_to_state (kevent->keyval);
                  state |= key;

                  if (! picman_image_is_empty (image))
                    tool_manager_modifier_state_active (picman, state, display);
                }
                break;
              }

            tool_manager_oper_update_active (picman,
                                             &image_coords, state,
                                             shell->proximity,
                                             display);
          }
      }
      break;

    case GDK_KEY_RELEASE:
      {
        GdkEventKey *kevent = (GdkEventKey *) event;
        PicmanTool    *active_tool;

        active_tool = tool_manager_get_active (picman);

        if (state & GDK_BUTTON1_MASK)
          {
            switch (kevent->keyval)
              {
              case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
              case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
              case GDK_KEY_Control_L: case GDK_KEY_Control_R:
              case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
                {
                  GdkModifierType key;

                  key = picman_display_shell_key_to_state (kevent->keyval);
                  state &= ~key;

                  if (active_tool                                        &&
                      picman_tool_control_is_active (active_tool->control) &&
                      ! picman_image_is_empty (image))
                    {
                      tool_manager_active_modifier_state_active (picman, state,
                                                                 display);
                    }
                }
                break;
              }
          }
        else
          {
            tool_manager_focus_display_active (picman, display);

            if (picman_tool_control_get_wants_all_key_events (active_tool->control))
              {
                if (tool_manager_key_release_active (picman, kevent, display))
                  {
                    /* FIXME: need to do some of the stuff below, like
                     * calling oper_update()
                     */

                    return TRUE;
                  }
              }

            switch (kevent->keyval)
              {
              case GDK_KEY_space:
              case GDK_KEY_KP_Space:
                picman_display_shell_space_released (shell, event, NULL);
                return_val = TRUE;
                break;

                /*  Update the state based on modifiers being pressed  */
              case GDK_KEY_Alt_L:     case GDK_KEY_Alt_R:
              case GDK_KEY_Shift_L:   case GDK_KEY_Shift_R:
              case GDK_KEY_Control_L: case GDK_KEY_Control_R:
              case GDK_KEY_Meta_L:    case GDK_KEY_Meta_R:
                {
                  GdkModifierType key;

                  key = picman_display_shell_key_to_state (kevent->keyval);
                  state &= ~key;

                  /*  For all modifier keys: call the tools
                   *  modifier_state *and* oper_update method so tools
                   *  can choose if they are interested in the press
                   *  itself or only in the resulting state
                   */
                  if (! picman_image_is_empty (image))
                    tool_manager_modifier_state_active (picman, state, display);
                }
                break;
              }

            tool_manager_oper_update_active (picman,
                                             &image_coords, state,
                                             shell->proximity,
                                             display);
          }
      }
      break;

    default:
      break;
    }

  /*  if we reached this point in picman_busy mode, return now  */
  if (picman->busy)
    return return_val;

  /*  cursor update   */
  picman_display_shell_update_cursor (shell, &display_coords, &image_coords,
                                    state, update_sw_cursor);

  return return_val;
}

void
picman_display_shell_buffer_stroke (PicmanMotionBuffer *buffer,
                                  const PicmanCoords *coords,
                                  guint32           time,
                                  GdkModifierType   state,
                                  PicmanDisplayShell *shell)
{
  PicmanDisplay *display = shell->display;
  Picman        *picman    = picman_display_get_picman (display);
  PicmanTool    *active_tool;

  active_tool = tool_manager_get_active (picman);

  if (active_tool &&
      picman_tool_control_is_active (active_tool->control))
    {
      tool_manager_motion_active (picman,
                                  coords, time, state,
                                  display);
    }
}

void
picman_display_shell_buffer_hover (PicmanMotionBuffer *buffer,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 gboolean          proximity,
                                 PicmanDisplayShell *shell)
{
  PicmanDisplay *display = shell->display;
  Picman        *picman    = picman_display_get_picman (display);
  PicmanTool    *active_tool;

  active_tool = tool_manager_get_active (picman);

  if (active_tool &&
      ! picman_tool_control_is_active (active_tool->control))
    {
      tool_manager_oper_update_active (picman,
                                       coords, state, proximity,
                                       display);
    }
}

static gboolean
picman_display_shell_ruler_button_press (GtkWidget        *widget,
                                       GdkEventButton   *event,
                                       PicmanDisplayShell *shell,
                                       gboolean          horizontal)
{
  PicmanDisplay *display = shell->display;

  if (display->picman->busy)
    return TRUE;

  if (! picman_display_get_image (display))
    return TRUE;

  if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
      PicmanTool *active_tool;
      gboolean  sample_point;

      active_tool  = tool_manager_get_active (display->picman);
      sample_point = (event->state & picman_get_toggle_behavior_mask ());

      if (! ((sample_point && (PICMAN_IS_COLOR_TOOL (active_tool) &&
                               ! PICMAN_IS_IMAGE_MAP_TOOL (active_tool) &&
                               ! (PICMAN_IS_PAINT_TOOL (active_tool) &&
                                  ! PICMAN_PAINT_TOOL (active_tool)->pick_colors)))

             ||

             (! sample_point && PICMAN_IS_MOVE_TOOL (active_tool))))
        {
          PicmanToolInfo *tool_info;

          tool_info = picman_get_tool_info (display->picman,
                                          sample_point ?
                                          "picman-color-picker-tool" :
                                          "picman-move-tool");

          if (tool_info)
            picman_context_set_tool (picman_get_user_context (display->picman),
                                   tool_info);
        }

      active_tool = tool_manager_get_active (display->picman);

      if (active_tool)
        {
          picman_display_shell_update_focus (shell, TRUE,
                                           NULL, event->state);

          if (picman_display_shell_pointer_grab (shell, NULL, 0))
            {
              if (picman_display_shell_keyboard_grab (shell,
                                                    (GdkEvent *) event))
                {
                  if (sample_point)
                    picman_color_tool_start_sample_point (active_tool, display);
                  else if (horizontal)
                    picman_move_tool_start_hguide (active_tool, display);
                  else
                    picman_move_tool_start_vguide (active_tool, display);

                  return TRUE;
                }
              else
                {
                  picman_display_shell_pointer_ungrab (shell, NULL);
                }
            }
        }
    }

  return FALSE;
}

gboolean
picman_display_shell_hruler_button_press (GtkWidget        *widget,
                                        GdkEventButton   *event,
                                        PicmanDisplayShell *shell)
{
  return picman_display_shell_ruler_button_press (widget, event, shell, TRUE);
}

gboolean
picman_display_shell_vruler_button_press (GtkWidget        *widget,
                                        GdkEventButton   *event,
                                        PicmanDisplayShell *shell)
{
  return picman_display_shell_ruler_button_press (widget, event, shell, FALSE);
}


/*  private functions  */

static GdkModifierType
picman_display_shell_key_to_state (gint key)
{
  /* FIXME: need some proper GDK API to figure this */

  switch (key)
    {
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
      return GDK_MOD1_MASK;
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
      return GDK_SHIFT_MASK;
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
      return GDK_CONTROL_MASK;
#ifdef GDK_WINDOWING_QUARTZ
    case GDK_KEY_Meta_L:
    case GDK_KEY_Meta_R:
      return GDK_MOD2_MASK;
#endif
    default:
      return 0;
    }
}

static GdkModifierType
picman_display_shell_button_to_state (gint button)
{
  if (button == 1)
    return GDK_BUTTON1_MASK;
  else if (button == 2)
    return GDK_BUTTON2_MASK;
  else if (button == 3)
    return GDK_BUTTON3_MASK;

  return 0;
}

static void
picman_display_shell_proximity_in (PicmanDisplayShell *shell)
{
  if (! shell->proximity)
    {
      shell->proximity = TRUE;

      picman_display_shell_check_device_cursor (shell);
    }
}

static void
picman_display_shell_proximity_out (PicmanDisplayShell *shell)
{
  if (shell->proximity)
    {
      shell->proximity = FALSE;

      picman_display_shell_clear_software_cursor (shell);
    }
}

static void
picman_display_shell_check_device_cursor (PicmanDisplayShell *shell)
{
  PicmanDeviceManager *manager;
  PicmanDeviceInfo    *current_device;

  manager = picman_devices_get_manager (shell->display->picman);

  current_device = picman_device_manager_get_current_device (manager);

  shell->draw_cursor = ! picman_device_info_has_cursor (current_device);
}

static void
picman_display_shell_start_scrolling (PicmanDisplayShell *shell,
                                    const GdkEvent   *event,
                                    GdkModifierType   state,
                                    gint              x,
                                    gint              y)
{
  g_return_if_fail (! shell->scrolling);

  picman_display_shell_pointer_grab (shell, event, GDK_POINTER_MOTION_MASK);

  shell->scrolling         = TRUE;
  shell->scroll_last_x     = x;
  shell->scroll_last_y     = y;
  shell->rotating          = (state & GDK_SHIFT_MASK) ? TRUE : FALSE;
  shell->rotate_drag_angle = shell->rotate_angle;

  if (shell->rotating)
    picman_display_shell_set_override_cursor (shell, GDK_EXCHANGE);
  else
    picman_display_shell_set_override_cursor (shell, GDK_FLEUR);
}

static void
picman_display_shell_stop_scrolling (PicmanDisplayShell *shell,
                                   const GdkEvent   *event)
{
  g_return_if_fail (shell->scrolling);

  picman_display_shell_unset_override_cursor (shell);

  shell->scrolling         = FALSE;
  shell->scroll_last_x     = 0;
  shell->scroll_last_y     = 0;
  shell->rotating          = FALSE;
  shell->rotate_drag_angle = 0.0;

  picman_display_shell_pointer_ungrab (shell, event);
}

static void
picman_display_shell_space_pressed (PicmanDisplayShell *shell,
                                  const GdkEvent   *event)
{
  Picman *picman = picman_display_get_picman (shell->display);

  if (shell->space_pressed)
    return;

  if (! picman_display_shell_keyboard_grab (shell, event))
    return;

  switch (shell->display->config->space_bar_action)
    {
    case PICMAN_SPACE_BAR_ACTION_NONE:
      break;

    case PICMAN_SPACE_BAR_ACTION_PAN:
      {
        PicmanDeviceManager *manager;
        PicmanDeviceInfo    *current_device;
        PicmanCoords         coords;
        GdkModifierType    state = 0;

        manager = picman_devices_get_manager (picman);
        current_device = picman_device_manager_get_current_device (manager);

        picman_device_info_get_device_coords (current_device,
                                            gtk_widget_get_window (shell->canvas),
                                            &coords);
        gdk_event_get_state (event, &state);

        picman_display_shell_start_scrolling (shell, event, state,
                                            coords.x, coords.y);
      }
      break;

    case PICMAN_SPACE_BAR_ACTION_MOVE:
      {
        PicmanTool *active_tool = tool_manager_get_active (picman);

        if (active_tool || ! PICMAN_IS_MOVE_TOOL (active_tool))
          {
            GdkModifierType state;

            shell->space_shaded_tool =
              picman_object_get_name (active_tool->tool_info);

            picman_context_set_tool (picman_get_user_context (picman),
                                   picman_get_tool_info (picman, "picman-move-tool"));

            gdk_event_get_state (event, &state);

            picman_display_shell_update_focus (shell, TRUE,
                                             NULL, state);
          }
      }
      break;
    }

  shell->space_pressed = TRUE;
}

static void
picman_display_shell_space_released (PicmanDisplayShell *shell,
                                   const GdkEvent   *event,
                                   const PicmanCoords *image_coords)
{
  Picman *picman = picman_display_get_picman (shell->display);

  if (! shell->space_pressed && ! shell->space_release_pending)
    return;

  switch (shell->display->config->space_bar_action)
    {
    case PICMAN_SPACE_BAR_ACTION_NONE:
      break;

    case PICMAN_SPACE_BAR_ACTION_PAN:
      picman_display_shell_stop_scrolling (shell, event);
      break;

    case PICMAN_SPACE_BAR_ACTION_MOVE:
      if (shell->space_shaded_tool)
        {
          picman_context_set_tool (picman_get_user_context (picman),
                                 picman_get_tool_info (picman,
                                                     shell->space_shaded_tool));
          shell->space_shaded_tool = NULL;

          if (gtk_widget_has_focus (shell->canvas))
            {
              GdkModifierType state;

              gdk_event_get_state (event, &state);

              picman_display_shell_update_focus (shell, TRUE,
                                               image_coords, state);
            }
          else
            {
              picman_display_shell_update_focus (shell, FALSE,
                                               image_coords, 0);
            }
        }
      break;
    }

  picman_display_shell_keyboard_ungrab (shell, event);

  shell->space_pressed         = FALSE;
  shell->space_release_pending = FALSE;
}

static gboolean
picman_display_shell_tab_pressed (PicmanDisplayShell  *shell,
                                const GdkEventKey *kevent)
{
  PicmanImageWindow *window  = picman_display_shell_get_window (shell);
  PicmanUIManager   *manager = picman_image_window_get_ui_manager (window);
  PicmanImage       *image   = picman_display_get_image (shell->display);

  if (kevent->state & GDK_CONTROL_MASK)
    {
      if (image && ! picman_image_is_empty (image))
        {
          if (kevent->keyval == GDK_KEY_Tab)
            picman_display_shell_layer_select_init (shell,
                                                  1, kevent->time);
          else
            picman_display_shell_layer_select_init (shell,
                                                  -1, kevent->time);

          return TRUE;
        }
    }
  else if (kevent->state & GDK_MOD1_MASK)
    {
      if (image)
        {
          if (kevent->keyval == GDK_KEY_Tab)
            picman_ui_manager_activate_action (manager, "windows",
                                             "windows-show-display-next");
          else
            picman_ui_manager_activate_action (manager, "windows",
                                             "windows-show-display-previous");

          return TRUE;
        }
    }
  else
    {
      picman_ui_manager_activate_action (manager, "windows",
                                       "windows-hide-docks");

      return TRUE;
    }

  return FALSE;
}

static void
picman_display_shell_update_focus (PicmanDisplayShell *shell,
                                 gboolean          focus_in,
                                 const PicmanCoords *image_coords,
                                 GdkModifierType   state)
{
  Picman *picman = picman_display_get_picman (shell->display);

  if (focus_in)
    {
      tool_manager_focus_display_active (picman, shell->display);
      tool_manager_modifier_state_active (picman, state, shell->display);
    }
  else
    {
      tool_manager_focus_display_active (picman, NULL);
    }

  if (image_coords)
    tool_manager_oper_update_active (picman,
                                     image_coords, state,
                                     shell->proximity,
                                     shell->display);
}

static void
picman_display_shell_update_cursor (PicmanDisplayShell *shell,
                                  const PicmanCoords *display_coords,
                                  const PicmanCoords *image_coords,
                                  GdkModifierType   state,
                                  gboolean          update_software_cursor)
{
  PicmanDisplay *display = shell->display;
  Picman        *picman    = picman_display_get_picman (display);
  PicmanImage   *image   = picman_display_get_image (display);
  PicmanTool    *active_tool;

  if (! shell->display->config->cursor_updating)
    return;

  active_tool = tool_manager_get_active (picman);

  if (active_tool)
    {
      if ((! picman_image_is_empty (image) ||
           picman_tool_control_get_handle_empty_image (active_tool->control)) &&
          ! (state & (GDK_BUTTON1_MASK |
                      GDK_BUTTON2_MASK |
                      GDK_BUTTON3_MASK)))
        {
          tool_manager_cursor_update_active (picman,
                                             image_coords, state,
                                             display);
        }
      else if (picman_image_is_empty (image) &&
               ! picman_tool_control_get_handle_empty_image (active_tool->control))
        {
          picman_display_shell_set_cursor (shell,
                                         PICMAN_CURSOR_MOUSE,
                                         picman_tool_control_get_tool_cursor (active_tool->control),
                                         PICMAN_CURSOR_MODIFIER_BAD);
        }
    }
  else
    {
      picman_display_shell_set_cursor (shell,
                                     PICMAN_CURSOR_MOUSE,
                                     PICMAN_TOOL_CURSOR_NONE,
                                     PICMAN_CURSOR_MODIFIER_BAD);
    }

  if (update_software_cursor)
    {
      PicmanCursorPrecision precision = PICMAN_CURSOR_PRECISION_PIXEL_CENTER;

      if (active_tool)
        precision = picman_tool_control_get_precision (active_tool->control);

      picman_display_shell_update_software_cursor (shell,
                                                 precision,
                                                 (gint) display_coords->x,
                                                 (gint) display_coords->y,
                                                 image_coords->x,
                                                 image_coords->y);
    }
}

static gboolean
picman_display_shell_initialize_tool (PicmanDisplayShell *shell,
                                    const PicmanCoords *image_coords,
                                    GdkModifierType   state)
{
  PicmanDisplay *display     = shell->display;
  PicmanImage   *image       = picman_display_get_image (display);
  Picman        *picman        = picman_display_get_picman (display);
  gboolean     initialized = FALSE;
  PicmanTool    *active_tool;

  active_tool = tool_manager_get_active (picman);

  if (active_tool &&
      (! picman_image_is_empty (image) ||
       picman_tool_control_get_handle_empty_image (active_tool->control)))
    {
      initialized = TRUE;

      /*  initialize the current tool if it has no drawable  */
      if (! active_tool->drawable)
        {
          initialized = tool_manager_initialize_active (picman, display);
        }
      else if ((active_tool->drawable !=
                picman_image_get_active_drawable (image)) &&
               (! picman_tool_control_get_preserve (active_tool->control) &&
                (picman_tool_control_get_dirty_mask (active_tool->control) &
                 PICMAN_DIRTY_ACTIVE_DRAWABLE)))
        {
          /*  create a new one, deleting the current  */
          picman_context_tool_changed (picman_get_user_context (picman));

          /*  make sure the newly created tool has the right state  */
          picman_display_shell_update_focus (shell, TRUE, image_coords, state);

          initialized = tool_manager_initialize_active (picman, display);
        }
    }

  return initialized;
}

static void
picman_display_shell_get_event_coords (PicmanDisplayShell *shell,
                                     const GdkEvent   *event,
                                     PicmanCoords       *display_coords,
                                     GdkModifierType  *state,
                                     guint32          *time)
{
  Picman              *picman = picman_display_get_picman (shell->display);
  PicmanDeviceManager *manager;
  PicmanDeviceInfo    *current_device;

  manager = picman_devices_get_manager (picman);
  current_device = picman_device_manager_get_current_device (manager);

  picman_device_info_get_event_coords (current_device,
                                     gtk_widget_get_window (shell->canvas),
                                     event,
                                     display_coords);

  picman_device_info_get_event_state (current_device,
                                    gtk_widget_get_window (shell->canvas),
                                    event,
                                    state);

  *time = gdk_event_get_time (event);
}

static void
picman_display_shell_untransform_event_coords (PicmanDisplayShell *shell,
                                             const PicmanCoords *display_coords,
                                             PicmanCoords       *image_coords,
                                             gboolean         *update_software_cursor)
{
  Picman     *picman = picman_display_get_picman (shell->display);
  PicmanTool *active_tool;

  /*  PicmanCoords passed to tools are ALWAYS in image coordinates  */
  picman_display_shell_untransform_coords (shell,
                                         display_coords,
                                         image_coords);

  active_tool = tool_manager_get_active (picman);

  if (active_tool && picman_tool_control_get_snap_to (active_tool->control))
    {
      gint x, y, width, height;

      picman_tool_control_get_snap_offsets (active_tool->control,
                                          &x, &y, &width, &height);

      if (picman_display_shell_snap_coords (shell,
                                          image_coords,
                                          x, y, width, height))
        {
          if (update_software_cursor)
            *update_software_cursor = TRUE;
        }
    }
}

/* picman_display_shell_compress_motion:
 *
 * This function walks the whole GDK event queue seeking motion events
 * corresponding to the widget 'widget'.  If it finds any it will
 * remove them from the queue, and return the most recent motion event.
 * Otherwise it will return NULL.
 *
 * The picman_display_shell_compress_motion function source may be re-used under
 * the XFree86-style license. <adam@picman.org>
 */
static GdkEvent *
picman_display_shell_compress_motion (PicmanDisplayShell *shell)
{
  GList       *requeued_events = NULL;
  const GList *list;
  GdkEvent    *last_motion = NULL;

  /*  Move the entire GDK event queue to a private list, filtering
   *  out any motion events for the desired widget.
   */
  while (gdk_events_pending ())
    {
      GdkEvent *event = gdk_event_get ();

      if (!event)
        {
          /* Do nothing */
        }
      else if ((gtk_get_event_widget (event) == shell->canvas) &&
               (event->any.type == GDK_MOTION_NOTIFY))
        {
          if (last_motion)
            gdk_event_free (last_motion);

          last_motion = event;
        }
      else if ((gtk_get_event_widget (event) == shell->canvas) &&
               (event->any.type == GDK_BUTTON_RELEASE))
        {
          requeued_events = g_list_prepend (requeued_events, event);

          while (gdk_events_pending ())
            if ((event = gdk_event_get ()))
              requeued_events = g_list_prepend (requeued_events, event);

          break;
        }
      else
        {
          requeued_events = g_list_prepend (requeued_events, event);
        }
    }

  /* Replay the remains of our private event list back into the
   * event queue in order.
   */
  requeued_events = g_list_reverse (requeued_events);

  for (list = requeued_events; list; list = g_list_next (list))
    {
      GdkEvent *event = list->data;

      gdk_event_put (event);
      gdk_event_free (event);
    }

  g_list_free (requeued_events);

  return last_motion;
}
