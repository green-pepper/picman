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

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "core-types.h"

#include "picmancurve.h"
#include "picmancurve-load.h"

#include "picman-intl.h"


GList *
picman_curve_load (const gchar  *filename,
                 GError      **error)
{
  FILE *file;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  file = g_fopen (filename, "rb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  /* load curves */

  fclose (file);

  return NULL;
}
