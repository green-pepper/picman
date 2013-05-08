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

#include <errno.h>
#include <stdarg.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "gui-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"

#include "themes.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   themes_apply_theme         (Picman                   *picman,
                                          const gchar            *theme_name);
static void   themes_directories_foreach (const PicmanDatafileData *file_data,
                                          gpointer                user_data);
static void   themes_list_themes_foreach (gpointer                key,
                                          gpointer                value,
                                          gpointer                data);
static void   themes_theme_change_notify (PicmanGuiConfig          *config,
                                          GParamSpec             *pspec,
                                          Picman                   *picman);


/*  private variables  */

static GHashTable *themes_hash = NULL;


/*  public functions  */

void
themes_init (Picman *picman)
{
  PicmanGuiConfig *config;
  gchar         *themerc;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  config = PICMAN_GUI_CONFIG (picman->config);

  themes_hash = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       g_free);

  if (config->theme_path)
    {
      gchar *path;

      path = picman_config_path_expand (config->theme_path, TRUE, NULL);

      picman_datafiles_read_directories (path,
                                       G_FILE_TEST_IS_DIR,
                                       themes_directories_foreach,
                                       picman);

      g_free (path);
    }

  themes_apply_theme (picman, config->theme);

  themerc = picman_personal_rc_file ("themerc");
  gtk_rc_parse (themerc);
  g_free (themerc);

  g_signal_connect (config, "notify::theme",
                    G_CALLBACK (themes_theme_change_notify),
                    picman);
}

void
themes_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (themes_hash)
    {
      g_signal_handlers_disconnect_by_func (picman->config,
                                            themes_theme_change_notify,
                                            picman);

      g_hash_table_destroy (themes_hash);
      themes_hash = NULL;
    }
}

gchar **
themes_list_themes (Picman *picman,
                    gint *n_themes)
{

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (n_themes != NULL, NULL);

  *n_themes = g_hash_table_size (themes_hash);

  if (*n_themes > 0)
    {
      gchar **themes;
      gchar **index;

      themes = g_new0 (gchar *, *n_themes + 1);

      index = themes;

      g_hash_table_foreach (themes_hash, themes_list_themes_foreach, &index);

      return themes;
    }

  return NULL;
}

const gchar *
themes_get_theme_dir (Picman        *picman,
                      const gchar *theme_name)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (! theme_name)
    theme_name = "Default";

  return g_hash_table_lookup (themes_hash, theme_name);
}

gchar *
themes_get_theme_file (Picman        *picman,
                       const gchar *first_component,
                       ...)
{
  PicmanGuiConfig *gui_config;
  gchar         *file;
  gchar         *component;
  gchar         *path;
  va_list        args;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (first_component != NULL, NULL);

  file = g_strdup (first_component);

  va_start (args, first_component);

  while ((component = va_arg (args, gchar *)))
    {
      gchar *tmp;

      tmp = g_build_filename (file, component, NULL);
      g_free (file);
      file = tmp;
    }

  va_end (args);

  gui_config = PICMAN_GUI_CONFIG (picman->config);

  path = g_build_filename (themes_get_theme_dir (picman, gui_config->theme),
                           file, NULL);

  if (! g_file_test (path, G_FILE_TEST_EXISTS))
    {
      g_free (path);

      path = g_build_filename (themes_get_theme_dir (picman, NULL),
                               file, NULL);
    }

  g_free (file);

  return path;
}


/*  private functions  */

static void
themes_apply_theme (Picman        *picman,
                    const gchar *theme_name)
{
  const gchar *theme_dir;
  gchar       *gtkrc_theme;
  gchar       *gtkrc_user;
  gchar       *themerc;
  FILE        *file;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  theme_dir = themes_get_theme_dir (picman, theme_name);

  if (theme_dir)
    {
      gtkrc_theme = g_build_filename (theme_dir, "gtkrc", NULL);
    }
  else
    {
      /*  get the hardcoded default theme gtkrc  */
      gtkrc_theme = g_strdup (picman_gtkrc ());
    }

  gtkrc_user = picman_personal_rc_file ("gtkrc");

  themerc = picman_personal_rc_file ("themerc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n",
             picman_filename_to_utf8 (themerc));

  file = g_fopen (themerc, "w");

  if (! file)
    {
      picman_message (picman, NULL, PICMAN_MESSAGE_ERROR,
                    _("Could not open '%s' for writing: %s"),
                    picman_filename_to_utf8 (themerc), g_strerror (errno));
      goto cleanup;
    }

  {
    gchar *esc_gtkrc_theme = g_strescape (gtkrc_theme, NULL);
    gchar *esc_gtkrc_user  = g_strescape (gtkrc_user, NULL);

    fprintf (file,
             "# PICMAN themerc\n"
             "#\n"
             "# This file is written on PICMAN startup and on every theme change.\n"
             "# It is NOT supposed to be edited manually. Edit your personal\n"
             "# gtkrc file instead (%s).\n"
             "\n"
             "include \"%s\"\n"
             "include \"%s\"\n"
             "\n"
             "# end of themerc\n",
             gtkrc_user,
             esc_gtkrc_theme,
             esc_gtkrc_user);

    g_free (esc_gtkrc_theme);
    g_free (esc_gtkrc_user);
  }

  fclose (file);

 cleanup:
  g_free (gtkrc_theme);
  g_free (gtkrc_user);
  g_free (themerc);
}

static void
themes_directories_foreach (const PicmanDatafileData *file_data,
                            gpointer                user_data)
{
  Picman *picman = PICMAN (user_data);

  if (picman->be_verbose)
    g_print ("Adding theme '%s' (%s)\n",
             picman_filename_to_utf8 (file_data->basename),
             picman_filename_to_utf8 (file_data->filename));

  g_hash_table_insert (themes_hash,
                       g_strdup (file_data->basename),
                       g_strdup (file_data->filename));
}

static void
themes_list_themes_foreach (gpointer key,
                            gpointer value,
                            gpointer data)
{
  gchar ***index = data;

  **index = g_strdup ((gchar *) key);

  (*index)++;
}

static void
themes_theme_change_notify (PicmanGuiConfig *config,
                            GParamSpec    *pspec,
                            Picman          *picman)
{
  themes_apply_theme (picman, config->theme);

  gtk_rc_reparse_all ();
}
