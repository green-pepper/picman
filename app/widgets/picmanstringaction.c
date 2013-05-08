/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanstringaction.c
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

#include "picmanstringaction.h"


enum
{
  SELECTED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_VALUE
};


static void   picman_string_action_finalize     (GObject      *object);
static void   picman_string_action_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec);
static void   picman_string_action_get_property (GObject      *object,
                                               guint         prop_id,
                                               GValue       *value,
                                               GParamSpec   *pspec);

static void   picman_string_action_activate     (GtkAction    *action);


G_DEFINE_TYPE (PicmanStringAction, picman_string_action, PICMAN_TYPE_ACTION)

#define parent_class picman_string_action_parent_class

static guint action_signals[LAST_SIGNAL] = { 0 };


static void
picman_string_action_class_init (PicmanStringActionClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkActionClass *action_class = GTK_ACTION_CLASS (klass);

  object_class->finalize     = picman_string_action_finalize;
  object_class->set_property = picman_string_action_set_property;
  object_class->get_property = picman_string_action_get_property;

  action_class->activate = picman_string_action_activate;

  g_object_class_install_property (object_class, PROP_VALUE,
                                   g_param_spec_string ("value",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));

  action_signals[SELECTED] =
    g_signal_new ("selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanStringActionClass, selected),
                  NULL, NULL,
                  picman_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

static void
picman_string_action_init (PicmanStringAction *action)
{
  action->value = NULL;
}

static void
picman_string_action_finalize (GObject *object)
{
  PicmanStringAction *action = PICMAN_STRING_ACTION (object);

  if (action->value)
    {
      g_free (action->value);
      action->value = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_string_action_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanStringAction *action = PICMAN_STRING_ACTION (object);

  switch (prop_id)
    {
    case PROP_VALUE:
      g_value_set_string (value, action->value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_string_action_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanStringAction *action = PICMAN_STRING_ACTION (object);

  switch (prop_id)
    {
    case PROP_VALUE:
      g_free (action->value);
      action->value = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

PicmanStringAction *
picman_string_action_new (const gchar *name,
                        const gchar *label,
                        const gchar *tooltip,
                        const gchar *stock_id,
                        const gchar *value)
{
  PicmanStringAction *action;

  action = g_object_new (PICMAN_TYPE_STRING_ACTION,
                         "name",     name,
                         "label",    label,
                         "tooltip",  tooltip,
                         "stock-id", stock_id,
                         "value",    value,
                         NULL);

  if (stock_id)
    {
      if (gtk_icon_theme_has_icon (gtk_icon_theme_get_default (), stock_id))
        gtk_action_set_icon_name (GTK_ACTION (action), stock_id);
    }

  return action;
}

static void
picman_string_action_activate (GtkAction *action)
{
  PicmanStringAction *string_action = PICMAN_STRING_ACTION (action);

  picman_string_action_selected (string_action, string_action->value);
}

void
picman_string_action_selected (PicmanStringAction *action,
                             const gchar      *value)
{
  g_return_if_fail (PICMAN_IS_STRING_ACTION (action));

  g_signal_emit (action, action_signals[SELECTED], 0, value);
}
