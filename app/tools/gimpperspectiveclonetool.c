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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-transform-utils.h"
#include "core/picmanimage.h"

#include "paint/picmanperspectiveclone.h"
#include "paint/picmanperspectivecloneoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"

#include "picmanperspectiveclonetool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


/*  index into trans_info array  */
enum
{
  X0,
  Y0,
  X1,
  Y1,
  X2,
  Y2,
  X3,
  Y3
};


static void          picman_perspective_clone_tool_constructed   (GObject          *object);

static gboolean      picman_perspective_clone_tool_initialize    (PicmanTool         *tool,
                                                                PicmanDisplay      *display,
                                                                GError          **error);

static gboolean      picman_perspective_clone_tool_has_display   (PicmanTool         *tool,
                                                                PicmanDisplay      *display);
static PicmanDisplay * picman_perspective_clone_tool_has_image     (PicmanTool         *tool,
                                                                PicmanImage        *image);
static void          picman_perspective_clone_tool_control       (PicmanTool         *tool,
                                                                PicmanToolAction    action,
                                                                PicmanDisplay      *display);
static void          picman_perspective_clone_tool_halt          (PicmanPerspectiveCloneTool *clone_tool);
static void          picman_perspective_clone_tool_button_press  (PicmanTool         *tool,
                                                                const PicmanCoords *coords,
                                                                guint32           time,
                                                                GdkModifierType   state,
                                                                PicmanButtonPressType  press_type,
                                                                PicmanDisplay      *display);
static void          picman_perspective_clone_tool_button_release(PicmanTool         *tool,
                                                                const PicmanCoords *coords,
                                                                guint32           time,
                                                                GdkModifierType   state,
                                                                PicmanButtonReleaseType  release_type,
                                                                PicmanDisplay      *display);
static void          picman_perspective_clone_tool_motion        (PicmanTool         *tool,
                                                                const PicmanCoords *coords,
                                                                guint32           time,
                                                                GdkModifierType   state,
                                                                PicmanDisplay      *display);
static void          picman_perspective_clone_tool_cursor_update (PicmanTool         *tool,
                                                                const PicmanCoords *coords,
                                                                GdkModifierType   state,
                                                                PicmanDisplay      *display);
static void          picman_perspective_clone_tool_oper_update   (PicmanTool         *tool,
                                                                const PicmanCoords *coords,
                                                                GdkModifierType   state,
                                                                gboolean          proximity,
                                                                PicmanDisplay      *display);

static void          picman_perspective_clone_tool_mode_notify   (PicmanPerspectiveCloneOptions *options,
                                                                GParamSpec       *pspec,
                                                                PicmanPerspectiveCloneTool *clone_tool);

static void          picman_perspective_clone_tool_draw                   (PicmanDrawTool             *draw_tool);
static void          picman_perspective_clone_tool_transform_bounding_box (PicmanPerspectiveCloneTool *clone_tool);
static void          picman_perspective_clone_tool_bounds                 (PicmanPerspectiveCloneTool *tool,
                                                                         PicmanDisplay              *display);
static void          picman_perspective_clone_tool_prepare                (PicmanPerspectiveCloneTool *clone_tool);
static void          picman_perspective_clone_tool_recalc_matrix          (PicmanPerspectiveCloneTool *clone_tool);

static GtkWidget   * picman_perspective_clone_options_gui                 (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanPerspectiveCloneTool, picman_perspective_clone_tool,
               PICMAN_TYPE_BRUSH_TOOL)

#define parent_class picman_perspective_clone_tool_parent_class


void
picman_perspective_clone_tool_register (PicmanToolRegisterCallback  callback,
                                      gpointer                  data)
{
  (* callback) (PICMAN_TYPE_PERSPECTIVE_CLONE_TOOL,
                PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS,
                picman_perspective_clone_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_PATTERN_MASK,
                "picman-perspective-clone-tool",
                _("Perspective Clone"),
                _("Perspective Clone Tool: Clone from an image source "
                  "after applying a perspective transformation"),
                N_("_Perspective Clone"), NULL,
                NULL, PICMAN_HELP_TOOL_PERSPECTIVE_CLONE,
                PICMAN_STOCK_TOOL_PERSPECTIVE_CLONE,
                data);
}

static void
picman_perspective_clone_tool_class_init (PicmanPerspectiveCloneToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed  = picman_perspective_clone_tool_constructed;

  tool_class->initialize     = picman_perspective_clone_tool_initialize;
  tool_class->has_display    = picman_perspective_clone_tool_has_display;
  tool_class->has_image      = picman_perspective_clone_tool_has_image;
  tool_class->control        = picman_perspective_clone_tool_control;
  tool_class->button_press   = picman_perspective_clone_tool_button_press;
  tool_class->button_release = picman_perspective_clone_tool_button_release;
  tool_class->motion         = picman_perspective_clone_tool_motion;
  tool_class->cursor_update  = picman_perspective_clone_tool_cursor_update;
  tool_class->oper_update    = picman_perspective_clone_tool_oper_update;

  draw_tool_class->draw      = picman_perspective_clone_tool_draw;
}

static void
picman_perspective_clone_tool_init (PicmanPerspectiveCloneTool *clone_tool)
{
  PicmanTool *tool = PICMAN_TOOL (clone_tool);

  picman_tool_control_set_action_object_2 (tool->control,
                                         "context/context-pattern-select-set");

  picman_matrix3_identity (&clone_tool->transform);
}

static void
picman_perspective_clone_tool_constructed (GObject *object)
{
  PicmanTool                    *tool       = PICMAN_TOOL (object);
  PicmanPerspectiveCloneTool    *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (object);
  PicmanPerspectiveCloneOptions *options;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  g_signal_connect_object (options,
                           "notify::clone-mode",
                           G_CALLBACK (picman_perspective_clone_tool_mode_notify),
                           clone_tool, 0);

  picman_perspective_clone_tool_mode_notify (options, NULL, clone_tool);
}

static gboolean
picman_perspective_clone_tool_initialize (PicmanTool     *tool,
                                        PicmanDisplay  *display,
                                        GError      **error)
{
  PicmanPerspectiveCloneTool *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  if (display != tool->display)
    {
      PicmanImage *image = picman_display_get_image (display);
      gint       i;

      tool->display  = display;
      tool->drawable = picman_image_get_active_drawable (image);

      /*  Find the transform bounds initializing */
      picman_perspective_clone_tool_bounds (clone_tool, display);

      picman_perspective_clone_tool_prepare (clone_tool);

      /*  Recalculate the transform tool  */
      picman_perspective_clone_tool_recalc_matrix (clone_tool);

      /*  start drawing the bounding box and handles...  */
      picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

      clone_tool->function = TRANSFORM_CREATING;

      /*  Save the current transformation info  */
      for (i = 0; i < TRANS_INFO_SIZE; i++)
        clone_tool->old_trans_info[i] = clone_tool->trans_info[i];
    }

  return TRUE;
}

static gboolean
picman_perspective_clone_tool_has_display (PicmanTool    *tool,
                                         PicmanDisplay *display)
{
  PicmanPerspectiveCloneTool *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);

  return (display == clone_tool->src_display ||
          PICMAN_TOOL_CLASS (parent_class)->has_display (tool, display));
}

static PicmanDisplay *
picman_perspective_clone_tool_has_image (PicmanTool  *tool,
                                       PicmanImage *image)
{
  PicmanPerspectiveCloneTool *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);
  PicmanDisplay              *display;

  display = PICMAN_TOOL_CLASS (parent_class)->has_image (tool, image);

  if (! display && clone_tool->src_display)
    {
      if (image && picman_display_get_image (clone_tool->src_display) == image)
        display = clone_tool->src_display;

      /*  NULL image means any display  */
      if (! image)
        display = clone_tool->src_display;
    }

  return display;
}

static void
picman_perspective_clone_tool_control (PicmanTool       *tool,
                                     PicmanToolAction  action,
                                     PicmanDisplay    *display)
{
  PicmanPerspectiveCloneTool *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
      break;

    case PICMAN_TOOL_ACTION_RESUME:
      /* only in the case that "Modify Polygon" mode is set " */
      picman_perspective_clone_tool_bounds (clone_tool, display);
      picman_perspective_clone_tool_recalc_matrix (clone_tool);
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_perspective_clone_tool_halt (clone_tool);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_perspective_clone_tool_halt (PicmanPerspectiveCloneTool *clone_tool)
{
  PicmanTool *tool = PICMAN_TOOL (clone_tool);

  clone_tool->src_display = NULL;

  g_object_set (PICMAN_PAINT_TOOL (tool)->core,
                "src-drawable", NULL,
                NULL);

  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tool)))
    picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  tool->display  = NULL;
  tool->drawable = NULL;
}

static void
picman_perspective_clone_tool_button_press (PicmanTool            *tool,
                                          const PicmanCoords    *coords,
                                          guint32              time,
                                          GdkModifierType      state,
                                          PicmanButtonPressType  press_type,
                                          PicmanDisplay         *display)
{
  PicmanPaintTool               *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanPerspectiveCloneTool    *clone_tool  = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);
  PicmanPerspectiveClone        *clone       = PICMAN_PERSPECTIVE_CLONE (paint_tool->core);
  PicmanSourceCore              *source_core = PICMAN_SOURCE_CORE (clone);
  PicmanPerspectiveCloneOptions *options;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  switch (options->clone_mode)
    {
    case PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST:
      if (clone_tool->function == TRANSFORM_CREATING)
        picman_perspective_clone_tool_oper_update (tool,
                                                 coords, state, TRUE, display);

      clone_tool->lastx = coords->x;
      clone_tool->lasty = coords->y;

      picman_tool_control_activate (tool->control);
      break;

    case PICMAN_PERSPECTIVE_CLONE_MODE_PAINT:
      {
        GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();
        gdouble         nnx, nny;

        picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

        if ((state & (toggle_mask | GDK_SHIFT_MASK)) == toggle_mask)
          {
            source_core->set_source = TRUE;

            clone_tool->src_display = display;
          }
        else
          {
            source_core->set_source = FALSE;
          }

        PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                      press_type, display);

        /* Set the coordinates for the reference cross */
        picman_perspective_clone_get_source_point (clone,
                                                 coords->x, coords->y,
                                                 &nnx, &nny);

        clone_tool->src_x = nnx;
        clone_tool->src_y = nny;

        picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
      }
      break;
    }
}

static void
picman_perspective_clone_tool_button_release (PicmanTool              *tool,
                                            const PicmanCoords      *coords,
                                            guint32                time,
                                            GdkModifierType        state,
                                            PicmanButtonReleaseType  release_type,
                                            PicmanDisplay           *display)
{
  PicmanPerspectiveCloneOptions *options;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  switch (options->clone_mode)
    {
    case PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST:
      picman_tool_control_halt (tool->control);
      break;

    case PICMAN_PERSPECTIVE_CLONE_MODE_PAINT:
      PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state,
                                                      release_type, display);
      break;
    }
}

static void
picman_perspective_clone_tool_prepare (PicmanPerspectiveCloneTool *clone_tool)
{
  clone_tool->trans_info[X0] = clone_tool->x1;
  clone_tool->trans_info[Y0] = clone_tool->y1;
  clone_tool->trans_info[X1] = clone_tool->x2;
  clone_tool->trans_info[Y1] = clone_tool->y1;
  clone_tool->trans_info[X2] = clone_tool->x1;
  clone_tool->trans_info[Y2] = clone_tool->y2;
  clone_tool->trans_info[X3] = clone_tool->x2;
  clone_tool->trans_info[Y3] = clone_tool->y2;
}

static void
picman_perspective_clone_tool_recalc_matrix (PicmanPerspectiveCloneTool *clone_tool)
{
  picman_matrix3_identity (&clone_tool->transform);
  picman_transform_matrix_perspective (&clone_tool->transform,
                                     clone_tool->x1,
                                     clone_tool->y1,
                                     clone_tool->x2 - clone_tool->x1,
                                     clone_tool->y2 - clone_tool->y1,
                                     clone_tool->trans_info[X0],
                                     clone_tool->trans_info[Y0],
                                     clone_tool->trans_info[X1],
                                     clone_tool->trans_info[Y1],
                                     clone_tool->trans_info[X2],
                                     clone_tool->trans_info[Y2],
                                     clone_tool->trans_info[X3],
                                     clone_tool->trans_info[Y3]);

  picman_perspective_clone_tool_transform_bounding_box (clone_tool);
}

static void
picman_perspective_clone_tool_motion (PicmanTool         *tool,
                                    const PicmanCoords *coords,
                                    guint32           time,
                                    GdkModifierType   state,
                                    PicmanDisplay      *display)
{
  PicmanPerspectiveCloneTool    *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);
  PicmanPaintTool               *paint_tool = PICMAN_PAINT_TOOL (tool);
  PicmanPerspectiveClone        *clone      = PICMAN_PERSPECTIVE_CLONE (paint_tool->core);
  PicmanPerspectiveCloneOptions *options;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST)
    {
      gdouble diff_x, diff_y;

      /*  if we are creating, there is nothing to be done so exit.  */
      if (clone_tool->function == TRANSFORM_CREATING)
        return;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      clone_tool->curx = coords->x;
      clone_tool->cury = coords->y;

      /*  recalculate the tool's transformation matrix  */

      diff_x = clone_tool->curx - clone_tool->lastx;
      diff_y = clone_tool->cury - clone_tool->lasty;

      switch (clone_tool->function)
        {
        case TRANSFORM_HANDLE_NW:
          clone_tool->trans_info[X0] += diff_x;
          clone_tool->trans_info[Y0] += diff_y;
          break;
        case TRANSFORM_HANDLE_NE:
          clone_tool->trans_info[X1] += diff_x;
          clone_tool->trans_info[Y1] += diff_y;
          break;
        case TRANSFORM_HANDLE_SW:
          clone_tool->trans_info[X2] += diff_x;
          clone_tool->trans_info[Y2] += diff_y;
          break;
        case TRANSFORM_HANDLE_SE:
          clone_tool->trans_info[X3] += diff_x;
          clone_tool->trans_info[Y3] += diff_y;
          break;
        default:
          break;
        }

      picman_perspective_clone_tool_recalc_matrix (clone_tool);

      clone_tool->lastx = clone_tool->curx;
      clone_tool->lasty = clone_tool->cury;

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
  else if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_PAINT)
    {
      gdouble nnx, nny;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      PICMAN_TOOL_CLASS (parent_class)->motion (tool, coords, time, state,
                                              display);

      /* Set the coordinates for the reference cross */
      picman_perspective_clone_get_source_point (clone,
                                               coords->x, coords->y,
                                               &nnx, &nny);

      clone_tool->src_x = nnx;
      clone_tool->src_y = nny;

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
}

static void
picman_perspective_clone_tool_cursor_update (PicmanTool         *tool,
                                           const PicmanCoords *coords,
                                           GdkModifierType   state,
                                           PicmanDisplay      *display)
{
  PicmanPerspectiveCloneTool    *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);
  PicmanPerspectiveCloneOptions *options;
  PicmanImage                   *image;
  PicmanToolClass               *tool_class;
  PicmanCursorType               cursor     = PICMAN_CURSOR_MOUSE;
  PicmanCursorModifier           modifier   = PICMAN_CURSOR_MODIFIER_NONE;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  image = picman_display_get_image (display);

  if (picman_image_coords_in_active_pickable (image, coords,
                                            FALSE, TRUE))
    {
      cursor = PICMAN_CURSOR_MOUSE;
    }

  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST)
    {
      /* perspective cursors */
      cursor = picman_tool_control_get_cursor (tool->control);

      switch (clone_tool->function)
        {
        case TRANSFORM_HANDLE_NW:
          cursor = PICMAN_CURSOR_CORNER_TOP_LEFT;
          break;

        case TRANSFORM_HANDLE_NE:
          cursor = PICMAN_CURSOR_CORNER_TOP_RIGHT;
          break;

        case TRANSFORM_HANDLE_SW:
          cursor = PICMAN_CURSOR_CORNER_BOTTOM_LEFT;
          break;

        case TRANSFORM_HANDLE_SE:
          cursor = PICMAN_CURSOR_CORNER_BOTTOM_RIGHT;
          break;

        default:
          cursor = PICMAN_CURSOR_CROSSHAIR_SMALL;
          break;
        }
    }
  else
    {
      GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

      if ((state & (toggle_mask | GDK_SHIFT_MASK)) == toggle_mask)
        {
          cursor = PICMAN_CURSOR_CROSSHAIR_SMALL;
        }
      else if (! PICMAN_SOURCE_CORE (PICMAN_PAINT_TOOL (tool)->core)->src_drawable)
        {
          modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  /*  If we are in adjust mode, skip the PicmanBrushClass when chaining up.
   *  This ensures that the cursor will be set regardless of
   *  PicmanBrushTool::show_cursor (see bug #354933).
   */
  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST)
    tool_class = PICMAN_TOOL_CLASS (g_type_class_peek_parent (parent_class));
  else
    tool_class = PICMAN_TOOL_CLASS (parent_class);

  tool_class->cursor_update (tool, coords, state, display);
}

static void
picman_perspective_clone_tool_oper_update (PicmanTool         *tool,
                                         const PicmanCoords *coords,
                                         GdkModifierType   state,
                                         gboolean          proximity,
                                         PicmanDisplay      *display)
{
  PicmanPerspectiveCloneTool    *clone_tool = PICMAN_PERSPECTIVE_CLONE_TOOL (tool);
  PicmanPerspectiveCloneOptions *options;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST)
    {
      PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);
      gdouble       closest_dist;
      gdouble       dist;

      clone_tool->function = TRANSFORM_HANDLE_NONE;

      if (display != tool->display)
        return;

      dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                  coords->x, coords->y,
                                                  clone_tool->tx1,
                                                  clone_tool->ty1);
      closest_dist = dist;
      clone_tool->function = TRANSFORM_HANDLE_NW;

      dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                  coords->x, coords->y,
                                                  clone_tool->tx2,
                                                  clone_tool->ty2);
      if (dist < closest_dist)
        {
          closest_dist = dist;
          clone_tool->function = TRANSFORM_HANDLE_NE;
        }

      dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                  coords->x, coords->y,
                                                  clone_tool->tx3,
                                                  clone_tool->ty3);
      if (dist < closest_dist)
        {
          closest_dist = dist;
          clone_tool->function = TRANSFORM_HANDLE_SW;
        }

      dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                  coords->x, coords->y,
                                                  clone_tool->tx4,
                                                  clone_tool->ty4);
      if (dist < closest_dist)
        {
          closest_dist = dist;
          clone_tool->function = TRANSFORM_HANDLE_SE;
        }
    }
  else
    {
      PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state,
                                                   proximity, display);

      if (proximity)
        {
          PicmanPaintCore        *core        = PICMAN_PAINT_TOOL (tool)->core;
          PicmanPerspectiveClone *clone       = PICMAN_PERSPECTIVE_CLONE (core);
          PicmanSourceCore       *source_core = PICMAN_SOURCE_CORE (core);

          if (source_core->src_drawable == NULL)
            {
              picman_tool_replace_status (tool, display,
                                        _("Ctrl-Click to set a clone source"));
            }
          else
            {
              picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

              clone_tool->src_x = source_core->src_x;
              clone_tool->src_y = source_core->src_y;

              if (! source_core->first_stroke)
                {
                  if (PICMAN_SOURCE_OPTIONS (options)->align_mode ==
                      PICMAN_SOURCE_ALIGN_YES)
                    {
                      gdouble nnx, nny;

                      /* Set the coordinates for the reference cross */
                      picman_perspective_clone_get_source_point (clone,
                                                               coords->x,
                                                               coords->y,
                                                               &nnx, &nny);

                      clone_tool->src_x = nnx;
                      clone_tool->src_y = nny;
                    }
                }

              picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
            }
        }
    }
}

static void
picman_perspective_clone_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanTool                    *tool        = PICMAN_TOOL (draw_tool);
  PicmanPerspectiveCloneTool    *clone_tool  = PICMAN_PERSPECTIVE_CLONE_TOOL (draw_tool);
  PicmanPerspectiveClone        *clone       = PICMAN_PERSPECTIVE_CLONE (PICMAN_PAINT_TOOL (tool)->core);
  PicmanSourceCore              *source_core = PICMAN_SOURCE_CORE (clone);
  PicmanPerspectiveCloneOptions *options;
  PicmanCanvasGroup             *stroke_group;

  options = PICMAN_PERSPECTIVE_CLONE_TOOL_GET_OPTIONS (tool);

  stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

  /*  draw the bounding box  */
  picman_draw_tool_push_group (draw_tool, stroke_group);

  picman_draw_tool_add_line (draw_tool,
                           clone_tool->tx1, clone_tool->ty1,
                           clone_tool->tx2, clone_tool->ty2);
  picman_draw_tool_add_line (draw_tool,
                           clone_tool->tx2, clone_tool->ty2,
                           clone_tool->tx4, clone_tool->ty4);
  picman_draw_tool_add_line (draw_tool,
                           clone_tool->tx3, clone_tool->ty3,
                           clone_tool->tx4, clone_tool->ty4);
  picman_draw_tool_add_line (draw_tool,
                           clone_tool->tx3, clone_tool->ty3,
                           clone_tool->tx1, clone_tool->ty1);

  picman_draw_tool_pop_group (draw_tool);

  /*  draw the tool handles only when they can be used  */
  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST)
    {
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_SQUARE,
                                 clone_tool->tx1, clone_tool->ty1,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_SQUARE,
                                 clone_tool->tx2, clone_tool->ty2,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_SQUARE,
                                 clone_tool->tx3, clone_tool->ty3,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_SQUARE,
                                 clone_tool->tx4, clone_tool->ty4,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_TOOL_HANDLE_SIZE_LARGE,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
    }

  if (source_core->src_drawable && clone_tool->src_display)
    {
      PicmanDisplay *tmp_display;

      tmp_display = draw_tool->display;
      draw_tool->display = clone_tool->src_display;

      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_CROSS,
                                 clone_tool->src_x,
                                 clone_tool->src_y,
                                 PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                 PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                 PICMAN_HANDLE_ANCHOR_CENTER);

      draw_tool->display = tmp_display;
    }

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);
}

static void
picman_perspective_clone_tool_transform_bounding_box (PicmanPerspectiveCloneTool *clone_tool)
{
  g_return_if_fail (PICMAN_IS_PERSPECTIVE_CLONE_TOOL (clone_tool));

  picman_matrix3_transform_point (&clone_tool->transform,
                                clone_tool->x1,
                                clone_tool->y1,
                                &clone_tool->tx1,
                                &clone_tool->ty1);
  picman_matrix3_transform_point (&clone_tool->transform,
                                clone_tool->x2,
                                clone_tool->y1,
                                &clone_tool->tx2,
                                &clone_tool->ty2);
  picman_matrix3_transform_point (&clone_tool->transform,
                                clone_tool->x1,
                                clone_tool->y2,
                                &clone_tool->tx3,
                                &clone_tool->ty3);
  picman_matrix3_transform_point (&clone_tool->transform,
                                clone_tool->x2,
                                clone_tool->y2,
                                &clone_tool->tx4,
                                &clone_tool->ty4);
}

static void
picman_perspective_clone_tool_bounds (PicmanPerspectiveCloneTool *tool,
                                    PicmanDisplay              *display)
{
  PicmanImage *image = picman_display_get_image (display);

  tool->x1 = 0;
  tool->y1 = 0;
  tool->x2 = picman_image_get_width  (image);
  tool->y2 = picman_image_get_height (image);
}

static void
picman_perspective_clone_tool_mode_notify (PicmanPerspectiveCloneOptions *options,
                                         GParamSpec                  *pspec,
                                         PicmanPerspectiveCloneTool    *clone_tool)
{
  PicmanTool             *tool = PICMAN_TOOL (clone_tool);
  PicmanPerspectiveClone *clone;

  clone = PICMAN_PERSPECTIVE_CLONE (PICMAN_PAINT_TOOL (clone_tool)->core);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (clone_tool));

  if (options->clone_mode == PICMAN_PERSPECTIVE_CLONE_MODE_PAINT)
    {
      /* PicmanPaintTool's notify callback will set the right precision */
      g_object_notify (G_OBJECT (options), "hard");

      picman_tool_control_set_tool_cursor (tool->control,
                                         PICMAN_TOOL_CURSOR_CLONE);

      picman_perspective_clone_set_transform (clone, &clone_tool->transform);
    }
  else
    {
      picman_tool_control_set_precision (tool->control,
                                       PICMAN_CURSOR_PRECISION_SUBPIXEL);

      picman_tool_control_set_tool_cursor (tool->control,
                                         PICMAN_TOOL_CURSOR_PERSPECTIVE);

      /*  start drawing the bounding box and handles...  */
      if (tool->display &&
          ! picman_draw_tool_is_active (PICMAN_DRAW_TOOL (clone_tool)))
        {
          picman_draw_tool_start (PICMAN_DRAW_TOOL (clone_tool), tool->display);
        }
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (clone_tool));
}


/*  tool options stuff  */

static GtkWidget *
picman_perspective_clone_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox;
  GtkWidget *paint_options;
  GtkWidget *frame;
  GtkWidget *mode;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  paint_options = picman_paint_options_gui (tool_options);

  /* radio buttons to set if you are modifying perspe plane or painting */
  mode = picman_prop_enum_radio_box_new (config, "clone-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), mode, FALSE, FALSE, 0);
  gtk_widget_show (mode);

  gtk_box_pack_start (GTK_BOX (vbox), paint_options, FALSE, FALSE, 0);
  gtk_widget_show (paint_options);

  frame = picman_prop_enum_radio_frame_new (config, "clone-type",
                                          _("Source"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = picman_prop_check_button_new (config, "sample-merged",
                                       _("Sample merged"));
  picman_enum_radio_frame_add (GTK_FRAME (frame), button,
                             PICMAN_IMAGE_CLONE, TRUE);

  hbox = picman_prop_pattern_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                    NULL, 2,
                                    "pattern-view-type", "pattern-view-size");
  picman_enum_radio_frame_add (GTK_FRAME (frame), hbox,
                             PICMAN_PATTERN_CLONE, TRUE);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Alignment:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (config, "align-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  return vbox;
}
