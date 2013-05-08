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

#include "core/picmanchannel.h"
#include "core/picmanchannel-select.h"
#include "core/picmanlayer-floating-sel.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanpickable.h"
#include "core/picman-utils.h"
#include "core/picmanundostack.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasgroup.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-transform.h"
#include "display/picmandisplayshell-appearance.h"

#include "picmaneditselectiontool.h"
#include "picmanselectiontool.h"
#include "picmanselectionoptions.h"
#include "picmanrectangletool.h"
#include "picmanrectangleoptions.h"
#include "picmanrectangleselecttool.h"
#include "picmanrectangleselectoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


typedef struct PicmanRectangleSelectToolPrivate
{
  PicmanChannelOps     operation;            /* remember for use when modifying   */
  gboolean           use_saved_op;         /* use operation or get from options */
  gboolean           saved_show_selection; /* used to remember existing value   */
  PicmanUndo          *undo;
  PicmanUndo          *redo;

  gboolean           round_corners;
  gdouble            corner_radius;

  gdouble            press_x;
  gdouble            press_y;
} PicmanRectangleSelectToolPrivate;


#define PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE(obj) \
  ((PicmanRectangleSelectToolPrivate *) ((PicmanRectangleSelectTool *) (obj))->priv)


static void     picman_rectangle_select_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface);

static void     picman_rectangle_select_tool_constructed    (GObject               *object);

static void     picman_rectangle_select_tool_control        (PicmanTool              *tool,
                                                           PicmanToolAction         action,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_button_press   (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           guint32                time,
                                                           GdkModifierType        state,
                                                           PicmanButtonPressType    press_type,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_button_release (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           guint32                time,
                                                           GdkModifierType        state,
                                                           PicmanButtonReleaseType  release_type,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_active_modifier_key
                                                          (PicmanTool              *tool,
                                                           GdkModifierType        key,
                                                           gboolean               press,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static gboolean picman_rectangle_select_tool_key_press      (PicmanTool              *tool,
                                                           GdkEventKey           *kevent,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_oper_update    (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           GdkModifierType        state,
                                                           gboolean               proximity,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_cursor_update  (PicmanTool              *tool,
                                                           const PicmanCoords      *coords,
                                                           GdkModifierType        state,
                                                           PicmanDisplay           *display);
static void     picman_rectangle_select_tool_draw           (PicmanDrawTool          *draw_tool);
static gboolean picman_rectangle_select_tool_select         (PicmanRectangleTool     *rect_tool,
                                                           gint                   x,
                                                           gint                   y,
                                                           gint                   w,
                                                           gint                   h);
static gboolean picman_rectangle_select_tool_execute        (PicmanRectangleTool     *rect_tool,
                                                           gint                   x,
                                                           gint                   y,
                                                           gint                   w,
                                                           gint                   h);
static void     picman_rectangle_select_tool_cancel         (PicmanRectangleTool     *rect_tool);
static gboolean picman_rectangle_select_tool_rectangle_change_complete
                                                          (PicmanRectangleTool     *rect_tool);
static void     picman_rectangle_select_tool_real_select    (PicmanRectangleSelectTool *rect_sel_tool,
                                                           PicmanChannelOps         operation,
                                                           gint                   x,
                                                           gint                   y,
                                                           gint                   w,
                                                           gint                   h);
static PicmanChannelOps
                picman_rectangle_select_tool_get_operation  (PicmanRectangleSelectTool    *rect_sel_tool);
static void     picman_rectangle_select_tool_update_option_defaults
                                                          (PicmanRectangleSelectTool    *rect_sel_tool,
                                                           gboolean                    ignore_pending);

static void     picman_rectangle_select_tool_round_corners_notify
                                                          (PicmanRectangleSelectOptions *options,
                                                           GParamSpec                 *pspec,
                                                           PicmanRectangleSelectTool    *rect_sel_tool);


G_DEFINE_TYPE_WITH_CODE (PicmanRectangleSelectTool, picman_rectangle_select_tool,
                         PICMAN_TYPE_SELECTION_TOOL,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_RECTANGLE_TOOL,
                                                picman_rectangle_select_tool_rectangle_tool_iface_init))

#define parent_class picman_rectangle_select_tool_parent_class


void
picman_rectangle_select_tool_register (PicmanToolRegisterCallback  callback,
                                     gpointer                  data)
{
  (* callback) (PICMAN_TYPE_RECTANGLE_SELECT_TOOL,
                PICMAN_TYPE_RECTANGLE_SELECT_OPTIONS,
                picman_rectangle_select_options_gui,
                0,
                "picman-rect-select-tool",
                _("Rectangle Select"),
                _("Rectangle Select Tool: Select a rectangular region"),
                N_("_Rectangle Select"), "R",
                NULL, PICMAN_HELP_TOOL_RECT_SELECT,
                PICMAN_STOCK_TOOL_RECT_SELECT,
                data);
}

static void
picman_rectangle_select_tool_class_init (PicmanRectangleSelectToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  g_type_class_add_private (klass, sizeof (PicmanRectangleSelectToolPrivate));

  object_class->constructed       = picman_rectangle_select_tool_constructed;
  object_class->set_property      = picman_rectangle_tool_set_property;
  object_class->get_property      = picman_rectangle_tool_get_property;

  tool_class->control             = picman_rectangle_select_tool_control;
  tool_class->button_press        = picman_rectangle_select_tool_button_press;
  tool_class->button_release      = picman_rectangle_select_tool_button_release;
  tool_class->motion              = picman_rectangle_tool_motion;
  tool_class->key_press           = picman_rectangle_select_tool_key_press;
  tool_class->active_modifier_key = picman_rectangle_select_tool_active_modifier_key;
  tool_class->oper_update         = picman_rectangle_select_tool_oper_update;
  tool_class->cursor_update       = picman_rectangle_select_tool_cursor_update;

  draw_tool_class->draw           = picman_rectangle_select_tool_draw;

  klass->select                   = picman_rectangle_select_tool_real_select;

  picman_rectangle_tool_install_properties (object_class);
}

static void
picman_rectangle_select_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface)
{
  iface->execute                   = picman_rectangle_select_tool_execute;
  iface->cancel                    = picman_rectangle_select_tool_cancel;
  iface->rectangle_change_complete = picman_rectangle_select_tool_rectangle_change_complete;
}

static void
picman_rectangle_select_tool_init (PicmanRectangleSelectTool *rect_sel_tool)
{
  PicmanTool                       *tool = PICMAN_TOOL (rect_sel_tool);
  PicmanRectangleSelectToolPrivate *priv;

  picman_rectangle_tool_init (PICMAN_RECTANGLE_TOOL (rect_sel_tool));

  rect_sel_tool->priv = G_TYPE_INSTANCE_GET_PRIVATE (rect_sel_tool,
                                                     PICMAN_TYPE_RECTANGLE_SELECT_TOOL,
                                                     PicmanRectangleSelectToolPrivate);

  priv = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  picman_tool_control_set_wants_click (tool->control, TRUE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_PIXEL_BORDER);
  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_RECT_SELECT);
  picman_tool_control_set_preserve    (tool->control, FALSE);
  picman_tool_control_set_dirty_mask  (tool->control,
                                     PICMAN_DIRTY_IMAGE_SIZE |
                                     PICMAN_DIRTY_SELECTION);

  priv->undo    = NULL;
  priv->redo    = NULL;

  priv->press_x = 0.0;
  priv->press_y = 0.0;
}

static void
picman_rectangle_select_tool_constructed (GObject *object)
{
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectOptions     *options;
  PicmanRectangleSelectToolPrivate *priv;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_rectangle_tool_constructor (object);

  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (object);
  options       = PICMAN_RECTANGLE_SELECT_TOOL_GET_OPTIONS (rect_sel_tool);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  priv->round_corners = options->round_corners;
  priv->corner_radius = options->corner_radius;

  g_signal_connect_object (options, "notify::round-corners",
                           G_CALLBACK (picman_rectangle_select_tool_round_corners_notify),
                           object, 0);
  g_signal_connect_object (options, "notify::corner-radius",
                           G_CALLBACK (picman_rectangle_select_tool_round_corners_notify),
                           object, 0);

  picman_rectangle_select_tool_update_option_defaults (rect_sel_tool, FALSE);
}

static void
picman_rectangle_select_tool_control (PicmanTool       *tool,
                                    PicmanToolAction  action,
                                    PicmanDisplay    *display)
{
  picman_rectangle_tool_control (tool, action, display);

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_rectangle_select_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectToolPrivate *priv;
  PicmanCanvasGroup                *stroke_group = NULL;

  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (draw_tool);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  if (priv->round_corners)
    {
      PicmanCanvasItem *item;
      gint            x1, y1, x2, y2;
      gdouble         radius;
      gint            square_size;

      g_object_get (rect_sel_tool,
                    "x1", &x1,
                    "y1", &y1,
                    "x2", &x2,
                    "y2", &y2,
                    NULL);

      radius = MIN (priv->corner_radius,
                    MIN ((x2 - x1) / 2.0, (y2 - y1) / 2.0));

      square_size = (int) (radius * 2);

      stroke_group =
        PICMAN_CANVAS_GROUP (picman_draw_tool_add_stroke_group (draw_tool));

      item = picman_draw_tool_add_arc (draw_tool, FALSE,
                                     x1, y1,
                                     square_size, square_size,
                                     G_PI / 2.0, G_PI / 2.0);
      picman_canvas_group_add_item (stroke_group, item);
      picman_draw_tool_remove_item (draw_tool, item);

      item = picman_draw_tool_add_arc (draw_tool, FALSE,
                                     x2 - square_size, y1,
                                     square_size, square_size,
                                     0.0, G_PI / 2.0);
      picman_canvas_group_add_item (stroke_group, item);
      picman_draw_tool_remove_item (draw_tool, item);

      item = picman_draw_tool_add_arc (draw_tool, FALSE,
                                     x2 - square_size, y2 - square_size,
                                     square_size, square_size,
                                     G_PI * 1.5, G_PI / 2.0);
      picman_canvas_group_add_item (stroke_group, item);
      picman_draw_tool_remove_item (draw_tool, item);

      item = picman_draw_tool_add_arc (draw_tool, FALSE,
                                     x1, y2 - square_size,
                                     square_size, square_size,
                                     G_PI, G_PI / 2.0);
      picman_canvas_group_add_item (stroke_group, item);
      picman_draw_tool_remove_item (draw_tool, item);
    }

  picman_rectangle_tool_draw (draw_tool, stroke_group);
}

static void
picman_rectangle_select_tool_button_press (PicmanTool            *tool,
                                         const PicmanCoords    *coords,
                                         guint32              time,
                                         GdkModifierType      state,
                                         PicmanButtonPressType  press_type,
                                         PicmanDisplay         *display)
{
  PicmanRectangleTool              *rectangle;
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanDisplayShell               *shell;
  PicmanRectangleSelectToolPrivate *priv;
  PicmanRectangleFunction           function;

  rectangle     = PICMAN_RECTANGLE_TOOL (tool);
  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (tool);
  shell         = picman_display_get_shell (display);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  if (tool->display && display != tool->display)
    {
      picman_rectangle_tool_cancel (PICMAN_RECTANGLE_TOOL (tool));
    }

  if (picman_selection_tool_start_edit (PICMAN_SELECTION_TOOL (tool),
                                      display, coords))
    {
      /* In some cases we want to finish the rectangle select tool
       * and hand over responsibility to the selection tool
       */
      picman_rectangle_tool_execute (rectangle);
      picman_rectangle_tool_control (tool, PICMAN_TOOL_ACTION_HALT, display);
      picman_rectangle_select_tool_update_option_defaults (rect_sel_tool,
                                                         TRUE);
      return;
    }

  picman_tool_control_activate (tool->control);

  priv->saved_show_selection = picman_display_shell_get_show_selection (shell);

  /* if the shift or ctrl keys are down, we don't want to adjust, we
   * want to create a new rectangle, regardless of pointer loc */
  if (state & (picman_get_extend_selection_mask () |
               picman_get_modify_selection_mask ()))
    {
      picman_rectangle_tool_set_function (rectangle,
                                        PICMAN_RECTANGLE_TOOL_CREATING);
    }

  picman_rectangle_tool_button_press (tool, coords, time, state, display);

  priv->press_x = coords->x;
  priv->press_y = coords->y;

  /* if we have an existing rectangle in the current display, then
   * we have already "executed", and need to undo at this point,
   * unless the user has done something in the meantime
   */
  function = picman_rectangle_tool_get_function (rectangle);

  if (function == PICMAN_RECTANGLE_TOOL_CREATING)
    {
      priv->use_saved_op = FALSE;
    }
  else
    {
      PicmanImage      *image      = picman_display_get_image (tool->display);
      PicmanUndoStack  *undo_stack = picman_image_get_undo_stack (image);
      PicmanUndoStack  *redo_stack = picman_image_get_redo_stack (image);
      PicmanUndo       *undo;
      PicmanChannelOps  operation;

      undo = picman_undo_stack_peek (undo_stack);

      if (undo && priv->undo == undo)
        {
          /* prevent this change from halting the tool */
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_undo (image);

          picman_tool_control_pop_preserve (tool->control);

          /* we will need to redo if the user cancels or executes */
          priv->redo = picman_undo_stack_peek (redo_stack);
        }

      /* if the operation is "Replace", turn off the marching ants,
         because they are confusing */
      operation = picman_rectangle_select_tool_get_operation (rect_sel_tool);

      if (operation == PICMAN_CHANNEL_OP_REPLACE)
        picman_display_shell_set_show_selection (shell, FALSE);
    }

  priv->undo = NULL;
}

static void
picman_rectangle_select_tool_button_release (PicmanTool              *tool,
                                           const PicmanCoords      *coords,
                                           guint32                time,
                                           GdkModifierType        state,
                                           PicmanButtonReleaseType  release_type,
                                           PicmanDisplay           *display)
{
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectToolPrivate *priv;
  PicmanImage                      *image;

  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (tool);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  image = picman_display_get_image (tool->display);

  picman_tool_control_halt (tool->control);

  picman_tool_pop_status (tool, display);
  picman_display_shell_set_show_selection (picman_display_get_shell (display),
                                         priv->saved_show_selection);

  /*
   * if the user has not moved the mouse, we need to redo the operation
   * that was undone on button press.
   */
  if (release_type == PICMAN_BUTTON_RELEASE_CLICK)
    {
      PicmanUndoStack *redo_stack = picman_image_get_redo_stack (image);
      PicmanUndo      *redo       = picman_undo_stack_peek (redo_stack);

      if (redo && priv->redo == redo)
        {
          /* prevent this from halting the tool */
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_redo (image);
          priv->redo = NULL;

          picman_tool_control_pop_preserve (tool->control);
        }
    }

  picman_rectangle_tool_button_release (tool, coords, time, state, release_type,
                                      display);

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    {
      if (priv->redo)
        {
          /* prevent this from halting the tool */
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_redo (image);

          picman_tool_control_pop_preserve (tool->control);
        }

      priv->use_saved_op = TRUE;  /* is this correct? */
    }

  priv->redo = NULL;
}

static void
picman_rectangle_select_tool_active_modifier_key (PicmanTool        *tool,
                                                GdkModifierType  key,
                                                gboolean         press,
                                                GdkModifierType  state,
                                                PicmanDisplay     *display)
{
  PICMAN_TOOL_CLASS (parent_class)->active_modifier_key (tool, key, press, state,
                                                       display);

  picman_rectangle_tool_active_modifier_key (tool, key, press, state, display);
}

static gboolean
picman_rectangle_select_tool_key_press (PicmanTool    *tool,
                                      GdkEventKey *kevent,
                                      PicmanDisplay *display)
{
  return (picman_rectangle_tool_key_press (tool, kevent, display) ||
          picman_edit_selection_tool_key_press (tool, kevent, display));
}

static void
picman_rectangle_select_tool_oper_update (PicmanTool         *tool,
                                        const PicmanCoords *coords,
                                        GdkModifierType   state,
                                        gboolean          proximity,
                                        PicmanDisplay      *display)
{
  picman_rectangle_tool_oper_update (tool, coords, state, proximity, display);

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);
}

static void
picman_rectangle_select_tool_cursor_update (PicmanTool         *tool,
                                          const PicmanCoords *coords,
                                          GdkModifierType   state,
                                          PicmanDisplay      *display)
{
  picman_rectangle_tool_cursor_update (tool, coords, state, display);

  /* override the previous if shift or ctrl are down */
  if (state & (picman_get_extend_selection_mask () |
               picman_get_modify_selection_mask ()))
    {
      picman_tool_control_set_cursor (tool->control,
                                    PICMAN_CURSOR_CROSSHAIR_SMALL);
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}


static gboolean
picman_rectangle_select_tool_select (PicmanRectangleTool *rectangle,
                                   gint               x,
                                   gint               y,
                                   gint               w,
                                   gint               h)
{
  PicmanTool                *tool;
  PicmanRectangleSelectTool *rect_sel_tool;
  PicmanImage               *image;
  gboolean                 rectangle_exists;
  PicmanChannelOps           operation;

  tool          = PICMAN_TOOL (rectangle);
  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (rectangle);

  image         = picman_display_get_image (tool->display);

  picman_tool_pop_status (tool, tool->display);

  rectangle_exists = (x <= picman_image_get_width  (image) &&
                      y <= picman_image_get_height (image) &&
                      x + w >= 0                         &&
                      y + h >= 0                         &&
                      w > 0                              &&
                      h > 0);

  operation = picman_rectangle_select_tool_get_operation (rect_sel_tool);

  /* if rectangle exists, turn it into a selection */
  if (rectangle_exists)
    PICMAN_RECTANGLE_SELECT_TOOL_GET_CLASS (rect_sel_tool)->select (rect_sel_tool,
                                                                  operation,
                                                                  x, y, w, h);

  return rectangle_exists;
}

static void
picman_rectangle_select_tool_real_select (PicmanRectangleSelectTool *rect_sel_tool,
                                        PicmanChannelOps           operation,
                                        gint                     x,
                                        gint                     y,
                                        gint                     w,
                                        gint                     h)
{
  PicmanTool                   *tool    = PICMAN_TOOL (rect_sel_tool);
  PicmanSelectionOptions       *options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  PicmanRectangleSelectOptions *rect_select_options;
  PicmanChannel                *channel;

  rect_select_options = PICMAN_RECTANGLE_SELECT_TOOL_GET_OPTIONS (tool);

  channel = picman_image_get_mask (picman_display_get_image (tool->display));

  if (rect_select_options->round_corners)
    {
      /* To prevent elliptification of the rectangle,
       * we must cap the corner radius.
       */
      gdouble max    = MIN (w / 2.0, h / 2.0);
      gdouble radius = MIN (rect_select_options->corner_radius, max);

      picman_channel_select_round_rect (channel,
                                      x, y, w, h,
                                      radius, radius,
                                      operation,
                                      options->antialias,
                                      options->feather,
                                      options->feather_radius,
                                      options->feather_radius,
                                      TRUE);
    }
  else
    {
      picman_channel_select_rectangle (channel,
                                     x, y, w, h,
                                     operation,
                                     options->feather,
                                     options->feather_radius,
                                     options->feather_radius,
                                     TRUE);
    }
}

static PicmanChannelOps
picman_rectangle_select_tool_get_operation (PicmanRectangleSelectTool *rect_sel_tool)
{
  PicmanRectangleSelectToolPrivate *priv;
  PicmanSelectionOptions           *options;

  priv    = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);
  options = PICMAN_SELECTION_TOOL_GET_OPTIONS (rect_sel_tool);

  if (priv->use_saved_op)
    return priv->operation;
  else
    return options->operation;
}

/**
 * picman_rectangle_select_tool_update_option_defaults:
 * @crop_tool:
 * @ignore_pending: %TRUE to ignore any pending crop rectangle.
 *
 * Sets the default Fixed: Aspect ratio and Fixed: Size option
 * properties.
 */
static void
picman_rectangle_select_tool_update_option_defaults (PicmanRectangleSelectTool *rect_sel_tool,
                                                   gboolean                 ignore_pending)
{
  PicmanTool             *tool;
  PicmanRectangleTool    *rectangle_tool;
  PicmanRectangleOptions *rectangle_options;

  tool              = PICMAN_TOOL (rect_sel_tool);
  rectangle_tool    = PICMAN_RECTANGLE_TOOL (tool);
  rectangle_options = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rectangle_tool);

  if (tool->display != NULL && !ignore_pending)
    {
      /* There is a pending rectangle and we should not ignore it, so
       * set default Fixed: Size to the same as the current pending
       * rectangle width/height.
       */

      picman_rectangle_tool_pending_size_set (rectangle_tool,
                                            G_OBJECT (rectangle_options),
                                            "default-aspect-numerator",
                                            "default-aspect-denominator");

      g_object_set (G_OBJECT (rectangle_options),
                    "use-string-current", TRUE,
                    NULL);
    }
  else
    {
      g_object_set (G_OBJECT (rectangle_options),
                    "default-aspect-numerator",   1.0,
                    "default-aspect-denominator", 1.0,
                    NULL);

      g_object_set (G_OBJECT (rectangle_options),
                    "use-string-current", FALSE,
                    NULL);
    }
}

/*
 * This function is called if the user clicks and releases the left
 * button without moving it.  There are the things we might want
 * to do here:
 * 1) If there is an existing rectangle and we are inside it, we
 *    convert it into a selection.
 * 2) If there is an existing rectangle and we are outside it, we
 *    clear it.
 * 3) If there is no rectangle and there is a floating selection,
 *    we anchor it.
 * 4) If there is no rectangle and we are inside the selection, we
 *    create a rectangle from the selection bounds.
 * 5) If there is no rectangle and we are outside the selection,
 *    we clear the selection.
 */
static gboolean
picman_rectangle_select_tool_execute (PicmanRectangleTool *rectangle,
                                    gint               x,
                                    gint               y,
                                    gint               w,
                                    gint               h)
{
  PicmanTool                       *tool = PICMAN_TOOL (rectangle);
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectToolPrivate *priv;

  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (rectangle);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  if (w == 0 && h == 0 && tool->display != NULL)
    {
      PicmanImage   *image     = picman_display_get_image (tool->display);
      PicmanChannel *selection = picman_image_get_mask (image);
      gint         pressx;
      gint         pressy;

      if (picman_image_get_floating_selection (image))
        {
          floating_sel_anchor (picman_image_get_floating_selection (image));
          picman_image_flush (image);
          return TRUE;
        }

      pressx = ROUND (priv->press_x);
      pressy = ROUND (priv->press_y);

      /*  if the click was inside the marching ants  */
      if (picman_pickable_get_opacity_at (PICMAN_PICKABLE (selection),
                                        pressx, pressy) > 0.5)
        {
          gint x1, y1, x2, y2;

          if (picman_channel_bounds (selection, &x1, &y1, &x2, &y2))
            {
              g_object_set (rectangle,
                            "x1", x1,
                            "y1", y1,
                            "x2", x2,
                            "y2", y2,
                            NULL);
            }

          picman_rectangle_tool_set_function (rectangle,
                                            PICMAN_RECTANGLE_TOOL_MOVING);
          picman_rectangle_select_tool_update_option_defaults (rect_sel_tool,
                                                             FALSE);

          return FALSE;
        }
      else
        {
          PicmanTool       *tool = PICMAN_TOOL (rectangle);
          PicmanChannelOps  operation;

          /* prevent this change from halting the tool */
          picman_tool_control_push_preserve (tool->control, TRUE);

          /* We can conceptually think of a click outside of the
           * selection as adding a 0px selection. Behave intuitivly
           * for the current selection mode
           */
          operation = picman_rectangle_select_tool_get_operation (rect_sel_tool);
          switch (operation)
            {
            case PICMAN_CHANNEL_OP_REPLACE:
            case PICMAN_CHANNEL_OP_INTERSECT:
              picman_channel_clear (selection, NULL, TRUE);
              picman_image_flush (image);
              break;

            case PICMAN_CHANNEL_OP_ADD:
            case PICMAN_CHANNEL_OP_SUBTRACT:
            default:
              /* Do nothing */
              break;
            }

          picman_tool_control_pop_preserve (tool->control);
        }
    }

  picman_rectangle_select_tool_update_option_defaults (rect_sel_tool, FALSE);

  /* Reset the automatic undo/redo mechanism */
  priv->undo = NULL;
  priv->redo = NULL;

  return TRUE;
}

static void
picman_rectangle_select_tool_cancel (PicmanRectangleTool *rectangle)
{
  PicmanTool                       *tool;
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectToolPrivate *priv;

  tool          = PICMAN_TOOL (rectangle);
  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (rectangle);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  if (tool->display)
    {
      PicmanImage     *image      = picman_display_get_image (tool->display);
      PicmanUndoStack *undo_stack = picman_image_get_undo_stack (image);
      PicmanUndo      *undo       = picman_undo_stack_peek (undo_stack);

      /* if we have an existing rectangle in the current display, then
       * we have already "executed", and need to undo at this point,
       * unless the user has done something in the meantime
       */
      if (undo && priv->undo == undo)
        {
          /* prevent this change from halting the tool */
          picman_tool_control_push_preserve (tool->control, TRUE);

          picman_image_undo (image);
          picman_image_flush (image);

          picman_tool_control_pop_preserve (tool->control);
        }
    }

  picman_rectangle_select_tool_update_option_defaults (rect_sel_tool, TRUE);

  priv->undo = NULL;
  priv->redo = NULL;
}

static gboolean
picman_rectangle_select_tool_rectangle_change_complete (PicmanRectangleTool *rectangle)
{
  PicmanTool                       *tool;
  PicmanRectangleSelectTool        *rect_sel_tool;
  PicmanRectangleSelectToolPrivate *priv;

  tool          = PICMAN_TOOL (rectangle);
  rect_sel_tool = PICMAN_RECTANGLE_SELECT_TOOL (tool);
  priv          = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  /* prevent change in selection from halting the tool */
  picman_tool_control_push_preserve (tool->control, TRUE);

  if (tool->display && ! picman_tool_control_is_active (tool->control))
    {
      PicmanImage     *image      = picman_display_get_image (tool->display);
      PicmanUndoStack *undo_stack = picman_image_get_undo_stack (image);
      PicmanUndo      *undo       = picman_undo_stack_peek (undo_stack);
      gint           x1, y1, x2, y2;

      /* if we got here via button release, we have already undone the
       * previous operation.  But if we got here by some other means,
       * we need to undo it now.
       */
      if (undo && priv->undo == undo)
        {
          picman_image_undo (image);
          priv->undo = NULL;
        }

      g_object_get (rectangle,
                    "x1", &x1,
                    "y1", &y1,
                    "x2", &x2,
                    "y2", &y2,
                    NULL);

      if (picman_rectangle_select_tool_select (rectangle, x1, y1, x2 - x1, y2 - y1))
        {
          /* save the undo that we got when executing, but only if
           * we actually selected something
           */
          priv->undo = picman_undo_stack_peek (undo_stack);
          priv->redo = NULL;
        }

      if (! priv->use_saved_op)
        {
          PicmanSelectionOptions *options;

          options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);

          /* remember the operation now in case we modify the rectangle */
          priv->operation    = options->operation;
          priv->use_saved_op = TRUE;
        }

      picman_image_flush (image);
    }

  picman_tool_control_pop_preserve (tool->control);

  picman_rectangle_select_tool_update_option_defaults (rect_sel_tool, FALSE);

  return TRUE;
}

static void
picman_rectangle_select_tool_round_corners_notify (PicmanRectangleSelectOptions *options,
                                                 GParamSpec                 *pspec,
                                                 PicmanRectangleSelectTool    *rect_sel_tool)
{
  PicmanDrawTool                   *draw_tool = PICMAN_DRAW_TOOL (rect_sel_tool);
  PicmanRectangleTool              *rect_tool = PICMAN_RECTANGLE_TOOL (rect_sel_tool);
  PicmanRectangleSelectToolPrivate *priv;

  priv = PICMAN_RECTANGLE_SELECT_TOOL_GET_PRIVATE (rect_sel_tool);

  picman_draw_tool_pause (draw_tool);

  priv->round_corners = options->round_corners;
  priv->corner_radius = options->corner_radius;

  picman_rectangle_select_tool_rectangle_change_complete (rect_tool);

  picman_draw_tool_resume (draw_tool);
}
