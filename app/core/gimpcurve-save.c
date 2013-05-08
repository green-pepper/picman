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

#include "core-types.h"

#include "picmancurve.h"
#include "picmancurve-save.h"

#include "picman-intl.h"


gboolean
picman_curve_save (PicmanData  *data,
                 GError   **error)
{
  /* PicmanCurve *curve; */
  FILE      *file;

  g_return_val_if_fail (PICMAN_IS_CURVE (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* curve = PICMAN_CURVE (data); */

  file = g_fopen (picman_data_get_filename (data), "wb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (picman_data_get_filename (data)),
                   g_strerror (errno));
      return FALSE;
    }

  /* FIXME: write curve */

  fclose (file);

  return TRUE;
}
