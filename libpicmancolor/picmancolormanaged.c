/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * PicmanColorManaged interface
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib-object.h>

#include "picmancolortypes.h"

#include "picmancolormanaged.h"


/**
 * SECTION: picmancolormanaged
 * @title: PicmanColorManaged
 * @short_description: An interface dealing with color profiles.
 *
 * An interface dealing with color profiles.
 **/


enum
{
  PROFILE_CHANGED,
  LAST_SIGNAL
};


static void  picman_color_managed_base_init (PicmanColorManagedInterface *iface);


static guint picman_color_managed_signals[LAST_SIGNAL] = { 0 };


GType
picman_color_managed_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
    {
      const GTypeInfo iface_info =
      {
        sizeof (PicmanColorManagedInterface),
        (GBaseInitFunc)     picman_color_managed_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                           "PicmanColorManagedInterface",
                                           &iface_info, 0);

      g_type_interface_add_prerequisite (iface_type, G_TYPE_OBJECT);
    }

  return iface_type;
}

static void
picman_color_managed_base_init (PicmanColorManagedInterface *iface)
{
  static gboolean initialized = FALSE;

  if (! initialized)
    {
      picman_color_managed_signals[PROFILE_CHANGED] =
        g_signal_new ("profile-changed",
                      G_TYPE_FROM_INTERFACE (iface),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (PicmanColorManagedInterface,
                                       profile_changed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      iface->get_icc_profile = NULL;
      iface->profile_changed = NULL;

      initialized = TRUE;
    }
}

/**
 * picman_color_managed_get_icc_profile:
 * @managed: an object the implements the #PicmanColorManaged interface
 * @len:     return location for the number of bytes in the profile data
 *
 * Return value: A pointer to a blob of data that represents an ICC
 *               color profile.
 *
 * Since: PICMAN 2.4
 **/
const guint8 *
picman_color_managed_get_icc_profile (PicmanColorManaged *managed,
                                    gsize            *len)
{
  PicmanColorManagedInterface *iface;

  g_return_val_if_fail (PICMAN_IS_COLOR_MANAGED (managed), NULL);
  g_return_val_if_fail (len != NULL, NULL);

  *len = 0;

  iface = PICMAN_COLOR_MANAGED_GET_INTERFACE (managed);

  if (iface->get_icc_profile)
    return iface->get_icc_profile (managed, len);

  return NULL;
}

/**
 * picman_color_managed_profile_changed:
 * @managed: an object the implements the #PicmanColorManaged interface
 *
 * Emits the "profile-changed" signal.
 *
 * Since: PICMAN 2.4
 **/
void
picman_color_managed_profile_changed (PicmanColorManaged *managed)
{
  g_return_if_fail (PICMAN_IS_COLOR_MANAGED (managed));

  g_signal_emit (managed, picman_color_managed_signals[PROFILE_CHANGED], 0);
}
