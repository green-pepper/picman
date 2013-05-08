/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "tools-types.h"

#include "core/picmandrawable.h"
#include "core/picmanimage.h"

#include "vectors/picmananchor.h"
#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "display/picmancanvas.h"
#include "display/picmancanvasarc.h"
#include "display/picmancanvasboundary.h"
#include "display/picmancanvascorner.h"
#include "display/picmancanvasgroup.h"
#include "display/picmancanvasguide.h"
#include "display/picmancanvashandle.h"
#include "display/picmancanvasitem-utils.h"
#include "display/picmancanvasline.h"
#include "display/picmancanvaspath.h"
#include "display/picmancanvaspen.h"
#include "display/picmancanvaspolygon.h"
#include "display/picmancanvasrectangle.h"
#include "display/picmancanvasrectangleguides.h"
#include "display/picmancanvassamplepoint.h"
#include "display/picmancanvastextcursor.h"
#include "display/picmancanvastransformguides.h"
#include "display/picmancanvastransformpreview.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-items.h"
#include "display/picmandisplayshell-transform.h"

#include "picmandrawtool.h"


#define DRAW_TIMEOUT              4
#define USE_TIMEOUT               1
#define MINIMUM_DRAW_INTERVAL 50000 /* 50000 microseconds == 20 fps */


static void          picman_draw_tool_dispose      (GObject          *object);

static gboolean      picman_draw_tool_has_display  (PicmanTool         *tool,
                                                  PicmanDisplay      *display);
static PicmanDisplay * picman_draw_tool_has_image    (PicmanTool         *tool,
                                                  PicmanImage        *image);
static void          picman_draw_tool_control      (PicmanTool         *tool,
                                                  PicmanToolAction    action,
                                                  PicmanDisplay      *display);

static void          picman_draw_tool_draw         (PicmanDrawTool     *draw_tool);
static void          picman_draw_tool_undraw       (PicmanDrawTool     *draw_tool);
static void          picman_draw_tool_real_draw    (PicmanDrawTool     *draw_tool);


G_DEFINE_TYPE (PicmanDrawTool, picman_draw_tool, PICMAN_TYPE_TOOL)

#define parent_class picman_draw_tool_parent_class


static void
picman_draw_tool_class_init (PicmanDrawToolClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanToolClass *tool_class   = PICMAN_TOOL_CLASS (klass);

  object_class->dispose   = picman_draw_tool_dispose;

  tool_class->has_display = picman_draw_tool_has_display;
  tool_class->has_image   = picman_draw_tool_has_image;
  tool_class->control     = picman_draw_tool_control;

  klass->draw             = picman_draw_tool_real_draw;
}

static void
picman_draw_tool_init (PicmanDrawTool *draw_tool)
{
  draw_tool->display      = NULL;
  draw_tool->paused_count = 0;
  draw_tool->preview      = NULL;
  draw_tool->item         = NULL;
}

static void
picman_draw_tool_dispose (GObject *object)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (object);

  if (draw_tool->draw_timeout)
    {
      g_source_remove (draw_tool->draw_timeout);
      draw_tool->draw_timeout = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
picman_draw_tool_has_display (PicmanTool    *tool,
                            PicmanDisplay *display)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

  return (display == draw_tool->display ||
          PICMAN_TOOL_CLASS (parent_class)->has_display (tool, display));
}

static PicmanDisplay *
picman_draw_tool_has_image (PicmanTool  *tool,
                          PicmanImage *image)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);
  PicmanDisplay  *display;

  display = PICMAN_TOOL_CLASS (parent_class)->has_image (tool, image);

  if (! display && draw_tool->display)
    {
      if (image && picman_display_get_image (draw_tool->display) == image)
        display = draw_tool->display;

      /*  NULL image means any display  */
      if (! image)
        display = draw_tool->display;
    }

  return display;
}

static void
picman_draw_tool_control (PicmanTool       *tool,
                        PicmanToolAction  action,
                        PicmanDisplay    *display)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      if (picman_draw_tool_is_active (draw_tool))
        picman_draw_tool_stop (draw_tool);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

#ifdef USE_TIMEOUT
static gboolean
picman_draw_tool_draw_timeout (PicmanDrawTool *draw_tool)
{
  guint64 now = g_get_monotonic_time ();

  /* keep the timeout running if the last drawing just happened */
  if ((now - draw_tool->last_draw_time) <= MINIMUM_DRAW_INTERVAL)
    return FALSE;

  draw_tool->draw_timeout = 0;

  picman_draw_tool_draw (draw_tool);

  return FALSE;
}
#endif

static void
picman_draw_tool_draw (PicmanDrawTool *draw_tool)
{
  guint64 now = g_get_monotonic_time ();

  if (draw_tool->display &&
      draw_tool->paused_count == 0 &&
      (! draw_tool->draw_timeout ||
       (now - draw_tool->last_draw_time) > MINIMUM_DRAW_INTERVAL))
    {
      PicmanDisplayShell *shell = picman_display_get_shell (draw_tool->display);

      if (draw_tool->draw_timeout)
        {
          g_source_remove (draw_tool->draw_timeout);
          draw_tool->draw_timeout = 0;
        }

      picman_draw_tool_undraw (draw_tool);

      PICMAN_DRAW_TOOL_GET_CLASS (draw_tool)->draw (draw_tool);

      if (draw_tool->group_stack)
        {
          g_warning ("%s: draw_tool->group_stack not empty after calling "
                     "PicmanDrawTool::draw() of %s",
                     G_STRFUNC,
                     g_type_name (G_TYPE_FROM_INSTANCE (draw_tool)));

          while (draw_tool->group_stack)
            picman_draw_tool_pop_group (draw_tool);
        }

      if (draw_tool->preview)
        picman_display_shell_add_preview_item (shell, draw_tool->preview);

      if (draw_tool->item)
        picman_display_shell_add_tool_item (shell, draw_tool->item);

      draw_tool->last_draw_time = now;
    }
}

static void
picman_draw_tool_undraw (PicmanDrawTool *draw_tool)
{
  if (draw_tool->display)
    {
      PicmanDisplayShell *shell = picman_display_get_shell (draw_tool->display);

      if (draw_tool->preview)
        {
          picman_display_shell_remove_preview_item (shell, draw_tool->preview);
          g_object_unref (draw_tool->preview);
          draw_tool->preview = NULL;
        }

      if (draw_tool->item)
        {
          picman_display_shell_remove_tool_item (shell, draw_tool->item);
          g_object_unref (draw_tool->item);
          draw_tool->item = NULL;
        }
    }
}

static void
picman_draw_tool_real_draw (PicmanDrawTool *draw_tool)
{
  /* the default implementation does nothing */
}

void
picman_draw_tool_start (PicmanDrawTool *draw_tool,
                      PicmanDisplay  *display)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));
  g_return_if_fail (picman_draw_tool_is_active (draw_tool) == FALSE);

  draw_tool->display = display;

  picman_draw_tool_draw (draw_tool);
}

void
picman_draw_tool_stop (PicmanDrawTool *draw_tool)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (picman_draw_tool_is_active (draw_tool) == TRUE);

  picman_draw_tool_undraw (draw_tool);

  if (draw_tool->draw_timeout)
    {
      g_source_remove (draw_tool->draw_timeout);
      draw_tool->draw_timeout = 0;
    }

  draw_tool->last_draw_time = 0;

  draw_tool->display = NULL;
}

gboolean
picman_draw_tool_is_active (PicmanDrawTool *draw_tool)
{
  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), FALSE);

  return draw_tool->display != NULL;
}

void
picman_draw_tool_pause (PicmanDrawTool *draw_tool)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));

  draw_tool->paused_count++;

  if (draw_tool->draw_timeout)
    {
      g_source_remove (draw_tool->draw_timeout);
      draw_tool->draw_timeout = 0;
    }
}

void
picman_draw_tool_resume (PicmanDrawTool *draw_tool)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (draw_tool->paused_count > 0);

  draw_tool->paused_count--;

  if (draw_tool->paused_count == 0)
    {
#ifdef USE_TIMEOUT
      /* Don't install the timeout if the draw tool isn't active, so
       * suspend()/resume() can always be called, and have no side
       * effect on an inactive tool. See bug #687851.
       */
      if (picman_draw_tool_is_active (draw_tool) && ! draw_tool->draw_timeout)
        {
          draw_tool->draw_timeout =
            gdk_threads_add_timeout_full (G_PRIORITY_HIGH_IDLE,
                                          DRAW_TIMEOUT,
                                          (GSourceFunc) picman_draw_tool_draw_timeout,
                                          draw_tool, NULL);
        }
#endif

      /* call draw() anyway, it will do nothing if the timeout is
       * running, but will additionally check the drawing times to
       * ensure the minimum framerate
       */
      picman_draw_tool_draw (draw_tool);
    }
}

/**
 * picman_draw_tool_calc_distance:
 * @draw_tool: a #PicmanDrawTool
 * @display:   a #PicmanDisplay
 * @x1:        start point X in image coordinates
 * @y1:        start point Y in image coordinates
 * @x2:        end point X in image coordinates
 * @y2:        end point Y in image coordinates
 *
 * If you just need to compare distances, consider to use
 * picman_draw_tool_calc_distance_square() instead.
 *
 * Returns: the distance between the given points in display coordinates
 **/
gdouble
picman_draw_tool_calc_distance (PicmanDrawTool *draw_tool,
                              PicmanDisplay  *display,
                              gdouble       x1,
                              gdouble       y1,
                              gdouble       x2,
                              gdouble       y2)
{
  return sqrt (picman_draw_tool_calc_distance_square (draw_tool, display,
                                                    x1, y1, x2, y2));
}

/**
 * picman_draw_tool_calc_distance_square:
 * @draw_tool: a #PicmanDrawTool
 * @display:   a #PicmanDisplay
 * @x1:        start point X in image coordinates
 * @y1:        start point Y in image coordinates
 * @x2:        end point X in image coordinates
 * @y2:        end point Y in image coordinates
 *
 * This function is more effective than picman_draw_tool_calc_distance()
 * as it doesn't perform a sqrt(). Use this if you just need to compare
 * distances.
 *
 * Returns: the square of the distance between the given points in
 *          display coordinates
 **/
gdouble
picman_draw_tool_calc_distance_square (PicmanDrawTool *draw_tool,
                                     PicmanDisplay  *display,
                                     gdouble       x1,
                                     gdouble       y1,
                                     gdouble       x2,
                                     gdouble       y2)
{
  PicmanDisplayShell *shell;
  gdouble           tx1, ty1;
  gdouble           tx2, ty2;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), 0.0);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), 0.0);

  shell = picman_display_get_shell (display);

  picman_display_shell_transform_xy_f (shell, x1, y1, &tx1, &ty1);
  picman_display_shell_transform_xy_f (shell, x2, y2, &tx2, &ty2);

  return SQR (tx2 - tx1) + SQR (ty2 - ty1);
}

void
picman_draw_tool_add_preview (PicmanDrawTool   *draw_tool,
                            PicmanCanvasItem *item)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  if (! draw_tool->preview)
    draw_tool->preview =
      picman_canvas_group_new (picman_display_get_shell (draw_tool->display));

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (draw_tool->preview), item);
}

void
picman_draw_tool_remove_preview (PicmanDrawTool   *draw_tool,
                               PicmanCanvasItem *item)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));
  g_return_if_fail (draw_tool->preview != NULL);

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (draw_tool->preview), item);
}

void
picman_draw_tool_add_item (PicmanDrawTool   *draw_tool,
                         PicmanCanvasItem *item)
{
  PicmanCanvasGroup *group;

  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  if (! draw_tool->item)
    draw_tool->item =
      picman_canvas_group_new (picman_display_get_shell (draw_tool->display));

  group = PICMAN_CANVAS_GROUP (draw_tool->item);

  if (draw_tool->group_stack)
    group = draw_tool->group_stack->data;

  picman_canvas_group_add_item (group, item);
}

void
picman_draw_tool_remove_item (PicmanDrawTool   *draw_tool,
                            PicmanCanvasItem *item)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));
  g_return_if_fail (draw_tool->item != NULL);

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (draw_tool->item), item);
}

PicmanCanvasGroup *
picman_draw_tool_add_stroke_group (PicmanDrawTool *draw_tool)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_group_new (picman_display_get_shell (draw_tool->display));
  picman_canvas_group_set_group_stroking (PICMAN_CANVAS_GROUP (item), TRUE);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return PICMAN_CANVAS_GROUP (item);
}

PicmanCanvasGroup *
picman_draw_tool_add_fill_group (PicmanDrawTool *draw_tool)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_group_new (picman_display_get_shell (draw_tool->display));
  picman_canvas_group_set_group_filling (PICMAN_CANVAS_GROUP (item), TRUE);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return PICMAN_CANVAS_GROUP (item);
}

void
picman_draw_tool_push_group (PicmanDrawTool    *draw_tool,
                           PicmanCanvasGroup *group)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));

  draw_tool->group_stack = g_list_prepend (draw_tool->group_stack, group);
}

void
picman_draw_tool_pop_group (PicmanDrawTool *draw_tool)
{
  g_return_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool));
  g_return_if_fail (draw_tool->group_stack != NULL);

  draw_tool->group_stack = g_list_remove (draw_tool->group_stack,
                                          draw_tool->group_stack->data);
}

/**
 * picman_draw_tool_add_line:
 * @draw_tool:   the #PicmanDrawTool
 * @x1:          start point X in image coordinates
 * @y1:          start point Y in image coordinates
 * @x2:          end point X in image coordinates
 * @y2:          end point Y in image coordinates
 *
 * This function takes image space coordinates and transforms them to
 * screen window coordinates, then draws a line between the resulting
 * coordindates.
 **/
PicmanCanvasItem *
picman_draw_tool_add_line (PicmanDrawTool *draw_tool,
                         gdouble       x1,
                         gdouble       y1,
                         gdouble       x2,
                         gdouble       y2)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_line_new (picman_display_get_shell (draw_tool->display),
                               x1, y1, x2, y2);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

/**
 * picman_draw_tool_add_guide:
 * @draw_tool:   the #PicmanDrawTool
 * @orientation: the orientation of the guide line
 * @position:    the position of the guide line in image coordinates
 *
 * This function draws a guide line across the canvas.
 **/
PicmanCanvasItem *
picman_draw_tool_add_guide (PicmanDrawTool        *draw_tool,
                          PicmanOrientationType  orientation,
                          gint                 position,
                          gboolean             guide_style)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_guide_new (picman_display_get_shell (draw_tool->display),
                                orientation, position, guide_style);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

/**
 * picman_draw_tool_add_crosshair:
 * @draw_tool:  the #PicmanDrawTool
 * @position_x: the position of the vertical guide line in image coordinates
 * @position_y: the position of the horizontal guide line in image coordinates
 *
 * This function draws two crossing guide lines across the canvas.
 **/
PicmanCanvasItem *
picman_draw_tool_add_crosshair (PicmanDrawTool *draw_tool,
                              gint          position_x,
                              gint          position_y)
{
  PicmanCanvasGroup *group;

  group = picman_draw_tool_add_stroke_group (draw_tool);

  picman_draw_tool_push_group (draw_tool, group);
  picman_draw_tool_add_guide (draw_tool,
                            PICMAN_ORIENTATION_VERTICAL, position_x, FALSE);
  picman_draw_tool_add_guide (draw_tool,
                            PICMAN_ORIENTATION_HORIZONTAL, position_y, FALSE);
  picman_draw_tool_pop_group (draw_tool);

  return PICMAN_CANVAS_ITEM (group);
}

/**
 * picman_draw_tool_add_sample_point:
 * @draw_tool: the #PicmanDrawTool
 * @x:         X position of the sample point
 * @y:         Y position of the sample point
 * @index:     Index of the sample point
 *
 * This function draws a sample point
 **/
PicmanCanvasItem *
picman_draw_tool_add_sample_point (PicmanDrawTool *draw_tool,
                                 gint          x,
                                 gint          y,
                                 gint          index)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_sample_point_new (picman_display_get_shell (draw_tool->display),
                                       x, y, index, TRUE);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

/**
 * picman_draw_tool_add_rectangle:
 * @draw_tool:   the #PicmanDrawTool
 * @filled:      whether to fill the rectangle
 * @x:           horizontal image coordinate
 * @y:           vertical image coordinate
 * @width:       width in image coordinates
 * @height:      height in image coordinates
 *
 * This function takes image space coordinates and transforms them to
 * screen window coordinates, then draws the resulting rectangle.
 **/
PicmanCanvasItem *
picman_draw_tool_add_rectangle (PicmanDrawTool *draw_tool,
                              gboolean      filled,
                              gdouble       x,
                              gdouble       y,
                              gdouble       width,
                              gdouble       height)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_rectangle_new (picman_display_get_shell (draw_tool->display),
                                    x, y, width, height, filled);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_rectangle_guides (PicmanDrawTool   *draw_tool,
                                     PicmanGuidesType  type,
                                     gdouble         x,
                                     gdouble         y,
                                     gdouble         width,
                                     gdouble         height)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_rectangle_guides_new (picman_display_get_shell (draw_tool->display),
                                           x, y, width, height, type, 4);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_arc (PicmanDrawTool *draw_tool,
                        gboolean      filled,
                        gdouble       x,
                        gdouble       y,
                        gdouble       width,
                        gdouble       height,
                        gdouble       start_angle,
                        gdouble       slice_angle)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_arc_new (picman_display_get_shell (draw_tool->display),
                              x + width  / 2.0,
                              y + height / 2.0,
                              width  / 2.0,
                              height / 2.0,
                              start_angle,
                              slice_angle,
                              filled);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_handle (PicmanDrawTool     *draw_tool,
                           PicmanHandleType    type,
                           gdouble           x,
                           gdouble           y,
                           gint              width,
                           gint              height,
                           PicmanHandleAnchor  anchor)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_handle_new (picman_display_get_shell (draw_tool->display),
                                 type, anchor, x, y, width, height);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

/**
 * picman_draw_tool_add_corner:
 * @draw_tool:   the #PicmanDrawTool
 * @highlight:
 * @put_outside: whether to put the handles on the outside of the rectangle
 * @x1:
 * @y1:
 * @x2:
 * @y2:
 * @width:       corner width
 * @height:      corner height
 * @anchor:      which corner to draw
 *
 * This function takes image space coordinates and transforms them to
 * screen window coordinates. It draws a corner into an already drawn
 * rectangle outline, taking care of not drawing over an already drawn line.
 **/
PicmanCanvasItem *
picman_draw_tool_add_corner (PicmanDrawTool     *draw_tool,
                           gboolean          highlight,
                           gboolean          put_outside,
                           gdouble           x1,
                           gdouble           y1,
                           gdouble           x2,
                           gdouble           y2,
                           gint              width,
                           gint              height,
                           PicmanHandleAnchor  anchor)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_corner_new (picman_display_get_shell (draw_tool->display),
                                 x1, y1, x2 - x1, y2 - y1,
                                 anchor, width, height, put_outside);
  picman_canvas_item_set_highlight (item, highlight);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_lines (PicmanDrawTool      *draw_tool,
                          const PicmanVector2 *points,
                          gint               n_points,
                          gboolean           filled)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  if (points == NULL || n_points < 2)
    return NULL;

  item = picman_canvas_polygon_new (picman_display_get_shell (draw_tool->display),
                                  points, n_points, filled);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_strokes (PicmanDrawTool     *draw_tool,
                            const PicmanCoords *points,
                            gint              n_points,
                            gboolean          filled)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  if (points == NULL || n_points < 2)
    return NULL;

  item = picman_canvas_polygon_new_from_coords (picman_display_get_shell (draw_tool->display),
                                              points, n_points, filled);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_path (PicmanDrawTool         *draw_tool,
                         const PicmanBezierDesc *desc,
                         gdouble               x,
                         gdouble               y)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);
  g_return_val_if_fail (desc != NULL, NULL);

  item = picman_canvas_path_new (picman_display_get_shell (draw_tool->display),
                               desc, x, y, FALSE, PICMAN_PATH_STYLE_DEFAULT);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_pen (PicmanDrawTool      *draw_tool,
                        const PicmanVector2 *points,
                        gint               n_points,
                        PicmanContext       *context,
                        PicmanActiveColor    color,
                        gint               width)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  if (points == NULL || n_points < 2)
    return NULL;

  item = picman_canvas_pen_new (picman_display_get_shell (draw_tool->display),
                              points, n_points, context, color, width);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

/**
 * picman_draw_tool_add_boundary:
 * @draw_tool:    a #PicmanDrawTool
 * @bound_segs:   the sorted brush outline
 * @n_bound_segs: the number of segments in @bound_segs
 * @matrix:       transform matrix for the boundary
 * @offset_x:     x offset
 * @offset_y:     y offset
 *
 * Draw the boundary of the brush that @draw_tool uses. The boundary
 * should be sorted with sort_boundary(), and @n_bound_segs should
 * include the sentinel segments inserted by sort_boundary() that
 * indicate the end of connected segment sequences (groups) .
 */
PicmanCanvasItem *
picman_draw_tool_add_boundary (PicmanDrawTool       *draw_tool,
                             const PicmanBoundSeg *bound_segs,
                             gint                n_bound_segs,
                             PicmanMatrix3        *transform,
                             gdouble             offset_x,
                             gdouble             offset_y)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);
  g_return_val_if_fail (n_bound_segs > 0, NULL);
  g_return_val_if_fail (bound_segs != NULL, NULL);

  item = picman_canvas_boundary_new (picman_display_get_shell (draw_tool->display),
                                   bound_segs, n_bound_segs,
                                   transform,
                                   offset_x, offset_y);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_text_cursor (PicmanDrawTool   *draw_tool,
                                PangoRectangle *cursor,
                                gboolean        overwrite)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);

  item = picman_canvas_text_cursor_new (picman_display_get_shell (draw_tool->display),
                                      cursor, overwrite);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_transform_guides (PicmanDrawTool      *draw_tool,
                                     const PicmanMatrix3 *transform,
                                     PicmanGuidesType     type,
                                     gint               n_guides,
                                     gdouble            x1,
                                     gdouble            y1,
                                     gdouble            x2,
                                     gdouble            y2)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);
  g_return_val_if_fail (transform != NULL, NULL);

  item = picman_canvas_transform_guides_new (picman_display_get_shell (draw_tool->display),
                                           transform, x1, y1, x2, y2,
                                           type, n_guides);

  picman_draw_tool_add_item (draw_tool, item);
  g_object_unref (item);

  return item;
}

PicmanCanvasItem *
picman_draw_tool_add_transform_preview (PicmanDrawTool      *draw_tool,
                                      PicmanDrawable      *drawable,
                                      const PicmanMatrix3 *transform,
                                      gdouble            x1,
                                      gdouble            y1,
                                      gdouble            x2,
                                      gdouble            y2,
                                      gboolean           perspective,
                                      gdouble            opacity)
{
  PicmanCanvasItem *item;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (transform != NULL, NULL);

  item = picman_canvas_transform_preview_new (picman_display_get_shell (draw_tool->display),
                                            drawable, transform,
                                            x1, y1, x2, y2,
                                            perspective, opacity);

  picman_draw_tool_add_preview (draw_tool, item);
  g_object_unref (item);

  return item;
}

gboolean
picman_draw_tool_on_handle (PicmanDrawTool     *draw_tool,
                          PicmanDisplay      *display,
                          gdouble           x,
                          gdouble           y,
                          PicmanHandleType    type,
                          gdouble           handle_x,
                          gdouble           handle_y,
                          gint              width,
                          gint              height,
                          PicmanHandleAnchor  anchor)
{
  PicmanDisplayShell *shell;
  gdouble           tx, ty;
  gdouble           handle_tx, handle_ty;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);

  shell = picman_display_get_shell (display);

  picman_display_shell_zoom_xy_f (shell,
                                x, y,
                                &tx, &ty);
  picman_display_shell_zoom_xy_f (shell,
                                handle_x, handle_y,
                                &handle_tx, &handle_ty);

  switch (type)
    {
    case PICMAN_HANDLE_SQUARE:
    case PICMAN_HANDLE_FILLED_SQUARE:
    case PICMAN_HANDLE_CROSS:
      picman_canvas_item_shift_to_north_west (anchor,
                                            handle_tx, handle_ty,
                                            width, height,
                                            &handle_tx, &handle_ty);

      return (tx == CLAMP (tx, handle_tx, handle_tx + width) &&
              ty == CLAMP (ty, handle_ty, handle_ty + height));

    case PICMAN_HANDLE_CIRCLE:
    case PICMAN_HANDLE_FILLED_CIRCLE:
      picman_canvas_item_shift_to_center (anchor,
                                        handle_tx, handle_ty,
                                        width, height,
                                        &handle_tx, &handle_ty);

      /* FIXME */
      if (width != height)
        width = (width + height) / 2;

      width /= 2;

      return ((SQR (handle_tx - tx) + SQR (handle_ty - ty)) < SQR (width));

    default:
      g_warning ("%s: invalid handle type %d", G_STRFUNC, type);
      break;
    }

  return FALSE;
}

gboolean
picman_draw_tool_on_vectors_handle (PicmanDrawTool      *draw_tool,
                                  PicmanDisplay       *display,
                                  PicmanVectors       *vectors,
                                  const PicmanCoords  *coord,
                                  gint               width,
                                  gint               height,
                                  PicmanAnchorType     preferred,
                                  gboolean           exclusive,
                                  PicmanAnchor       **ret_anchor,
                                  PicmanStroke       **ret_stroke)
{
  PicmanStroke *stroke       = NULL;
  PicmanStroke *pref_stroke  = NULL;
  PicmanAnchor *anchor       = NULL;
  PicmanAnchor *pref_anchor  = NULL;
  gdouble     dx, dy;
  gdouble     pref_mindist = -1;
  gdouble     mindist      = -1;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (coord != NULL, FALSE);

  if (ret_anchor) *ret_anchor = NULL;
  if (ret_stroke) *ret_stroke = NULL;

  while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
    {
      GList *anchor_list;
      GList *list;

      anchor_list = g_list_concat (picman_stroke_get_draw_anchors (stroke),
                                   picman_stroke_get_draw_controls (stroke));

      for (list = anchor_list; list; list = g_list_next (list))
        {
          dx = coord->x - PICMAN_ANCHOR (list->data)->position.x;
          dy = coord->y - PICMAN_ANCHOR (list->data)->position.y;

          if (mindist < 0 || mindist > dx * dx + dy * dy)
            {
              mindist = dx * dx + dy * dy;
              anchor = PICMAN_ANCHOR (list->data);

              if (ret_stroke)
                *ret_stroke = stroke;
            }

          if ((pref_mindist < 0 || pref_mindist > dx * dx + dy * dy) &&
              PICMAN_ANCHOR (list->data)->type == preferred)
            {
              pref_mindist = dx * dx + dy * dy;
              pref_anchor = PICMAN_ANCHOR (list->data);
              pref_stroke = stroke;
            }
        }

      g_list_free (anchor_list);
    }

  /* If the data passed into ret_anchor is a preferred anchor, return it. */
  if (ret_anchor && *ret_anchor &&
      picman_draw_tool_on_handle (draw_tool, display,
                                coord->x,
                                coord->y,
                                PICMAN_HANDLE_CIRCLE,
                                (*ret_anchor)->position.x,
                                (*ret_anchor)->position.y,
                                width, height,
                                PICMAN_HANDLE_ANCHOR_CENTER) &&
      (*ret_anchor)->type == preferred)
    {
      if (ret_stroke) *ret_stroke = pref_stroke;

      return TRUE;
    }

  if (pref_anchor && picman_draw_tool_on_handle (draw_tool, display,
                                               coord->x,
                                               coord->y,
                                               PICMAN_HANDLE_CIRCLE,
                                               pref_anchor->position.x,
                                               pref_anchor->position.y,
                                               width, height,
                                               PICMAN_HANDLE_ANCHOR_CENTER))
    {
      if (ret_anchor) *ret_anchor = pref_anchor;
      if (ret_stroke) *ret_stroke = pref_stroke;

      return TRUE;
    }
  else if (!exclusive && anchor &&
           picman_draw_tool_on_handle (draw_tool, display,
                                     coord->x,
                                     coord->y,
                                     PICMAN_HANDLE_CIRCLE,
                                     anchor->position.x,
                                     anchor->position.y,
                                     width, height,
                                     PICMAN_HANDLE_ANCHOR_CENTER))
    {
      if (ret_anchor)
        *ret_anchor = anchor;

      /* *ret_stroke already set correctly. */
      return TRUE;
    }

  if (ret_anchor)
    *ret_anchor = NULL;
  if (ret_stroke)
    *ret_stroke = NULL;

  return FALSE;
}

gboolean
picman_draw_tool_on_vectors_curve (PicmanDrawTool      *draw_tool,
                                 PicmanDisplay       *display,
                                 PicmanVectors       *vectors,
                                 const PicmanCoords  *coord,
                                 gint               width,
                                 gint               height,
                                 PicmanCoords        *ret_coords,
                                 gdouble           *ret_pos,
                                 PicmanAnchor       **ret_segment_start,
                                 PicmanAnchor       **ret_segment_end,
                                 PicmanStroke       **ret_stroke)
{
  PicmanStroke *stroke = NULL;
  PicmanAnchor *segment_start;
  PicmanAnchor *segment_end;
  PicmanCoords  min_coords = PICMAN_COORDS_DEFAULT_VALUES;
  PicmanCoords  cur_coords;
  gdouble     min_dist, cur_dist, cur_pos;

  g_return_val_if_fail (PICMAN_IS_DRAW_TOOL (draw_tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (coord != NULL, FALSE);

  if (ret_coords)        *ret_coords        = *coord;
  if (ret_pos)           *ret_pos           = -1.0;
  if (ret_segment_start) *ret_segment_start = NULL;
  if (ret_segment_start) *ret_segment_end   = NULL;
  if (ret_stroke)        *ret_stroke        = NULL;

  min_dist = -1.0;

  while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
    {
      cur_dist = picman_stroke_nearest_point_get (stroke, coord, 1.0,
                                                &cur_coords,
                                                &segment_start,
                                                &segment_end,
                                                &cur_pos);

      if (cur_dist >= 0 && (min_dist < 0 || cur_dist < min_dist))
        {
          min_dist   = cur_dist;
          min_coords = cur_coords;

          if (ret_coords)        *ret_coords        = cur_coords;
          if (ret_pos)           *ret_pos           = cur_pos;
          if (ret_segment_start) *ret_segment_start = segment_start;
          if (ret_segment_end)   *ret_segment_end   = segment_end;
          if (ret_stroke)        *ret_stroke        = stroke;
        }
    }

  if (min_dist >= 0 &&
      picman_draw_tool_on_handle (draw_tool, display,
                                coord->x,
                                coord->y,
                                PICMAN_HANDLE_CIRCLE,
                                min_coords.x,
                                min_coords.y,
                                width, height,
                                PICMAN_HANDLE_ANCHOR_CENTER))
    {
      return TRUE;
    }

  return FALSE;
}

gboolean
picman_draw_tool_on_vectors (PicmanDrawTool      *draw_tool,
                           PicmanDisplay       *display,
                           const PicmanCoords  *coords,
                           gint               width,
                           gint               height,
                           PicmanCoords        *ret_coords,
                           gdouble           *ret_pos,
                           PicmanAnchor       **ret_segment_start,
                           PicmanAnchor       **ret_segment_end,
                           PicmanStroke       **ret_stroke,
                           PicmanVectors      **ret_vectors)
{
  GList *all_vectors;
  GList *list;

  if (ret_coords)        *ret_coords         = *coords;
  if (ret_pos)           *ret_pos            = -1.0;
  if (ret_segment_start) *ret_segment_start  = NULL;
  if (ret_segment_end)   *ret_segment_end    = NULL;
  if (ret_stroke)        *ret_stroke         = NULL;
  if (ret_vectors)       *ret_vectors        = NULL;

  all_vectors = picman_image_get_vectors_list (picman_display_get_image (display));

  for (list = all_vectors; list; list = g_list_next (list))
    {
      PicmanVectors *vectors = list->data;

      if (! picman_item_get_visible (PICMAN_ITEM (vectors)))
        continue;

      if (picman_draw_tool_on_vectors_curve (draw_tool,
                                           display,
                                           vectors, coords,
                                           width, height,
                                           ret_coords,
                                           ret_pos,
                                           ret_segment_start,
                                           ret_segment_end,
                                           ret_stroke))
        {
          if (ret_vectors)
            *ret_vectors = vectors;

          g_list_free (all_vectors);

          return TRUE;
        }
    }

  g_list_free (all_vectors);

  return FALSE;
}
