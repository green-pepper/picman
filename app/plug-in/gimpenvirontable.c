/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanenvirontable.c
 * (C) 2002 Manish Singh <yosh@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "plug-in-types.h"

#include "picmanenvirontable.h"

#include "picman-intl.h"


typedef struct _PicmanEnvironValue PicmanEnvironValue;

struct _PicmanEnvironValue
{
  gchar *value;
  gchar *separator;
};


static void     picman_environ_table_finalize       (GObject               *object);

static void     picman_environ_table_load_env_file  (const PicmanDatafileData *file_data,
                                                   gpointer                user_data);
static gboolean picman_environ_table_legal_name     (gchar                 *name);

static void     picman_environ_table_populate       (PicmanEnvironTable      *environ_table);
static void     picman_environ_table_populate_one   (const gchar           *name,
                                                   PicmanEnvironValue      *val,
                                                   GPtrArray             *env_array);
static gboolean picman_environ_table_pass_through   (PicmanEnvironTable      *environ_table,
                                                   const gchar           *name);

static void     picman_environ_table_clear_vars     (PicmanEnvironTable      *environ_table);
static void     picman_environ_table_clear_internal (PicmanEnvironTable      *environ_table);
static void     picman_environ_table_clear_envp     (PicmanEnvironTable      *environ_table);

static void     picman_environ_table_free_value     (PicmanEnvironValue      *val);


G_DEFINE_TYPE (PicmanEnvironTable, picman_environ_table, G_TYPE_OBJECT)

#define parent_class picman_environ_table_parent_class


static void
picman_environ_table_class_init (PicmanEnvironTableClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = picman_environ_table_finalize;
}

static void
picman_environ_table_init (PicmanEnvironTable *environ_table)
{
  environ_table->vars     = NULL;
  environ_table->internal = NULL;

  environ_table->envp     = NULL;
}

static void
picman_environ_table_finalize (GObject *object)
{
  PicmanEnvironTable *environ_table = PICMAN_ENVIRON_TABLE (object);

  picman_environ_table_clear_all (environ_table);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanEnvironTable *
picman_environ_table_new (void)
{
  return g_object_new (PICMAN_TYPE_ENVIRON_TABLE, NULL);
}

static guint
picman_environ_table_str_hash (gconstpointer v)
{
#ifdef G_OS_WIN32
  gchar *p      = g_ascii_strup ((const gchar *) v, -1);
  guint  retval = g_str_hash (p);

  g_free (p);

  return retval;
#else
  return g_str_hash (v);
#endif
}

static gboolean
picman_environ_table_str_equal (gconstpointer v1,
                              gconstpointer v2)
{
#ifdef G_OS_WIN32
  gchar    *string1 = g_ascii_strup ((const gchar *) v1, -1);
  gchar    *string2 = g_ascii_strup ((const gchar *) v2, -1);
  gboolean  retval  = g_str_equal (string1, string2);

  g_free (string1);
  g_free (string2);

  return retval;
#else
  return g_str_equal (v1, v2);
#endif
}

void
picman_environ_table_load (PicmanEnvironTable *environ_table,
                         const gchar      *env_path)
{
  g_return_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table));

  picman_environ_table_clear (environ_table);

  environ_table->vars =
    g_hash_table_new_full (picman_environ_table_str_hash,
                           picman_environ_table_str_equal,
                           g_free,
                           (GDestroyNotify) picman_environ_table_free_value);

  picman_datafiles_read_directories (env_path,
                                   G_FILE_TEST_EXISTS,
                                   picman_environ_table_load_env_file,
                                   environ_table);
}

void
picman_environ_table_add (PicmanEnvironTable *environ_table,
                        const gchar      *name,
                        const gchar      *value,
                        const gchar      *separator)
{
  PicmanEnvironValue *val;

  g_return_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table));

  picman_environ_table_clear_envp (environ_table);

  if (! environ_table->internal)
    environ_table->internal =
      g_hash_table_new_full (g_str_hash, g_str_equal,
                             g_free,
                             (GDestroyNotify) picman_environ_table_free_value);

  val = g_slice_new (PicmanEnvironValue);

  val->value     = g_strdup (value);
  val->separator = g_strdup (separator);

  g_hash_table_insert (environ_table->internal, g_strdup (name), val);
}

void
picman_environ_table_remove (PicmanEnvironTable *environ_table,
                           const gchar      *name)
{
  g_return_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table));

  if (! environ_table->internal)
    return;

  picman_environ_table_clear_envp (environ_table);

  g_hash_table_remove (environ_table->internal, name);

  if (g_hash_table_size (environ_table->internal) == 0)
    picman_environ_table_clear_internal (environ_table);
}

void
picman_environ_table_clear (PicmanEnvironTable *environ_table)
{
  g_return_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table));

  picman_environ_table_clear_envp (environ_table);

  picman_environ_table_clear_vars (environ_table);
}

void
picman_environ_table_clear_all (PicmanEnvironTable *environ_table)
{
  g_return_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table));

  picman_environ_table_clear_envp (environ_table);

  picman_environ_table_clear_vars (environ_table);
  picman_environ_table_clear_internal (environ_table);
}

gchar **
picman_environ_table_get_envp (PicmanEnvironTable *environ_table)
{
  g_return_val_if_fail (PICMAN_IS_ENVIRON_TABLE (environ_table), NULL);

  /* Hmm.. should we return a copy here in the future? Not thread safe atm,
   * but the rest of it isn't either.
   */

  if (! environ_table->envp)
    picman_environ_table_populate (environ_table);

  return environ_table->envp;
}


/* private */

static void
picman_environ_table_load_env_file (const PicmanDatafileData *file_data,
                                  gpointer                user_data)
{
  PicmanEnvironTable *environ_table = PICMAN_ENVIRON_TABLE (user_data);
  FILE             *env;
  gchar             buffer[4096];
  gsize             len;
  gchar            *name, *value, *separator, *p, *q;
  PicmanEnvironValue *val;

  env = g_fopen (file_data->filename, "r");
  if (! env)
    return;

  while (fgets (buffer, sizeof (buffer), env))
    {
      /* Skip comments */
      if (buffer[0] == '#')
        continue;

      len = strlen (buffer) - 1;

      /* Skip too long lines */
      if (buffer[len] != '\n')
        continue;

      buffer[len] = '\0';

      p = strchr (buffer, '=');
      if (! p)
        continue;

      *p = '\0';

      name = buffer;
      value = p + 1;

      if (name[0] == '\0')
        {
          g_message (_("Empty variable name in environment file %s"),
                     picman_filename_to_utf8 (file_data->filename));
          continue;
        }

      separator = NULL;

      q = strchr (name, ' ');
      if (q)
        {
          *q = '\0';

          separator = name;
          name = q + 1;
        }

      if (! picman_environ_table_legal_name (name))
        {
          g_message (_("Illegal variable name in environment file %s: %s"),
                     picman_filename_to_utf8 (file_data->filename), name);
          continue;
        }

      if (! g_hash_table_lookup (environ_table->vars, name))
        {
          val = g_slice_new (PicmanEnvironValue);

          val->value     = picman_config_path_expand (value, FALSE, NULL);
          val->separator = g_strdup (separator);

          g_hash_table_insert (environ_table->vars, g_strdup (name), val);
        }
    }

  fclose (env);
}

static gboolean
picman_environ_table_legal_name (gchar *name)
{
  gchar *s;

  if (! g_ascii_isalpha (*name) && (*name != '_'))
    return FALSE;

  for (s = name + 1; *s; s++)
    if (! g_ascii_isalnum (*s) && (*s != '_'))
      return FALSE;

  return TRUE;
}

static void
picman_environ_table_populate (PicmanEnvironTable *environ_table)
{
  gchar     **env = g_listenv ();
  gchar     **var;
  GPtrArray  *env_array;

  var = env;
  env_array = g_ptr_array_new ();

  while (*var)
    {
      /* g_listenv() only returns the names of environment variables
       * that are correctly specified (name=value) in the environ
       * array (Unix) or the process environment string table (Win32).
       */

      if (picman_environ_table_pass_through (environ_table, *var))
	g_ptr_array_add (env_array, g_strconcat (*var, "=", g_getenv (*var), NULL));

      var++;
    }

  g_strfreev (env);

  if (environ_table->vars)
    g_hash_table_foreach (environ_table->vars,
                          (GHFunc) picman_environ_table_populate_one,
                          env_array);

  if (environ_table->internal)
    g_hash_table_foreach (environ_table->internal,
                          (GHFunc) picman_environ_table_populate_one,
                          env_array);

  g_ptr_array_add (env_array, NULL);

  environ_table->envp = (gchar **) g_ptr_array_free (env_array, FALSE);

#ifdef ENVP_DEBUG
  var = environ_table->envp;

  g_print ("PicmanEnvironTable:\n");
  while (*var)
    {
      g_print ("%s\n", *var);
      var++;
    }
#endif /* ENVP_DEBUG */
}

static void
picman_environ_table_populate_one (const gchar      *name,
                                 PicmanEnvironValue *val,
                                 GPtrArray        *env_array)
{
  const gchar *old;
  gchar *var = NULL;

  if (val->separator)
    {
      old = g_getenv (name);

      if (old)
        var = g_strconcat (name, "=", val->value, val->separator, old, NULL);
    }

  if (! var)
    var = g_strconcat (name, "=", val->value, NULL);

  g_ptr_array_add (env_array, var);
}

static gboolean
picman_environ_table_pass_through (PicmanEnvironTable *environ_table,
                                 const gchar      *name)
{
  gboolean vars, internal;

  vars = environ_table->vars &&
         g_hash_table_lookup (environ_table->vars, name);

  internal = environ_table->internal &&
             g_hash_table_lookup (environ_table->internal, name);

  return (!vars && !internal);
}

static void
picman_environ_table_clear_vars (PicmanEnvironTable *environ_table)
{
  if (environ_table->vars)
    {
      g_hash_table_destroy (environ_table->vars);
      environ_table->vars = NULL;
    }
}

static void
picman_environ_table_clear_internal (PicmanEnvironTable *environ_table)
{
  if (environ_table->internal)
    {
      g_hash_table_destroy (environ_table->internal);
      environ_table->internal = NULL;
    }
}

static void
picman_environ_table_clear_envp (PicmanEnvironTable *environ_table)
{
  if (environ_table->envp)
    {
      g_strfreev (environ_table->envp);
      environ_table->envp = NULL;
    }
}

static void
picman_environ_table_free_value (PicmanEnvironValue *val)
{
  g_free (val->value);
  g_free (val->separator);

  g_slice_free (PicmanEnvironValue, val);
}
