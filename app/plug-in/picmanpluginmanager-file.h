/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-file.h
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

#ifndef __PICMAN_PLUG_IN_MANAGER_FILE_H__
#define __PICMAN_PLUG_IN_MANAGER_FILE_H__


gboolean   picman_plug_in_manager_register_load_handler (PicmanPlugInManager *manager,
                                                       const gchar       *name,
                                                       const gchar       *extensions,
                                                       const gchar       *prefixes,
                                                       const gchar       *magics);
gboolean   picman_plug_in_manager_register_save_handler (PicmanPlugInManager *manager,
                                                       const gchar       *name,
                                                       const gchar       *extensions,
                                                       const gchar       *prefixes);

gboolean   picman_plug_in_manager_register_mime_type    (PicmanPlugInManager *manager,
                                                       const gchar       *name,
                                                       const gchar       *mime_type);

gboolean   picman_plug_in_manager_register_handles_uri  (PicmanPlugInManager *manager,
                                                       const gchar       *name);

gboolean   picman_plug_in_manager_register_thumb_loader (PicmanPlugInManager *manager,
                                                       const gchar       *load_proc,
                                                       const gchar       *thumb_proc);
gboolean   picman_plug_in_manager_uri_has_exporter      (PicmanPlugInManager *manager,
                                                       const gchar       *uri);


#endif /* __PICMAN_PLUG_IN_MANAGER_FILE_H__ */
