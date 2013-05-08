/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanconfig-path.c
 * Copyright (C) 2001  Sven Neumann <sven@picman.org>
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

#include <stdio.h>
#include <string.h>

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanconfig-error.h"
#include "picmanconfig-path.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanconfig-path
 * @title: PicmanConfig-path
 * @short_description: File path utilities for libpicmanconfig.
 *
 * File path utilities for libpicmanconfig.
 **/


/**
 * picman_config_path_get_type:
 *
 * Reveals the object type
 *
 * Returns: the #GType for a PicmanConfigPath string property
 *
 * Since: PICMAN 2.4
 **/
GType
picman_config_path_get_type (void)
{
  static GType path_type = 0;

  if (! path_type)
    {
      const GTypeInfo type_info = { 0, };

      path_type = g_type_register_static (G_TYPE_STRING, "PicmanConfigPath",
                                          &type_info, 0);
    }

  return path_type;
}


/*
 * PICMAN_TYPE_PARAM_CONFIG_PATH
 */

#define PICMAN_PARAM_SPEC_CONFIG_PATH(pspec) (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_CONFIG_PATH, PicmanParamSpecConfigPath))

typedef struct _PicmanParamSpecConfigPath PicmanParamSpecConfigPath;

struct _PicmanParamSpecConfigPath
{
  GParamSpecString    parent_instance;

  PicmanConfigPathType  type;
};

static void  picman_param_config_path_class_init (GParamSpecClass *class);

/**
 * picman_param_config_path_get_type:
 *
 * Reveals the object type
 *
 * Returns: the #GType for a directory path object
 *
 * Since: PICMAN 2.4
 **/
GType
picman_param_config_path_get_type (void)
{
  static GType spec_type = 0;

  if (! spec_type)
    {
      const GTypeInfo type_info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_config_path_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecConfigPath),
        0, NULL, NULL
      };

      spec_type = g_type_register_static (G_TYPE_PARAM_STRING,
                                          "PicmanParamConfigPath",
                                          &type_info, 0);
    }

  return spec_type;
}

static void
picman_param_config_path_class_init (GParamSpecClass *class)
{
  class->value_type = PICMAN_TYPE_CONFIG_PATH;
}

/**
 * picman_param_spec_config_path:
 * @name:          Canonical name of the param
 * @nick:          Nickname of the param
 * @blurb:         Brief desciption of param.
 * @type:          a #PicmanConfigPathType value.
 * @default_value: Value to use if none is assigned.
 * @flags:         a combination of #GParamFlags
 *
 * Creates a param spec to hold a filename, dir name,
 * or list of file or dir names.
 * See g_param_spec_internal() for more information.
 *
 * Returns: a newly allocated #GParamSpec instance
 *
 * Since: PICMAN 2.4
 **/
GParamSpec *
picman_param_spec_config_path (const gchar        *name,
                             const gchar        *nick,
                             const gchar        *blurb,
                             PicmanConfigPathType  type,
                             const gchar        *default_value,
                             GParamFlags         flags)
{
  GParamSpecString *pspec;

  pspec = g_param_spec_internal (PICMAN_TYPE_PARAM_CONFIG_PATH,
                                 name, nick, blurb, flags);

  pspec->default_value = g_strdup (default_value);

  PICMAN_PARAM_SPEC_CONFIG_PATH (pspec)->type = type;

  return G_PARAM_SPEC (pspec);
}

/**
 * picman_param_spec_config_path_type:
 * @pspec:         A #GParamSpec for a path param
 *
 * Tells whether the path param encodes a filename,
 * dir name, or list of file or dir names.
 *
 * Returns: a #PicmanConfigPathType value
 *
 * Since: PICMAN 2.4
 **/
PicmanConfigPathType
picman_param_spec_config_path_type (GParamSpec *pspec)
{
  g_return_val_if_fail (PICMAN_IS_PARAM_SPEC_CONFIG_PATH (pspec), 0);

  return PICMAN_PARAM_SPEC_CONFIG_PATH (pspec)->type;
}


/*
 * PicmanConfig path utilities
 */

static gchar        * picman_config_path_expand_only   (const gchar  *path,
                                                      GError      **error) G_GNUC_MALLOC;
static inline gchar * picman_config_path_extract_token (const gchar **str);


/**
 * picman_config_build_data_path:
 * @name: directory name (in UTF-8 encoding)
 *
 * Creates a search path as it is used in the picmanrc file.  The path
 * returned by picman_config_build_data_path() includes a directory
 * below the user's picman directory and one in the system-wide data
 * directory.
 *
 * Note that you cannot use this path directly with picman_path_parse().
 * As it is in the picmanrc notation, you first need to expand and
 * recode it using picman_config_path_expand().
 *
 * Returns: a newly allocated string
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_config_build_data_path (const gchar *name)
{
  return g_strconcat ("${picman_dir}", G_DIR_SEPARATOR_S, name,
                      G_SEARCHPATH_SEPARATOR_S,
                      "${picman_data_dir}", G_DIR_SEPARATOR_S, name,
                      NULL);
}

/**
 * picman_config_build_plug_in_path:
 * @name: directory name (in UTF-8 encoding)
 *
 * Creates a search path as it is used in the picmanrc file.  The path
 * returned by picman_config_build_plug_in_path() includes a directory
 * below the user's picman directory and one in the system-wide plug-in
 * directory.
 *
 * Note that you cannot use this path directly with picman_path_parse().
 * As it is in the picmanrc notation, you first need to expand and
 * recode it using picman_config_path_expand().
 *
 * Returns: a newly allocated string
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_config_build_plug_in_path (const gchar *name)
{
  return g_strconcat ("${picman_dir}", G_DIR_SEPARATOR_S, name,
                      G_SEARCHPATH_SEPARATOR_S,
                      "${picman_plug_in_dir}", G_DIR_SEPARATOR_S, name,
                      NULL);
}

/**
 * picman_config_build_writable_path:
 * @name: directory name (in UTF-8 encoding)
 *
 * Creates a search path as it is used in the picmanrc file.  The path
 * returned by picman_config_build_writable_path() is just the writable
 * parts of the search path constructed by picman_config_build_data_path().
 *
 * Note that you cannot use this path directly with picman_path_parse().
 * As it is in the picmanrc notation, you first need to expand and
 * recode it using picman_config_path_expand().
 *
 * Returns: a newly allocated string
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_config_build_writable_path (const gchar *name)
{
  return g_strconcat ("${picman_dir}", G_DIR_SEPARATOR_S, name, NULL);
}


/**
 * picman_config_path_expand:
 * @path: a NUL-terminated string in UTF-8 encoding
 * @recode: whether to convert to the filesystem's encoding
 * @error: return location for errors
 *
 * Paths as stored in the picmanrc have to be treated special.  The
 * string may contain special identifiers such as for example
 * ${picman_dir} that have to be substituted before use. Also the user's
 * filesystem may be in a different encoding than UTF-8 (which is what
 * is used for the picmanrc). This function does the variable
 * substitution for you and can also attempt to convert to the
 * filesystem encoding.
 *
 * Return value: a newly allocated NUL-terminated string
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_config_path_expand (const gchar  *path,
                         gboolean      recode,
                         GError      **error)
{
  g_return_val_if_fail (path != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (recode)
    {
      gchar *retval;
      gchar *expanded = picman_config_path_expand_only (path, error);

      if (! expanded)
        return NULL;

      retval = g_filename_from_utf8 (expanded, -1, NULL, NULL, error);

      g_free (expanded);

      return retval;
    }

  return picman_config_path_expand_only (path, error);
}


#define SUBSTS_ALLOC 4

static gchar *
picman_config_path_expand_only (const gchar  *path,
                              GError      **error)
{
  const gchar *home;
  const gchar *p;
  const gchar *s;
  gchar       *n;
  gchar       *token;
  gchar       *filename = NULL;
  gchar       *expanded = NULL;
  gchar      **substs   = NULL;
  guint        n_substs = 0;
  gint         length   = 0;
  gint         i;

  home = g_get_home_dir ();
  if (home)
    home = picman_filename_to_utf8 (home);

  p = path;

  while (*p)
    {
      if (*p == '~' && home)
        {
          length += strlen (home);
          p += 1;
        }
      else if ((token = picman_config_path_extract_token (&p)) != NULL)
        {
          for (i = 0; i < n_substs; i++)
            if (strcmp (substs[2*i], token) == 0)
              break;

          if (i < n_substs)
            {
              s = substs[2*i+1];
            }
          else
            {
              s = NULL;

              if (strcmp (token, "picman_dir") == 0)
                s = picman_directory ();
              else if (strcmp (token, "picman_data_dir") == 0)
                s = picman_data_directory ();
              else if (strcmp (token, "picman_plug_in_dir") == 0 ||
                       strcmp (token, "picman_plugin_dir") == 0)
                s = picman_plug_in_directory ();
              else if (strcmp (token, "picman_sysconf_dir") == 0)
                s = picman_sysconf_directory ();
              else if (strcmp (token, "picman_installation_dir") == 0)
                s = picman_installation_directory ();

              if (!s)
                s = g_getenv (token);

#ifdef G_OS_WIN32
              /* The default user picmanrc on Windows references
               * ${TEMP}, but not all Windows installations have that
               * environment variable, even if it should be kinda
               * standard. So special-case it.
               */
              if (!s && strcmp (token, "TEMP") == 0)
                s = g_get_tmp_dir ();
#endif  /* G_OS_WIN32 */
            }

          if (!s)
            {
              g_set_error (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_PARSE,
			   _("Cannot expand ${%s}"), token);
              g_free (token);
              goto cleanup;
            }

          if (n_substs % SUBSTS_ALLOC == 0)
            substs = g_renew (gchar *, substs, 2 * (n_substs + SUBSTS_ALLOC));

          substs[2*n_substs]     = token;
          substs[2*n_substs + 1] = (gchar *) picman_filename_to_utf8 (s);

          length += strlen (substs[2*n_substs + 1]);

          n_substs++;
        }
      else
        {
          length += g_utf8_skip[(const guchar) *p];
          p = g_utf8_next_char (p);
        }
    }

  if (n_substs == 0)
    return g_strdup (path);

  expanded = g_new (gchar, length + 1);

  p = path;
  n = expanded;

  while (*p)
    {
      if (*p == '~' && home)
        {
          *n = '\0';
          strcat (n, home);
          n += strlen (home);
          p += 1;
        }
      else if ((token = picman_config_path_extract_token (&p)) != NULL)
        {
          for (i = 0; i < n_substs; i++)
            {
              if (strcmp (substs[2*i], token) == 0)
                {
                  s = substs[2*i+1];

                  *n = '\0';
                  strcat (n, s);
                  n += strlen (s);

                  break;
                }
            }

          g_free (token);
        }
      else
        {
          *n++ = *p++;
        }
    }

  *n = '\0';

 cleanup:
  for (i = 0; i < n_substs; i++)
    g_free (substs[2*i]);

  g_free (substs);
  g_free (filename);

  return expanded;
}

static inline gchar *
picman_config_path_extract_token (const gchar **str)
{
  const gchar *p;
  gchar       *token;

  if (strncmp (*str, "${", 2))
    return NULL;

  p = *str + 2;

  while (*p && (*p != '}'))
    p = g_utf8_next_char (p);

  if (!p)
    return NULL;

  token = g_strndup (*str + 2, g_utf8_pointer_to_offset (*str + 2, p));

  *str = p + 1; /* after the closing bracket */

  return token;
}
