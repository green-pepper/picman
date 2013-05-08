/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <gegl.h>

#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picmandynamics.h"
#include "picmandynamics-save.h"


gboolean
picman_dynamics_save (PicmanData  *data,
                    GError   **error)
{
  g_return_val_if_fail (PICMAN_IS_DYNAMICS (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return picman_config_serialize_to_file (PICMAN_CONFIG (data),
                                        picman_data_get_filename (data),
                                        "PICMAN dynamics file",
                                        "end of PICMAN dynamics file",
                                        NULL, error);
}
