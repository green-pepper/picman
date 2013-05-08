/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-help-domain.h
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

#ifndef __PICMAN_PLUG_IN_MANAGER_HELP_DOMAIN_H__
#define __PICMAN_PLUG_IN_MANAGER_HELP_DOMAIN_H__


void          picman_plug_in_manager_help_domain_exit (PicmanPlugInManager   *manager);

/* Add a help domain */
void          picman_plug_in_manager_add_help_domain  (PicmanPlugInManager   *manager,
                                                     const gchar         *prog_name,
                                                     const gchar         *domain_name,
                                                     const gchar         *domain_uri);

/* Retrieve a plug-ins help domain */
const gchar * picman_plug_in_manager_get_help_domain  (PicmanPlugInManager   *manager,
                                                     const gchar         *prog_name,
                                                     const gchar        **help_uri);

/* Retrieve all help domains */
gint          picman_plug_in_manager_get_help_domains (PicmanPlugInManager   *manager,
                                                     gchar             ***help_domains,
                                                     gchar             ***help_uris);


#endif /* __PICMAN_PLUG_IN_MANAGER_HELP_DOMAIN_H__ */
