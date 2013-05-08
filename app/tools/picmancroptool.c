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
#include "core/picmanimage.h"
#include "core/picmanimage-crop.h"
#include "core/picmanitem.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanrectangleoptions.h"
#include "picmanrectangletool.h"
#include "picmancropoptions.h"
#include "picmancroptool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void      picman_crop_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface);

static void      picman_crop_tool_constructed               (GObject              *object);

static void      picman_crop_tool_control                   (PicmanTool             *tool,
                                                           PicmanToolAction        action,
                                                           PicmanDisplay          *display);
static void      picman_crop_tool_button_press              (PicmanTool             *tool,
                                                           const PicmanCoords     *coords,
                                                           guint32               time,
                                                           GdkModifierType       state,
                                                           PicmanButtonPressType   press_type,
                                                           PicmanDisplay          *display);
static void      picman_crop_tool_button_release            (PicmanTool             *tool,
                                                           const PicmanCoords     *coords,
                                                           guint32               time,
                                                           GdkModifierType       state,
                                                           PicmanButtonReleaseType release_type,
                                                           PicmanDisplay          *display);
static void      picman_crop_tool_active_modifier_key       (PicmanTool             *tool,
                                                           GdkModifierType       key,
                                                           gboolean              press,
                                                           GdkModifierType       state,
                                                           PicmanDisplay          *display);
static void      picman_crop_tool_cursor_update             (PicmanTool             *tool,
                                                           const PicmanCoords     *coords,
                                                           GdkModifierType       state,
                                                           PicmanDisplay          *display);

static void      picman_crop_tool_draw                      (PicmanDrawTool         *draw_tool);

static gboolean  picman_crop_tool_execute                   (PicmanRectangleTool    *rectangle,
                                                           gint                  x,
                                                           gint                  y,
                                                           gint                  w,
                                                           gint                  h);

static void      picman_crop_tool_update_option_defaults    (PicmanCropTool         *crop_tool,
                                                           gboolean              ignore_pending);
static PicmanRectangleConstraint
                 picman_crop_tool_get_constraint            (PicmanCropTool         *crop_tool);

static void      picman_crop_tool_options_notify            (PicmanCropOptions      *options,
                                                           GParamSpec           *pspec,
                                                           PicmanCropTool         *crop_tool);
static void      picman_crop_tool_image_changed             (PicmanCropTool         *crop_tool,
                                                           PicmanImage            *image,
                                                           PicmanContext          *context);
static void      picman_crop_tool_image_size_changed        (PicmanCropTool         *crop_tool);
static void      picman_crop_tool_cancel                    (PicmanRectangleTool    *rect_tool);
static gboolean  picman_crop_tool_rectangle_change_complete (PicmanRectangleTool    *rect_tool);


G_DEFINE_TYPE_WITH_CODE (PicmanCropTool, picman_crop_tool, PICMAN_TYPE_DRAW_TOOL,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_RECTANGLE_TOOL,
                                                picman_crop_tool_rectangle_tool_iface_init));

#define parent_class picman_crop_tool_parent_class


/*  public functions  */

void
picman_crop_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_CROP_TOOL,
                PICMAN_TYPE_CROP_OPTIONS,
                picman_crop_options_gui,
                0,
                "picman-crop-tool",
                _("Crop"),
                _("Crop Tool: Remove edge areas from image or layer"),
                N_("_Crop"), "<shift>C",
                NULL, PICMAN_HELP_TOOL_CROP,
                PICMAN_STOCK_TOOL_CROP,
                data);
}

static void
picman_crop_tool_class_init (PicmanCropToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed       = picman_crop_tool_constructed;
  object_class->set_property      = picman_rectangle_tool_set_property;
  object_class->get_property      = picman_rectangle_tool_get_property;

  tool_class->control             = picman_crop_tool_control;
  tool_class->button_press        = picman_crop_tool_button_press;
  tool_class->button_release      = picman_crop_tool_button_release;
  tool_class->motion              = picman_rectangle_tool_motion;
  tool_class->key_press           = picman_rectangle_tool_key_press;
  tool_class->active_modifier_key = picman_crop_tool_active_modifier_key;
  tool_class->oper_update         = picman_rectangle_tool_oper_update;
  tool_class->cursor_update       = picman_crop_tool_cursor_update;

  draw_tool_class->draw           = picman_crop_tool_draw;

  picman_rectangle_tool_install_properties (object_class);
}

static void
picman_crop_tool_rectangle_tool_iface_init (PicmanRectangleToolInterface *iface)
{
  iface->execute                   = picman_crop_tool_execute;
  iface->cancel                    = picman_crop_tool_cancel;
  iface->rectangle_change_complete = picman_crop_tool_rectangle_change_complete;
}

static void
picman_crop_tool_init (PicmanCropTool *crop_tool)
{
  PicmanTool *tool = PICMAN_TOOL (crop_tool);

  picman_tool_control_set_wants_click (tool->control, TRUE);
  picman_tool_control_set_precision   (tool->control,
                                     PICMAN_CURSOR_PRECISION_PIXEL_BORDER);
  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_CROP);

  picman_rectangle_tool_init (PICMAN_RECTANGLE_TOOL (crop_tool));

  crop_tool->current_image = NULL;
}

static void
picman_crop_tool_constructed (GObject *object)
{
  PicmanCropTool    *crop_tool = PICMAN_CROP_TOOL (object);
  PicmanCropOptions *options;
  PicmanContext     *picman_context;
  PicmanToolInfo    *tool_info;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_rectangle_tool_constructor (object);

  tool_info = PICMAN_TOOL (crop_tool)->tool_info;

  picman_context = picman_get_user_context (tool_info->picman);

  g_signal_connect_object (picman_context, "image-changed",
                           G_CALLBACK (picman_crop_tool_image_changed),
                           crop_tool,
                           G_CONNECT_SWAPPED);

  /* Make sure we are connected to "size-changed" for the initial
   * image.
   */
  picman_crop_tool_image_changed (crop_tool,
                                picman_context_get_image (picman_context),
                                picman_context);


  options = PICMAN_CROP_TOOL_GET_OPTIONS (object);

  g_signal_connect_object (options, "notify::layer-only",
                           G_CALLBACK (picman_crop_tool_options_notify),
                           object, 0);

  g_signal_connect_object (options, "notify::allow-growing",
                           G_CALLBACK (picman_crop_tool_options_notify),
                           object, 0);

  picman_rectangle_tool_set_constraint (PICMAN_RECTANGLE_TOOL (object),
                                      picman_crop_tool_get_constraint (crop_tool));

  picman_crop_tool_update_option_defaults (crop_tool, FALSE);
}

static void
picman_crop_tool_control (PicmanTool       *tool,
                        PicmanToolAction  action,
                        PicmanDisplay    *display)
{
  picman_rectangle_tool_control (tool, action, display);

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_crop_tool_button_press (PicmanTool            *tool,
                             const PicmanCoords    *coords,
                             guint32              time,
                             GdkModifierType      state,
                             PicmanButtonPressType  press_type,
                             PicmanDisplay         *display)
{
  if (tool->display && display != tool->display)
    picman_rectangle_tool_cancel (PICMAN_RECTANGLE_TOOL (tool));

  picman_tool_control_activate (tool->control);

  picman_rectangle_tool_button_press (tool, coords, time, state, display);
}

static void
picman_crop_tool_button_release (PicmanTool              *tool,
                               const PicmanCoords      *coords,
                               guint32                time,
                               GdkModifierType        state,
                               PicmanButtonReleaseType  release_type,
                               PicmanDisplay           *display)
{
  picman_tool_push_status (tool, display, _("Click or press Enter to crop"));

  picman_rectangle_tool_button_release (tool,
                                      coords,
                                      time,
                                      state,
                                      release_type,
                                      display);

  picman_tool_control_halt (tool->control);
}

static void
picman_crop_tool_active_modifier_key (PicmanTool        *tool,
                                    GdkModifierType  key,
                                    gboolean         press,
                                    GdkModifierType  state,
                                    PicmanDisplay     *display)
{
  PICMAN_TOOL_CLASS (parent_class)->active_modifier_key (tool, key, press, state,
                                                       display);

  picman_rectangle_tool_active_modifier_key (tool, key, press, state, display);
}

static void
picman_crop_tool_cursor_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              PicmanDisplay      *display)
{
  picman_rectangle_tool_cursor_update (tool, coords, state, display);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_crop_tool_draw (PicmanDrawTool *draw_tool)
{
  picman_rectangle_tool_draw (draw_tool, NULL);
}

static gboolean
picman_crop_tool_execute (PicmanRectangleTool  *rectangle,
                        gint                x,
                        gint                y,
                        gint                w,
                        gint                h)
{
  PicmanTool        *tool    = PICMAN_TOOL (rectangle);
  PicmanCropOptions *options = PICMAN_CROP_TOOL_GET_OPTIONS (tool);
  PicmanImage       *image   = picman_display_get_image (tool->display);

  picman_tool_pop_status (tool, tool->display);

  /* if rectangle exists, crop it */
  if (w > 0 && h > 0)
    {
      if (options->layer_only)
        {
          PicmanLayer *layer = picman_image_get_active_layer (image);
          gint       off_x, off_y;

          if (! layer)
            {
              picman_tool_message_literal (tool, tool->display,
                                         _("There is no active layer to crop."));
              return FALSE;
            }

          if (picman_item_is_content_locked (PICMAN_ITEM (layer)))
            {
              picman_tool_message_literal (tool, tool->display,
                                         _("The active layer's pixels are locked."));
              return FALSE;
            }

          picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

          off_x -= x;
          off_y -= y;

          picman_item_resize (PICMAN_ITEM (layer), PICMAN_CONTEXT (options),
                            w, h, off_x, off_y);
        }
      else
        {
          picman_image_crop (image, PICMAN_CONTEXT (options),
                           x, y, w + x, h + y,
                           TRUE);
        }

      picman_image_flush (image);

      return TRUE;
    }

  return TRUE;
}

/**
 * picman_crop_tool_rectangle_change_complete:
 * @rectangle:
 *
 * Returns:
 **/
static gboolean
picman_crop_tool_rectangle_change_complete (PicmanRectangleTool *rectangle)
{
  picman_crop_tool_update_option_defaults (PICMAN_CROP_TOOL (rectangle), FALSE);

  return TRUE;
}

/**
 * picman_crop_tool_update_option_defaults:
 * @crop_tool:
 * @ignore_pending: %TRUE to ignore any pending crop rectangle.
 *
 * Sets the default Fixed: Aspect ratio and Fixed: Size option
 * properties.
 */
static void
picman_crop_tool_update_option_defaults (PicmanCropTool *crop_tool,
                                       gboolean      ignore_pending)
{
  PicmanTool             *tool;
  PicmanRectangleTool    *rectangle_tool;
  PicmanRectangleOptions *rectangle_options;

  tool              = PICMAN_TOOL (crop_tool);
  rectangle_tool    = PICMAN_RECTANGLE_TOOL (tool);
  rectangle_options = PICMAN_RECTANGLE_TOOL_GET_OPTIONS (rectangle_tool);

  if (tool->display != NULL && !ignore_pending)
    {
      /* There is a pending rectangle and we should not ignore it, so
       * set default Fixed: Aspect ratio to the same as the current
       * pending rectangle width/height.
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
      /* There is no pending rectangle, set default Fixed: Aspect
       * ratio to that of the current image/layer.
       */

      picman_rectangle_tool_constraint_size_set (rectangle_tool,
                                               G_OBJECT (rectangle_options),
                                               "default-aspect-numerator",
                                               "default-aspect-denominator");

      g_object_set (G_OBJECT (rectangle_options),
                    "use-string-current", FALSE,
                    NULL);
    }
}

static PicmanRectangleConstraint
picman_crop_tool_get_constraint (PicmanCropTool *crop_tool)
{
  PicmanCropOptions *crop_options = PICMAN_CROP_TOOL_GET_OPTIONS (crop_tool);

  if (crop_options->allow_growing)
    {
      return PICMAN_RECTANGLE_CONSTRAIN_NONE;
    }
  else
    {
      return crop_options->layer_only ? PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE :
                                        PICMAN_RECTANGLE_CONSTRAIN_IMAGE;
    }
}

static void
picman_crop_tool_options_notify (PicmanCropOptions *options,
                               GParamSpec      *pspec,
                               PicmanCropTool    *crop_tool)
{
  picman_rectangle_tool_set_constraint (PICMAN_RECTANGLE_TOOL (crop_tool),
                                      picman_crop_tool_get_constraint (crop_tool));
}

static void
picman_crop_tool_image_changed (PicmanCropTool *crop_tool,
                              PicmanImage    *image,
                              PicmanContext  *context)
{
  if (crop_tool->current_image)
    {
      g_signal_handlers_disconnect_by_func (crop_tool->current_image,
                                            picman_crop_tool_image_size_changed,
                                            NULL);
    }

  if (image)
    {
      g_signal_connect_object (image, "size-changed",
                               G_CALLBACK (picman_crop_tool_image_size_changed),
                               crop_tool,
                               G_CONNECT_SWAPPED);
    }

  crop_tool->current_image = image;

  picman_crop_tool_update_option_defaults (PICMAN_CROP_TOOL (crop_tool),
                                         FALSE);
}

static void
picman_crop_tool_image_size_changed (PicmanCropTool *crop_tool)
{
  picman_crop_tool_update_option_defaults (crop_tool,
                                         FALSE);
}

static void
picman_crop_tool_cancel (PicmanRectangleTool *rect_tool)
{
  picman_crop_tool_update_option_defaults (PICMAN_CROP_TOOL (rect_tool),
                                         TRUE);
}
