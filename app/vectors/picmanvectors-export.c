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
#include <string.h>

#include <gegl.h>
#include <glib/gstdio.h>

#include "libpicmanbase/picmanbase.h"

#include "vectors-types.h"

#include "core/picmanimage.h"
#include "core/picmanitem.h"

#include "picmananchor.h"
#include "picmanstroke.h"
#include "picmanbezierstroke.h"
#include "picmanvectors.h"
#include "picmanvectors-export.h"

#include "picman-intl.h"


static GString * picman_vectors_export            (const PicmanImage   *image,
                                                 const PicmanVectors *vectors);
static void      picman_vectors_export_image_size (const PicmanImage   *image,
                                                 GString           *str);
static void      picman_vectors_export_path       (const PicmanVectors *vectors,
                                                 GString           *str);
static gchar   * picman_vectors_export_path_data  (const PicmanVectors *vectors);


/**
 * picman_vectors_export_file:
 * @image: the #PicmanImage from which to export vectors
 * @vectors: a #PicmanVectors object or %NULL to export all vectors in @image
 * @filename: the name of the file to write
 * @error: return location for errors
 *
 * Exports one or more vectors to a SVG file.
 *
 * Return value: %TRUE on success,
 *               %FALSE if there was an error writing the file
 **/
gboolean
picman_vectors_export_file (const PicmanImage    *image,
                          const PicmanVectors  *vectors,
                          const gchar        *filename,
                          GError            **error)
{
  FILE    *file;
  GString *str;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (vectors == NULL || PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  file = g_fopen (filename, "w");
  if (!file)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
		   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }

  str = picman_vectors_export (image, vectors);

  fprintf (file, "%s", str->str);

  g_string_free (str, TRUE);

  if (fclose (file))
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
		   _("Error while writing '%s': %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return FALSE;
    }

  return TRUE;
}

/**
 * picman_vectors_export_string:
 * @image: the #PicmanImage from which to export vectors
 * @vectors: a #PicmanVectors object or %NULL to export all vectors in @image
 *
 * Exports one or more vectors to a SVG string.
 *
 * Return value: a %NUL-terminated string that holds a complete XML document
 **/
gchar *
picman_vectors_export_string (const PicmanImage    *image,
                            const PicmanVectors  *vectors)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (vectors == NULL || PICMAN_IS_VECTORS (vectors), NULL);

  return g_string_free (picman_vectors_export (image, vectors), FALSE);
}

static GString *
picman_vectors_export (const PicmanImage   *image,
                     const PicmanVectors *vectors)
{
  GString *str = g_string_new (NULL);

  g_string_append_printf (str,
                          "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"\n"
                          "              \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n"
                          "\n"
                          "<svg xmlns=\"http://www.w3.org/2000/svg\"\n");

  g_string_append (str, "     ");
  picman_vectors_export_image_size (image, str);
  g_string_append_c (str, '\n');

  g_string_append_printf (str,
                          "     viewBox=\"0 0 %d %d\">\n",
                          picman_image_get_width  (image),
                          picman_image_get_height (image));

  if (vectors)
    {
      picman_vectors_export_path (vectors, str);
    }
  else
    {
      GList *list;

      for (list = picman_image_get_vectors_iter (image);
           list;
           list = list->next)
        {
          picman_vectors_export_path (PICMAN_VECTORS (list->data), str);
        }
    }

  g_string_append (str, "</svg>\n");

  return str;
}

static void
picman_vectors_export_image_size (const PicmanImage *image,
                                GString         *str)
{
  PicmanUnit     unit;
  const gchar *abbrev;
  gchar        wbuf[G_ASCII_DTOSTR_BUF_SIZE];
  gchar        hbuf[G_ASCII_DTOSTR_BUF_SIZE];
  gdouble      xres;
  gdouble      yres;
  gdouble      w, h;

  picman_image_get_resolution (image, &xres, &yres);

  w = (gdouble) picman_image_get_width  (image) / xres;
  h = (gdouble) picman_image_get_height (image) / yres;

  /*  FIXME: should probably use the display unit here  */
  unit = picman_image_get_unit (image);
  switch (unit)
    {
    case PICMAN_UNIT_INCH:  abbrev = "in";  break;
    case PICMAN_UNIT_MM:    abbrev = "mm";  break;
    case PICMAN_UNIT_POINT: abbrev = "pt";  break;
    case PICMAN_UNIT_PICA:  abbrev = "pc";  break;
    default:              abbrev = "cm";
      unit = PICMAN_UNIT_MM;
      w /= 10.0;
      h /= 10.0;
      break;
    }

  g_ascii_formatd (wbuf, sizeof (wbuf), "%g", w * picman_unit_get_factor (unit));
  g_ascii_formatd (hbuf, sizeof (hbuf), "%g", h * picman_unit_get_factor (unit));

  g_string_append_printf (str,
                          "width=\"%s%s\" height=\"%s%s\"",
                          wbuf, abbrev, hbuf, abbrev);
}

static void
picman_vectors_export_path (const PicmanVectors *vectors,
                          GString           *str)
{
  const gchar *name = picman_object_get_name (vectors);
  gchar       *data = picman_vectors_export_path_data (vectors);
  gchar       *esc_name;

  esc_name = g_markup_escape_text (name, strlen (name));

  g_string_append_printf (str,
                          "  <path id=\"%s\"\n"
                          "        fill=\"none\" stroke=\"black\" stroke-width=\"1\"\n"
                          "        d=\"%s\" />\n",
                          esc_name, data);

  g_free (esc_name);
  g_free (data);
}


#define NEWLINE "\n           "

static gchar *
picman_vectors_export_path_data (const PicmanVectors *vectors)
{
  GString  *str;
  GList    *strokes;
  gchar     x_string[G_ASCII_DTOSTR_BUF_SIZE];
  gchar     y_string[G_ASCII_DTOSTR_BUF_SIZE];
  gboolean  closed = FALSE;

  str = g_string_new (NULL);

  for (strokes = vectors->strokes; strokes; strokes = strokes->next)
    {
      PicmanStroke *stroke = strokes->data;
      GArray     *control_points;
      PicmanAnchor *anchor;
      gint        i;

      if (closed)
        g_string_append_printf (str, NEWLINE);

      control_points = picman_stroke_control_points_get (stroke, &closed);

      if (PICMAN_IS_BEZIER_STROKE (stroke))
        {
          if (control_points->len >= 3)
            {
              anchor = &g_array_index (control_points, PicmanAnchor, 1);
              g_ascii_formatd (x_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.x);
              g_ascii_formatd (y_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.y);
              g_string_append_printf (str, "M %s,%s", x_string, y_string);
            }

          if (control_points->len > 3)
            {
              g_string_append_printf (str, NEWLINE "C");
            }

          for (i = 2; i < (control_points->len + (closed ? 2 : - 1)); i++)
            {
              if (i > 2 && i % 3 == 2)
                g_string_append_printf (str, NEWLINE " ");

              anchor = &g_array_index (control_points, PicmanAnchor,
                                       i % control_points->len);
              g_ascii_formatd (x_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.x);
              g_ascii_formatd (y_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.y);
              g_string_append_printf (str, " %s,%s", x_string, y_string);
            }

          if (closed && control_points->len > 3)
            g_string_append_printf (str, " Z");
        }
      else
        {
          g_printerr ("Unknown stroke type\n");

          if (control_points->len >= 1)
            {
              anchor = &g_array_index (control_points, PicmanAnchor, 0);
              g_ascii_formatd (x_string, G_ASCII_DTOSTR_BUF_SIZE,
                               ".2f", anchor->position.x);
              g_ascii_formatd (y_string, G_ASCII_DTOSTR_BUF_SIZE,
                               ".2f", anchor->position.y);
              g_string_append_printf (str, "M %s,%s", x_string, y_string);
            }

          if (control_points->len > 1)
            {
              g_string_append_printf (str, NEWLINE "L");
            }

          for (i = 1; i < control_points->len; i++)
            {
              if (i > 1 && i % 3 == 1)
                g_string_append_printf (str, NEWLINE " ");

              anchor = &g_array_index (control_points, PicmanAnchor, i);
              g_ascii_formatd (x_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.x);
              g_ascii_formatd (y_string, G_ASCII_DTOSTR_BUF_SIZE,
                               "%.2f", anchor->position.y);
              g_string_append_printf (str, " %s,%s", x_string, y_string);
            }

          if (closed && control_points->len > 1)
            g_string_append_printf (str, " Z");
        }

      g_array_free (control_points, TRUE);
    }

  return g_strchomp (g_string_free (str, FALSE));
}
