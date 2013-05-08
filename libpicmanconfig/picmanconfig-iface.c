/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * Config file serialization and deserialization interface
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
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

#include <string.h>

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanconfigtypes.h"

#include "picmanconfigwriter.h"
#include "picmanconfig-iface.h"
#include "picmanconfig-deserialize.h"
#include "picmanconfig-serialize.h"
#include "picmanconfig-params.h"
#include "picmanconfig-utils.h"
#include "picmanscanner.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanconfig-iface
 * @title: PicmanConfig-iface
 * @short_description: High-level API for libpicmanconfig.
 *
 * High-level API for libpicmanconfig.
 **/


/*
 * The PicmanConfig serialization and deserialization interface.
 */

static void         picman_config_iface_base_init (PicmanConfigInterface  *config_iface);

static gboolean     picman_config_iface_serialize   (PicmanConfig       *config,
                                                   PicmanConfigWriter *writer,
                                                   gpointer          data);
static gboolean     picman_config_iface_deserialize (PicmanConfig       *config,
                                                   GScanner         *scanner,
                                                   gint              nest_level,
                                                   gpointer          data);
static PicmanConfig * picman_config_iface_duplicate   (PicmanConfig       *config);
static gboolean     picman_config_iface_equal       (PicmanConfig       *a,
                                                   PicmanConfig       *b);
static void         picman_config_iface_reset       (PicmanConfig       *config);
static gboolean     picman_config_iface_copy        (PicmanConfig       *src,
                                                   PicmanConfig       *dest,
                                                   GParamFlags       flags);


GType
picman_config_interface_get_type (void)
{
  static GType config_iface_type = 0;

  if (! config_iface_type)
    {
      const GTypeInfo config_iface_info =
      {
        sizeof (PicmanConfigInterface),
        (GBaseInitFunc)     picman_config_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      config_iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                                  "PicmanConfigInterface",
                                                  &config_iface_info,
                                                  0);

      g_type_interface_add_prerequisite (config_iface_type, G_TYPE_OBJECT);
    }

  return config_iface_type;
}

static void
picman_config_iface_base_init (PicmanConfigInterface *config_iface)
{
  if (! config_iface->serialize)
    {
      config_iface->serialize   = picman_config_iface_serialize;
      config_iface->deserialize = picman_config_iface_deserialize;
      config_iface->duplicate   = picman_config_iface_duplicate;
      config_iface->equal       = picman_config_iface_equal;
      config_iface->reset       = picman_config_iface_reset;
      config_iface->copy        = picman_config_iface_copy;
    }

  /*  always set these to NULL since we don't want to inherit them
   *  from parent classes
   */
  config_iface->serialize_property   = NULL;
  config_iface->deserialize_property = NULL;
}

static gboolean
picman_config_iface_serialize (PicmanConfig       *config,
                             PicmanConfigWriter *writer,
                             gpointer          data)
{
  return picman_config_serialize_properties (config, writer);
}

static gboolean
picman_config_iface_deserialize (PicmanConfig *config,
                               GScanner   *scanner,
                               gint        nest_level,
                               gpointer    data)
{
  return picman_config_deserialize_properties (config, scanner, nest_level);
}

static PicmanConfig *
picman_config_iface_duplicate (PicmanConfig *config)
{
  GObject       *object = G_OBJECT (config);
  GObjectClass  *klass  = G_OBJECT_GET_CLASS (object);
  GParamSpec   **property_specs;
  guint          n_property_specs;
  GParameter    *construct_params   = NULL;
  gint           n_construct_params = 0;
  guint          i;
  GObject       *dup;

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  construct_params = g_new0 (GParameter, n_property_specs);

  for (i = 0; i < n_property_specs; i++)
    {
      GParamSpec *prop_spec = property_specs[i];

      if ((prop_spec->flags & G_PARAM_READABLE) &&
          (prop_spec->flags & G_PARAM_WRITABLE) &&
          (prop_spec->flags & G_PARAM_CONSTRUCT_ONLY))
        {
          GParameter *construct_param;

          construct_param = &construct_params[n_construct_params++];

          construct_param->name = prop_spec->name;

          g_value_init (&construct_param->value, prop_spec->value_type);
          g_object_get_property (object,
                                 prop_spec->name, &construct_param->value);
        }
    }

  g_free (property_specs);

  dup = g_object_newv (G_TYPE_FROM_INSTANCE (object),
                       n_construct_params, construct_params);

  for (i = 0; i < n_construct_params; i++)
    g_value_unset (&construct_params[i].value);

  g_free (construct_params);

  picman_config_copy (config, PICMAN_CONFIG (dup), 0);

  return PICMAN_CONFIG (dup);
}

static gboolean
picman_config_iface_equal (PicmanConfig *a,
                         PicmanConfig *b)
{
  GObjectClass  *klass;
  GParamSpec   **property_specs;
  guint          n_property_specs;
  guint          i;
  gboolean       equal = TRUE;

  klass = G_OBJECT_GET_CLASS (a);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  for (i = 0; equal && i < n_property_specs; i++)
    {
      GParamSpec  *prop_spec;
      GValue       a_value = { 0, };
      GValue       b_value = { 0, };

      prop_spec = property_specs[i];

      if (! (prop_spec->flags & G_PARAM_READABLE))
        continue;

      g_value_init (&a_value, prop_spec->value_type);
      g_value_init (&b_value, prop_spec->value_type);
      g_object_get_property (G_OBJECT (a), prop_spec->name, &a_value);
      g_object_get_property (G_OBJECT (b), prop_spec->name, &b_value);

      if (g_param_values_cmp (prop_spec, &a_value, &b_value))
        {
          if ((prop_spec->flags & PICMAN_CONFIG_PARAM_AGGREGATE) &&
              G_IS_PARAM_SPEC_OBJECT (prop_spec)        &&
              g_type_interface_peek (g_type_class_peek (prop_spec->value_type),
                                     PICMAN_TYPE_CONFIG))
            {
              if (! picman_config_is_equal_to (g_value_get_object (&a_value),
                                             g_value_get_object (&b_value)))
                {
                  equal = FALSE;
                }
            }
          else
            {
              equal = FALSE;
            }
        }

      g_value_unset (&a_value);
      g_value_unset (&b_value);
    }

  g_free (property_specs);

  return equal;
}

static void
picman_config_iface_reset (PicmanConfig *config)
{
  picman_config_reset_properties (G_OBJECT (config));
}

static gboolean
picman_config_iface_copy (PicmanConfig  *src,
                        PicmanConfig  *dest,
                        GParamFlags  flags)
{
  return picman_config_sync (G_OBJECT (src), G_OBJECT (dest), flags);
}

/**
 * picman_config_serialize_to_file:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @filename: the name of the file to write the configuration to.
 * @header: optional file header (must be ASCII only)
 * @footer: optional file footer (must be ASCII only)
 * @data: user data passed to the serialize implementation.
 * @error: return location for a possible error
 *
 * Serializes the object properties of @config to the file specified
 * by @filename. If a file with that name already exists, it is
 * overwritten. Basically this function opens @filename for you and
 * calls the serialize function of the @config's #PicmanConfigInterface.
 *
 * Return value: %TRUE if serialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_serialize_to_file (PicmanConfig   *config,
                               const gchar  *filename,
                               const gchar  *header,
                               const gchar  *footer,
                               gpointer      data,
                               GError      **error)
{
  PicmanConfigWriter *writer;

  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  writer = picman_config_writer_new_file (filename, TRUE, header, error);
  if (!writer)
    return FALSE;

  PICMAN_CONFIG_GET_INTERFACE (config)->serialize (config, writer, data);

  return picman_config_writer_finish (writer, footer, error);
}

/**
 * picman_config_serialize_to_fd:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @fd: a file descriptor, opened for writing
 * @data: user data passed to the serialize implementation.
 *
 * Serializes the object properties of @config to the given file
 * descriptor.
 *
 * Return value: %TRUE if serialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_serialize_to_fd (PicmanConfig *config,
                             gint        fd,
                             gpointer    data)
{
  PicmanConfigWriter *writer;

  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);
  g_return_val_if_fail (fd > 0, FALSE);

  writer = picman_config_writer_new_fd (fd);
  if (!writer)
    return FALSE;

  PICMAN_CONFIG_GET_INTERFACE (config)->serialize (config, writer, data);

  return picman_config_writer_finish (writer, NULL, NULL);
}

/**
 * picman_config_serialize_to_string:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @data: user data passed to the serialize implementation.
 *
 * Serializes the object properties of @config to a string.
 *
 * Return value: a newly allocated NUL-terminated string.
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_config_serialize_to_string (PicmanConfig *config,
                                 gpointer    data)
{
  PicmanConfigWriter *writer;
  GString          *str;

  g_return_val_if_fail (PICMAN_IS_CONFIG (config), NULL);

  str = g_string_new (NULL);
  writer = picman_config_writer_new_string (str);

  PICMAN_CONFIG_GET_INTERFACE (config)->serialize (config, writer, data);

  picman_config_writer_finish (writer, NULL, NULL);

  return g_string_free (str, FALSE);
}

/**
 * picman_config_deserialize_file:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @filename: the name of the file to read configuration from.
 * @data: user data passed to the deserialize implementation.
 * @error: return location for a possible error
 *
 * Opens the file specified by @filename, reads configuration data
 * from it and configures @config accordingly. Basically this function
 * creates a properly configured #GScanner for you and calls the
 * deserialize function of the @config's #PicmanConfigInterface.
 *
 * Return value: %TRUE if deserialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_deserialize_file (PicmanConfig   *config,
                              const gchar  *filename,
                              gpointer      data,
                              GError      **error)
{
  GScanner *scanner;
  gboolean  success;

  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  scanner = picman_scanner_new_file (filename, error);
  if (! scanner)
    return FALSE;

  g_object_freeze_notify (G_OBJECT (config));

  success = PICMAN_CONFIG_GET_INTERFACE (config)->deserialize (config,
                                                             scanner, 0, data);

  g_object_thaw_notify (G_OBJECT (config));

  picman_scanner_destroy (scanner);

  if (! success)
    g_assert (error == NULL || *error != NULL);

  return success;
}

/**
 * picman_config_deserialize_string:
 * @config:   a #GObject that implements the #PicmanConfigInterface.
 * @text:     string to deserialize (in UTF-8 encoding)
 * @text_len: length of @text in bytes or -1
 * @data:     client data
 * @error:    return location for a possible error
 *
 * Configures @config from @text. Basically this function creates a
 * properly configured #GScanner for you and calls the deserialize
 * function of the @config's #PicmanConfigInterface.
 *
 * Returns: %TRUE if deserialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_deserialize_string (PicmanConfig      *config,
                                const gchar  *text,
                                gint          text_len,
                                gpointer      data,
                                GError      **error)
{
  GScanner *scanner;
  gboolean  success;

  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);
  g_return_val_if_fail (text != NULL || text_len == 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  scanner = picman_scanner_new_string (text, text_len, error);

  g_object_freeze_notify (G_OBJECT (config));

  success = PICMAN_CONFIG_GET_INTERFACE (config)->deserialize (config,
                                                             scanner, 0, data);

  g_object_thaw_notify (G_OBJECT (config));

  picman_scanner_destroy (scanner);

  if (! success)
    g_assert (error == NULL || *error != NULL);

  return success;
}

/**
 * picman_config_deserialize_return:
 * @scanner:        a #GScanner
 * @expected_token: the expected token
 * @nest_level:     the next level
 *
 * Returns:
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_deserialize_return (GScanner     *scanner,
                                GTokenType    expected_token,
                                gint          nest_level)
{
  GTokenType next_token;

  g_return_val_if_fail (scanner != NULL, FALSE);

  next_token = g_scanner_peek_next_token (scanner);

  if (expected_token != G_TOKEN_LEFT_PAREN)
    {
      g_scanner_get_next_token (scanner);
      g_scanner_unexp_token (scanner, expected_token, NULL, NULL, NULL,
                             _("fatal parse error"), TRUE);
      return FALSE;
    }
  else
    {
      if (nest_level > 0 && next_token == G_TOKEN_RIGHT_PAREN)
        {
          return TRUE;
        }
      else if (next_token != G_TOKEN_EOF)
        {
          g_scanner_get_next_token (scanner);
          g_scanner_unexp_token (scanner, expected_token, NULL, NULL, NULL,
                                 _("fatal parse error"), TRUE);
          return FALSE;
        }
    }

  return TRUE;
}


/**
 * picman_config_serialize:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @writer: the #PicmanConfigWriter to use.
 * @data: client data
 *
 * Serialize the #PicmanConfig object.
 *
 * Returns: %TRUE if serialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.8
 **/
gboolean
picman_config_serialize (PicmanConfig       *config,
                       PicmanConfigWriter *writer,
                       gpointer          data)
{
  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);

  return PICMAN_CONFIG_GET_INTERFACE (config)->serialize (config,
                                                        writer,
                                                        data);
}

/**
 * picman_config_deserialize:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 * @scanner: the #GScanner to use.
 * @nest_level: the nest level.
 * @data: client data.
 *
 * Deserialize the #PicmanConfig object.
 *
 * Returns: %TRUE if deserialization succeeded, %FALSE otherwise.
 *
 * Since: PICMAN 2.8
 **/
gboolean
picman_config_deserialize (PicmanConfig *config,
                         GScanner   *scanner,
                         gint        nest_level,
                         gpointer    data)
{
  g_return_val_if_fail (PICMAN_IS_CONFIG (config), FALSE);

  return PICMAN_CONFIG_GET_INTERFACE (config)->deserialize (config,
                                                          scanner,
                                                          nest_level,
                                                          data);
}

/**
 * picman_config_duplicate:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 *
 * Creates a copy of the passed object by copying all object
 * properties. The default implementation of the #PicmanConfigInterface
 * only works for objects that are completely defined by their
 * properties.
 *
 * Return value: the duplicated #PicmanConfig object
 *
 * Since: PICMAN 2.4
 **/
gpointer
picman_config_duplicate (PicmanConfig *config)
{
  g_return_val_if_fail (PICMAN_IS_CONFIG (config), NULL);

  return PICMAN_CONFIG_GET_INTERFACE (config)->duplicate (config);
}

/**
 * picman_config_is_equal_to:
 * @a: a #GObject that implements the #PicmanConfigInterface.
 * @b: another #GObject of the same type as @a.
 *
 * Compares the two objects. The default implementation of the
 * #PicmanConfigInterface compares the object properties and thus only
 * works for objects that are completely defined by their
 * properties.
 *
 * Return value: %TRUE if the two objects are equal.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_config_is_equal_to (PicmanConfig *a,
                         PicmanConfig *b)
{
  g_return_val_if_fail (PICMAN_IS_CONFIG (a), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONFIG (b), FALSE);
  g_return_val_if_fail (G_TYPE_FROM_INSTANCE (a) == G_TYPE_FROM_INSTANCE (b),
                        FALSE);

  return PICMAN_CONFIG_GET_INTERFACE (a)->equal (a, b);
}

/**
 * picman_config_reset:
 * @config: a #GObject that implements the #PicmanConfigInterface.
 *
 * Resets the object to its default state. The default implementation of the
 * #PicmanConfigInterface only works for objects that are completely defined by
 * their properties.
 *
 * Since: PICMAN 2.4
 **/
void
picman_config_reset (PicmanConfig *config)
{
  g_return_if_fail (PICMAN_IS_CONFIG (config));

  g_object_freeze_notify (G_OBJECT (config));

  PICMAN_CONFIG_GET_INTERFACE (config)->reset (config);

  g_object_thaw_notify (G_OBJECT (config));
}

/**
 * picman_config_copy:
 * @src: a #GObject that implements the #PicmanConfigInterface.
 * @dest: another #GObject of the same type as @a.
 * @flags: a mask of GParamFlags
 *
 * Compares all read- and write-able properties from @src and @dest
 * that have all @flags set. Differing values are then copied from
 * @src to @dest. If @flags is 0, all differing read/write properties.
 *
 * Properties marked as "construct-only" are not touched.
 *
 * Return value: %TRUE if @dest was modified, %FALSE otherwise
 *
 * Since: PICMAN 2.6
 **/
gboolean
picman_config_copy (PicmanConfig  *src,
                  PicmanConfig  *dest,
                  GParamFlags  flags)
{
  gboolean changed;

  g_return_val_if_fail (PICMAN_IS_CONFIG (src), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONFIG (dest), FALSE);
  g_return_val_if_fail (G_TYPE_FROM_INSTANCE (src) == G_TYPE_FROM_INSTANCE (dest),
                        FALSE);

  g_object_freeze_notify (G_OBJECT (dest));

  changed = PICMAN_CONFIG_GET_INTERFACE (src)->copy (src, dest, flags);

  g_object_thaw_notify (G_OBJECT (dest));

  return changed;
}
