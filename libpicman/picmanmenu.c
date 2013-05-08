/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmenu.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "picman.h"

#include "picmanpixbuf.h"

#undef PICMAN_DISABLE_DEPRECATED
#include "picmanmenu.h"

#include "libpicman-intl.h"


/**
 * SECTION: picmanmenu
 * @title: picmanmenu
 * @short_description: Menus for selecting images, layers, channels
 *                     and drawables.
 *
 * Menus for selecting images, layers, channels and drawables.
 **/


#define MENU_THUMBNAIL_WIDTH   24
#define MENU_THUMBNAIL_HEIGHT  24


/*  local function prototypes  */

static GtkWidget * picman_menu_make_menu     (PicmanMenuCallback    callback,
                                            gpointer            data);
static GtkWidget * picman_menu_add_item      (GtkWidget          *menu,
                                            const gchar        *image_name,
                                            const gchar        *drawable_name,
                                            gint32              any_ID);
static GtkWidget * picman_menu_add_empty     (GtkWidget          *menu);
static GtkWidget * picman_menu_make_preview  (gint32              any_ID,
                                            gboolean            is_image,
                                            gint                width,
                                            gint                height);
static void        picman_menu_callback      (GtkWidget          *widget,
                                            gpointer            any_ID);


/*  public functions  */

/**
 * picman_image_menu_new:
 * @constraint:   a function to filter the menu contents
 * @callback:     the callback to call when an image is selected
 * @data:         the callback's user_data
 * @active_image: an image to preselect
 *
 * Deprecated: Use picman_image_combo_box_new() instead.
 *
 * Returns: the image menu.
 */
GtkWidget *
picman_image_menu_new (PicmanConstraintFunc constraint,
                     PicmanMenuCallback   callback,
                     gpointer           data,
                     gint32             active_image)
{
  GtkWidget *menu;
  gchar     *name;
  gchar     *label;
  gint32    *images;
  gint32     image = -1;
  gint       n_images;
  gint       i, k;

  g_return_val_if_fail (callback != NULL, NULL);

  menu = picman_menu_make_menu (callback, data);

  images = picman_image_list (&n_images);

  for (i = 0, k = 0; i < n_images; i++)
    if (! constraint || (* constraint) (images[i], -1, data))
      {
        name = picman_image_get_name (images[i]);
        label = g_strdup_printf ("%s-%d", name, images[i]);
        g_free (name);

        picman_menu_add_item (menu, label, NULL, images[i]);

        g_free (label);

        if (images[i] == active_image)
          {
            image = active_image;
            gtk_menu_set_active (GTK_MENU (menu), k);
          }
        else if (image == -1)
          {
            image = images[i];
          }

        k += 1;
      }

  if (k == 0)
    picman_menu_add_empty (menu);

  (* callback) (image, data);

  g_free (images);

  return menu;
}

/**
 * picman_layer_menu_new:
 * @constraint:   a function to filter the menu contents
 * @callback:     the callback to call when a channel is selected
 * @data:         the callback's user_data
 * @active_layer: a layer to preselect
 *
 * Deprecated: Use picman_layer_combo_box_new() instead.
 *
 * Returns: the layer menu.
 */
GtkWidget *
picman_layer_menu_new (PicmanConstraintFunc constraint,
                     PicmanMenuCallback   callback,
                     gpointer           data,
                     gint32             active_layer)
{
  GtkWidget *menu;
  gchar     *image_label;
  gint32    *images;
  gint32    *layers;
  gint32     layer = -1;
  gint       n_images;
  gint       n_layers;
  gint       i, j, k;

  g_return_val_if_fail (callback != NULL, NULL);

  menu = picman_menu_make_menu (callback, data);

  images = picman_image_list (&n_images);

  for (i = 0, k = 0; i < n_images; i++)
    if (! constraint || (* constraint) (images[i], -1, data))
      {
        gchar *name;

        name = picman_image_get_name (images[i]);
        image_label = g_strdup_printf ("%s-%d", name, images[i]);
        g_free (name);

        layers = picman_image_get_layers (images[i], &n_layers);

        for (j = 0; j < n_layers; j++)
          if (! constraint || (* constraint) (images[i], layers[j], data))
            {
              name = picman_item_get_name (layers[j]);
              picman_menu_add_item (menu, image_label, name, layers[j]);
              g_free (name);

              if (layers[j] == active_layer)
                {
                  layer = active_layer;
                  gtk_menu_set_active (GTK_MENU (menu), k);
                }
              else if (layer == -1)
                {
                  layer = layers[j];
                }

              k += 1;
            }

        g_free (image_label);
        g_free (layers);
      }

  g_free (images);

  if (k == 0)
    picman_menu_add_empty (menu);

  (* callback) (layer, data);

  return menu;
}

/**
 * picman_channel_menu_new:
 * @constraint:     a function to filter the menu contents
 * @callback:       the callback to call when a channel is selected
 * @data:           the callback's user_data
 * @active_channel: a channel to preselect
 *
 * Deprecated: Use picman_channel_combo_box_new() instead.
 *
 * Returns: the channel menu.
 */
GtkWidget *
picman_channel_menu_new (PicmanConstraintFunc constraint,
                       PicmanMenuCallback   callback,
                       gpointer           data,
                       gint32             active_channel)
{
  GtkWidget *menu;
  gchar     *image_label;
  gint32    *images;
  gint32    *channels;
  gint32     channel;
  gint       n_images;
  gint       n_channels;
  gint       i, j, k;

  g_return_val_if_fail (callback != NULL, NULL);

  menu = picman_menu_make_menu (callback, data);

  channel = -1;

  images = picman_image_list (&n_images);

  for (i = 0, k = 0; i < n_images; i++)
    if (! constraint || (* constraint) (images[i], -1, data))
      {
        gchar *name;

        name = picman_image_get_name (images[i]);
        image_label = g_strdup_printf ("%s-%d", name, images[i]);
        g_free (name);

        channels = picman_image_get_channels (images[i], &n_channels);

        for (j = 0; j < n_channels; j++)
          if (! constraint || (* constraint) (images[i], channels[j], data))
            {
              name = picman_item_get_name (channels[j]);
              picman_menu_add_item (menu, image_label, name, channels[j]);
              g_free (name);

              if (channels[j] == active_channel)
                {
                  channel = active_channel;
                  gtk_menu_set_active (GTK_MENU (menu), k);
                }
              else if (channel == -1)
                {
                  channel = channels[j];
                }

              k += 1;
            }

        g_free (image_label);
        g_free (channels);
      }

  g_free (images);

  if (k == 0)
    picman_menu_add_empty (menu);

  (* callback) (channel, data);

  return menu;
}

/**
 * picman_drawable_menu_new:
 * @constraint:      a function to filter the menu contents
 * @callback:        the callback to call when a channel is selected
 * @data:            the callback's user_data
 * @active_drawable: a drawable to preselect
 *
 * Deprecated: Use picman_drawable_combo_box_new() instead.
 *
 * Returns: the drawable menu.
 */
GtkWidget *
picman_drawable_menu_new (PicmanConstraintFunc constraint,
                        PicmanMenuCallback   callback,
                        gpointer           data,
                        gint32             active_drawable)
{
  GtkWidget *menu;
  gchar     *name;
  gchar     *image_label;
  gint32    *images;
  gint32    *layers;
  gint32    *channels;
  gint32     drawable;
  gint       n_images;
  gint       n_layers;
  gint       n_channels;
  gint       i, j, k;

  menu = picman_menu_make_menu (callback, data);

  drawable = -1;

  images = picman_image_list (&n_images);

  for (i = 0, k = 0; i < n_images; i++)
    if (! constraint || (* constraint) (images[i], -1, data))
      {
        name = picman_image_get_name (images[i]);
        image_label = g_strdup_printf ("%s-%d", name, images[i]);
        g_free (name);

        layers   = picman_image_get_layers   (images[i], &n_layers);
        channels = picman_image_get_channels (images[i], &n_channels);

        for (j = 0; j < n_layers; j++)
          if (! constraint || (* constraint) (images[i], layers[j], data))
            {
              name = picman_item_get_name (layers[j]);
              picman_menu_add_item (menu, image_label, name, layers[j]);
              g_free (name);

              if (layers[j] == active_drawable)
                {
                  drawable = active_drawable;
                  gtk_menu_set_active (GTK_MENU (menu), k);
                }
              else if (drawable == -1)
                {
                  drawable = layers[j];
                }

              k += 1;
            }

        for (j = 0; j < n_channels; j++)
          if (! constraint || (* constraint) (images[i], channels[j], data))
            {
              name = picman_item_get_name (channels[j]);
              picman_menu_add_item (menu, image_label, name, channels[j]);
              g_free (name);

              if (channels[j] == active_drawable)
                {
                  drawable = active_drawable;
                  gtk_menu_set_active (GTK_MENU (menu), k);
                }
              else if (drawable == -1)
                {
                  drawable = channels[j];
                }

              k += 1;
            }

        g_free (image_label);
        g_free (layers);
        g_free (channels);
      }

  g_free (images);

  if (k == 0)
    picman_menu_add_empty (menu);

  (* callback) (drawable, data);

  return menu;
}


/*  private functions  */

static GtkWidget *
picman_menu_make_menu (PicmanMenuCallback callback,
                     gpointer         data)
{
  GtkWidget *menu;

  menu = gtk_menu_new ();
  g_object_set_data (G_OBJECT (menu), "picman-menu-callback",      callback);
  g_object_set_data (G_OBJECT (menu), "picman-menu-callback-data", data);

  return menu;
}

static GtkWidget *
picman_menu_add_item (GtkWidget   *menu,
                    const gchar *image_name,
                    const gchar *drawable_name,
                    gint32       any_ID)
{
  GtkWidget *menuitem;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *preview;
  GtkWidget *label;
  gchar     *str;

  if (drawable_name)
    str = g_strdup_printf ("%s/%s-%d", image_name, drawable_name, any_ID);
  else
    str = g_strdup (image_name);

  menuitem = gtk_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
  gtk_widget_show (menuitem);

  g_signal_connect (menuitem, "activate",
                    G_CALLBACK (picman_menu_callback),
                    GINT_TO_POINTER (any_ID));

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_add (GTK_CONTAINER (menuitem), hbox);
  gtk_widget_show (hbox);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  preview = picman_menu_make_preview (any_ID, drawable_name == NULL,
                                    MENU_THUMBNAIL_WIDTH,
                                    MENU_THUMBNAIL_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  label = gtk_label_new (str);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  g_free (str);

  return menuitem;
}

static GtkWidget *
picman_menu_add_empty (GtkWidget *menu)
{
  GtkWidget *menuitem;

  menuitem = gtk_menu_item_new_with_label (_("(Empty)"));
  gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
  gtk_widget_show (menuitem);

  return menuitem;
}

static GtkWidget *
picman_menu_make_preview (gint32     any_ID,
                        gboolean   is_image,
                        gint       width,
                        gint       height)
{
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  if (is_image)
    pixbuf = picman_image_get_thumbnail (any_ID,
                                       width, height,
                                       PICMAN_PIXBUF_SMALL_CHECKS);
  else
    pixbuf = picman_drawable_get_thumbnail (any_ID,
                                          width, height,
                                          PICMAN_PIXBUF_SMALL_CHECKS);

  image = gtk_image_new_from_pixbuf (pixbuf);

  g_object_unref (pixbuf);

  return image;
}

static void
picman_menu_callback (GtkWidget *widget,
                    gpointer   any_ID)
{
  GtkWidget        *parent = gtk_widget_get_parent (widget);
  PicmanMenuCallback  callback;
  gpointer          callback_data;

  callback = (PicmanMenuCallback) g_object_get_data (G_OBJECT (parent),
                                                   "picman-menu-callback");
  callback_data = g_object_get_data (G_OBJECT (parent),
                                     "picman-menu-callback-data");

  (* callback) (GPOINTER_TO_INT (any_ID), callback_data);
}
