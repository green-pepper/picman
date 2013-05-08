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

#ifndef __PICMAN_HELP_DOMAIN_H__
#define __PICMAN_HELP_DOMAIN_H__


struct _PicmanHelpDomain
{
  gchar      *help_domain;
  gchar      *help_uri;
  GHashTable *help_locales;
};


PicmanHelpDomain * picman_help_domain_new           (const gchar       *domain_name,
                                                 const gchar       *domain_uri);
void             picman_help_domain_free          (PicmanHelpDomain    *domain);

PicmanHelpLocale * picman_help_domain_lookup_locale (PicmanHelpDomain    *domain,
                                                 const gchar       *locale_id,
                                                 PicmanHelpProgress  *progress);
gchar          * picman_help_domain_map           (PicmanHelpDomain    *domain,
                                                 GList             *help_locales,
                                                 const gchar       *help_id,
                                                 PicmanHelpProgress  *progress,
                                                 PicmanHelpLocale   **locale,
                                                 gboolean          *fatal_error);
void             picman_help_domain_exit          (void);


#endif /* ! __PICMAN_HELP_DOMAIN_H__ */
