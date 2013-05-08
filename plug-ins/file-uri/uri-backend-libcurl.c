/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * libcurl backend for the URI plug-in
 * Copyright (C) 2006 Mukund Sivaraman <muks@mukund.org>
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

#include <curl/curl.h>
#include <curl/easy.h>

#include <glib/gstdio.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "uri-backend.h"

#include "libpicman/stdplugins-intl.h"


/*  private variables  */

static gchar *supported_protocols = NULL;
static gchar *user_agent          = NULL;


/*  public functions  */

gboolean
uri_backend_init (const gchar  *plugin_name,
                  gboolean      run,
                  PicmanRunMode   run_mode,
                  GError      **error)
{
  GString                *protocols;
  curl_version_info_data *vinfo;

  if (curl_global_init (CURL_GLOBAL_ALL))
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "%s", _("Could not initialize libcurl"));
      return FALSE;
    }

  vinfo = curl_version_info (CURLVERSION_NOW);

  protocols = g_string_new ("http:,ftp:,gopher:");

  if (vinfo->features & CURL_VERSION_SSL)
    {
      g_string_append (protocols, ",https:,ftps:");
    }

  supported_protocols = g_string_free (protocols, FALSE);
  user_agent = g_strconcat ("PICMAN/", PICMAN_VERSION, NULL);

  return TRUE;
}


void
uri_backend_shutdown (void)
{
  g_free (user_agent);
  g_free (supported_protocols);
  curl_global_cleanup ();
}


const gchar *
uri_backend_get_load_help (void)
{
  return "Loads a file using the libcurl file transfer library";
}


const gchar *
uri_backend_get_save_help (void)
{
  return NULL;
}


const gchar *
uri_backend_get_load_protocols (void)
{
  return supported_protocols;
}


const gchar *
uri_backend_get_save_protocols (void)
{
  return NULL;
}


static int
progress_callback (void   *clientp,
                   double  dltotal,
                   double  dlnow,
                   double  ultotal,
                   double  ulnow)
{
  gchar *memsize = NULL;

  if (dltotal > 0.0)
    {
      memsize = g_format_size (dltotal);
      picman_progress_set_text_printf (_("Downloading %s of image data"),
                                     memsize);
      picman_progress_update (dlnow / dltotal);
    }
  else
    {
      memsize = g_format_size (dlnow);
      picman_progress_set_text_printf (_("Downloaded %s of image data"),
                                     memsize);
      picman_progress_pulse ();
    }

  g_free (memsize);

  return 0;
}


gboolean
uri_backend_load_image (const gchar  *uri,
                        const gchar  *tmpname,
                        PicmanRunMode   run_mode,
                        GError      **error)
{
  FILE      *out_file;
  CURL      *curl_handle;
  CURLcode   result;
  glong      response_code;
  gchar     *eff_url   = NULL;
  gchar     *proto     = NULL;
  gboolean   is_http   = FALSE;
  gboolean   is_ftp    = FALSE;
  gboolean   is_gopher = FALSE;

  picman_progress_init (_("Connecting to server"));

  if ((out_file = g_fopen (tmpname, "wb")) == NULL)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   _("Could not open '%s' for writing: %s"),
                   picman_filename_to_utf8 (tmpname),  g_strerror (errno));
      return FALSE;
    }

  curl_handle = curl_easy_init ();
  curl_easy_setopt (curl_handle, CURLOPT_URL, uri);
  curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, out_file);
  curl_easy_setopt (curl_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
  curl_easy_setopt (curl_handle, CURLOPT_NOPROGRESS, FALSE);
  curl_easy_setopt (curl_handle, CURLOPT_USERAGENT, user_agent);

  curl_easy_setopt (curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
  curl_easy_setopt (curl_handle, CURLOPT_MAXREDIRS, 10);
  curl_easy_setopt (curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);

  /* the following empty string causes libcurl to send a list of
   * all supported encodings which turns on compression
   * if libcurl has support for compression
   */

  curl_easy_setopt (curl_handle, CURLOPT_ENCODING, "");

  if ((result = curl_easy_perform (curl_handle)) != 0)
    {
      fclose (out_file);
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Could not open '%s' for reading: %s"),
                   uri, curl_easy_strerror (result));
      curl_easy_cleanup (curl_handle);
      return FALSE;
    }

  curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

  /* protocol could be not specified in provided uri
     get complete url guessed by curl */
  curl_easy_getinfo (curl_handle, CURLINFO_EFFECTIVE_URL, &eff_url);

  /* detect uri protocol */
  if (! g_ascii_strncasecmp (eff_url, "http://", 7))
    {
      is_http = TRUE;
      proto = "HTTP";
    }
  else
  if (! g_ascii_strncasecmp (eff_url, "https://", 8))
    {
      is_http = TRUE;
      proto = "HTTPS";
    }
  else
  if (! g_ascii_strncasecmp (eff_url, "ftp://", 6))
    {
      is_ftp = TRUE;
      proto = "FTP";
    }
  else
  if (! g_ascii_strncasecmp (eff_url, "ftps://", 7))
    {
      is_ftp = TRUE;
      proto = "FTPS";
    }
  else
  if (! g_ascii_strncasecmp (eff_url ,"gopher://", 9))
    {
      is_gopher = TRUE;
      proto = "GOPHER";
    }
  else
    {
      proto = "UNKNOWN";
    }

  if (! ((is_http && response_code == 200) || (is_ftp && response_code == 226) || (is_gopher)))
    {
      fclose (out_file);
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   _("Opening '%s' for reading resulted in %s "
                     "response code: %ld"),
                   uri, proto, response_code);
      curl_easy_cleanup (curl_handle);
      return FALSE;
    }

  fclose (out_file);
  picman_progress_update (1.0);
  curl_easy_cleanup (curl_handle);

  return TRUE;
}


gboolean
uri_backend_save_image (const gchar  *uri,
                        const gchar  *tmpname,
                        PicmanRunMode   run_mode,
                        GError      **error)
{
  g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "not implemented");

  return FALSE;
}

gchar *
uri_backend_map_image (const gchar  *uri,
                       PicmanRunMode   run_mode)
{
  return NULL;
}
