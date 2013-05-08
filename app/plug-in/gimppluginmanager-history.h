/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-history.h
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

#ifndef __PICMAN_PLUG_IN_MANAGER_HISTORY_H__
#define __PICMAN_PLUG_IN_MANAGER_HISTORY_H__


guint                 picman_plug_in_manager_history_size   (PicmanPlugInManager   *manager);
guint                 picman_plug_in_manager_history_length (PicmanPlugInManager   *manager);
PicmanPlugInProcedure * picman_plug_in_manager_history_nth    (PicmanPlugInManager   *manager,
                                                           guint                n);
void                  picman_plug_in_manager_history_add    (PicmanPlugInManager   *manager,
                                                           PicmanPlugInProcedure *procedure);
void                  picman_plug_in_manager_history_remove (PicmanPlugInManager   *manager,
                                                           PicmanPlugInProcedure *procedure);
void                  picman_plug_in_manager_history_clear  (PicmanPlugInManager   *manager);


#endif /* __PICMAN_PLUG_IN_MANAGER_HISTORY_H__ */
