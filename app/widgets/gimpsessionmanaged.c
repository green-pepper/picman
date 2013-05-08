/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessionmanaged.c
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

#include "picmansessionmanaged.h"


static void   picman_session_managed_iface_base_init   (PicmanSessionManagedInterface *managed_iface);


GType
picman_session_managed_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
    {
      const GTypeInfo iface_info =
      {
        sizeof (PicmanSessionManagedInterface),
        (GBaseInitFunc)     picman_session_managed_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                           "PicmanSessionManagedInterface",
                                           &iface_info,
                                           0);

      g_type_interface_add_prerequisite (iface_type, GTK_TYPE_WIDGET);
    }

  return iface_type;
}

static void
picman_session_managed_iface_base_init (PicmanSessionManagedInterface *managed_iface)
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  initialized = TRUE;

  managed_iface->get_aux_info = NULL;
  managed_iface->set_aux_info = NULL;
}

/**
 * picman_session_managed_get_aux_info:
 * @session_managed: A #PicmanSessionManaged
 *
 * Returns: A list of #PicmanSessionInfoAux created with
 *          picman_session_info_aux_new().
 **/
GList *
picman_session_managed_get_aux_info (PicmanSessionManaged *session_managed)
{
  PicmanSessionManagedInterface *iface;

  g_return_val_if_fail (PICMAN_IS_SESSION_MANAGED (session_managed), NULL);

  iface = PICMAN_SESSION_MANAGED_GET_INTERFACE (session_managed);

  if (iface->get_aux_info)
    return iface->get_aux_info (session_managed);

  return NULL;
}

/**
 * picman_session_managed_get_ui_manager:
 * @session_managed: A #PicmanSessionManaged
 * @aux_info         A list of #PicmanSessionInfoAux
 *
 * Sets aux data previously returned from
 * picman_session_managed_get_aux_info().
 **/
void
picman_session_managed_set_aux_info (PicmanSessionManaged *session_managed,
                                   GList              *aux_info)
{
  PicmanSessionManagedInterface *iface;

  g_return_if_fail (PICMAN_IS_SESSION_MANAGED (session_managed));

  iface = PICMAN_SESSION_MANAGED_GET_INTERFACE (session_managed);

  if (iface->set_aux_info)
    iface->set_aux_info (session_managed, aux_info);
}
