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

#ifndef __PICMAN_HELP_LOCALE_H__
#define __PICMAN_HELP_LOCALE_H__


struct _PicmanHelpLocale
{
  gchar      *locale_id;
  GHashTable *help_id_mapping;
  gchar      *help_missing;

  /* eek */
  GList      *toplevel_items;
};


PicmanHelpLocale * picman_help_locale_new   (const gchar       *locale_id);
void             picman_help_locale_free  (PicmanHelpLocale    *locale);

const gchar    * picman_help_locale_map   (PicmanHelpLocale    *locale,
                                         const gchar       *help_id);

gboolean         picman_help_locale_parse (PicmanHelpLocale    *locale,
                                         const gchar       *uri,
                                         const gchar       *help_domain,
                                         PicmanHelpProgress  *progress,
                                         GError           **error);


#endif /* __PICMAN_HELP_LOCALE_H__ */
