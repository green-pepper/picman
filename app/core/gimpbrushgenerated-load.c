/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman_brush_generated module Copyright 1998 Jay Cox <jaycox@earthlink.net>
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
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picmanbrushgenerated.h"
#include "picmanbrushgenerated-load.h"

#include "picman-intl.h"


GList *
picman_brush_generated_load (PicmanContext  *context,
                           const gchar  *filename,
                           GError      **error)
{
  PicmanBrush               *brush;
  FILE                    *file;
  gchar                    string[256];
  gint                     linenum;
  gchar                   *name       = NULL;
  PicmanBrushGeneratedShape  shape      = PICMAN_BRUSH_GENERATED_CIRCLE;
  gboolean                 have_shape = FALSE;
  gint                     spikes     = 2;
  gdouble                  spacing;
  gdouble                  radius;
  gdouble                  hardness;
  gdouble                  aspect_ratio;
  gdouble                  angle;

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

  /* make sure the file we are reading is the right type */
  errno = 0;
  linenum = 1;
  if (! fgets (string, sizeof (string), file))
    goto failed;

  if (! g_str_has_prefix (string, "PICMAN-VBR"))
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Fatal parse error in brush file '%s': "
                     "Not a PICMAN brush file."),
                   picman_filename_to_utf8 (filename));
      goto failed;
    }

  /* make sure we are reading a compatible version */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;

  if (! g_str_has_prefix (string, "1.0"))
    {
      if (! g_str_has_prefix (string, "1.5"))
        {
          g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                       _("Fatal parse error in brush file '%s': "
                         "Unknown PICMAN brush version in line %d."),
                       picman_filename_to_utf8 (filename), linenum);
          goto failed;
        }
      else
        {
          have_shape = TRUE;
        }
    }

  /* read name */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;

  g_strstrip (string);

  /* the empty string is not an allowed name */
  if (strlen (string) < 1)
    g_strlcpy (string, _("Untitled"), sizeof (string));

  name = picman_any_to_utf8 (string, -1,
                           _("Invalid UTF-8 string in brush file '%s'."),
                           picman_filename_to_utf8 (filename));

  if (have_shape)
    {
      GEnumClass *enum_class;
      GEnumValue *shape_val;

      enum_class = g_type_class_peek (PICMAN_TYPE_BRUSH_GENERATED_SHAPE);

      /* read shape */
      errno = 0;
      linenum++;
      if (! fgets (string, sizeof (string), file))
        goto failed;

      g_strstrip (string);
      shape_val = g_enum_get_value_by_nick (enum_class, string);

      if (! shape_val)
        {
          g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                       _("Fatal parse error in brush file '%s': "
                         "Unknown PICMAN brush shape in line %d."),
                       picman_filename_to_utf8 (filename), linenum);
          goto failed;
        }

      shape = shape_val->value;
    }

  /* read brush spacing */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;
  spacing = g_ascii_strtod (string, NULL);

  /* read brush radius */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;
  radius = g_ascii_strtod (string, NULL);

  if (have_shape)
    {
      /* read number of spikes */
      errno = 0;
      linenum++;
      if (! fgets (string, sizeof (string), file))
        goto failed;
      spikes = CLAMP (atoi (string), 2, 20);
    }

  /* read brush hardness */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;
  hardness = g_ascii_strtod (string, NULL);

  /* read brush aspect_ratio */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;
  aspect_ratio = g_ascii_strtod (string, NULL);

  /* read brush angle */
  errno = 0;
  linenum++;
  if (! fgets (string, sizeof (string), file))
    goto failed;
  angle = g_ascii_strtod (string, NULL);

  fclose (file);

  brush = PICMAN_BRUSH (picman_brush_generated_new (name, shape, radius, spikes,
                                                hardness, aspect_ratio, angle));
  g_free (name);

  brush->spacing = spacing;

  return g_list_prepend (NULL, brush);

 failed:

  fclose (file);

  if (name)
    g_free (name);

  if (error && *error == NULL)
    {
      gchar *msg;

      if (errno)
        msg = g_strdup_printf (_("Line %d: %s"), linenum, g_strerror (errno));
      else
        msg = g_strdup_printf (_("File is truncated in line %d"), linenum);

      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Error while reading brush file '%s': %s"),
                   picman_filename_to_utf8 (filename), msg);

      g_free (msg);
    }

  return NULL;
}
