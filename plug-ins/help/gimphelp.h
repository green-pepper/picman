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

#ifndef __PICMAN_HELP_H__
#define __PICMAN_HELP_H__


#include "picmanhelptypes.h"

#include "picmanhelpdomain.h"
#include "picmanhelpitem.h"
#include "picmanhelplocale.h"
#include "picmanhelpprogress.h"


#define PICMAN_HELP_DEFAULT_DOMAIN  "http://www.picman.org/help"
#define PICMAN_HELP_DEFAULT_ID      "picman-main"
#define PICMAN_HELP_DEFAULT_LOCALE  "en"

#define PICMAN_HELP_PREFIX          "help"
#define PICMAN_HELP_ENV_URI         "PICMAN2_HELP_URI"

/* #define PICMAN_HELP_DEBUG */


gboolean         picman_help_init            (gint            n_domain_names,
                                            gchar         **domain_names,
                                            gint            n_domain_uris,
                                            gchar         **domain_uris);
void             picman_help_exit            (void);

void             picman_help_register_domain (const gchar    *domain_name,
                                            const gchar    *domain_uri);
PicmanHelpDomain * picman_help_lookup_domain   (const gchar    *domain_name);

GList          * picman_help_parse_locales   (const gchar    *help_locales);


#endif /* ! __PICMAN_HELP_H__ */
