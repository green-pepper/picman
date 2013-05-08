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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmandisplayconfig.h"
#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanguide.h"
#include "core/picmanimage.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-pick-layer.h"
#include "core/picmanlayer.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayermask.h"
#include "core/picmanlayer-floating-sel.h"
#include "core/picmanundostack.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasitem.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmandisplayshell-selection.h"
#include "display/picmandisplayshell-transform.h"

#include "picmaneditselectiontool.h"
#include "picmanmoveoptions.h"
#include "picmanmovetool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define GUIDE_POSITION_INVALID G_MININT

#define SWAP_ORIENT(orient) ((orient) == PICMAN_ORIENTATION_HORIZONTAL ? \
                             PICMAN_ORIENTATION_VERTICAL : \
                             PICMAN_ORIENTATION_HORIZONTAL)


/*  local function prototypes  */

static void   picman_move_tool_button_press   (PicmanTool              *tool,
                                             const PicmanCoords      *coords,
                                             guint32                time,
                                             GdkModifierType        state,
                                             PicmanButtonPressType    press_type,
                                             PicmanDisplay           *display);
static void   picman_move_tool_button_release (PicmanTool              *tool,
                                             const PicmanCoords      *coords,
                                             guint32                time,
                                             GdkModifierType        state,
                                             PicmanButtonReleaseType  release_type,
                                             PicmanDisplay           *display);
static void   picman_move_tool_motion         (PicmanTool              *tool,
                                             const PicmanCoords      *coords,
                                             guint32                time,
                                             GdkModifierType        state,
                                             PicmanDisplay           *display);
static gboolean picman_move_tool_key_press    (PicmanTool              *tool,
                                             GdkEventKey           *kevent,
                                             PicmanDisplay           *display);
static void   picman_move_tool_modifier_key   (PicmanTool              *tool,
                                             GdkModifierType        key,
                                             gboolean               press,
                                             GdkModifierType        state,
                                             PicmanDisplay           *display);
static void   picman_move_tool_oper_update    (PicmanTool              *tool,
                                             const PicmanCoords      *coords,
                                             GdkModifierType        state,
                                             gboolean               proximity,
                                             PicmanDisplay           *display);
static void   picman_move_tool_cursor_update  (PicmanTool              *tool,
                                             const PicmanCoords      *coords,
                                             GdkModifierType        state,
                                             PicmanDisplay           *display);

static void   picman_move_tool_draw           (PicmanDrawTool          *draw_tool);

static void   picman_move_tool_start_guide    (PicmanMoveTool          *move,
                                             PicmanDisplay           *display,
                                             PicmanOrientationType    orientation);


G_DEFINE_TYPE (PicmanMoveTool, picman_move_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_move_tool_parent_class


void
picman_move_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_MOVE_TOOL,
                PICMAN_TYPE_MOVE_OPTIONS,
                picman_move_options_gui,
                0,
                "picman-move-tool",
                C_("tool", "Move"),
                _("Move Tool: Move layers, selections, and other objects"),
                N_("_Move"), "M",
                NULL, PICMAN_HELP_TOOL_MOVE,
                PICMAN_STOCK_TOOL_MOVE,
                data);
}

static void
picman_move_tool_class_init (PicmanMoveToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->button_press   = picman_move_tool_button_press;
  tool_class->button_release = picman_move_tool_button_release;
  tool_class->motion         = picman_move_tool_motion;
  tool_class->key_press      = picman_move_tool_key_press;
  tool_class->modifier_key   = picman_move_tool_modifier_key;
  tool_class->oper_update    = picman_move_tool_oper_update;
  tool_class->cursor_update  = picman_move_tool_cursor_update;

  draw_tool_class->draw      = picman_move_tool_draw;
}

static void
picman_move_tool_init (PicmanMoveTool *move_tool)
{
  PicmanTool *tool = PICMAN_TOOL (move_tool);

  picman_tool_control_set_motion_mode        (tool->control,
                                            PICMAN_MOTION_MODE_COMPRESS);
  picman_tool_control_set_snap_to            (tool->control, FALSE);
  picman_tool_control_set_handle_empty_image (tool->control, TRUE);
  picman_tool_control_set_tool_cursor        (tool->control,
                                            PICMAN_TOOL_CURSOR_MOVE);

  move_tool->floating_layer     = NULL;
  move_tool->guide              = NULL;

  move_tool->moving_guide       = FALSE;
  move_tool->guide_position     = GUIDE_POSITION_INVALID;
  move_tool->guide_orientation  = PICMAN_ORIENTATION_UNKNOWN;

  move_tool->saved_type         = PICMAN_TRANSFORM_TYPE_LAYER;

  move_tool->old_active_layer   = NULL;
  move_tool->old_active_vectors = NULL;
}

static void
picman_move_tool_button_press (PicmanTool            *tool,
                             const PicmanCoords    *coords,
                             guint32              time,
                             GdkModifierType      state,
                             PicmanButtonPressType  press_type,
                             PicmanDisplay         *display)
{
  PicmanMoveTool     *move           = PICMAN_MOVE_TOOL (tool);
  PicmanMoveOptions  *options        = PICMAN_MOVE_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell *shell          = picman_display_get_shell (display);
  PicmanImage        *image          = picman_display_get_image (display);
  PicmanItem         *active_item    = NULL;
  const gchar      *null_message   = NULL;
  const gchar      *locked_message = NULL;

  tool->display = display;

  move->floating_layer     = NULL;
  move->guide              = NULL;
  move->moving_guide       = FALSE;
  move->old_active_layer   = NULL;
  move->old_active_vectors = NULL;

  if (! options->move_current)
    {
      if (options->move_type == PICMAN_TRANSFORM_TYPE_PATH)
        {
          PicmanVectors *vectors;

          if (picman_draw_tool_on_vectors (PICMAN_DRAW_TOOL (tool), display,
                                         coords, 7, 7,
                                         NULL, NULL, NULL, NULL, NULL,
                                         &vectors))
            {
              move->old_active_vectors =
                picman_image_get_active_vectors (image);

              picman_image_set_active_vectors (image, vectors);
            }
          else
            {
              /*  no path picked  */
              return;
            }
        }
      else if (options->move_type == PICMAN_TRANSFORM_TYPE_LAYER)
        {
          PicmanGuide  *guide;
          PicmanLayer  *layer;
          const gint  snap_distance = display->config->snap_distance;

          if (picman_display_shell_get_show_guides (shell) &&
              (guide = picman_image_find_guide (image,
                                              coords->x, coords->y,
                                              FUNSCALEX (shell, snap_distance),
                                              FUNSCALEY (shell, snap_distance))))
            {
              move->guide             = guide;
              move->moving_guide      = TRUE;
              move->guide_position    = picman_guide_get_position (guide);
              move->guide_orientation = picman_guide_get_orientation (guide);

              picman_tool_control_set_scroll_lock (tool->control, TRUE);
              picman_tool_control_set_precision   (tool->control,
                                                 PICMAN_CURSOR_PRECISION_PIXEL_BORDER);

              picman_tool_control_activate (tool->control);

              picman_display_shell_selection_pause (shell);

              if (! picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tool)))
                picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

              picman_tool_push_status_length (tool, display,
                                            _("Move Guide: "),
                                            SWAP_ORIENT (move->guide_orientation),
                                            move->guide_position,
                                            NULL);

              return;
            }
          else if ((layer = picman_image_pick_layer (image,
                                                   coords->x,
                                                   coords->y)))
            {
              if (picman_image_get_floating_selection (image) &&
                  ! picman_layer_is_floating_sel (layer))
                {
                  /*  If there is a floating selection, and this aint it,
                   *  use the move tool to anchor it.
                   */
                  move->floating_layer =
                    picman_image_get_floating_selection (image);

                  picman_tool_control_activate (tool->control);

                  return;
                }
              else
                {
                  move->old_active_layer = picman_image_get_active_layer (image);

                  picman_image_set_active_layer (image, layer);
                }
            }
          else
            {
              /*  no guide and no layer picked  */

              return;
            }
        }
    }

  switch (options->move_type)
    {
    case PICMAN_TRANSFORM_TYPE_PATH:
      {
        active_item    = PICMAN_ITEM (picman_image_get_active_vectors (image));
        null_message   = _("There is no path to move.");
        locked_message = _("The active path's position is locked.");

        if (active_item && ! picman_item_is_position_locked (active_item))
          {
            picman_tool_control_activate (tool->control);
            picman_edit_selection_tool_start (tool, display, coords,
                                            PICMAN_TRANSLATE_MODE_VECTORS,
                                            TRUE);
            return;
          }
      }
      break;

    case PICMAN_TRANSFORM_TYPE_SELECTION:
      {
        active_item    = PICMAN_ITEM (picman_image_get_mask (image));
        /* cannot happen, so don't translate these messages */
        null_message   = "There is no selection to move.";
        locked_message = "The selection's position is locked.";

        if (active_item && ! picman_item_is_position_locked (active_item))
          {
            if (! picman_channel_is_empty (picman_image_get_mask (image)))
              {
                picman_tool_control_activate (tool->control);
                picman_edit_selection_tool_start (tool, display, coords,
                                                PICMAN_TRANSLATE_MODE_MASK,
                                                TRUE);
                return;
              }
            else
              locked_message = _("The selection is empty.");
          }
      }
      break;

    case PICMAN_TRANSFORM_TYPE_LAYER:
      {
        active_item  = PICMAN_ITEM (picman_image_get_active_drawable (image));
        null_message = _("There is no layer to move.");

        if (PICMAN_IS_LAYER_MASK (active_item))
          {
            locked_message = _("The active layer's position is locked.");

            if (! picman_item_is_position_locked (active_item))
              {
                picman_tool_control_activate (tool->control);
                picman_edit_selection_tool_start (tool, display, coords,
                                                PICMAN_TRANSLATE_MODE_LAYER_MASK,
                                                TRUE);
                return;
              }
          }
        else if (PICMAN_IS_CHANNEL (active_item))
          {
            locked_message = _("The active channel's position is locked.");

            if (! picman_item_is_position_locked (active_item))
              {
                picman_tool_control_activate (tool->control);
                picman_edit_selection_tool_start (tool, display, coords,
                                                PICMAN_TRANSLATE_MODE_CHANNEL,
                                                TRUE);
                return;
              }
          }
        else if (PICMAN_IS_LAYER (active_item))
          {
            locked_message = _("The active layer's position is locked.");

            if (! picman_item_is_position_locked (active_item))
              {
                picman_tool_control_activate (tool->control);
                picman_edit_selection_tool_start (tool, display, coords,
                                                PICMAN_TRANSLATE_MODE_LAYER,
                                                TRUE);
                return;
              }
          }
      }
      break;
    }

  if (! active_item)
    {
      picman_tool_message_literal (tool, display, null_message);
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
    }
  else
    {
      picman_tool_message_literal (tool, display, locked_message);
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
    }
}

static void
picman_move_tool_button_release (PicmanTool              *tool,
                               const PicmanCoords      *coords,
                               guint32                time,
                               GdkModifierType        state,
                               PicmanButtonReleaseType  release_type,
                               PicmanDisplay           *display)
{
  PicmanMoveTool     *move   = PICMAN_MOVE_TOOL (tool);
  PicmanGuiConfig    *config = PICMAN_GUI_CONFIG (display->picman->config);
  PicmanDisplayShell *shell  = picman_display_get_shell (display);
  PicmanImage        *image  = picman_display_get_image (display);

  picman_tool_control_halt (tool->control);

  if (move->moving_guide)
    {
      gboolean delete_guide = FALSE;
      gint     width  = picman_image_get_width  (image);
      gint     height = picman_image_get_height (image);

      picman_tool_pop_status (tool, display);

      picman_tool_control_set_scroll_lock (tool->control, FALSE);
      picman_tool_control_set_precision   (tool->control,
                                         PICMAN_CURSOR_PRECISION_PIXEL_CENTER);

      picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

      if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
        {
          move->moving_guide      = FALSE;
          move->guide_position    = GUIDE_POSITION_INVALID;
          move->guide_orientation = PICMAN_ORIENTATION_UNKNOWN;

          picman_display_shell_selection_resume (shell);
          return;
        }

      switch (move->guide_orientation)
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          if (move->guide_position == GUIDE_POSITION_INVALID ||
              move->guide_position <  0                      ||
              move->guide_position >= height)
            delete_guide = TRUE;
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          if (move->guide_position == GUIDE_POSITION_INVALID ||
              move->guide_position <  0                      ||
              move->guide_position >= width)
            delete_guide = TRUE;
          break;

        default:
          break;
        }

      if (delete_guide)
        {
          if (move->guide)
            {
              picman_image_remove_guide (image, move->guide, TRUE);
              move->guide = NULL;
            }
        }
      else
        {
          if (move->guide)
            {
              picman_image_move_guide (image, move->guide,
                                     move->guide_position, TRUE);
            }
          else
            {
              switch (move->guide_orientation)
                {
                case PICMAN_ORIENTATION_HORIZONTAL:
                  move->guide = picman_image_add_hguide (image,
                                                       move->guide_position,
                                                       TRUE);
                  break;

                case PICMAN_ORIENTATION_VERTICAL:
                  move->guide = picman_image_add_vguide (image,
                                                       move->guide_position,
                                                       TRUE);
                  break;

                default:
                  g_assert_not_reached ();
                }
            }
        }

      picman_display_shell_selection_resume (shell);
      picman_image_flush (image);

      move->moving_guide      = FALSE;
      move->guide_position    = GUIDE_POSITION_INVALID;
      move->guide_orientation = PICMAN_ORIENTATION_UNKNOWN;

      if (move->guide)
        picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
    }
  else
    {
      gboolean flush = FALSE;

      if (! config->move_tool_changes_active ||
          (release_type == PICMAN_BUTTON_RELEASE_CANCEL))
        {
          if (move->old_active_layer)
            {
              picman_image_set_active_layer (image, move->old_active_layer);
              move->old_active_layer = NULL;

              flush = TRUE;
            }

          if (move->old_active_vectors)
            {
              picman_image_set_active_vectors (image, move->old_active_vectors);
              move->old_active_vectors = NULL;

              flush = TRUE;
            }
        }

      if (release_type != PICMAN_BUTTON_RELEASE_CANCEL)
        {
          if (move->floating_layer)
            {
              floating_sel_anchor (move->floating_layer);

              flush = TRUE;
            }
        }

      if (flush)
        picman_image_flush (image);
    }
}

static void
picman_move_tool_motion (PicmanTool         *tool,
                       const PicmanCoords *coords,
                       guint32           time,
                       GdkModifierType   state,
                       PicmanDisplay      *display)

{
  PicmanMoveTool     *move  = PICMAN_MOVE_TOOL (tool);
  PicmanDisplayShell *shell = picman_display_get_shell (display);

  if (move->moving_guide)
    {
      gint      tx, ty;
      gboolean  delete_guide = FALSE;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      picman_display_shell_transform_xy (shell,
                                       coords->x, coords->y,
                                       &tx, &ty);

      if (tx < 0 || tx >= shell->disp_width ||
          ty < 0 || ty >= shell->disp_height)
        {
          move->guide_position = GUIDE_POSITION_INVALID;

          delete_guide = TRUE;
        }
      else
        {
          PicmanImage *image  = picman_display_get_image (display);
          gint       width  = picman_image_get_width  (image);
          gint       height = picman_image_get_height (image);

          if (move->guide_orientation == PICMAN_ORIENTATION_HORIZONTAL)
            move->guide_position = RINT (coords->y);
          else
            move->guide_position = RINT (coords->x);

          switch (move->guide_orientation)
            {
            case PICMAN_ORIENTATION_HORIZONTAL:
              if (move->guide_position <  0 ||
                  move->guide_position >= height)
                delete_guide = TRUE;
              break;

            case PICMAN_ORIENTATION_VERTICAL:
              if (move->guide_position <  0 ||
                  move->guide_position >= width)
                delete_guide = TRUE;
              break;

            default:
              break;
            }
        }

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

      picman_tool_pop_status (tool, display);

      if (delete_guide)
        {
          picman_tool_push_status (tool, display,
                                 move->guide ?
                                 _("Remove Guide") : _("Cancel Guide"));
        }
      else
        {
          picman_tool_push_status_length (tool, display,
                                        move->guide ?
                                        _("Move Guide: ") : _("Add Guide: "),
                                        SWAP_ORIENT (move->guide_orientation),
                                        move->guide_position,
                                        NULL);
        }
    }
}

static gboolean
picman_move_tool_key_press (PicmanTool    *tool,
                          GdkEventKey *kevent,
                          PicmanDisplay *display)
{
  PicmanMoveOptions *options = PICMAN_MOVE_TOOL_GET_OPTIONS (tool);

  return picman_edit_selection_tool_translate (tool, kevent,
                                             options->move_type,
                                             display);
}

static void
picman_move_tool_modifier_key (PicmanTool        *tool,
                             GdkModifierType  key,
                             gboolean         press,
                             GdkModifierType  state,
                             PicmanDisplay     *display)
{
  PicmanMoveTool    *move    = PICMAN_MOVE_TOOL (tool);
  PicmanMoveOptions *options = PICMAN_MOVE_TOOL_GET_OPTIONS (tool);

  if (key == GDK_SHIFT_MASK)
    {
      g_object_set (options, "move-current", ! options->move_current, NULL);
    }
  else if (key == GDK_MOD1_MASK ||
           key == picman_get_toggle_behavior_mask ())
    {
      PicmanTransformType button_type;

      button_type = options->move_type;

      if (press)
        {
          if (key == (state & (GDK_MOD1_MASK |
                               picman_get_toggle_behavior_mask ())))
            {
              /*  first modifier pressed  */

              move->saved_type = options->move_type;
            }
        }
      else
        {
          if (! (state & (GDK_MOD1_MASK |
                          picman_get_toggle_behavior_mask ())))
            {
              /*  last modifier released  */

              button_type = move->saved_type;
            }
        }

      if (state & GDK_MOD1_MASK)
        {
          button_type = PICMAN_TRANSFORM_TYPE_SELECTION;
        }
      else if (state & picman_get_toggle_behavior_mask ())
        {
          button_type = PICMAN_TRANSFORM_TYPE_PATH;
        }

      if (button_type != options->move_type)
        {
          g_object_set (options, "move-type", button_type, NULL);
        }
    }
}

static void
picman_move_tool_oper_update (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            GdkModifierType   state,
                            gboolean          proximity,
                            PicmanDisplay      *display)
{
  PicmanMoveTool     *move    = PICMAN_MOVE_TOOL (tool);
  PicmanMoveOptions  *options = PICMAN_MOVE_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell *shell   = picman_display_get_shell (display);
  PicmanImage        *image   = picman_display_get_image (display);
  PicmanGuide        *guide   = NULL;

  if (options->move_type == PICMAN_TRANSFORM_TYPE_LAYER &&
      ! options->move_current                         &&
      picman_display_shell_get_show_guides (shell)      &&
      proximity)
    {
      gint snap_distance = display->config->snap_distance;

      guide = picman_image_find_guide (image, coords->x, coords->y,
                                     FUNSCALEX (shell, snap_distance),
                                     FUNSCALEY (shell, snap_distance));
    }

  if (move->guide != guide)
    {
      PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

      picman_draw_tool_pause (draw_tool);

      if (picman_draw_tool_is_active (draw_tool) &&
          draw_tool->display != display)
        picman_draw_tool_stop (draw_tool);

      move->guide = guide;

      if (! picman_draw_tool_is_active (draw_tool))
        picman_draw_tool_start (draw_tool, display);

      picman_draw_tool_resume (draw_tool);
    }
}

static void
picman_move_tool_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  PicmanMoveOptions    *options     = PICMAN_MOVE_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell   *shell       = picman_display_get_shell (display);
  PicmanImage          *image       = picman_display_get_image (display);
  PicmanCursorType      cursor      = PICMAN_CURSOR_MOUSE;
  PicmanToolCursorType  tool_cursor = PICMAN_TOOL_CURSOR_MOVE;
  PicmanCursorModifier  modifier    = PICMAN_CURSOR_MODIFIER_NONE;

  if (options->move_type == PICMAN_TRANSFORM_TYPE_PATH)
    {
      tool_cursor = PICMAN_TOOL_CURSOR_PATHS;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;

      if (options->move_current)
        {
          PicmanItem *item = PICMAN_ITEM (picman_image_get_active_vectors (image));

          if (! item || picman_item_is_position_locked (item))
            modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }
      else
        {
          if (picman_draw_tool_on_vectors (PICMAN_DRAW_TOOL (tool), display,
                                         coords, 7, 7,
                                         NULL, NULL, NULL, NULL, NULL, NULL))
            {
              tool_cursor = PICMAN_TOOL_CURSOR_HAND;
            }
          else
            {
              modifier = PICMAN_CURSOR_MODIFIER_BAD;
            }
        }
    }
  else if (options->move_type == PICMAN_TRANSFORM_TYPE_SELECTION)
    {
      tool_cursor = PICMAN_TOOL_CURSOR_RECT_SELECT;
      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;

      if (picman_channel_is_empty (picman_image_get_mask (image)))
        modifier = PICMAN_CURSOR_MODIFIER_BAD;
    }
  else if (options->move_current)
    {
      PicmanItem *item = PICMAN_ITEM (picman_image_get_active_drawable (image));

      if (! item || picman_item_is_position_locked (item))
        modifier = PICMAN_CURSOR_MODIFIER_BAD;
    }
  else
    {
      PicmanLayer  *layer;
      const gint  snap_distance = display->config->snap_distance;

      if (picman_display_shell_get_show_guides (shell) &&
          picman_image_find_guide (image, coords->x, coords->y,
                                 FUNSCALEX (shell, snap_distance),
                                 FUNSCALEY (shell, snap_distance)))
        {
          tool_cursor = PICMAN_TOOL_CURSOR_HAND;
          modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
        }
      else if ((layer = picman_image_pick_layer (image,
                                               coords->x, coords->y)))
        {
          /*  if there is a floating selection, and this aint it...  */
          if (picman_image_get_floating_selection (image) &&
              ! picman_layer_is_floating_sel (layer))
            {
              tool_cursor = PICMAN_TOOL_CURSOR_MOVE;
              modifier    = PICMAN_CURSOR_MODIFIER_ANCHOR;
            }
          else if (picman_item_is_position_locked (PICMAN_ITEM (layer)))
            {
              modifier = PICMAN_CURSOR_MODIFIER_BAD;
            }
          else if (layer != picman_image_get_active_layer (image))
            {
              tool_cursor = PICMAN_TOOL_CURSOR_HAND;
	      modifier    = PICMAN_CURSOR_MODIFIER_MOVE;
            }
        }
      else
        {
          modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_tool_cursor     (tool->control, tool_cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_move_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanMoveTool *move = PICMAN_MOVE_TOOL (draw_tool);

  if (move->guide)
    {
      PicmanCanvasItem *item;

      item = picman_draw_tool_add_guide (draw_tool,
                                       picman_guide_get_orientation (move->guide),
                                       picman_guide_get_position (move->guide),
                                       TRUE);
      picman_canvas_item_set_highlight (item, TRUE);
    }

  if (move->moving_guide && move->guide_position != GUIDE_POSITION_INVALID)
    {
      picman_draw_tool_add_guide (draw_tool,
                                move->guide_orientation,
                                move->guide_position,
                                FALSE);
    }
}

void
picman_move_tool_start_hguide (PicmanTool    *tool,
                             PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_MOVE_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  picman_move_tool_start_guide (PICMAN_MOVE_TOOL (tool), display,
                              PICMAN_ORIENTATION_HORIZONTAL);
}

void
picman_move_tool_start_vguide (PicmanTool    *tool,
                             PicmanDisplay *display)
{
  g_return_if_fail (PICMAN_IS_MOVE_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  picman_move_tool_start_guide (PICMAN_MOVE_TOOL (tool), display,
                              PICMAN_ORIENTATION_VERTICAL);
}

static void
picman_move_tool_start_guide (PicmanMoveTool        *move,
                            PicmanDisplay         *display,
                            PicmanOrientationType  orientation)
{
  PicmanTool *tool = PICMAN_TOOL (move);

  picman_display_shell_selection_pause (picman_display_get_shell (display));

  tool->display = display;
  picman_tool_control_activate (tool->control);
  picman_tool_control_set_scroll_lock (tool->control, TRUE);

  if (picman_draw_tool_is_active  (PICMAN_DRAW_TOOL (tool)))
    picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  move->guide             = NULL;
  move->moving_guide      = TRUE;
  move->guide_position    = GUIDE_POSITION_INVALID;
  move->guide_orientation = orientation;

  picman_tool_set_cursor (tool, display,
                        PICMAN_CURSOR_MOUSE,
                        PICMAN_TOOL_CURSOR_HAND,
                        PICMAN_CURSOR_MODIFIER_MOVE);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (move), display);
}
