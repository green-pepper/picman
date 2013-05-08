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

/* Author: Josh MacDonald. */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/wait.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "uri-backend.h"

#include "libpicman/stdplugins-intl.h"


#define TIMEOUT  300
#define BUFSIZE 1024


gboolean
uri_backend_init (const gchar  *plugin_name,
                  gboolean      run,
                  PicmanRunMode   run_mode,
                  GError      **error)
{
  return TRUE;
}

void
uri_backend_shutdown (void)
{
}

const gchar *
uri_backend_get_load_help (void)
{
  return "Loads a file using GNU Wget";
}

const gchar *
uri_backend_get_save_help (void)
{
  return NULL;
}

const gchar *
uri_backend_get_load_protocols (void)
{
  return "http:,https:,ftp:";
}

const gchar *
uri_backend_get_save_protocols (void)
{
  return NULL;
}

gboolean
uri_backend_load_image (const gchar  *uri,
                        const gchar  *tmpname,
                        PicmanRunMode   run_mode,
                        GError      **error)
{
  gint pid;
  gint p[2];

  if (pipe (p) != 0)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "pipe() failed: %s", g_strerror (errno));
      return FALSE;
    }

  /*  open a process group, so killing the plug-in will kill wget too  */
  setpgid (0, 0);

  if ((pid = fork()) < 0)
    {
      g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                   "fork() failed: %s", g_strerror (errno));
      return FALSE;
    }
  else if (pid == 0)
    {
      gchar timeout_str[16];

      close (p[0]);
      close (2);
      dup (p[1]);
      close (p[1]);

      /* produce deterministic output */
      g_setenv ("LANGUAGE", "C", TRUE);
      g_setenv ("LC_ALL", "C", TRUE);
      g_setenv ("LANG", "C", TRUE);

      g_snprintf (timeout_str, sizeof (timeout_str), "%d", TIMEOUT);

      execlp ("wget",
              "wget", "-v", "-e", "server-response=off", "--progress=dot", "-T", timeout_str,
              uri, "-O", tmpname, NULL);
      _exit (127);
    }
  else
    {
      FILE     *input;
      gchar     buf[BUFSIZE];
      gboolean  seen_resolve = FALSE;
      gboolean  seen_ftp     = FALSE;
      gboolean  connected    = FALSE;
      gboolean  redirect     = FALSE;
      gboolean  file_found   = FALSE;
      gchar     sizestr[37];
      gchar    *endptr;
      guint64   size         = 0;
      gint      i, j;
      gchar     dot;
      guint64   kilobytes    = 0;
      gboolean  finished     = FALSE;
      gboolean  debug        = FALSE;
      gchar    *memsize;
      gchar    *message;
      gchar    *timeout_msg;

#define DEBUG(x) if (debug) g_printerr ("%s\n", x)

      close (p[1]);

      input = fdopen (p[0], "r");

      /*  hardcoded and not-really-foolproof scanning of wget output  */

    wget_begin:
      /* Eat any Location lines */
      if (redirect && fgets (buf, sizeof (buf), input) == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("wget exited abnormally on URI '%s'"), uri);
          return FALSE;
        }

      redirect = FALSE;

      if (fgets (buf, sizeof (buf), input) == NULL)
        {
          /*  no message here because failing on the first line means
           *  that wget was not found
           */
          return FALSE;
        }

      DEBUG (buf);

      /*  The second line is the local copy of the file  */
      if (fgets (buf, sizeof (buf), input) == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("wget exited abnormally on URI '%s'"), uri);
          return FALSE;
        }
      /* with an ftp url wget has a "=> `filename.foo" */
      else if ( !seen_ftp && strstr (buf, "=> `"))
        {
          seen_ftp = TRUE;
        }

      DEBUG (buf);

      /*  The third line is "Connecting to..."  */

      timeout_msg = g_strdup_printf (ngettext ("(timeout is %d second)",
                                               "(timeout is %d seconds)",
                                               TIMEOUT), TIMEOUT);

      picman_progress_init_printf ("%s %s",
                                 _("Connecting to server"), timeout_msg);

    read_connect:
      if (fgets (buf, sizeof (buf), input) == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("wget exited abnormally on URI '%s'"), uri);
          return FALSE;
        }
      else if (strstr (buf, "connected"))
        {
          connected = TRUE;
        }
      /* newer wgets have a "Resolving foo" line, so eat it */
      else if (!seen_resolve && strstr (buf, "Resolving"))
        {
          seen_resolve = TRUE;
          goto read_connect;
        }

      DEBUG (buf);

      /*  The fourth line is either the network request or an error  */

      picman_progress_set_text_printf ("%s %s", _("Opening URI"), timeout_msg);

      if (fgets (buf, sizeof (buf), input) == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("wget exited abnormally on URI '%s'"), uri);
          return FALSE;
        }
      else if (! connected)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("A network error occurred: %s"), buf);

          DEBUG (buf);

          return FALSE;
        }
      /* on successful ftp login wget prints a "Logged in" message */
      else if ( seen_ftp && !strstr(buf, "Logged in!"))
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("A network error occurred: %s"), buf);

          DEBUG (buf);

          return FALSE;
        }
      else if (strstr (buf, "302 Found"))
        {
          DEBUG (buf);

          connected = FALSE;
          seen_resolve = FALSE;

          redirect = TRUE;
          goto wget_begin;
        }

      DEBUG (buf);

      /* for an ftp session wget has extra output*/
    ftp_session:
      if (seen_ftp)
        {
          if (fgets (buf, sizeof (buf), input) == NULL)
            {
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                           _("A network error occurred: %s"), buf);

              DEBUG (buf);

              return FALSE;
            }
          /* if there is no size output file does not exist on server */
          else if (strstr (buf, "==> SIZE") && strstr (buf, "... done"))
            {
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                           _("wget exited abnormally on URI '%s'"), uri);

              DEBUG (buf);

              return FALSE;
            }
          /* while no PASV line we eat other messages */
          else if (!strstr (buf, "==> PASV"))
            {
              DEBUG (buf);
              goto ftp_session;
            }
        }

      /*  The fifth line is either the length of the file or an error  */
      if (fgets (buf, sizeof (buf), input) == NULL)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("wget exited abnormally on URI '%s'"), uri);
          return FALSE;
        }
      else if (strstr (buf, "Length"))
        {
          file_found = TRUE;
        }
      else
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       _("A network error occurred: %s"), buf);

          DEBUG (buf);

          return FALSE;
        }

      DEBUG (buf);

      if (sscanf (buf, "Length: %37s", sizestr) != 1)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       "Could not parse wget's file length message");
          return FALSE;
        }

      /*  strip away commas  */
      for (i = 0, j = 0; i < sizeof (sizestr); i++, j++)
        {
          if (sizestr[i] == ',')
            i++;

          sizestr[j] = sizestr[i];

          if (sizestr[j] == '\0')
            break;
        }

      if (*sizestr != '\0')
        {
          size = g_ascii_strtoull (sizestr, &endptr, 10);

          if (*endptr != '\0' || size == G_MAXUINT64)
            size = 0;
        }

      /* on http sessions wget has "Saving to: ..." */
      if (!seen_ftp)
        {
          if (fgets (buf, sizeof (buf), input) == NULL)
            {
              g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                           _("wget exited abnormally on URI '%s'"), uri);

              return FALSE;
            }
        }

      /*  Start the actual download...  */
      if (size > 0)
        {
          memsize = g_format_size (size);
          message = g_strdup_printf (_("Downloading %s of image data"),
                                     memsize);
        }
      else
        {
          message = g_strdup (_("Downloading unknown amount of image data"));
          memsize = NULL;
        }

      picman_progress_set_text_printf ("%s %s", message, timeout_msg);

      g_free (message);
      g_free (memsize);

      /*  Switch to byte parsing wget's output...  */

      while (TRUE)
        {
          dot = fgetc (input);

          if (feof (input))
            break;

          if (debug)
            {
              fputc (dot, stderr);
              fflush (stderr);
            }

          if (dot == '.')  /* one kilobyte */
            {
              kilobytes++;

              if (size > 0)
                {
                  picman_progress_update ((gdouble) (kilobytes * 1024) /
                                        (gdouble) size);
                }
              else
                {
                  memsize = g_format_size (kilobytes * 1024);

                  picman_progress_set_text_printf
                    (_("Downloaded %s of image data"), memsize);
                  picman_progress_pulse ();

                  g_free (memsize);
                }
            }
          else if (dot == ':')  /* the time string contains a ':' */
            {
              fgets (buf, sizeof (buf), input);

              DEBUG (buf);

              if (! strstr (buf, "error"))
                {
                  finished = TRUE;
                  picman_progress_update (1.0);
                }

              break;
            }
        }

      if (! finished)
        {
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
                       "wget exited before finishing downloading URI\n'%s'",
                       uri);
          return FALSE;
        }
    }

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
