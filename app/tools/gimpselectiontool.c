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

#include "core/picmanchannel.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-layer.h"
#include "core/picmanpickable.h"

#include "display/picmandisplay.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmaneditselectiontool.h"
#include "picmanselectiontool.h"
#include "picmanselectionoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_selection_tool_modifier_key  (PicmanTool         *tool,
                                                 GdkModifierType   key,
                                                 gboolean          press,
                                                 GdkModifierType   state,
                                                 PicmanDisplay      *display);
static void   picman_selection_tool_oper_update   (PicmanTool         *tool,
                                                 const PicmanCoords *coords,
                                                 GdkModifierType   state,
                                                 gboolean          proximity,
                                                 PicmanDisplay      *display);
static void   picman_selection_tool_cursor_update (PicmanTool         *tool,
                                                 const PicmanCoords *coords,
                                                 GdkModifierType   state,
                                                 PicmanDisplay      *display);


G_DEFINE_TYPE (PicmanSelectionTool, picman_selection_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_selection_tool_parent_class


static void
picman_selection_tool_class_init (PicmanSelectionToolClass *klass)
{
  PicmanToolClass *tool_class = PICMAN_TOOL_CLASS (klass);

  tool_class->modifier_key  = picman_selection_tool_modifier_key;
  tool_class->key_press     = picman_edit_selection_tool_key_press;
  tool_class->oper_update   = picman_selection_tool_oper_update;
  tool_class->cursor_update = picman_selection_tool_cursor_update;
}

static void
picman_selection_tool_init (PicmanSelectionTool *selection_tool)
{
  selection_tool->function        = SELECTION_SELECT;
  selection_tool->saved_operation = PICMAN_CHANNEL_OP_REPLACE;

  selection_tool->allow_move      = TRUE;
}

static void
picman_selection_tool_modifier_key (PicmanTool        *tool,
                                  GdkModifierType  key,
                                  gboolean         press,
                                  GdkModifierType  state,
                                  PicmanDisplay     *display)
{
  PicmanSelectionTool    *selection_tool = PICMAN_SELECTION_TOOL (tool);
  PicmanSelectionOptions *options        = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  GdkModifierType       extend_mask;
  GdkModifierType       modify_mask;

  extend_mask = picman_get_extend_selection_mask ();
  modify_mask = picman_get_modify_selection_mask ();

  if (key == extend_mask ||
      key == modify_mask ||
      key == GDK_MOD1_MASK)
    {
      PicmanChannelOps button_op = options->operation;

      if (press)
        {
          if (key == (state & (extend_mask |
                               modify_mask |
                               GDK_MOD1_MASK)))
            {
              /*  first modifier pressed  */

              selection_tool->saved_operation = options->operation;
            }
        }
      else
        {
          if (! (state & (extend_mask |
                          modify_mask |
                          GDK_MOD1_MASK)))
            {
              /*  last modifier released  */

              button_op = selection_tool->saved_operation;
            }
        }

      if (state & GDK_MOD1_MASK)
        {
          /*  if alt is down, pretend that neither
           *  shift nor control are down
           */
          button_op = selection_tool->saved_operation;
        }
      else if (state & (extend_mask |
                        modify_mask))
        {
          /*  else get the operation from the modifier state, but only
           *  if there is actually a modifier pressed, so we don't
           *  override the "last modifier released" assignment above
           */
          button_op = picman_modifiers_to_channel_op (state);
        }

      if (button_op != options->operation)
        {
          g_object_set (options, "operation", button_op, NULL);
        }
    }
}

static void
picman_selection_tool_oper_update (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 gboolean          proximity,
                                 PicmanDisplay      *display)
{
  PicmanSelectionTool    *selection_tool = PICMAN_SELECTION_TOOL (tool);
  PicmanSelectionOptions *options        = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  PicmanImage            *image;
  PicmanChannel          *selection;
  PicmanDrawable         *drawable;
  PicmanLayer            *layer;
  PicmanLayer            *floating_sel;
  GdkModifierType       extend_mask;
  GdkModifierType       modify_mask;
  gboolean              move_layer        = FALSE;
  gboolean              move_floating_sel = FALSE;
  gboolean              selection_empty;

  image        = picman_display_get_image (display);
  selection    = picman_image_get_mask (image);
  drawable     = picman_image_get_active_drawable (image);
  layer        = picman_image_pick_layer (image, coords->x, coords->y);
  floating_sel = picman_image_get_floating_selection (image);

  extend_mask = picman_get_extend_selection_mask ();
  modify_mask = picman_get_modify_selection_mask ();

  if (drawable)
    {
      if (floating_sel)
        {
          if (layer == floating_sel)
            move_floating_sel = TRUE;
        }
      else if (picman_item_mask_intersect (PICMAN_ITEM (drawable),
                                         NULL, NULL, NULL, NULL))
        {
          move_layer = TRUE;
        }
    }

  selection_empty = picman_channel_is_empty (selection);

  selection_tool->function = SELECTION_SELECT;

  if (selection_tool->allow_move &&
      (state & GDK_MOD1_MASK) && (state & modify_mask) && move_layer)
    {
      /* move the selection */
      selection_tool->function = SELECTION_MOVE;
    }
  else if (selection_tool->allow_move &&
           (state & GDK_MOD1_MASK) && (state & extend_mask) && move_layer)
    {
      /* move a copy of the selection */
      selection_tool->function = SELECTION_MOVE_COPY;
    }
  else if (selection_tool->allow_move &&
           (state & GDK_MOD1_MASK) && ! selection_empty)
    {
      /* move the selection mask */
      selection_tool->function = SELECTION_MOVE_MASK;
    }
  else if (selection_tool->allow_move &&
           ! (state & (extend_mask | modify_mask)) &&
           move_floating_sel)
    {
      /* move the selection */
      selection_tool->function = SELECTION_MOVE;
    }
  else if ((state & modify_mask) || (state & extend_mask))
    {
      /* select */
      selection_tool->function = SELECTION_SELECT;
    }
  else if (floating_sel)
    {
      /* anchor the selection */
      selection_tool->function = SELECTION_ANCHOR;
    }

  picman_tool_pop_status (tool, display);

  if (proximity)
    {
      const gchar     *status      = NULL;
      gboolean         free_status = FALSE;
      GdkModifierType  modifiers   = (extend_mask | modify_mask);

      if (! selection_empty)
        modifiers |= GDK_MOD1_MASK;

      switch (selection_tool->function)
        {
        case SELECTION_SELECT:
          switch (options->operation)
            {
            case PICMAN_CHANNEL_OP_REPLACE:
              if (! selection_empty)
                {
                  status = picman_suggest_modifiers (_("Click-Drag to replace the "
                                                     "current selection"),
                                                   modifiers & ~state,
                                                   NULL, NULL, NULL);
                  free_status = TRUE;
                }
              else
                {
                  status = _("Click-Drag to create a new selection");
                }
              break;

            case PICMAN_CHANNEL_OP_ADD:
              status = picman_suggest_modifiers (_("Click-Drag to add to the "
                                                 "current selection"),
                                               modifiers
                                               & ~(state | extend_mask),
                                               NULL, NULL, NULL);
              free_status = TRUE;
              break;

            case PICMAN_CHANNEL_OP_SUBTRACT:
              status = picman_suggest_modifiers (_("Click-Drag to subtract from the "
                                                 "current selection"),
                                               modifiers
                                               & ~(state | modify_mask),
                                               NULL, NULL, NULL);
              free_status = TRUE;
              break;

            case PICMAN_CHANNEL_OP_INTERSECT:
              status = picman_suggest_modifiers (_("Click-Drag to intersect with "
                                                 "the current selection"),
                                               modifiers & ~state,
                                               NULL, NULL, NULL);
              free_status = TRUE;
              break;
            }
          break;

        case SELECTION_MOVE_MASK:
          status = picman_suggest_modifiers (_("Click-Drag to move the "
                                             "selection mask"),
                                           modifiers & ~state,
                                           NULL, NULL, NULL);
          free_status = TRUE;
          break;

        case SELECTION_MOVE:
          status = _("Click-Drag to move the selected pixels");
          break;

        case SELECTION_MOVE_COPY:
          status = _("Click-Drag to move a copy of the selected pixels");
          break;

        case SELECTION_ANCHOR:
          status = _("Click to anchor the floating selection");
          break;

        default:
          g_return_if_reached ();
        }

      if (status)
        picman_tool_push_status (tool, display, "%s", status);

      if (free_status)
        g_free ((gchar *) status);
    }
}

static void
picman_selection_tool_cursor_update (PicmanTool         *tool,
                                   const PicmanCoords *coords,
                                   GdkModifierType   state,
                                   PicmanDisplay      *display)
{
  PicmanSelectionTool    *selection_tool = PICMAN_SELECTION_TOOL (tool);
  PicmanSelectionOptions *options;
  PicmanToolCursorType    tool_cursor;
  PicmanCursorModifier    modifier;

  options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);

  tool_cursor = picman_tool_control_get_tool_cursor (tool->control);
  modifier    = PICMAN_CURSOR_MODIFIER_NONE;

  switch (selection_tool->function)
    {
    case SELECTION_SELECT:
      switch (options->operation)
        {
        case PICMAN_CHANNEL_OP_REPLACE:
          break;
        case PICMAN_CHANNEL_OP_ADD:
          modifier = PICMAN_CURSOR_MODIFIER_PLUS;
          break;
        case PICMAN_CHANNEL_OP_SUBTRACT:
          modifier = PICMAN_CURSOR_MODIFIER_MINUS;
          break;
        case PICMAN_CHANNEL_OP_INTERSECT:
          modifier = PICMAN_CURSOR_MODIFIER_INTERSECT;
          break;
        }
      break;

    case SELECTION_MOVE_MASK:
      modifier = PICMAN_CURSOR_MODIFIER_MOVE;
      break;

    case SELECTION_MOVE:
    case SELECTION_MOVE_COPY:
      tool_cursor = PICMAN_TOOL_CURSOR_MOVE;
      break;

    case SELECTION_ANCHOR:
      modifier = PICMAN_CURSOR_MODIFIER_ANCHOR;
      break;
    }

  /*  we don't set the bad modifier ourselves, so a subclass has set
   *  it, always leave it there since it's more important than what we
   *  have to say.
   */
  if (picman_tool_control_get_cursor_modifier (tool->control) ==
      PICMAN_CURSOR_MODIFIER_BAD)
    {
      modifier = PICMAN_CURSOR_MODIFIER_BAD;
    }

  picman_tool_set_cursor (tool, display,
                        picman_tool_control_get_cursor (tool->control),
                        tool_cursor,
                        modifier);
}


/*  public functions  */

gboolean
picman_selection_tool_start_edit (PicmanSelectionTool *sel_tool,
                                PicmanDisplay       *display,
                                const PicmanCoords  *coords)
{
  PicmanTool *tool;

  g_return_val_if_fail (PICMAN_IS_SELECTION_TOOL (sel_tool), FALSE);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), FALSE);
  g_return_val_if_fail (coords != NULL, FALSE);

  tool = PICMAN_TOOL (sel_tool);

  g_return_val_if_fail (picman_tool_control_is_active (tool->control) == FALSE,
                        FALSE);

  switch (sel_tool->function)
    {
    case SELECTION_MOVE_MASK:
      picman_edit_selection_tool_start (tool, display, coords,
                                      PICMAN_TRANSLATE_MODE_MASK, FALSE);
      return TRUE;

    case SELECTION_MOVE:
    case SELECTION_MOVE_COPY:
      {
        PicmanImage    *image    = picman_display_get_image (display);
        PicmanDrawable *drawable = picman_image_get_active_drawable (image);

        if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
          {
            picman_tool_message_literal (tool, display,
                                       _("Cannot modify the pixels of layer groups."));
          }
        else if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
          {
            picman_tool_message_literal (tool, display,
                                       _("The active layer's pixels are locked."));
          }
        else
          {
            PicmanTranslateMode edit_mode;

            if (sel_tool->function == SELECTION_MOVE)
              edit_mode = PICMAN_TRANSLATE_MODE_MASK_TO_LAYER;
            else
              edit_mode = PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER;

            picman_edit_selection_tool_start (tool, display, coords,
                                            edit_mode, FALSE);
         }

        return TRUE;
      }

    default:
      break;
    }

  return FALSE;
}
