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

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picmanpalette.h"
#include "picmanpalette-save.h"

#include "picman-intl.h"


gboolean
picman_palette_save (PicmanData  *data,
                   GError   **error)
{
  PicmanPalette *palette = PICMAN_PALETTE (data);
  GList       *list;
  FILE        *file;

  file = g_fopen (picman_data_get_filename (data), "wb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (picman_data_get_filename (data)),
                   g_strerror (errno));
      return FALSE;
    }

  fprintf (file, "PICMAN Palette\n");
  fprintf (file, "Name: %s\n", picman_object_get_name (palette));
  fprintf (file, "Columns: %d\n#\n", CLAMP (picman_palette_get_columns (palette),
                                            0, 256));

  for (list = picman_palette_get_colors (palette);
       list;
       list = g_list_next (list))
    {
      PicmanPaletteEntry *entry = list->data;
      guchar            r, g, b;

      picman_rgb_get_uchar (&entry->color, &r, &g, &b);

      fprintf (file, "%3d %3d %3d\t%s\n", r, g, b, entry->name);
    }

  fclose (file);

  return TRUE;
}
