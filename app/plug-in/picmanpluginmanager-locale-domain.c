/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-locale-domain.c
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "plug-in-types.h"

#include "picmanpluginmanager.h"
#include "picmanpluginmanager-locale-domain.h"


#define STD_PLUG_INS_LOCALE_DOMAIN GETTEXT_PACKAGE "-std-plug-ins"


typedef struct _PicmanPlugInLocaleDomain PicmanPlugInLocaleDomain;

struct _PicmanPlugInLocaleDomain
{
  gchar *prog_name;
  gchar *domain_name;
  gchar *domain_path;
};


void
picman_plug_in_manager_locale_domain_exit (PicmanPlugInManager *manager)
{
  GSList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));

  for (list = manager->locale_domains; list; list = list->next)
    {
      PicmanPlugInLocaleDomain *domain = list->data;

      g_free (domain->prog_name);
      g_free (domain->domain_name);
      g_free (domain->domain_path);
      g_slice_free (PicmanPlugInLocaleDomain, domain);
    }

  g_slist_free (manager->locale_domains);
  manager->locale_domains = NULL;
}

void
picman_plug_in_manager_add_locale_domain (PicmanPlugInManager *manager,
                                        const gchar       *prog_name,
                                        const gchar       *domain_name,
                                        const gchar       *domain_path)
{
  PicmanPlugInLocaleDomain *domain;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (prog_name != NULL);
  g_return_if_fail (domain_name != NULL);

  domain = g_slice_new (PicmanPlugInLocaleDomain);

  domain->prog_name   = g_strdup (prog_name);
  domain->domain_name = g_strdup (domain_name);
  domain->domain_path = g_strdup (domain_path);

  manager->locale_domains = g_slist_prepend (manager->locale_domains, domain);

#ifdef VERBOSE
  g_print ("added locale domain \"%s\" for path \"%s\"\n",
           domain->domain_name ? domain->domain_name : "(null)",
           domain->domain_path ?
           picman_filename_to_utf8 (domain->domain_path) : "(null)");
#endif
}

const gchar *
picman_plug_in_manager_get_locale_domain (PicmanPlugInManager  *manager,
                                        const gchar        *prog_name,
                                        const gchar       **domain_path)
{
  GSList *list;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), NULL);

  if (domain_path)
    *domain_path = picman_locale_directory ();

  /*  A NULL prog_name is PICMAN itself, return the default domain  */
  if (! prog_name)
    return NULL;

  for (list = manager->locale_domains; list; list = list->next)
    {
      PicmanPlugInLocaleDomain *domain = list->data;

      if (domain && domain->prog_name &&
          ! strcmp (domain->prog_name, prog_name))
        {
          if (domain_path && domain->domain_path)
            *domain_path = domain->domain_path;

          return domain->domain_name;
        }
    }

  return STD_PLUG_INS_LOCALE_DOMAIN;
}

gint
picman_plug_in_manager_get_locale_domains (PicmanPlugInManager   *manager,
                                         gchar             ***locale_domains,
                                         gchar             ***locale_paths)
{
  GSList *list;
  GSList *unique = NULL;
  gint    n_domains;
  gint    i;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), 0);
  g_return_val_if_fail (locale_domains != NULL, 0);
  g_return_val_if_fail (locale_paths != NULL, 0);

  for (list = manager->locale_domains; list; list = list->next)
    {
      PicmanPlugInLocaleDomain *domain = list->data;
      GSList                 *tmp;

      for (tmp = unique; tmp; tmp = tmp->next)
        if (! strcmp (domain->domain_name, (const gchar *) tmp->data))
          break;

      if (! tmp)
        unique = g_slist_prepend (unique, domain);
    }

  unique = g_slist_reverse (unique);

  n_domains = g_slist_length (unique) + 1;

  *locale_domains = g_new0 (gchar *, n_domains + 1);
  *locale_paths   = g_new0 (gchar *, n_domains + 1);

  (*locale_domains)[0] = g_strdup (STD_PLUG_INS_LOCALE_DOMAIN);
  (*locale_paths)[0]   = g_strdup (picman_locale_directory ());

  for (list = unique, i = 1; list; list = list->next, i++)
    {
      PicmanPlugInLocaleDomain *domain = list->data;

      (*locale_domains)[i] = g_strdup (domain->domain_name);
      (*locale_paths)[i]   = (domain->domain_path ?
                              g_strdup (domain->domain_path) :
                              g_strdup (picman_locale_directory ()));
    }

  g_slist_free (unique);

  return n_domains;
}
