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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picmanpalette.h"
#include "picmanpalette-load.h"

#include "picman-intl.h"


GList *
picman_palette_load (PicmanContext  *context,
                   const gchar  *filename,
                   GError      **error)
{
  PicmanPalette      *palette;
  PicmanPaletteEntry *entry;
  gchar             str[1024];
  gchar            *tok;
  FILE             *file;
  gint              r, g, b;
  gint              linenum;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  r = g = b = 0;

  file = g_fopen (filename, "rb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  linenum = 1;
  if (! fgets (str, sizeof (str), file))
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Fatal parse error in palette file '%s': "
                     "Read error in line %d."),
                   picman_filename_to_utf8 (filename), linenum);
      fclose (file);
      return NULL;
    }

  if (! g_str_has_prefix (str, "PICMAN Palette"))
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Fatal parse error in palette file '%s': "
                     "Missing magic header."),
                   picman_filename_to_utf8 (filename));
      fclose (file);
      return NULL;
    }

  palette = g_object_new (PICMAN_TYPE_PALETTE,
                          "mime-type", "application/x-picman-palette",
                          NULL);

  linenum++;

  if (! fgets (str, sizeof (str), file))
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Fatal parse error in palette file '%s': "
                     "Read error in line %d."),
                   picman_filename_to_utf8 (filename), linenum);
      fclose (file);
      g_object_unref (palette);
      return NULL;
    }

  if (g_str_has_prefix (str, "Name: "))
    {
      gchar *utf8;

      utf8 = picman_any_to_utf8 (g_strstrip (str + strlen ("Name: ")), -1,
                               _("Invalid UTF-8 string in palette file '%s'"),
                               picman_filename_to_utf8 (filename));
      picman_object_take_name (PICMAN_OBJECT (palette), utf8);

      linenum++;
      if (! fgets (str, sizeof (str), file))
        {
          g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                       _("Fatal parse error in palette file '%s': "
                         "Read error in line %d."),
                       picman_filename_to_utf8 (filename), linenum);
          fclose (file);
          g_object_unref (palette);
          return NULL;
        }

      if (g_str_has_prefix (str, "Columns: "))
        {
          gint columns;

          columns = atoi (g_strstrip (str + strlen ("Columns: ")));

          if (columns < 0 || columns > 256)
            {
              g_message (_("Reading palette file '%s': "
                           "Invalid number of columns in line %d. "
                           "Using default value."),
                         picman_filename_to_utf8 (filename), linenum);
              columns = 0;
            }

          picman_palette_set_columns (palette, columns);

          linenum++;
          if (! fgets (str, sizeof (str), file))
            {
              g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                           _("Fatal parse error in palette file '%s': "
                             "Read error in line %d."),
                           picman_filename_to_utf8 (filename), linenum);
              fclose (file);
              g_object_unref (palette);
              return NULL;
            }
        }
    }
  else /* old palette format */
    {
      picman_object_take_name (PICMAN_OBJECT (palette),
                             g_filename_display_basename (filename));
    }

  while (! feof (file))
    {
      if (str[0] != '#' && str[0] != '\n')
        {
          tok = strtok (str, " \t");
          if (tok)
            r = atoi (tok);
          else
            g_message (_("Reading palette file '%s': "
                         "Missing RED component in line %d."),
                       picman_filename_to_utf8 (filename), linenum);

          tok = strtok (NULL, " \t");
          if (tok)
            g = atoi (tok);
          else
            g_message (_("Reading palette file '%s': "
                         "Missing GREEN component in line %d."),
                       picman_filename_to_utf8 (filename), linenum);

          tok = strtok (NULL, " \t");
          if (tok)
            b = atoi (tok);
          else
            g_message (_("Reading palette file '%s': "
                         "Missing BLUE component in line %d."),
                       picman_filename_to_utf8 (filename), linenum);

          /* optional name */
          tok = strtok (NULL, "\n");

          if (r < 0 || r > 255 ||
              g < 0 || g > 255 ||
              b < 0 || b > 255)
            g_message (_("Reading palette file '%s': "
                         "RGB value out of range in line %d."),
                       picman_filename_to_utf8 (filename), linenum);

          /* don't call picman_palette_add_entry here, it's rather inefficient */
          entry = g_slice_new0 (PicmanPaletteEntry);

          picman_rgba_set_uchar (&entry->color,
                               (guchar) r,
                               (guchar) g,
                               (guchar) b,
                               255);

          entry->name     = g_strdup (tok ? tok : _("Untitled"));
          entry->position = picman_palette_get_n_colors (palette);

          palette->colors = g_list_prepend (palette->colors, entry);
          palette->n_colors++;
        }

      linenum++;
      if (! fgets (str, sizeof (str), file))
        {
          if (feof (file))
            break;

          g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                       _("Fatal parse error in palette file '%s': "
                         "Read error in line %d."),
                       picman_filename_to_utf8 (filename), linenum);
          fclose (file);
          g_object_unref (palette);
          return NULL;
        }
    }

  fclose (file);

  palette->colors = g_list_reverse (palette->colors);

  return g_list_prepend (NULL, palette);
}

GList *
picman_palette_load_act (PicmanContext  *context,
                       const gchar  *filename,
                       GError      **error)
{
  PicmanPalette *palette;
  gchar       *palette_name;
  gint         fd;
  guchar       color_bytes[4];

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);

  if (! fd)
    {
      g_set_error (error,
                   G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  palette_name = g_filename_display_basename (filename);
  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));
  g_free (palette_name);

  while (read (fd, color_bytes, 3) == 3)
    {
      PicmanRGB color;

      picman_rgba_set_uchar (&color,
                           color_bytes[0],
                           color_bytes[1],
                           color_bytes[2],
                           255);
      picman_palette_add_entry (palette, -1, NULL, &color);
    }

  close (fd);

  return g_list_prepend (NULL, palette);
}

GList *
picman_palette_load_riff (PicmanContext  *context,
                        const gchar  *filename,
                        GError      **error)
{
  PicmanPalette *palette;
  gchar       *palette_name;
  gint         fd;
  guchar       color_bytes[4];

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);

  if (! fd)
    {
      g_set_error (error,
                   G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  palette_name = g_filename_display_basename (filename);
  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));
  g_free (palette_name);

  lseek (fd, 28, SEEK_SET);

  while (read (fd,
               color_bytes, sizeof (color_bytes)) == sizeof (color_bytes))
    {
      PicmanRGB color;

      picman_rgba_set_uchar (&color,
                           color_bytes[0],
                           color_bytes[1],
                           color_bytes[2],
                           255);
      picman_palette_add_entry (palette, -1, NULL, &color);
    }

  close (fd);

  return g_list_prepend (NULL, palette);
}

GList *
picman_palette_load_psp (PicmanContext  *context,
                       const gchar  *filename,
                       GError      **error)
{
  PicmanPalette *palette;
  gchar       *palette_name;
  gint         fd;
  guchar       color_bytes[4];
  gint         number_of_colors;
  gint         data_size;
  gint         i, j;
  gboolean     color_ok;
  gchar        buffer[4096];
  /*Maximum valid file size: 256 * 4 * 3 + 256 * 2  ~= 3650 bytes */
  gchar      **lines;
  gchar      **ascii_colors;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);

  if (! fd)
    {
      g_set_error (error,
                   G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  palette_name = g_filename_display_basename (filename);
  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));
  g_free (palette_name);

  lseek (fd, 16, SEEK_SET);
  data_size = read (fd, buffer, sizeof (buffer) - 1);
  buffer[data_size] = '\0';

  lines = g_strsplit (buffer, "\x0d\x0a", -1);

  number_of_colors = atoi (lines[0]);

  for (i = 0; i < number_of_colors; i++)
    {
      if (lines[i + 1] == NULL)
        {
          g_printerr ("Premature end of file reading %s.",
                      picman_filename_to_utf8 (filename));
          break;
        }

      ascii_colors = g_strsplit (lines[i + 1], " ", 3);
      color_ok = TRUE;

      for (j = 0 ; j < 3; j++)
        {
          if (ascii_colors[j] == NULL)
            {
              g_printerr ("Corrupted palette file %s.",
                          picman_filename_to_utf8 (filename));
              color_ok = FALSE;
              break;
            }

          color_bytes[j] = atoi (ascii_colors[j]);
        }

      if (color_ok)
        {
          PicmanRGB color;

          picman_rgba_set_uchar (&color,
                               color_bytes[0],
                               color_bytes[1],
                               color_bytes[2],
                               255);
          picman_palette_add_entry (palette, -1, NULL, &color);
        }

      g_strfreev (ascii_colors);
    }

  g_strfreev (lines);

  close (fd);

  return g_list_prepend (NULL, palette);
}

GList *
picman_palette_load_aco (PicmanContext  *context,
                       const gchar  *filename,
                       GError      **error)
{
  PicmanPalette *palette;
  gchar       *palette_name;
  gint         fd;
  gint         format_version;
  gint         number_of_colors;
  gint         i;
  gchar        header[4];
  gchar        color_info[10];
  gchar        format2_preamble[4];
  gint         status;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);

  if (! fd)
    {
      g_set_error (error,
                   G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  status = read (fd, header, sizeof (header));

  if (status != sizeof (header))
    {
      g_set_error (error,
                   PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Could not read header from palette file '%s'"),
                   picman_filename_to_utf8 (filename));
      close (fd);
      return NULL;
    }

  palette_name = g_filename_display_basename (filename);
  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));
  g_free (palette_name);

  format_version = header[1] + (header[0] << 8);
  number_of_colors = header[3] + (header[2] << 8);

  for (i = 0; i < number_of_colors; i++)
    {
      gint     color_space;
      gint     w, x, y, z;
      gboolean color_ok = FALSE;
      PicmanRGB  color;

      if (read (fd, color_info, sizeof (color_info)) != sizeof (color_info))
	{
          g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                       _("Fatal parse error in palette file '%s'"),
                       picman_filename_to_utf8 (filename));
          close (fd);
          g_object_unref (palette);
          return NULL;
	}

      color_space = color_info[1] + (color_info[0] << 8);

      w = (guchar) color_info[3] + ((guchar) color_info[2] << 8);
      x = (guchar) color_info[5] + ((guchar) color_info[4] << 8);
      y = (guchar) color_info[7] + ((guchar) color_info[6] << 8);
      z = (guchar) color_info[9] + ((guchar) color_info[8] << 8);

      if (color_space == 0) /* RGB */
        {
          gdouble R = ((gdouble) w) / 65536.0;
          gdouble G = ((gdouble) x) / 65536.0;
          gdouble B = ((gdouble) y) / 65536.0;

          picman_rgba_set (&color, R, G, B, 1.0);

          color_ok = TRUE;
        }
      else if (color_space == 1) /* HSV */
        {
          PicmanHSV hsv;

          gdouble H = ((gdouble) w) / 65536.0;
          gdouble S = ((gdouble) x) / 65536.0;
          gdouble V = ((gdouble) y) / 65536.0;

          picman_hsva_set (&hsv, H, S, V, 1.0);
          picman_hsv_to_rgb (&hsv, &color);

          color_ok = TRUE;
        }
      else if (color_space == 2) /* CMYK */
        {
          PicmanCMYK cmyk;

          gdouble C = 1.0 - (((gdouble) w) / 65536.0);
          gdouble M = 1.0 - (((gdouble) x) / 65536.0);
          gdouble Y = 1.0 - (((gdouble) y) / 65536.0);
          gdouble K = 1.0 - (((gdouble) z) / 65536.0);

          picman_cmyka_set (&cmyk, C, M, Y, K, 1.0);
          picman_cmyk_to_rgb (&cmyk, &color);

          color_ok = TRUE;
        }
      else if (color_space == 8) /* Grayscale */
        {
          gdouble K = 1.0 - (((gdouble) w) / 10000.0);

          picman_rgba_set (&color, K, K, K, 1.0);

          color_ok = TRUE;
        }
      else if (color_space == 9) /* Wide? CMYK */
        {
          PicmanCMYK cmyk;

          gdouble C = 1.0 - (((gdouble) w) / 10000.0);
          gdouble M = 1.0 - (((gdouble) x) / 10000.0);
          gdouble Y = 1.0 - (((gdouble) y) / 10000.0);
          gdouble K = 1.0 - (((gdouble) z) / 10000.0);

          picman_cmyka_set (&cmyk, C, M, Y, K, 1.0);
          picman_cmyk_to_rgb (&cmyk, &color);

          color_ok = TRUE;
        }
      else
        {
          g_printerr ("Unsupported color space (%d) in ACO file %s\n",
                      color_space, picman_filename_to_utf8 (filename));
        }

      if (format_version == 2)
        {
          gint number_of_chars;

	  if (read (fd,
		    format2_preamble,
		    sizeof (format2_preamble)) != sizeof (format2_preamble))
	    {
	      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
			   _("Fatal parse error in palette file '%s'"),
			   picman_filename_to_utf8 (filename));
	      close (fd);
	      g_object_unref (palette);
	      return NULL;
	    }

          number_of_chars = format2_preamble[3] + (format2_preamble[2] << 8);
          lseek (fd, number_of_chars * 2, SEEK_SET);
        }

      if (color_ok)
        picman_palette_add_entry (palette, -1, NULL, &color);
    }

  close(fd);

  return g_list_prepend (NULL, palette);
}


GList *
picman_palette_load_css (PicmanContext  *context,
                       const gchar  *filename,
                       GError      **error)
{
  PicmanPalette *palette;
  gchar       *name;
  FILE        *file;
  GRegex      *regex;
  PicmanRGB      color;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  regex = g_regex_new (".*color.*:(?P<param>.*);", G_REGEX_CASELESS, 0, error);
  if (! regex)
    return NULL;

  file = g_fopen (filename, "rb");

  if (! file)
    {
      g_set_error (error, PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_OPEN,
                   _("Could not open '%s' for reading: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      return NULL;
    }

  name = g_filename_display_basename (filename);
  palette = PICMAN_PALETTE (picman_palette_new (context, name));
  g_free (name);

  do
    {
      GMatchInfo *matches;
      gchar       buf[1024];

      if (fgets (buf, sizeof (buf), file) != NULL)
        {
          if (g_regex_match (regex, buf, 0, &matches))
            {
              gchar *word = g_match_info_fetch_named (matches, "param");

              if (picman_rgb_parse_css (&color, word, -1))
                {
                  if (! picman_palette_find_entry (palette, &color, NULL))
                    {
                      picman_palette_add_entry (palette, -1, NULL, &color);
                    }
                }

              g_free (word);
            }
        }
    } while (! feof (file));

  fclose (file);

  g_regex_unref (regex);

  return g_list_prepend (NULL, palette);
}

PicmanPaletteFileFormat
picman_palette_load_detect_format (const gchar *filename)
{
  PicmanPaletteFileFormat format = PICMAN_PALETTE_FILE_FORMAT_UNKNOWN;
  gint                  fd;
  gchar                 header[16];

  fd = g_open (filename, O_RDONLY | _O_BINARY, 0);

  if (fd)
    {
      if (read (fd, header, sizeof (header)) == sizeof (header))
        {
          if (g_str_has_prefix (header + 0, "RIFF") &&
              g_str_has_prefix (header + 8, "PAL data"))
            {
              format = PICMAN_PALETTE_FILE_FORMAT_RIFF_PAL;
            }
          else if (g_str_has_prefix (header, "PICMAN Palette"))
            {
              format = PICMAN_PALETTE_FILE_FORMAT_GPL;
            }
          else if (g_str_has_prefix (header, "JASC-PAL"))
            {
              format = PICMAN_PALETTE_FILE_FORMAT_PSP_PAL;
            }
        }

      if (format == PICMAN_PALETTE_FILE_FORMAT_UNKNOWN)
        {
          gchar *lower_filename = g_ascii_strdown (filename, -1);

          if (g_str_has_suffix (lower_filename, ".aco"))
            {
              format = PICMAN_PALETTE_FILE_FORMAT_ACO;
            }
          else if (g_str_has_suffix (lower_filename, ".css"))
            {
              format = PICMAN_PALETTE_FILE_FORMAT_CSS;
            }

          g_free (lower_filename);
        }

      if (format == PICMAN_PALETTE_FILE_FORMAT_UNKNOWN)
        {
          struct stat file_stat;

          if (fstat (fd, &file_stat) >= 0)
            {
              if (file_stat.st_size == 768)
                format = PICMAN_PALETTE_FILE_FORMAT_ACT;
            }
        }

      close (fd);
    }

  return format;
}

