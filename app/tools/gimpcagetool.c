/* PICMAN - The GNU Image Manipulation Program
 *
 * picmancagetool.c
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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
#include <stdlib.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "gegl/picman-gegl-utils.h"

#include "operations/picmancageconfig.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmandrawable-shadow.h"
#include "core/picmanimage.h"
#include "core/picmanimagemap.h"
#include "core/picmanlayer.h"
#include "core/picmanprogress.h"
#include "core/picmanprojection.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmancagetool.h"
#include "picmancageoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


enum
{
  CAGE_STATE_INIT,
  CAGE_STATE_WAIT,
  CAGE_STATE_MOVE_HANDLE,
  CAGE_STATE_SELECTING,
  CAGE_STATE_CLOSING,
  DEFORM_STATE_WAIT,
  DEFORM_STATE_MOVE_HANDLE,
  DEFORM_STATE_SELECTING
};


static void       picman_cage_tool_start              (PicmanCageTool          *ct,
                                                     PicmanDisplay           *display);

static void       picman_cage_tool_options_notify     (PicmanTool              *tool,
                                                     PicmanToolOptions       *options,
                                                     const GParamSpec      *pspec);
static void       picman_cage_tool_button_press       (PicmanTool              *tool,
                                                     const PicmanCoords      *coords,
                                                     guint32                time,
                                                     GdkModifierType        state,
                                                     PicmanButtonPressType    press_type,
                                                     PicmanDisplay           *display);
static void       picman_cage_tool_button_release     (PicmanTool              *tool,
                                                     const PicmanCoords      *coords,
                                                     guint32                time,
                                                     GdkModifierType        state,
                                                     PicmanButtonReleaseType  release_type,
                                                     PicmanDisplay           *display);
static gboolean   picman_cage_tool_key_press          (PicmanTool              *tool,
                                                     GdkEventKey           *kevent,
                                                     PicmanDisplay           *display);
static void       picman_cage_tool_motion             (PicmanTool              *tool,
                                                     const PicmanCoords      *coords,
                                                     guint32                time,
                                                     GdkModifierType        state,
                                                     PicmanDisplay           *display);
static void       picman_cage_tool_control            (PicmanTool              *tool,
                                                     PicmanToolAction         action,
                                                     PicmanDisplay           *display);
static void       picman_cage_tool_cursor_update      (PicmanTool              *tool,
                                                     const PicmanCoords      *coords,
                                                     GdkModifierType        state,
                                                     PicmanDisplay           *display);
static void       picman_cage_tool_oper_update        (PicmanTool              *tool,
                                                     const PicmanCoords      *coords,
                                                     GdkModifierType        state,
                                                     gboolean               proximity,
                                                     PicmanDisplay           *display);

static void       picman_cage_tool_draw               (PicmanDrawTool          *draw_tool);

static gint       picman_cage_tool_is_on_handle       (PicmanCageTool          *ct,
                                                     PicmanDrawTool          *draw_tool,
                                                     PicmanDisplay           *display,
                                                     gdouble                x,
                                                     gdouble                y,
                                                     gint                   handle_size);
static gint       picman_cage_tool_is_on_edge         (PicmanCageTool          *ct,
                                                     gdouble                x,
                                                     gdouble                y,
                                                     gint                   handle_size);

static void       picman_cage_tool_remove_last_handle (PicmanCageTool          *ct);
static void       picman_cage_tool_compute_coef       (PicmanCageTool          *ct);
static void       picman_cage_tool_create_image_map   (PicmanCageTool          *ct,
                                                     PicmanDrawable          *drawable);
static void       picman_cage_tool_image_map_flush    (PicmanImageMap          *image_map,
                                                     PicmanTool              *tool);
static void       picman_cage_tool_image_map_update   (PicmanCageTool          *ct);

static void       picman_cage_tool_create_render_node (PicmanCageTool          *ct);
static void       picman_cage_tool_render_node_update (PicmanCageTool          *ct);


G_DEFINE_TYPE (PicmanCageTool, picman_cage_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_cage_tool_parent_class


void
picman_cage_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_CAGE_TOOL,
                PICMAN_TYPE_CAGE_OPTIONS,
                picman_cage_options_gui,
                0,
                "picman-cage-tool",
                _("Cage Transform"),
                _("Cage Transform: Deform a selection with a cage"),
                N_("_Cage Transform"), "<shift>G",
                NULL, PICMAN_HELP_TOOL_CAGE,
                PICMAN_STOCK_TOOL_CAGE,
                data);
}

static void
picman_cage_tool_class_init (PicmanCageToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->options_notify = picman_cage_tool_options_notify;
  tool_class->button_press   = picman_cage_tool_button_press;
  tool_class->button_release = picman_cage_tool_button_release;
  tool_class->key_press      = picman_cage_tool_key_press;
  tool_class->motion         = picman_cage_tool_motion;
  tool_class->control        = picman_cage_tool_control;
  tool_class->cursor_update  = picman_cage_tool_cursor_update;
  tool_class->oper_update    = picman_cage_tool_oper_update;

  draw_tool_class->draw      = picman_cage_tool_draw;
}

static void
picman_cage_tool_init (PicmanCageTool *self)
{
  PicmanTool *tool = PICMAN_TOOL (self);

  picman_tool_control_set_dirty_mask  (tool->control,
                                     PICMAN_DIRTY_IMAGE           |
                                     PICMAN_DIRTY_IMAGE_STRUCTURE |
                                     PICMAN_DIRTY_DRAWABLE        |
                                     PICMAN_DIRTY_SELECTION       |
                                     PICMAN_DIRTY_ACTIVE_DRAWABLE);
  picman_tool_control_set_wants_click (tool->control, TRUE);
  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_PERSPECTIVE);

  self->config          = g_object_new (PICMAN_TYPE_CAGE_CONFIG, NULL);
  self->hovering_handle = -1;
  self->cage_complete   = FALSE;
  self->tool_state      = CAGE_STATE_INIT;

  self->coef            = NULL;
  self->render_node     = NULL;
  self->coef_node       = NULL;
  self->cage_node       = NULL;
  self->image_map       = NULL;
}

static void
picman_cage_tool_control (PicmanTool       *tool,
                        PicmanToolAction  action,
                        PicmanDisplay    *display)
{
  PicmanCageTool *ct = PICMAN_CAGE_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      if (ct->config)
        {
          g_object_unref (ct->config);
          ct->config = NULL;
        }

      if (ct->coef)
        {
          g_object_unref (ct->coef);
          ct->coef = NULL;
        }

      if (ct->render_node)
        {
          g_object_unref (ct->render_node);
          ct->render_node = NULL;
          ct->coef_node   = NULL;
          ct->cage_node   = NULL;
        }

      if (ct->image_map)
        {
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_map_abort (ct->image_map);
          g_object_unref (ct->image_map);
          ct->image_map = NULL;

          picman_tool_control_pop_preserve (tool->control);

          picman_image_flush (picman_display_get_image (tool->display));
        }

      tool->display = NULL;

      g_object_set (picman_tool_get_options (tool),
                    "cage-mode", PICMAN_CAGE_MODE_CAGE_CHANGE,
                    NULL);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_cage_tool_start (PicmanCageTool *ct,
                      PicmanDisplay  *display)
{
  PicmanTool     *tool     = PICMAN_TOOL (ct);
  PicmanImage    *image    = picman_display_get_image (display);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);
  gint          off_x;
  gint          off_y;

  picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  tool->display = display;

  if (ct->config)
    {
      g_object_unref (ct->config);
      ct->config = NULL;
    }

  if (ct->coef)
    {
      g_object_unref (ct->coef);
      ct->dirty_coef = TRUE;
      ct->coef = NULL;
    }

  if (ct->image_map)
    {
      picman_image_map_abort (ct->image_map);
      g_object_unref (ct->image_map);
      ct->image_map = NULL;
    }

  if (ct->render_node)
    {
      g_object_unref (ct->render_node);
      ct->render_node = NULL;
      ct->coef_node   = NULL;
      ct->cage_node   = NULL;
    }

  ct->config          = g_object_new (PICMAN_TYPE_CAGE_CONFIG, NULL);
  ct->hovering_handle = -1;
  ct->hovering_edge   = -1;
  ct->cage_complete   = FALSE;
  ct->tool_state      = CAGE_STATE_INIT;

  /* Setting up cage offset to convert the cage point coords to
   * drawable coords
   */
  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  ct->offset_x = off_x;
  ct->offset_y = off_y;

  picman_draw_tool_start (PICMAN_DRAW_TOOL (ct), display);
}

static void
picman_cage_tool_options_notify (PicmanTool         *tool,
                               PicmanToolOptions  *options,
                               const GParamSpec *pspec)
{
  PicmanCageTool *ct = PICMAN_CAGE_TOOL (tool);

  PICMAN_TOOL_CLASS (parent_class)->options_notify (tool, options, pspec);

  if (! tool->display)
    return;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  if (strcmp (pspec->name, "cage-mode") == 0)
    {
      PicmanCageMode mode;

      g_object_get (options,
                    "cage-mode", &mode,
                    NULL);

      if (mode == PICMAN_CAGE_MODE_DEFORM)
        {
          /* switch to deform mode */

          ct->cage_complete = TRUE;
          picman_cage_config_reset_displacement (ct->config);
          picman_cage_config_reverse_cage_if_needed (ct->config);
          picman_tool_push_status (tool, tool->display,
                                 _("Press ENTER to commit the transform"));
          ct->tool_state = DEFORM_STATE_WAIT;

          if (! ct->render_node)
            {
              picman_cage_tool_create_render_node (ct);
            }

          if (ct->dirty_coef)
            {
              picman_cage_tool_compute_coef (ct);
              picman_cage_tool_render_node_update (ct);
            }

          if (! ct->image_map)
            {
              PicmanImage    *image    = picman_display_get_image (tool->display);
              PicmanDrawable *drawable = picman_image_get_active_drawable (image);

              picman_cage_tool_create_image_map (ct, drawable);
            }

          picman_cage_tool_image_map_update (ct);
        }
      else
        {
          /* switch to edit mode */
          picman_image_map_abort (ct->image_map);

          picman_tool_pop_status (tool, tool->display);
          ct->tool_state = CAGE_STATE_WAIT;
        }
    }
  else if (strcmp  (pspec->name, "fill-plain-color") == 0)
    {
      picman_cage_tool_render_node_update (ct);
      picman_cage_tool_image_map_update (ct);
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static gboolean
picman_cage_tool_key_press (PicmanTool    *tool,
                          GdkEventKey *kevent,
                          PicmanDisplay *display)
{
  PicmanCageTool *ct = PICMAN_CAGE_TOOL (tool);

  switch (kevent->keyval)
    {
    case GDK_KEY_BackSpace:
      if (! ct->cage_complete && ct->tool_state == CAGE_STATE_WAIT)
        {
          picman_cage_tool_remove_last_handle (ct);
        }
      else if (ct->cage_complete && ct->tool_state == CAGE_STATE_WAIT)
        {
          picman_cage_config_remove_selected_points(ct->config);

          /* if the cage have less than 3 handles, we reopen it */
          if (picman_cage_config_get_n_points(ct->config) <= 2)
            ct->cage_complete = FALSE;
        }
      return TRUE;

    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
    case GDK_KEY_ISO_Enter:
      if (ct->cage_complete == FALSE && picman_cage_config_get_n_points (ct->config) > 2)
        {
          g_object_set (picman_tool_get_options (tool),
                        "cage-mode", PICMAN_CAGE_MODE_DEFORM,
                        NULL);
        }
      else if (ct->tool_state == DEFORM_STATE_WAIT)
        {
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_map_commit (ct->image_map,
                                 PICMAN_PROGRESS (tool));
          g_object_unref (ct->image_map);
          ct->image_map = NULL;

          picman_tool_control_pop_preserve (tool->control);

          picman_image_flush (picman_display_get_image (display));

          picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
        }
      break;

    case GDK_KEY_Escape:
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
      return TRUE;

    default:
      break;
    }

  return FALSE;
}

static void
picman_cage_tool_motion (PicmanTool         *tool,
                       const PicmanCoords *coords,
                       guint32           time,
                       GdkModifierType   state,
                       PicmanDisplay      *display)
{
  PicmanCageTool    *ct       = PICMAN_CAGE_TOOL (tool);
  PicmanCageOptions *options  = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  ct->cursor_x = coords->x;
  ct->cursor_y = coords->y;

  switch (ct->tool_state)
    {
    case CAGE_STATE_MOVE_HANDLE:
    case CAGE_STATE_CLOSING:
    case DEFORM_STATE_MOVE_HANDLE:
      picman_cage_config_add_displacement (ct->config,
                                         options->cage_mode,
                                         ct->cursor_x - ct->movement_start_x,
                                         ct->cursor_y - ct->movement_start_y);
      break;
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_cage_tool_oper_update (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            GdkModifierType   state,
                            gboolean          proximity,
                            PicmanDisplay      *display)
{
  PicmanCageTool *ct        = PICMAN_CAGE_TOOL (tool);
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

  if (ct->config)
    {
      ct->hovering_handle = picman_cage_tool_is_on_handle (ct,
                                                         draw_tool,
                                                         display,
                                                         coords->x,
                                                         coords->y,
                                                         PICMAN_TOOL_HANDLE_SIZE_CIRCLE);

      ct->hovering_edge = picman_cage_tool_is_on_edge (ct,
                                                     coords->x,
                                                     coords->y,
                                                     PICMAN_TOOL_HANDLE_SIZE_CIRCLE);
    }

  picman_draw_tool_pause (draw_tool);

  ct->cursor_x        = coords->x;
  ct->cursor_y        = coords->y;

  picman_draw_tool_resume (draw_tool);
}

static void
picman_cage_tool_button_press (PicmanTool            *tool,
                             const PicmanCoords    *coords,
                             guint32              time,
                             GdkModifierType      state,
                             PicmanButtonPressType  press_type,
                             PicmanDisplay         *display)
{
  PicmanCageTool    *ct        = PICMAN_CAGE_TOOL (tool);
  PicmanDrawTool    *draw_tool = PICMAN_DRAW_TOOL (tool);
  gint             handle    = -1;
  gint             edge      = -1;

  if (display != tool->display)
    picman_cage_tool_start (ct, display);

  picman_tool_control_activate (tool->control);

  if (ct->config)
    {
      handle = picman_cage_tool_is_on_handle (ct,
                                            draw_tool,
                                            display,
                                            coords->x,
                                            coords->y,
                                            PICMAN_TOOL_HANDLE_SIZE_CIRCLE);
      edge = picman_cage_tool_is_on_edge (ct,
                                        coords->x,
                                        coords->y,
                                        PICMAN_TOOL_HANDLE_SIZE_CIRCLE);
    }

  ct->movement_start_x = coords->x;
  ct->movement_start_y = coords->y;

  switch (ct->tool_state)
    {
      case CAGE_STATE_INIT:
        /* No handle yet, we add the first one and swith the tool to
         * moving handle state.
         */
        picman_cage_config_add_cage_point (ct->config,
                                         coords->x - ct->offset_x,
                                         coords->y - ct->offset_y);
        picman_cage_config_select_point (ct->config, 0);
        ct->tool_state = CAGE_STATE_MOVE_HANDLE;
        break;

      case CAGE_STATE_WAIT:
        if (ct->cage_complete == FALSE)
          {
            if (handle == -1 && edge <= 0)
              {
                /* User clicked on the background, we add a new handle
                 * and move it
                 */
                picman_cage_config_add_cage_point (ct->config,
                                                 coords->x - ct->offset_x,
                                                 coords->y - ct->offset_y);
                picman_cage_config_select_point (ct->config,
                                               picman_cage_config_get_n_points (ct->config) - 1);
                ct->tool_state = CAGE_STATE_MOVE_HANDLE;
              }
            else if (handle == 0 && picman_cage_config_get_n_points (ct->config) > 2)
              {
                /* User clicked on the first handle, we wait for
                 * release for closing the cage and switching to
                 * deform if possible
                 */
                picman_cage_config_select_point (ct->config, 0);
                ct->tool_state = CAGE_STATE_CLOSING;
              }
            else if (handle >= 0)
              {
                /* User clicked on a handle, so we move it */

                if (state & GDK_SHIFT_MASK)
                  {
                    /* Multiple selection */

                    picman_cage_config_toggle_point_selection (ct->config, handle);
                  }
                else
                  {
                    /* New selection */

                    if (! picman_cage_config_point_is_selected (ct->config, handle))
                      {
                        picman_cage_config_select_point (ct->config, handle);
                      }
                  }

                ct->tool_state = CAGE_STATE_MOVE_HANDLE;
              }
            else if (edge > 0)
              {
                /* User clicked on an edge, we add a new handle here and select it */

                picman_cage_config_insert_cage_point (ct->config, edge, coords->x, coords->y);
                picman_cage_config_select_point (ct->config, edge);
                ct->tool_state = CAGE_STATE_MOVE_HANDLE;
              }
          }
        else
          {
            /* Cage already closed */

            if (handle == -1 && edge == -1)
              {
                /* User clicked on the background, we start a rubber
                 * band selection
                 */
                ct->selection_start_x = coords->x;
                ct->selection_start_y = coords->y;
                ct->tool_state = CAGE_STATE_SELECTING;
              }
            else if (handle >= 0)
              {
                /* User clicked on a handle, so we move it */

                if (state & GDK_SHIFT_MASK)
                  {
                    /* Multiple selection */

                    picman_cage_config_toggle_point_selection (ct->config, handle);
                  }
                else
                  {
                    /* New selection */

                    if (! picman_cage_config_point_is_selected (ct->config, handle))
                      {
                        picman_cage_config_select_point (ct->config, handle);
                      }
                  }

                ct->tool_state = CAGE_STATE_MOVE_HANDLE;
              }
            else if (edge >= 0)
              {
                /* User clicked on an edge, we add a new handle here and select it */

                picman_cage_config_insert_cage_point (ct->config, edge, coords->x, coords->y);
                picman_cage_config_select_point (ct->config, edge);
                ct->tool_state = CAGE_STATE_MOVE_HANDLE;
              }
          }
        break;

      case DEFORM_STATE_WAIT:
        if (handle == -1)
          {
            /* User clicked on the background, we start a rubber band
             * selection
             */
            ct->selection_start_x = coords->x;
            ct->selection_start_y = coords->y;
            ct->tool_state = DEFORM_STATE_SELECTING;
          }

        if (handle >= 0)
          {
            /* User clicked on a handle, so we move it */

            if (state & GDK_SHIFT_MASK)
              {
                /* Multiple selection */

                picman_cage_config_toggle_point_selection (ct->config, handle);
              }
            else
              {
                /* New selection */

                if (! picman_cage_config_point_is_selected (ct->config, handle))
                  {
                    picman_cage_config_select_point (ct->config, handle);
                  }
              }

            ct->tool_state = DEFORM_STATE_MOVE_HANDLE;
          }
        break;
    }
}

void
picman_cage_tool_button_release (PicmanTool              *tool,
                               const PicmanCoords      *coords,
                               guint32                time,
                               GdkModifierType        state,
                               PicmanButtonReleaseType  release_type,
                               PicmanDisplay           *display)
{
  PicmanCageTool    *ct      = PICMAN_CAGE_TOOL (tool);
  PicmanCageOptions *options = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (ct));

  picman_tool_control_halt (tool->control);

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    {
      /* Cancelling */

      switch (ct->tool_state)
        {
        case CAGE_STATE_CLOSING:
          ct->tool_state = CAGE_STATE_WAIT;
          break;

        case CAGE_STATE_MOVE_HANDLE:
          picman_cage_config_remove_last_cage_point (ct->config);
          ct->tool_state = CAGE_STATE_WAIT;
          break;

        case CAGE_STATE_SELECTING:
          ct->tool_state = CAGE_STATE_WAIT;
          break;

        case DEFORM_STATE_MOVE_HANDLE:
          picman_cage_tool_image_map_update (ct);
          ct->tool_state = DEFORM_STATE_WAIT;
          break;

        case DEFORM_STATE_SELECTING:
          ct->tool_state = DEFORM_STATE_WAIT;
          break;
        }

      picman_cage_config_reset_displacement (ct->config);
    }
  else
    {
      /* Normal release */

      switch (ct->tool_state)
        {
        case CAGE_STATE_CLOSING:
          ct->dirty_coef = TRUE;
          picman_cage_config_commit_displacement (ct->config);

          if (release_type == PICMAN_BUTTON_RELEASE_CLICK)
            g_object_set (options, "cage-mode", PICMAN_CAGE_MODE_DEFORM, NULL);
          break;

        case CAGE_STATE_MOVE_HANDLE:
          ct->dirty_coef = TRUE;
          ct->tool_state = CAGE_STATE_WAIT;
          picman_cage_config_commit_displacement (ct->config);
          break;

        case CAGE_STATE_SELECTING:
          {
            GeglRectangle area = { MIN (ct->selection_start_x, coords->x) - ct->offset_x,
                                   MIN (ct->selection_start_y, coords->y) - ct->offset_y,
                                   abs (ct->selection_start_x - coords->x),
                                   abs (ct->selection_start_y - coords->y) };

            if (state & GDK_SHIFT_MASK)
              {
                picman_cage_config_select_add_area (ct->config,
                                                  PICMAN_CAGE_MODE_CAGE_CHANGE,
                                                  area);
              }
            else
              {
                picman_cage_config_select_area (ct->config,
                                              PICMAN_CAGE_MODE_CAGE_CHANGE,
                                              area);
              }

            ct->tool_state = CAGE_STATE_WAIT;
          }
          break;

        case DEFORM_STATE_MOVE_HANDLE:
          ct->tool_state = DEFORM_STATE_WAIT;
          picman_cage_config_commit_displacement (ct->config);
          picman_cage_tool_image_map_update (ct);
          break;

        case DEFORM_STATE_SELECTING:
          {
            GeglRectangle area = { MIN (ct->selection_start_x, coords->x) - ct->offset_x,
                                   MIN (ct->selection_start_y, coords->y) - ct->offset_y,
                                   abs (ct->selection_start_x - coords->x),
                                   abs (ct->selection_start_y - coords->y) };

            if (state & GDK_SHIFT_MASK)
              {
                picman_cage_config_select_add_area (ct->config,
                                                  PICMAN_CAGE_MODE_DEFORM, area);
              }
            else
              {
                picman_cage_config_select_area (ct->config,
                                              PICMAN_CAGE_MODE_DEFORM, area);
              }

            ct->tool_state = DEFORM_STATE_WAIT;
          }
          break;
        }
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_cage_tool_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  PicmanCageTool       *ct       = PICMAN_CAGE_TOOL (tool);
  PicmanCageOptions    *options  = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  PicmanCursorModifier  modifier = PICMAN_CURSOR_MODIFIER_PLUS;

  if (tool->display)
    {
      if (ct->hovering_handle != -1)
        {
          modifier = PICMAN_CURSOR_MODIFIER_MOVE;
        }
      else if (ct->hovering_edge != -1 && options->cage_mode == PICMAN_CAGE_MODE_CAGE_CHANGE)
        {
          modifier = PICMAN_CURSOR_MODIFIER_PLUS;
        }
      else
        {
          if (ct->cage_complete)
            modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }
    }

  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_cage_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanCageTool    *ct        = PICMAN_CAGE_TOOL (draw_tool);
  PicmanCageOptions *options   = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  PicmanCageConfig  *config    = ct->config;
  PicmanCanvasGroup *stroke_group;
  gint             n_vertices;
  gint             i;
  PicmanHandleType   handle;

  n_vertices = picman_cage_config_get_n_points (config);

  if (n_vertices == 0)
    return;

  if (ct->tool_state == CAGE_STATE_INIT)
    return;

  stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

  picman_draw_tool_push_group (draw_tool, stroke_group);

  /* If needed, draw line to the cursor. */
  if (! ct->cage_complete)
    {
      PicmanVector2 last_point;

      last_point = picman_cage_config_get_point_coordinate (ct->config,
                                                          options->cage_mode,
                                                          n_vertices - 1);

      picman_draw_tool_add_line (draw_tool,
                               last_point.x + ct->offset_x,
                               last_point.y + ct->offset_y,
                               ct->cursor_x,
                               ct->cursor_y);
    }

  picman_draw_tool_pop_group (draw_tool);

  /* Draw the cage with handles. */
  for (i = 0; i < n_vertices; i++)
    {
      PicmanVector2 point1, point2;

      point1 = picman_cage_config_get_point_coordinate (ct->config,
                                                      options->cage_mode,
                                                      i);
      point1.x += ct->offset_x;
      point1.y += ct->offset_y;

      if (i > 0 || ct->cage_complete)
        {
          gint index_point2;

          if (i == 0)
            index_point2 = n_vertices - 1;
          else
            index_point2 = i - 1;

          point2 = picman_cage_config_get_point_coordinate (ct->config,
                                                          options->cage_mode,
                                                          index_point2);
          point2.x += ct->offset_x;
          point2.y += ct->offset_y;

          picman_draw_tool_push_group (draw_tool, stroke_group);

          picman_draw_tool_add_line (draw_tool,
                                   point1.x,
                                   point1.y,
                                   point2.x,
                                   point2.y);

          picman_draw_tool_pop_group (draw_tool);
        }

      if (i == ct->hovering_handle)
        handle = PICMAN_HANDLE_FILLED_CIRCLE;
      else
        handle = PICMAN_HANDLE_CIRCLE;

      picman_draw_tool_add_handle (draw_tool,
                                 handle,
                                 point1.x,
                                 point1.y,
                                 PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                 PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                 PICMAN_HANDLE_ANCHOR_CENTER);

      if (picman_cage_config_point_is_selected (ct->config, i))
        {
          picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_SQUARE,
                                     point1.x,
                                     point1.y,
                                     PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                     PICMAN_TOOL_HANDLE_SIZE_CIRCLE,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
        }
    }

  if (ct->tool_state == DEFORM_STATE_SELECTING ||
      ct->tool_state == CAGE_STATE_SELECTING)
    {
      picman_draw_tool_add_rectangle (draw_tool,
                                    FALSE,
                                    MIN (ct->selection_start_x, ct->cursor_x),
                                    MIN (ct->selection_start_y, ct->cursor_y),
                                    abs (ct->selection_start_x - ct->cursor_x),
                                    abs (ct->selection_start_y - ct->cursor_y));
    }
}

static gint
picman_cage_tool_is_on_handle (PicmanCageTool *ct,
                             PicmanDrawTool *draw_tool,
                             PicmanDisplay  *display,
                             gdouble       x,
                             gdouble       y,
                             gint          handle_size)
{
  PicmanCageOptions *options = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  PicmanCageConfig  *config  = ct->config;
  gdouble          dist    = G_MAXDOUBLE;
  gint             i;
  PicmanVector2      cage_point;
  guint            n_cage_vertices;

  g_return_val_if_fail (PICMAN_IS_CAGE_TOOL (ct), -1);

  n_cage_vertices = picman_cage_config_get_n_points (config);

  if (n_cage_vertices == 0)
    return -1;

  for (i = 0; i < n_cage_vertices; i++)
    {
      cage_point = picman_cage_config_get_point_coordinate (config,
                                                          options->cage_mode,
                                                          i);
      cage_point.x += ct->offset_x;
      cage_point.y += ct->offset_y;

      dist = picman_draw_tool_calc_distance_square (PICMAN_DRAW_TOOL (draw_tool),
                                                  display,
                                                  x, y,
                                                  cage_point.x,
                                                  cage_point.y);

      if (dist <= SQR (handle_size / 2))
        return i;
    }

  return -1;
}

static gint
picman_cage_tool_is_on_edge (PicmanCageTool *ct,
                           gdouble       x,
                           gdouble       y,
                           gint          handle_size)
{
  PicmanCageOptions *options = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  PicmanCageConfig  *config  = ct->config;
  gint             i;
  guint            n_cage_vertices;
  PicmanVector2      A, B, C, AB, BC, AC;
  gdouble          lAB, lBC, lAC, lEB, lEC;

  g_return_val_if_fail (PICMAN_IS_CAGE_TOOL (ct), -1);

  n_cage_vertices = picman_cage_config_get_n_points (config);

  if (n_cage_vertices < 2)
    return -1;

  A = picman_cage_config_get_point_coordinate (config,
                                             options->cage_mode,
                                             n_cage_vertices-1);
  B = picman_cage_config_get_point_coordinate (config,
                                             options->cage_mode,
                                             0);
  C.x = x;
  C.y = y;

  for (i = 0; i < n_cage_vertices; i++)
    {
      picman_vector2_sub (&AB, &A, &B);
      picman_vector2_sub (&BC, &B, &C);
      picman_vector2_sub (&AC, &A, &C);

      lAB = picman_vector2_length (&AB);
      lBC = picman_vector2_length (&BC);
      lAC = picman_vector2_length (&AC);
      lEB = lAB / 2 + (SQR (lBC) - SQR (lAC)) / (2 * lAB);
      lEC = sqrt (SQR (lBC) - SQR (lEB));

      if ((lEC < handle_size / 2) && (abs (SQR (lBC) - SQR (lAC)) <= SQR (lAB)))
        return i;

      A = B;
      B = picman_cage_config_get_point_coordinate (config,
                                                 options->cage_mode,
                                                 (i+1) % n_cage_vertices);
    }

  return -1;
}

static void
picman_cage_tool_remove_last_handle (PicmanCageTool *ct)
{
  PicmanCageConfig *config = ct->config;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (ct));

  picman_cage_config_remove_last_cage_point (config);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (ct));
}

static void
picman_cage_tool_compute_coef (PicmanCageTool *ct)
{
  PicmanCageConfig *config = ct->config;
  PicmanProgress   *progress;
  const Babl     *format;
  GeglNode       *gegl;
  GeglNode       *input;
  GeglNode       *output;
  GeglProcessor  *processor;
  GeglBuffer     *buffer;
  gdouble         value;

  progress = picman_progress_start (PICMAN_PROGRESS (ct),
                                  _("Computing Cage Coefficients"), FALSE);

  if (ct->coef)
    {
      g_object_unref (ct->coef);
      ct->coef = NULL;
    }

  format = babl_format_n (babl_type ("float"),
                          picman_cage_config_get_n_points (config) * 2);


  gegl = gegl_node_new ();

  input = gegl_node_new_child (gegl,
                               "operation", "picman:cage-coef-calc",
                               "config",    ct->config,
                               NULL);

  output = gegl_node_new_child (gegl,
                                "operation", "gegl:buffer-sink",
                                "buffer",    &buffer,
                                "format",    format,
                                NULL);

  gegl_node_connect_to (input, "output",
                        output, "input");

  processor = gegl_node_new_processor (output, NULL);

  while (gegl_processor_work (processor, &value))
    {
      if (progress)
        picman_progress_set_value (progress, value);
    }

  if (progress)
    picman_progress_end (progress);

  g_object_unref (processor);

  ct->coef = buffer;
  g_object_unref (gegl);

  ct->dirty_coef = FALSE;
}

static void
picman_cage_tool_create_render_node (PicmanCageTool *ct)
{
  PicmanCageOptions *options  = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  GeglNode        *coef, *cage, *render; /* Render nodes */
  GeglNode        *input, *output; /* Proxy nodes*/
  GeglNode        *node; /* wraper to be returned */

  g_return_if_fail (ct->render_node == NULL);
  /* render_node is not supposed to be recreated */

  node = gegl_node_new ();

  input  = gegl_node_get_input_proxy  (node, "input");
  output = gegl_node_get_output_proxy (node, "output");

  coef = gegl_node_new_child (node,
                              "operation", "gegl:buffer-source",
                              "buffer",    ct->coef,
                              NULL);

  cage = gegl_node_new_child (node,
                              "operation",        "picman:cage-transform",
                              "config",           ct->config,
                              "fill_plain_color", options->fill_plain_color,
                              NULL);

  render = gegl_node_new_child (node,
                                "operation", "gegl:map-absolute",
                                NULL);

  gegl_node_connect_to (input, "output",
                        cage, "input");

  gegl_node_connect_to (coef, "output",
                        cage, "aux");

  gegl_node_connect_to (input, "output",
                        render, "input");

  gegl_node_connect_to (cage, "output",
                        render, "aux");

  gegl_node_connect_to (render, "output",
                        output, "input");

  ct->render_node = node;
  ct->cage_node = cage;
  ct->coef_node = coef;

  picman_gegl_progress_connect (cage, PICMAN_PROGRESS (ct), _("Cage Transform"));
}

static void
picman_cage_tool_render_node_update (PicmanCageTool *ct)
{
  PicmanCageOptions *options  = PICMAN_CAGE_TOOL_GET_OPTIONS (ct);
  gboolean         option_fill, node_fill;
  GeglBuffer      *buffer;

  g_object_get (options,
                "fill-plain-color", &option_fill,
                NULL);

  gegl_node_get (ct->cage_node,
                 "fill-plain-color", &node_fill,
                 NULL);

  if (option_fill != node_fill)
    {
      gegl_node_set (ct->cage_node,
                     "fill_plain_color", option_fill,
                     NULL);
    }

  gegl_node_get (ct->coef_node,
                 "buffer", &buffer,
                 NULL);

  if (buffer != ct->coef)
    {
      gegl_node_set (ct->coef_node,
                     "buffer",  ct->coef,
                     NULL);
    }

  /* This just unref buffer, since gegl_node_get add a refcount on it */
  if (buffer)
    {
      g_object_unref (buffer);
    }
}

static void
picman_cage_tool_create_image_map (PicmanCageTool *ct,
                                 PicmanDrawable *drawable)
{
  if (!ct->render_node)
    picman_cage_tool_create_render_node (ct);

  ct->image_map = picman_image_map_new (drawable,
                                      _("Cage transform"),
                                      ct->render_node,
                                      PICMAN_STOCK_TOOL_CAGE);

  g_signal_connect (ct->image_map, "flush",
                    G_CALLBACK (picman_cage_tool_image_map_flush),
                    ct);
}

static void
picman_cage_tool_image_map_flush (PicmanImageMap *image_map,
                                PicmanTool     *tool)
{
  PicmanImage *image = picman_display_get_image (tool->display);

  picman_projection_flush (picman_image_get_projection (image));
}

static void
picman_cage_tool_image_map_update (PicmanCageTool *ct)
{
  picman_image_map_apply (ct->image_map);
}
