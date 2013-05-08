/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The PICMAN Help plug-in
 * Copyright (C) 1999-2008 Sven Neumann <sven@picman.org>
 *                         Michael Natterer <mitch@picman.org>
 *                         Henrik Brix Andersen <brix@picman.org>
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

/*  This code is written so that it can also be compiled standalone.
 *  It shouldn't depend on libpicman.
 */

#include "config.h"

#include <string.h>

#include <glib-object.h>
#include <gio/gio.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanhelp.h"

#ifdef DISABLE_NLS
#define _(String)  (String)
#else
#include "libpicman/stdplugins-intl.h"
#endif


/*  local function prototypes  */

static gboolean   domain_locale_parse (PicmanHelpDomain    *domain,
                                       PicmanHelpLocale    *locale,
                                       PicmanHelpProgress  *progress,
                                       GError           **error);


/*  public functions  */

PicmanHelpDomain *
picman_help_domain_new (const gchar *domain_name,
                      const gchar *domain_uri)
{
  PicmanHelpDomain *domain = g_slice_new0 (PicmanHelpDomain);

  domain->help_domain = g_strdup (domain_name);
  domain->help_uri    = g_strdup (domain_uri);

  if (domain_uri)
    {
      /*  strip trailing slash  */
      if (g_str_has_suffix (domain->help_uri, "/"))
        domain->help_uri[strlen (domain->help_uri) - 1] = '\0';
    }

  return domain;
}

void
picman_help_domain_free (PicmanHelpDomain *domain)
{
  g_return_if_fail (domain != NULL);

  if (domain->help_locales)
    g_hash_table_destroy (domain->help_locales);

  g_free (domain->help_domain);
  g_free (domain->help_uri);

  g_slice_free (PicmanHelpDomain, domain);
}

PicmanHelpLocale *
picman_help_domain_lookup_locale (PicmanHelpDomain    *domain,
                                const gchar       *locale_id,
                                PicmanHelpProgress  *progress)
{
  PicmanHelpLocale *locale = NULL;

  if (domain->help_locales)
    locale = g_hash_table_lookup (domain->help_locales, locale_id);
  else
    domain->help_locales =
      g_hash_table_new_full (g_str_hash, g_str_equal,
                             g_free,
                             (GDestroyNotify) picman_help_locale_free);

  if (locale)
    return locale;

  locale = picman_help_locale_new (locale_id);
  g_hash_table_insert (domain->help_locales, g_strdup (locale_id), locale);

  domain_locale_parse (domain, locale, progress, NULL);

  return locale;
}

gchar *
picman_help_domain_map (PicmanHelpDomain    *domain,
                      GList             *help_locales,
                      const gchar       *help_id,
                      PicmanHelpProgress  *progress,
                      PicmanHelpLocale   **ret_locale,
                      gboolean          *fatal_error)
{
  PicmanHelpLocale *locale = NULL;
  const gchar    *ref    = NULL;
  GList          *list;

  g_return_val_if_fail (domain != NULL, NULL);
  g_return_val_if_fail (help_locales != NULL, NULL);
  g_return_val_if_fail (help_id != NULL, NULL);

  if (fatal_error)
    *fatal_error = FALSE;

  /*  first pass: look for a reference matching the help_id  */
  for (list = help_locales; list && !ref; list = list->next)
    {
      locale = picman_help_domain_lookup_locale (domain,
                                               (const gchar *) list->data,
                                               progress);
      ref = picman_help_locale_map (locale, help_id);
    }

  /*  second pass: look for a fallback                 */
  for (list = help_locales; list && !ref; list = list->next)
    {
      locale = picman_help_domain_lookup_locale (domain,
                                               (const gchar *) list->data,
                                               progress);
      ref = locale->help_missing;
    }

  if (ret_locale)
    *ret_locale = locale;

  if (ref)
    {
      return g_strconcat (domain->help_uri,  "/",
                          locale->locale_id, "/",
                          ref,
                          NULL);
    }
  else  /*  try to assemble a useful error message  */
    {
      GError *error = NULL;

#ifdef PICMAN_HELP_DEBUG
      g_printerr ("help: help_id lookup and all fallbacks failed for '%s'\n",
                  help_id);
#endif

      locale = picman_help_domain_lookup_locale (domain,
                                               PICMAN_HELP_DEFAULT_LOCALE, NULL);

      if (! domain_locale_parse (domain, locale, NULL, &error))
        {
          switch (error->code)
            {
            case G_IO_ERROR_NOT_FOUND:
              if (domain->help_domain)
                {
                  g_message (_("The help pages for '%s' are not available."),
                             domain->help_domain);
                }
              else
                {
                  g_message ("%s\n\n%s",
                             _("The PICMAN user manual is not available."),
                             _("Please install the additional help package "
                               "or use the online user manual at "
                               "http://docs.picman.org/."));
                }
              break;

            case G_IO_ERROR_NOT_SUPPORTED:
              g_message ("%s\n\n%s",
                         error->message,
                         _("Perhaps you are missing GIO backends and need "
                           "to install GVFS?"));
              break;

            case G_IO_ERROR_CANCELLED:
              break;

            default:
              g_message ("%s", error->message);
              break;
            }

          g_error_free (error);

          if (fatal_error)
            *fatal_error = TRUE;
        }
      else
        {
          g_message (_("Help ID '%s' unknown"), help_id);
        }

      return NULL;
    }
}


/*  private functions  */

static gboolean
domain_locale_parse (PicmanHelpDomain    *domain,
                     PicmanHelpLocale    *locale,
                     PicmanHelpProgress  *progress,
                     GError           **error)
{
  gchar    *uri;
  gboolean  success;

  g_return_val_if_fail (domain != NULL, FALSE);
  g_return_val_if_fail (locale != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  uri = g_strdup_printf ("%s/%s/picman-help.xml",
                         domain->help_uri, locale->locale_id);

  success = picman_help_locale_parse (locale, uri, domain->help_domain,
                                    progress, error);

  g_free (uri);

  return success;
}
