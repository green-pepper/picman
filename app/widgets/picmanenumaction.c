/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanenumaction.c
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

#include "picmanenumaction.h"


enum
{
  SELECTED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_VALUE,
  PROP_VALUE_VARIABLE
};


static void   picman_enum_action_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void   picman_enum_action_get_property (GObject      *object,
                                             guint         prop_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);

static void   picman_enum_action_activate     (GtkAction    *action);


G_DEFINE_TYPE (PicmanEnumAction, picman_enum_action, PICMAN_TYPE_ACTION)

#define parent_class picman_enum_action_parent_class

static guint action_signals[LAST_SIGNAL] = { 0 };


static void
picman_enum_action_class_init (PicmanEnumActionClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkActionClass *action_class = GTK_ACTION_CLASS (klass);

  object_class->set_property = picman_enum_action_set_property;
  object_class->get_property = picman_enum_action_get_property;

  action_class->activate     = picman_enum_action_activate;

  g_object_class_install_property (object_class, PROP_VALUE,
                                   g_param_spec_int ("value",
                                                     NULL, NULL,
                                                     G_MININT, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_VALUE_VARIABLE,
                                   g_param_spec_boolean ("value-variable",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  action_signals[SELECTED] =
    g_signal_new ("selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanEnumActionClass, selected),
                  NULL, NULL,
                  picman_marshal_VOID__INT,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);
}

static void
picman_enum_action_init (PicmanEnumAction *action)
{
  action->value          = 0;
  action->value_variable = FALSE;
}

static void
picman_enum_action_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanEnumAction *action = PICMAN_ENUM_ACTION (object);

  switch (prop_id)
    {
    case PROP_VALUE:
      g_value_set_int (value, action->value);
      break;
    case PROP_VALUE_VARIABLE:
      g_value_set_boolean (value, action->value_variable);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_enum_action_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanEnumAction *action = PICMAN_ENUM_ACTION (object);

  switch (prop_id)
    {
    case PROP_VALUE:
      action->value = g_value_get_int (value);
      break;
    case PROP_VALUE_VARIABLE:
      action->value_variable = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

PicmanEnumAction *
picman_enum_action_new (const gchar *name,
                      const gchar *label,
                      const gchar *tooltip,
                      const gchar *stock_id,
                      gint         value,
                      gboolean     value_variable)
{
  return g_object_new (PICMAN_TYPE_ENUM_ACTION,
                       "name",           name,
                       "label",          label,
                       "tooltip",        tooltip,
                       "stock-id",       stock_id,
                       "value",          value,
                       "value-variable", value_variable,
                       NULL);
}

static void
picman_enum_action_activate (GtkAction *action)
{
  PicmanEnumAction *enum_action = PICMAN_ENUM_ACTION (action);

  picman_enum_action_selected (enum_action, enum_action->value);
}

void
picman_enum_action_selected (PicmanEnumAction *action,
                           gint            value)
{
  g_return_if_fail (PICMAN_IS_ENUM_ACTION (action));

  g_signal_emit (action, action_signals[SELECTED], 0, value);
}
