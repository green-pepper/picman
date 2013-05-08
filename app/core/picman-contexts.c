/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picman-contexts.c
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picman-contexts.h"
#include "picmancontext.h"

#include "config/picmanconfig-file.h"

#include "picman-intl.h"


void
picman_contexts_init (Picman *picman)
{
  PicmanContext *context;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  /*  the default context contains the user's saved preferences
   *
   *  TODO: load from disk
   */
  context = picman_context_new (picman, "Default", NULL);
  picman_set_default_context (picman, context);
  g_object_unref (context);

  /*  the initial user_context is a straight copy of the default context
   */
  context = picman_context_new (picman, "User", context);
  picman_set_user_context (picman, context);
  g_object_unref (context);
}

void
picman_contexts_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_set_user_context (picman, NULL);
  picman_set_default_context (picman, NULL);
}

gboolean
picman_contexts_load (Picman    *picman,
                    GError **error)
{
  gchar    *filename;
  GError   *my_error = NULL;
  gboolean  success;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = picman_personal_rc_file ("contextrc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  success = picman_config_deserialize_file (PICMAN_CONFIG (picman_get_user_context (picman)),
                                          filename,
                                          NULL, &my_error);

  if (! success)
    {
      if (my_error->code == PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        {
          g_clear_error (&my_error);
          success = TRUE;
        }
      else
        {
          g_propagate_error (error, my_error);
        }
    }

  g_free (filename);

  return success;
}

gboolean
picman_contexts_save (Picman    *picman,
                    GError **error)
{
  gchar    *filename;
  gboolean  success;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = picman_personal_rc_file ("contextrc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  success = picman_config_serialize_to_file (PICMAN_CONFIG (picman_get_user_context (picman)),
                                           filename,
                                           "PICMAN user context",
                                           "end of user context",
                                           NULL, error);

  g_free (filename);

  return success;
}

gboolean
picman_contexts_clear (Picman    *picman,
                     GError **error)
{
  gchar    *filename;
  gboolean  success = TRUE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  filename = picman_personal_rc_file ("contextrc");

  if (g_unlink (filename) != 0 && errno != ENOENT)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Deleting \"%s\" failed: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      success = FALSE;
    }

  g_free (filename);

  return success;
}
