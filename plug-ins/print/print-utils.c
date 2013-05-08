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

#include <libpicman/picman.h>

#include "print-utils.h"


GKeyFile *
print_utils_key_file_load_from_rcfile (const gchar *basename)
{
  GKeyFile *key_file;
  gchar    *filename;

  g_return_val_if_fail (basename != NULL, NULL);

  filename = g_build_filename (picman_directory (), basename, NULL);

  key_file = g_key_file_new ();

  if (! g_key_file_load_from_file (key_file, filename, G_KEY_FILE_NONE, NULL))
    {
      g_key_file_free (key_file);
      key_file = NULL;
    }

  g_free (filename);

  return key_file;
}

GKeyFile *
print_utils_key_file_load_from_parasite (gint32       image_ID,
                                         const gchar *parasite_name)
{
  PicmanParasite *parasite;
  GKeyFile     *key_file;
  GError       *error = NULL;

  g_return_val_if_fail (parasite_name != NULL, NULL);

  parasite = picman_image_get_parasite (image_ID, parasite_name);

  if (! parasite)
    return NULL;

  key_file = g_key_file_new ();

  if (! g_key_file_load_from_data (key_file,
                                   picman_parasite_data (parasite),
                                   picman_parasite_data_size (parasite),
                                   G_KEY_FILE_NONE, &error))
    {
      g_key_file_free (key_file);
      picman_parasite_free (parasite);

      g_warning ("Unable to create key file from image parasite '%s': %s",
                 parasite_name, error->message);
      g_error_free (error);
      return NULL;
    }

  picman_parasite_free (parasite);

  return key_file;
}

void
print_utils_key_file_save_as_rcfile (GKeyFile    *key_file,
                                     const gchar *basename)
{
  gchar  *filename;
  gchar  *contents;
  gsize   length;
  GError *error = NULL;

  g_return_if_fail (basename != NULL);

  contents = g_key_file_to_data (key_file, &length, &error);

  if (! contents)
    {
      g_warning ("Unable to get contents of key file for '%s': %s",
                 basename, error->message);
      g_error_free (error);
      return;
    }

  filename = g_build_filename (picman_directory (), basename, NULL);

  if (! g_file_set_contents (filename, contents, length, &error))
    {
      g_warning ("Unable to write settings to '%s': %s",
                 picman_filename_to_utf8 (filename), error->message);
      g_error_free (error);
    }

  g_free (filename);
  g_free (contents);
}

void
print_utils_key_file_save_as_parasite (GKeyFile    *key_file,
                                       gint32       image_ID,
                                       const gchar *parasite_name)
{
  PicmanParasite *parasite;
  gchar        *contents;
  gsize         length;
  GError       *error = NULL;

  g_return_if_fail (parasite_name != NULL);

  contents = g_key_file_to_data (key_file, &length, &error);

  if (! contents)
    {
      g_warning ("Unable to get contents of key file for parasite '%s': %s",
                 parasite_name, error->message);
      g_error_free (error);
      return;
    }

  parasite = picman_parasite_new (parasite_name, 0, length, contents);
  g_free (contents);

  picman_image_attach_parasite (image_ID, parasite);
  picman_parasite_free (parasite);
}
