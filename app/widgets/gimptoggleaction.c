/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantoggleaction.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
 * Copyright (C) 2008 Sven Neumann <sven@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "picmantoggleaction.h"


static void   picman_toggle_action_connect_proxy     (GtkAction        *action,
                                                    GtkWidget        *proxy);
static void   picman_toggle_action_set_proxy_tooltip (PicmanToggleAction *action,
                                                    GtkWidget        *proxy);
static void   picman_toggle_action_tooltip_notify    (PicmanToggleAction *action,
                                                    const GParamSpec *pspec,
                                                    gpointer          data);


G_DEFINE_TYPE (PicmanToggleAction, picman_toggle_action, GTK_TYPE_TOGGLE_ACTION)

#define parent_class picman_toggle_action_parent_class


static void
picman_toggle_action_class_init (PicmanToggleActionClass *klass)
{
  GtkActionClass *action_class = GTK_ACTION_CLASS (klass);

  action_class->connect_proxy = picman_toggle_action_connect_proxy;
}

static void
picman_toggle_action_init (PicmanToggleAction *action)
{
  g_signal_connect (action, "notify::tooltip",
                    G_CALLBACK (picman_toggle_action_tooltip_notify),
                    NULL);
}

static void
picman_toggle_action_connect_proxy (GtkAction *action,
                                  GtkWidget *proxy)
{
  GTK_ACTION_CLASS (parent_class)->connect_proxy (action, proxy);

  picman_toggle_action_set_proxy_tooltip (PICMAN_TOGGLE_ACTION (action), proxy);
}


/*  public functions  */

GtkToggleAction *
picman_toggle_action_new (const gchar *name,
                        const gchar *label,
                        const gchar *tooltip,
                        const gchar *stock_id)
{
  GtkToggleAction *action;

  action = g_object_new (PICMAN_TYPE_TOGGLE_ACTION,
                         "name",    name,
                         "label",   label,
                         "tooltip", tooltip,
                         NULL);

  if (stock_id)
    {
      if (gtk_icon_factory_lookup_default (stock_id))
        gtk_action_set_stock_id (GTK_ACTION (action), stock_id);
      else
        gtk_action_set_icon_name (GTK_ACTION (action), stock_id);
    }

  return action;
}


/*  private functions  */


static void
picman_toggle_action_set_proxy_tooltip (PicmanToggleAction *action,
                                      GtkWidget        *proxy)
{
  const gchar *tooltip = gtk_action_get_tooltip (GTK_ACTION (action));

  if (tooltip)
    picman_help_set_help_data (proxy, tooltip,
                             g_object_get_qdata (G_OBJECT (proxy),
                                                 PICMAN_HELP_ID));
}

static void
picman_toggle_action_tooltip_notify (PicmanToggleAction *action,
                                   const GParamSpec *pspec,
                                   gpointer          data)
{
  GSList *list;

  for (list = gtk_action_get_proxies (GTK_ACTION (action));
       list;
       list = g_slist_next (list))
    {
      picman_toggle_action_set_proxy_tooltip (action, list->data);
    }
}
