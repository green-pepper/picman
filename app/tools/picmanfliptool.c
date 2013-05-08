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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmandrawable-transform.h"
#include "core/picmanimage.h"
#include "core/picmanitem-linked.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmanpickable.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"

#include "picmanflipoptions.h"
#include "picmanfliptool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void         picman_flip_tool_modifier_key  (PicmanTool          *tool,
                                                  GdkModifierType    key,
                                                  gboolean           press,
                                                  GdkModifierType    state,
                                                  PicmanDisplay       *display);
static void         picman_flip_tool_cursor_update (PicmanTool          *tool,
                                                  const PicmanCoords  *coords,
                                                  GdkModifierType    state,
                                                  PicmanDisplay       *display);

static gchar      * picman_flip_tool_get_undo_desc (PicmanTransformTool *tool);
static GeglBuffer * picman_flip_tool_transform     (PicmanTransformTool *tool,
                                                  PicmanItem          *item,
                                                  GeglBuffer        *orig_buffer,
                                                  gint               orig_offset_x,
                                                  gint               orig_offset_y,
                                                  gint              *new_offset_x,
                                                  gint              *new_offset_y);


G_DEFINE_TYPE (PicmanFlipTool, picman_flip_tool, PICMAN_TYPE_TRANSFORM_TOOL)

#define parent_class picman_flip_tool_parent_class


void
picman_flip_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_FLIP_TOOL,
                PICMAN_TYPE_FLIP_OPTIONS,
                picman_flip_options_gui,
                PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-flip-tool",
                _("Flip"),
                _("Flip Tool: "
                  "Reverse the layer, selection or path horizontally or vertically"),
                N_("_Flip"), "<shift>F",
                NULL, PICMAN_HELP_TOOL_FLIP,
                PICMAN_STOCK_TOOL_FLIP,
                data);
}

static void
picman_flip_tool_class_init (PicmanFlipToolClass *klass)
{
  PicmanToolClass          *tool_class  = PICMAN_TOOL_CLASS (klass);
  PicmanTransformToolClass *trans_class = PICMAN_TRANSFORM_TOOL_CLASS (klass);

  tool_class->modifier_key   = picman_flip_tool_modifier_key;
  tool_class->cursor_update  = picman_flip_tool_cursor_update;

  trans_class->get_undo_desc = picman_flip_tool_get_undo_desc;
  trans_class->transform     = picman_flip_tool_transform;
}

static void
picman_flip_tool_init (PicmanFlipTool *flip_tool)
{
  PicmanTool *tool = PICMAN_TOOL (flip_tool);

  picman_tool_control_set_snap_to            (tool->control, FALSE);
  picman_tool_control_set_precision          (tool->control,
                                            PICMAN_CURSOR_PRECISION_PIXEL_CENTER);
  picman_tool_control_set_cursor             (tool->control, PICMAN_CURSOR_MOUSE);
  picman_tool_control_set_toggle_cursor      (tool->control, PICMAN_CURSOR_MOUSE);
  picman_tool_control_set_tool_cursor        (tool->control,
                                            PICMAN_TOOL_CURSOR_FLIP_HORIZONTAL);
  picman_tool_control_set_toggle_tool_cursor (tool->control,
                                            PICMAN_TOOL_CURSOR_FLIP_VERTICAL);
}

static void
picman_flip_tool_modifier_key (PicmanTool        *tool,
                             GdkModifierType  key,
                             gboolean         press,
                             GdkModifierType  state,
                             PicmanDisplay     *display)
{
  PicmanFlipOptions *options = PICMAN_FLIP_TOOL_GET_OPTIONS (tool);

  if (key == picman_get_toggle_behavior_mask ())
    {
      switch (options->flip_type)
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          g_object_set (options,
                        "flip-type", PICMAN_ORIENTATION_VERTICAL,
                        NULL);
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          g_object_set (options,
                        "flip-type", PICMAN_ORIENTATION_HORIZONTAL,
                        NULL);
          break;

        default:
          break;
        }
    }
}

static void
picman_flip_tool_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  PicmanFlipOptions    *options  = PICMAN_FLIP_TOOL_GET_OPTIONS (tool);
  PicmanCursorModifier  modifier = PICMAN_CURSOR_MODIFIER_BAD;
  PicmanImage          *image    = picman_display_get_image (display);

  if (picman_image_coords_in_active_pickable (image, coords,
                                            FALSE, TRUE))
    {
      modifier = PICMAN_CURSOR_MODIFIER_NONE;
    }

  picman_tool_control_set_cursor_modifier        (tool->control, modifier);
  picman_tool_control_set_toggle_cursor_modifier (tool->control, modifier);

  picman_tool_control_set_toggled (tool->control,
                                 options->flip_type ==
                                 PICMAN_ORIENTATION_VERTICAL);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static gchar *
picman_flip_tool_get_undo_desc (PicmanTransformTool *tr_tool)
{
  PicmanFlipOptions *options = PICMAN_FLIP_TOOL_GET_OPTIONS (tr_tool);

  switch (options->flip_type)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      return g_strdup (C_("undo-type", "Flip horizontally"));

    case PICMAN_ORIENTATION_VERTICAL:
      return g_strdup (C_("undo-type", "Flip vertically"));

    default:
      /* probably this is not actually reached today, but
       * could be if someone defined FLIP_DIAGONAL, say...
       */
      return g_strdup (C_("undo-desc", "Flip"));
    }
}

static GeglBuffer *
picman_flip_tool_transform (PicmanTransformTool *trans_tool,
                          PicmanItem          *active_item,
                          GeglBuffer        *orig_buffer,
                          gint               orig_offset_x,
                          gint               orig_offset_y,
                          gint              *new_offset_x,
                          gint              *new_offset_y)
{
  PicmanFlipOptions *options = PICMAN_FLIP_TOOL_GET_OPTIONS (trans_tool);
  PicmanContext     *context = PICMAN_CONTEXT (options);
  gdouble          axis    = 0.0;
  GeglBuffer      *ret     = NULL;

  switch (options->flip_type)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      axis = ((gdouble) trans_tool->x1 +
              (gdouble) (trans_tool->x2 - trans_tool->x1) / 2.0);
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      axis = ((gdouble) trans_tool->y1 +
              (gdouble) (trans_tool->y2 - trans_tool->y1) / 2.0);
      break;

    default:
      break;
    }

  if (picman_item_get_linked (active_item))
    picman_item_linked_flip (active_item, context, options->flip_type, axis,
                           FALSE);

  if (orig_buffer)
    {
      /*  this happens when transforming a selection cut out of a
       *  normal drawable, or the selection
       */

      ret = picman_drawable_transform_buffer_flip (PICMAN_DRAWABLE (active_item),
                                                 context,
                                                 orig_buffer,
                                                 orig_offset_x,
                                                 orig_offset_y,
                                                 options->flip_type, axis,
                                                 FALSE,
                                                 new_offset_x,
                                                 new_offset_y);
    }
  else
    {
      /*  this happens for entire drawables, paths and layer groups  */

      picman_item_flip (active_item, context, options->flip_type, axis, FALSE);
    }

  return ret;
}
