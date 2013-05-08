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

#include <stdlib.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picmanboundary.h"
#include "core/picmancontext.h"
#include "core/picmanchannel.h"
#include "core/picmandrawable-transform.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanitem-linked.h"
#include "core/picmanlayer.h"
#include "core/picmanprogress.h"
#include "core/picmantoolinfo.h"
#include "core/picman-transform-utils.h"
#include "core/picman-utils.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanstroke.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-transform.h"
#include "display/picmantooldialog.h"

#include "picmantoolcontrol.h"
#include "picmanperspectivetool.h"
#include "picmanunifiedtransformtool.h"
#include "picmantransformoptions.h"
#include "picmantransformtool.h"
#include "picmantransformtoolundo.h"

#include "picman-intl.h"


#define RESPONSE_RESET  1
#define RESPONSE_UNDO   2
#define RESPONSE_REDO   3
#define MIN_HANDLE_SIZE 6


static void      picman_transform_tool_finalize               (GObject               *object);

static gboolean  picman_transform_tool_initialize             (PicmanTool              *tool,
                                                             PicmanDisplay           *display,
                                                             GError               **error);
static void      picman_transform_tool_control                (PicmanTool              *tool,
                                                             PicmanToolAction         action,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_button_press           (PicmanTool              *tool,
                                                             const PicmanCoords      *coords,
                                                             guint32                time,
                                                             GdkModifierType        state,
                                                             PicmanButtonPressType    press_type,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_button_release         (PicmanTool              *tool,
                                                             const PicmanCoords      *coords,
                                                             guint32                time,
                                                             GdkModifierType        state,
                                                             PicmanButtonReleaseType  release_type,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_motion                 (PicmanTool              *tool,
                                                             const PicmanCoords      *coords,
                                                             guint32                time,
                                                             GdkModifierType        state,
                                                             PicmanDisplay           *display);
static gboolean  picman_transform_tool_key_press              (PicmanTool              *tool,
                                                             GdkEventKey           *kevent,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_modifier_key           (PicmanTool              *tool,
                                                             GdkModifierType        key,
                                                             gboolean               press,
                                                             GdkModifierType        state,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_oper_update            (PicmanTool              *tool,
                                                             const PicmanCoords      *coords,
                                                             GdkModifierType        state,
                                                             gboolean               proximity,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_cursor_update          (PicmanTool              *tool,
                                                             const PicmanCoords      *coords,
                                                             GdkModifierType        state,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_options_notify         (PicmanTool              *tool,
                                                             PicmanToolOptions       *options,
                                                             const GParamSpec      *pspec);

static void      picman_transform_tool_draw                   (PicmanDrawTool          *draw_tool);

static void      picman_transform_tool_dialog_update          (PicmanTransformTool     *tr_tool);

static GeglBuffer *
                 picman_transform_tool_real_transform         (PicmanTransformTool     *tr_tool,
                                                             PicmanItem              *item,
                                                             GeglBuffer            *orig_buffer,
                                                             gint                   orig_offset_x,
                                                             gint                   orig_offset_y,
                                                             gint                  *new_offset_x,
                                                             gint                  *new_offset_y);
static void      picman_transform_tool_real_draw_gui          (PicmanTransformTool     *tr_tool,
                                                             gint                   handle_w,
                                                             gint                   handle_h);
static TransformAction
                 picman_transform_tool_real_pick_function     (PicmanTransformTool     *tr_tool,
                                                             const PicmanCoords      *coords,
                                                             GdkModifierType        state,
                                                             PicmanDisplay           *display);

static void      picman_transform_tool_set_function           (PicmanTransformTool     *tr_tool,
                                                             TransformAction        function);
static void      picman_transform_tool_bounds                 (PicmanTransformTool     *tr_tool,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_dialog                 (PicmanTransformTool     *tr_tool);
static void      picman_transform_tool_prepare                (PicmanTransformTool     *tr_tool,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_transform              (PicmanTransformTool     *tr_tool,
                                                             PicmanDisplay           *display);
static void      picman_transform_tool_transform_bounding_box (PicmanTransformTool     *tr_tool);

static void      picman_transform_tool_handles_recalc         (PicmanTransformTool     *tr_tool,
                                                             PicmanDisplay           *display,
                                                             gint                  *handle_w,
                                                             gint                  *handle_h);
static void      picman_transform_tool_response               (GtkWidget             *widget,
                                                             gint                   response_id,
                                                             PicmanTransformTool     *tr_tool);

static void free_trans         (gpointer           data);
static void update_sensitivity (PicmanTransformTool *tr_tool);


G_DEFINE_TYPE (PicmanTransformTool, picman_transform_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_transform_tool_parent_class


static void
picman_transform_tool_class_init (PicmanTransformToolClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class   = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_class   = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->finalize          = picman_transform_tool_finalize;

  tool_class->initialize          = picman_transform_tool_initialize;
  tool_class->control             = picman_transform_tool_control;
  tool_class->button_press        = picman_transform_tool_button_press;
  tool_class->button_release      = picman_transform_tool_button_release;
  tool_class->motion              = picman_transform_tool_motion;
  tool_class->key_press           = picman_transform_tool_key_press;
  tool_class->modifier_key        = picman_transform_tool_modifier_key;
  tool_class->active_modifier_key = picman_transform_tool_modifier_key;
  tool_class->oper_update         = picman_transform_tool_oper_update;
  tool_class->cursor_update       = picman_transform_tool_cursor_update;
  tool_class->options_notify      = picman_transform_tool_options_notify;

  draw_class->draw                = picman_transform_tool_draw;

  klass->dialog                   = NULL;
  klass->dialog_update            = NULL;
  klass->prepare                  = NULL;
  klass->motion                   = NULL;
  klass->recalc_matrix            = NULL;
  klass->get_undo_desc            = NULL;
  klass->transform                = picman_transform_tool_real_transform;
  klass->pick_function            = picman_transform_tool_real_pick_function;
  klass->draw_gui                 = picman_transform_tool_real_draw_gui;
}

static void
picman_transform_tool_init (PicmanTransformTool *tr_tool)
{
  PicmanTool *tool = PICMAN_TOOL (tr_tool);

  picman_tool_control_set_action_value_1 (tool->control,
                                        "tools/tools-transform-preview-opacity-set");

  picman_tool_control_set_scroll_lock (tool->control, TRUE);
  picman_tool_control_set_preserve    (tool->control, FALSE);
  picman_tool_control_set_dirty_mask  (tool->control,
                                     PICMAN_DIRTY_IMAGE_SIZE |
                                     PICMAN_DIRTY_DRAWABLE   |
                                     PICMAN_DIRTY_SELECTION  |
                                     PICMAN_DIRTY_ACTIVE_DRAWABLE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_SUBPIXEL);

  tr_tool->function      = TRANSFORM_CREATING;
  tr_tool->progress_text = _("Transforming");

  picman_matrix3_identity (&tr_tool->transform);
}

static void
picman_transform_tool_finalize (GObject *object)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (object);

  if (tr_tool->dialog)
    {
      gtk_widget_destroy (tr_tool->dialog);
      tr_tool->dialog = NULL;
    }

  g_list_free_full (tr_tool->redo_list, free_trans);
  g_list_free_full (tr_tool->undo_list, free_trans);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
picman_transform_tool_initialize (PicmanTool     *tool,
                                PicmanDisplay  *display,
                                GError      **error)
{
  PicmanTransformTool *tr_tool  = PICMAN_TRANSFORM_TOOL (tool);
  PicmanImage         *image    = picman_display_get_image (display);
  PicmanDrawable      *drawable = picman_image_get_active_drawable (image);

  if (! PICMAN_TOOL_CLASS (parent_class)->initialize (tool, display, error))
    {
      return FALSE;
    }

  if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("The active layer's pixels are locked."));
      return FALSE;
    }

  if (picman_item_is_position_locked (PICMAN_ITEM (drawable)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("The active layer's position and size are locked."));
      return FALSE;
    }

  if (display != tool->display)
    {
      gint i;

      /*  Set the pointer to the active display  */
      tool->display  = display;
      tool->drawable = drawable;

      /*  Initialize the transform tool dialog */
      if (! tr_tool->dialog)
        picman_transform_tool_dialog (tr_tool);

      /*  Find the transform bounds for some tools (like scale,
       *  perspective) that actually need the bounds for initializing
       */
      picman_transform_tool_bounds (tr_tool, display);

      /*  Inizialize the tool-specific trans_info, and adjust the
       *  tool dialog
       */
      picman_transform_tool_prepare (tr_tool, display);

      /*  Recalculate the transform tool  */
      picman_transform_tool_recalc_matrix (tr_tool);

      /*  start drawing the bounding box and handles...  */
      picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

      tr_tool->function = TRANSFORM_CREATING;

      /* Initialize undo and redo lists */
      tr_tool->undo_list = g_list_prepend (NULL, g_slice_new (TransInfo));
      tr_tool->redo_list = NULL;
      tr_tool->old_trans_info = g_list_last (tr_tool->undo_list)->data;
      tr_tool->prev_trans_info = g_list_first (tr_tool->undo_list)->data;
      update_sensitivity (tr_tool);

      /*  Save the current transformation info  */
      for (i = 0; i < TRANS_INFO_SIZE; i++)
        {
          (*tr_tool->prev_trans_info)[i] = tr_tool->trans_info[i];
        }
    }

  return TRUE;
}

static void
picman_transform_tool_control (PicmanTool       *tool,
                             PicmanToolAction  action,
                             PicmanDisplay    *display)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
      break;

    case PICMAN_TOOL_ACTION_RESUME:
      picman_transform_tool_bounds (tr_tool, display);
      picman_transform_tool_recalc_matrix (tr_tool);
      break;

    case PICMAN_TOOL_ACTION_HALT:
      tr_tool->function = TRANSFORM_CREATING;

      if (tr_tool->dialog)
        picman_dialog_factory_hide_dialog (tr_tool->dialog);

      tool->drawable = NULL;
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_transform_tool_button_press (PicmanTool            *tool,
                                  const PicmanCoords    *coords,
                                  guint32              time,
                                  GdkModifierType      state,
                                  PicmanButtonPressType  press_type,
                                  PicmanDisplay         *display)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (tool);

  if (tr_tool->function == TRANSFORM_CREATING)
    PICMAN_TOOL_GET_CLASS (tool)->oper_update (tool, coords, state, TRUE, display);

  tr_tool->lastx = tr_tool->mousex = coords->x;
  tr_tool->lasty = tr_tool->mousey = coords->y;

  picman_tool_control_activate (tool->control);
}

void picman_transform_tool_push_internal_undo (PicmanTransformTool *tr_tool)
{
  gint i;

  /* push current state on the undo list and set this state as the
   * current state, but avoid doing this if there were no changes
   */
  for (i = 0; i < TRANS_INFO_SIZE; i++)
    if ((*tr_tool->prev_trans_info)[i] != tr_tool->trans_info[i])
      break;

  if (i < TRANS_INFO_SIZE)
    {
      tr_tool->prev_trans_info = g_slice_new (TransInfo);
      for (i = 0; i < TRANS_INFO_SIZE; i++)
        (*tr_tool->prev_trans_info)[i] = tr_tool->trans_info[i];
      tr_tool->undo_list = g_list_prepend (tr_tool->undo_list,
                                           tr_tool->prev_trans_info);

      /* If we undid anything and started interacting, we have to
       * discard the redo history
       */
      g_list_free_full (tr_tool->redo_list, free_trans);
      tr_tool->redo_list = NULL;

      update_sensitivity (tr_tool);
    }
}

static void
picman_transform_tool_button_release (PicmanTool              *tool,
                                    const PicmanCoords      *coords,
                                    guint32                time,
                                    GdkModifierType        state,
                                    PicmanButtonReleaseType  release_type,
                                    PicmanDisplay           *display)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (tool);
  gint               i;

  picman_tool_control_halt (tool->control);

  /*  if we are creating, there is nothing to be done...exit  */
  if (tr_tool->function == TRANSFORM_CREATING && tr_tool->use_grid)
    return;

  if (release_type != PICMAN_BUTTON_RELEASE_CANCEL)
    {
      /* This hack is to perform the flip immediately with the flip tool */
      if (! tr_tool->use_grid)
        {
          picman_transform_tool_response (NULL, GTK_RESPONSE_OK, tr_tool);
        }

      /* We're done with an interaction, save it on the undo list */
      picman_transform_tool_push_internal_undo (tr_tool);
    }
  else
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      /*  Restore the last saved state  */
      for (i = 0; i < TRANS_INFO_SIZE; i++)
        tr_tool->trans_info[i] = (*tr_tool->prev_trans_info)[i];

      /*  reget the selection bounds  */
      picman_transform_tool_bounds (tr_tool, display);

      /*  recalculate the tool's transformation matrix  */
      picman_transform_tool_recalc_matrix (tr_tool);

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }
}

static void
picman_transform_tool_motion (PicmanTool         *tool,
                            const PicmanCoords *coords,
                            guint32           time,
                            GdkModifierType   state,
                            PicmanDisplay      *display)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (tool);

  /*  if we are creating, there is nothing to be done so exit.  */
  if (tr_tool->function == TRANSFORM_CREATING || ! tr_tool->use_grid)
    return;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  tr_tool->curx = coords->x;
  tr_tool->cury = coords->y;

  /*  recalculate the tool's transformation matrix  */
  if (PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->motion)
    {
      PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->motion (tr_tool);

      picman_transform_tool_recalc_matrix (tr_tool);
    }

  tr_tool->lastx = tr_tool->curx;
  tr_tool->lasty = tr_tool->cury;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static gboolean
picman_transform_tool_key_press (PicmanTool    *tool,
                               GdkEventKey *kevent,
                               PicmanDisplay *display)
{
  PicmanTransformTool *trans_tool = PICMAN_TRANSFORM_TOOL (tool);
  PicmanDrawTool      *draw_tool  = PICMAN_DRAW_TOOL (tool);

  if (display == draw_tool->display)
    {
      switch (kevent->keyval)
        {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_ISO_Enter:
          picman_transform_tool_response (NULL, GTK_RESPONSE_OK, trans_tool);
          return TRUE;

        case GDK_KEY_BackSpace:
          picman_transform_tool_response (NULL, RESPONSE_UNDO, trans_tool);
          return TRUE;

        case GDK_KEY_space:
          picman_transform_tool_response (NULL, RESPONSE_REDO, trans_tool);
          return TRUE;

        case GDK_KEY_Escape:
          picman_transform_tool_response (NULL, GTK_RESPONSE_CANCEL, trans_tool);
          return TRUE;
        }
    }

  return FALSE;
}

static void
picman_transform_tool_modifier_key (PicmanTool        *tool,
                                  GdkModifierType  key,
                                  gboolean         press,
                                  GdkModifierType  state,
                                  PicmanDisplay     *display)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tool);

  if (key == picman_get_constrain_behavior_mask ())
    {
      g_object_set (options,
                    "frompivot-scale", ! options->frompivot_scale,
                    NULL);
      g_object_set (options,
                    "frompivot-shear", ! options->frompivot_shear,
                    NULL);
      g_object_set (options,
                    "frompivot-perspective", ! options->frompivot_perspective,
                    NULL);
    }

  if (key == picman_get_extend_selection_mask ())
    {
      g_object_set (options,
                    "cornersnap", ! options->cornersnap,
                    NULL);

      g_object_set (options,
                    "constrain-move", ! options->constrain_move,
                    NULL);
      g_object_set (options,
                    "constrain-scale", ! options->constrain_scale,
                    NULL);
      g_object_set (options,
                    "constrain-rotate", ! options->constrain_rotate,
                    NULL);
      g_object_set (options,
                    "constrain-shear", ! options->constrain_shear,
                    NULL);
      g_object_set (options,
                    "constrain-perspective", ! options->constrain_perspective,
                    NULL);
    }
}

static TransformAction
picman_transform_tool_real_pick_function (PicmanTransformTool *tr_tool,
                                        const PicmanCoords  *coords,
                                        GdkModifierType    state,
                                        PicmanDisplay       *display)
{
  PicmanDrawTool   *draw_tool = PICMAN_DRAW_TOOL (tr_tool);
  TransformAction function  = TRANSFORM_HANDLE_NONE;

  if (tr_tool->use_handles)
    {
        gdouble closest_dist;
        gdouble dist;

        dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                    coords->x, coords->y,
                                                    tr_tool->tx1, tr_tool->ty1);
        closest_dist = dist;
        function = TRANSFORM_HANDLE_NW;

        dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                    coords->x, coords->y,
                                                    tr_tool->tx2, tr_tool->ty2);
        if (dist < closest_dist)
          {
            closest_dist = dist;
            function = TRANSFORM_HANDLE_NE;
          }

        dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                    coords->x, coords->y,
                                                    tr_tool->tx3, tr_tool->ty3);
        if (dist < closest_dist)
          {
            closest_dist = dist;
            function = TRANSFORM_HANDLE_SW;
          }

        dist = picman_draw_tool_calc_distance_square (draw_tool, display,
                                                    coords->x, coords->y,
                                                    tr_tool->tx4, tr_tool->ty4);
        if (dist < closest_dist)
          {
            closest_dist = dist;
            function = TRANSFORM_HANDLE_SE;
          }

        if (tr_tool->use_mid_handles)
          {
            if (picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_N],
                                      coords->x, coords->y))
              {
                function = TRANSFORM_HANDLE_N;
              }
            else if (picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_E],
                                           coords->x, coords->y))
              {
                function = TRANSFORM_HANDLE_E;
              }
            else if (picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_S],
                                           coords->x, coords->y))
              {
                function = TRANSFORM_HANDLE_S;
              }
            else if (picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_W],
                                           coords->x, coords->y))
              {
                function = TRANSFORM_HANDLE_W;
              }
          }
    }

  if (tr_tool->use_pivot)
    {
      if (picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_PIVOT],
                                coords->x, coords->y))
        {
          function = TRANSFORM_HANDLE_PIVOT;
        }
    }

  if (tr_tool->use_center &&
      picman_canvas_item_hit (tr_tool->handles[TRANSFORM_HANDLE_CENTER],
                            coords->x, coords->y))
    {
      function = TRANSFORM_HANDLE_CENTER;
    }

  return function;
}

static void
picman_transform_tool_oper_update (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 gboolean          proximity,
                                 PicmanDisplay      *display)
{
  PicmanTransformTool *tr_tool   = PICMAN_TRANSFORM_TOOL (tool);
  PicmanDrawTool      *draw_tool = PICMAN_DRAW_TOOL (tool);
  TransformAction    function  = TRANSFORM_HANDLE_NONE;

  if (display != tool->display || draw_tool->item == NULL)
    {
      picman_transform_tool_set_function (tr_tool, function);
      return;
    }

  function = PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->pick_function (tr_tool, coords, state, display);

  picman_transform_tool_set_function (tr_tool, function);
}

static void
picman_transform_tool_cursor_update (PicmanTool         *tool,
                                   const PicmanCoords *coords,
                                   GdkModifierType   state,
                                   PicmanDisplay      *display)
{
  PicmanTransformTool    *tr_tool = PICMAN_TRANSFORM_TOOL (tool);
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tool);
  PicmanCursorType        cursor;
  PicmanCursorModifier    modifier = PICMAN_CURSOR_MODIFIER_NONE;
  PicmanImage            *image    = picman_display_get_image (display);

  cursor = picman_tool_control_get_cursor (tool->control);

  if (tr_tool->use_handles)
    {
      switch (tr_tool->function)
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

        case TRANSFORM_HANDLE_N:
          cursor = PICMAN_CURSOR_SIDE_TOP;
          break;

        case TRANSFORM_HANDLE_E:
          cursor = PICMAN_CURSOR_SIDE_RIGHT;
          break;

        case TRANSFORM_HANDLE_S:
          cursor = PICMAN_CURSOR_SIDE_BOTTOM;
          break;

        case TRANSFORM_HANDLE_W:
          cursor = PICMAN_CURSOR_SIDE_LEFT;
          break;

        default:
          cursor = PICMAN_CURSOR_CROSSHAIR_SMALL;
          break;
        }
    }

  if (tr_tool->use_center && tr_tool->function == TRANSFORM_HANDLE_CENTER)
    {
      modifier = PICMAN_CURSOR_MODIFIER_MOVE;
    }

  if (PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->cursor_update)
    {
      PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->cursor_update (tr_tool, &cursor, &modifier);
    }

  switch (options->type)
    {
      PicmanDrawable *drawable = NULL;
      PicmanVectors  *vectors  = NULL;

    case PICMAN_TRANSFORM_TYPE_LAYER:
      drawable = picman_image_get_active_drawable (image);
      if (picman_item_is_content_locked (PICMAN_ITEM (drawable)) ||
          picman_item_is_position_locked (PICMAN_ITEM (drawable)))
        modifier = PICMAN_CURSOR_MODIFIER_BAD;
      break;

    case PICMAN_TRANSFORM_TYPE_SELECTION:
      break;

    case PICMAN_TRANSFORM_TYPE_PATH:
      vectors = picman_image_get_active_vectors (image);
      if (! vectors ||
          picman_item_is_content_locked (PICMAN_ITEM (vectors)) ||
          picman_item_is_position_locked (PICMAN_ITEM (vectors)))
        modifier = PICMAN_CURSOR_MODIFIER_BAD;
      break;
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_transform_tool_options_notify (PicmanTool         *tool,
                                    PicmanToolOptions  *options,
                                    const GParamSpec *pspec)
{
  PicmanTransformTool *tr_tool = PICMAN_TRANSFORM_TOOL (tool);

  PICMAN_TOOL_CLASS (parent_class)->options_notify (tool, options, pspec);

  if (tr_tool->use_grid)
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tr_tool));

      if (! strcmp (pspec->name, "type") ||
          ! strcmp (pspec->name, "direction"))
        {
          if (tr_tool->function != TRANSFORM_CREATING)
            {
              if (tool->display)
                {
                  /*  reget the selection bounds  */
                  picman_transform_tool_bounds (tr_tool, tool->display);

                  /*  recalculate the tool's transformation matrix  */
                  picman_transform_tool_recalc_matrix (tr_tool);
                }
            }
        }

      if (tr_tool->function != TRANSFORM_CREATING)
        {
          picman_transform_tool_transform_bounding_box (tr_tool);
        }

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tr_tool));
    }

  if (g_str_has_prefix (pspec->name, "constrain-") ||
      g_str_has_prefix (pspec->name, "frompivot-") ||
      ! strcmp (pspec->name, "fixedpivot") ||
      ! strcmp (pspec->name, "cornersnap"))
    {
      picman_transform_tool_dialog_update (tr_tool);
    }
}

static void
picman_transform_tool_real_draw_gui (PicmanTransformTool *tr_tool, gint handle_w, gint handle_h)
{
  PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tr_tool);

  if (tr_tool->use_handles)
    {
      /*  draw the tool handles  */
      tr_tool->handles[TRANSFORM_HANDLE_NW] =
        picman_draw_tool_add_handle (draw_tool,
                                   PICMAN_HANDLE_SQUARE,
                                   tr_tool->tx1, tr_tool->ty1,
                                   handle_w, handle_h,
                                   PICMAN_HANDLE_ANCHOR_CENTER);

      tr_tool->handles[TRANSFORM_HANDLE_NE] =
        picman_draw_tool_add_handle (draw_tool,
                                   PICMAN_HANDLE_SQUARE,
                                   tr_tool->tx2, tr_tool->ty2,
                                   handle_w, handle_h,
                                   PICMAN_HANDLE_ANCHOR_CENTER);

      tr_tool->handles[TRANSFORM_HANDLE_SW] =
        picman_draw_tool_add_handle (draw_tool,
                                   PICMAN_HANDLE_SQUARE,
                                   tr_tool->tx3, tr_tool->ty3,
                                   handle_w, handle_h,
                                   PICMAN_HANDLE_ANCHOR_CENTER);

      tr_tool->handles[TRANSFORM_HANDLE_SE] =
        picman_draw_tool_add_handle (draw_tool,
                                   PICMAN_HANDLE_SQUARE,
                                   tr_tool->tx4, tr_tool->ty4,
                                   handle_w, handle_h,
                                   PICMAN_HANDLE_ANCHOR_CENTER);

      if (tr_tool->use_mid_handles)
        {
          gdouble x, y;

          x = (tr_tool->tx1 + tr_tool->tx2) / 2.0;
          y = (tr_tool->ty1 + tr_tool->ty2) / 2.0;

          tr_tool->handles[TRANSFORM_HANDLE_N] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_SQUARE,
                                       x, y,
                                       handle_w, handle_h,
                                       PICMAN_HANDLE_ANCHOR_CENTER);

          x = (tr_tool->tx2 + tr_tool->tx4) / 2.0;
          y = (tr_tool->ty2 + tr_tool->ty4) / 2.0;

          tr_tool->handles[TRANSFORM_HANDLE_E] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_SQUARE,
                                       x, y,
                                       handle_w, handle_h,
                                       PICMAN_HANDLE_ANCHOR_CENTER);

          x = (tr_tool->tx3 + tr_tool->tx4) / 2.0;
          y = (tr_tool->ty3 + tr_tool->ty4) / 2.0;

          tr_tool->handles[TRANSFORM_HANDLE_S] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_SQUARE,
                                       x, y,
                                       handle_w, handle_h,
                                       PICMAN_HANDLE_ANCHOR_CENTER);

          x = (tr_tool->tx3 + tr_tool->tx1) / 2.0;
          y = (tr_tool->ty3 + tr_tool->ty1) / 2.0;

          tr_tool->handles[TRANSFORM_HANDLE_W] =
            picman_draw_tool_add_handle (draw_tool,
                                       PICMAN_HANDLE_SQUARE,
                                       x, y,
                                       handle_w, handle_h,
                                       PICMAN_HANDLE_ANCHOR_CENTER);
        }
    }

  if (tr_tool->use_pivot)
    {
      PicmanCanvasGroup *stroke_group;
      gint d = MIN (handle_w, handle_h);
      if (tr_tool->use_center)
        d *= 2; /* so you can grab it from under the center handle */

      stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

      tr_tool->handles[TRANSFORM_HANDLE_PIVOT] = PICMAN_CANVAS_ITEM (stroke_group);

      picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_CIRCLE,
                                 tr_tool->tpx, tr_tool->tpy,
                                 d, d,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_CROSS,
                                 tr_tool->tpx, tr_tool->tpy,
                                 d, d,
                                 PICMAN_HANDLE_ANCHOR_CENTER);

      picman_draw_tool_pop_group (draw_tool);
    }

  /*  draw the center  */
  if (tr_tool->use_center)
    {
      PicmanCanvasGroup *stroke_group;
      gint             d = MIN (handle_w, handle_h);

      stroke_group = picman_draw_tool_add_stroke_group (draw_tool);

      tr_tool->handles[TRANSFORM_HANDLE_CENTER] = PICMAN_CANVAS_ITEM (stroke_group);

      picman_draw_tool_push_group (draw_tool, stroke_group);

      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_SQUARE,
                                 tr_tool->tcx, tr_tool->tcy,
                                 d, d,
                                 PICMAN_HANDLE_ANCHOR_CENTER);
      picman_draw_tool_add_handle (draw_tool,
                                 PICMAN_HANDLE_CROSS,
                                 tr_tool->tcx, tr_tool->tcy,
                                 d, d,
                                 PICMAN_HANDLE_ANCHOR_CENTER);

      picman_draw_tool_pop_group (draw_tool);
    }
}

static void
picman_transform_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanTool             *tool    = PICMAN_TOOL (draw_tool);
  PicmanTransformTool    *tr_tool = PICMAN_TRANSFORM_TOOL (draw_tool);
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tool);
  PicmanImage            *image   = picman_display_get_image (tool->display);
  gint                  handle_w;
  gint                  handle_h;
  gint                  i;

  for (i = 0; i < G_N_ELEMENTS (tr_tool->handles); i++)
    tr_tool->handles[i] = NULL;

  if (tr_tool->use_grid)
    {
      if (picman_transform_options_show_preview (options))
        {
          PicmanMatrix3 matrix = tr_tool->transform;

          if (options->direction == PICMAN_TRANSFORM_BACKWARD)
            picman_matrix3_invert (&matrix);

          picman_draw_tool_add_transform_preview (draw_tool,
                                                tool->drawable,
                                                &matrix,
                                                tr_tool->x1,
                                                tr_tool->y1,
                                                tr_tool->x2,
                                                tr_tool->y2,
                                                PICMAN_IS_PERSPECTIVE_TOOL (tr_tool) ||
                                                PICMAN_IS_UNIFIED_TRANSFORM_TOOL (tr_tool),
                                                options->preview_opacity);
        }

      picman_draw_tool_add_transform_guides (draw_tool,
                                           &tr_tool->transform,
                                           options->grid_type,
                                           options->grid_size,
                                           tr_tool->x1,
                                           tr_tool->y1,
                                           tr_tool->x2,
                                           tr_tool->y2);
    }

  picman_transform_tool_handles_recalc (tr_tool, tool->display,
                                      &handle_w, &handle_h);

  PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->draw_gui (tr_tool, handle_w, handle_h);

  if (tr_tool->handles[tr_tool->function])
    {
      picman_canvas_item_set_highlight (tr_tool->handles[tr_tool->function],
                                      TRUE);
    }

  if (options->type == PICMAN_TRANSFORM_TYPE_SELECTION)
    {
      PicmanMatrix3         matrix = tr_tool->transform;
      const PicmanBoundSeg *orig_in;
      const PicmanBoundSeg *orig_out;
      PicmanBoundSeg       *segs_in;
      PicmanBoundSeg       *segs_out;
      gint                num_segs_in;
      gint                num_segs_out;

      if (options->direction == PICMAN_TRANSFORM_BACKWARD)
        picman_matrix3_invert (&matrix);

      picman_channel_boundary (picman_image_get_mask (image),
                             &orig_in, &orig_out,
                             &num_segs_in, &num_segs_out,
                             0, 0, 0, 0);

      segs_in  = g_memdup (orig_in,  num_segs_in  * sizeof (PicmanBoundSeg));
      segs_out = g_memdup (orig_out, num_segs_out * sizeof (PicmanBoundSeg));

      if (segs_in)
        {
          for (i = 0; i < num_segs_in; i++)
            {
              gdouble tx, ty;

              picman_matrix3_transform_point (&matrix,
                                            segs_in[i].x1, segs_in[i].y1,
                                            &tx, &ty);
              segs_in[i].x1 = RINT (tx);
              segs_in[i].y1 = RINT (ty);

              picman_matrix3_transform_point (&matrix,
                                            segs_in[i].x2, segs_in[i].y2,
                                            &tx, &ty);
              segs_in[i].x2 = RINT (tx);
              segs_in[i].y2 = RINT (ty);
            }

          picman_draw_tool_add_boundary (draw_tool,
                                       segs_in, num_segs_in,
                                       NULL,
                                       0, 0);
          g_free (segs_in);
        }

      if (segs_out)
        {
          for (i = 0; i < num_segs_out; i++)
            {
              gdouble tx, ty;

              picman_matrix3_transform_point (&matrix,
                                            segs_out[i].x1, segs_out[i].y1,
                                            &tx, &ty);
              segs_out[i].x1 = RINT (tx);
              segs_out[i].y1 = RINT (ty);

              picman_matrix3_transform_point (&matrix,
                                            segs_out[i].x2, segs_out[i].y2,
                                            &tx, &ty);
              segs_out[i].x2 = RINT (tx);
              segs_out[i].y2 = RINT (ty);
            }

          picman_draw_tool_add_boundary (draw_tool,
                                       segs_out, num_segs_out,
                                       NULL,
                                       0, 0);
          g_free (segs_out);
        }
    }
  else if (options->type == PICMAN_TRANSFORM_TYPE_PATH)
    {
      PicmanVectors *vectors;
      PicmanStroke  *stroke = NULL;
      PicmanMatrix3  matrix = tr_tool->transform;

      vectors = picman_image_get_active_vectors (image);

      if (vectors)
        {
          if (options->direction == PICMAN_TRANSFORM_BACKWARD)
            picman_matrix3_invert (&matrix);

          while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
            {
              GArray   *coords;
              gboolean  closed;

              coords = picman_stroke_interpolate (stroke, 1.0, &closed);

              if (coords && coords->len)
                {
                  gint i;

                  for (i = 0; i < coords->len; i++)
                    {
                      PicmanCoords *curr = &g_array_index (coords, PicmanCoords, i);

                      picman_matrix3_transform_point (&matrix,
                                                    curr->x, curr->y,
                                                    &curr->x, &curr->y);
                    }

                  picman_draw_tool_add_strokes (draw_tool,
                                              &g_array_index (coords,
                                                              PicmanCoords, 0),
                                              coords->len, FALSE);
                }

              if (coords)
                g_array_free (coords, TRUE);
            }
        }
    }
}

static void
picman_transform_tool_dialog_update (PicmanTransformTool *tr_tool)
{
  if (tr_tool->dialog &&
      PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->dialog_update)
    {
      PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->dialog_update (tr_tool);
    }
}

static GeglBuffer *
picman_transform_tool_real_transform (PicmanTransformTool *tr_tool,
                                    PicmanItem          *active_item,
                                    GeglBuffer        *orig_buffer,
                                    gint               orig_offset_x,
                                    gint               orig_offset_y,
                                    gint              *new_offset_x,
                                    gint              *new_offset_y)
{
  PicmanTool             *tool    = PICMAN_TOOL (tr_tool);
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tool);
  PicmanContext          *context = PICMAN_CONTEXT (options);
  GeglBuffer           *ret     = NULL;
  PicmanTransformResize   clip    = options->clip;
  PicmanProgress         *progress;

  progress = picman_progress_start (PICMAN_PROGRESS (tool),
                                  tr_tool->progress_text, FALSE);

  if (picman_item_get_linked (active_item))
    picman_item_linked_transform (active_item, context,
                                &tr_tool->transform,
                                options->direction,
                                options->interpolation,
                                options->recursion_level,
                                clip,
                                progress);

  if (orig_buffer)
    {
      /*  this happens when transforming a selection cut out of a
       *  normal drawable, or the selection
       */

      /*  always clip the selction and unfloated channels
       *  so they keep their size
       */
      if (PICMAN_IS_CHANNEL (active_item) &&
          ! babl_format_has_alpha (gegl_buffer_get_format (orig_buffer)))
        clip = PICMAN_TRANSFORM_RESIZE_CLIP;

      ret = picman_drawable_transform_buffer_affine (PICMAN_DRAWABLE (active_item),
                                                   context,
                                                   orig_buffer,
                                                   orig_offset_x,
                                                   orig_offset_y,
                                                   &tr_tool->transform,
                                                   options->direction,
                                                   options->interpolation,
                                                   options->recursion_level,
                                                   clip,
                                                   new_offset_x,
                                                   new_offset_y,
                                                   progress);
    }
  else
    {
      /*  this happens for entire drawables, paths and layer groups  */

      /*  always clip layer masks so they keep their size
       */
      if (PICMAN_IS_CHANNEL (active_item))
        clip = PICMAN_TRANSFORM_RESIZE_CLIP;

      picman_item_transform (active_item, context,
                           &tr_tool->transform,
                           options->direction,
                           options->interpolation,
                           options->recursion_level,
                           clip,
                           progress);
    }

  if (progress)
    picman_progress_end (progress);

  return ret;
}

static void
picman_transform_tool_transform (PicmanTransformTool *tr_tool,
                               PicmanDisplay       *display)
{
  PicmanTool             *tool           = PICMAN_TOOL (tr_tool);
  PicmanTransformOptions *options        = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tool);
  PicmanContext          *context        = PICMAN_CONTEXT (options);
  PicmanImage            *image          = picman_display_get_image (display);
  PicmanItem             *active_item    = NULL;
  GeglBuffer           *orig_buffer    = NULL;
  gint                  orig_offset_x;
  gint                  orig_offset_y;
  GeglBuffer           *new_buffer;
  gint                  new_offset_x;
  gint                  new_offset_y;
  const gchar          *null_message   = NULL;
  const gchar          *locked_message = NULL;
  gchar                *undo_desc      = NULL;
  gboolean              new_layer;

  switch (options->type)
    {
    case PICMAN_TRANSFORM_TYPE_LAYER:
      active_item  = PICMAN_ITEM (picman_image_get_active_drawable (image));
      null_message = _("There is no layer to transform.");

      if (picman_item_is_content_locked (active_item))
        locked_message = _("The active layer's pixels are locked.");
      else
        locked_message = _("The active layer's position and size are locked.");
      break;

    case PICMAN_TRANSFORM_TYPE_SELECTION:
      active_item  = PICMAN_ITEM (picman_image_get_mask (image));
      /* cannot happen, so don't translate these messages */
      null_message = "There is no selection to transform.";

      if (picman_item_is_content_locked (active_item))
        locked_message = "The selection's pixels are locked.";
      else
        locked_message = "The selection's position and size are locked.";
      break;

    case PICMAN_TRANSFORM_TYPE_PATH:
      active_item  = PICMAN_ITEM (picman_image_get_active_vectors (image));
      null_message = _("There is no path to transform.");

      if (picman_item_is_content_locked (active_item))
        locked_message = _("The active path's strokes are locked.");
      else
        locked_message = _("The active path's position is locked.");
      break;
    }

  if (! active_item)
    {
      picman_tool_message_literal (tool, display, null_message);
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
      return;
    }

  if (picman_item_is_content_locked (active_item) ||
      picman_item_is_position_locked (active_item))
    {
      picman_tool_message_literal (tool, display, locked_message);
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
      return;
    }

  if (tr_tool->dialog)
    picman_dialog_factory_hide_dialog (tr_tool->dialog);

  picman_set_busy (display->picman);

  /* undraw the tool before we muck around with the transform matrix */
  picman_draw_tool_stop (PICMAN_DRAW_TOOL (tr_tool));

  /*  We're going to dirty this image, but we want to keep the tool around  */
  picman_tool_control_push_preserve (tool->control, TRUE);

  undo_desc = PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->get_undo_desc (tr_tool);
  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM, undo_desc);
  g_free (undo_desc);

  switch (options->type)
    {
    case PICMAN_TRANSFORM_TYPE_LAYER:
      if (! picman_viewable_get_children (PICMAN_VIEWABLE (tool->drawable)) &&
          ! picman_channel_is_empty (picman_image_get_mask (image)))
        {
          orig_buffer = picman_drawable_transform_cut (tool->drawable,
                                                     context,
                                                     &orig_offset_x,
                                                     &orig_offset_y,
                                                     &new_layer);
        }
      break;

    case PICMAN_TRANSFORM_TYPE_SELECTION:
      orig_buffer = g_object_ref (picman_drawable_get_buffer (PICMAN_DRAWABLE (active_item)));
      orig_offset_x = 0;
      orig_offset_y = 0;
      break;

    case PICMAN_TRANSFORM_TYPE_PATH:
      break;
    }

  /*  Send the request for the transformation to the tool...
   */
  new_buffer = PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->transform (tr_tool,
                                                                   active_item,
                                                                   orig_buffer,
                                                                   orig_offset_x,
                                                                   orig_offset_y,
                                                                   &new_offset_x,
                                                                   &new_offset_y);

  if (orig_buffer)
    g_object_unref (orig_buffer);

  switch (options->type)
    {
    case PICMAN_TRANSFORM_TYPE_LAYER:
      if (new_buffer)
        {
          /*  paste the new transformed image to the image...also implement
           *  undo...
           */
          picman_drawable_transform_paste (tool->drawable, new_buffer,
                                         new_offset_x, new_offset_y,
                                         new_layer);
          g_object_unref (new_buffer);
        }
      break;

     case PICMAN_TRANSFORM_TYPE_SELECTION:
      if (new_buffer)
        {
          picman_channel_push_undo (PICMAN_CHANNEL (active_item), NULL);

          picman_drawable_set_buffer (PICMAN_DRAWABLE (active_item),
                                    FALSE, NULL, new_buffer);
          g_object_unref (new_buffer);
        }
      break;

    case PICMAN_TRANSFORM_TYPE_PATH:
      /*  Nothing to be done  */
      break;
    }

  picman_image_undo_push (image, PICMAN_TYPE_TRANSFORM_TOOL_UNDO,
                        PICMAN_UNDO_TRANSFORM, NULL,
                        0,
                        "transform-tool", tr_tool,
                        NULL);

  picman_image_undo_group_end (image);

  /*  We're done dirtying the image, and would like to be restarted if
   *  the image gets dirty while the tool exists
   */
  picman_tool_control_pop_preserve (tool->control);

  picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);

  picman_unset_busy (display->picman);

  picman_image_flush (image);
}

static void
picman_transform_tool_set_function (PicmanTransformTool *tr_tool,
                                  TransformAction    function)
{
  if (function != tr_tool->function)
    {
      if (tr_tool->handles[tr_tool->function] &&
          picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tr_tool)))
        {
          picman_canvas_item_set_highlight (tr_tool->handles[tr_tool->function],
                                          FALSE);
        }

      tr_tool->function = function;

      if (tr_tool->handles[tr_tool->function] &&
          picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tr_tool)))
        {
          picman_canvas_item_set_highlight (tr_tool->handles[tr_tool->function],
                                          TRUE);
        }
    }
}

static void
picman_transform_tool_transform_bounding_box (PicmanTransformTool *tr_tool)
{
  g_return_if_fail (PICMAN_IS_TRANSFORM_TOOL (tr_tool));

  picman_matrix3_transform_point (&tr_tool->transform,
                                tr_tool->x1, tr_tool->y1,
                                &tr_tool->tx1, &tr_tool->ty1);
  picman_matrix3_transform_point (&tr_tool->transform,
                                tr_tool->x2, tr_tool->y1,
                                &tr_tool->tx2, &tr_tool->ty2);
  picman_matrix3_transform_point (&tr_tool->transform,
                                tr_tool->x1, tr_tool->y2,
                                &tr_tool->tx3, &tr_tool->ty3);
  picman_matrix3_transform_point (&tr_tool->transform,
                                tr_tool->x2, tr_tool->y2,
                                &tr_tool->tx4, &tr_tool->ty4);
  picman_matrix3_transform_point (&tr_tool->transform,
                                tr_tool->px, tr_tool->py,
                                &tr_tool->tpx, &tr_tool->tpy);

  /* don't transform these */
  tr_tool->tpx = tr_tool->px;
  tr_tool->tpy = tr_tool->py;

  tr_tool->tcx = (tr_tool->tx1 +
                  tr_tool->tx2 +
                  tr_tool->tx3 +
                  tr_tool->tx4) / 4.0;
  tr_tool->tcy = (tr_tool->ty1 +
                  tr_tool->ty2 +
                  tr_tool->ty3 +
                  tr_tool->ty4) / 4.0;
}

static void
picman_transform_tool_bounds (PicmanTransformTool *tr_tool,
                            PicmanDisplay       *display)
{
  PicmanTransformOptions *options = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);
  PicmanImage            *image   = picman_display_get_image (display);

  switch (options->type)
    {
    case PICMAN_TRANSFORM_TYPE_LAYER:
      {
        PicmanDrawable *drawable;
        gint          offset_x;
        gint          offset_y;

        drawable = picman_image_get_active_drawable (image);

        picman_item_get_offset (PICMAN_ITEM (drawable), &offset_x, &offset_y);

        picman_item_mask_bounds (PICMAN_ITEM (drawable),
                               &tr_tool->x1, &tr_tool->y1,
                               &tr_tool->x2, &tr_tool->y2);
        tr_tool->x1 += offset_x;
        tr_tool->y1 += offset_y;
        tr_tool->x2 += offset_x;
        tr_tool->y2 += offset_y;
      }
      break;

    case PICMAN_TRANSFORM_TYPE_SELECTION:
    case PICMAN_TRANSFORM_TYPE_PATH:
      picman_channel_bounds (picman_image_get_mask (image),
                           &tr_tool->x1, &tr_tool->y1,
                           &tr_tool->x2, &tr_tool->y2);
      break;
    }

  tr_tool->aspect = ((gdouble) (tr_tool->x2 - tr_tool->x1) /
                     (gdouble) (tr_tool->y2 - tr_tool->y1));
}

static void
picman_transform_tool_handles_recalc (PicmanTransformTool *tr_tool,
                                    PicmanDisplay       *display,
                                    gint              *handle_w,
                                    gint              *handle_h)
{
  gint dx1, dy1;
  gint dx2, dy2;
  gint dx3, dy3;
  gint dx4, dy4;
  gint x1, y1;
  gint x2, y2;

  picman_display_shell_transform_xy (picman_display_get_shell (display),
                                   tr_tool->tx1, tr_tool->ty1,
                                   &dx1, &dy1);
  picman_display_shell_transform_xy (picman_display_get_shell (display),
                                   tr_tool->tx2, tr_tool->ty2,
                                   &dx2, &dy2);
  picman_display_shell_transform_xy (picman_display_get_shell (display),
                                   tr_tool->tx3, tr_tool->ty3,
                                   &dx3, &dy3);
  picman_display_shell_transform_xy (picman_display_get_shell (display),
                                   tr_tool->tx4, tr_tool->ty4,
                                   &dx4, &dy4);

  x1 = MIN4 (dx1, dx2, dx3, dx4);
  y1 = MIN4 (dy1, dy2, dy3, dy4);
  x2 = MAX4 (dx1, dx2, dx3, dx4);
  y2 = MAX4 (dy1, dy2, dy3, dy4);

  *handle_w = CLAMP ((x2 - x1) / 3,
                     MIN_HANDLE_SIZE, PICMAN_TOOL_HANDLE_SIZE_LARGE);
  *handle_h = CLAMP ((y2 - y1) / 3,
                     MIN_HANDLE_SIZE, PICMAN_TOOL_HANDLE_SIZE_LARGE);
}

static void
picman_transform_tool_dialog (PicmanTransformTool *tr_tool)
{
  PicmanTool     *tool      = PICMAN_TOOL (tr_tool);
  PicmanToolInfo *tool_info = tool->tool_info;
  const gchar  *stock_id;

  if (! PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->dialog)
    return;

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info));

  tr_tool->dialog = picman_tool_dialog_new (tool_info,
                                          picman_display_get_shell (tool->display),
                                          tool_info->blurb,
                                          PICMAN_STOCK_RESET, RESPONSE_RESET,
                                          GTK_STOCK_UNDO, RESPONSE_UNDO,
                                          GTK_STOCK_REDO, RESPONSE_REDO,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          stock_id,         GTK_RESPONSE_OK,
                                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (tr_tool->dialog),
                                   GTK_RESPONSE_OK);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (tr_tool->dialog),
                                           RESPONSE_RESET,
                                           RESPONSE_UNDO,
                                           RESPONSE_REDO,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (tr_tool->dialog, "response",
                    G_CALLBACK (picman_transform_tool_response),
                    tr_tool);

  PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->dialog (tr_tool);
}

static void
picman_transform_tool_prepare (PicmanTransformTool *tr_tool,
                             PicmanDisplay       *display)
{
  if (tr_tool->dialog)
    {
      PicmanTransformOptions *options  = PICMAN_TRANSFORM_TOOL_GET_OPTIONS (tr_tool);
      PicmanImage            *image    = picman_display_get_image (display);
      PicmanDrawable         *drawable = picman_image_get_active_drawable (image);

      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (tr_tool->dialog),
                                         PICMAN_VIEWABLE (drawable),
                                         PICMAN_CONTEXT (options));
      picman_tool_dialog_set_shell (PICMAN_TOOL_DIALOG (tr_tool->dialog),
                                  picman_display_get_shell (display));
    }

  if (PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->prepare)
    PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->prepare (tr_tool);
}

void
picman_transform_tool_recalc_matrix (PicmanTransformTool *tr_tool)
{
  g_return_if_fail (PICMAN_IS_TRANSFORM_TOOL (tr_tool));

  if (PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->recalc_matrix)
    PICMAN_TRANSFORM_TOOL_GET_CLASS (tr_tool)->recalc_matrix (tr_tool);

  picman_transform_tool_transform_bounding_box (tr_tool);

  picman_transform_tool_dialog_update (tr_tool);

  if (tr_tool->dialog)
    gtk_widget_show (tr_tool->dialog);
}

static void
picman_transform_tool_response (GtkWidget         *widget,
                              gint               response_id,
                              PicmanTransformTool *tr_tool)
{
  PicmanTool *tool = PICMAN_TOOL (tr_tool);
  GList    *it   = tr_tool->redo_list;
  gint      i;

  switch (response_id)
    {
    case RESPONSE_UNDO:
      {
        it = g_list_next (tr_tool->undo_list);

    case RESPONSE_REDO:
        if (it)
          {
    case RESPONSE_RESET:
            if (response_id == RESPONSE_UNDO)
              {
                /* Move prev_trans_info from undo_list to redo_list */
                tr_tool->redo_list = g_list_prepend (tr_tool->redo_list,
                                                     tr_tool->prev_trans_info);
                tr_tool->undo_list = g_list_remove (tr_tool->undo_list,
                                                    tr_tool->prev_trans_info);
                tr_tool->prev_trans_info = it->data;
              }
            else if (response_id == RESPONSE_REDO)
              {
                /* And the opposite */
                tr_tool->prev_trans_info = it->data;
                tr_tool->undo_list = g_list_prepend (tr_tool->undo_list,
                                                     tr_tool->prev_trans_info);
                tr_tool->redo_list = g_list_remove (tr_tool->redo_list,
                                                    tr_tool->prev_trans_info);
              }
            else if (response_id == RESPONSE_RESET)
              {
                /* Move all undo events to redo, and pop off the first
                 * one as that's the current one, which always sits on
                 * the undo_list
                 */
                tr_tool->redo_list =
                  g_list_remove (g_list_concat (g_list_reverse (tr_tool->undo_list),
                                                tr_tool->redo_list),
                                 tr_tool->old_trans_info);
                tr_tool->prev_trans_info = tr_tool->old_trans_info;
                tr_tool->undo_list = g_list_prepend (NULL,
                                                     tr_tool->prev_trans_info);
              }
            update_sensitivity (tr_tool);

            picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

            /*  Restore the previous transformation info  */
            for (i = 0; i < TRANS_INFO_SIZE; i++)
              {
                tr_tool->trans_info[i] = (*tr_tool->prev_trans_info)[i];
              }

            /*  reget the selection bounds  */
            picman_transform_tool_bounds (tr_tool, tool->display);

            /*  recalculate the tool's transformation matrix  */
            picman_transform_tool_recalc_matrix (tr_tool);

            picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
          }
      }
      break;

    case GTK_RESPONSE_OK:
      g_return_if_fail (tool->display != NULL);
      picman_transform_tool_transform (tr_tool, tool->display);
      break;

    default:
      picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);
      break;
    }
}

static void
free_trans (gpointer data)
{
  g_slice_free (TransInfo, data);
}

static void
update_sensitivity (PicmanTransformTool *tr_tool)
{
  if (!tr_tool->dialog)
    return;

  gtk_dialog_set_response_sensitive (GTK_DIALOG (tr_tool->dialog), RESPONSE_UNDO,
      g_list_next (tr_tool->undo_list) != NULL);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (tr_tool->dialog), RESPONSE_REDO,
      tr_tool->redo_list != NULL);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (tr_tool->dialog), RESPONSE_RESET,
      g_list_next (tr_tool->undo_list) != NULL);
}
