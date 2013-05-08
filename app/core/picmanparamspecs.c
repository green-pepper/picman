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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman.h"
#include "picmanimage.h"
#include "picmanlayer.h"
#include "picmanlayermask.h"
#include "picmanparamspecs.h"
#include "picmanselection.h"

#include "vectors/picmanvectors.h"


/*
 * PICMAN_TYPE_INT32
 */

GType
picman_int32_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanInt32", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT32
 */

static void   picman_param_int32_class_init (GParamSpecClass *klass);
static void   picman_param_int32_init       (GParamSpec      *pspec);

GType
picman_param_int32_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int32_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecInt32),
        0,
        (GInstanceInitFunc) picman_param_int32_init
      };

      type = g_type_register_static (G_TYPE_PARAM_INT,
                                     "PicmanParamInt32", &info, 0);
    }

  return type;
}

static void
picman_param_int32_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT32;
}

static void
picman_param_int32_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int32 (const gchar *name,
                       const gchar *nick,
                       const gchar *blurb,
                       gint         minimum,
                       gint         maximum,
                       gint         default_value,
                       GParamFlags  flags)
{
  GParamSpecInt *ispec;

  g_return_val_if_fail (minimum >= G_MININT32, NULL);
  g_return_val_if_fail (maximum <= G_MAXINT32, NULL);
  g_return_val_if_fail (default_value >= minimum &&
                        default_value <= maximum, NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT32,
                                 name, nick, blurb, flags);

  ispec->minimum       = minimum;
  ispec->maximum       = maximum;
  ispec->default_value = default_value;

  return G_PARAM_SPEC (ispec);
}


/*
 * PICMAN_TYPE_INT16
 */

GType
picman_int16_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanInt16", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT16
 */

static void   picman_param_int16_class_init (GParamSpecClass *klass);
static void   picman_param_int16_init       (GParamSpec      *pspec);

GType
picman_param_int16_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int16_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecInt16),
        0,
        (GInstanceInitFunc) picman_param_int16_init
      };

      type = g_type_register_static (G_TYPE_PARAM_INT,
                                     "PicmanParamInt16", &info, 0);
    }

  return type;
}

static void
picman_param_int16_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT16;
}

static void
picman_param_int16_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int16 (const gchar *name,
                       const gchar *nick,
                       const gchar *blurb,
                       gint         minimum,
                       gint         maximum,
                       gint         default_value,
                       GParamFlags  flags)
{
  GParamSpecInt *ispec;

  g_return_val_if_fail (minimum >= G_MININT16, NULL);
  g_return_val_if_fail (maximum <= G_MAXINT16, NULL);
  g_return_val_if_fail (default_value >= minimum &&
                        default_value <= maximum, NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT16,
                                 name, nick, blurb, flags);

  ispec->minimum       = minimum;
  ispec->maximum       = maximum;
  ispec->default_value = default_value;

  return G_PARAM_SPEC (ispec);
}


/*
 * PICMAN_TYPE_INT8
 */

GType
picman_int8_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_UINT, "PicmanInt8", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT8
 */

static void   picman_param_int8_class_init (GParamSpecClass *klass);
static void   picman_param_int8_init       (GParamSpec      *pspec);

GType
picman_param_int8_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int8_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecInt8),
        0,
        (GInstanceInitFunc) picman_param_int8_init
      };

      type = g_type_register_static (G_TYPE_PARAM_UINT,
                                     "PicmanParamInt8", &info, 0);
    }

  return type;
}

static void
picman_param_int8_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT8;
}

static void
picman_param_int8_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int8 (const gchar *name,
                      const gchar *nick,
                      const gchar *blurb,
                      guint        minimum,
                      guint        maximum,
                      guint        default_value,
                      GParamFlags  flags)
{
  GParamSpecInt *ispec;

  g_return_val_if_fail (maximum <= G_MAXUINT8, NULL);
  g_return_val_if_fail (default_value >= minimum &&
                        default_value <= maximum, NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT8,
                                 name, nick, blurb, flags);

  ispec->minimum       = minimum;
  ispec->maximum       = maximum;
  ispec->default_value = default_value;

  return G_PARAM_SPEC (ispec);
}


/*
 * PICMAN_TYPE_PARAM_STRING
 */

static void       picman_param_string_class_init (GParamSpecClass *klass);
static void       picman_param_string_init       (GParamSpec      *pspec);
static gboolean   picman_param_string_validate   (GParamSpec      *pspec,
                                                GValue          *value);

static GParamSpecClass * picman_param_string_parent_class = NULL;

GType
picman_param_string_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_string_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecString),
        0,
        (GInstanceInitFunc) picman_param_string_init
      };

      type = g_type_register_static (G_TYPE_PARAM_STRING,
                                     "PicmanParamString", &info, 0);
    }

  return type;
}

static void
picman_param_string_class_init (GParamSpecClass *klass)
{
  picman_param_string_parent_class = g_type_class_peek_parent (klass);

  klass->value_type     = G_TYPE_STRING;
  klass->value_validate = picman_param_string_validate;
}

static void
picman_param_string_init (GParamSpec *pspec)
{
  PicmanParamSpecString *sspec = PICMAN_PARAM_SPEC_STRING (pspec);

  G_PARAM_SPEC_STRING (pspec)->ensure_non_null = TRUE;

  sspec->allow_non_utf8 = FALSE;
  sspec->non_empty      = FALSE;
}

static gboolean
picman_param_string_validate (GParamSpec *pspec,
                            GValue     *value)
{
  PicmanParamSpecString *sspec  = PICMAN_PARAM_SPEC_STRING (pspec);
  gchar               *string = value->data[0].v_pointer;

  if (picman_param_string_parent_class->value_validate (pspec, value))
    return TRUE;

  if (string)
    {
      gchar *s;

      if (sspec->non_empty && ! string[0])
        {
          if (!(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS))
            g_free (string);
          else
            value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;

          value->data[0].v_pointer = g_strdup ("none");
          return TRUE;
        }

      if (! sspec->allow_non_utf8 &&
          ! g_utf8_validate (string, -1, (const gchar **) &s))
        {
          if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)
            {
              value->data[0].v_pointer = g_strdup (string);
              value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
              string = value->data[0].v_pointer;
            }

          for (s = string; *s; s++)
            if (*s < ' ')
              *s = '?';

          return TRUE;
        }
    }
  else if (sspec->non_empty)
    {
      value->data[1].v_uint &= ~G_VALUE_NOCOPY_CONTENTS;
      value->data[0].v_pointer = g_strdup ("none");
      return TRUE;
    }

  return FALSE;
}

GParamSpec *
picman_param_spec_string (const gchar *name,
                        const gchar *nick,
                        const gchar *blurb,
                        gboolean     allow_non_utf8,
                        gboolean     null_ok,
                        gboolean     non_empty,
                        const gchar *default_value,
                        GParamFlags  flags)
{
  PicmanParamSpecString *sspec;

  g_return_val_if_fail (! (null_ok && non_empty), NULL);

  sspec = g_param_spec_internal (PICMAN_TYPE_PARAM_STRING,
                                 name, nick, blurb, flags);

  if (sspec)
    {
      g_free (G_PARAM_SPEC_STRING (sspec)->default_value);
      G_PARAM_SPEC_STRING (sspec)->default_value = g_strdup (default_value);

      G_PARAM_SPEC_STRING (sspec)->ensure_non_null = null_ok ? FALSE : TRUE;

      sspec->allow_non_utf8 = allow_non_utf8 ? TRUE : FALSE;
      sspec->non_empty      = non_empty      ? TRUE : FALSE;
    }

  return G_PARAM_SPEC (sspec);
}


/*
 * PICMAN_TYPE_PARAM_ENUM
 */

static void       picman_param_enum_class_init (GParamSpecClass *klass);
static void       picman_param_enum_init       (GParamSpec      *pspec);
static void       picman_param_enum_finalize   (GParamSpec      *pspec);
static gboolean   picman_param_enum_validate   (GParamSpec      *pspec,
                                              GValue          *value);

GType
picman_param_enum_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_enum_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecEnum),
        0,
        (GInstanceInitFunc) picman_param_enum_init
      };

      type = g_type_register_static (G_TYPE_PARAM_ENUM,
                                     "PicmanParamEnum", &info, 0);
    }

  return type;
}

static void
picman_param_enum_class_init (GParamSpecClass *klass)
{
  klass->value_type     = G_TYPE_ENUM;
  klass->finalize       = picman_param_enum_finalize;
  klass->value_validate = picman_param_enum_validate;
}

static void
picman_param_enum_init (GParamSpec *pspec)
{
  PicmanParamSpecEnum *espec = PICMAN_PARAM_SPEC_ENUM (pspec);

  espec->excluded_values = NULL;
}

static void
picman_param_enum_finalize (GParamSpec *pspec)
{
  PicmanParamSpecEnum *espec = PICMAN_PARAM_SPEC_ENUM (pspec);
  GParamSpecClass   *parent_class;

  parent_class = g_type_class_peek (g_type_parent (PICMAN_TYPE_PARAM_ENUM));

  g_slist_free (espec->excluded_values);

  parent_class->finalize (pspec);
}

static gboolean
picman_param_enum_validate (GParamSpec *pspec,
                          GValue     *value)
{
  PicmanParamSpecEnum *espec  = PICMAN_PARAM_SPEC_ENUM (pspec);
  GParamSpecClass   *parent_class;
  GSList            *list;

  parent_class = g_type_class_peek (g_type_parent (PICMAN_TYPE_PARAM_ENUM));

  if (parent_class->value_validate (pspec, value))
    return TRUE;

  for (list = espec->excluded_values; list; list = g_slist_next (list))
    {
      if (GPOINTER_TO_INT (list->data) == value->data[0].v_long)
        {
          value->data[0].v_long = G_PARAM_SPEC_ENUM (pspec)->default_value;
          return TRUE;
        }
    }

  return FALSE;
}

GParamSpec *
picman_param_spec_enum (const gchar *name,
                      const gchar *nick,
                      const gchar *blurb,
                      GType        enum_type,
                      gint         default_value,
                      GParamFlags  flags)
{
  PicmanParamSpecEnum *espec;
  GEnumClass        *enum_class;

  g_return_val_if_fail (G_TYPE_IS_ENUM (enum_type), NULL);

  enum_class = g_type_class_ref (enum_type);

  g_return_val_if_fail (g_enum_get_value (enum_class, default_value) != NULL,
                        NULL);

  espec = g_param_spec_internal (PICMAN_TYPE_PARAM_ENUM,
                                 name, nick, blurb, flags);

  G_PARAM_SPEC_ENUM (espec)->enum_class    = enum_class;
  G_PARAM_SPEC_ENUM (espec)->default_value = default_value;
  G_PARAM_SPEC (espec)->value_type         = enum_type;

  return G_PARAM_SPEC (espec);
}

void
picman_param_spec_enum_exclude_value (PicmanParamSpecEnum *espec,
                                    gint               value)
{
  g_return_if_fail (PICMAN_IS_PARAM_SPEC_ENUM (espec));
  g_return_if_fail (g_enum_get_value (G_PARAM_SPEC_ENUM (espec)->enum_class,
                                      value) != NULL);

  espec->excluded_values = g_slist_prepend (espec->excluded_values,
                                            GINT_TO_POINTER (value));
}


/*
 * PICMAN_TYPE_IMAGE_ID
 */

GType
picman_image_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanImageID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_IMAGE_ID
 */

static void       picman_param_image_id_class_init  (GParamSpecClass *klass);
static void       picman_param_image_id_init        (GParamSpec      *pspec);
static void       picman_param_image_id_set_default (GParamSpec      *pspec,
                                                   GValue          *value);
static gboolean   picman_param_image_id_validate    (GParamSpec      *pspec,
                                                   GValue          *value);
static gint       picman_param_image_id_values_cmp  (GParamSpec      *pspec,
                                                   const GValue    *value1,
                                                   const GValue    *value2);

GType
picman_param_image_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_image_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecImageID),
        0,
        (GInstanceInitFunc) picman_param_image_id_init
      };

      type = g_type_register_static (G_TYPE_PARAM_INT,
                                     "PicmanParamImageID", &info, 0);
    }

  return type;
}

static void
picman_param_image_id_class_init (GParamSpecClass *klass)
{
  klass->value_type        = PICMAN_TYPE_IMAGE_ID;
  klass->value_set_default = picman_param_image_id_set_default;
  klass->value_validate    = picman_param_image_id_validate;
  klass->values_cmp        = picman_param_image_id_values_cmp;
}

static void
picman_param_image_id_init (GParamSpec *pspec)
{
  PicmanParamSpecImageID *ispec = PICMAN_PARAM_SPEC_IMAGE_ID (pspec);

  ispec->picman    = NULL;
  ispec->none_ok = FALSE;
}

static void
picman_param_image_id_set_default (GParamSpec *pspec,
                                 GValue     *value)
{
  value->data[0].v_int = -1;
}

static gboolean
picman_param_image_id_validate (GParamSpec *pspec,
                              GValue     *value)
{
  PicmanParamSpecImageID *ispec    = PICMAN_PARAM_SPEC_IMAGE_ID (pspec);
  gint                  image_id = value->data[0].v_int;
  PicmanImage            *image;

  if (ispec->none_ok && (image_id == 0 || image_id == -1))
    return FALSE;

  image = picman_image_get_by_ID (ispec->picman, image_id);

  if (! PICMAN_IS_IMAGE (image))
    {
      value->data[0].v_int = -1;
      return TRUE;
    }

  return FALSE;
}

static gint
picman_param_image_id_values_cmp (GParamSpec   *pspec,
                                const GValue *value1,
                                const GValue *value2)
{
  gint image_id1 = value1->data[0].v_int;
  gint image_id2 = value2->data[0].v_int;

  /*  try to return at least *something*, it's useless anyway...  */

  if (image_id1 < image_id2)
    return -1;
  else if (image_id1 > image_id2)
    return 1;
  else
    return 0;
}

GParamSpec *
picman_param_spec_image_id (const gchar *name,
                          const gchar *nick,
                          const gchar *blurb,
                          Picman        *picman,
                          gboolean     none_ok,
                          GParamFlags  flags)
{
  PicmanParamSpecImageID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_IMAGE_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanImage *
picman_value_get_image (const GValue *value,
                      Picman         *picman)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_IMAGE_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman_image_get_by_ID (picman, value->data[0].v_int);
}

void
picman_value_set_image (GValue    *value,
                      PicmanImage *image)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_IMAGE_ID (value));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));

  value->data[0].v_int = image ? picman_image_get_ID (image) : -1;
}


/*
 * PICMAN_TYPE_ITEM_ID
 */

GType
picman_item_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanItemID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_ITEM_ID
 */

static void       picman_param_item_id_class_init  (GParamSpecClass *klass);
static void       picman_param_item_id_init        (GParamSpec      *pspec);
static void       picman_param_item_id_set_default (GParamSpec      *pspec,
                                                  GValue          *value);
static gboolean   picman_param_item_id_validate    (GParamSpec      *pspec,
                                                  GValue          *value);
static gint       picman_param_item_id_values_cmp  (GParamSpec      *pspec,
                                                  const GValue    *value1,
                                                  const GValue    *value2);

GType
picman_param_item_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_item_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecItemID),
        0,
        (GInstanceInitFunc) picman_param_item_id_init
      };

      type = g_type_register_static (G_TYPE_PARAM_INT,
                                     "PicmanParamItemID", &info, 0);
    }

  return type;
}

static void
picman_param_item_id_class_init (GParamSpecClass *klass)
{
  klass->value_type        = PICMAN_TYPE_ITEM_ID;
  klass->value_set_default = picman_param_item_id_set_default;
  klass->value_validate    = picman_param_item_id_validate;
  klass->values_cmp        = picman_param_item_id_values_cmp;
}

static void
picman_param_item_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->picman      = NULL;
  ispec->item_type = PICMAN_TYPE_ITEM;
  ispec->none_ok   = FALSE;
}

static void
picman_param_item_id_set_default (GParamSpec *pspec,
                                GValue     *value)
{
  value->data[0].v_int = -1;
}

static gboolean
picman_param_item_id_validate (GParamSpec *pspec,
                             GValue     *value)
{
  PicmanParamSpecItemID *ispec   = PICMAN_PARAM_SPEC_ITEM_ID (pspec);
  gint                 item_id = value->data[0].v_int;
  PicmanItem            *item;

  if (ispec->none_ok && (item_id == 0 || item_id == -1))
    return FALSE;

  item = picman_item_get_by_ID (ispec->picman, item_id);

  if (! item || ! g_type_is_a (G_TYPE_FROM_INSTANCE (item), ispec->item_type))
    {
      value->data[0].v_int = -1;
      return TRUE;
    }
  else if (picman_item_is_removed (item))
    {
      value->data[0].v_int = -1;
      return TRUE;
    }

  return FALSE;
}

static gint
picman_param_item_id_values_cmp (GParamSpec   *pspec,
                               const GValue *value1,
                               const GValue *value2)
{
  gint item_id1 = value1->data[0].v_int;
  gint item_id2 = value2->data[0].v_int;

  /*  try to return at least *something*, it's useless anyway...  */

  if (item_id1 < item_id2)
    return -1;
  else if (item_id1 > item_id2)
    return 1;
  else
    return 0;
}

GParamSpec *
picman_param_spec_item_id (const gchar *name,
                         const gchar *nick,
                         const gchar *blurb,
                         Picman        *picman,
                         gboolean     none_ok,
                         GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_ITEM_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok;

  return G_PARAM_SPEC (ispec);
}

PicmanItem *
picman_value_get_item (const GValue *value,
                     Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_ITEM_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_ITEM (item))
    return NULL;

  return item;
}

void
picman_value_set_item (GValue   *value,
                     PicmanItem *item)
{
  g_return_if_fail (item == NULL || PICMAN_IS_ITEM (item));

  /* FIXME remove hack as soon as bug #375864 is fixed */

  if (PICMAN_VALUE_HOLDS_ITEM_ID (value))
    {
      value->data[0].v_int = item ? picman_item_get_ID (item) : -1;
    }
  else if (PICMAN_VALUE_HOLDS_DRAWABLE_ID (value) &&
           (item == NULL || PICMAN_IS_DRAWABLE (item)))
    {
      picman_value_set_drawable (value, PICMAN_DRAWABLE (item));
    }
  else if (PICMAN_VALUE_HOLDS_LAYER_ID (value) &&
           (item == NULL || PICMAN_IS_LAYER (item)))
    {
      picman_value_set_layer (value, PICMAN_LAYER (item));
    }
  else if (PICMAN_VALUE_HOLDS_CHANNEL_ID (value) &&
           (item == NULL || PICMAN_IS_CHANNEL (item)))
    {
      picman_value_set_channel (value, PICMAN_CHANNEL (item));
    }
  else if (PICMAN_VALUE_HOLDS_LAYER_MASK_ID (value) &&
           (item == NULL || PICMAN_IS_LAYER_MASK (item)))
    {
      picman_value_set_layer_mask (value, PICMAN_LAYER_MASK (item));
    }
  else if (PICMAN_VALUE_HOLDS_SELECTION_ID (value) &&
           (item == NULL || PICMAN_IS_SELECTION (item)))
    {
      picman_value_set_selection (value, PICMAN_SELECTION (item));
    }
  else if (PICMAN_VALUE_HOLDS_VECTORS_ID (value) &&
           (item == NULL || PICMAN_IS_VECTORS (item)))
    {
      picman_value_set_vectors (value, PICMAN_VECTORS (item));
    }
  else
    {
      g_return_if_reached ();
    }
}


/*
 * PICMAN_TYPE_DRAWABLE_ID
 */

GType
picman_drawable_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanDrawableID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_DRAWABLE_ID
 */

static void   picman_param_drawable_id_class_init (GParamSpecClass *klass);
static void   picman_param_drawable_id_init       (GParamSpec      *pspec);

GType
picman_param_drawable_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_drawable_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecDrawableID),
        0,
        (GInstanceInitFunc) picman_param_drawable_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ITEM_ID,
                                     "PicmanParamDrawableID", &info, 0);
    }

  return type;
}

static void
picman_param_drawable_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_DRAWABLE_ID;
}

static void
picman_param_drawable_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_DRAWABLE;
}

GParamSpec *
picman_param_spec_drawable_id (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             Picman        *picman,
                             gboolean     none_ok,
                             GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_DRAWABLE_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanDrawable *
picman_value_get_drawable (const GValue *value,
                         Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_DRAWABLE_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_DRAWABLE (item))
    return NULL;

  return PICMAN_DRAWABLE (item);
}

void
picman_value_set_drawable (GValue       *value,
                         PicmanDrawable *drawable)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_DRAWABLE_ID (value));
  g_return_if_fail (drawable == NULL || PICMAN_IS_DRAWABLE (drawable));

  value->data[0].v_int = drawable ? picman_item_get_ID (PICMAN_ITEM (drawable)) : -1;
}


/*
 * PICMAN_TYPE_LAYER_ID
 */

GType
picman_layer_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanLayerID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_LAYER_ID
 */

static void   picman_param_layer_id_class_init (GParamSpecClass *klass);
static void   picman_param_layer_id_init       (GParamSpec      *pspec);

GType
picman_param_layer_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_layer_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecLayerID),
        0,
        (GInstanceInitFunc) picman_param_layer_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_DRAWABLE_ID,
                                     "PicmanParamLayerID", &info, 0);
    }

  return type;
}

static void
picman_param_layer_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_LAYER_ID;
}

static void
picman_param_layer_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_LAYER;
}

GParamSpec *
picman_param_spec_layer_id (const gchar *name,
                          const gchar *nick,
                          const gchar *blurb,
                          Picman        *picman,
                          gboolean     none_ok,
                          GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_LAYER_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanLayer *
picman_value_get_layer (const GValue *value,
                      Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_LAYER_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_LAYER (item))
    return NULL;

  return PICMAN_LAYER (item);
}

void
picman_value_set_layer (GValue    *value,
                      PicmanLayer *layer)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_LAYER_ID (value));
  g_return_if_fail (layer == NULL || PICMAN_IS_LAYER (layer));

  value->data[0].v_int = layer ? picman_item_get_ID (PICMAN_ITEM (layer)) : -1;
}


/*
 * PICMAN_TYPE_CHANNEL_ID
 */

GType
picman_channel_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanChannelID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_CHANNEL_ID
 */

static void   picman_param_channel_id_class_init (GParamSpecClass *klass);
static void   picman_param_channel_id_init       (GParamSpec      *pspec);

GType
picman_param_channel_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_channel_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecChannelID),
        0,
        (GInstanceInitFunc) picman_param_channel_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_DRAWABLE_ID,
                                     "PicmanParamChannelID", &info, 0);
    }

  return type;
}

static void
picman_param_channel_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_CHANNEL_ID;
}

static void
picman_param_channel_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_CHANNEL;
}

GParamSpec *
picman_param_spec_channel_id (const gchar *name,
                            const gchar *nick,
                            const gchar *blurb,
                            Picman        *picman,
                            gboolean     none_ok,
                            GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_CHANNEL_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanChannel *
picman_value_get_channel (const GValue *value,
                        Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_CHANNEL_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_CHANNEL (item))
    return NULL;

  return PICMAN_CHANNEL (item);
}

void
picman_value_set_channel (GValue      *value,
                        PicmanChannel *channel)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_CHANNEL_ID (value));
  g_return_if_fail (channel == NULL || PICMAN_IS_CHANNEL (channel));

  value->data[0].v_int = channel ? picman_item_get_ID (PICMAN_ITEM (channel)) : -1;
}


/*
 * PICMAN_TYPE_LAYER_MASK_ID
 */

GType
picman_layer_mask_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanLayerMaskID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_LAYER_MASK_ID
 */

static void   picman_param_layer_mask_id_class_init (GParamSpecClass *klass);
static void   picman_param_layer_mask_id_init       (GParamSpec      *pspec);

GType
picman_param_layer_mask_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_layer_mask_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecLayerMaskID),
        0,
        (GInstanceInitFunc) picman_param_layer_mask_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_CHANNEL_ID,
                                     "PicmanParamLayerMaskID", &info, 0);
    }

  return type;
}

static void
picman_param_layer_mask_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_LAYER_MASK_ID;
}

static void
picman_param_layer_mask_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_LAYER_MASK;
}

GParamSpec *
picman_param_spec_layer_mask_id (const gchar *name,
                               const gchar *nick,
                               const gchar *blurb,
                               Picman        *picman,
                               gboolean     none_ok,
                               GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_LAYER_MASK_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanLayerMask *
picman_value_get_layer_mask (const GValue *value,
                           Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_LAYER_MASK_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_LAYER_MASK (item))
    return NULL;

  return PICMAN_LAYER_MASK (item);
}

void
picman_value_set_layer_mask (GValue        *value,
                           PicmanLayerMask *layer_mask)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_LAYER_MASK_ID (value));
  g_return_if_fail (layer_mask == NULL || PICMAN_IS_LAYER_MASK (layer_mask));

  value->data[0].v_int = layer_mask ? picman_item_get_ID (PICMAN_ITEM (layer_mask)) : -1;
}


/*
 * PICMAN_TYPE_SELECTION_ID
 */

GType
picman_selection_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanSelectionID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_SELECTION_ID
 */

static void   picman_param_selection_id_class_init (GParamSpecClass *klass);
static void   picman_param_selection_id_init       (GParamSpec      *pspec);

GType
picman_param_selection_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_selection_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecSelectionID),
        0,
        (GInstanceInitFunc) picman_param_selection_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_CHANNEL_ID,
                                     "PicmanParamSelectionID", &info, 0);
    }

  return type;
}

static void
picman_param_selection_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_SELECTION_ID;
}

static void
picman_param_selection_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_SELECTION;
}

GParamSpec *
picman_param_spec_selection_id (const gchar *name,
                              const gchar *nick,
                              const gchar *blurb,
                              Picman        *picman,
                              gboolean     none_ok,
                              GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_SELECTION_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanSelection *
picman_value_get_selection (const GValue *value,
                          Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_SELECTION_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_SELECTION (item))
    return NULL;

  return PICMAN_SELECTION (item);
}

void
picman_value_set_selection (GValue        *value,
                          PicmanSelection *selection)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_SELECTION_ID (value));
  g_return_if_fail (selection == NULL || PICMAN_IS_SELECTION (selection));

  value->data[0].v_int = selection ? picman_item_get_ID (PICMAN_ITEM (selection)) : -1;
}


/*
 * PICMAN_TYPE_VECTORS_ID
 */

GType
picman_vectors_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanVectorsID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_VECTORS_ID
 */

static void   picman_param_vectors_id_class_init (GParamSpecClass *klass);
static void   picman_param_vectors_id_init       (GParamSpec      *pspec);

GType
picman_param_vectors_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_vectors_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecVectorsID),
        0,
        (GInstanceInitFunc) picman_param_vectors_id_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ITEM_ID,
                                     "PicmanParamVectorsID", &info, 0);
    }

  return type;
}

static void
picman_param_vectors_id_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_VECTORS_ID;
}

static void
picman_param_vectors_id_init (GParamSpec *pspec)
{
  PicmanParamSpecItemID *ispec = PICMAN_PARAM_SPEC_ITEM_ID (pspec);

  ispec->item_type = PICMAN_TYPE_VECTORS;
}

GParamSpec *
picman_param_spec_vectors_id (const gchar *name,
                            const gchar *nick,
                            const gchar *blurb,
                            Picman        *picman,
                            gboolean     none_ok,
                            GParamFlags  flags)
{
  PicmanParamSpecItemID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_VECTORS_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanVectors *
picman_value_get_vectors (const GValue *value,
                        Picman         *picman)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_VECTORS_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  item = picman_item_get_by_ID (picman, value->data[0].v_int);

  if (item && ! PICMAN_IS_VECTORS (item))
    return NULL;

  return PICMAN_VECTORS (item);
}

void
picman_value_set_vectors (GValue      *value,
                        PicmanVectors *vectors)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_VECTORS_ID (value));
  g_return_if_fail (vectors == NULL || PICMAN_IS_VECTORS (vectors));

  value->data[0].v_int = vectors ? picman_item_get_ID (PICMAN_ITEM (vectors)) : -1;
}


/*
 * PICMAN_TYPE_DISPLAY_ID
 */

GType
picman_display_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info = { 0, };

      type = g_type_register_static (G_TYPE_INT, "PicmanDisplayID", &info, 0);
    }

  return type;
}


/*
 * PICMAN_TYPE_PARAM_DISPLAY_ID
 */

static void       picman_param_display_id_class_init  (GParamSpecClass *klass);
static void       picman_param_display_id_init        (GParamSpec      *pspec);
static void       picman_param_display_id_set_default (GParamSpec      *pspec,
                                                     GValue          *value);
static gboolean   picman_param_display_id_validate    (GParamSpec      *pspec,
                                                     GValue          *value);
static gint       picman_param_display_id_values_cmp  (GParamSpec      *pspec,
                                                     const GValue    *value1,
                                                     const GValue    *value2);

GType
picman_param_display_id_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_display_id_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecDisplayID),
        0,
        (GInstanceInitFunc) picman_param_display_id_init
      };

      type = g_type_register_static (G_TYPE_PARAM_INT,
                                     "PicmanParamDisplayID", &info, 0);
    }

  return type;
}

static void
picman_param_display_id_class_init (GParamSpecClass *klass)
{
  klass->value_type        = PICMAN_TYPE_DISPLAY_ID;
  klass->value_set_default = picman_param_display_id_set_default;
  klass->value_validate    = picman_param_display_id_validate;
  klass->values_cmp        = picman_param_display_id_values_cmp;
}

static void
picman_param_display_id_init (GParamSpec *pspec)
{
  PicmanParamSpecDisplayID *ispec = PICMAN_PARAM_SPEC_DISPLAY_ID (pspec);

  ispec->picman    = NULL;
  ispec->none_ok = FALSE;
}

static void
picman_param_display_id_set_default (GParamSpec *pspec,
                                   GValue     *value)
{
  value->data[0].v_int = -1;
}

static gboolean
picman_param_display_id_validate (GParamSpec *pspec,
                                GValue     *value)
{
  PicmanParamSpecDisplayID *ispec      = PICMAN_PARAM_SPEC_DISPLAY_ID (pspec);
  gint                    display_id = value->data[0].v_int;
  PicmanObject             *display;

  if (ispec->none_ok && (display_id == 0 || display_id == -1))
    return FALSE;

  display = picman_get_display_by_ID (ispec->picman, display_id);

  if (! PICMAN_IS_OBJECT (display))
    {
      value->data[0].v_int = -1;
      return TRUE;
    }

  return FALSE;
}

static gint
picman_param_display_id_values_cmp (GParamSpec   *pspec,
                                  const GValue *value1,
                                  const GValue *value2)
{
  gint display_id1 = value1->data[0].v_int;
  gint display_id2 = value2->data[0].v_int;

  /*  try to return at least *something*, it's useless anyway...  */

  if (display_id1 < display_id2)
    return -1;
  else if (display_id1 > display_id2)
    return 1;
  else
    return 0;
}

GParamSpec *
picman_param_spec_display_id (const gchar *name,
                            const gchar *nick,
                            const gchar *blurb,
                            Picman        *picman,
                            gboolean     none_ok,
                            GParamFlags  flags)
{
  PicmanParamSpecDisplayID *ispec;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  ispec = g_param_spec_internal (PICMAN_TYPE_PARAM_DISPLAY_ID,
                                 name, nick, blurb, flags);

  ispec->picman    = picman;
  ispec->none_ok = none_ok ? TRUE : FALSE;

  return G_PARAM_SPEC (ispec);
}

PicmanObject *
picman_value_get_display (const GValue *value,
                        Picman         *picman)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_DISPLAY_ID (value), NULL);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return picman_get_display_by_ID (picman, value->data[0].v_int);
}

void
picman_value_set_display (GValue     *value,
                        PicmanObject *display)
{
  gint id = -1;

  g_return_if_fail (PICMAN_VALUE_HOLDS_DISPLAY_ID (value));
  g_return_if_fail (display == NULL || PICMAN_IS_OBJECT (display));

  if (display)
    g_object_get (display, "id", &id, NULL);

  value->data[0].v_int = id;
}


/*
 * PICMAN_TYPE_ARRAY
 */

PicmanArray *
picman_array_new (const guint8 *data,
                gsize         length,
                gboolean      static_data)
{
  PicmanArray *array;

  g_return_val_if_fail ((data == NULL && length == 0) ||
                        (data != NULL && length  > 0), NULL);

  array = g_slice_new0 (PicmanArray);

  array->data        = static_data ? (guint8 *) data : g_memdup (data, length);
  array->length      = length;
  array->static_data = static_data;

  return array;
}

PicmanArray *
picman_array_copy (const PicmanArray *array)
{
  if (array)
    return picman_array_new (array->data, array->length, FALSE);

  return NULL;
}

void
picman_array_free (PicmanArray *array)
{
  if (array)
    {
      if (! array->static_data)
        g_free (array->data);

      g_slice_free (PicmanArray, array);
    }
}

GType
picman_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanArray",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_ARRAY
 */

static void       picman_param_array_class_init  (GParamSpecClass *klass);
static void       picman_param_array_init        (GParamSpec      *pspec);
static gboolean   picman_param_array_validate    (GParamSpec      *pspec,
                                                GValue          *value);
static gint       picman_param_array_values_cmp  (GParamSpec      *pspec,
                                                const GValue    *value1,
                                                const GValue    *value2);

GType
picman_param_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_array_init
      };

      type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                     "PicmanParamArray", &info, 0);
    }

  return type;
}

static void
picman_param_array_class_init (GParamSpecClass *klass)
{
  klass->value_type     = PICMAN_TYPE_ARRAY;
  klass->value_validate = picman_param_array_validate;
  klass->values_cmp     = picman_param_array_values_cmp;
}

static void
picman_param_array_init (GParamSpec *pspec)
{
}

static gboolean
picman_param_array_validate (GParamSpec *pspec,
                           GValue     *value)
{
  PicmanArray *array = value->data[0].v_pointer;

  if (array)
    {
      if ((array->data == NULL && array->length != 0) ||
          (array->data != NULL && array->length == 0))
        {
          g_value_set_boxed (value, NULL);
          return TRUE;
        }
    }

  return FALSE;
}

static gint
picman_param_array_values_cmp (GParamSpec   *pspec,
                             const GValue *value1,
                             const GValue *value2)
{
  PicmanArray *array1 = value1->data[0].v_pointer;
  PicmanArray *array2 = value2->data[0].v_pointer;

  /*  try to return at least *something*, it's useless anyway...  */

  if (! array1)
    return array2 != NULL ? -1 : 0;
  else if (! array2)
    return array1 != NULL ? 1 : 0;
  else if (array1->length < array2->length)
    return -1;
  else if (array1->length > array2->length)
    return 1;

  return 0;
}

GParamSpec *
picman_param_spec_array (const gchar *name,
                       const gchar *nick,
                       const gchar *blurb,
                       GParamFlags  flags)
{
  PicmanParamSpecArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

static const guint8 *
picman_value_get_array (const GValue *value)
{
  PicmanArray *array = value->data[0].v_pointer;

  if (array)
    return array->data;

  return NULL;
}

static guint8 *
picman_value_dup_array (const GValue *value)
{
  PicmanArray *array = value->data[0].v_pointer;

  if (array)
    return g_memdup (array->data, array->length);

  return NULL;
}

static void
picman_value_set_array (GValue       *value,
                      const guint8 *data,
                      gsize         length)
{
  PicmanArray *array = picman_array_new (data, length, FALSE);

  g_value_take_boxed (value, array);
}

static void
picman_value_set_static_array (GValue       *value,
                             const guint8 *data,
                             gsize         length)
{
  PicmanArray *array = picman_array_new (data, length, TRUE);

  g_value_take_boxed (value, array);
}

static void
picman_value_take_array (GValue *value,
                       guint8 *data,
                       gsize   length)
{
  PicmanArray *array = picman_array_new (data, length, TRUE);

  array->static_data = FALSE;

  g_value_take_boxed (value, array);
}


/*
 * PICMAN_TYPE_INT8_ARRAY
 */

GType
picman_int8_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanInt8Array",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT8_ARRAY
 */

static void   picman_param_int8_array_class_init (GParamSpecClass *klass);
static void   picman_param_int8_array_init       (GParamSpec      *pspec);

GType
picman_param_int8_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int8_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_int8_array_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ARRAY,
                                     "PicmanParamInt8Array", &info, 0);
    }

  return type;
}

static void
picman_param_int8_array_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT8_ARRAY;
}

static void
picman_param_int8_array_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int8_array (const gchar *name,
                            const gchar *nick,
                            const gchar *blurb,
                            GParamFlags  flags)
{
  PicmanParamSpecArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT8_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const guint8 *
picman_value_get_int8array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT8_ARRAY (value), NULL);

  return picman_value_get_array (value);
}

guint8 *
picman_value_dup_int8array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT8_ARRAY (value), NULL);

  return picman_value_dup_array (value);
}

void
picman_value_set_int8array (GValue       *value,
                          const guint8 *data,
                          gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT8_ARRAY (value));

  picman_value_set_array (value, data, length);
}

void
picman_value_set_static_int8array (GValue       *value,
                                 const guint8 *data,
                                 gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT8_ARRAY (value));

  picman_value_set_static_array (value, data, length);
}

void
picman_value_take_int8array (GValue *value,
                           guint8 *data,
                           gsize   length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT8_ARRAY (value));

  picman_value_take_array (value, data, length);
}


/*
 * PICMAN_TYPE_INT16_ARRAY
 */

GType
picman_int16_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanInt16Array",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT16_ARRAY
 */

static void   picman_param_int16_array_class_init (GParamSpecClass *klass);
static void   picman_param_int16_array_init       (GParamSpec      *pspec);

GType
picman_param_int16_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int16_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_int16_array_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ARRAY,
                                     "PicmanParamInt16Array", &info, 0);
    }

  return type;
}

static void
picman_param_int16_array_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT16_ARRAY;
}

static void
picman_param_int16_array_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int16_array (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             GParamFlags  flags)
{
  PicmanParamSpecArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT16_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const gint16 *
picman_value_get_int16array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT16_ARRAY (value), NULL);

  return (const gint16 *) picman_value_get_array (value);
}

gint16 *
picman_value_dup_int16array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT16_ARRAY (value), NULL);

  return (gint16 *) picman_value_dup_array (value);
}

void
picman_value_set_int16array (GValue       *value,
                           const gint16 *data,
                           gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT16_ARRAY (value));

  picman_value_set_array (value, (const guint8 *) data,
                        length * sizeof (gint16));
}

void
picman_value_set_static_int16array (GValue       *value,
                                  const gint16 *data,
                                  gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT16_ARRAY (value));

  picman_value_set_static_array (value, (const guint8 *) data,
                               length * sizeof (gint16));
}

void
picman_value_take_int16array (GValue *value,
                            gint16 *data,
                            gsize   length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT16_ARRAY (value));

  picman_value_take_array (value, (guint8 *) data,
                         length * sizeof (gint16));
}


/*
 * PICMAN_TYPE_INT32_ARRAY
 */

GType
picman_int32_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanInt32Array",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_INT32_ARRAY
 */

static void   picman_param_int32_array_class_init (GParamSpecClass *klass);
static void   picman_param_int32_array_init       (GParamSpec      *pspec);

GType
picman_param_int32_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_int32_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_int32_array_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ARRAY,
                                     "PicmanParamInt32Array", &info, 0);
    }

  return type;
}

static void
picman_param_int32_array_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_INT32_ARRAY;
}

static void
picman_param_int32_array_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_int32_array (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             GParamFlags  flags)
{
  PicmanParamSpecArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_INT32_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const gint32 *
picman_value_get_int32array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT32_ARRAY (value), NULL);

  return (const gint32 *) picman_value_get_array (value);
}

gint32 *
picman_value_dup_int32array (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_INT32_ARRAY (value), NULL);

  return (gint32 *) picman_value_dup_array (value);
}

void
picman_value_set_int32array (GValue       *value,
                           const gint32 *data,
                           gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT32_ARRAY (value));

  picman_value_set_array (value, (const guint8 *) data,
                        length * sizeof (gint32));
}

void
picman_value_set_static_int32array (GValue       *value,
                                  const gint32 *data,
                                  gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT32_ARRAY (value));

  picman_value_set_static_array (value, (const guint8 *) data,
                               length * sizeof (gint32));
}

void
picman_value_take_int32array (GValue *value,
                            gint32 *data,
                            gsize   length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_INT32_ARRAY (value));

  picman_value_take_array (value, (guint8 *) data,
                         length * sizeof (gint32));
}


/*
 * PICMAN_TYPE_FLOAT_ARRAY
 */

GType
picman_float_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanFloatArray",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_FLOAT_ARRAY
 */

static void   picman_param_float_array_class_init (GParamSpecClass *klass);
static void   picman_param_float_array_init       (GParamSpec      *pspec);

GType
picman_param_float_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_float_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_float_array_init
      };

      type = g_type_register_static (PICMAN_TYPE_PARAM_ARRAY,
                                     "PicmanParamFloatArray", &info, 0);
    }

  return type;
}

static void
picman_param_float_array_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_FLOAT_ARRAY;
}

static void
picman_param_float_array_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_float_array (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             GParamFlags  flags)
{
  PicmanParamSpecArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_FLOAT_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const gdouble *
picman_value_get_floatarray (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_FLOAT_ARRAY (value), NULL);

  return (const gdouble *) picman_value_get_array (value);
}

gdouble *
picman_value_dup_floatarray (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_FLOAT_ARRAY (value), NULL);

  return (gdouble *) picman_value_dup_array (value);
}

void
picman_value_set_floatarray (GValue        *value,
                           const gdouble *data,
                           gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_FLOAT_ARRAY (value));

  picman_value_set_array (value, (const guint8 *) data,
                        length * sizeof (gdouble));
}

void
picman_value_set_static_floatarray (GValue        *value,
                                  const gdouble *data,
                                  gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_FLOAT_ARRAY (value));

  picman_value_set_static_array (value, (const guint8 *) data,
                               length * sizeof (gdouble));
}

void
picman_value_take_floatarray (GValue  *value,
                            gdouble *data,
                            gsize    length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_FLOAT_ARRAY (value));

  picman_value_take_array (value, (guint8 *) data,
                         length * sizeof (gdouble));
}


/*
 * PICMAN_TYPE_STRING_ARRAY
 */

PicmanArray *
picman_string_array_new (const gchar **data,
                       gsize         length,
                       gboolean      static_data)
{
  PicmanArray *array;

  g_return_val_if_fail ((data == NULL && length == 0) ||
                        (data != NULL && length  > 0), NULL);

  array = g_slice_new0 (PicmanArray);

  if (! static_data)
    {
      gchar **tmp = g_new (gchar *, length);
      gint    i;

      for (i = 0; i < length; i++)
        tmp[i] = g_strdup (data[i]);

      array->data = (guint8 *) tmp;
    }
  else
    {
      array->data = (guint8 *) data;
    }

  array->length      = length;
  array->static_data = static_data;

  return array;
}

PicmanArray *
picman_string_array_copy (const PicmanArray *array)
{
  if (array)
    return picman_string_array_new ((const gchar **) array->data,
                                  array->length, FALSE);

  return NULL;
}

void
picman_string_array_free (PicmanArray *array)
{
  if (array)
    {
      if (! array->static_data)
        {
          gchar **tmp = (gchar **) array->data;
          gint    i;

          for (i = 0; i < array->length; i++)
            g_free (tmp[i]);

          g_free (array->data);
        }

      g_slice_free (PicmanArray, array);
    }
}

GType
picman_string_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanStringArray",
                                         (GBoxedCopyFunc) picman_string_array_copy,
                                         (GBoxedFreeFunc) picman_string_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_STRING_ARRAY
 */

static void       picman_param_string_array_class_init  (GParamSpecClass *klass);
static void       picman_param_string_array_init        (GParamSpec      *pspec);
static gboolean   picman_param_string_array_validate    (GParamSpec      *pspec,
                                                       GValue          *value);
static gint       picman_param_string_array_values_cmp  (GParamSpec      *pspec,
                                                       const GValue    *value1,
                                                       const GValue    *value2);

GType
picman_param_string_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_string_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_string_array_init
      };

      type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                     "PicmanParamStringArray", &info, 0);
    }

  return type;
}

static void
picman_param_string_array_class_init (GParamSpecClass *klass)
{
  klass->value_type     = PICMAN_TYPE_STRING_ARRAY;
  klass->value_validate = picman_param_string_array_validate;
  klass->values_cmp     = picman_param_string_array_values_cmp;
}

static void
picman_param_string_array_init (GParamSpec *pspec)
{
}

static gboolean
picman_param_string_array_validate (GParamSpec *pspec,
                                  GValue     *value)
{
  PicmanArray *array = value->data[0].v_pointer;

  if (array)
    {
      if ((array->data == NULL && array->length != 0) ||
          (array->data != NULL && array->length == 0))
        {
          g_value_set_boxed (value, NULL);
          return TRUE;
        }
    }

  return FALSE;
}

static gint
picman_param_string_array_values_cmp (GParamSpec   *pspec,
                                    const GValue *value1,
                                    const GValue *value2)
{
  PicmanArray *array1 = value1->data[0].v_pointer;
  PicmanArray *array2 = value2->data[0].v_pointer;

  /*  try to return at least *something*, it's useless anyway...  */

  if (! array1)
    return array2 != NULL ? -1 : 0;
  else if (! array2)
    return array1 != NULL ? 1 : 0;
  else if (array1->length < array2->length)
    return -1;
  else if (array1->length > array2->length)
    return 1;

  return 0;
}

GParamSpec *
picman_param_spec_string_array (const gchar *name,
                              const gchar *nick,
                              const gchar *blurb,
                              GParamFlags  flags)
{
  PicmanParamSpecStringArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_STRING_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const gchar **
picman_value_get_stringarray (const GValue *value)
{
  PicmanArray *array;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_STRING_ARRAY (value), NULL);

  array = value->data[0].v_pointer;

  if (array)
    return (const gchar **) array->data;

  return NULL;
}

gchar **
picman_value_dup_stringarray (const GValue *value)
{
  PicmanArray *array;

  g_return_val_if_fail (PICMAN_VALUE_HOLDS_STRING_ARRAY (value), NULL);

  array = value->data[0].v_pointer;

  if (array)
    {
      gchar **ret = g_memdup (array->data, array->length * sizeof (gchar *));
      gint    i;

      for (i = 0; i < array->length; i++)
        ret[i] = g_strdup (ret[i]);

      return ret;
    }

  return NULL;
}

void
picman_value_set_stringarray (GValue       *value,
                            const gchar **data,
                            gsize         length)
{
  PicmanArray *array;

  g_return_if_fail (PICMAN_VALUE_HOLDS_STRING_ARRAY (value));

  array = picman_string_array_new (data, length, FALSE);

  g_value_take_boxed (value, array);
}

void
picman_value_set_static_stringarray (GValue       *value,
                                   const gchar **data,
                                   gsize         length)
{
  PicmanArray *array;

  g_return_if_fail (PICMAN_VALUE_HOLDS_STRING_ARRAY (value));

  array = picman_string_array_new (data, length, TRUE);

  g_value_take_boxed (value, array);
}

void
picman_value_take_stringarray (GValue  *value,
                             gchar  **data,
                             gsize    length)
{
  PicmanArray *array;

  g_return_if_fail (PICMAN_VALUE_HOLDS_STRING_ARRAY (value));

  array = picman_string_array_new ((const gchar **) data, length, TRUE);
  array->static_data = FALSE;

  g_value_take_boxed (value, array);
}


/*
 * PICMAN_TYPE_COLOR_ARRAY
 */

GType
picman_color_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanColorArray",
                                         (GBoxedCopyFunc) picman_array_copy,
                                         (GBoxedFreeFunc) picman_array_free);

  return type;
}


/*
 * PICMAN_TYPE_PARAM_COLOR_ARRAY
 */

static void  picman_param_color_array_class_init (GParamSpecClass *klass);
static void  picman_param_color_array_init       (GParamSpec      *pspec);

GType
picman_param_color_array_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      const GTypeInfo info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_color_array_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecArray),
        0,
        (GInstanceInitFunc) picman_param_color_array_init
      };

      type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                     "PicmanParamColorArray", &info, 0);
    }

  return type;
}

static void
picman_param_color_array_class_init (GParamSpecClass *klass)
{
  klass->value_type = PICMAN_TYPE_COLOR_ARRAY;
}

static void
picman_param_color_array_init (GParamSpec *pspec)
{
}

GParamSpec *
picman_param_spec_color_array (const gchar *name,
                             const gchar *nick,
                             const gchar *blurb,
                             GParamFlags  flags)
{
  PicmanParamSpecColorArray *array_spec;

  array_spec = g_param_spec_internal (PICMAN_TYPE_PARAM_COLOR_ARRAY,
                                      name, nick, blurb, flags);

  return G_PARAM_SPEC (array_spec);
}

const PicmanRGB *
picman_value_get_colorarray (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_COLOR_ARRAY (value), NULL);

  return (const PicmanRGB *) picman_value_get_array (value);
}

PicmanRGB *
picman_value_dup_colorarray (const GValue *value)
{
  g_return_val_if_fail (PICMAN_VALUE_HOLDS_COLOR_ARRAY (value), NULL);

  return (PicmanRGB *) picman_value_dup_array (value);
}

void
picman_value_set_colorarray (GValue        *value,
                           const PicmanRGB *data,
                           gsize         length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_COLOR_ARRAY (value));

  picman_value_set_array (value, (const guint8 *) data,
                        length * sizeof (PicmanRGB));
}

void
picman_value_set_static_colorarray (GValue        *value,
                                  const PicmanRGB *data,
                                  gsize          length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_COLOR_ARRAY (value));

  picman_value_set_static_array (value, (const guint8 *) data,
                               length * sizeof (PicmanRGB));
}

void
picman_value_take_colorarray (GValue  *value,
                            PicmanRGB *data,
                            gsize    length)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_COLOR_ARRAY (value));

  picman_value_take_array (value, (guint8 *) data,
                         length * sizeof (PicmanRGB));
}
