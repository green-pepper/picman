/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picman-templates.h"
#include "picmanlist.h"
#include "picmantemplate.h"


/* functions to load and save the picman templates files */

void
picman_templates_load (Picman *picman)
{
  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_LIST (picman->templates));

  filename = picman_personal_rc_file ("templaterc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (picman->templates),
                                      filename, NULL, &error))
    {
      if (error->code == PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        {
          g_clear_error (&error);
          g_free (filename);

          filename = g_build_filename (picman_sysconf_directory (),
                                       "templaterc", NULL);

          if (! picman_config_deserialize_file (PICMAN_CONFIG (picman->templates),
                                              filename, NULL, &error))
            {
              picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR,
				    error->message);
            }
        }
      else
        {
          picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
        }

      g_clear_error (&error);
    }

  picman_list_reverse (PICMAN_LIST (picman->templates));

  g_free (filename);
}

void
picman_templates_save (Picman *picman)
{
  const gchar *header =
    "PICMAN templaterc\n"
    "\n"
    "This file will be entirely rewritten each time you exit.";
  const gchar *footer =
    "end of templaterc";

  gchar  *filename;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_LIST (picman->templates));

  filename = picman_personal_rc_file ("templaterc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (picman->templates),
                                       filename,
                                       header, footer, NULL,
                                       &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_error_free (error);
    }

  g_free (filename);
}


/*  just like picman_list_get_child_by_name() but matches case-insensitive
 *  and dpi/ppi-insensitive
 */
static PicmanObject *
picman_templates_migrate_get_child_by_name (const PicmanContainer *container,
                                          const gchar         *name)
{
  PicmanList   *list   = PICMAN_LIST (container);
  PicmanObject *retval = NULL;
  GList      *glist;

  for (glist = list->list; glist; glist = g_list_next (glist))
    {
      PicmanObject *object = glist->data;
      gchar      *str1   = g_ascii_strdown (picman_object_get_name (object), -1);
      gchar      *str2   = g_ascii_strdown (name, -1);

      if (! strcmp (str1, str2))
        {
          retval = object;
        }
      else
        {
          gchar *dpi = strstr (str1, "dpi");

          if (dpi)
            {
              strncpy (dpi, "ppi", 3);

              g_print ("replaced: %s\n", str1);

              if (! strcmp (str1, str2))
                retval = object;
            }
        }

      g_free (str1);
      g_free (str2);
    }

  return retval;
}

/**
 * picman_templates_migrate:
 * @olddir: the old user directory
 *
 * Migrating the templaterc from PICMAN 2.0 to PICMAN 2.2 needs this special
 * hack since we changed the way that units are handled. This function
 * merges the user's templaterc with the systemwide templaterc. The goal
 * is to replace the unit for a couple of default templates with "pixels".
 **/
void
picman_templates_migrate (const gchar *olddir)
{
  PicmanContainer *templates = picman_list_new (PICMAN_TYPE_TEMPLATE, TRUE);
  gchar         *filename  = picman_personal_rc_file ("templaterc");

  if (picman_config_deserialize_file (PICMAN_CONFIG (templates), filename,
                                    NULL, NULL))
    {
      gchar *tmp = g_build_filename (picman_sysconf_directory (),
                                     "templaterc", NULL);

      if (olddir && (strstr (olddir, "2.0") || strstr (olddir, "2.2")))
        {
          /* We changed the spelling of a couple of template names:
           *
           * - from upper to lower case between 2.0 and 2.2
           * - from "dpi" to "ppi" between 2.2 and 2.4
           */
          PicmanContainerClass *class = PICMAN_CONTAINER_GET_CLASS (templates);
          gpointer            func  = class->get_child_by_name;

          class->get_child_by_name = picman_templates_migrate_get_child_by_name;

          picman_config_deserialize_file (PICMAN_CONFIG (templates),
                                        tmp, NULL, NULL);

          class->get_child_by_name = func;
        }
      else
        {
          picman_config_deserialize_file (PICMAN_CONFIG (templates),
                                        tmp, NULL, NULL);
        }

      g_free (tmp);

      picman_list_reverse (PICMAN_LIST (templates));

      picman_config_serialize_to_file (PICMAN_CONFIG (templates), filename,
                                     NULL, NULL, NULL, NULL);
    }

  g_free (filename);
}
