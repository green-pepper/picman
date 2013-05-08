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

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "picman-gegl-types.h"

#include "core/picmanlist.h"
#include "core/picmanparamspecs-duplicate.h"

#include "picman-gegl-config-proxy.h"
#include "picman-gegl-utils.h"


static GHashTable *config_types      = NULL;
static GHashTable *config_containers = NULL;


static GValue *
picman_gegl_config_value_new (GParamSpec *pspec)
{
  GValue *value = g_slice_new0 (GValue);

  g_value_init (value, pspec->value_type);

  return value;
}

static void
picman_gegl_config_value_free (GValue *value)
{
  g_value_unset (value);
  g_slice_free (GValue, value);
}

static GHashTable *
picman_gegl_config_get_properties (GObject *object)
{
  GHashTable *properties = g_object_get_data (object, "properties");

  if (! properties)
    {
      properties = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          (GDestroyNotify) g_free,
                                          (GDestroyNotify) picman_gegl_config_value_free);

      g_object_set_data_full (object, "properties", properties,
                              (GDestroyNotify) g_hash_table_unref);
    }

  return properties;
}

static GValue *
picman_gegl_config_value_get (GObject    *object,
                            GParamSpec *pspec)
{
  GHashTable *properties = picman_gegl_config_get_properties (object);
  GValue     *value;

  value = g_hash_table_lookup (properties, pspec->name);

  if (! value)
    {
      value = picman_gegl_config_value_new (pspec);
      g_hash_table_insert (properties, g_strdup (pspec->name), value);
    }

  return value;
}

static void
picman_gegl_config_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GValue *val = picman_gegl_config_value_get (object, pspec);

  g_value_copy (value, val);
}

static void
picman_gegl_config_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GValue *val = picman_gegl_config_value_get (object, pspec);

  g_value_copy (val, value);
}

static void
picman_gegl_config_class_init (GObjectClass *klass,
                             const gchar  *operation)
{
  GParamSpec **pspecs;
  guint        n_pspecs;
  gint         i;

  klass->set_property = picman_gegl_config_set_property;
  klass->get_property = picman_gegl_config_get_property;

  pspecs = gegl_operation_list_properties (operation, &n_pspecs);

  for (i = 0; i < n_pspecs; i++)
    {
      GParamSpec *pspec = pspecs[i];

      if ((pspec->flags & G_PARAM_READABLE) &&
          (pspec->flags & G_PARAM_WRITABLE) &&
          strcmp (pspec->name, "input")     &&
          strcmp (pspec->name, "output"))
        {
          GParamSpec *copy = picman_param_spec_duplicate (pspec);

          if (copy)
            {
              g_object_class_install_property (klass, i + 1, copy);
            }
        }
    }

  g_free (pspecs);
}

static gboolean
picman_gegl_config_equal (PicmanConfig *a,
                        PicmanConfig *b)
{
  GList    *diff;
  gboolean  equal = TRUE;

  diff = picman_config_diff (G_OBJECT (a), G_OBJECT (b),
                           PICMAN_CONFIG_PARAM_SERIALIZE);

  if (G_TYPE_FROM_INSTANCE (a) == G_TYPE_FROM_INSTANCE (b))
    {
      GList *list;

      for (list = diff; list; list = g_list_next (list))
        {
          GParamSpec *pspec = list->data;

          if (pspec->owner_type == G_TYPE_FROM_INSTANCE (a))
            {
              equal = FALSE;
              break;
            }
        }
    }
  else if (diff)
    {
      equal = FALSE;
    }

  g_list_free (diff);

  return equal;
}

static void
picman_gegl_config_config_iface_init (PicmanConfigInterface *iface)
{
  iface->equal = picman_gegl_config_equal;
}

PicmanObject *
picman_gegl_get_config_proxy (const gchar *operation,
                            GType        parent_type)
{
  GType config_type;

  g_return_val_if_fail (operation != NULL, NULL);
  g_return_val_if_fail (g_type_is_a (parent_type, PICMAN_TYPE_OBJECT), NULL);

  if (! config_types)
    config_types = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          (GDestroyNotify) g_free,
                                          NULL);

  config_type = (GType) g_hash_table_lookup (config_types, operation);

  if (! config_type)
    {
      GTypeQuery query;

      g_type_query (parent_type, &query);

      {
        GTypeInfo info =
          {
            query.class_size,
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) picman_gegl_config_class_init,
            NULL,           /* class_finalize */
            operation,
            query.instance_size,
            0,              /* n_preallocs */
            (GInstanceInitFunc) NULL,
          };

        const GInterfaceInfo config_info =
          {
            (GInterfaceInitFunc) picman_gegl_config_config_iface_init,
            NULL, /* interface_finalize */
            NULL  /* interface_data     */
          };

        gchar *type_name = g_strdup_printf ("PicmanGegl-%s-config",
                                            operation);

        g_strcanon (type_name,
                    G_CSET_DIGITS "-" G_CSET_a_2_z G_CSET_A_2_Z, '-');

        config_type = g_type_register_static (parent_type, type_name,
                                              &info, 0);

        g_free (type_name);

        g_type_add_interface_static (config_type, PICMAN_TYPE_CONFIG,
                                     &config_info);

        g_hash_table_insert (config_types,
                             g_strdup (operation),
                             (gpointer) config_type);
      }
    }

  return g_object_new (config_type, NULL);
}

PicmanContainer *
picman_gegl_get_config_container (GType config_type)
{
  PicmanContainer *container;

  g_return_val_if_fail (g_type_is_a (config_type, PICMAN_TYPE_OBJECT), NULL);

  if (! config_containers)
    config_containers = g_hash_table_new_full (g_direct_hash,
                                               g_direct_equal,
                                               (GDestroyNotify) g_free,
                                               NULL);

  container = g_hash_table_lookup (config_containers, (gpointer) config_type);

  if (! container)
    {
      container = picman_list_new (config_type, TRUE);

      g_hash_table_insert (config_containers,
                           (gpointer) config_type, container);
    }

  return container;
}

void
picman_gegl_config_proxy_sync (PicmanObject  *proxy,
                             GeglNode    *node)
{
  GParamSpec **pspecs;
  gchar       *operation;
  guint        n_pspecs;
  gint         i;

  g_return_if_fail (PICMAN_IS_OBJECT (proxy));
  g_return_if_fail (GEGL_IS_NODE (node));

  gegl_node_get (node,
                 "operation", &operation,
                 NULL);

  g_return_if_fail (operation != NULL);

  pspecs = gegl_operation_list_properties (operation, &n_pspecs);
  g_free (operation);

  for (i = 0; i < n_pspecs; i++)
    {
      GParamSpec *gegl_pspec = pspecs[i];
      GParamSpec *picman_pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (proxy),
                                                             gegl_pspec->name);

      if (picman_pspec)
        {
          GValue value = { 0, };

          g_value_init (&value, picman_pspec->value_type);

          g_object_get_property (G_OBJECT (proxy), picman_pspec->name,
                                 &value);

          if (GEGL_IS_PARAM_SPEC_COLOR (gegl_pspec))
            {
              PicmanRGB    picman_color;
              GeglColor *gegl_color;

              picman_value_get_rgb (&value, &picman_color);
              g_value_unset (&value);

              gegl_color = picman_gegl_color_new (&picman_color);

              g_value_init (&value, gegl_pspec->value_type);
              g_value_take_object (&value, gegl_color);
            }

          gegl_node_set_property (node, gegl_pspec->name,
                                  &value);

          g_value_unset (&value);
        }
    }

  g_free (pspecs);
}
