/* picmanparasite.c: Copyright 1998 Jay Cox <jaycox@picman.org>
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
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picman-parasites.h"
#include "picmanparasitelist.h"


void
picman_parasite_attach (Picman               *picman,
                      const PicmanParasite *parasite)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (parasite != NULL);

  picman_parasite_list_add (picman->parasites, parasite);
}

void
picman_parasite_detach (Picman        *picman,
                      const gchar *name)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (name != NULL);

  picman_parasite_list_remove (picman->parasites, name);
}

const PicmanParasite *
picman_parasite_find (Picman        *picman,
                    const gchar *name)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return picman_parasite_list_find (picman->parasites, name);
}

static void
list_func (const gchar    *key,
           PicmanParasite   *parasite,
           gchar        ***current)
{
  *(*current)++ = g_strdup (key);
}

gchar **
picman_parasite_list (Picman *picman,
                    gint *count)
{
  gchar **list;
  gchar **current;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (count != NULL, NULL);

  *count = picman_parasite_list_length (picman->parasites);

  list = current = g_new (gchar *, *count);

  picman_parasite_list_foreach (picman->parasites, (GHFunc) list_func, &current);

  return list;
}


/*  FIXME: this doesn't belong here  */

void
picman_parasite_shift_parent (PicmanParasite *parasite)
{
  g_return_if_fail (parasite != NULL);

  parasite->flags = (parasite->flags >> 8);
}


/*  parasiterc functions  */

void
picman_parasiterc_load (Picman *picman)
{
  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  filename = picman_personal_rc_file ("parasiterc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (picman->parasites),
                                      filename, NULL, &error))
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);

      g_error_free (error);
    }

  g_free (filename);
}

void
picman_parasiterc_save (Picman *picman)
{
  const gchar *header =
    "PICMAN parasiterc\n"
    "\n"
    "This file will be entirely rewritten each time you exit.";
  const gchar *footer =
    "end of parasiterc";

  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_PARASITE_LIST (picman->parasites));

  filename = picman_personal_rc_file ("parasiterc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (picman->parasites),
                                       filename,
                                       header, footer, NULL,
                                       &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_error_free (error);
    }

  g_free (filename);
}
