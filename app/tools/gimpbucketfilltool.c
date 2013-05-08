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

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picman-edit.h"
#include "core/picmandrawable-bucket-fill.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanitem.h"
#include "core/picmanpickable.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"

#include "picmanbucketfilloptions.h"
#include "picmanbucketfilltool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean picman_bucket_fill_tool_initialize   (PicmanTool              *tool,
                                                    PicmanDisplay           *display,
                                                    GError               **error);
static void   picman_bucket_fill_tool_button_release (PicmanTool              *tool,
                                                    const PicmanCoords      *coords,
                                                    guint32                time,
                                                    GdkModifierType        state,
                                                    PicmanButtonReleaseType  release_type,
                                                    PicmanDisplay           *display);
static void   picman_bucket_fill_tool_modifier_key   (PicmanTool              *tool,
                                                    GdkModifierType        key,
                                                    gboolean               press,
                                                    GdkModifierType        state,
                                                    PicmanDisplay           *display);
static void   picman_bucket_fill_tool_cursor_update  (PicmanTool              *tool,
                                                    const PicmanCoords      *coords,
                                                    GdkModifierType        state,
                                                    PicmanDisplay           *display);


G_DEFINE_TYPE (PicmanBucketFillTool, picman_bucket_fill_tool, PICMAN_TYPE_TOOL)

#define parent_class picman_bucket_fill_tool_parent_class


void
picman_bucket_fill_tool_register (PicmanToolRegisterCallback  callback,
                                gpointer                  data)
{
  (* callback) (PICMAN_TYPE_BUCKET_FILL_TOOL,
                PICMAN_TYPE_BUCKET_FILL_OPTIONS,
                picman_bucket_fill_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK |
                PICMAN_CONTEXT_BACKGROUND_MASK |
                PICMAN_CONTEXT_OPACITY_MASK    |
                PICMAN_CONTEXT_PAINT_MODE_MASK |
                PICMAN_CONTEXT_PATTERN_MASK,
                "picman-bucket-fill-tool",
                _("Bucket Fill"),
                _("Bucket Fill Tool: Fill selected area with a color or pattern"),
                N_("_Bucket Fill"), "<shift>B",
                NULL, PICMAN_HELP_TOOL_BUCKET_FILL,
                PICMAN_STOCK_TOOL_BUCKET_FILL,
                data);
}

static void
picman_bucket_fill_tool_class_init (PicmanBucketFillToolClass *klass)
{
  PicmanToolClass *tool_class = PICMAN_TOOL_CLASS (klass);

  tool_class->initialize     = picman_bucket_fill_tool_initialize;
  tool_class->button_release = picman_bucket_fill_tool_button_release;
  tool_class->modifier_key   = picman_bucket_fill_tool_modifier_key;
  tool_class->cursor_update  = picman_bucket_fill_tool_cursor_update;
}

static void
picman_bucket_fill_tool_init (PicmanBucketFillTool *bucket_fill_tool)
{
  PicmanTool *tool = PICMAN_TOOL (bucket_fill_tool);

  picman_tool_control_set_scroll_lock     (tool->control, TRUE);
  picman_tool_control_set_wants_click     (tool->control, TRUE);
  picman_tool_control_set_tool_cursor     (tool->control,
                                         PICMAN_TOOL_CURSOR_BUCKET_FILL);
  picman_tool_control_set_action_value_1  (tool->control,
                                         "context/context-opacity-set");
  picman_tool_control_set_action_object_1 (tool->control,
                                         "context/context-pattern-select-set");
}

static gboolean
picman_bucket_fill_tool_initialize (PicmanTool     *tool,
                                  PicmanDisplay  *display,
                                  GError      **error)
{
  PicmanImage    *image    = picman_display_get_image (display);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);

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

  return TRUE;
}

static void
picman_bucket_fill_tool_button_release (PicmanTool              *tool,
                                      const PicmanCoords      *coords,
                                      guint32                time,
                                      GdkModifierType        state,
                                      PicmanButtonReleaseType  release_type,
                                      PicmanDisplay           *display)
{
  PicmanBucketFillOptions *options = PICMAN_BUCKET_FILL_TOOL_GET_OPTIONS (tool);
  PicmanImage             *image   = picman_display_get_image (display);

  if ((release_type == PICMAN_BUTTON_RELEASE_CLICK ||
       release_type == PICMAN_BUTTON_RELEASE_NO_MOTION) &&
      picman_image_coords_in_active_pickable (image, coords,
                                            options->sample_merged, TRUE))
    {
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);
      PicmanContext  *context  = PICMAN_CONTEXT (options);
      gint          x, y;

      x = coords->x;
      y = coords->y;

      if (! options->sample_merged)
        {
          gint off_x, off_y;

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

          x -= off_x;
          y -= off_y;
        }

      if (options->fill_selection)
        {
          PicmanFillType fill_type;

          switch (options->fill_mode)
            {
            default:
            case PICMAN_FG_BUCKET_FILL:
              fill_type = PICMAN_FOREGROUND_FILL;
              break;
            case PICMAN_BG_BUCKET_FILL:
              fill_type = PICMAN_BACKGROUND_FILL;
              break;
            case PICMAN_PATTERN_BUCKET_FILL:
              fill_type = PICMAN_PATTERN_FILL;
              break;
            }

          picman_edit_fill (image, drawable, context, fill_type,
                          picman_context_get_opacity (context),
                          picman_context_get_paint_mode (context));
          picman_image_flush (image);
        }
      else
        {
          GError *error = NULL;

          if (! picman_drawable_bucket_fill (drawable,
                                           context,
                                           options->fill_mode,
                                           picman_context_get_paint_mode (context),
                                           picman_context_get_opacity (context),
                                           options->fill_transparent,
                                           options->fill_criterion,
                                           options->threshold / 255.0,
                                           options->sample_merged,
                                           x, y, &error))
            {
              picman_message_literal (display->picman, G_OBJECT (display),
                                    PICMAN_MESSAGE_WARNING, error->message);
              g_clear_error (&error);
            }
          else
            {
              picman_image_flush (image);
            }
        }
    }

  PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state,
                                                  release_type, display);

  tool->display  = NULL;
  tool->drawable = NULL;
}

static void
picman_bucket_fill_tool_modifier_key (PicmanTool        *tool,
                                    GdkModifierType  key,
                                    gboolean         press,
                                    GdkModifierType  state,
                                    PicmanDisplay     *display)
{
  PicmanBucketFillOptions *options = PICMAN_BUCKET_FILL_TOOL_GET_OPTIONS (tool);

  if (key == picman_get_toggle_behavior_mask ())
    {
      switch (options->fill_mode)
        {
        case PICMAN_FG_BUCKET_FILL:
          g_object_set (options, "fill-mode", PICMAN_BG_BUCKET_FILL, NULL);
          break;

        case PICMAN_BG_BUCKET_FILL:
          g_object_set (options, "fill-mode", PICMAN_FG_BUCKET_FILL, NULL);
          break;

        default:
          break;
        }
    }
  else if (key == GDK_SHIFT_MASK)
    {
      g_object_set (options, "fill-selection", ! options->fill_selection, NULL);
    }
}

static void
picman_bucket_fill_tool_cursor_update (PicmanTool         *tool,
                                     const PicmanCoords *coords,
                                     GdkModifierType   state,
                                     PicmanDisplay      *display)
{
  PicmanBucketFillOptions *options  = PICMAN_BUCKET_FILL_TOOL_GET_OPTIONS (tool);
  PicmanCursorModifier     modifier = PICMAN_CURSOR_MODIFIER_BAD;
  PicmanImage             *image    = picman_display_get_image (display);

  if (picman_image_coords_in_active_pickable (image, coords,
                                            options->sample_merged, TRUE))
    {
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);

      if (! picman_viewable_get_children (PICMAN_VIEWABLE (drawable)) &&
          ! picman_item_is_content_locked (PICMAN_ITEM (drawable)))
        {
          switch (options->fill_mode)
            {
            case PICMAN_FG_BUCKET_FILL:
              modifier = PICMAN_CURSOR_MODIFIER_FOREGROUND;
              break;

            case PICMAN_BG_BUCKET_FILL:
              modifier = PICMAN_CURSOR_MODIFIER_BACKGROUND;
              break;

            case PICMAN_PATTERN_BUCKET_FILL:
              modifier = PICMAN_CURSOR_MODIFIER_PATTERN;
              break;
            }
        }
    }

  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}
