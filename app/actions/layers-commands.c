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
#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmangrouplayer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-merge.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanitemundo.h"
#include "core/picmanlayer-floating-sel.h"
#include "core/picmanpickable.h"
#include "core/picmanpickable-auto-shrink.h"
#include "core/picmantoolinfo.h"
#include "core/picmanundostack.h"
#include "core/picmanprogress.h"

#include "text/picmantext.h"
#include "text/picmantext-vectors.h"
#include "text/picmantextlayer.h"

#include "vectors/picmanvectors-warp.h"

#include "widgets/picmanaction.h"
#include "widgets/picmandock.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanprogressdialog.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmanimagewindow.h"

#include "tools/picmantexttool.h"
#include "tools/tool_manager.h"

#include "dialogs/layer-add-mask-dialog.h"
#include "dialogs/layer-options-dialog.h"
#include "dialogs/resize-dialog.h"
#include "dialogs/scale-dialog.h"

#include "actions.h"
#include "layers-commands.h"

#include "picman-intl.h"


static const PicmanLayerModeEffects layer_modes[] =
{
  PICMAN_NORMAL_MODE,
  PICMAN_DISSOLVE_MODE,
  PICMAN_MULTIPLY_MODE,
  PICMAN_DIVIDE_MODE,
  PICMAN_SCREEN_MODE,
  PICMAN_OVERLAY_MODE,
  PICMAN_DODGE_MODE,
  PICMAN_BURN_MODE,
  PICMAN_HARDLIGHT_MODE,
  PICMAN_SOFTLIGHT_MODE,
  PICMAN_GRAIN_EXTRACT_MODE,
  PICMAN_GRAIN_MERGE_MODE,
  PICMAN_DIFFERENCE_MODE,
  PICMAN_ADDITION_MODE,
  PICMAN_SUBTRACT_MODE,
  PICMAN_DARKEN_ONLY_MODE,
  PICMAN_LIGHTEN_ONLY_MODE,
  PICMAN_HUE_MODE,
  PICMAN_SATURATION_MODE,
  PICMAN_COLOR_MODE,
  PICMAN_VALUE_MODE
};


/*  local function prototypes  */

static void   layers_new_layer_response    (GtkWidget             *widget,
                                            gint                   response_id,
                                            LayerOptionsDialog    *dialog);
static void   layers_edit_layer_response   (GtkWidget             *widget,
                                            gint                   response_id,
                                            LayerOptionsDialog    *dialog);
static void   layers_add_mask_response     (GtkWidget             *widget,
                                            gint                   response_id,
                                            LayerAddMaskDialog    *dialog);

static void   layers_scale_layer_callback  (GtkWidget             *dialog,
                                            PicmanViewable          *viewable,
                                            gint                   width,
                                            gint                   height,
                                            PicmanUnit               unit,
                                            PicmanInterpolationType  interpolation,
                                            gdouble                xresolution,
                                            gdouble                yresolution,
                                            PicmanUnit               resolution_unit,
                                            gpointer               data);
static void   layers_resize_layer_callback (GtkWidget             *dialog,
                                            PicmanViewable          *viewable,
                                            gint                   width,
                                            gint                   height,
                                            PicmanUnit               unit,
                                            gint                   offset_x,
                                            gint                   offset_y,
                                            PicmanItemSet            unused,
                                            gboolean               unused2,
                                            gpointer               data);

static gint   layers_mode_index            (PicmanLayerModeEffects   layer_mode);


/*  private variables  */

static PicmanFillType           layer_fill_type     = PICMAN_TRANSPARENT_FILL;
static gchar                 *layer_name          = NULL;
static PicmanUnit               layer_resize_unit   = PICMAN_UNIT_PIXEL;
static PicmanUnit               layer_scale_unit    = PICMAN_UNIT_PIXEL;
static PicmanInterpolationType  layer_scale_interp  = -1;
static PicmanAddMaskType        layer_add_mask_type = PICMAN_ADD_WHITE_MASK;
static gboolean               layer_mask_invert   = FALSE;


/*  public functions  */

void
layers_text_tool_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  GtkWidget *widget;
  PicmanTool  *active_tool;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  if (! picman_item_is_text_layer (PICMAN_ITEM (layer)))
    {
      layers_edit_attributes_cmd_callback (action, data);
      return;
    }

  active_tool = tool_manager_get_active (image->picman);

  if (! PICMAN_IS_TEXT_TOOL (active_tool))
    {
      PicmanToolInfo *tool_info = picman_get_tool_info (image->picman,
                                                    "picman-text-tool");

      if (PICMAN_IS_TOOL_INFO (tool_info))
        {
          picman_context_set_tool (action_data_get_context (data), tool_info);
          active_tool = tool_manager_get_active (image->picman);
        }
    }

  if (PICMAN_IS_TEXT_TOOL (active_tool))
    picman_text_tool_set_layer (PICMAN_TEXT_TOOL (active_tool), layer);
}

void
layers_edit_attributes_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  LayerOptionsDialog *dialog;
  PicmanImage          *image;
  PicmanLayer          *layer;
  GtkWidget          *widget;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  dialog = layer_options_dialog_new (picman_item_get_image (PICMAN_ITEM (layer)),
                                     layer,
                                     action_data_get_context (data),
                                     widget,
                                     picman_object_get_name (layer),
                                     layer_fill_type,
                                     _("Layer Attributes"),
                                     "picman-layer-edit",
                                     GTK_STOCK_EDIT,
                                     _("Edit Layer Attributes"),
                                     PICMAN_HELP_LAYER_EDIT);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (layers_edit_layer_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
layers_new_cmd_callback (GtkAction *action,
                         gpointer   data)
{
  LayerOptionsDialog *dialog;
  PicmanImage          *image;
  GtkWidget          *widget;
  PicmanLayer          *floating_sel;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  /*  If there is a floating selection, the new command transforms
   *  the current fs into a new layer
   */
  if ((floating_sel = picman_image_get_floating_selection (image)))
    {
      GError *error = NULL;

      if (! floating_sel_to_layer (floating_sel, &error))
        {
          picman_message_literal (image->picman,
				G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
				error->message);
          g_clear_error (&error);
          return;
        }

      picman_image_flush (image);
      return;
    }

  dialog = layer_options_dialog_new (image, NULL,
                                     action_data_get_context (data),
                                     widget,
                                     layer_name ? layer_name : _("Layer"),
                                     layer_fill_type,
                                     _("New Layer"),
                                     "picman-layer-new",
                                     PICMAN_STOCK_LAYER,
                                     _("Create a New Layer"),
                                     PICMAN_HELP_LAYER_NEW);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (layers_new_layer_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
layers_new_last_vals_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanImage            *image;
  GtkWidget            *widget;
  PicmanLayer            *floating_sel;
  PicmanLayer            *new_layer;
  gint                  width, height;
  gint                  off_x, off_y;
  gdouble               opacity;
  PicmanLayerModeEffects  mode;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  /*  If there is a floating selection, the new command transforms
   *  the current fs into a new layer
   */
  if ((floating_sel = picman_image_get_floating_selection (image)))
    {
      GError *error = NULL;

      if (! floating_sel_to_layer (floating_sel, &error))
        {
          picman_message_literal (image->picman, G_OBJECT (widget),
				PICMAN_MESSAGE_WARNING, error->message);
          g_clear_error (&error);
          return;
        }

      picman_image_flush (image);
      return;
    }

  if (PICMAN_IS_LAYER (PICMAN_ACTION (action)->viewable))
    {
      PicmanLayer *template = PICMAN_LAYER (PICMAN_ACTION (action)->viewable);

      picman_item_get_offset (PICMAN_ITEM (template), &off_x, &off_y);
      width   = picman_item_get_width  (PICMAN_ITEM (template));
      height  = picman_item_get_height (PICMAN_ITEM (template));
      opacity = picman_layer_get_opacity (template);
      mode    = picman_layer_get_mode (template);
    }
  else
    {
      width   = picman_image_get_width (image);
      height  = picman_image_get_height (image);
      off_x   = 0;
      off_y   = 0;
      opacity = 1.0;
      mode    = PICMAN_NORMAL_MODE;
    }

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                               _("New Layer"));

  new_layer = picman_layer_new (image, width, height,
                              picman_image_get_layer_format (image, TRUE),
                              layer_name,
                              opacity, mode);

  picman_drawable_fill_by_type (PICMAN_DRAWABLE (new_layer),
                              action_data_get_context (data),
                              layer_fill_type);
  picman_item_translate (PICMAN_ITEM (new_layer), off_x, off_y, FALSE);

  picman_image_add_layer (image, new_layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_undo_group_end (image);

  picman_image_flush (image);
}

void
layers_new_from_visible_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage    *image;
  PicmanLayer    *layer;
  PicmanPickable *pickable;
  return_if_no_image (image, data);

  pickable = PICMAN_PICKABLE (picman_image_get_projection (image));

  picman_pickable_flush (pickable);

  layer = picman_layer_new_from_buffer (picman_pickable_get_buffer (pickable),
                                      image,
                                      picman_image_get_layer_format (image, TRUE),
                                      _("Visible"),
                                      PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_image_add_layer (image, layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_flush (image);
}

void
layers_new_group_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_image (image, data);

  layer = picman_group_layer_new (image);

  picman_image_add_layer (image, layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_flush (image);
}

void
layers_select_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanImage     *image;
  PicmanLayer     *layer;
  PicmanContainer *container;
  PicmanLayer     *new_layer;
  return_if_no_image (image, data);

  layer = picman_image_get_active_layer (image);

  if (layer)
    container = picman_item_get_container (PICMAN_ITEM (layer));
  else
    container = picman_image_get_layers (image);

  new_layer = (PicmanLayer *) action_select_object ((PicmanActionSelectType) value,
                                                  container,
                                                  (PicmanObject *) layer);

  if (new_layer && new_layer != layer)
    {
      picman_image_set_active_layer (image, new_layer);
      picman_image_flush (image);
    }
}

void
layers_raise_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_raise_item (image, PICMAN_ITEM (layer), NULL);
  picman_image_flush (image);
}

void
layers_raise_to_top_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_raise_item_to_top (image, PICMAN_ITEM (layer));
  picman_image_flush (image);
}

void
layers_lower_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_lower_item (image, PICMAN_ITEM (layer), NULL);
  picman_image_flush (image);
}

void
layers_lower_to_bottom_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_lower_item_to_bottom (image, PICMAN_ITEM (layer));
  picman_image_flush (image);
}

void
layers_duplicate_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  PicmanLayer *new_layer;
  return_if_no_layer (image, layer, data);

  new_layer = PICMAN_LAYER (picman_item_duplicate (PICMAN_ITEM (layer),
                                               G_TYPE_FROM_INSTANCE (layer)));

  /*  use the actual parent here, not PICMAN_IMAGE_ACTIVE_PARENT because
   *  the latter would add a duplicated group inside itself instead of
   *  above it
   */
  picman_image_add_layer (image, new_layer,
                        picman_layer_get_parent (layer), -1,
                        TRUE);

  picman_image_flush (image);
}

void
layers_anchor_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  if (picman_layer_is_floating_sel (layer))
    {
      floating_sel_anchor (layer);
      picman_image_flush (image);
    }
}

void
layers_merge_down_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_merge_down (image, layer, action_data_get_context (data),
                         PICMAN_EXPAND_AS_NECESSARY, NULL);
  picman_image_flush (image);
}

void
layers_merge_group_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_merge_group_layer (image, PICMAN_GROUP_LAYER (layer));
  picman_image_flush (image);
}

void
layers_delete_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_image_remove_layer (image, layer, TRUE, NULL);
  picman_image_flush (image);
}

void
layers_text_discard_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  if (PICMAN_IS_TEXT_LAYER (layer))
    picman_text_layer_discard (PICMAN_TEXT_LAYER (layer));
}

void
layers_text_to_vectors_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  if (PICMAN_IS_TEXT_LAYER (layer))
    {
      PicmanVectors *vectors;
      gint         x, y;

      vectors = picman_text_vectors_new (image, PICMAN_TEXT_LAYER (layer)->text);

      picman_item_get_offset (PICMAN_ITEM (layer), &x, &y);
      picman_item_translate (PICMAN_ITEM (vectors), x, y, FALSE);

      picman_image_add_vectors (image, vectors,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_flush (image);
    }
}

void
layers_text_along_vectors_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanImage   *image;
  PicmanLayer   *layer;
  PicmanVectors *vectors;
  return_if_no_layer (image, layer, data);
  return_if_no_vectors (image, vectors, data);

  if (PICMAN_IS_TEXT_LAYER (layer))
    {
      PicmanVectors *new_vectors;

      new_vectors = picman_text_vectors_new (image, PICMAN_TEXT_LAYER (layer)->text);

      picman_vectors_warp_vectors (vectors, new_vectors,
                                 0.5 * picman_item_get_height (PICMAN_ITEM (layer)));

      picman_item_set_visible (PICMAN_ITEM (new_vectors), TRUE, FALSE);

      picman_image_add_vectors (image, new_vectors,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_flush (image);
    }
}

void
layers_resize_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanDisplay *display = NULL;
  PicmanImage   *image;
  PicmanLayer   *layer;
  GtkWidget   *widget;
  GtkWidget   *dialog;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  if (PICMAN_IS_IMAGE_WINDOW (data))
    display = action_data_get_display (data);

  if (layer_resize_unit != PICMAN_UNIT_PERCENT && display)
    layer_resize_unit = picman_display_get_shell (display)->unit;

  dialog = resize_dialog_new (PICMAN_VIEWABLE (layer),
                              action_data_get_context (data),
                              _("Set Layer Boundary Size"), "picman-layer-resize",
                              widget,
                              picman_standard_help_func, PICMAN_HELP_LAYER_RESIZE,
                              layer_resize_unit,
                              layers_resize_layer_callback,
                              action_data_get_context (data));

  gtk_widget_show (dialog);
}

void
layers_resize_to_image_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_layer_resize_to_image (layer, action_data_get_context (data));
  picman_image_flush (image);
}

void
layers_scale_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  PicmanDisplay *display = NULL;
  PicmanImage   *image;
  PicmanLayer   *layer;
  GtkWidget   *widget;
  GtkWidget   *dialog;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  if (PICMAN_IS_IMAGE_WINDOW (data))
    display = action_data_get_display (data);

  if (layer_scale_unit != PICMAN_UNIT_PERCENT && display)
    layer_scale_unit = picman_display_get_shell (display)->unit;

  if (layer_scale_interp == -1)
    layer_scale_interp = image->picman->config->interpolation_type;

  dialog = scale_dialog_new (PICMAN_VIEWABLE (layer),
                             action_data_get_context (data),
                             _("Scale Layer"), "picman-layer-scale",
                             widget,
                             picman_standard_help_func, PICMAN_HELP_LAYER_SCALE,
                             layer_scale_unit,
                             layer_scale_interp,
                             layers_scale_layer_callback,
                             display);

  gtk_widget_show (dialog);
}

void
layers_crop_to_selection_cmd_callback (GtkAction *action,
                                       gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  GtkWidget *widget;
  gint       x1, y1, x2, y2;
  gint       off_x, off_y;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  if (! picman_channel_bounds (picman_image_get_mask (image),
                             &x1, &y1, &x2, &y2))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("Cannot crop because the current selection is empty."));
      return;
    }

  picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

  off_x -= x1;
  off_y -= y1;

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_RESIZE,
                               _("Crop Layer to Selection"));

  picman_item_resize (PICMAN_ITEM (layer), action_data_get_context (data),
                    x2 - x1, y2 - y1, off_x, off_y);

  picman_image_undo_group_end (image);

  picman_image_flush (image);
}

void
layers_crop_to_content_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  GtkWidget *widget;
  gint       x1, y1, x2, y2;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  if (! picman_pickable_auto_shrink (PICMAN_PICKABLE (layer),
                                   0, 0,
                                   picman_item_get_width  (PICMAN_ITEM (layer)),
                                   picman_item_get_height (PICMAN_ITEM (layer)),
                                   &x1, &y1, &x2, &y2))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("Cannot crop because the active layer has no content."));
      return;
    }

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_RESIZE,
                               _("Crop Layer to Content"));

  picman_item_resize (PICMAN_ITEM (layer), action_data_get_context (data),
                    x2 - x1, y2 - y1, -x1, -y1);

  picman_image_undo_group_end (image);

  picman_image_flush (image);
}

void
layers_mask_add_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  LayerAddMaskDialog *dialog;
  PicmanImage          *image;
  PicmanLayer          *layer;
  GtkWidget          *widget;
  return_if_no_layer (image, layer, data);
  return_if_no_widget (widget, data);

  dialog = layer_add_mask_dialog_new (layer, action_data_get_context (data),
                                      widget,
                                      layer_add_mask_type, layer_mask_invert);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (layers_add_mask_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
layers_mask_apply_cmd_callback (GtkAction *action,
                                gint       value,
                                gpointer   data)
{
  PicmanImage         *image;
  PicmanLayer         *layer;
  PicmanMaskApplyMode  mode;
  return_if_no_layer (image, layer, data);

  mode = (PicmanMaskApplyMode) value;

  if (picman_layer_get_mask (layer))
    {
      picman_layer_apply_mask (layer, mode, TRUE);
      picman_image_flush (image);
    }
}

void
layers_mask_edit_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage     *image;
  PicmanLayer     *layer;
  PicmanLayerMask *mask;
  return_if_no_layer (image, layer, data);

  mask = picman_layer_get_mask (layer);

  if (mask)
    {
      gboolean active;

      active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_layer_set_edit_mask (layer, active);
      picman_image_flush (image);
    }
}

void
layers_mask_show_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage     *image;
  PicmanLayer     *layer;
  PicmanLayerMask *mask;
  return_if_no_layer (image, layer, data);

  mask = picman_layer_get_mask (layer);

  if (mask)
    {
      gboolean active;

      active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_layer_set_show_mask (layer, active, TRUE);
      picman_image_flush (image);
    }
}

void
layers_mask_disable_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanImage     *image;
  PicmanLayer     *layer;
  PicmanLayerMask *mask;
  return_if_no_layer (image, layer, data);

  mask = picman_layer_get_mask (layer);

  if (mask)
    {
      gboolean active;

      active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_layer_set_apply_mask (layer, ! active, TRUE);
      picman_image_flush (image);
    }
}

void
layers_mask_to_selection_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanImage     *image;
  PicmanLayer     *layer;
  PicmanLayerMask *mask;
  return_if_no_layer (image, layer, data);

  mask = picman_layer_get_mask (layer);

  if (mask)
    {
      picman_item_to_selection (PICMAN_ITEM (mask),
                              (PicmanChannelOps) value,
                              TRUE, FALSE, 0.0, 0.0);
      picman_image_flush (image);
    }
}

void
layers_alpha_add_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  if (! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    {
      picman_layer_add_alpha (layer);
      picman_image_flush (image);
    }
}

void
layers_alpha_remove_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  if (picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    {
      picman_layer_flatten (layer, action_data_get_context (data));
      picman_image_flush (image);
    }
}

void
layers_alpha_to_selection_cmd_callback (GtkAction *action,
                                        gint       value,
                                        gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  return_if_no_layer (image, layer, data);

  picman_item_to_selection (PICMAN_ITEM (layer),
                          (PicmanChannelOps) value,
                          TRUE, FALSE, 0.0, 0.0);
  picman_image_flush (image);
}

void
layers_opacity_cmd_callback (GtkAction *action,
                             gint       value,
                             gpointer   data)
{
  PicmanImage      *image;
  PicmanLayer      *layer;
  gdouble         opacity;
  PicmanUndo       *undo;
  gboolean        push_undo = TRUE;
  return_if_no_layer (image, layer, data);

  undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                       PICMAN_UNDO_LAYER_OPACITY);

  if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
    push_undo = FALSE;

  opacity = action_select_value ((PicmanActionSelectType) value,
                                 picman_layer_get_opacity (layer),
                                 0.0, 1.0, 1.0,
                                 1.0 / 255.0, 0.01, 0.1, 0.0, FALSE);
  picman_layer_set_opacity (layer, opacity, push_undo);
  picman_image_flush (image);
}

void
layers_mode_cmd_callback (GtkAction *action,
                          gint       value,
                          gpointer   data)
{
  PicmanImage            *image;
  PicmanLayer            *layer;
  PicmanLayerModeEffects  layer_mode;
  gint                  index;
  PicmanUndo             *undo;
  gboolean              push_undo = TRUE;
  return_if_no_layer (image, layer, data);

  undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                       PICMAN_UNDO_LAYER_MODE);

  if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
    push_undo = FALSE;

  layer_mode = picman_layer_get_mode (layer);

  index = action_select_value ((PicmanActionSelectType) value,
                               layers_mode_index (layer_mode),
                               0, G_N_ELEMENTS (layer_modes) - 1, 0,
                               0.0, 1.0, 1.0, 0.0, FALSE);
  picman_layer_set_mode (layer, layer_modes[index], push_undo);
  picman_image_flush (image);
}

void
layers_lock_alpha_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage *image;
  PicmanLayer *layer;
  gboolean   lock_alpha;
  return_if_no_layer (image, layer, data);

  lock_alpha = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (lock_alpha != picman_layer_get_lock_alpha (layer))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_LAYER_LOCK_ALPHA);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
        push_undo = FALSE;

      picman_layer_set_lock_alpha (layer, lock_alpha, push_undo);
      picman_image_flush (image);
    }
}


/*  private functions  */

static void
layers_new_layer_response (GtkWidget          *widget,
                           gint                response_id,
                           LayerOptionsDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanLayer *layer;

      if (layer_name)
        g_free (layer_name);

      layer_name =
        g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->name_entry)));

      layer_fill_type = dialog->fill_type;

      dialog->xsize =
        RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (dialog->size_se),
                                          0));
      dialog->ysize =
        RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (dialog->size_se),
                                          1));

      layer = picman_layer_new (dialog->image,
                              dialog->xsize,
                              dialog->ysize,
                              picman_image_get_layer_format (dialog->image, TRUE),
                              layer_name,
                              PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

      if (layer)
        {
          picman_drawable_fill_by_type (PICMAN_DRAWABLE (layer),
                                      dialog->context,
                                      layer_fill_type);

          picman_image_add_layer (dialog->image, layer,
                                PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

          picman_image_flush (dialog->image);
        }
      else
        {
          g_warning ("%s: could not allocate new layer", G_STRFUNC);
        }
    }

  gtk_widget_destroy (dialog->dialog);
}

static void
layers_edit_layer_response (GtkWidget          *widget,
                            gint                response_id,
                            LayerOptionsDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanLayer   *layer = dialog->layer;
      const gchar *new_name;

      new_name = gtk_entry_get_text (GTK_ENTRY (dialog->name_entry));

      if (strcmp (new_name, picman_object_get_name (layer)))
        {
          GError *error = NULL;

          if (picman_item_rename (PICMAN_ITEM (layer), new_name, &error))
            {
              picman_image_flush (dialog->image);
            }
          else
            {
              picman_message_literal (dialog->image->picman,
				    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
				    error->message);
              g_clear_error (&error);

              return;
            }
        }

      if (dialog->rename_toggle &&
          picman_item_is_text_layer (PICMAN_ITEM (layer)))
        {
          g_object_set (layer,
                        "auto-rename",
                        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->rename_toggle)),
                        NULL);
        }
    }

  gtk_widget_destroy (dialog->dialog);
}

static void
layers_add_mask_response (GtkWidget          *widget,
                          gint                response_id,
                          LayerAddMaskDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanLayer     *layer = dialog->layer;
      PicmanImage     *image = picman_item_get_image (PICMAN_ITEM (layer));
      PicmanLayerMask *mask;

      if (dialog->add_mask_type == PICMAN_ADD_CHANNEL_MASK &&
          ! dialog->channel)
        {
          picman_message_literal (image->picman,
				G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
				_("Please select a channel first"));
          return;
        }

      layer_add_mask_type = dialog->add_mask_type;
      layer_mask_invert   = dialog->invert;

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_LAYER_ADD_MASK,
                                   _("Add Layer Mask"));

      mask = picman_layer_create_mask (layer, layer_add_mask_type,
                                     dialog->channel);

      if (layer_mask_invert)
        picman_channel_invert (PICMAN_CHANNEL (mask), FALSE);

      picman_layer_add_mask (layer, mask, TRUE, NULL);

      picman_image_undo_group_end (image);

      picman_image_flush (image);
    }

  gtk_widget_destroy (dialog->dialog);
}

static void
layers_scale_layer_callback (GtkWidget             *dialog,
                             PicmanViewable          *viewable,
                             gint                   width,
                             gint                   height,
                             PicmanUnit               unit,
                             PicmanInterpolationType  interpolation,
                             gdouble                xresolution,    /* unused */
                             gdouble                yresolution,    /* unused */
                             PicmanUnit               resolution_unit,/* unused */
                             gpointer               data)
{
  PicmanDisplay *display = PICMAN_DISPLAY (data);

  layer_scale_unit   = unit;
  layer_scale_interp = interpolation;

  if (width > 0 && height > 0)
    {
      PicmanItem     *item = PICMAN_ITEM (viewable);
      PicmanProgress *progress;
      GtkWidget    *progress_dialog = NULL;

      gtk_widget_destroy (dialog);

      if (width  == picman_item_get_width  (item) &&
          height == picman_item_get_height (item))
        return;

      if (display)
        {
          progress = PICMAN_PROGRESS (display);
        }
      else
        {
          progress_dialog = picman_progress_dialog_new ();
          progress = PICMAN_PROGRESS (progress_dialog);
        }

      progress = picman_progress_start (progress, _("Scaling"), FALSE);

      picman_item_scale_by_origin (item,
                                 width, height, interpolation,
                                 progress, TRUE);

      if (progress)
        picman_progress_end (progress);

      if (progress_dialog)
        gtk_widget_destroy (progress_dialog);

      picman_image_flush (picman_item_get_image (item));
    }
  else
    {
      g_warning ("Scale Error: "
                 "Both width and height must be greater than zero.");
    }
}

static void
layers_resize_layer_callback (GtkWidget    *dialog,
                              PicmanViewable *viewable,
                              gint          width,
                              gint          height,
                              PicmanUnit      unit,
                              gint          offset_x,
                              gint          offset_y,
                              PicmanItemSet   unused,
                              gboolean      unused2,
                              gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  layer_resize_unit = unit;

  if (width > 0 && height > 0)
    {
      PicmanItem *item = PICMAN_ITEM (viewable);

      gtk_widget_destroy (dialog);

      if (width  == picman_item_get_width  (item) &&
          height == picman_item_get_height (item))
        return;

      picman_item_resize (item, context,
                        width, height, offset_x, offset_y);

      picman_image_flush (picman_item_get_image (item));
    }
  else
    {
      g_warning ("Resize Error: "
                 "Both width and height must be greater than zero.");
    }
}

static gint
layers_mode_index (PicmanLayerModeEffects layer_mode)
{
  gint i = 0;

  while (i < (G_N_ELEMENTS (layer_modes) - 1) && layer_modes[i] != layer_mode)
    i++;

  return i;
}
