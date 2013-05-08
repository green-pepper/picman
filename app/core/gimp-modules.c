/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmodules.c
 * (C) 1999 Austin Donnelly <austin@picman.org>
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmodule/picmanmodule.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picman-modules.h"

#include "picman-intl.h"


void
picman_modules_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (! picman->no_interface)
    {
      picman->module_db = picman_module_db_new (picman->be_verbose);
      picman->write_modulerc = FALSE;
    }
}

void
picman_modules_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->module_db)
    {
      g_object_unref (picman->module_db);
      picman->module_db = NULL;
    }
}

void
picman_modules_load (Picman *picman)
{
  gchar    *filename;
  gchar    *path;
  GScanner *scanner;
  gchar    *module_load_inhibit = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->no_interface)
    return;

  /* FIXME, picman->be_verbose is not yet initialized in init() */
  picman->module_db->verbose = picman->be_verbose;

  filename = picman_personal_rc_file ("modulerc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  scanner = picman_scanner_new_file (filename, NULL);
  g_free (filename);

  if (scanner)
    {
      GTokenType  token;
      GError     *error = NULL;

#define MODULE_LOAD_INHIBIT 1

      g_scanner_scope_add_symbol (scanner, 0, "module-load-inhibit",
                                  GINT_TO_POINTER (MODULE_LOAD_INHIBIT));

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
              if (scanner->value.v_symbol == GINT_TO_POINTER (MODULE_LOAD_INHIBIT))
                {
                  token = G_TOKEN_STRING;

                  if (! picman_scanner_parse_string_no_validate (scanner,
                                                               &module_load_inhibit))
                    goto error;
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

#undef MODULE_LOAD_INHIBIT

      if (token != G_TOKEN_LEFT_PAREN)
        {
          g_scanner_get_next_token (scanner);
          g_scanner_unexp_token (scanner, token, NULL, NULL, NULL,
                                 _("fatal parse error"), TRUE);
        }

    error:

      if (error)
        {
          picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
          g_clear_error (&error);
        }

      picman_scanner_destroy (scanner);
    }

  if (module_load_inhibit)
    {
      picman_module_db_set_load_inhibit (picman->module_db, module_load_inhibit);
      g_free (module_load_inhibit);
    }

  path = picman_config_path_expand (picman->config->module_path, TRUE, NULL);
  picman_module_db_load (picman->module_db, path);
  g_free (path);
}

static void
add_to_inhibit_string (gpointer data,
                       gpointer user_data)
{
  PicmanModule *module = data;
  GString    *str    = user_data;

  if (module->load_inhibit)
    {
      g_string_append_c (str, G_SEARCHPATH_SEPARATOR);
      g_string_append (str, module->filename);
    }
}

void
picman_modules_unload (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (! picman->no_interface && picman->write_modulerc)
    {
      PicmanConfigWriter *writer;
      GString          *str;
      const gchar      *p;
      gchar            *filename;
      GError           *error = NULL;

      str = g_string_new (NULL);
      g_list_foreach (picman->module_db->modules, add_to_inhibit_string, str);
      if (str->len > 0)
        p = str->str + 1;
      else
        p = "";

      filename = picman_personal_rc_file ("modulerc");

      if (picman->be_verbose)
        g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

      writer = picman_config_writer_new_file (filename, TRUE,
                                            "PICMAN modulerc", &error);
      g_free (filename);

      if (writer)
        {
          picman_config_writer_open (writer, "module-load-inhibit");
          picman_config_writer_string (writer, p);
          picman_config_writer_close (writer);

          picman_config_writer_finish (writer, "end of modulerc", &error);

          picman->write_modulerc = FALSE;
        }

      g_string_free (str, TRUE);

      if (error)
        {
          picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
          g_clear_error (&error);
        }
    }
}

void
picman_modules_refresh (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (! picman->no_interface)
    {
      gchar *path;

      path = picman_config_path_expand (picman->config->module_path, TRUE, NULL);
      picman_module_db_refresh (picman->module_db, path);
      g_free (path);
    }
}
