/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
 *
 * file-utils.c
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

#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanthumb/picmanthumb.h"

#include "core/core-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"

#include "plug-in/picmanpluginmanager.h"

#include "file-procedure.h"
#include "file-utils.h"

#include "picman-intl.h"


static gchar * file_utils_unescape_uri  (const gchar *escaped,
                                         gint         len,
                                         const gchar *illegal_escaped_characters,
                                         gboolean     ascii_must_not_be_escaped);


gboolean
file_utils_filename_is_uri (const gchar  *filename,
                            GError      **error)
{
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (strstr (filename, "://"))
    {
      gchar *scheme;
      gchar *canon;

      scheme = g_strndup (filename, (strstr (filename, "://") - filename));
      canon  = g_strdup (scheme);

      g_strcanon (canon, G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS "+-.", '-');

      if (strcmp (scheme, canon) || ! g_ascii_isgraph (canon[0]))
        {
          g_set_error (error, G_FILE_ERROR, 0,
                       _("'%s:' is not a valid URI scheme"), scheme);

          g_free (scheme);
          g_free (canon);

          return FALSE;
        }

      g_free (scheme);
      g_free (canon);

      if (! g_utf8_validate (filename, -1, NULL))
        {
          g_set_error_literal (error,
			       G_CONVERT_ERROR,
			       G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
			       _("Invalid character sequence in URI"));
          return FALSE;
        }

      return TRUE;
    }

  return FALSE;
}

gchar *
file_utils_filename_to_uri (Picman         *picman,
                            const gchar  *filename,
                            GError      **error)
{
  GError *temp_error = NULL;
  gchar  *absolute;
  gchar  *uri;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /*  check for prefixes like http or ftp  */
  if (file_procedure_find_by_prefix (picman->plug_in_manager->load_procs,
                                     filename))
    {
      if (g_utf8_validate (filename, -1, NULL))
        {
          return g_strdup (filename);
        }
      else
        {
          g_set_error_literal (error,
			       G_CONVERT_ERROR,
			       G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
			       _("Invalid character sequence in URI"));
          return NULL;
        }
    }
  else if (file_utils_filename_is_uri (filename, &temp_error))
    {
      return g_strdup (filename);
    }
  else if (temp_error)
    {
      g_propagate_error (error, temp_error);

      return NULL;
    }

  if (! g_path_is_absolute (filename))
    {
      gchar *current;

      current = g_get_current_dir ();
      absolute = g_build_filename (current, filename, NULL);
      g_free (current);
    }
  else
    {
      absolute = g_strdup (filename);
    }

  uri = g_filename_to_uri (absolute, NULL, error);

  g_free (absolute);

  return uri;
}

gchar *
file_utils_any_to_uri (Picman         *picman,
                       const gchar  *filename_or_uri,
                       GError      **error)
{
  gchar *uri;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (filename_or_uri != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /*  first try if we got a file uri  */
  uri = g_filename_from_uri (filename_or_uri, NULL, NULL);

  if (uri)
    {
      g_free (uri);
      uri = g_strdup (filename_or_uri);
    }
  else
    {
      uri = file_utils_filename_to_uri (picman, filename_or_uri, error);
    }

  return uri;
}


/**
 * file_utils_filename_from_uri:
 * @uri: a URI
 *
 * A utility function to be used as a replacement for
 * g_filename_from_uri(). It deals with file: URIs with hostname in a
 * platform-specific way. On Win32, a UNC path is created and
 * returned, on other platforms the URI is detected as non-local and
 * NULL is returned.
 *
 * Returns: newly allocated filename or %NULL if @uri is a remote file
 **/
gchar *
file_utils_filename_from_uri (const gchar *uri)
{
  gchar *filename;
  gchar *hostname;

  g_return_val_if_fail (uri != NULL, NULL);

  filename = g_filename_from_uri (uri, &hostname, NULL);

  if (!filename)
    return NULL;

  if (hostname)
    {
      /*  we have a file: URI with a hostname                           */
#ifdef G_OS_WIN32
      /*  on Win32, create a valid UNC path and use it as the filename  */

      gchar *tmp = g_build_filename ("//", hostname, filename, NULL);

      g_free (filename);
      filename = tmp;
#else
      /*  otherwise return NULL, caller should use URI then             */
      g_free (filename);
      filename = NULL;
#endif

      g_free (hostname);
    }

  return filename;
}

gchar *
file_utils_uri_with_new_ext (const gchar *uri,
                             const gchar *ext_uri)
{
  const gchar *uri_ext      = file_utils_uri_get_ext (uri);
  const gchar *ext_uri_ext  = ext_uri ? file_utils_uri_get_ext (ext_uri) : NULL;
  gchar *uri_without_ext    = g_strndup (uri, uri_ext - uri);
  gchar *ret                = g_strconcat (uri_without_ext, ext_uri_ext, NULL);
  g_free (uri_without_ext);
  return ret;
}


/**
 * file_utils_uri_get_ext:
 * @uri:
 *
 * Returns the position of the extension (including the .) for an URI. If
 * there is no extension the returned position is right after the
 * string, at the terminating NULL character.
 *
 * Returns:
 **/
const gchar *
file_utils_uri_get_ext (const gchar *uri)
{
  const gchar *ext        = NULL;
  int          uri_len    = strlen (uri);
  int          search_len = 0;

  if (g_strrstr (uri, ".gz"))
    search_len = uri_len - 3;
  else if (g_strrstr (uri, ".bz2"))
    search_len = uri_len - 4;
  else if (g_strrstr (uri, ".xz"))
    search_len = uri_len - 3;
  else
    search_len = uri_len;

  ext = g_strrstr_len (uri, search_len, ".");

  if (! ext)
    ext = uri + uri_len;

  return ext;
}

gchar *
file_utils_uri_to_utf8_filename (const gchar *uri)
{
  g_return_val_if_fail (uri != NULL, NULL);

  if (g_str_has_prefix (uri, "file:"))
    {
      gchar *filename = file_utils_filename_from_uri (uri);

      if (filename)
        {
          GError *error = NULL;
          gchar  *utf8;

          utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, &error);
          g_free (filename);

          if (utf8)
            return utf8;

          g_warning ("%s: cannot convert filename to UTF-8: %s",
                     G_STRLOC, error->message);
          g_error_free (error);
        }
    }

  return g_strdup (uri);
}

static gchar *
file_utils_uri_to_utf8_basename (const gchar *uri)
{
  gchar *filename;
  gchar *basename = NULL;

  g_return_val_if_fail (uri != NULL, NULL);

  filename = file_utils_uri_to_utf8_filename (uri);

  if (strstr (filename, G_DIR_SEPARATOR_S))
    {
      basename = g_path_get_basename (filename);
    }
  else if (strstr (filename, "://"))
    {
      basename = strrchr (uri, '/');

      if (basename)
        basename = g_strdup (basename + 1);
    }

  if (basename)
    {
      g_free (filename);
      return basename;
    }

  return filename;
}

gchar *
file_utils_uri_display_basename (const gchar *uri)
{
  gchar *basename = NULL;

  g_return_val_if_fail (uri != NULL, NULL);

  if (g_str_has_prefix (uri, "file:"))
    {
      gchar *filename = file_utils_filename_from_uri (uri);

      if (filename)
        {
          basename = g_filename_display_basename (filename);
          g_free (filename);
        }
    }
  else
    {
      gchar *name = file_utils_uri_display_name (uri);

      basename = strrchr (name, '/');
      if (basename)
        basename = g_strdup (basename + 1);

      g_free (name);
    }

  return basename ? basename : file_utils_uri_to_utf8_basename (uri);
}

gchar *
file_utils_uri_display_name (const gchar *uri)
{
  gchar *name = NULL;

  g_return_val_if_fail (uri != NULL, NULL);

  if (g_str_has_prefix (uri, "file:"))
    {
      gchar *filename = file_utils_filename_from_uri (uri);

      if (filename)
        {
          name = g_filename_display_name (filename);
          g_free (filename);
        }
    }
  else
    {
      name = file_utils_unescape_uri (uri, -1, "/", FALSE);
    }

  return name ? name : g_strdup (uri);
}

GdkPixbuf *
file_utils_load_thumbnail (const gchar *filename)
{
  PicmanThumbnail *thumbnail = NULL;
  GdkPixbuf     *pixbuf    = NULL;
  gchar         *uri;

  g_return_val_if_fail (filename != NULL, NULL);

  uri = g_filename_to_uri (filename, NULL, NULL);

  if (uri)
    {
      thumbnail = picman_thumbnail_new ();
      picman_thumbnail_set_uri (thumbnail, uri);

      pixbuf = picman_thumbnail_load_thumb (thumbnail,
                                          PICMAN_THUMBNAIL_SIZE_NORMAL,
                                          NULL);
    }

  g_free (uri);

  if (pixbuf)
    {
      gint width  = gdk_pixbuf_get_width (pixbuf);
      gint height = gdk_pixbuf_get_height (pixbuf);

      if (gdk_pixbuf_get_n_channels (pixbuf) != 3)
        {
          GdkPixbuf *tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
                                           width, height);

          gdk_pixbuf_composite_color (pixbuf, tmp,
                                      0, 0, width, height, 0, 0, 1.0, 1.0,
                                      GDK_INTERP_NEAREST, 255,
                                      0, 0, PICMAN_CHECK_SIZE_SM,
                                      0x66666666, 0x99999999);

          g_object_unref (pixbuf);
          pixbuf = tmp;
        }
    }

  return pixbuf;
}

gboolean
file_utils_save_thumbnail (PicmanImage   *image,
                           const gchar *filename)
{
  const gchar *image_uri;
  gboolean     success = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  image_uri = picman_image_get_uri (image);

  if (image_uri)
    {
      gchar *uri = g_filename_to_uri (filename, NULL, NULL);

      if (uri)
        {
          if ( ! strcmp (uri, image_uri))
            {
              PicmanImagefile *imagefile;

              imagefile = picman_imagefile_new (image->picman, uri);
              success = picman_imagefile_save_thumbnail (imagefile, NULL, image);
              g_object_unref (imagefile);
            }

          g_free (uri);
        }
    }

  return success;
}


/* the following two functions are copied from glib/gconvert.c */

static gint
unescape_character (const gchar *scanner)
{
  gint first_digit;
  gint second_digit;

  first_digit = g_ascii_xdigit_value (scanner[0]);
  if (first_digit < 0)
    return -1;

  second_digit = g_ascii_xdigit_value (scanner[1]);
  if (second_digit < 0)
    return -1;

  return (first_digit << 4) | second_digit;
}

static gchar *
file_utils_unescape_uri (const gchar *escaped,
                         gint         len,
                         const gchar *illegal_escaped_characters,
                         gboolean     ascii_must_not_be_escaped)
{
  const gchar *in, *in_end;
  gchar *out, *result;
  gint c;

  if (escaped == NULL)
    return NULL;

  if (len < 0)
    len = strlen (escaped);

  result = g_malloc (len + 1);

  out = result;
  for (in = escaped, in_end = escaped + len; in < in_end; in++)
    {
      c = *in;

      if (c == '%')
	{
	  /* catch partial escape sequences past the end of the substring */
	  if (in + 3 > in_end)
	    break;

	  c = unescape_character (in + 1);

	  /* catch bad escape sequences and NUL characters */
	  if (c <= 0)
	    break;

	  /* catch escaped ASCII */
	  if (ascii_must_not_be_escaped && c <= 0x7F)
	    break;

	  /* catch other illegal escaped characters */
	  if (strchr (illegal_escaped_characters, c) != NULL)
	    break;

	  in += 2;
	}

      *out++ = c;
    }

  g_assert (out - result <= len);
  *out = '\0';

  if (in != in_end)
    {
      g_free (result);
      return NULL;
    }

  return result;
}
