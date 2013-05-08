/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanThrobber
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "picmanthrobber.h"


enum
{
  CLICKED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_STOCK_ID,
  PROP_IMAGE
};


static void      picman_throbber_class_init      (PicmanThrobberClass *klass);

static void      picman_throbber_init                 (PicmanThrobber *button);

static void      picman_throbber_set_property         (GObject      *object,
                                                     guint         prop_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec);
static void      picman_throbber_get_property         (GObject      *object,
                                                     guint         prop_id,
                                                     GValue       *value,
                                                     GParamSpec   *pspec);
static void      picman_throbber_finalize             (GObject      *object);

static gboolean  picman_throbber_create_menu_proxy    (GtkToolItem  *tool_item);
static void      picman_throbber_toolbar_reconfigured (GtkToolItem  *tool_item);
static void      picman_throbber_button_clicked       (GtkWidget    *widget,
                                                     PicmanThrobber *button);

static void      picman_throbber_construct_contents   (GtkToolItem  *tool_item);


static GObjectClass *parent_class                    = NULL;
static guint         toolbutton_signals[LAST_SIGNAL] = { 0 };


#define PICMAN_THROBBER_GET_PRIVATE(obj)(G_TYPE_INSTANCE_GET_PRIVATE ((obj), PICMAN_TYPE_THROBBER, PicmanThrobberPrivate))


struct _PicmanThrobberPrivate
{
  GtkWidget *button;
  GtkWidget *image;
  gchar     *stock_id;
};


GType
picman_throbber_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo type_info =
        {
          sizeof (PicmanThrobberClass),
          (GBaseInitFunc) NULL,
          (GBaseFinalizeFunc) NULL,
          (GClassInitFunc) picman_throbber_class_init,
          (GClassFinalizeFunc) NULL,
          NULL,
          sizeof (PicmanThrobber),
          0, /* n_preallocs */
          (GInstanceInitFunc) picman_throbber_init,
        };

      type = g_type_register_static (GTK_TYPE_TOOL_ITEM,
                                     "PicmanThrobber",
                                     &type_info, 0);
    }

  return type;
}

static void
picman_throbber_class_init (PicmanThrobberClass *klass)
{
  GObjectClass     *object_class    = G_OBJECT_CLASS (klass);
  GtkToolItemClass *tool_item_class = GTK_TOOL_ITEM_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = picman_throbber_set_property;
  object_class->get_property = picman_throbber_get_property;
  object_class->finalize     = picman_throbber_finalize;

  tool_item_class->create_menu_proxy    = picman_throbber_create_menu_proxy;
  tool_item_class->toolbar_reconfigured = picman_throbber_toolbar_reconfigured;

  g_object_class_install_property (object_class,
                                   PROP_STOCK_ID,
                                   g_param_spec_string ("stock-id", NULL, NULL,
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                                   PROP_IMAGE,
                                   g_param_spec_object ("image", NULL, NULL,
                                                        GTK_TYPE_IMAGE,
                                                        G_PARAM_READWRITE));

  toolbutton_signals[CLICKED] =
    g_signal_new ("clicked",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanThrobberClass, clicked),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_type_class_add_private (object_class, sizeof (PicmanThrobberPrivate));
}

static void
picman_throbber_init (PicmanThrobber *button)
{
  GtkToolItem *toolitem = GTK_TOOL_ITEM (button);

  button->priv = PICMAN_THROBBER_GET_PRIVATE (button);

  gtk_tool_item_set_homogeneous (toolitem, TRUE);

  button->priv->button = g_object_new (GTK_TYPE_BUTTON,
                                       "yalign",         0.0,
                                       "focus-on-click", FALSE,
                                       NULL);

  g_signal_connect_object (button->priv->button, "clicked",
                           G_CALLBACK (picman_throbber_button_clicked),
                           button, 0);

  gtk_container_add (GTK_CONTAINER (button), button->priv->button);
  gtk_widget_show (button->priv->button);
}

static void
picman_throbber_construct_contents (GtkToolItem *tool_item)
{
  PicmanThrobber    *button = PICMAN_THROBBER (tool_item);
  GtkWidget       *image;
  GtkToolbarStyle  style;

  if (button->priv->image && gtk_widget_get_parent (button->priv->image))
    gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (button->priv->image)),
                          button->priv->image);

  if (gtk_bin_get_child (GTK_BIN (button->priv->button)))
    gtk_widget_destroy (gtk_bin_get_child (GTK_BIN (button->priv->button)));

  style = gtk_tool_item_get_toolbar_style (tool_item);

  if (style == GTK_TOOLBAR_TEXT)
    {
      image = gtk_image_new_from_stock (button->priv->stock_id,
                                        GTK_ICON_SIZE_MENU);
    }
  else if (style == GTK_TOOLBAR_ICONS)
    {
      image = gtk_image_new_from_stock (button->priv->stock_id,
                                        GTK_ICON_SIZE_LARGE_TOOLBAR);
    }
  else if (button->priv->image)
    {
      image = button->priv->image;
    }
  else
    {
      image = gtk_image_new_from_stock (button->priv->stock_id,
                                        GTK_ICON_SIZE_DND);
    }

  gtk_container_add (GTK_CONTAINER (button->priv->button), image);
  gtk_widget_show (image);

  gtk_button_set_relief (GTK_BUTTON (button->priv->button),
                         gtk_tool_item_get_relief_style (tool_item));

  gtk_widget_queue_resize (GTK_WIDGET (button));
}

static void
picman_throbber_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PicmanThrobber *button = PICMAN_THROBBER (object);

  switch (prop_id)
    {
    case PROP_STOCK_ID:
      picman_throbber_set_stock_id (button, g_value_get_string (value));
      break;

    case PROP_IMAGE:
      picman_throbber_set_image (button, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_throbber_get_property (GObject         *object,
                              guint            prop_id,
                              GValue          *value,
                              GParamSpec      *pspec)
{
  PicmanThrobber *button = PICMAN_THROBBER (object);

  switch (prop_id)
    {
    case PROP_STOCK_ID:
      g_value_set_string (value, button->priv->stock_id);
      break;

    case PROP_IMAGE:
      g_value_set_object (value, button->priv->image);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_throbber_finalize (GObject *object)
{
  PicmanThrobber *button = PICMAN_THROBBER (object);

  if (button->priv->stock_id)
    g_free (button->priv->stock_id);

  if (button->priv->image)
    g_object_unref (button->priv->image);

  parent_class->finalize (object);
}

static void
picman_throbber_button_clicked (GtkWidget    *widget,
                              PicmanThrobber *button)
{
  g_signal_emit_by_name (button, "clicked");
}

static gboolean
picman_throbber_create_menu_proxy (GtkToolItem *tool_item)
{
  gtk_tool_item_set_proxy_menu_item (tool_item, "picman-throbber-menu-id", NULL);

  return FALSE;
}

static void
picman_throbber_toolbar_reconfigured (GtkToolItem *tool_item)
{
  picman_throbber_construct_contents (tool_item);
}

GtkToolItem *
picman_throbber_new (const gchar *stock_id)
{
  return g_object_new (PICMAN_TYPE_THROBBER,
                       "stock-id", stock_id,
                       NULL);
}

void
picman_throbber_set_stock_id (PicmanThrobber *button,
                            const gchar  *stock_id)
{
  gchar *old_stock_id;

  g_return_if_fail (PICMAN_IS_THROBBER (button));

  old_stock_id = button->priv->stock_id;

  button->priv->stock_id = g_strdup (stock_id);
  picman_throbber_construct_contents (GTK_TOOL_ITEM (button));

  g_object_notify (G_OBJECT (button), "stock-id");

  g_free (old_stock_id);
}

const gchar *
picman_throbber_get_stock_id (PicmanThrobber *button)
{
  g_return_val_if_fail (PICMAN_IS_THROBBER (button), NULL);

  return button->priv->stock_id;
}

void
picman_throbber_set_image (PicmanThrobber *button,
                         GtkWidget    *image)
{
  g_return_if_fail (PICMAN_IS_THROBBER (button));
  g_return_if_fail (image == NULL || GTK_IS_IMAGE (image));

  if (image != button->priv->image)
    {
      if (button->priv->image)
	{
	  if (gtk_widget_get_parent (button->priv->image))
            gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (button->priv->image)),
                                  button->priv->image);

	  g_object_unref (button->priv->image);
	}

      if (image)
        g_object_ref_sink (image);

      button->priv->image = image;

      picman_throbber_construct_contents (GTK_TOOL_ITEM (button));

      g_object_notify (G_OBJECT (button), "image");
    }
}

GtkWidget *
picman_throbber_get_image (PicmanThrobber *button)
{
  g_return_val_if_fail (PICMAN_IS_THROBBER (button), NULL);

  return button->priv->image;
}
