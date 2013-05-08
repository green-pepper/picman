/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanregionselecttool.c
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmanboundary.h"
#include "core/picmanchannel.h"
#include "core/picmanchannel-select.h"
#include "core/picmanimage.h"
#include "core/picmanlayer-floating-sel.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-cursor.h"

#include "picmanregionselectoptions.h"
#include "picmanregionselecttool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_region_select_tool_finalize       (GObject               *object);

static void   picman_region_select_tool_button_press   (PicmanTool              *tool,
                                                      const PicmanCoords      *coords,
                                                      guint32                time,
                                                      GdkModifierType        state,
                                                      PicmanButtonPressType    press_type,
                                                      PicmanDisplay           *display);
static void   picman_region_select_tool_button_release (PicmanTool              *tool,
                                                      const PicmanCoords      *coords,
                                                      guint32                time,
                                                      GdkModifierType        state,
                                                      PicmanButtonReleaseType  release_type,
                                                      PicmanDisplay           *display);
static void   picman_region_select_tool_motion         (PicmanTool              *tool,
                                                      const PicmanCoords      *coords,
                                                      guint32                time,
                                                      GdkModifierType        state,
                                                      PicmanDisplay           *display);
static void   picman_region_select_tool_cursor_update  (PicmanTool              *tool,
                                                      const PicmanCoords      *coords,
                                                      GdkModifierType        state,
                                                      PicmanDisplay           *display);

static void   picman_region_select_tool_draw           (PicmanDrawTool          *draw_tool);

static PicmanBoundSeg * picman_region_select_tool_calculate (PicmanRegionSelectTool *region_sel,
                                                         PicmanDisplay          *display,
                                                         gint                 *n_segs);


G_DEFINE_TYPE (PicmanRegionSelectTool, picman_region_select_tool,
               PICMAN_TYPE_SELECTION_TOOL)

#define parent_class picman_region_select_tool_parent_class


static void
picman_region_select_tool_class_init (PicmanRegionSelectToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->finalize     = picman_region_select_tool_finalize;

  tool_class->button_press   = picman_region_select_tool_button_press;
  tool_class->button_release = picman_region_select_tool_button_release;
  tool_class->motion         = picman_region_select_tool_motion;
  tool_class->cursor_update  = picman_region_select_tool_cursor_update;

  draw_tool_class->draw      = picman_region_select_tool_draw;
}

static void
picman_region_select_tool_init (PicmanRegionSelectTool *region_select)
{
  PicmanTool *tool = PICMAN_TOOL (region_select);

  picman_tool_control_set_scroll_lock (tool->control, TRUE);
  picman_tool_control_set_motion_mode (tool->control, PICMAN_MOTION_MODE_COMPRESS);

  region_select->x               = 0;
  region_select->y               = 0;
  region_select->saved_threshold = 0.0;

  region_select->region_mask     = NULL;
  region_select->segs            = NULL;
  region_select->n_segs          = 0;
}

static void
picman_region_select_tool_finalize (GObject *object)
{
  PicmanRegionSelectTool *region_sel = PICMAN_REGION_SELECT_TOOL (object);

  if (region_sel->region_mask)
    {
      g_object_unref (region_sel->region_mask);
      region_sel->region_mask = NULL;
    }

  if (region_sel->segs)
    {
      g_free (region_sel->segs);
      region_sel->segs   = NULL;
      region_sel->n_segs = 0;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_region_select_tool_button_press (PicmanTool            *tool,
                                      const PicmanCoords    *coords,
                                      guint32              time,
                                      GdkModifierType      state,
                                      PicmanButtonPressType  press_type,
                                      PicmanDisplay         *display)
{
  PicmanRegionSelectTool    *region_sel = PICMAN_REGION_SELECT_TOOL (tool);
  PicmanRegionSelectOptions *options    = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (tool);

  region_sel->x               = coords->x;
  region_sel->y               = coords->y;
  region_sel->saved_threshold = options->threshold;

  if (picman_selection_tool_start_edit (PICMAN_SELECTION_TOOL (region_sel),
                                      display, coords))
    {
      return;
    }

  picman_tool_control_activate (tool->control);
  tool->display = display;

  picman_tool_push_status (tool, display,
                         _("Move the mouse to change threshold"));

  /*  calculate the region boundary  */
  region_sel->segs = picman_region_select_tool_calculate (region_sel, display,
                                                        &region_sel->n_segs);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
}

static void
picman_region_select_tool_button_release (PicmanTool              *tool,
                                        const PicmanCoords      *coords,
                                        guint32                time,
                                        GdkModifierType        state,
                                        PicmanButtonReleaseType  release_type,
                                        PicmanDisplay           *display)
{
  PicmanRegionSelectTool    *region_sel  = PICMAN_REGION_SELECT_TOOL (tool);
  PicmanSelectionOptions    *sel_options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  PicmanRegionSelectOptions *options     = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (tool);
  PicmanImage               *image       = picman_display_get_image (display);

  picman_tool_pop_status (tool, display);

  picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  picman_tool_control_halt (tool->control);

  if (release_type != PICMAN_BUTTON_RELEASE_CANCEL)
    {
      if (PICMAN_SELECTION_TOOL (tool)->function == SELECTION_ANCHOR)
        {
          if (picman_image_get_floating_selection (image))
            {
              /*  If there is a floating selection, anchor it  */
              floating_sel_anchor (picman_image_get_floating_selection (image));
            }
          else
            {
              /*  Otherwise, clear the selection mask  */
              picman_channel_clear (picman_image_get_mask (image), NULL, TRUE);
            }

          picman_image_flush (image);
        }
      else if (region_sel->region_mask)
        {
          gint off_x = 0;
          gint off_y = 0;

          if (! options->sample_merged)
            {
              PicmanDrawable *drawable = picman_image_get_active_drawable (image);

              picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);
            }

          picman_channel_select_buffer (picman_image_get_mask (image),
                                      PICMAN_REGION_SELECT_TOOL_GET_CLASS (tool)->undo_desc,
                                      region_sel->region_mask,
                                      off_x,
                                      off_y,
                                      sel_options->operation,
                                      sel_options->feather,
                                      sel_options->feather_radius,
                                      sel_options->feather_radius);


          picman_image_flush (image);
        }
    }

  if (region_sel->region_mask)
    {
      g_object_unref (region_sel->region_mask);
      region_sel->region_mask = NULL;
    }

  if (region_sel->segs)
    {
      g_free (region_sel->segs);
      region_sel->segs   = NULL;
      region_sel->n_segs = 0;
    }

  /*  Restore the original threshold  */
  g_object_set (options,
                "threshold", region_sel->saved_threshold,
                NULL);
}

static void
picman_region_select_tool_motion (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                guint32           time,
                                GdkModifierType   state,
                                PicmanDisplay      *display)
{
  PicmanRegionSelectTool    *region_sel = PICMAN_REGION_SELECT_TOOL (tool);
  PicmanRegionSelectOptions *options    = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (tool);
  gint                     diff_x, diff_y;
  gdouble                  diff;

  static guint32 last_time = 0;

  /* don't let the events come in too fast, ignore below a delay of 100 ms */
  if (time - last_time < 100)
    return;

  last_time = time;

  diff_x = coords->x - region_sel->x;
  diff_y = coords->y - region_sel->y;

  diff = ((ABS (diff_x) > ABS (diff_y)) ? diff_x : diff_y) / 2.0;

  g_object_set (options,
                "threshold", CLAMP (region_sel->saved_threshold + diff, 0, 255),
                NULL);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  if (region_sel->segs)
    g_free (region_sel->segs);

  region_sel->segs = picman_region_select_tool_calculate (region_sel, display,
                                                        &region_sel->n_segs);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_region_select_tool_cursor_update (PicmanTool         *tool,
                                       const PicmanCoords *coords,
                                       GdkModifierType   state,
                                       PicmanDisplay      *display)
{
  PicmanRegionSelectOptions *options  = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (tool);
  PicmanCursorModifier       modifier = PICMAN_CURSOR_MODIFIER_NONE;
  PicmanImage               *image    = picman_display_get_image (display);

  if (! picman_image_coords_in_active_pickable (image, coords,
                                              options->sample_merged, FALSE))
    modifier = PICMAN_CURSOR_MODIFIER_BAD;

  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_region_select_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanRegionSelectTool    *region_sel = PICMAN_REGION_SELECT_TOOL (draw_tool);
  PicmanRegionSelectOptions *options    = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (draw_tool);

  if (region_sel->segs)
    {
      gint off_x = 0;
      gint off_y = 0;

      if (! options->sample_merged)
        {
          PicmanImage    *image    = picman_display_get_image (draw_tool->display);
          PicmanDrawable *drawable = picman_image_get_active_drawable (image);

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);
        }

      picman_draw_tool_add_boundary (draw_tool,
                                   region_sel->segs,
                                   region_sel->n_segs,
                                   NULL,
                                   off_x, off_y);
    }
}

static PicmanBoundSeg *
picman_region_select_tool_calculate (PicmanRegionSelectTool *region_sel,
                                   PicmanDisplay          *display,
                                   gint                 *n_segs)
{
  PicmanDisplayShell *shell = picman_display_get_shell (display);
  PicmanBoundSeg     *segs;

  picman_display_shell_set_override_cursor (shell, GDK_WATCH);

  if (region_sel->region_mask)
    g_object_unref (region_sel->region_mask);

  region_sel->region_mask =
    PICMAN_REGION_SELECT_TOOL_GET_CLASS (region_sel)->get_mask (region_sel,
                                                              display);

  if (! region_sel->region_mask)
    {
      picman_display_shell_unset_override_cursor (shell);

      *n_segs = 0;
      return NULL;
    }

  /*  calculate and allocate a new segment array which represents the
   *  boundary of the contiguous region
   */
  segs = picman_boundary_find (region_sel->region_mask, NULL,
                             babl_format ("Y float"),
                             PICMAN_BOUNDARY_WITHIN_BOUNDS,
                             0, 0,
                             gegl_buffer_get_width  (region_sel->region_mask),
                             gegl_buffer_get_height (region_sel->region_mask),
                             PICMAN_BOUNDARY_HALF_WAY,
                             n_segs);

  picman_display_shell_unset_override_cursor (shell);

  return segs;
}
