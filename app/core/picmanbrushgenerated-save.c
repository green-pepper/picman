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
#include "picmanbrushgenerated-save.h"

#include "picman-intl.h"


gboolean
picman_brush_generated_save (PicmanData  *data,
                           GError   **error)
{
  PicmanBrushGenerated *brush = PICMAN_BRUSH_GENERATED (data);
  const gchar        *name  = picman_object_get_name (data);
  FILE               *file;
  gchar               buf[G_ASCII_DTOSTR_BUF_SIZE];
  gboolean            have_shape = FALSE;

  g_return_val_if_fail (name != NULL && *name != '\0', FALSE);

  file = g_fopen (picman_data_get_filename (data), "wb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (picman_data_get_filename (data)),
                   g_strerror (errno));
      return FALSE;
    }

  /* write magic header */
  fprintf (file, "PICMAN-VBR\n");

  /* write version */
  if (brush->shape != PICMAN_BRUSH_GENERATED_CIRCLE || brush->spikes > 2)
    {
      fprintf (file, "1.5\n");
      have_shape = TRUE;
    }
  else
    {
      fprintf (file, "1.0\n");
    }

  /* write name */
  fprintf (file, "%.255s\n", name);

  if (have_shape)
    {
      GEnumClass *enum_class;
      GEnumValue *shape_val;

      enum_class = g_type_class_peek (PICMAN_TYPE_BRUSH_GENERATED_SHAPE);

      /* write shape */
      shape_val = g_enum_get_value (enum_class, brush->shape);
      fprintf (file, "%s\n", shape_val->value_nick);
    }

  /* write brush spacing */
  fprintf (file, "%s\n",
           g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                            picman_brush_get_spacing (PICMAN_BRUSH (brush))));

  /* write brush radius */
  fprintf (file, "%s\n",
           g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                            brush->radius));

  if (have_shape)
    {
      /* write brush spikes */
      fprintf (file, "%d\n", brush->spikes);
    }

  /* write brush hardness */
  fprintf (file, "%s\n",
           g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                            brush->hardness));

  /* write brush aspect_ratio */
  fprintf (file, "%s\n",
           g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                            brush->aspect_ratio));

  /* write brush angle */
  fprintf (file, "%s\n",
           g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                            brush->angle));

  fclose (file);

  return TRUE;
}
