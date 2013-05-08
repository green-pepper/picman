/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-user-install.c
 * Copyright (C) 2000-2008 Michael Natterer and Sven Neumann
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

/* This file contains functions to help migrate the settings from a
 * previous PICMAN version to be used with the current (newer) version.
 */

#include "config.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib/gstdio.h>

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <libpicmanbase/picmanwin32-io.h>
#endif

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "config/picmanconfig-file.h"
#include "config/picmanrc.h"

#include "picman-templates.h"
#include "picman-tags.h"
#include "picman-user-install.h"

#include "picman-intl.h"


struct _PicmanUserInstall
{
  gboolean                verbose;

  gchar                  *old_dir;
  gint                    old_major;
  gint                    old_minor;

  const gchar            *migrate;

  PicmanUserInstallLogFunc  log;
  gpointer                log_data;
};

typedef enum
{
  USER_INSTALL_MKDIR, /* Create the directory        */
  USER_INSTALL_COPY   /* Copy from sysconf directory */
} PicmanUserInstallAction;

static const struct
{
  const gchar           *name;
  PicmanUserInstallAction  action;
}
picman_user_install_items[] =
{
  { "gtkrc",           USER_INSTALL_COPY  },
  { "menurc",          USER_INSTALL_COPY  },
  { "brushes",         USER_INSTALL_MKDIR },
  { "dynamics",        USER_INSTALL_MKDIR },
  { "fonts",           USER_INSTALL_MKDIR },
  { "gradients",       USER_INSTALL_MKDIR },
  { "palettes",        USER_INSTALL_MKDIR },
  { "patterns",        USER_INSTALL_MKDIR },
  { "tool-presets",    USER_INSTALL_MKDIR },
  { "plug-ins",        USER_INSTALL_MKDIR },
  { "modules",         USER_INSTALL_MKDIR },
  { "interpreters",    USER_INSTALL_MKDIR },
  { "environ",         USER_INSTALL_MKDIR },
  { "scripts",         USER_INSTALL_MKDIR },
  { "templates",       USER_INSTALL_MKDIR },
  { "themes",          USER_INSTALL_MKDIR },
  { "tmp",             USER_INSTALL_MKDIR },
  { "curves",          USER_INSTALL_MKDIR },
  { "levels",          USER_INSTALL_MKDIR },
  { "filters",         USER_INSTALL_MKDIR },
  { "fractalexplorer", USER_INSTALL_MKDIR },
  { "gfig",            USER_INSTALL_MKDIR },
  { "gflare",          USER_INSTALL_MKDIR },
  { "picmanressionist",  USER_INSTALL_MKDIR }
};


static gboolean  user_install_detect_old         (PicmanUserInstall    *install,
                                                  const gchar        *picman_dir);
static gchar *   user_install_old_style_picmandir  (void);

static void      user_install_log                (PicmanUserInstall    *install,
                                                  const gchar        *format,
                                                  ...) G_GNUC_PRINTF (2, 3);
static void      user_install_log_newline        (PicmanUserInstall    *install);
static void      user_install_log_error          (PicmanUserInstall    *install,
                                                  GError            **error);

static gboolean  user_install_mkdir              (PicmanUserInstall    *install,
                                                  const gchar        *dirname);
static gboolean  user_install_mkdir_with_parents (PicmanUserInstall    *install,
                                                  const gchar        *dirname);
static gboolean  user_install_file_copy          (PicmanUserInstall    *install,
                                                  const gchar        *source,
                                                  const gchar        *dest,
                                                  const gchar        *old_options_regexp,
                                                  GRegexEvalCallback  update_callback);
static gboolean  user_install_dir_copy           (PicmanUserInstall    *install,
                                                  const gchar        *source,
                                                  const gchar        *base);

static gboolean  user_install_create_files       (PicmanUserInstall    *install);
static gboolean  user_install_migrate_files      (PicmanUserInstall    *install);


/*  public functions  */

PicmanUserInstall *
picman_user_install_new (gboolean verbose)
{
  PicmanUserInstall *install = g_slice_new0 (PicmanUserInstall);

  install->verbose = verbose;

  user_install_detect_old (install, picman_directory ());

  if (! install->old_dir)
    {
      /* if the default XDG-style config directory was not found, try
       * the "old-style" path in the home folder.
       */
      gchar *dir = user_install_old_style_picmandir ();
      user_install_detect_old (install, dir);
      g_free (dir);
    }

  return install;
}

gboolean
picman_user_install_run (PicmanUserInstall *install)
{
  gchar *dirname;

  g_return_val_if_fail (install != NULL, FALSE);

  dirname = g_filename_display_name (picman_directory ());

  if (install->migrate)
    user_install_log (install,
		      _("It seems you have used PICMAN %s before.  "
			"PICMAN will now migrate your user settings to '%s'."),
		      install->migrate, dirname);
  else
    user_install_log (install,
		      _("It appears that you are using PICMAN for the "
			"first time.  PICMAN will now create a folder "
			"named '%s' and copy some files to it."),
			dirname);

  g_free (dirname);

  user_install_log_newline (install);

  if (! user_install_mkdir_with_parents (install, picman_directory ()))
    return FALSE;

  if (install->migrate)
    if (! user_install_migrate_files (install))
      return FALSE;

  return user_install_create_files (install);
}

void
picman_user_install_free (PicmanUserInstall *install)
{
  g_return_if_fail (install != NULL);

  g_free (install->old_dir);

  g_slice_free (PicmanUserInstall, install);
}

void
picman_user_install_set_log_handler (PicmanUserInstall        *install,
                                   PicmanUserInstallLogFunc  log,
                                   gpointer                user_data)
{
  g_return_if_fail (install != NULL);

  install->log      = log;
  install->log_data = user_data;
}


/*  Local functions  */

static gboolean
user_install_detect_old (PicmanUserInstall *install,
                         const gchar     *picman_dir)
{
  gchar    *dir     = g_strdup (picman_dir);
  gchar    *version;
  gboolean  migrate = FALSE;

  version = strstr (dir, PICMAN_APP_VERSION);

  if (version)
    {
      gint i;

      for (i = (PICMAN_MINOR_VERSION & ~1); i >= 0; i -= 2)
        {
          /*  we assume that PICMAN_APP_VERSION is in the form '2.x'  */
          g_snprintf (version + 2, 2, "%d", i);

          migrate = g_file_test (dir, G_FILE_TEST_IS_DIR);

          if (migrate)
            {
#ifdef PICMAN_UNSTABLE
	      g_printerr ("picman-user-install: migrating from %s\n", dir);
#endif
              install->old_major = 2;
              install->old_minor = i;

              break;
            }
        }
    }

  if (migrate)
    {
      install->old_dir = dir;
      install->migrate = (const gchar *) version;
    }
  else
    {
      g_free (dir);
    }

  return migrate;
}

static gchar *
user_install_old_style_picmandir (void)
{
  const gchar *home_dir = g_get_home_dir ();
  gchar       *picman_dir = NULL;

  if (home_dir)
    {
      picman_dir = g_build_filename (home_dir, ".picman-" PICMAN_APP_VERSION, NULL);
    }
  else
    {
      gchar *user_name = g_strdup (g_get_user_name ());
      gchar *subdir_name;

#ifdef G_OS_WIN32
      gchar *p = user_name;

      while (*p)
        {
          /* Replace funny characters in the user name with an
           * underscore. The code below also replaces some
           * characters that in fact are legal in file names, but
           * who cares, as long as the definitely illegal ones are
           * caught.
           */
          if (!g_ascii_isalnum (*p) && !strchr ("-.,@=", *p))
            *p = '_';
          p++;
        }
#endif

#ifndef G_OS_WIN32
      g_message ("warning: no home directory.");
#endif
      subdir_name = g_strconcat (".picman-" PICMAN_APP_VERSION ".", user_name, NULL);
      picman_dir = g_build_filename (picman_data_directory (),
                                   subdir_name,
                                   NULL);
      g_free (user_name);
      g_free (subdir_name);
    }

  return picman_dir;
}

static void
user_install_log (PicmanUserInstall *install,
                  const gchar     *format,
                  ...)
{
  va_list args;

  va_start (args, format);

  if (format)
    {
      gchar *message = g_strdup_vprintf (format, args);

      if (install->verbose)
        g_print ("%s\n", message);

      if (install->log)
        install->log (message, FALSE, install->log_data);

      g_free (message);
    }

  va_end (args);
}

static void
user_install_log_newline (PicmanUserInstall *install)
{
  if (install->verbose)
    g_print ("\n");

  if (install->log)
    install->log (NULL, FALSE, install->log_data);
}

static void
user_install_log_error (PicmanUserInstall  *install,
                        GError          **error)
{
  if (error && *error)
    {
      const gchar *message = ((*error)->message ?
                              (*error)->message : "(unknown error)");

      if (install->log)
        install->log (message, TRUE, install->log_data);
      else
        g_print ("error: %s\n", message);

      g_clear_error (error);
    }
}

static gboolean
user_install_file_copy (PicmanUserInstall    *install,
                        const gchar        *source,
                        const gchar        *dest,
                        const gchar        *old_options_regexp,
                        GRegexEvalCallback  update_callback)
{
  GError   *error = NULL;
  gboolean  success;

  user_install_log (install, _("Copying file '%s' from '%s'..."),
                    picman_filename_to_utf8 (dest),
                    picman_filename_to_utf8 (source));

  success = picman_config_file_copy (source, dest, old_options_regexp, update_callback, &error);

  user_install_log_error (install, &error);

  return success;
}

static gboolean
user_install_mkdir (PicmanUserInstall *install,
                    const gchar     *dirname)
{
  user_install_log (install, _("Creating folder '%s'..."),
                    picman_filename_to_utf8 (dirname));

  if (g_mkdir (dirname,
               S_IRUSR | S_IWUSR | S_IXUSR |
               S_IRGRP | S_IXGRP |
               S_IROTH | S_IXOTH) == -1)
    {
      GError *error = NULL;

      g_set_error (&error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Cannot create folder '%s': %s"),
                   picman_filename_to_utf8 (dirname), g_strerror (errno));

      user_install_log_error (install, &error);

      return FALSE;
    }

  return TRUE;
}

static gboolean
user_install_mkdir_with_parents (PicmanUserInstall *install,
                                 const gchar     *dirname)
{
  user_install_log (install, _("Creating folder '%s'..."),
                    picman_filename_to_utf8 (dirname));

  if (g_mkdir_with_parents (dirname,
                            S_IRUSR | S_IWUSR | S_IXUSR |
                            S_IRGRP | S_IXGRP |
                            S_IROTH | S_IXOTH) == -1)
    {
      GError *error = NULL;

      g_set_error (&error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Cannot create folder '%s': %s"),
                   picman_filename_to_utf8 (dirname), g_strerror (errno));

      user_install_log_error (install, &error);

      return FALSE;
    }

  return TRUE;
}

/* The regexp pattern of all options changed from previous menurc.
 * Add any pattern that we want to recognize for replacement in the menurc of
 * the next release*/
#define MENURC_OVER20_UPDATE_PATTERN "NOMATCH^"

/**
 * callback to use for updating any change value in the menurc.
 * data is unused (always NULL).
 * The updated value will be matched line by line.
 */
static gboolean
user_update_menurc_over20 (const GMatchInfo *matched_value,
                           GString          *new_value,
                           gpointer          data)
{
  gchar *match;
  match = g_match_info_fetch (matched_value, 0);

  /* This is an example of how to use it.
   * If view-close were to be renamed to file-close for instance, we'd add:

  if (strcmp (match, "\"<Actions>/view/view-close\"") == 0)
    g_string_append (new_value, "\"<Actions>/file/file-close\"");
  else
  */
  /* Should not happen. Just in case we match something unexpected by mistake. */
  g_string_append (new_value, match);

  g_free (match);
  return FALSE;
}

static gboolean
user_install_dir_copy (PicmanUserInstall *install,
                       const gchar     *source,
                       const gchar     *base)
{
  GDir        *source_dir = NULL;
  GDir        *dest_dir   = NULL;
  gchar        dest[1024];
  const gchar *basename;
  gchar       *dirname;
  gboolean     success;
  GError      *error = NULL;

  {
    gchar *basename = g_path_get_basename (source);

    dirname = g_build_filename (base, basename, NULL);
    g_free (basename);
  }

  success = user_install_mkdir (install, dirname);
  if (! success)
    goto error;

  success = (dest_dir = g_dir_open (dirname, 0, &error)) != NULL;
  if (! success)
    goto error;

  success = (source_dir = g_dir_open (source, 0, &error)) != NULL;
  if (! success)
    goto error;

  while ((basename = g_dir_read_name (source_dir)) != NULL)
    {
      gchar *name = g_build_filename (source, basename, NULL);

      if (g_file_test (name, G_FILE_TEST_IS_REGULAR))
        {
          g_snprintf (dest, sizeof (dest), "%s%c%s",
                      dirname, G_DIR_SEPARATOR, basename);

          if (! user_install_file_copy (install, name, dest, NULL, NULL))
            {
              g_free (name);
              goto error;
            }
        }

      g_free (name);
    }

 error:
  user_install_log_error (install, &error);

  if (source_dir)
    g_dir_close (source_dir);

  if (dest_dir)
    g_dir_close (dest_dir);

  g_free (dirname);

  return success;
}

static gboolean
user_install_create_files (PicmanUserInstall *install)
{
  gchar dest[1024];
  gchar source[1024];
  gint  i;

  for (i = 0; i < G_N_ELEMENTS (picman_user_install_items); i++)
    {
      g_snprintf (dest, sizeof (dest), "%s%c%s",
                  picman_directory (),
                  G_DIR_SEPARATOR,
                  picman_user_install_items[i].name);

      if (g_file_test (dest, G_FILE_TEST_EXISTS))
        continue;

      switch (picman_user_install_items[i].action)
        {
        case USER_INSTALL_MKDIR:
          if (! user_install_mkdir (install, dest))
            return FALSE;
          break;

        case USER_INSTALL_COPY:
          g_snprintf (source, sizeof (source), "%s%c%s",
                      picman_sysconf_directory (), G_DIR_SEPARATOR,
                      picman_user_install_items[i].name);

          if (! user_install_file_copy (install, source, dest, NULL, NULL))
            return FALSE;
          break;
        }
    }

  g_snprintf (dest, sizeof (dest), "%s%c%s",
              picman_directory (), G_DIR_SEPARATOR, "tags.xml");

  if (! g_file_test (dest, G_FILE_TEST_IS_REGULAR))
    {
      /* if there was no tags.xml, install it with default tag set.
       */
      if (! picman_tags_user_install ())
        {
          return FALSE;
        }
    }

  return TRUE;
}

static gboolean
user_install_migrate_files (PicmanUserInstall *install)
{
  GDir        *dir;
  const gchar *basename;
  gchar        dest[1024];
  PicmanRc      *picmanrc;
  GError      *error = NULL;

  dir = g_dir_open (install->old_dir, 0, &error);

  if (! dir)
    {
      user_install_log_error (install, &error);
      return FALSE;
    }

  while ((basename = g_dir_read_name (dir)) != NULL)
    {
      gchar *source = g_build_filename (install->old_dir, basename, NULL);
      const gchar* update_pattern = NULL;
      GRegexEvalCallback update_callback = NULL;

      if (g_file_test (source, G_FILE_TEST_IS_REGULAR))
        {
          /*  skip these files for all old versions  */
          if (strcmp (basename, "documents") == 0      ||
              g_str_has_prefix (basename, "picmanswap.") ||
              strcmp (basename, "pluginrc") == 0       ||
              strcmp (basename, "themerc") == 0        ||
              strcmp (basename, "toolrc") == 0)
            {
              goto next_file;
            }

          if (strcmp (basename, "menurc") == 0)
            {
              /*  skip menurc for picman 2.0 as the format has changed  */
              if (install->old_minor == 0)
                goto next_file;
              update_pattern = MENURC_OVER20_UPDATE_PATTERN;
              update_callback = user_update_menurc_over20;
            }

          g_snprintf (dest, sizeof (dest), "%s%c%s",
                      picman_directory (), G_DIR_SEPARATOR, basename);

          user_install_file_copy (install, source, dest, update_pattern, update_callback);
        }
      else if (g_file_test (source, G_FILE_TEST_IS_DIR))
        {
          /*  skip these directories for all old versions  */
          if (strcmp (basename, "tmp") == 0 ||
              strcmp (basename, "tool-options") == 0)
            {
              goto next_file;
            }

          user_install_dir_copy (install, source, picman_directory ());
        }

    next_file:
      g_free (source);
    }

  /*  create the tmp directory that was explicitly not copied  */

  g_snprintf (dest, sizeof (dest), "%s%c%s",
              picman_directory (), G_DIR_SEPARATOR, "tmp");

  user_install_mkdir (install, dest);
  g_dir_close (dir);

  picman_templates_migrate (install->old_dir);

  picmanrc = picman_rc_new (NULL, NULL, FALSE);
  picman_rc_migrate (picmanrc);
  picman_rc_save (picmanrc);
  g_object_unref (picmanrc);

  return TRUE;
}
