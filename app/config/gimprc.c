/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanRc, the object for PICMANs user configuration file picmanrc.
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "config-types.h"

#include "picmanconfig-file.h"
#include "picmanrc.h"
#include "picmanrc-deserialize.h"
#include "picmanrc-serialize.h"
#include "picmanrc-unknown.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_VERBOSE,
  PROP_SYSTEM_PICMANRC,
  PROP_USER_PICMANRC
};


static void         picman_rc_config_iface_init (PicmanConfigInterface *iface);

static void         picman_rc_dispose           (GObject      *object);
static void         picman_rc_finalize          (GObject      *object);
static void         picman_rc_set_property      (GObject      *object,
                                               guint         property_id,
                                               const GValue *value,
                                               GParamSpec   *pspec);
static void         picman_rc_get_property      (GObject      *object,
                                               guint         property_id,
                                               GValue       *value,
                                               GParamSpec   *pspec);

static PicmanConfig * picman_rc_duplicate         (PicmanConfig   *object);
static void         picman_rc_load              (PicmanRc       *rc);
static gboolean     picman_rc_idle_save         (PicmanRc       *rc);
static void         picman_rc_notify            (PicmanRc       *rc,
                                               GParamSpec   *param,
                                               gpointer      data);


G_DEFINE_TYPE_WITH_CODE (PicmanRc, picman_rc, PICMAN_TYPE_PLUGIN_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_rc_config_iface_init))

#define parent_class picman_rc_parent_class


static void
picman_rc_class_init (PicmanRcClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = picman_rc_dispose;
  object_class->finalize     = picman_rc_finalize;
  object_class->set_property = picman_rc_set_property;
  object_class->get_property = picman_rc_get_property;

  g_object_class_install_property (object_class, PROP_VERBOSE,
                                   g_param_spec_boolean ("verbose",
                                                         NULL, NULL,
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_SYSTEM_PICMANRC,
                                   g_param_spec_string ("system-picmanrc",
                                                        NULL, NULL,
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_USER_PICMANRC,
                                   g_param_spec_string ("user-picmanrc",
                                                        NULL, NULL,
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_rc_init (PicmanRc *rc)
{
  rc->autosave     = FALSE;
  rc->save_idle_id = 0;
}

static void
picman_rc_dispose (GObject *object)
{
  PicmanRc *rc = PICMAN_RC (object);

  if (rc->save_idle_id)
    picman_rc_idle_save (rc);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_rc_finalize (GObject *object)
{
  PicmanRc *rc = PICMAN_RC (object);

  if (rc->system_picmanrc)
    {
      g_free (rc->system_picmanrc);
      rc->system_picmanrc = NULL;
    }
  if (rc->user_picmanrc)
    {
      g_free (rc->user_picmanrc);
      rc->user_picmanrc = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_rc_set_property (GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  PicmanRc      *rc       = PICMAN_RC (object);
  const gchar *filename = NULL;

  switch (property_id)
    {
    case PROP_SYSTEM_PICMANRC:
    case PROP_USER_PICMANRC:
      filename = g_value_get_string (value);
      break;
    default:
      break;
    }

  switch (property_id)
    {
    case PROP_VERBOSE:
      rc->verbose = g_value_get_boolean (value);
      break;

    case PROP_SYSTEM_PICMANRC:
      g_free (rc->system_picmanrc);

      if (filename)
        rc->system_picmanrc = g_strdup (filename);
      else
        rc->system_picmanrc = g_build_filename (picman_sysconf_directory (),
                                              "picmanrc", NULL);
      break;

    case PROP_USER_PICMANRC:
      g_free (rc->user_picmanrc);

      if (filename)
        rc->user_picmanrc = g_strdup (filename);
      else
        rc->user_picmanrc = picman_personal_rc_file ("picmanrc");
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_rc_get_property (GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  PicmanRc *rc = PICMAN_RC (object);

  switch (property_id)
    {
    case PROP_VERBOSE:
      g_value_set_boolean (value, rc->verbose);
      break;
    case PROP_SYSTEM_PICMANRC:
      g_value_set_string (value, rc->system_picmanrc);
      break;
    case PROP_USER_PICMANRC:
      g_value_set_string (value, rc->user_picmanrc);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_rc_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_rc_serialize;
  iface->deserialize = picman_rc_deserialize;
  iface->duplicate   = picman_rc_duplicate;
}

static void
picman_rc_duplicate_unknown_token (const gchar *key,
                                 const gchar *value,
                                 gpointer     user_data)
{
  picman_rc_add_unknown_token (PICMAN_CONFIG (user_data), key, value);
}

static PicmanConfig *
picman_rc_duplicate (PicmanConfig *config)
{
  PicmanConfig *dup = g_object_new (PICMAN_TYPE_RC, NULL);

  picman_config_sync (G_OBJECT (config), G_OBJECT (dup), 0);

  picman_rc_foreach_unknown_token (config,
                                 picman_rc_duplicate_unknown_token, dup);

  return dup;
}

static void
picman_rc_load (PicmanRc *rc)
{
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_RC (rc));

  if (rc->verbose)
    g_print ("Parsing '%s'\n",
             picman_filename_to_utf8 (rc->system_picmanrc));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (rc),
                                      rc->system_picmanrc, NULL, &error))
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        g_message ("%s", error->message);

      g_clear_error (&error);
    }

  if (rc->verbose)
    g_print ("Parsing '%s'\n",
             picman_filename_to_utf8 (rc->user_picmanrc));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (rc),
                                      rc->user_picmanrc, NULL, &error))
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        {
          g_message ("%s", error->message);

          picman_config_file_backup_on_error (rc->user_picmanrc, "picmanrc", NULL);
        }

      g_clear_error (&error);
    }
}

static gboolean
picman_rc_idle_save (PicmanRc *rc)
{
  picman_rc_save (rc);

  rc->save_idle_id = 0;

  return FALSE;
}

static void
picman_rc_notify (PicmanRc     *rc,
                GParamSpec *param,
                gpointer    data)
{
  if (!rc->autosave)
    return;

  if (!rc->save_idle_id)
    rc->save_idle_id = g_idle_add ((GSourceFunc) picman_rc_idle_save, rc);
}

/**
 * picman_rc_new:
 * @system_picmanrc: the name of the system-wide picmanrc file or %NULL to
 *                 use the standard location
 * @user_picmanrc:   the name of the user picmanrc file or %NULL to use the
 *                 standard location
 * @verbose:       enable console messages about loading and saving
 *
 * Creates a new PicmanRc object and loads the system-wide and the user
 * configuration files.
 *
 * Returns: the new #PicmanRc.
 */
PicmanRc *
picman_rc_new (const gchar *system_picmanrc,
             const gchar *user_picmanrc,
             gboolean     verbose)
{
  PicmanRc *rc = g_object_new (PICMAN_TYPE_RC,
                             "verbose",       verbose,
                             "system-picmanrc", system_picmanrc,
                             "user-picmanrc",   user_picmanrc,
                             NULL);

  picman_rc_load (rc);

  return rc;
}

void
picman_rc_set_autosave (PicmanRc   *rc,
                      gboolean  autosave)
{
  g_return_if_fail (PICMAN_IS_RC (rc));

  autosave = autosave ? TRUE : FALSE;

  if (rc->autosave == autosave)
    return;

  if (autosave)
    g_signal_connect (rc, "notify",
                      G_CALLBACK (picman_rc_notify),
                      NULL);
  else
    g_signal_handlers_disconnect_by_func (rc, picman_rc_notify, NULL);

  rc->autosave = autosave;
}


/**
 * picman_rc_query:
 * @rc:  a #PicmanRc object.
 * @key: a string used as a key for the lookup.
 *
 * This function looks up @key in the object properties of @rc. If
 * there's a matching property, a string representation of its value
 * is returned. If no property is found, the list of unknown tokens
 * attached to the @rc object is searched.
 *
 * Return value: a newly allocated string representing the value or %NULL
 *               if the key couldn't be found.
 **/
gchar *
picman_rc_query (PicmanRc      *rc,
               const gchar *key)
{
  GObjectClass  *klass;
  GObject       *rc_object;
  GParamSpec   **property_specs;
  GParamSpec    *prop_spec;
  guint          i, n_property_specs;
  gchar         *retval = NULL;

  g_return_val_if_fail (PICMAN_IS_RC (rc), NULL);
  g_return_val_if_fail (key != NULL, NULL);

  rc_object = G_OBJECT (rc);
  klass = G_OBJECT_GET_CLASS (rc);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  if (!property_specs)
    return NULL;

  for (i = 0, prop_spec = NULL; i < n_property_specs && !prop_spec; i++)
    {
      prop_spec = property_specs[i];

      if (! (prop_spec->flags & PICMAN_CONFIG_PARAM_SERIALIZE) ||
          strcmp (prop_spec->name, key))
        {
          prop_spec = NULL;
        }
    }

  if (prop_spec)
    {
      GString *str   = g_string_new (NULL);
      GValue   value = { 0, };

      g_value_init (&value, prop_spec->value_type);
      g_object_get_property (rc_object, prop_spec->name, &value);

      if (picman_config_serialize_value (&value, str, FALSE))
        retval = g_string_free (str, FALSE);
      else
        g_string_free (str, TRUE);

      g_value_unset (&value);
    }
  else
    {
      retval = g_strdup (picman_rc_lookup_unknown_token (PICMAN_CONFIG (rc), key));
    }

  g_free (property_specs);

  if (!retval)
    {
      const gchar * const path_tokens[] =
      {
        "picman_dir",
        "picman_data_dir",
        "picman_plug_in_dir",
        "picman_plugin_dir",
        "picman_sysconf_dir"
      };

      for (i = 0; !retval && i < G_N_ELEMENTS (path_tokens); i++)
        if (strcmp (key, path_tokens[i]) == 0)
          retval = g_strdup_printf ("${%s}", path_tokens[i]);
    }

  if (retval)
    {
      gchar *tmp = picman_config_path_expand (retval, FALSE, NULL);

      if (tmp)
        {
          g_free (retval);
          retval = tmp;
        }
    }

  return retval;
}

/**
 * picman_rc_set_unknown_token:
 * @picmanrc: a #PicmanRc object.
 * @token:
 * @value:
 *
 * Calls picman_rc_add_unknown_token() and triggers an idle-save if
 * autosave is enabled on @picmanrc.
 **/
void
picman_rc_set_unknown_token (PicmanRc      *rc,
                           const gchar *token,
                           const gchar *value)
{
  g_return_if_fail (PICMAN_IS_RC (rc));

  picman_rc_add_unknown_token (PICMAN_CONFIG (rc), token, value);

  if (rc->autosave)
    picman_rc_notify (rc, NULL, NULL);
}

/**
 * picman_rc_save:
 * @picmanrc: a #PicmanRc object.
 *
 * Saves any settings that differ from the system-wide defined
 * defaults to the users personal picmanrc file.
 **/
void
picman_rc_save (PicmanRc *rc)
{
  PicmanRc *global;
  gchar  *header;
  GError *error = NULL;

  const gchar *top =
    "PICMAN picmanrc\n"
    "\n"
    "This is your personal picmanrc file.  Any variable defined in this file "
    "takes precedence over the value defined in the system-wide picmanrc: ";
  const gchar *bottom =
    "\n"
    "Most values can be set within PICMAN by changing some options in "
    "the Preferences dialog.";
  const gchar *footer =
    "end of picmanrc";

  g_return_if_fail (PICMAN_IS_RC (rc));

  global = g_object_new (PICMAN_TYPE_RC, NULL);

  picman_config_deserialize_file (PICMAN_CONFIG (global),
                                rc->system_picmanrc, NULL, NULL);

  header = g_strconcat (top, rc->system_picmanrc, bottom, NULL);

  if (rc->verbose)
    g_print ("Writing '%s'\n",
             picman_filename_to_utf8 (rc->user_picmanrc));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (rc),
                                       rc->user_picmanrc,
                                       header, footer, global,
                                       &error))
    {
      g_message ("%s", error->message);
      g_error_free (error);
    }

  g_free (header);
  g_object_unref (global);
}

/**
 * picman_rc_migrate:
 * @rc: a #PicmanRc object.
 *
 * Resets all PicmanParamConfigPath properties of the passed rc object
 * to their default values, in order to prevent paths in a migrated
 * picmanrc to refer to folders in the old PICMAN's user directory.
 **/
void
picman_rc_migrate (PicmanRc *rc)
{
  GParamSpec **pspecs;
  guint        n_pspecs;
  gint         i;

  g_return_if_fail (PICMAN_IS_RC (rc));

  pspecs = g_object_class_list_properties (G_OBJECT_GET_CLASS (rc), &n_pspecs);

  for (i = 0; i < n_pspecs; i++)
    {
      GParamSpec *pspec = pspecs[i];

      if (PICMAN_IS_PARAM_SPEC_CONFIG_PATH (pspec))
        {
          GValue value = { 0, };

          g_value_init (&value, pspec->value_type);

          g_param_value_set_default (pspec, &value);
          g_object_set_property (G_OBJECT (rc), pspec->name, &value);

          g_value_unset (&value);
        }
    }

  g_free (pspecs);
}
