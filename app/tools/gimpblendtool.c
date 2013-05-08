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

#include <stdlib.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman-utils.h"
#include "core/picmandrawable.h"
#include "core/picmandrawable-blend.h"
#include "core/picmanerror.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvashandle.h"
#include "display/picmancanvasline.h"
#include "display/picmandisplay.h"

#include "picmanblendoptions.h"
#include "picmanblendtool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean picman_blend_tool_initialize        (PicmanTool              *tool,
                                                   PicmanDisplay           *display,
                                                   GError               **error);
static void   picman_blend_tool_button_press        (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanButtonPressType    press_type,
                                                   PicmanDisplay           *display);
static void   picman_blend_tool_button_release      (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanButtonReleaseType  release_type,
                                                   PicmanDisplay           *display);
static void   picman_blend_tool_motion              (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   guint32                time,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);
static void   picman_blend_tool_active_modifier_key (PicmanTool              *tool,
                                                   GdkModifierType        key,
                                                   gboolean               press,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);
static void   picman_blend_tool_cursor_update       (PicmanTool              *tool,
                                                   const PicmanCoords      *coords,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);

static void   picman_blend_tool_draw                (PicmanDrawTool          *draw_tool);
static void   picman_blend_tool_update_items        (PicmanBlendTool         *blend_tool);

static void   picman_blend_tool_push_status         (PicmanBlendTool         *blend_tool,
                                                   GdkModifierType        state,
                                                   PicmanDisplay           *display);


G_DEFINE_TYPE (PicmanBlendTool, picman_blend_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_blend_tool_parent_class


void
picman_blend_tool_register (PicmanToolRegisterCallback  callback,
                          gpointer                  data)
{
  (* callback) (PICMAN_TYPE_BLEND_TOOL,
                PICMAN_TYPE_BLEND_OPTIONS,
                picman_blend_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK |
                PICMAN_CONTEXT_BACKGROUND_MASK |
                PICMAN_CONTEXT_OPACITY_MASK    |
                PICMAN_CONTEXT_PAINT_MODE_MASK |
                PICMAN_CONTEXT_GRADIENT_MASK,
                "picman-blend-tool",
                _("Blend"),
                _("Blend Tool: Fill selected area with a color gradient"),
                N_("Blen_d"), "L",
                NULL, PICMAN_HELP_TOOL_BLEND,
                PICMAN_STOCK_TOOL_BLEND,
                data);
}

static void
picman_blend_tool_class_init (PicmanBlendToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->initialize          = picman_blend_tool_initialize;
  tool_class->button_press        = picman_blend_tool_button_press;
  tool_class->button_release      = picman_blend_tool_button_release;
  tool_class->motion              = picman_blend_tool_motion;
  tool_class->active_modifier_key = picman_blend_tool_active_modifier_key;
  tool_class->cursor_update       = picman_blend_tool_cursor_update;

  draw_tool_class->draw           = picman_blend_tool_draw;
}

static void
picman_blend_tool_init (PicmanBlendTool *blend_tool)
{
  PicmanTool *tool = PICMAN_TOOL (blend_tool);

  picman_tool_control_set_scroll_lock     (tool->control, TRUE);
  picman_tool_control_set_precision       (tool->control,
                                         PICMAN_CURSOR_PRECISION_SUBPIXEL);
  picman_tool_control_set_tool_cursor     (tool->control,
                                         PICMAN_TOOL_CURSOR_BLEND);
  picman_tool_control_set_action_value_1  (tool->control,
                                         "context/context-opacity-set");
  picman_tool_control_set_action_object_1 (tool->control,
                                         "context/context-gradient-select-set");
}

static gboolean
picman_blend_tool_initialize (PicmanTool     *tool,
                            PicmanDisplay  *display,
                            GError      **error)
{
  PicmanImage        *image    = picman_display_get_image (display);
  PicmanDrawable     *drawable = picman_image_get_active_drawable (image);
  PicmanBlendOptions *options  = PICMAN_BLEND_TOOL_GET_OPTIONS (tool);

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot modify the pixels of layer groups."));
      return FALSE;
    }

  if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("The active layer's pixels are locked."));
      return FALSE;
    }

  if (! picman_context_get_gradient (PICMAN_CONTEXT (options)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("No gradient available for use with this tool."));
      return FALSE;
    }

  return TRUE;
}

static void
picman_blend_tool_button_press (PicmanTool            *tool,
                              const PicmanCoords    *coords,
                              guint32              time,
                              GdkModifierType      state,
                              PicmanButtonPressType  press_type,
                              PicmanDisplay         *display)
{
  PicmanBlendTool *blend_tool = PICMAN_BLEND_TOOL (tool);

  blend_tool->start_x = blend_tool->end_x = coords->x;
  blend_tool->start_y = blend_tool->end_y = coords->y;

  blend_tool->last_x = blend_tool->mouse_x = coords->x;
  blend_tool->last_y = blend_tool->mouse_y = coords->y;

  tool->display = display;

  picman_tool_control_activate (tool->control);

  picman_blend_tool_push_status (blend_tool, state, display);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
}

static void
picman_blend_tool_button_release (PicmanTool              *tool,
                                const PicmanCoords      *coords,
                                guint32                time,
                                GdkModifierType        state,
                                PicmanButtonReleaseType  release_type,
                                PicmanDisplay           *display)
{
  PicmanBlendTool    *blend_tool    = PICMAN_BLEND_TOOL (tool);
  PicmanBlendOptions *options       = PICMAN_BLEND_TOOL_GET_OPTIONS (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_OPTIONS (options);
  PicmanContext      *context       = PICMAN_CONTEXT (options);
  PicmanImage        *image         = picman_display_get_image (display);

  picman_tool_pop_status (tool, display);

  picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  picman_tool_control_halt (tool->control);

  if ((release_type != PICMAN_BUTTON_RELEASE_CANCEL) &&
      ((blend_tool->start_x != blend_tool->end_x) ||
       (blend_tool->start_y != blend_tool->end_y)))
    {
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);
      PicmanProgress *progress;
      gint          off_x;
      gint          off_y;

      progress = picman_progress_start (PICMAN_PROGRESS (tool),
                                      _("Blending"), FALSE);

      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

      picman_drawable_blend (drawable,
                           context,
                           PICMAN_CUSTOM_MODE,
                           picman_context_get_paint_mode (context),
                           options->gradient_type,
                           picman_context_get_opacity (context),
                           options->offset,
                           paint_options->gradient_options->gradient_repeat,
                           paint_options->gradient_options->gradient_reverse,
                           options->supersample,
                           options->supersample_depth,
                           options->supersample_threshold,
                           options->dither,
                           blend_tool->start_x - off_x,
                           blend_tool->start_y - off_y,
                           blend_tool->end_x - off_x,
                           blend_tool->end_y - off_y,
                           progress);

      if (progress)
        picman_progress_end (progress);

      picman_image_flush (image);
    }

  tool->display  = NULL;
  tool->drawable = NULL;
}

static void
picman_blend_tool_motion (PicmanTool         *tool,
                        const PicmanCoords *coords,
                        guint32           time,
                        GdkModifierType   state,
                        PicmanDisplay      *display)
{
  PicmanBlendTool *blend_tool = PICMAN_BLEND_TOOL (tool);

  blend_tool->mouse_x = coords->x;
  blend_tool->mouse_y = coords->y;

  /* Move the whole line if alt is pressed */
  if (state & GDK_MOD1_MASK)
    {
      gdouble dx = blend_tool->last_x - coords->x;
      gdouble dy = blend_tool->last_y - coords->y;

      blend_tool->start_x -= dx;
      blend_tool->start_y -= dy;

      blend_tool->end_x -= dx;
      blend_tool->end_y -= dy;
    }
  else
    {
      blend_tool->end_x = coords->x;
      blend_tool->end_y = coords->y;
    }

  if (state & picman_get_constrain_behavior_mask ())
    {
      picman_constrain_line (blend_tool->start_x, blend_tool->start_y,
                           &blend_tool->end_x, &blend_tool->end_y,
                           PICMAN_CONSTRAIN_LINE_15_DEGREES);
    }

  picman_tool_pop_status (tool, display);
  picman_blend_tool_push_status (blend_tool, state, display);

  blend_tool->last_x = coords->x;
  blend_tool->last_y = coords->y;

  picman_blend_tool_update_items (blend_tool);
}

static void
picman_blend_tool_active_modifier_key (PicmanTool        *tool,
                                     GdkModifierType  key,
                                     gboolean         press,
                                     GdkModifierType  state,
                                     PicmanDisplay     *display)
{
  PicmanBlendTool *blend_tool = PICMAN_BLEND_TOOL (tool);

  if (key == picman_get_constrain_behavior_mask ())
    {
      blend_tool->end_x = blend_tool->mouse_x;
      blend_tool->end_y = blend_tool->mouse_y;

      /* Restrict to multiples of 15 degrees if ctrl is pressed */
      if (press)
        {
          picman_constrain_line (blend_tool->start_x, blend_tool->start_y,
                               &blend_tool->end_x, &blend_tool->end_y,
                               PICMAN_CONSTRAIN_LINE_15_DEGREES);
        }

      picman_tool_pop_status (tool, display);
      picman_blend_tool_push_status (blend_tool, state, display);

      picman_blend_tool_update_items (blend_tool);
    }
  else if (key == GDK_MOD1_MASK)
    {
      picman_tool_pop_status (tool, display);
      picman_blend_tool_push_status (blend_tool, state, display);
    }
}

static void
picman_blend_tool_cursor_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display)
{
  PicmanImage          *image    = picman_display_get_image (display);
  PicmanDrawable       *drawable = picman_image_get_active_drawable (image);
  PicmanCursorModifier  modifier = PICMAN_CURSOR_MODIFIER_NONE;

  if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)) ||
      picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      modifier = PICMAN_CURSOR_MODIFIER_BAD;
    }

  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_blend_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanBlendTool *blend_tool = PICMAN_BLEND_TOOL (draw_tool);

  /*  Draw start target  */
  blend_tool->start_handle =
    picman_draw_tool_add_handle (draw_tool,
                               PICMAN_HANDLE_CROSS,
                               blend_tool->start_x,
                               blend_tool->start_y,
                               PICMAN_TOOL_HANDLE_SIZE_CROSS,
                               PICMAN_TOOL_HANDLE_SIZE_CROSS,
                               PICMAN_HANDLE_ANCHOR_CENTER);

  /*  Draw the line between the start and end coords  */
  blend_tool->line =
    picman_draw_tool_add_line (draw_tool,
                             blend_tool->start_x,
                             blend_tool->start_y,
                             blend_tool->end_x,
                             blend_tool->end_y);

  /*  Draw end target  */
  blend_tool->end_handle =
    picman_draw_tool_add_handle (draw_tool,
                               PICMAN_HANDLE_CROSS,
                               blend_tool->end_x,
                               blend_tool->end_y,
                               PICMAN_TOOL_HANDLE_SIZE_CROSS,
                               PICMAN_TOOL_HANDLE_SIZE_CROSS,
                               PICMAN_HANDLE_ANCHOR_CENTER);
}

static void
picman_blend_tool_update_items (PicmanBlendTool *blend_tool)
{
  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (blend_tool)))
    {
      picman_canvas_handle_set_position (blend_tool->start_handle,
                                       blend_tool->start_x,
                                       blend_tool->start_y);

      picman_canvas_line_set (blend_tool->line,
                            blend_tool->start_x,
                            blend_tool->start_y,
                            blend_tool->end_x,
                            blend_tool->end_y);

      picman_canvas_handle_set_position (blend_tool->end_handle,
                                       blend_tool->end_x,
                                       blend_tool->end_y);
    }
}

static void
picman_blend_tool_push_status (PicmanBlendTool   *blend_tool,
                             GdkModifierType  state,
                             PicmanDisplay     *display)
{
  PicmanTool *tool = PICMAN_TOOL (blend_tool);
  gchar    *status_help;

  status_help = picman_suggest_modifiers ("",
                                        (picman_get_constrain_behavior_mask () |
                                         GDK_MOD1_MASK) &
                                        ~state,
                                        NULL,
                                        _("%s for constrained angles"),
                                        _("%s to move the whole line"));

  picman_tool_push_status_coords (tool, display,
                                picman_tool_control_get_precision (tool->control),
                                _("Blend: "),
                                blend_tool->end_x - blend_tool->start_x,
                                ", ",
                                blend_tool->end_y - blend_tool->start_y,
                                status_help);

  g_free (status_help);
}
