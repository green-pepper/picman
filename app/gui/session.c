/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Session-managment stuff
 * Copyright (C) 1998 Sven Neumann <sven@picman.org>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "gui-types.h"

#include "config/picmanconfig-file.h"
#include "config/picmanguiconfig.h"

#include "core/picman.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmansessioninfo.h"

#include "dialogs/dialogs.h"

#include "session.h"
#include "picman-log.h"

#include "picman-intl.h"


enum
{
  SESSION_INFO = 1,
  HIDE_DOCKS,
  SINGLE_WINDOW_MODE,
  LAST_TIP_SHOWN
};


static gchar * session_filename (Picman *picman);


/*  private variables  */

static gboolean   sessionrc_deleted = FALSE;


/*  public functions  */

void
session_init (Picman *picman)
{
  gchar      *filename;
  GScanner   *scanner;
  GTokenType  token;
  GError     *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = session_filename (picman);

  scanner = picman_scanner_new_file (filename, &error);

  if (! scanner && error->code == PICMAN_CONFIG_ERROR_OPEN_ENOENT)
    {
      g_clear_error (&error);
      g_free (filename);

      filename = g_build_filename (picman_sysconf_directory (),
                                   "sessionrc", NULL);
      scanner = picman_scanner_new_file (filename, NULL);
    }

  if (! scanner)
    {
      g_clear_error (&error);
      g_free (filename);
      return;
    }

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  g_scanner_scope_add_symbol (scanner, 0, "session-info",
                              GINT_TO_POINTER (SESSION_INFO));
  g_scanner_scope_add_symbol (scanner, 0,  "hide-docks",
                              GINT_TO_POINTER (HIDE_DOCKS));
  g_scanner_scope_add_symbol (scanner, 0,  "single-window-mode",
                              GINT_TO_POINTER (SINGLE_WINDOW_MODE));
  g_scanner_scope_add_symbol (scanner, 0,  "last-tip-shown",
                              GINT_TO_POINTER (LAST_TIP_SHOWN));

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_SYMBOL;
          break;

        case G_TOKEN_SYMBOL:
          if (scanner->value.v_symbol == GINT_TO_POINTER (SESSION_INFO))
            {
              PicmanDialogFactory      *factory      = NULL;
              PicmanSessionInfo        *info         = NULL;
              gchar                  *factory_name = NULL;
              gchar                  *entry_name   = NULL;
              PicmanDialogFactoryEntry *entry        = NULL;

              token = G_TOKEN_STRING;

              if (! picman_scanner_parse_string (scanner, &factory_name))
                break;

              /* In versions <= PICMAN 2.6 there was a "toolbox", a
               * "dock", a "display" and a "toplevel" factory. These
               * are now merged to a single picman_dialog_factory_get_singleton (). We
               * need the legacy name though, so keep it around.
               */
              factory = picman_dialog_factory_get_singleton ();

              info = picman_session_info_new ();

              /* PICMAN 2.6 has the entry name as part of the
               * session-info header, so try to get it
               */
              picman_scanner_parse_string (scanner, &entry_name);
              if (entry_name)
                {
                  /* Previously, PicmanDock was a toplevel. That is why
                   * versions <= PICMAN 2.6 has "dock" as the entry name. We
                   * want "dock" to be interpreted as 'dock window'
                   * however so have some special-casing for that. When
                   * the entry name is "dock" the factory name is either
                   * "dock" or "toolbox".
                   */
                  if (strcmp (entry_name, "dock") == 0)
                    {
                      entry =
                        picman_dialog_factory_find_entry (factory,
                                                        (strcmp (factory_name, "toolbox") == 0 ?
                                                         "picman-toolbox-window" :
                                                         "picman-dock-window"));
                    }
                  else
                    {
                      entry = picman_dialog_factory_find_entry (factory,
                                                              entry_name);
                    }
                }

              /* We're done with these now */
              g_free (factory_name);
              g_free (entry_name);

              /* We can get the factory entry either now (the PICMAN <=
               * 2.6 way), or when we deserialize (the PICMAN 2.8 way)
               */
              if (entry)
                {
                  picman_session_info_set_factory_entry (info, entry);
                }

              /* Always try to deserialize */
              if (picman_config_deserialize (PICMAN_CONFIG (info), scanner, 1, NULL))
                {
                  /* Make sure we got a factory entry either the 2.6
                   * or 2.8 way
                   */
                  if (picman_session_info_get_factory_entry (info))
                    {
                      PICMAN_LOG (DIALOG_FACTORY,
                                "successfully parsed and added session info %p",
                                info);

                      picman_dialog_factory_add_session_info (factory, info);
                    }
                  else
                    {
                      PICMAN_LOG (DIALOG_FACTORY,
                                "failed to parse session info %p, not adding",
                                info);
                    }

                  g_object_unref (info);
                }
              else
                {
                  g_object_unref (info);

                  /* set token to left paren to we won't set another
                   * error below, picman_config_deserialize() already did
                   */
                  token = G_TOKEN_LEFT_PAREN;
                  goto error;
                }
            }
          else if (scanner->value.v_symbol == GINT_TO_POINTER (HIDE_DOCKS))
            {
              gboolean hide_docks;

              token = G_TOKEN_IDENTIFIER;

              if (! picman_scanner_parse_boolean (scanner, &hide_docks))
                break;

              g_object_set (picman->config,
                            "hide-docks", hide_docks,
                            NULL);
            }
          else if (scanner->value.v_symbol == GINT_TO_POINTER (SINGLE_WINDOW_MODE))
            {
              gboolean single_window_mode;

              token = G_TOKEN_IDENTIFIER;

              if (! picman_scanner_parse_boolean (scanner, &single_window_mode))
                break;

              g_object_set (picman->config,
                            "single-window-mode", single_window_mode,
                            NULL);
            }
          else if (scanner->value.v_symbol == GINT_TO_POINTER (LAST_TIP_SHOWN))
            {
              gint last_tip_shown;

              token = G_TOKEN_INT;

              if (! picman_scanner_parse_int (scanner, &last_tip_shown))
                break;

              g_object_set (picman->config,
                            "last-tip-shown", last_tip_shown,
                            NULL);
            }
          token = G_TOKEN_RIGHT_PAREN;
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default: /* do nothing */
          break;
        }
    }

 error:

  if (token != G_TOKEN_LEFT_PAREN)
    {
      g_scanner_get_next_token (scanner);
      g_scanner_unexp_token (scanner, token, NULL, NULL, NULL,
                             _("fatal parse error"), TRUE);
    }

  if (error)
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);

      picman_config_file_backup_on_error (filename, "sessionrc", NULL);
    }

  picman_scanner_destroy (scanner);
  g_free (filename);

  dialogs_load_recent_docks (picman);
}

void
session_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
}

void
session_restore (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_dialog_factory_restore (picman_dialog_factory_get_singleton ());
}

void
session_save (Picman     *picman,
              gboolean  always_save)
{
  PicmanConfigWriter *writer;
  gchar            *filename;
  GError           *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (sessionrc_deleted && ! always_save)
    return;

  filename = session_filename (picman);

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  writer =
    picman_config_writer_new_file (filename,
                                 TRUE,
                                 "PICMAN sessionrc\n\n"
                                 "This file takes session-specific info "
                                 "(that is info, you want to keep between "
                                 "two PICMAN sessions).  You are not supposed "
                                 "to edit it manually, but of course you "
                                 "can do.  The sessionrc will be entirely "
                                 "rewritten every time you quit PICMAN.  "
                                 "If this file isn't found, defaults are "
                                 "used.",
                                 NULL);
  g_free (filename);

  if (!writer)
    return;

  picman_dialog_factory_save (picman_dialog_factory_get_singleton (), writer);
  picman_config_writer_linefeed (writer);

  picman_config_writer_open (writer, "hide-docks");
  picman_config_writer_identifier (writer,
                                 PICMAN_GUI_CONFIG (picman->config)->hide_docks ?
                                 "yes" : "no");
  picman_config_writer_close (writer);

  picman_config_writer_open (writer, "single-window-mode");
  picman_config_writer_identifier (writer,
                                 PICMAN_GUI_CONFIG (picman->config)->single_window_mode ?
                                 "yes" : "no");
  picman_config_writer_close (writer);

  picman_config_writer_open (writer, "last-tip-shown");
  picman_config_writer_printf (writer, "%d",
                             PICMAN_GUI_CONFIG (picman->config)->last_tip_shown);
  picman_config_writer_close (writer);

  if (! picman_config_writer_finish (writer, "end of sessionrc", &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);
    }

  dialogs_save_recent_docks (picman);

  sessionrc_deleted = FALSE;
}

gboolean
session_clear (Picman    *picman,
               GError **error)
{
  gchar    *filename;
  gboolean  success = TRUE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = session_filename (picman);

  if (g_unlink (filename) != 0 && errno != ENOENT)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
		   _("Deleting \"%s\" failed: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      success = FALSE;
    }
  else
    {
      sessionrc_deleted = TRUE;
    }

  g_free (filename);

  return success;
}


static gchar *
session_filename (Picman *picman)
{
  const gchar *basename;
  gchar       *filename;

  basename = g_getenv ("PICMAN_TESTING_SESSIONRC_NAME");
  if (! basename)
    basename = "sessionrc";

  filename = picman_personal_rc_file (basename);

  if (picman->session_name)
    {
      gchar *tmp = g_strconcat (filename, ".", picman->session_name, NULL);

      g_free (filename);
      filename = tmp;
    }

  return filename;
}
