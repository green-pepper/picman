/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockcontainer.h
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef __PICMAN_DOCK_CONTAINER_H__
#define __PICMAN_DOCK_CONTAINER_H__


#define PICMAN_TYPE_DOCK_CONTAINER               (picman_dock_container_interface_get_type ())
#define PICMAN_DOCK_CONTAINER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCK_CONTAINER, PicmanDockContainer))
#define PICMAN_IS_DOCK_CONTAINER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCK_CONTAINER))
#define PICMAN_DOCK_CONTAINER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_DOCK_CONTAINER, PicmanDockContainerInterface))


typedef struct _PicmanDockContainerInterface PicmanDockContainerInterface;

struct _PicmanDockContainerInterface
{
  GTypeInterface base_iface;

  /*  virtual functions  */
  GList           * (* get_docks)         (PicmanDockContainer   *container);
  PicmanUIManager   * (* get_ui_manager)    (PicmanDockContainer   *container);
  void              (* add_dock)          (PicmanDockContainer   *container,
                                           PicmanDock            *dock,
                                           PicmanSessionInfoDock *dock_info);
  PicmanAlignmentType (* get_dock_side)     (PicmanDockContainer   *container,
                                           PicmanDock            *dock);
};


GType              picman_dock_container_interface_get_type  (void) G_GNUC_CONST;
GList            * picman_dock_container_get_docks           (PicmanDockContainer   *container);
PicmanUIManager    * picman_dock_container_get_ui_manager      (PicmanDockContainer   *container);
void               picman_dock_container_add_dock            (PicmanDockContainer   *container,
                                                            PicmanDock            *dock,
                                                            PicmanSessionInfoDock *dock_info);
PicmanAlignmentType  picman_dock_container_get_dock_side       (PicmanDockContainer   *container,
                                                            PicmanDock            *dock);


#endif  /*  __PICMAN_DOCK_CONTAINER_H__  */
