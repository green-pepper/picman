/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2005 Peter Mattis and Spencer Kimball
 *
 * picmanpicmanrc.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "picman.h"


/**
 * picman_get_color_configuration:
 *
 * Retrieve a copy of the current color management configuration.
 *
 * Returns: A copy of the core's #PicmanColorConfig. You should unref
 *          this copy if you don't need it any longer.
 *
 * Since: PICMAN 2.4
 */
PicmanColorConfig *
picman_get_color_configuration (void)
{
  PicmanColorConfig *config;
  gchar           *text;
  GError          *error = NULL;

  text = _picman_get_color_configuration ();

  g_return_val_if_fail (text != NULL, NULL);

  config = g_object_new (PICMAN_TYPE_COLOR_CONFIG, NULL);

  if (! picman_config_deserialize_string (PICMAN_CONFIG (config), text, -1, NULL,
                                        &error))
    {
      g_warning ("failed to deserialize color configuration: %s",
                 error->message);
      g_error_free (error);
    }

  g_free (text);

  return config;
}
