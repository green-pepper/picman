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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-scale.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "scale-dialog.h"

#include "image-scale-dialog.h"

#include "picman-intl.h"


typedef struct
{
  GtkWidget             *dialog;

  PicmanImage             *image;

  gint                   width;
  gint                   height;
  PicmanUnit               unit;
  PicmanInterpolationType  interpolation;
  gdouble                xresolution;
  gdouble                yresolution;
  PicmanUnit               resolution_unit;

  PicmanScaleCallback      callback;
  gpointer               user_data;
} ImageScaleDialog;


static void        image_scale_callback (GtkWidget             *widget,
                                         PicmanViewable          *viewable,
                                         gint                   width,
                                         gint                   height,
                                         PicmanUnit               unit,
                                         PicmanInterpolationType  interpolation,
                                         gdouble                xresolution,
                                         gdouble                yresolution,
                                         PicmanUnit               resolution_unit,
                                         gpointer               data);

static void        image_scale_dialog_free      (ImageScaleDialog *dialog);

static GtkWidget * image_scale_confirm_dialog   (ImageScaleDialog *dialog);
static void        image_scale_confirm_large    (ImageScaleDialog *dialog,
                                                 gint64            new_memsize,
                                                 gint64            max_memsize);
static void        image_scale_confirm_small    (ImageScaleDialog *dialog);
static void        image_scale_confirm_response (GtkWidget        *widget,
                                                 gint              response_id,
                                                 ImageScaleDialog *dialog);


/*  public functions  */

GtkWidget *
image_scale_dialog_new (PicmanImage             *image,
                        PicmanContext           *context,
                        GtkWidget             *parent,
                        PicmanUnit               unit,
                        PicmanInterpolationType  interpolation,
                        PicmanScaleCallback      callback,
                        gpointer               user_data)
{
  ImageScaleDialog *dialog;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (callback != NULL, NULL);

  dialog = g_slice_new0 (ImageScaleDialog);

  dialog->image  = image;
  dialog->dialog = scale_dialog_new (PICMAN_VIEWABLE (image), context,
                                     C_("dialog-title", "Scale Image"),
                                     "picman-image-scale",
                                     parent,
                                     picman_standard_help_func,
                                     PICMAN_HELP_IMAGE_SCALE,
                                     unit,
                                     interpolation,
                                     image_scale_callback,
                                     dialog);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) image_scale_dialog_free, dialog);

  dialog->callback  = callback;
  dialog->user_data = user_data;

  return dialog->dialog;
}

static void
image_scale_callback (GtkWidget             *widget,
                      PicmanViewable          *viewable,
                      gint                   width,
                      gint                   height,
                      PicmanUnit               unit,
                      PicmanInterpolationType  interpolation,
                      gdouble                xresolution,
                      gdouble                yresolution,
                      PicmanUnit               resolution_unit,
                      gpointer               data)
{
  ImageScaleDialog        *dialog = data;
  PicmanImage               *image  = PICMAN_IMAGE (viewable);
  PicmanImageScaleCheckType  scale_check;
  gint64                   max_memsize;
  gint64                   new_memsize;

  dialog->width           = width;
  dialog->height          = height;
  dialog->unit            = unit;
  dialog->interpolation   = interpolation;
  dialog->xresolution     = xresolution;
  dialog->yresolution     = yresolution;
  dialog->resolution_unit = resolution_unit;

  gtk_widget_set_sensitive (widget, FALSE);

  max_memsize = PICMAN_GUI_CONFIG (image->picman->config)->max_new_image_size;

  scale_check = picman_image_scale_check (image,
                                        width, height, max_memsize,
                                        &new_memsize);
  switch (scale_check)
    {
    case PICMAN_IMAGE_SCALE_TOO_BIG:
      image_scale_confirm_large (dialog, new_memsize, max_memsize);
      break;

    case PICMAN_IMAGE_SCALE_TOO_SMALL:
      image_scale_confirm_small (dialog);
      break;

    case PICMAN_IMAGE_SCALE_OK:
      gtk_widget_hide (widget);
      dialog->callback (dialog->dialog,
                        PICMAN_VIEWABLE (dialog->image),
                        dialog->width,
                        dialog->height,
                        dialog->unit,
                        dialog->interpolation,
                        dialog->xresolution,
                        dialog->yresolution,
                        dialog->resolution_unit,
                        dialog->user_data);
      gtk_widget_destroy (widget);

      /* remember the last used unit */
      g_object_set_data (G_OBJECT (image),
                         "scale-dialog-unit", GINT_TO_POINTER (unit));
      break;
    }
}

static void
image_scale_dialog_free (ImageScaleDialog *dialog)
{
  g_slice_free (ImageScaleDialog, dialog);
}

static GtkWidget *
image_scale_confirm_dialog (ImageScaleDialog *dialog)
{
  GtkWidget *widget;

  widget = picman_message_dialog_new (_("Confirm Scaling"),
                                    PICMAN_STOCK_WARNING,
                                    dialog->dialog,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func,
                                    PICMAN_HELP_IMAGE_SCALE_WARNING,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    PICMAN_STOCK_SCALE, GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (widget),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (widget, "response",
                    G_CALLBACK (image_scale_confirm_response),
                    dialog);

  return widget;
}

static void
image_scale_confirm_large (ImageScaleDialog *dialog,
                           gint64            new_memsize,
                           gint64            max_memsize)
{
  GtkWidget *widget = image_scale_confirm_dialog (dialog);
  gchar     *size;

  size = g_format_size (new_memsize);
  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (widget)->box,
                                     _("You are trying to create an image "
                                       "with a size of %s."), size);
  g_free (size);

  size = g_format_size (max_memsize);
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (widget)->box,
                             _("Scaling the image to the chosen size will "
                               "make it use more memory than what is "
                               "configured as \"Maximum Image Size\" in the "
                               "Preferences dialog (currently %s)."), size);
  g_free (size);

  gtk_widget_show (widget);
}

static void
image_scale_confirm_small (ImageScaleDialog *dialog)
{
  GtkWidget *widget = image_scale_confirm_dialog (dialog);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (widget)->box,
                                     _("Scaling the image to the chosen size "
                                       "will shrink some layers completely "
                                       "away."));
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (widget)->box,
                             _("Is this what you want to do?"));

  gtk_widget_show (widget);
}

static void
image_scale_confirm_response (GtkWidget        *widget,
                              gint              response_id,
                              ImageScaleDialog *dialog)
{
  gtk_widget_destroy (widget);

  if (response_id == GTK_RESPONSE_OK)
    {
      gtk_widget_hide (dialog->dialog);
      dialog->callback (dialog->dialog,
                        PICMAN_VIEWABLE (dialog->image),
                        dialog->width,
                        dialog->height,
                        dialog->unit,
                        dialog->interpolation,
                        dialog->xresolution,
                        dialog->yresolution,
                        dialog->resolution_unit,
                        dialog->user_data);
      gtk_widget_destroy (dialog->dialog);
    }
  else
    {
      gtk_widget_set_sensitive (dialog->dialog, TRUE);
    }
}
