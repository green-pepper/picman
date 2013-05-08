/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginaction.c
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

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picmanmarshal.h"

#include "plug-in/picmanpluginprocedure.h"

#include "picmanpluginaction.h"


enum
{
  SELECTED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_PROCEDURE
};


static void   picman_plug_in_action_finalize      (GObject      *object);
static void   picman_plug_in_action_set_property  (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);
static void   picman_plug_in_action_get_property  (GObject      *object,
                                                 guint         prop_id,
                                                 GValue       *value,
                                                 GParamSpec   *pspec);

static void   picman_plug_in_action_activate      (GtkAction    *action);
static void   picman_plug_in_action_connect_proxy (GtkAction    *action,
                                                 GtkWidget    *proxy);


G_DEFINE_TYPE (PicmanPlugInAction, picman_plug_in_action, PICMAN_TYPE_ACTION)

#define parent_class picman_plug_in_action_parent_class

static guint action_signals[LAST_SIGNAL] = { 0 };


static void
picman_plug_in_action_class_init (PicmanPlugInActionClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkActionClass *action_class = GTK_ACTION_CLASS (klass);

  object_class->finalize      = picman_plug_in_action_finalize;
  object_class->set_property  = picman_plug_in_action_set_property;
  object_class->get_property  = picman_plug_in_action_get_property;

  action_class->activate      = picman_plug_in_action_activate;
  action_class->connect_proxy = picman_plug_in_action_connect_proxy;

  g_object_class_install_property (object_class, PROP_PROCEDURE,
                                   g_param_spec_object ("procedure",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PLUG_IN_PROCEDURE,
                                                        PICMAN_PARAM_READWRITE));

  action_signals[SELECTED] =
    g_signal_new ("selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanPlugInActionClass, selected),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PLUG_IN_PROCEDURE);
}

static void
picman_plug_in_action_init (PicmanPlugInAction *action)
{
  action->procedure = NULL;
}

static void
picman_plug_in_action_finalize (GObject *object)
{
  PicmanPlugInAction *action = PICMAN_PLUG_IN_ACTION (object);

  if (action->procedure)
    {
      g_object_unref (action->procedure);
      action->procedure = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_plug_in_action_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanPlugInAction *action = PICMAN_PLUG_IN_ACTION (object);

  switch (prop_id)
    {
    case PROP_PROCEDURE:
      g_value_set_object (value, action->procedure);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_plug_in_action_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanPlugInAction *action = PICMAN_PLUG_IN_ACTION (object);

  switch (prop_id)
    {
    case PROP_PROCEDURE:
      if (action->procedure)
        g_object_unref (action->procedure);
      action->procedure = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_plug_in_action_activate (GtkAction *action)
{
  PicmanPlugInAction *plug_in_action = PICMAN_PLUG_IN_ACTION (action);

  /* Not all actions have procedures associated with them, for example
   * unused "plug-in-recent-[N]" actions, so check for NULL before we
   * invoke the plug-in action
   */
  if (plug_in_action->procedure)
    picman_plug_in_action_selected (plug_in_action, plug_in_action->procedure);
}

static void
picman_plug_in_action_connect_proxy (GtkAction *action,
                                   GtkWidget *proxy)
{
  PicmanPlugInAction *plug_in_action = PICMAN_PLUG_IN_ACTION (action);

  GTK_ACTION_CLASS (parent_class)->connect_proxy (action, proxy);

  if (GTK_IS_IMAGE_MENU_ITEM (proxy) && plug_in_action->procedure)
    {
      GdkPixbuf *pixbuf;

      pixbuf = picman_plug_in_procedure_get_pixbuf (plug_in_action->procedure);

      if (pixbuf)
        {
          GtkSettings *settings = gtk_widget_get_settings (proxy);
          gint         width;
          gint         height;
          GtkWidget   *image;

          gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_MENU,
                                             &width, &height);

          if (width  != gdk_pixbuf_get_width  (pixbuf) ||
              height != gdk_pixbuf_get_height (pixbuf))
            {
              GdkPixbuf *copy;

              copy = gdk_pixbuf_scale_simple (pixbuf, width, height,
                                              GDK_INTERP_BILINEAR);
              g_object_unref (pixbuf);
              pixbuf = copy;
            }

          image = gtk_image_new_from_pixbuf (pixbuf);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (proxy), image);
          g_object_unref (pixbuf);
        }
    }
}


/*  public functions  */

PicmanPlugInAction *
picman_plug_in_action_new (const gchar         *name,
                         const gchar         *label,
                         const gchar         *tooltip,
                         const gchar         *stock_id,
                         PicmanPlugInProcedure *procedure)
{
  return g_object_new (PICMAN_TYPE_PLUG_IN_ACTION,
                       "name",      name,
                       "label",     label,
                       "tooltip",   tooltip,
                       "stock-id",  stock_id,
                       "procedure", procedure,
                       NULL);
}

void
picman_plug_in_action_selected (PicmanPlugInAction    *action,
                              PicmanPlugInProcedure *procedure)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_ACTION (action));

  g_signal_emit (action, action_signals[SELECTED], 0, procedure);
}
