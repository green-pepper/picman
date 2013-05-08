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

#include <string.h>
#include <errno.h>

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picmangradient.h"
#include "picmangradient-save.h"

#include "picman-intl.h"


gboolean
picman_gradient_save (PicmanData  *data,
                    GError   **error)
{
  PicmanGradient        *gradient = PICMAN_GRADIENT (data);
  PicmanGradientSegment *seg;
  gint                 num_segments;
  FILE                *file;

  file = g_fopen (picman_data_get_filename (data), "wb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (picman_data_get_filename (data)),
                   g_strerror (errno));
      return FALSE;
    }

  /* File format is:
   *
   *   PICMAN Gradient
   *   Name: name
   *   number_of_segments
   *   left middle right r0 g0 b0 a0 r1 g1 b1 a1 type coloring left_color_type
   *   left middle right r0 g0 b0 a0 r1 g1 b1 a1 type coloring right_color_type
   *   ...
   */

  fprintf (file, "PICMAN Gradient\n");

  fprintf (file, "Name: %s\n", picman_object_get_name (gradient));

  /* Count number of segments */
  num_segments = 0;
  seg          = gradient->segments;

  while (seg)
    {
      num_segments++;
      seg = seg->next;
    }

  /* Write rest of file */
  fprintf (file, "%d\n", num_segments);

  for (seg = gradient->segments; seg; seg = seg->next)
    {
      gchar buf[11][G_ASCII_DTOSTR_BUF_SIZE];

      g_ascii_formatd (buf[0],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->left);
      g_ascii_formatd (buf[1],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->middle);
      g_ascii_formatd (buf[2],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->right);
      g_ascii_formatd (buf[3],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->left_color.r);
      g_ascii_formatd (buf[4],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->left_color.g);
      g_ascii_formatd (buf[5],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->left_color.b);
      g_ascii_formatd (buf[6],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->left_color.a);
      g_ascii_formatd (buf[7],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->right_color.r);
      g_ascii_formatd (buf[8],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->right_color.g);
      g_ascii_formatd (buf[9],  G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->right_color.b);
      g_ascii_formatd (buf[10], G_ASCII_DTOSTR_BUF_SIZE,
                       "%f", seg->right_color.a);

      fprintf (file, "%s %s %s %s %s %s %s %s %s %s %s %d %d %d %d\n",
               buf[0], buf[1], buf[2],          /* left, middle, right */
               buf[3], buf[4], buf[5], buf[6],  /* left color          */
               buf[7], buf[8], buf[9], buf[10], /* right color         */
               (gint) seg->type,
               (gint) seg->color,
               (gint) seg->left_color_type,
               (gint) seg->right_color_type);
    }

  fclose (file);

  return TRUE;
}

gboolean
picman_gradient_save_pov (PicmanGradient  *gradient,
                        const gchar   *filename,
                        GError       **error)
{
  FILE                *file;
  PicmanGradientSegment *seg;
  gchar                buf[G_ASCII_DTOSTR_BUF_SIZE];
  gchar                color_buf[4][G_ASCII_DTOSTR_BUF_SIZE];

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  file = g_fopen (filename, "wb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }
  else
    {
      fprintf (file, "/* color_map file created by PICMAN */\n");
      fprintf (file, "/* http://www.picman.org/           */\n");

      fprintf (file, "color_map {\n");

      for (seg = gradient->segments; seg; seg = seg->next)
        {
          /* Left */
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->left);
          g_ascii_formatd (color_buf[0], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->left_color.r);
          g_ascii_formatd (color_buf[1], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->left_color.g);
          g_ascii_formatd (color_buf[2], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->left_color.b);
          g_ascii_formatd (color_buf[3], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           1.0 - seg->left_color.a);

          fprintf (file, "\t[%s color rgbt <%s, %s, %s, %s>]\n",
                   buf,
                   color_buf[0], color_buf[1], color_buf[2], color_buf[3]);

          /* Middle */
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->middle);
          g_ascii_formatd (color_buf[0], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           (seg->left_color.r + seg->right_color.r) / 2.0);
          g_ascii_formatd (color_buf[1], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           (seg->left_color.g + seg->right_color.g) / 2.0);
          g_ascii_formatd (color_buf[2], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           (seg->left_color.b + seg->right_color.b) / 2.0);
          g_ascii_formatd (color_buf[3], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           1.0 - (seg->left_color.a + seg->right_color.a) / 2.0);

          fprintf (file, "\t[%s color rgbt <%s, %s, %s, %s>]\n",
                   buf,
                   color_buf[0], color_buf[1], color_buf[2], color_buf[3]);

          /* Right */
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->right);
          g_ascii_formatd (color_buf[0], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->right_color.r);
          g_ascii_formatd (color_buf[1], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->right_color.g);
          g_ascii_formatd (color_buf[2], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           seg->right_color.b);
          g_ascii_formatd (color_buf[3], G_ASCII_DTOSTR_BUF_SIZE, "%f",
                           1.0 - seg->right_color.a);

          fprintf (file, "\t[%s color rgbt <%s, %s, %s, %s>]\n",
                   buf,
                   color_buf[0], color_buf[1], color_buf[2], color_buf[3]);
        }

      fprintf (file, "} /* color_map */\n");
      fclose (file);
    }

  return TRUE;
}
