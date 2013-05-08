/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Measure tool
 * Copyright (C) 1999-2003 Sven Neumann <sven@picman.org>
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

#include <stdlib.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-utils.h"
#include "core/picmanimage.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmancanvashandle.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmantooldialog.h"

#include "picmanmeasureoptions.h"
#include "picmanmeasuretool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define ARC_RADIUS 30


/*  local function prototypes  */

static void     picman_measure_tool_control         (PicmanTool              *tool,
                                                   PicmanToolAction         action,
                                                   PicmanDisplay           *display);
static void     picman_measure_tool_button_press    (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanButtonPressType    press_type,
                                                   PicmanDisplay           *display);
static void     picman_measure_tool_button_release  (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanButtonReleaseType  release_type,
                                                   PicmanDisplay           *display);
static void     picman_measure_tool_motion          (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);
static gboolean picman_measure_tool_key_press       (PicmanTool              *tool,
                                                   GdkEventKey           *kevent,
                                                   PicmanDisplay           *display);
static void picman_measure_tool_active_modifier_key (PicmanTool              *tool,
                                                   GdkModifierType        key,
                                                   gboolean               press,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);
static void     picman_measure_tool_oper_update     (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   GdkModifierType        state,
                                                   gboolean               proximity,
                                                   PicmanDisplay           *display);
static void     picman_measure_tool_cursor_update   (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);

static void     picman_measure_tool_draw            (PicmanDrawTool          *draw_tool);

static gdouble  picman_measure_tool_get_angle       (gint                   dx,
                                                   gint                   dy,
                                                   gdouble                xres,
                                                   gdouble                yres);

static GtkWidget * picman_measure_tool_dialog_new   (PicmanMeasureTool       *measure);
static void     picman_measure_tool_dialog_update   (PicmanMeasureTool       *measure,
                                                   PicmanDisplay           *display);


G_DEFINE_TYPE (PicmanMeasureTool, picman_measure_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_measure_tool_parent_class


void
picman_measure_tool_register (PicmanToolRegisterCallback  callback,
                            gpointer                  data)
{
  (* callback) (PICMAN_TYPE_MEASURE_TOOL,
                PICMAN_TYPE_MEASURE_OPTIONS,
                picman_measure_options_gui,
                0,
                "picman-measure-tool",
                _("Measure"),
                _("Measure Tool: Measure distances and angles"),
                N_("_Measure"), "<shift>M",
                NULL, PICMAN_HELP_TOOL_MEASURE,
                PICMAN_STOCK_TOOL_MEASURE,
                data);
}

static void
picman_measure_tool_class_init (PicmanMeasureToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->control             = picman_measure_tool_control;
  tool_class->button_press        = picman_measure_tool_button_press;
  tool_class->button_release      = picman_measure_tool_button_release;
  tool_class->motion              = picman_measure_tool_motion;
  tool_class->key_press           = picman_measure_tool_key_press;
  tool_class->active_modifier_key = picman_measure_tool_active_modifier_key;
  tool_class->oper_update         = picman_measure_tool_oper_update;
  tool_class->cursor_update       = picman_measure_tool_cursor_update;

  draw_tool_class->draw           = picman_measure_tool_draw;
}

static void
picman_measure_tool_init (PicmanMeasureTool *measure)
{
  PicmanTool *tool = PICMAN_TOOL (measure);

  picman_tool_control_set_handle_empty_image (tool->control, TRUE);
  picman_tool_control_set_precision          (tool->control,
                                            PICMAN_CURSOR_PRECISION_PIXEL_BORDER);
  picman_tool_control_set_tool_cursor        (tool->control,
                                            PICMAN_TOOL_CURSOR_MEASURE);

  measure->function    = CREATING;
  measure->point       = -1;
  measure->status_help = TRUE;
}

static void
picman_measure_tool_control (PicmanTool       *tool,
                           PicmanToolAction  action,
                           PicmanDisplay    *display)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      if (measure->dialog)
        gtk_widget_destroy (measure->dialog);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_measure_tool_button_press (PicmanTool            *tool,
                                const PicmanCoords    *coords,
                                guint32              time,
                                GdkModifierType      state,
                                PicmanButtonPressType  press_type,
                                PicmanDisplay         *display)
{
  PicmanMeasureTool    *measure = PICMAN_MEASURE_TOOL (tool);
  PicmanMeasureOptions *options = PICMAN_MEASURE_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell   *shell   = picman_display_get_shell (display);
  PicmanImage          *image   = picman_display_get_image (display);

  /*  if we are changing displays, pop the statusbar of the old one  */
  if (display != tool->display)
    {
      if (tool->display)
        picman_tool_pop_status (tool, tool->display);
    }

  measure->function = CREATING;

  measure->mouse_x = coords->x;
  measure->mouse_y = coords->y;

  if (display == tool->display)
    {
      /*  if the cursor is in one of the handles, the new function
       *  will be moving or adding a new point or guide
       */
      if (measure->point != -1)
        {
          GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

          if (state & (toggle_mask | GDK_MOD1_MASK))
            {
              gboolean create_hguide;
              gboolean create_vguide;

              create_hguide = ((state & toggle_mask) &&
                               (measure->y[measure->point] ==
                                CLAMP (measure->y[measure->point],
                                       0,
                                       picman_image_get_height (image))));

              create_vguide = ((state & GDK_MOD1_MASK) &&
                               (measure->x[measure->point] ==
                                CLAMP (measure->x[measure->point],
                                       0,
                                       picman_image_get_width (image))));

              if (create_hguide || create_vguide)
                {
                  if (create_hguide && create_vguide)
                    picman_image_undo_group_start (image,
                                                 PICMAN_UNDO_GROUP_GUIDE,
                                                 _("Add Guides"));

                  if (create_hguide)
                    picman_image_add_hguide (image, measure->y[measure->point],
                                           TRUE);

                  if (create_vguide)
                    picman_image_add_vguide (image, measure->x[measure->point],
                                           TRUE);

                  if (create_hguide && create_vguide)
                    picman_image_undo_group_end (image);

                  picman_image_flush (image);
                }

              measure->function = GUIDING;
            }
          else
            {
              if (state & GDK_SHIFT_MASK)
                measure->function = ADDING;
              else
                measure->function = MOVING;
            }
        }

      /*  adding to the middle point makes no sense  */
      if (measure->point      == 0      &&
          measure->function   == ADDING &&
          measure->num_points == 3)
        {
          measure->function = MOVING;
        }

      /*  if the function is still CREATING, we are outside the handles  */
      if (measure->function == CREATING)
        {
          if (measure->num_points > 1 && (state & GDK_MOD1_MASK))
            {
              measure->function = MOVING_ALL;

              measure->last_x = coords->x;
              measure->last_y = coords->y;
            }
        }
    }

  if (measure->function == CREATING)
    {
      if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (measure)))
        picman_draw_tool_stop (PICMAN_DRAW_TOOL (measure));

      measure->x[0] = measure->x[1] = measure->x[2] = 0.0;
      measure->y[0] = measure->y[1] = measure->y[2] = 0.0;

      /*  set the first point and go into ADDING mode  */
      measure->x[0]       = coords->x + 0.5;
      measure->y[0]       = coords->y + 0.5;
      measure->point      = 0;
      measure->num_points = 1;
      measure->function   = ADDING;

      /*  set the displaylay  */
      tool->display = display;

      picman_tool_replace_status (tool, display, _("Drag to create a line"));

      picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
    }

  picman_tool_control_activate (tool->control);

  /*  create the info window if necessary  */
  if (! measure->dialog)
    {
      if (options->use_info_window ||
          ! picman_display_shell_get_show_statusbar (shell))
        {
          measure->dialog = picman_measure_tool_dialog_new (measure);
          g_object_add_weak_pointer (G_OBJECT (measure->dialog),
                                     (gpointer) &measure->dialog);
        }
    }

  if (measure->dialog)
    {
      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (measure->dialog),
                                         PICMAN_VIEWABLE (image),
                                         PICMAN_CONTEXT (options));
      picman_tool_dialog_set_shell (PICMAN_TOOL_DIALOG (measure->dialog), shell);

      picman_measure_tool_dialog_update (measure, display);
    }
}

static void
picman_measure_tool_button_release (PicmanTool              *tool,
                                  const PicmanCoords      *coords,
                                  guint32                time,
                                  GdkModifierType        state,
                                  PicmanButtonReleaseType  release_type,
                                  PicmanDisplay           *display)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (tool);

  measure->function = FINISHED;

  picman_tool_control_halt (tool->control);
}

static void
picman_measure_tool_motion (PicmanTool         *tool,
                          const PicmanCoords *coords,
                          guint32           time,
                          GdkModifierType   state,
                          PicmanDisplay      *display)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (tool);
  gint             dx, dy;
  gint             i;
  gint             tmp;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (measure));

  measure->mouse_x = coords->x;
  measure->mouse_y = coords->y;

  /*  A few comments here, because this routine looks quite weird at first ...
   *
   *  The goal is to keep point 0, called the start point, to be
   *  always the one in the middle or, if there are only two points,
   *  the one that is fixed.  The angle is then always measured at
   *  this point.
   */

  switch (measure->function)
    {
    case ADDING:
      switch (measure->point)
        {
        case 0:
          /*  we are adding to the start point  */
          break;

        case 1:
          /*  we are adding to the end point, make it the new start point  */
          tmp = measure->x[0];
          measure->x[0] = measure->x[1];
          measure->x[1] = tmp;

          tmp = measure->y[0];
          measure->y[0] = measure->y[1];
          measure->y[1] = tmp;
          break;

        case 2:
          /*  we are adding to the third point, make it the new start point  */
          measure->x[1] = measure->x[0];
          measure->y[1] = measure->y[0];
          measure->x[0] = measure->x[2];
          measure->y[0] = measure->y[2];
          break;

        default:
          break;
        }

      measure->num_points = MIN (measure->num_points + 1, 3);
      measure->point      = measure->num_points - 1;
      measure->function   = MOVING;
      /*  don't break here!  */

    case MOVING:
      /*  if we are moving the start point and only have two, make it
       *  the end point
       */
      if (measure->num_points == 2 && measure->point == 0)
        {
          tmp = measure->x[0];
          measure->x[0] = measure->x[1];
          measure->x[1] = tmp;

          tmp = measure->y[0];
          measure->y[0] = measure->y[1];
          measure->y[1] = tmp;

          measure->point = 1;
        }

      measure->x[measure->point] = ROUND (coords->x);
      measure->y[measure->point] = ROUND (coords->y);

      if (state & picman_get_constrain_behavior_mask ())
        {
          gdouble  x = measure->x[measure->point];
          gdouble  y = measure->y[measure->point];

          picman_constrain_line (measure->x[0], measure->y[0],
                               &x, &y,
                               PICMAN_CONSTRAIN_LINE_15_DEGREES);

          measure->x[measure->point] = ROUND (x);
          measure->y[measure->point] = ROUND (y);
        }
      break;

    case MOVING_ALL:
      dx = ROUND (coords->x) - measure->last_x;
      dy = ROUND (coords->y) - measure->last_y;

      for (i = 0; i < measure->num_points; i++)
        {
          measure->x[i] += dx;
          measure->y[i] += dy;
        }

      measure->last_x = ROUND (coords->x);
      measure->last_y = ROUND (coords->y);
      break;

    default:
      break;
    }

  if (measure->function == MOVING)
    picman_measure_tool_dialog_update (measure, display);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (measure));
}

static gboolean
picman_measure_tool_key_press (PicmanTool    *tool,
                             GdkEventKey *kevent,
                             PicmanDisplay *display)
{
  if (display == tool->display)
    {
      switch (kevent->keyval)
        {
        case GDK_KEY_Escape:
          picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
          return TRUE;

        default:
          break;
        }
    }

  return FALSE;
}

static void
picman_measure_tool_active_modifier_key (PicmanTool        *tool,
                                       GdkModifierType  key,
                                       gboolean         press,
                                       GdkModifierType  state,
                                       PicmanDisplay     *display)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (tool);

  if (key == picman_get_constrain_behavior_mask () &&
      measure->function == MOVING)
    {
      gdouble x, y;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      x = measure->mouse_x;
      y = measure->mouse_y;

      if (press)
        picman_constrain_line (measure->x[0], measure->y[0],
                             &x, &y,
                             PICMAN_CONSTRAIN_LINE_15_DEGREES);

      measure->x[measure->point] = ROUND (x);
      measure->y[measure->point] = ROUND (y);

      picman_measure_tool_dialog_update (measure, display);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
}

static void
picman_measure_tool_oper_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               gboolean          proximity,
                               PicmanDisplay      *display)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (tool);
  gchar           *status  = NULL;
  gint             i;

  if (tool->display == display)
    {
      gint point = -1;

      for (i = 0; i < measure->num_points; i++)
        {
          if (picman_canvas_item_hit (measure->handles[i],
                                    coords->x, coords->y))
            {
              GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

              point = i;

              if (state & toggle_mask)
                {
                  if (state & GDK_MOD1_MASK)
                    {
                      status = picman_suggest_modifiers (_("Click to place "
                                                         "vertical and "
                                                         "horizontal guides"),
                                                       0,
                                                       NULL, NULL, NULL);
                    }
                  else
                    {
                      status = picman_suggest_modifiers (_("Click to place a "
                                                         "horizontal guide"),
                                                       GDK_MOD1_MASK & ~state,
                                                       NULL, NULL, NULL);
                    }

                  picman_tool_replace_status (tool, display, "%s", status);
                  g_free (status);
                  measure->status_help = TRUE;
                  break;
                }

              if (state & GDK_MOD1_MASK)
                {
                  status = picman_suggest_modifiers (_("Click to place a "
                                                     "vertical guide"),
                                                   toggle_mask & ~state,
                                                   NULL, NULL, NULL);
                  picman_tool_replace_status (tool, display, "%s", status);
                  g_free (status);
                  measure->status_help = TRUE;
                  break;
                }

              if ((state & GDK_SHIFT_MASK)
                  && ! ((i == 0) && (measure->num_points == 3)))
                {
                  status = picman_suggest_modifiers (_("Click-Drag to add a "
                                                     "new point"),
                                                   (toggle_mask |
                                                    GDK_MOD1_MASK) & ~state,
                                                   NULL, NULL, NULL);
                }
              else
                {
                  if ((i == 0) && (measure->num_points == 3))
                    state |= GDK_SHIFT_MASK;
                  status = picman_suggest_modifiers (_("Click-Drag to move this "
                                                     "point"),
                                                   (GDK_SHIFT_MASK |
                                                    toggle_mask    |
                                                    GDK_MOD1_MASK) & ~state,
                                                   NULL, NULL, NULL);
                }

              picman_tool_replace_status (tool, display, "%s", status);
              g_free (status);
              measure->status_help = TRUE;
              break;
            }
        }

      if (point == -1)
        {
          if ((measure->num_points > 1) && (state & GDK_MOD1_MASK))
            {
              picman_tool_replace_status (tool, display, _("Click-Drag to move "
                                                         "all points"));
              measure->status_help = TRUE;
            }
          else if (measure->status_help)
            {
              if (measure->num_points > 1)
                {
                  /* replace status bar hint by distance and angle */
                  picman_measure_tool_dialog_update (measure, display);
                }
              else
                {
                  picman_tool_replace_status (tool, display, " ");
                }
            }
        }

      if (point != measure->point)
        {
          if (measure->point != -1 && measure->handles[measure->point])
            {
              picman_canvas_item_set_highlight (measure->handles[measure->point],
                                              FALSE);
            }

          measure->point = point;

          if (measure->point != -1 && measure->handles[measure->point])
            {
              picman_canvas_item_set_highlight (measure->handles[measure->point],
                                              TRUE);
            }
        }
    }
}

static void
picman_measure_tool_cursor_update (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 PicmanDisplay      *display)
{
  PicmanMeasureTool   *measure  = PICMAN_MEASURE_TOOL (tool);
  PicmanCursorType     cursor   = PICMAN_CURSOR_CROSSHAIR_SMALL;
  PicmanCursorModifier modifier = PICMAN_CURSOR_MODIFIER_NONE;

  if (tool->display == display)
    {
      if (measure->point != -1)
        {
          GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

          if (state & toggle_mask)
            {
              if (state & GDK_MOD1_MASK)
                cursor = PICMAN_CURSOR_CORNER_BOTTOM_RIGHT;
              else
                cursor = PICMAN_CURSOR_SIDE_BOTTOM;
            }
          else if (state & GDK_MOD1_MASK)
            {
              cursor = PICMAN_CURSOR_SIDE_RIGHT;
            }
          else if ((state & GDK_SHIFT_MASK) &&
                   ! ((measure->point == 0) &&
                      (measure->num_points == 3)))
            {
              modifier = PICMAN_CURSOR_MODIFIER_PLUS;
            }
          else
            {
              modifier = PICMAN_CURSOR_MODIFIER_MOVE;
            }
        }
      else
        {
          if ((measure->num_points > 1) && (state & GDK_MOD1_MASK))
            {
              modifier = PICMAN_CURSOR_MODIFIER_MOVE;
            }
        }
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_measure_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanMeasureTool *measure = PICMAN_MEASURE_TOOL (draw_tool);
  PicmanTool        *tool    = PICMAN_TOOL (draw_tool);
  PicmanCanvasGroup *stroke_group;
  gint             i;
  gint             draw_arc = 0;

  for (i = 0; i < 3; i++)
    measure->handles[i] = 0;

  stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

  for (i = 0; i < measure->num_points; i++)
    {
      if (i == 0 && measure->num_points == 3)
        {
          measure->handles[i] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_CIRCLE,
                                       measure->x[i],
                                       measure->y[i],
                                       PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                       PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                       PICMAN_HANDLE_ANCHOR_CENTER);
        }
      else
        {
          measure->handles[i] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_CROSS,
                                       measure->x[i],
                                       measure->y[i],
                                       PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                       PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                       PICMAN_HANDLE_ANCHOR_CENTER);
        }

      if (i > 0)
        {
          picman_draw_tool_push_group (draw_tool, stroke_group);

          picman_draw_tool_add_line (draw_tool,
                                   measure->x[0],
                                   measure->y[0],
                                   measure->x[i],
                                   measure->y[i]);

          picman_draw_tool_pop_group (draw_tool);

          /*  only draw the arc if the lines are long enough  */
          if (picman_draw_tool_calc_distance (draw_tool, tool->display,
                                            measure->x[0],
                                            measure->y[0],
                                            measure->x[i],
                                            measure->y[i]) > ARC_RADIUS)
            {
              draw_arc++;
            }
        }
    }

  if (measure->point != -1 && measure->handles[measure->point])
    {
      picman_canvas_item_set_highlight (measure->handles[measure->point],
                                      TRUE);
    }

  if (measure->num_points > 1 && draw_arc == measure->num_points - 1)
    {
      gdouble angle1 = measure->angle2 / 180.0 * G_PI;
      gdouble angle2 = (measure->angle1 - measure->angle2) / 180.0 * G_PI;

      if (angle2 > G_PI)
        angle2 -= 2.0 * G_PI;

      if (angle2 < -G_PI)
        angle2 += 2.0 * G_PI;

      if (angle2 != 0.0)
        {
          PicmanCanvasItem *item;

          picman_draw_tool_push_group (draw_tool, stroke_group);

          item = picman_draw_tool_add_handle (draw_tool,
                                            PICMAN_HANDLE_CIRCLE,
                                            measure->x[0],
                                            measure->y[0],
                                            ARC_RADIUS * 2 + 1,
                                            ARC_RADIUS * 2 + 1,
                                            PICMAN_HANDLE_ANCHOR_CENTER);

          picman_canvas_handle_set_angles (item, angle1, angle2);

          if (measure->num_points == 2)
            {
              PicmanDisplayShell *shell;
              gdouble           target;
              gdouble           arc_radius;

              shell = picman_display_get_shell (tool->display);

              target     = FUNSCALEX (shell, (PICMAN_TOOL_HANDLE_SIZE_CROSS >> 1));
              arc_radius = FUNSCALEX (shell, ARC_RADIUS);

              picman_draw_tool_add_line (draw_tool,
                                       measure->x[0],
                                       measure->y[0],
                                       (measure->x[1] >= measure->x[0] ?
                                        measure->x[0] + arc_radius + target :
                                        measure->x[0] - arc_radius - target),
                                       measure->y[0]);
            }

          picman_draw_tool_pop_group (draw_tool);
        }
    }
}

static gdouble
picman_measure_tool_get_angle (gint    dx,
                             gint    dy,
                             gdouble xres,
                             gdouble yres)
{
  gdouble angle;

  if (dx)
    angle = picman_rad_to_deg (atan (((gdouble) (dy) / yres) /
                                   ((gdouble) (dx) / xres)));
  else if (dy)
    angle = dy > 0 ? 270.0 : 90.0;
  else
    angle = 180.0;

  if (dx > 0)
    {
      if (dy > 0)
        angle = 360.0 - angle;
      else
        angle = -angle;
    }
  else
    {
      angle = 180.0 - angle;
    }

  return angle;
}

static void
picman_measure_tool_dialog_update (PicmanMeasureTool *measure,
                                 PicmanDisplay     *display)
{
  PicmanDisplayShell *shell = picman_display_get_shell (display);
  PicmanImage        *image = picman_display_get_image (display);
  gint              ax, ay;
  gint              bx, by;
  gint              pixel_width;
  gint              pixel_height;
  gdouble           unit_width;
  gdouble           unit_height;
  gdouble           pixel_distance;
  gdouble           unit_distance;
  gdouble           theta1, theta2;
  gdouble           pixel_angle;
  gdouble           unit_angle;
  gdouble           xres;
  gdouble           yres;
  gchar             format[128];

  /*  calculate distance and angle  */
  ax = measure->x[1] - measure->x[0];
  ay = measure->y[1] - measure->y[0];

  if (measure->num_points == 3)
    {
      bx = measure->x[2] - measure->x[0];
      by = measure->y[2] - measure->y[0];
    }
  else
    {
      bx = 0;
      by = 0;
    }

  pixel_width  = ABS (ax - bx);
  pixel_height = ABS (ay - by);

  picman_image_get_resolution (image, &xres, &yres);

  unit_width  = picman_pixels_to_units (pixel_width,  shell->unit, xres);
  unit_height = picman_pixels_to_units (pixel_height, shell->unit, yres);

  pixel_distance = sqrt (SQR (ax - bx) + SQR (ay - by));
  unit_distance  = (picman_unit_get_factor (shell->unit) *
                    sqrt (SQR ((gdouble) (ax - bx) / xres) +
                          SQR ((gdouble) (ay - by) / yres)));

  if (measure->num_points != 3)
    bx = ax > 0 ? 1 : -1;

  theta1 = picman_measure_tool_get_angle (ax, ay, 1.0, 1.0);
  theta2 = picman_measure_tool_get_angle (bx, by, 1.0, 1.0);

  pixel_angle = fabs (theta1 - theta2);
  if (pixel_angle > 180.0)
    pixel_angle = fabs (360.0 - pixel_angle);

  theta1 = picman_measure_tool_get_angle (ax, ay, xres, yres);
  theta2 = picman_measure_tool_get_angle (bx, by, xres, yres);

  measure->angle1 = theta1;
  measure->angle2 = theta2;

  unit_angle = fabs (theta1 - theta2);
  if (unit_angle > 180.0)
    unit_angle = fabs (360.0 - unit_angle);

  if (shell->unit == PICMAN_UNIT_PIXEL)
    {
      picman_tool_replace_status (PICMAN_TOOL (measure), display,
                                "%.1f %s, %.2f\302\260 (%d × %d)",
                                pixel_distance, _("pixels"), pixel_angle,
                                pixel_width, pixel_height);
    }
  else
    {
      g_snprintf (format, sizeof (format),
                  "%%.%df %s, %%.2f\302\260 (%%.%df × %%.%df)",
                  picman_unit_get_digits (shell->unit),
                  picman_unit_get_plural (shell->unit),
                  picman_unit_get_digits (shell->unit),
                  picman_unit_get_digits (shell->unit));

      picman_tool_replace_status (PICMAN_TOOL (measure), display, format,
                                unit_distance, unit_angle,
                                unit_width, unit_height);
    }
  measure->status_help = FALSE;

  if (measure->dialog)
    {
      gchar buf[128];

      g_snprintf (format, sizeof (format), "%%.%df",
                  picman_unit_get_digits (shell->unit));

      /* Distance */
      g_snprintf (buf, sizeof (buf), "%.1f", pixel_distance);
      gtk_label_set_text (GTK_LABEL (measure->distance_label[0]), buf);

      if (shell->unit != PICMAN_UNIT_PIXEL)
        {
          g_snprintf (buf, sizeof (buf), format, unit_distance);
          gtk_label_set_text (GTK_LABEL (measure->distance_label[1]), buf);

          gtk_label_set_text (GTK_LABEL (measure->unit_label[0]),
                              picman_unit_get_plural (shell->unit));
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (measure->distance_label[1]), NULL);
          gtk_label_set_text (GTK_LABEL (measure->unit_label[0]), NULL);
        }

      /* Angle */
      g_snprintf (buf, sizeof (buf), "%.2f", pixel_angle);
      gtk_label_set_text (GTK_LABEL (measure->angle_label[0]), buf);

      if (fabs (unit_angle - pixel_angle) > 0.01)
        {
          g_snprintf (buf, sizeof (buf), "%.2f", unit_angle);
          gtk_label_set_text (GTK_LABEL (measure->angle_label[1]), buf);

          gtk_label_set_text (GTK_LABEL (measure->unit_label[1]), "\302\260");
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (measure->angle_label[1]), NULL);
          gtk_label_set_text (GTK_LABEL (measure->unit_label[1]), NULL);
        }

      /* Width */
      g_snprintf (buf, sizeof (buf), "%d", pixel_width);
      gtk_label_set_text (GTK_LABEL (measure->width_label[0]), buf);

      if (shell->unit != PICMAN_UNIT_PIXEL)
        {
          g_snprintf (buf, sizeof (buf), format, unit_width);
          gtk_label_set_text (GTK_LABEL (measure->width_label[1]), buf);

          gtk_label_set_text (GTK_LABEL (measure->unit_label[2]),
                              picman_unit_get_plural (shell->unit));
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (measure->width_label[1]), NULL);
          gtk_label_set_text (GTK_LABEL (measure->unit_label[2]), NULL);
        }

      g_snprintf (buf, sizeof (buf), "%d", pixel_height);
      gtk_label_set_text (GTK_LABEL (measure->height_label[0]), buf);

      /* Height */
      if (shell->unit != PICMAN_UNIT_PIXEL)
        {
          g_snprintf (buf, sizeof (buf), format, unit_height);
          gtk_label_set_text (GTK_LABEL (measure->height_label[1]), buf);

          gtk_label_set_text (GTK_LABEL (measure->unit_label[3]),
                              picman_unit_get_plural (shell->unit));
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (measure->height_label[1]), NULL);
          gtk_label_set_text (GTK_LABEL (measure->unit_label[3]), NULL);
        }

      if (gtk_widget_get_visible (measure->dialog))
        gdk_window_show (gtk_widget_get_window (measure->dialog));
      else
        gtk_widget_show (measure->dialog);
    }
}

static GtkWidget *
picman_measure_tool_dialog_new (PicmanMeasureTool *measure)
{
  PicmanTool  *tool = PICMAN_TOOL (measure);
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *label;

  dialog = picman_tool_dialog_new (tool->tool_info,
                                 picman_display_get_shell (tool->display),
                                 _("Measure Distances and Angles"),

                                 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,

                                 NULL);

  gtk_window_set_focus_on_map (GTK_WINDOW (dialog), FALSE);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  table = gtk_table_new (4, 5, TRUE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_set_border_width (GTK_CONTAINER (table), 6);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      table, TRUE, TRUE, 0);
  gtk_widget_show (table);


  label = gtk_label_new (_("Distance:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  gtk_widget_show (label);

  measure->distance_label[0] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 0, 1);
  gtk_widget_show (label);

  label = gtk_label_new (_("pixels"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 0, 1);
  gtk_widget_show (label);

  measure->distance_label[1] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 3, 4, 0, 1);
  gtk_widget_show (label);

  measure->unit_label[0] = label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 4, 5, 0, 1);
  gtk_widget_show (label);


  label = gtk_label_new (_("Angle:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  gtk_widget_show (label);

  measure->angle_label[0] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 1, 2);
  gtk_widget_show (label);

  label = gtk_label_new ("\302\260");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 1, 2);
  gtk_widget_show (label);

  measure->angle_label[1] = label = gtk_label_new (NULL);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 3, 4, 1, 2);
  gtk_widget_show (label);

  measure->unit_label[1] = label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 4, 5, 1, 2);
  gtk_widget_show (label);


  label = gtk_label_new (_("Width:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  gtk_widget_show (label);

  measure->width_label[0] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 2, 3);
  gtk_widget_show (label);

  label = gtk_label_new (_("pixels"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 2, 3);
  gtk_widget_show (label);

  measure->width_label[1] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 3, 4, 2, 3);
  gtk_widget_show (label);

  measure->unit_label[2] = label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 4, 5, 2, 3);
  gtk_widget_show (label);


  label = gtk_label_new (_("Height:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);
  gtk_widget_show (label);

  measure->height_label[0] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 3, 4);
  gtk_widget_show (label);

  label = gtk_label_new (_("pixels"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 3, 4);
  gtk_widget_show (label);

  measure->height_label[1] = label = gtk_label_new ("0.0");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 3, 4, 3, 4);
  gtk_widget_show (label);

  measure->unit_label[3] = label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, 4, 5, 3, 4);
  gtk_widget_show (label);

  return dialog;
}
