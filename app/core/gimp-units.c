/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanunit.c
 * Copyright (C) 1999-2000 Michael Natterer <mitch@picman.org>
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

/* This file contains functions to load & save the file containing the
 * user-defined size units, when the application starts/finished.
 */

#include "config.h"

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picman-units.h"
#include "picmanunit.h"

#include "config/picmanconfig-file.h"

#include "picman-intl.h"


/*
 *  All deserialize functions return G_TOKEN_LEFT_PAREN on success,
 *  or the GTokenType they would have expected but didn't get.
 */

static GTokenType picman_unitrc_unit_info_deserialize (GScanner *scanner,
                                                     Picman     *picman);


void
picman_units_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->user_units   = NULL;
  picman->n_user_units = 0;
}

void
picman_units_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman_user_units_free (picman);
}


/*  unitrc functions  **********/

enum
{
  UNIT_INFO = 1,
  UNIT_FACTOR,
  UNIT_DIGITS,
  UNIT_SYMBOL,
  UNIT_ABBREV,
  UNIT_SINGULAR,
  UNIT_PLURAL
};

void
picman_unitrc_load (Picman *picman)
{
  gchar      *filename;
  GScanner   *scanner;
  GTokenType  token;
  GError     *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = picman_personal_rc_file ("unitrc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  scanner = picman_scanner_new_file (filename, &error);

  if (! scanner && error->code == PICMAN_CONFIG_ERROR_OPEN_ENOENT)
    {
      g_clear_error (&error);
      g_free (filename);

      filename = g_build_filename (picman_sysconf_directory (), "unitrc", NULL);
      scanner = picman_scanner_new_file (filename, NULL);
    }

  if (! scanner)
    {
      g_clear_error (&error);
      g_free (filename);
      return;
    }

  g_scanner_scope_add_symbol (scanner, 0,
                              "unit-info", GINT_TO_POINTER (UNIT_INFO));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "factor", GINT_TO_POINTER (UNIT_FACTOR));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "digits", GINT_TO_POINTER (UNIT_DIGITS));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "symbol", GINT_TO_POINTER (UNIT_SYMBOL));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "abbreviation", GINT_TO_POINTER (UNIT_ABBREV));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "singular", GINT_TO_POINTER (UNIT_SINGULAR));
  g_scanner_scope_add_symbol (scanner, UNIT_INFO,
                              "plural", GINT_TO_POINTER (UNIT_PLURAL));

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
          if (scanner->value.v_symbol == GINT_TO_POINTER (UNIT_INFO))
            {
              g_scanner_set_scope (scanner, UNIT_INFO);
              token = picman_unitrc_unit_info_deserialize (scanner, picman);

              if (token == G_TOKEN_RIGHT_PAREN)
                g_scanner_set_scope (scanner, 0);
            }
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default: /* do nothing */
          break;
        }
    }

  if (token != G_TOKEN_LEFT_PAREN)
    {
      g_scanner_get_next_token (scanner);
      g_scanner_unexp_token (scanner, token, NULL, NULL, NULL,
                             _("fatal parse error"), TRUE);

      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);

      picman_config_file_backup_on_error (filename, "unitrc", NULL);
    }

  picman_scanner_destroy (scanner);
  g_free (filename);
}

void
picman_unitrc_save (Picman *picman)
{
  PicmanConfigWriter *writer;
  gchar            *filename;
  gint              i;
  GError           *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = picman_personal_rc_file ("unitrc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  writer =
    picman_config_writer_new_file (filename,
                                 TRUE,
                                 "PICMAN units\n\n"
                                 "This file contains the user unit database. "
                                 "You can edit this list with the unit "
                                 "editor. You are not supposed to edit it "
                                 "manually, but of course you can do.\n"
                                 "This file will be entirely rewritten each "
                                 "time you exit.",
                                 NULL);

  g_free (filename);

  if (!writer)
    return;

  /*  save user defined units  */
  for (i = _picman_unit_get_number_of_built_in_units (picman);
       i < _picman_unit_get_number_of_units (picman);
       i++)
    {
      if (_picman_unit_get_deletion_flag (picman, i) == FALSE)
        {
          gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

          picman_config_writer_open (writer, "unit-info");
          picman_config_writer_string (writer,
                                     _picman_unit_get_identifier (picman, i));

          picman_config_writer_open (writer, "factor");
          picman_config_writer_print (writer,
                                    g_ascii_formatd (buf, sizeof (buf), "%f",
                                                     _picman_unit_get_factor (picman, i)),
                                    -1);
          picman_config_writer_close (writer);

          picman_config_writer_open (writer, "digits");
          picman_config_writer_printf (writer,
                                     "%d", _picman_unit_get_digits (picman, i));
          picman_config_writer_close (writer);

          picman_config_writer_open (writer, "symbol");
          picman_config_writer_string (writer,
                                     _picman_unit_get_symbol (picman, i));
          picman_config_writer_close (writer);

          picman_config_writer_open (writer, "abbreviation");
          picman_config_writer_string (writer,
                                     _picman_unit_get_abbreviation (picman, i));
          picman_config_writer_close (writer);

          picman_config_writer_open (writer, "singular");
          picman_config_writer_string (writer,
                                     _picman_unit_get_singular (picman, i));
          picman_config_writer_close (writer);

          picman_config_writer_open (writer, "plural");
          picman_config_writer_string (writer,
                                     _picman_unit_get_plural (picman, i));
          picman_config_writer_close (writer);

          picman_config_writer_close (writer);
        }
    }

  if (! picman_config_writer_finish (writer, "end of units", &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_clear_error (&error);
    }
}


/*  private functions  */

static GTokenType
picman_unitrc_unit_info_deserialize (GScanner *scanner,
                                   Picman     *picman)
{
  gchar      *identifier   = NULL;
  gdouble     factor       = 1.0;
  gint        digits       = 2.0;
  gchar      *symbol       = NULL;
  gchar      *abbreviation = NULL;
  gchar      *singular     = NULL;
  gchar      *plural       = NULL;
  GTokenType  token;

  if (! picman_scanner_parse_string (scanner, &identifier))
    return G_TOKEN_STRING;

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
          switch (GPOINTER_TO_INT (scanner->value.v_symbol))
            {
            case UNIT_FACTOR:
              token = G_TOKEN_FLOAT;
              if (! picman_scanner_parse_float (scanner, &factor))
                goto cleanup;
              break;

            case UNIT_DIGITS:
              token = G_TOKEN_INT;
              if (! picman_scanner_parse_int (scanner, &digits))
                goto cleanup;
              break;

            case UNIT_SYMBOL:
              token = G_TOKEN_STRING;
              if (! picman_scanner_parse_string (scanner, &symbol))
                goto cleanup;
              break;

            case UNIT_ABBREV:
              token = G_TOKEN_STRING;
              if (! picman_scanner_parse_string (scanner, &abbreviation))
                goto cleanup;
              break;

            case UNIT_SINGULAR:
              token = G_TOKEN_STRING;
              if (! picman_scanner_parse_string (scanner, &singular))
                goto cleanup;
              break;

            case UNIT_PLURAL:
              token = G_TOKEN_STRING;
              if (! picman_scanner_parse_string (scanner, &plural))
                goto cleanup;
             break;

            default:
              break;
            }
          token = G_TOKEN_RIGHT_PAREN;
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default:
          break;
        }
    }

  if (token == G_TOKEN_LEFT_PAREN)
    {
      token = G_TOKEN_RIGHT_PAREN;

      if (g_scanner_peek_next_token (scanner) == token)
        {
          PicmanUnit unit = _picman_unit_new (picman,
                                          identifier, factor, digits,
                                          symbol, abbreviation,
                                          singular, plural);

          /*  make the unit definition persistent  */
          _picman_unit_set_deletion_flag (picman, unit, FALSE);
        }
    }

 cleanup:

  g_free (identifier);
  g_free (symbol);
  g_free (abbreviation);
  g_free (singular);
  g_free (plural);

  return token;
}
