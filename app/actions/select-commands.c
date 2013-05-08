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

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmanimage.h"
#include "core/picmanselection.h"
#include "core/picmanstrokeoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanwindowstrategy.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "dialogs/stroke-dialog.h"

#include "actions.h"
#include "select-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   select_feather_callback (GtkWidget *widget,
                                       gdouble    size,
                                       PicmanUnit   unit,
                                       gpointer   data);
static void   select_border_callback  (GtkWidget *widget,
                                       gdouble    size,
                                       PicmanUnit   unit,
                                       gpointer   data);
static void   select_grow_callback    (GtkWidget *widget,
                                       gdouble    size,
                                       PicmanUnit   unit,
                                       gpointer   data);
static void   select_shrink_callback  (GtkWidget *widget,
                                       gdouble    size,
                                       PicmanUnit   unit,
                                       gpointer   data);


/*  private variables  */

static gdouble   select_feather_radius    = 5.0;
static gint      select_grow_pixels       = 1;
static gint      select_shrink_pixels     = 1;
static gboolean  select_shrink_edge_lock  = FALSE;
static gint      select_border_radius     = 5;
static gboolean  select_border_feather    = FALSE;
static gboolean  select_border_edge_lock  = FALSE;


/*  public functions  */

void
select_invert_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  picman_channel_invert (picman_image_get_mask (image), TRUE);
  picman_image_flush (image);
}

void
select_all_cmd_callback (GtkAction *action,
                         gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  picman_channel_all (picman_image_get_mask (image), TRUE);
  picman_image_flush (image);
}

void
select_none_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  picman_channel_clear (picman_image_get_mask (image), NULL, TRUE);
  picman_image_flush (image);
}

void
select_float_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  GError    *error = NULL;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  if (picman_selection_float (PICMAN_SELECTION (picman_image_get_mask (image)),
                            picman_image_get_active_drawable (image),
                            action_data_get_context (data),
                            TRUE, 0, 0, &error))
    {
      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

void
select_feather_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  gdouble      xres;
  gdouble      yres;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  picman_image_get_resolution (image, &xres, &yres);

  dialog = picman_query_size_box (_("Feather Selection"),
                                GTK_WIDGET (picman_display_get_shell (display)),
                                picman_standard_help_func,
                                PICMAN_HELP_SELECTION_FEATHER,
                                _("Feather selection by"),
                                select_feather_radius, 0, 32767, 3,
                                picman_display_get_shell (display)->unit,
                                MIN (xres, yres),
                                FALSE,
                                G_OBJECT (image), "disconnect",
                                select_feather_callback, image);
  gtk_widget_show (dialog);
}

void
select_sharpen_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  picman_channel_sharpen (picman_image_get_mask (image), TRUE);
  picman_image_flush (image);
}

void
select_shrink_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  GtkWidget   *button;
  gdouble      xres;
  gdouble      yres;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  picman_image_get_resolution (image, &xres, &yres);

  dialog = picman_query_size_box (_("Shrink Selection"),
                                GTK_WIDGET (picman_display_get_shell (display)),
                                picman_standard_help_func,
                                PICMAN_HELP_SELECTION_SHRINK,
                                _("Shrink selection by"),
                                select_shrink_pixels, 1, 32767, 0,
                                picman_display_get_shell (display)->unit,
                                MIN (xres, yres),
                                FALSE,
                                G_OBJECT (image), "disconnect",
                                select_shrink_callback, image);

  button = gtk_check_button_new_with_mnemonic (_("_Shrink from image border"));

  gtk_box_pack_start (GTK_BOX (PICMAN_QUERY_BOX_VBOX (dialog)), button,
                      FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (dialog), "edge-lock-toggle", button);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                ! select_shrink_edge_lock);
  gtk_widget_show (button);

  gtk_widget_show (dialog);
}

void
select_grow_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  gdouble      xres;
  gdouble      yres;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  picman_image_get_resolution (image, &xres, &yres);

  dialog = picman_query_size_box (_("Grow Selection"),
                                GTK_WIDGET (picman_display_get_shell (display)),
                                picman_standard_help_func,
                                PICMAN_HELP_SELECTION_GROW,
                                _("Grow selection by"),
                                select_grow_pixels, 1, 32767, 0,
                                picman_display_get_shell (display)->unit,
                                MIN (xres, yres),
                                FALSE,
                                G_OBJECT (image), "disconnect",
                                select_grow_callback, image);
  gtk_widget_show (dialog);
}

void
select_border_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  GtkWidget   *button;
  gdouble      xres;
  gdouble      yres;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  picman_image_get_resolution (image, &xres, &yres);

  dialog = picman_query_size_box (_("Border Selection"),
                                GTK_WIDGET (picman_display_get_shell (display)),
                                picman_standard_help_func,
                                PICMAN_HELP_SELECTION_BORDER,
                                _("Border selection by"),
                                select_border_radius, 1, 32767, 0,
                                picman_display_get_shell (display)->unit,
                                MIN (xres, yres),
                                FALSE,
                                G_OBJECT (image), "disconnect",
                                select_border_callback, image);

  /* Feather button */
  button = gtk_check_button_new_with_mnemonic (_("_Feather border"));

  gtk_box_pack_start (GTK_BOX (PICMAN_QUERY_BOX_VBOX (dialog)), button,
                      FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (dialog), "border-feather-toggle", button);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                select_border_feather);
  gtk_widget_show (button);


  /* Edge lock button */
  button = gtk_check_button_new_with_mnemonic (_("_Lock selection to image edges"));

  gtk_box_pack_start (GTK_BOX (PICMAN_QUERY_BOX_VBOX (dialog)), button,
                      FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT (dialog), "edge-lock-toggle", button);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                select_border_edge_lock);
  gtk_widget_show (button);



  gtk_widget_show (dialog);
}

void
select_save_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  picman_selection_save (PICMAN_SELECTION (picman_image_get_mask (image)));
  picman_image_flush (image);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (image->picman)),
                                             image->picman,
                                             picman_dialog_factory_get_singleton (),
                                             gtk_widget_get_screen (widget),
                                             "picman-channel-list");
}

void
select_stroke_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  GtkWidget    *widget;
  GtkWidget    *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to stroke to."));
      return;
    }

  dialog = stroke_dialog_new (PICMAN_ITEM (picman_image_get_mask (image)),
                              action_data_get_context (data),
                              _("Stroke Selection"),
                              PICMAN_STOCK_SELECTION_STROKE,
                              PICMAN_HELP_SELECTION_STROKE,
                              widget);
  gtk_widget_show (dialog);
}

void
select_stroke_last_vals_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage         *image;
  PicmanDrawable      *drawable;
  PicmanContext       *context;
  GtkWidget         *widget;
  PicmanStrokeOptions *options;
  GError            *error = NULL;
  return_if_no_image (image, data);
  return_if_no_context (context, data);
  return_if_no_widget (widget, data);

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to stroke to."));
      return;
    }

  options = g_object_get_data (G_OBJECT (image->picman), "saved-stroke-options");

  if (options)
    g_object_ref (options);
  else
    options = picman_stroke_options_new (image->picman, context, TRUE);

  if (! picman_item_stroke (PICMAN_ITEM (picman_image_get_mask (image)),
                          drawable, context, options, FALSE, TRUE, NULL, &error))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
  else
    {
      picman_image_flush (image);
    }

  g_object_unref (options);
}


/*  private functions  */

static void
select_feather_callback (GtkWidget *widget,
                         gdouble    size,
                         PicmanUnit   unit,
                         gpointer   data)
{
  PicmanImage *image = PICMAN_IMAGE (data);
  gdouble    radius_x;
  gdouble    radius_y;

  radius_x = radius_y = select_feather_radius = size;

  if (unit != PICMAN_UNIT_PIXEL)
    {
      gdouble xres;
      gdouble yres;
      gdouble factor;

      picman_image_get_resolution (image, &xres, &yres);

      factor = (MAX (xres, yres) /
                MIN (xres, yres));

      if (xres == MIN (xres, yres))
        radius_y *= factor;
      else
        radius_x *= factor;
    }

  picman_channel_feather (picman_image_get_mask (image), radius_x, radius_y, TRUE);
  picman_image_flush (image);
}

static void
select_border_callback (GtkWidget *widget,
                        gdouble    size,
                        PicmanUnit   unit,
                        gpointer   data)
{
  PicmanImage *image  = PICMAN_IMAGE (data);
  GtkWidget *feather_button = g_object_get_data (G_OBJECT (widget),
                                                 "border-feather-toggle");
  GtkWidget *edge_lock_button = g_object_get_data (G_OBJECT (widget),
                                                   "edge-lock-toggle");
  gdouble    radius_x;
  gdouble    radius_y;

  radius_x = radius_y = select_border_radius = ROUND (size);

  select_border_feather =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (feather_button));

  select_border_edge_lock =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (edge_lock_button));

  if (unit != PICMAN_UNIT_PIXEL)
    {
      gdouble xres;
      gdouble yres;
      gdouble factor;

      picman_image_get_resolution (image, &xres, &yres);

      factor = (MAX (xres, yres) /
                MIN (xres, yres));

      if (xres == MIN (xres, yres))
        radius_y *= factor;
      else
        radius_x *= factor;
    }

  picman_channel_border (picman_image_get_mask (image), radius_x, radius_y,
                       select_border_feather, select_border_edge_lock, TRUE);
  picman_image_flush (image);
}

static void
select_grow_callback (GtkWidget *widget,
                      gdouble    size,
                      PicmanUnit   unit,
                      gpointer   data)
{
  PicmanImage *image = PICMAN_IMAGE (data);
  gdouble    radius_x;
  gdouble    radius_y;

  radius_x = radius_y = select_grow_pixels = ROUND (size);

  if (unit != PICMAN_UNIT_PIXEL)
    {
      gdouble xres;
      gdouble yres;
      gdouble factor;

      picman_image_get_resolution (image, &xres, &yres);

      factor = (MAX (xres, yres) /
                MIN (xres, yres));

      if (xres == MIN (xres, yres))
        radius_y *= factor;
      else
        radius_x *= factor;
    }

  picman_channel_grow (picman_image_get_mask (image), radius_x, radius_y, TRUE);
  picman_image_flush (image);
}

static void
select_shrink_callback (GtkWidget *widget,
                        gdouble    size,
                        PicmanUnit   unit,
                        gpointer   data)
{
  PicmanImage *image  = PICMAN_IMAGE (data);
  GtkWidget *button = g_object_get_data (G_OBJECT (widget), "edge-lock-toggle");
  gint       radius_x;
  gint       radius_y;

  radius_x = radius_y = select_shrink_pixels = ROUND (size);

  select_shrink_edge_lock =
    ! gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

  if (unit != PICMAN_UNIT_PIXEL)
    {
      gdouble xres;
      gdouble yres;
      gdouble factor;

      picman_image_get_resolution (image, &xres, &yres);

      factor = (MAX (xres, yres) /
                MIN (xres, yres));

      if (xres == MIN (xres, yres))
        radius_y *= factor;
      else
        radius_x *= factor;
    }

  picman_channel_shrink (picman_image_get_mask (image), radius_x, radius_y,
                       select_shrink_edge_lock, TRUE);
  picman_image_flush (image);
}
