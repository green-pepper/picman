/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockcontainer.c
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

#include "config.h"

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "picmandockcontainer.h"


static void   picman_dock_container_iface_base_init   (PicmanDockContainerInterface *container_iface);


GType
picman_dock_container_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
    {
      const GTypeInfo iface_info =
      {
        sizeof (PicmanDockContainerInterface),
        (GBaseInitFunc)     picman_dock_container_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                           "PicmanDockContainerInterface",
                                           &iface_info,
                                           0);

      g_type_interface_add_prerequisite (iface_type, GTK_TYPE_WIDGET);
    }

  return iface_type;
}

static void
picman_dock_container_iface_base_init (PicmanDockContainerInterface *container_iface)
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  initialized = TRUE;

  container_iface->get_docks = NULL;
}

/**
 * picman_dock_container_get_docks:
 * @container: A #PicmanDockContainer
 *
 * Returns: A list of #PicmanDock:s in the dock container. Free with
 *          g_list_free() when done.
 **/
GList *
picman_dock_container_get_docks (PicmanDockContainer *container)
{
  PicmanDockContainerInterface *iface;

  g_return_val_if_fail (PICMAN_IS_DOCK_CONTAINER (container), NULL);

  iface = PICMAN_DOCK_CONTAINER_GET_INTERFACE (container);

  if (iface->get_docks)
    return iface->get_docks (container);

  return NULL;
}

/**
 * picman_dock_container_get_ui_manager:
 * @container: A #PicmanDockContainer
 *
 * Returns: The #PicmanUIManager of the #PicmanDockContainer
 **/
PicmanUIManager *
picman_dock_container_get_ui_manager (PicmanDockContainer *container)
{
  PicmanDockContainerInterface *iface;

  g_return_val_if_fail (PICMAN_IS_DOCK_CONTAINER (container), NULL);

  iface = PICMAN_DOCK_CONTAINER_GET_INTERFACE (container);

  if (iface->get_ui_manager)
    return iface->get_ui_manager (container);

  return NULL;
}

/**
 * picman_dock_container_add_dock:
 * @container: A #PicmanDockContainer
 * @dock:      The newly created #PicmanDock to add to the container.
 * @dock_info: The #PicmanSessionInfoDock the @dock was created from.
 *
 * Add @dock that was created from @dock_info to @container.
 **/
void
picman_dock_container_add_dock (PicmanDockContainer   *container,
                              PicmanDock            *dock,
                              PicmanSessionInfoDock *dock_info)
{
  PicmanDockContainerInterface *iface;

  g_return_if_fail (PICMAN_IS_DOCK_CONTAINER (container));

  iface = PICMAN_DOCK_CONTAINER_GET_INTERFACE (container);

  if (iface->add_dock)
    iface->add_dock (container,
                     dock,
                     dock_info);
}

/**
 * picman_dock_container_get_dock_side:
 * @container: A #PicmanDockContainer
 * @dock:      A #PicmanDock
 *
 * Returns: What side @dock is in in @container, either
 *          PICMAN_ALIGN_LEFT or PICMAN_ALIGN_RIGHT, or -1 if the side
 *          concept is not applicable.
 **/
PicmanAlignmentType
picman_dock_container_get_dock_side (PicmanDockContainer   *container,
                                   PicmanDock            *dock)
{
  PicmanDockContainerInterface *iface;

  g_return_val_if_fail (PICMAN_IS_DOCK_CONTAINER (container), -1);

  iface = PICMAN_DOCK_CONTAINER_GET_INTERFACE (container);

  if (iface->get_dock_side)
    return iface->get_dock_side (container, dock);

  return -1;
}
