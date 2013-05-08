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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmanimage.h"
#include "core/picmanimage-quick-mask.h"

#include "widgets/picmanhelp-ids.h"

#include "dialogs/channel-options-dialog.h"

#include "actions.h"
#include "quick-mask-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   quick_mask_configure_response (GtkWidget            *widget,
                                             gint                  response_id,
                                             ChannelOptionsDialog *options);


/*  public functionss */

void
quick_mask_toggle_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage *image;
  gboolean   active;
  return_if_no_image (image, data);

  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (active != picman_image_get_quick_mask_state (image))
    {
      picman_image_set_quick_mask_state (image, active);
      picman_image_flush (image);
    }
}

void
quick_mask_invert_cmd_callback (GtkAction *action,
                                GtkAction *current,
                                gpointer   data)
{
  PicmanImage *image;
  gint       value;
  return_if_no_image (image, data);

  value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (value != picman_image_get_quick_mask_inverted (image))
    {
      picman_image_quick_mask_invert (image);
      picman_image_flush (image);
    }
}

void
quick_mask_configure_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  ChannelOptionsDialog *options;
  PicmanImage            *image;
  GtkWidget            *widget;
  PicmanRGB               color;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  picman_image_get_quick_mask_color (image, &color);

  options = channel_options_dialog_new (image, NULL,
                                        action_data_get_context (data),
                                        widget,
                                        &color,
                                        NULL,
                                        _("Quick Mask Attributes"),
                                        "picman-quick-mask-edit",
                                        PICMAN_STOCK_QUICK_MASK_ON,
                                        _("Edit Quick Mask Attributes"),
                                        PICMAN_HELP_QUICK_MASK_EDIT,
                                        _("Edit Quick Mask Color"),
                                        _("_Mask opacity:"),
                                        FALSE);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (quick_mask_configure_response),
                    options);

  gtk_widget_show (options->dialog);
}


/*  private functions  */

static void
quick_mask_configure_response (GtkWidget            *widget,
                               gint                  response_id,
                               ChannelOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanRGB old_color;
      PicmanRGB new_color;

      picman_image_get_quick_mask_color (options->image, &old_color);
      picman_color_button_get_color (PICMAN_COLOR_BUTTON (options->color_panel),
                                   &new_color);

      if (picman_rgba_distance (&old_color, &new_color) > 0.0001)
        {
          picman_image_set_quick_mask_color (options->image, &new_color);

          picman_image_flush (options->image);
        }
    }

  gtk_widget_destroy (options->dialog);
}
