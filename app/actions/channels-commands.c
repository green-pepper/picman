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
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picmanchannel.h"
#include "core/picmanchannel-select.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"

#include "widgets/picmanaction.h"
#include "widgets/picmancolorpanel.h"
#include "widgets/picmancomponenteditor.h"
#include "widgets/picmandock.h"
#include "widgets/picmanhelp-ids.h"

#include "dialogs/channel-options-dialog.h"

#include "actions.h"
#include "channels-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   channels_new_channel_response  (GtkWidget            *widget,
                                              gint                  response_id,
                                              ChannelOptionsDialog *options);
static void   channels_edit_channel_response (GtkWidget            *widget,
                                              gint                  response_id,
                                              ChannelOptionsDialog *options);


/*  private variables  */

static gchar   *channel_name  = NULL;
static PicmanRGB  channel_color = { 0.0, 0.0, 0.0, 0.5 };


/*  public functions  */

void
channels_edit_attributes_cmd_callback (GtkAction *action,
                                       gpointer   data)
{
  ChannelOptionsDialog *options;
  PicmanImage            *image;
  PicmanChannel          *channel;
  GtkWidget            *widget;
  return_if_no_channel (image, channel, data);
  return_if_no_widget (widget, data);

  options = channel_options_dialog_new (image, channel,
                                        action_data_get_context (data),
                                        widget,
                                        &channel->color,
                                        picman_object_get_name (channel),
                                        _("Channel Attributes"),
                                        "picman-channel-edit",
                                        GTK_STOCK_EDIT,
                                        _("Edit Channel Attributes"),
                                        PICMAN_HELP_CHANNEL_EDIT,
                                        _("Edit Channel Color"),
                                        _("_Fill opacity:"),
                                        FALSE);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (channels_edit_channel_response),
                    options);

  gtk_widget_show (options->dialog);
}

void
channels_new_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  ChannelOptionsDialog *options;
  PicmanImage            *image;
  GtkWidget            *widget;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  options = channel_options_dialog_new (image, NULL,
                                        action_data_get_context (data),
                                        widget,
                                        &channel_color,
                                        channel_name ? channel_name :
                                        _("Channel"),
                                        _("New Channel"),
                                        "picman-channel-new",
                                        PICMAN_STOCK_CHANNEL,
                                        _("New Channel Options"),
                                        PICMAN_HELP_CHANNEL_NEW,
                                        _("New Channel Color"),
                                        _("_Fill opacity:"),
                                        TRUE);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (channels_new_channel_response),
                    options);

  gtk_widget_show (options->dialog);
}

void
channels_new_last_vals_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *new_channel;
  gint         width, height;
  PicmanRGB      color;
  return_if_no_image (image, data);

  if (PICMAN_IS_CHANNEL (PICMAN_ACTION (action)->viewable))
    {
      PicmanChannel *template = PICMAN_CHANNEL (PICMAN_ACTION (action)->viewable);

      width  = picman_item_get_width  (PICMAN_ITEM (template));
      height = picman_item_get_height (PICMAN_ITEM (template));
      color  = template->color;
    }
  else
    {
      width  = picman_image_get_width (image);
      height = picman_image_get_height (image);
      color  = channel_color;
    }

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                               _("New Channel"));

  new_channel = picman_channel_new (image, width, height,
                                  channel_name, &color);

  picman_drawable_fill_by_type (PICMAN_DRAWABLE (new_channel),
                              action_data_get_context (data),
                              PICMAN_TRANSPARENT_FILL);

  picman_image_add_channel (image, new_channel,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_undo_group_end (image);

  picman_image_flush (image);
}

void
channels_raise_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *channel;
  return_if_no_channel (image, channel, data);

  picman_image_raise_item (image, PICMAN_ITEM (channel), NULL);
  picman_image_flush (image);
}

void
channels_raise_to_top_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *channel;
  return_if_no_channel (image, channel, data);

  picman_image_raise_item_to_top (image, PICMAN_ITEM (channel));
  picman_image_flush (image);
}

void
channels_lower_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *channel;
  return_if_no_channel (image, channel, data);

  picman_image_lower_item (image, PICMAN_ITEM (channel), NULL);
  picman_image_flush (image);
}

void
channels_lower_to_bottom_cmd_callback (GtkAction *action,
                                       gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *channel;
  return_if_no_channel (image, channel, data);

  picman_image_lower_item_to_bottom (image, PICMAN_ITEM (channel));
  picman_image_flush (image);
}

void
channels_duplicate_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *new_channel;
  PicmanChannel *parent = PICMAN_IMAGE_ACTIVE_PARENT;

  if (PICMAN_IS_COMPONENT_EDITOR (data))
    {
      PicmanChannelType  component;
      const gchar     *desc;
      gchar           *name;
      return_if_no_image (image, data);

      component = PICMAN_COMPONENT_EDITOR (data)->clicked_component;

      picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                           NULL, NULL, &desc, NULL);

      name = g_strdup_printf (_("%s Channel Copy"), desc);

      new_channel = picman_channel_new_from_component (image, component,
                                                     name, NULL);

      /*  copied components are invisible by default so subsequent copies
       *  of components don't affect each other
       */
      picman_item_set_visible (PICMAN_ITEM (new_channel), FALSE, FALSE);

      g_free (name);
    }
  else
    {
      PicmanChannel *channel;
      return_if_no_channel (image, channel, data);

      new_channel =
        PICMAN_CHANNEL (picman_item_duplicate (PICMAN_ITEM (channel),
                                           G_TYPE_FROM_INSTANCE (channel)));

      /*  use the actual parent here, not PICMAN_IMAGE_ACTIVE_PARENT because
       *  the latter would add a duplicated group inside itself instead of
       *  above it
       */
      parent = picman_channel_get_parent (channel);
    }

  picman_image_add_channel (image, new_channel, parent, -1, TRUE);

  picman_image_flush (image);
}

void
channels_delete_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage   *image;
  PicmanChannel *channel;
  return_if_no_channel (image, channel, data);

  picman_image_remove_channel (image, channel, TRUE, NULL);
  picman_image_flush (image);
}

void
channels_to_selection_cmd_callback (GtkAction *action,
                                    gint       value,
                                    gpointer   data)
{
  PicmanChannelOps  op;
  PicmanImage      *image;

  op = (PicmanChannelOps) value;

  if (PICMAN_IS_COMPONENT_EDITOR (data))
    {
      PicmanChannelType component;
      return_if_no_image (image, data);

      component = PICMAN_COMPONENT_EDITOR (data)->clicked_component;

      picman_channel_select_component (picman_image_get_mask (image), component,
                                     op, FALSE, 0.0, 0.0);
    }
  else
    {
      PicmanChannel *channel;
      return_if_no_channel (image, channel, data);

      picman_item_to_selection (PICMAN_ITEM (channel),
                              op, TRUE, FALSE, 0.0, 0.0);
    }

  picman_image_flush (image);
}


/*  private functions  */

static void
channels_new_channel_response (GtkWidget            *widget,
                               gint                  response_id,
                               ChannelOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanChannel *new_channel;

      if (channel_name)
        g_free (channel_name);

      channel_name =
        g_strdup (gtk_entry_get_text (GTK_ENTRY (options->name_entry)));

      picman_color_button_get_color (PICMAN_COLOR_BUTTON (options->color_panel),
                                   &channel_color);

      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (options->save_sel_checkbutton)))
        {
          PicmanChannel *selection = picman_image_get_mask (options->image);

          new_channel =
            PICMAN_CHANNEL (picman_item_duplicate (PICMAN_ITEM (selection),
                                               PICMAN_TYPE_CHANNEL));

          picman_object_set_name (PICMAN_OBJECT (new_channel), channel_name);
          picman_channel_set_color (new_channel, &channel_color, FALSE);
        }
      else
        {
          new_channel = picman_channel_new (options->image,
                                          picman_image_get_width  (options->image),
                                          picman_image_get_height (options->image),
                                          channel_name,
                                          &channel_color);

          picman_drawable_fill_by_type (PICMAN_DRAWABLE (new_channel),
                                      options->context,
                                      PICMAN_TRANSPARENT_FILL);
        }

      picman_image_add_channel (options->image, new_channel,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_flush (options->image);
    }

  gtk_widget_destroy (options->dialog);
}

static void
channels_edit_channel_response (GtkWidget            *widget,
                                gint                  response_id,
                                ChannelOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanChannel *channel = options->channel;
      const gchar *new_name;
      PicmanRGB      color;
      gboolean     name_changed  = FALSE;
      gboolean     color_changed = FALSE;

      new_name = gtk_entry_get_text (GTK_ENTRY (options->name_entry));

      picman_color_button_get_color (PICMAN_COLOR_BUTTON (options->color_panel),
                                   &color);

      if (strcmp (new_name, picman_object_get_name (channel)))
        name_changed = TRUE;

      if (picman_rgba_distance (&color, &channel->color) > 0.0001)
        color_changed = TRUE;

      if (name_changed && color_changed)
        picman_image_undo_group_start (options->image,
                                     PICMAN_UNDO_GROUP_ITEM_PROPERTIES,
                                     _("Channel Attributes"));

      if (name_changed)
        picman_item_rename (PICMAN_ITEM (channel), new_name, NULL);

      if (color_changed)
        picman_channel_set_color (channel, &color, TRUE);

      if (name_changed && color_changed)
        picman_image_undo_group_end (options->image);

      if (name_changed || color_changed)
        picman_image_flush (options->image);
    }

  gtk_widget_destroy (options->dialog);
}
