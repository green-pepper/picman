/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenumlabel.c
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanwidgetstypes.h"

#include "picmanenumlabel.h"


/**
 * SECTION: picmanenumlabel
 * @title: PicmanEnumLabel
 * @short_description: A #GtkLabel subclass that displays an enum value.
 *
 * A #GtkLabel subclass that displays an enum value.
 **/


enum
{
  PROP_0,
  PROP_ENUM_TYPE,
  PROP_ENUM_VALUE
};


static void   picman_enum_label_finalize     (GObject      *object);
static void   picman_enum_label_get_property (GObject      *object,
                                            guint         property_id,
                                            GValue       *value,
                                            GParamSpec   *pspec);
static void   picman_enum_label_set_property (GObject      *object,
                                            guint         property_id,
                                            const GValue *value,
                                            GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanEnumLabel, picman_enum_label, GTK_TYPE_LABEL)

#define parent_class picman_enum_label_parent_class


static void
picman_enum_label_class_init (PicmanEnumLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_enum_label_finalize;
  object_class->get_property = picman_enum_label_get_property;
  object_class->set_property = picman_enum_label_set_property;

  /**
   * PicmanEnumLabel:enum-type:
   *
   * The #GType of the enum.
   *
   * Since: PICMAN 2.8
   **/
  g_object_class_install_property (object_class, PROP_ENUM_TYPE,
                                   g_param_spec_gtype ("enum-type", NULL, NULL,
                                                       G_TYPE_NONE,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  /**
   * PicmanEnumLabel:enum-value:
   *
   * The value to display.
   *
   * Since: PICMAN 2.8
   **/
  g_object_class_install_property (object_class, PROP_ENUM_VALUE,
                                   g_param_spec_int ("enum-value", NULL, NULL,
                                                     G_MININT, G_MAXINT, 0,
                                                     PICMAN_PARAM_WRITABLE |
                                                     G_PARAM_CONSTRUCT));
}

static void
picman_enum_label_init (PicmanEnumLabel *enum_label)
{
}

static void
picman_enum_label_finalize (GObject *object)
{
  PicmanEnumLabel *enum_label = PICMAN_ENUM_LABEL (object);

  if (enum_label->enum_class)
    g_type_class_unref (enum_label->enum_class);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_enum_label_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanEnumLabel *label = PICMAN_ENUM_LABEL (object);

  switch (property_id)
    {
    case PROP_ENUM_TYPE:
      if (label->enum_class)
        g_value_set_gtype (value, G_TYPE_FROM_CLASS (label->enum_class));
      else
        g_value_set_gtype (value, G_TYPE_NONE);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_enum_label_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanEnumLabel *label = PICMAN_ENUM_LABEL (object);

  switch (property_id)
    {
    case PROP_ENUM_TYPE:
      label->enum_class = g_type_class_ref (g_value_get_gtype (value));
      break;

    case PROP_ENUM_VALUE:
      picman_enum_label_set_value (label, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * picman_enum_label_new:
 * @enum_type: the #GType of an enum.
 * @value:
 *
 * Return value: a new #PicmanEnumLabel.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_enum_label_new (GType enum_type,
                     gint  value)
{
  g_return_val_if_fail (G_TYPE_IS_ENUM (enum_type), NULL);

  return g_object_new (PICMAN_TYPE_ENUM_LABEL,
                       "enum-type",  enum_type,
                       "enum-value", value,
                       NULL);
}

/**
 * picman_enum_label_set_value
 * @label: a #PicmanEnumLabel
 * @value:
 *
 * Since: PICMAN 2.4
 **/
void
picman_enum_label_set_value (PicmanEnumLabel *label,
                           gint           value)
{
  const gchar *desc;

  g_return_if_fail (PICMAN_IS_ENUM_LABEL (label));

  if (! picman_enum_get_value (G_TYPE_FROM_CLASS (label->enum_class), value,
                             NULL, NULL, &desc, NULL))
    {
      g_warning ("%s: %d is not valid for enum of type '%s'",
                 G_STRLOC, value,
                 g_type_name (G_TYPE_FROM_CLASS (label->enum_class)));
      return;
    }

  gtk_label_set_text (GTK_LABEL (label), desc);
}
