/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanaction.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanimagefile.h"  /* eek */
#include "core/picmanviewable.h"

#include "picmanaction.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"


enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_COLOR,
  PROP_VIEWABLE,
  PROP_ELLIPSIZE,
  PROP_MAX_WIDTH_CHARS
};


static void   picman_action_finalize          (GObject          *object);
static void   picman_action_set_property      (GObject          *object,
                                             guint             prop_id,
                                             const GValue     *value,
                                             GParamSpec       *pspec);
static void   picman_action_get_property      (GObject          *object,
                                             guint             prop_id,
                                             GValue           *value,
                                             GParamSpec       *pspec);

static void   picman_action_connect_proxy     (GtkAction        *action,
                                             GtkWidget        *proxy);
static void   picman_action_set_proxy         (PicmanAction       *action,
                                             GtkWidget        *proxy);
static void   picman_action_set_proxy_tooltip (PicmanAction       *action,
                                             GtkWidget        *proxy);
static void   picman_action_tooltip_notify    (PicmanAction       *action,
                                             const GParamSpec *pspec,
                                             gpointer          data);


G_DEFINE_TYPE (PicmanAction, picman_action, GTK_TYPE_ACTION)

#define parent_class picman_action_parent_class


static void
picman_action_class_init (PicmanActionClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkActionClass *action_class = GTK_ACTION_CLASS (klass);
  PicmanRGB         black;

  object_class->finalize      = picman_action_finalize;
  object_class->set_property  = picman_action_set_property;
  object_class->get_property  = picman_action_get_property;

  action_class->connect_proxy = picman_action_connect_proxy;

  picman_rgba_set (&black, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_COLOR,
                                   picman_param_spec_rgb ("color",
                                                        NULL, NULL,
                                                        TRUE, &black,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_VIEWABLE,
                                   g_param_spec_object ("viewable",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_VIEWABLE,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_ELLIPSIZE,
                                   g_param_spec_enum ("ellipsize",
                                                      NULL, NULL,
                                                      PANGO_TYPE_ELLIPSIZE_MODE,
                                                      PANGO_ELLIPSIZE_NONE,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_MAX_WIDTH_CHARS,
                                   g_param_spec_int ("max-width-chars",
                                                     NULL, NULL,
                                                     -1, G_MAXINT, -1,
                                                     PICMAN_PARAM_READWRITE));
}

static void
picman_action_init (PicmanAction *action)
{
  action->color           = NULL;
  action->viewable        = NULL;
  action->ellipsize       = PANGO_ELLIPSIZE_NONE;
  action->max_width_chars = -1;

  g_signal_connect (action, "notify::tooltip",
                    G_CALLBACK (picman_action_tooltip_notify),
                    NULL);
}

static void
picman_action_finalize (GObject *object)
{
  PicmanAction *action = PICMAN_ACTION (object);

  if (action->context)
    {
      g_object_unref (action->context);
      action->context = NULL;
    }

  if (action->color)
    {
      g_free (action->color);
      action->color = NULL;
    }

  if (action->viewable)
    {
      g_object_unref (action->viewable);
      action->viewable = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_action_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PicmanAction *action = PICMAN_ACTION (object);

  switch (prop_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, action->context);
      break;

    case PROP_COLOR:
      g_value_set_boxed (value, action->color);
      break;

    case PROP_VIEWABLE:
      g_value_set_object (value, action->viewable);
      break;

    case PROP_ELLIPSIZE:
      g_value_set_enum (value, action->ellipsize);
      break;

    case PROP_MAX_WIDTH_CHARS:
      g_value_set_int (value, action->max_width_chars);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_action_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PicmanAction *action    = PICMAN_ACTION (object);
  gboolean    set_proxy = FALSE;

  switch (prop_id)
    {
    case PROP_CONTEXT:
      if (action->context)
        g_object_unref  (action->context);
      action->context = g_value_dup_object (value);
      break;

    case PROP_COLOR:
      if (action->color)
        g_free (action->color);
      action->color = g_value_dup_boxed (value);
      set_proxy = TRUE;
      break;

    case PROP_VIEWABLE:
      if (action->viewable)
        g_object_unref  (action->viewable);
      action->viewable = g_value_dup_object (value);
      set_proxy = TRUE;
      break;

    case PROP_ELLIPSIZE:
      action->ellipsize = g_value_get_enum (value);
      set_proxy = TRUE;
      break;

    case PROP_MAX_WIDTH_CHARS:
      action->max_width_chars = g_value_get_int (value);
      set_proxy = TRUE;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }

  if (set_proxy)
    {
      GSList *list;

      for (list = gtk_action_get_proxies (GTK_ACTION (action));
           list;
           list = g_slist_next (list))
        {
          picman_action_set_proxy (action, list->data);
        }
    }
}

static void
picman_action_connect_proxy (GtkAction *action,
                           GtkWidget *proxy)
{
  GTK_ACTION_CLASS (parent_class)->connect_proxy (action, proxy);

  picman_action_set_proxy (PICMAN_ACTION (action), proxy);
  picman_action_set_proxy_tooltip (PICMAN_ACTION (action), proxy);
}


/*  public functions  */

PicmanAction *
picman_action_new (const gchar *name,
                 const gchar *label,
                 const gchar *tooltip,
                 const gchar *stock_id)
{
  PicmanAction *action;

  action = g_object_new (PICMAN_TYPE_ACTION,
                         "name",     name,
                         "label",    label,
                         "tooltip",  tooltip,
                         "stock-id", stock_id,
                         NULL);

  if (stock_id)
    {
      if (gtk_icon_theme_has_icon (gtk_icon_theme_get_default (), stock_id))
        gtk_action_set_icon_name (GTK_ACTION (action), stock_id);
    }

  return action;
}

gint
picman_action_name_compare (PicmanAction  *action1,
                          PicmanAction  *action2)
{
  return strcmp (gtk_action_get_name ((GtkAction *) action1),
                 gtk_action_get_name ((GtkAction *) action2));
}


/*  private functions  */

static void
picman_action_set_proxy (PicmanAction *action,
                       GtkWidget  *proxy)
{
  if (! GTK_IS_IMAGE_MENU_ITEM (proxy))
    return;

  if (action->color)
    {
      GtkWidget *area;

      area = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (proxy));

      if (PICMAN_IS_COLOR_AREA (area))
        {
          picman_color_area_set_color (PICMAN_COLOR_AREA (area), action->color);
        }
      else
        {
          gint width, height;

          area = picman_color_area_new (action->color,
                                      PICMAN_COLOR_AREA_SMALL_CHECKS, 0);
          picman_color_area_set_draw_border (PICMAN_COLOR_AREA (area), TRUE);

          gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (proxy),
                                             GTK_ICON_SIZE_MENU,
                                             &width, &height);

          gtk_widget_set_size_request (area, width, height);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (proxy), area);
          gtk_widget_show (area);
        }
    }
  else if (action->viewable)
    {
      GtkWidget *view;

      view = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (proxy));

      if (PICMAN_IS_VIEW (view) &&
          g_type_is_a (G_TYPE_FROM_INSTANCE (action->viewable),
                       PICMAN_VIEW (view)->renderer->viewable_type))
        {
          picman_view_set_viewable (PICMAN_VIEW (view), action->viewable);
        }
      else
        {
          GtkIconSize size;
          gint        width, height;
          gint        border_width;

          if (PICMAN_IS_IMAGEFILE (action->viewable))
            {
              size         = GTK_ICON_SIZE_LARGE_TOOLBAR;
              border_width = 0;
            }
          else
            {
              size         = GTK_ICON_SIZE_MENU;
              border_width = 1;
            }

          gtk_icon_size_lookup_for_settings (gtk_widget_get_settings (proxy),
                                             size, &width, &height);

          view = picman_view_new_full (action->context, action->viewable,
                                     width, height, border_width,
                                     FALSE, FALSE, FALSE);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (proxy), view);
          gtk_widget_show (view);
        }
    }
  else
    {
      GtkWidget *image;

      image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (proxy));

      if (PICMAN_IS_VIEW (image) || PICMAN_IS_COLOR_AREA (image))
        {
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (proxy), NULL);
          g_object_notify (G_OBJECT (action), "stock-id");
        }
    }

  {
    GtkWidget *child = gtk_bin_get_child (GTK_BIN (proxy));

    if (GTK_IS_LABEL (child))
      {
        GtkLabel *label = GTK_LABEL (child);

        gtk_label_set_ellipsize (label, action->ellipsize);
        gtk_label_set_max_width_chars (label, action->max_width_chars);
      }
  }
}

static void
picman_action_set_proxy_tooltip (PicmanAction *action,
                               GtkWidget  *proxy)
{
  const gchar *tooltip = gtk_action_get_tooltip (GTK_ACTION (action));

  if (tooltip)
    picman_help_set_help_data (proxy, tooltip,
                             g_object_get_qdata (G_OBJECT (proxy),
                                                 PICMAN_HELP_ID));
}

static void
picman_action_tooltip_notify (PicmanAction       *action,
                            const GParamSpec *pspec,
                            gpointer          data)
{
  GSList *list;

  for (list = gtk_action_get_proxies (GTK_ACTION (action));
       list;
       list = g_slist_next (list))
    {
      picman_action_set_proxy_tooltip (action, list->data);
    }
}
