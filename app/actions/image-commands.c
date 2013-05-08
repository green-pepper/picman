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

#include "actions-types.h"

#include "config/picmancoreconfig.h"

#include "core/core-enums.h"
#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-convert-precision.h"
#include "core/picmanimage-convert-type.h"
#include "core/picmanimage-crop.h"
#include "core/picmanimage-duplicate.h"
#include "core/picmanimage-flip.h"
#include "core/picmanimage-merge.h"
#include "core/picmanimage-resize.h"
#include "core/picmanimage-rotate.h"
#include "core/picmanimage-scale.h"
#include "core/picmanimage-undo.h"
#include "core/picmanpickable.h"
#include "core/picmanpickable-auto-shrink.h"
#include "core/picmanprogress.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "dialogs/convert-precision-dialog.h"
#include "dialogs/convert-type-dialog.h"
#include "dialogs/grid-dialog.h"
#include "dialogs/image-merge-layers-dialog.h"
#include "dialogs/image-new-dialog.h"
#include "dialogs/image-properties-dialog.h"
#include "dialogs/image-scale-dialog.h"
#include "dialogs/print-size-dialog.h"
#include "dialogs/resize-dialog.h"

#include "actions.h"
#include "image-commands.h"

#include "picman-intl.h"


#define IMAGE_CONVERT_PRECISION_DIALOG_KEY "image-convert-precision-dialog"
#define IMAGE_CONVERT_TYPE_DIALOG_KEY      "image-convert-type-dialog"


typedef struct
{
  PicmanContext *context;
  PicmanDisplay *display;
} ImageResizeOptions;


/*  local function prototypes  */

static void   image_resize_callback        (GtkWidget              *dialog,
                                            PicmanViewable           *viewable,
                                            gint                    width,
                                            gint                    height,
                                            PicmanUnit                unit,
                                            gint                    offset_x,
                                            gint                    offset_y,
                                            PicmanItemSet             layer_set,
                                            gboolean                resize_text_layers,
                                            gpointer                data);
static void   image_resize_options_free    (ImageResizeOptions     *options);

static void   image_print_size_callback    (GtkWidget              *dialog,
                                            PicmanImage              *image,
                                            gdouble                 xresolution,
                                            gdouble                 yresolution,
                                            PicmanUnit                resolution_unit,
                                            gpointer                data);

static void   image_scale_callback         (GtkWidget              *dialog,
                                            PicmanViewable           *viewable,
                                            gint                    width,
                                            gint                    height,
                                            PicmanUnit                unit,
                                            PicmanInterpolationType   interpolation,
                                            gdouble                 xresolution,
                                            gdouble                 yresolution,
                                            PicmanUnit                resolution_unit,
                                            gpointer                user_data);

static void   image_merge_layers_response  (GtkWidget              *widget,
                                            gint                    response_id,
                                            ImageMergeLayersDialog *dialog);


/*  private variables  */

static PicmanMergeType         image_merge_layers_type               = PICMAN_EXPAND_AS_NECESSARY;
static gboolean              image_merge_layers_merge_active_group = TRUE;
static gboolean              image_merge_layers_discard_invisible  = FALSE;
static PicmanUnit              image_resize_unit                     = PICMAN_UNIT_PIXEL;
static PicmanUnit              image_scale_unit                      = PICMAN_UNIT_PIXEL;
static PicmanInterpolationType image_scale_interp                    = -1;


/*  public functions  */

void
image_new_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  GtkWidget *widget;
  GtkWidget *dialog;
  return_if_no_widget (widget, data);

  dialog = picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                           gtk_widget_get_screen (widget),
                                           NULL /*ui_manager*/,
                                           "picman-image-new-dialog", -1, FALSE);

  if (dialog)
    {
      PicmanImage *image = action_data_get_image (data);

      image_new_dialog_set (dialog, image, NULL);

      gtk_window_present (GTK_WINDOW (dialog));
    }
}

static void
image_convert_type_dialog_unset (GtkWidget *widget)
{
  g_object_set_data (G_OBJECT (widget), IMAGE_CONVERT_TYPE_DIALOG_KEY, NULL);
}

void
image_convert_base_type_cmd_callback (GtkAction *action,
                                      GtkAction *current,
                                      gpointer   data)
{
  PicmanImage         *image;
  GtkWidget         *widget;
  PicmanDisplay       *display;
  PicmanImageBaseType  value;
  GError            *error = NULL;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);
  return_if_no_display (display, data);

  value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (value == picman_image_get_base_type (image))
    return;

  switch (value)
    {
    case PICMAN_RGB:
    case PICMAN_GRAY:
      if (! picman_image_convert_type (image, value,
                                     0, 0, FALSE, FALSE, FALSE, 0, NULL,
                                     NULL, &error))
        {
          picman_message_literal (image->picman,
				G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
				error->message);
          g_clear_error (&error);
          return;
        }
      break;

    case PICMAN_INDEXED:
      {
        GtkWidget *dialog;

        dialog = g_object_get_data (G_OBJECT (widget),
                                    IMAGE_CONVERT_TYPE_DIALOG_KEY);

        if (! dialog)
          {
            dialog = convert_type_dialog_new (image,
                                              action_data_get_context (data),
                                              widget,
                                              PICMAN_PROGRESS (display));

            g_object_set_data (G_OBJECT (widget),
                               IMAGE_CONVERT_TYPE_DIALOG_KEY, dialog);

            g_signal_connect_object (dialog, "destroy",
                                     G_CALLBACK (image_convert_type_dialog_unset),
                                     widget, G_CONNECT_SWAPPED);
          }

        gtk_window_present (GTK_WINDOW (dialog));
      }
      break;
    }

  picman_image_flush (image);
}

static void
image_convert_precision_dialog_unset (GtkWidget *widget)
{
  g_object_set_data (G_OBJECT (widget), IMAGE_CONVERT_PRECISION_DIALOG_KEY, NULL);
}

void
image_convert_precision_cmd_callback (GtkAction *action,
                                      GtkAction *current,
                                      gpointer   data)
{
  PicmanImage     *image;
  GtkWidget     *widget;
  PicmanDisplay   *display;
  PicmanPrecision  value;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);
  return_if_no_display (display, data);

  value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (value == picman_image_get_precision (image))
    return;

  if (value < picman_image_get_precision (image))
    {
      GtkWidget *dialog;

      dialog = g_object_get_data (G_OBJECT (widget),
                                  IMAGE_CONVERT_PRECISION_DIALOG_KEY);

      if (! dialog)
        {
          dialog = convert_precision_dialog_new (image,
                                                 action_data_get_context (data),
                                                 widget,
                                                 value,
                                                 PICMAN_PROGRESS (display));

          g_object_set_data (G_OBJECT (widget),
                             IMAGE_CONVERT_PRECISION_DIALOG_KEY, dialog);

          g_signal_connect_object (dialog, "destroy",
                                   G_CALLBACK (image_convert_precision_dialog_unset),
                                   widget, G_CONNECT_SWAPPED);
        }

      gtk_window_present (GTK_WINDOW (dialog));
    }
  else
    {
      picman_image_convert_precision (image, value, 0, 0, 0,
                                    PICMAN_PROGRESS (display));
    }

  picman_image_flush (image);
}

void
image_resize_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  ImageResizeOptions *options;
  PicmanImage          *image;
  GtkWidget          *widget;
  PicmanDisplay        *display;
  GtkWidget          *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);
  return_if_no_display (display, data);

  options = g_slice_new (ImageResizeOptions);

  options->display = display;
  options->context = action_data_get_context (data);

  if (image_resize_unit != PICMAN_UNIT_PERCENT)
    image_resize_unit = picman_display_get_shell (display)->unit;

  dialog = resize_dialog_new (PICMAN_VIEWABLE (image),
                              action_data_get_context (data),
                              _("Set Image Canvas Size"), "picman-image-resize",
                              widget,
                              picman_standard_help_func, PICMAN_HELP_IMAGE_RESIZE,
                              image_resize_unit,
                              image_resize_callback,
                              options);

  g_signal_connect_object (display, "disconnect",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  g_object_weak_ref (G_OBJECT (dialog),
                     (GWeakNotify) image_resize_options_free, options);

  gtk_widget_show (dialog);
}

void
image_resize_to_layers_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanDisplay  *display;
  PicmanImage    *image;
  PicmanProgress *progress;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  progress = picman_progress_start (PICMAN_PROGRESS (display),
                                  _("Resizing"), FALSE);

  picman_image_resize_to_layers (image,
                               action_data_get_context (data),
                               progress);

  if (progress)
    picman_progress_end (progress);

  picman_image_flush (image);
}

void
image_resize_to_selection_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanDisplay  *display;
  PicmanImage    *image;
  PicmanProgress *progress;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  progress = picman_progress_start (PICMAN_PROGRESS (display),
                                  _("Resizing"), FALSE);

  picman_image_resize_to_selection (image,
                                  action_data_get_context (data),
                                  progress);

  if (progress)
    picman_progress_end (progress);

  picman_image_flush (image);
}

void
image_print_size_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *widget;
  GtkWidget   *dialog;
  return_if_no_display (display, data);
  return_if_no_widget (widget, data);

  image = picman_display_get_image (display);

  dialog = print_size_dialog_new (image,
                                  action_data_get_context (data),
                                  _("Set Image Print Resolution"),
                                  "picman-image-print-size",
                                  widget,
                                  picman_standard_help_func,
                                  PICMAN_HELP_IMAGE_PRINT_SIZE,
                                  image_print_size_callback,
                                  NULL);

  g_signal_connect_object (display, "disconnect",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  gtk_widget_show (dialog);
}

void
image_scale_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *widget;
  GtkWidget   *dialog;
  return_if_no_display (display, data);
  return_if_no_widget (widget, data);

  image = picman_display_get_image (display);

  if (image_scale_unit != PICMAN_UNIT_PERCENT)
    image_scale_unit = picman_display_get_shell (display)->unit;

  if (image_scale_interp == -1)
    image_scale_interp = display->picman->config->interpolation_type;

  dialog = image_scale_dialog_new (image,
                                   action_data_get_context (data),
                                   widget,
                                   image_scale_unit,
                                   image_scale_interp,
                                   image_scale_callback,
                                   display);

  g_signal_connect_object (display, "disconnect",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  gtk_widget_show (dialog);
}

void
image_flip_cmd_callback (GtkAction *action,
                         gint       value,
                         gpointer   data)
{
  PicmanDisplay  *display;
  PicmanImage    *image;
  PicmanProgress *progress;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  progress = picman_progress_start (PICMAN_PROGRESS (display),
                                  _("Flipping"), FALSE);

  picman_image_flip (image, action_data_get_context (data),
                   (PicmanOrientationType) value, progress);

  if (progress)
    picman_progress_end (progress);

  picman_image_flush (image);
}

void
image_rotate_cmd_callback (GtkAction *action,
                           gint       value,
                           gpointer   data)
{
  PicmanDisplay  *display;
  PicmanImage    *image;
  PicmanProgress *progress;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  progress = picman_progress_start (PICMAN_PROGRESS (display),
                                  _("Rotating"), FALSE);

  picman_image_rotate (image, action_data_get_context (data),
                     (PicmanRotationType) value, progress);

  if (progress)
    picman_progress_end (progress);

  picman_image_flush (image);
}

void
image_crop_to_selection_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  gint       x1, y1, x2, y2;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  if (! picman_channel_bounds (picman_image_get_mask (image),
                             &x1, &y1, &x2, &y2))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("Cannot crop because the current selection is empty."));
      return;
    }

  picman_image_crop (image, action_data_get_context (data),
                   x1, y1, x2, y2, TRUE);
  picman_image_flush (image);
}

void
image_crop_to_content_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  gint       x1, y1, x2, y2;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  if (! picman_pickable_auto_shrink (PICMAN_PICKABLE (picman_image_get_projection (image)),
                                   0, 0,
                                   picman_image_get_width  (image),
                                   picman_image_get_height (image),
                                   &x1, &y1, &x2, &y2))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("Cannot crop because the image has no content."));
      return;
    }

  picman_image_crop (image, action_data_get_context (data),
                   x1, y1, x2, y2, TRUE);
  picman_image_flush (image);
}

void
image_duplicate_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanDisplay      *display;
  PicmanImage        *image;
  PicmanDisplayShell *shell;
  PicmanImage        *new_image;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);
  shell = picman_display_get_shell (display);

  new_image = picman_image_duplicate (image);

  picman_create_display (new_image->picman,
                       new_image,
                       shell->unit,
                       picman_zoom_model_get_factor (shell->zoom));

  g_object_unref (new_image);
}

void
image_merge_layers_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  ImageMergeLayersDialog *dialog;
  PicmanImage              *image;
  GtkWidget              *widget;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = image_merge_layers_dialog_new (image,
                                          action_data_get_context (data),
                                          widget,
                                          image_merge_layers_type,
                                          image_merge_layers_merge_active_group,
                                          image_merge_layers_discard_invisible);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (image_merge_layers_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
image_flatten_image_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  picman_image_flatten (image, action_data_get_context (data));
  picman_image_flush (image);
}

void
image_configure_grid_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanDisplay      *display;
  PicmanImage        *image;
  PicmanDisplayShell *shell;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);
  shell = picman_display_get_shell (display);

  if (! shell->grid_dialog)
    {
      shell->grid_dialog = grid_dialog_new (image,
                                            action_data_get_context (data),
                                            GTK_WIDGET (shell));

      gtk_window_set_transient_for (GTK_WINDOW (shell->grid_dialog),
                                    GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (shell))));
      gtk_window_set_destroy_with_parent (GTK_WINDOW (shell->grid_dialog),
                                          TRUE);

      g_object_add_weak_pointer (G_OBJECT (shell->grid_dialog),
                                 (gpointer) &shell->grid_dialog);
    }

  gtk_window_present (GTK_WINDOW (shell->grid_dialog));
}

void
image_properties_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay      *display;
  PicmanImage        *image;
  PicmanDisplayShell *shell;
  GtkWidget        *dialog;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);
  shell = picman_display_get_shell (display);

  dialog = image_properties_dialog_new (image,
                                        action_data_get_context (data),
                                        GTK_WIDGET (shell));

  gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (shell))));
  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog),
                                      TRUE);

  gtk_window_present (GTK_WINDOW (dialog));
}


/*  private functions  */

static void
image_resize_callback (GtkWidget    *dialog,
                       PicmanViewable *viewable,
                       gint          width,
                       gint          height,
                       PicmanUnit      unit,
                       gint          offset_x,
                       gint          offset_y,
                       PicmanItemSet   layer_set,
                       gboolean      resize_text_layers,
                       gpointer      data)
{
  ImageResizeOptions *options = data;

  image_resize_unit = unit;

  if (width > 0 && height > 0)
    {
      PicmanImage    *image   = PICMAN_IMAGE (viewable);
      PicmanDisplay  *display = options->display;
      PicmanContext  *context = options->context;
      PicmanProgress *progress;

      gtk_widget_destroy (dialog);

      if (width  == picman_image_get_width  (image) &&
          height == picman_image_get_height (image))
        return;

      progress = picman_progress_start (PICMAN_PROGRESS (display),
                                      _("Resizing"), FALSE);

      picman_image_resize_with_layers (image,
                                     context,
                                     width, height, offset_x, offset_y,
                                     layer_set,
                                     resize_text_layers,
                                     progress);

      if (progress)
        picman_progress_end (progress);

      picman_image_flush (image);
    }
  else
    {
      g_warning ("Resize Error: "
                 "Both width and height must be greater than zero.");
    }
}

static void
image_resize_options_free (ImageResizeOptions *options)
{
  g_slice_free (ImageResizeOptions, options);
}

static void
image_print_size_callback (GtkWidget *dialog,
                           PicmanImage *image,
                           gdouble    xresolution,
                           gdouble    yresolution,
                           PicmanUnit   resolution_unit,
                           gpointer   data)
{
  gdouble xres;
  gdouble yres;

  gtk_widget_destroy (dialog);

  picman_image_get_resolution (image, &xres, &yres);

  if (xresolution     == xres &&
      yresolution     == yres &&
      resolution_unit == picman_image_get_unit (image))
    return;

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_SCALE,
                               _("Change Print Size"));

  picman_image_set_resolution (image, xresolution, yresolution);
  picman_image_set_unit (image, resolution_unit);

  picman_image_undo_group_end (image);

  picman_image_flush (image);
}

static void
image_scale_callback (GtkWidget              *dialog,
                      PicmanViewable           *viewable,
                      gint                    width,
                      gint                    height,
                      PicmanUnit                unit,
                      PicmanInterpolationType   interpolation,
                      gdouble                 xresolution,
                      gdouble                 yresolution,
                      PicmanUnit                resolution_unit,
                      gpointer                user_data)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);
  gdouble    xres;
  gdouble    yres;

  image_scale_unit   = unit;
  image_scale_interp = interpolation;

  picman_image_get_resolution (image, &xres, &yres);

  if (width > 0 && height > 0)
    {
      if (width           == picman_image_get_width  (image) &&
          height          == picman_image_get_height (image) &&
          xresolution     == xres                          &&
          yresolution     == yres                          &&
          resolution_unit == picman_image_get_unit (image))
        return;

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_SCALE,
                                   _("Scale Image"));

      picman_image_set_resolution (image, xresolution, yresolution);
      picman_image_set_unit (image, resolution_unit);

      if (width  != picman_image_get_width  (image) ||
          height != picman_image_get_height (image))
        {
          PicmanProgress *progress;

          progress = picman_progress_start (PICMAN_PROGRESS (user_data),
                                          _("Scaling"), FALSE);

          picman_image_scale (image, width, height, interpolation, progress);

          if (progress)
            picman_progress_end (progress);
        }

      picman_image_undo_group_end (image);

      picman_image_flush (image);
    }
  else
    {
      g_warning ("Scale Error: "
                 "Both width and height must be greater than zero.");
    }
}

static void
image_merge_layers_response (GtkWidget              *widget,
                             gint                    response_id,
                             ImageMergeLayersDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      image_merge_layers_type               = dialog->merge_type;
      image_merge_layers_merge_active_group = dialog->merge_active_group;
      image_merge_layers_discard_invisible  = dialog->discard_invisible;

      picman_image_merge_visible_layers (dialog->image,
                                       dialog->context,
                                       image_merge_layers_type,
                                       image_merge_layers_merge_active_group,
                                       image_merge_layers_discard_invisible);

      picman_image_flush (dialog->image);
    }

  gtk_widget_destroy (widget);
}
