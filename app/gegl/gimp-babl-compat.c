/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-babl-compat.h
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#include "picman-gegl-types.h"

#include "picman-babl.h"
#include "picman-babl-compat.h"


PicmanImageType
picman_babl_format_get_image_type (const Babl *format)
{
  const Babl *model;

  g_return_val_if_fail (format != NULL, -1);

  model = babl_format_get_model (format);

  if (model == babl_model ("Y") ||
      model == babl_model ("Y'"))
    {
      return PICMAN_GRAY_IMAGE;
    }
  else if (model == babl_model ("YA") ||
           model == babl_model ("Y'A"))
    {
      return PICMAN_GRAYA_IMAGE;
    }
  else if (model == babl_model ("RGB") ||
           model == babl_model ("R'G'B'"))
    {
      return PICMAN_RGB_IMAGE;
    }
  else if (model == babl_model ("RGBA") ||
           model == babl_model ("R'G'B'A"))
    {
      return PICMAN_RGBA_IMAGE;
    }
  else if (babl_format_is_palette (format))
    {
      if (babl_format_has_alpha (format))
        return PICMAN_INDEXEDA_IMAGE;
      else
        return PICMAN_INDEXED_IMAGE;
    }

  g_return_val_if_reached (-1);
}

const Babl *
picman_babl_compat_u8_format (const Babl *format)
{
  g_return_val_if_fail (format != NULL, NULL);

  /*  indexed images only exist in u8, return the same format  */
  if (babl_format_is_palette (format))
    return format;

  return picman_babl_format (picman_babl_format_get_base_type (format),
                           PICMAN_PRECISION_U8,
                           babl_format_has_alpha (format));
}
