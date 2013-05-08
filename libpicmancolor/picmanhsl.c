/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#include "picmanhsl.h"


/*
 * PICMAN_TYPE_HSL
 */

static PicmanHSL * picman_hsl_copy (const PicmanHSL *hsl);


GType
picman_hsl_get_type (void)
{
  static GType hsl_type = 0;

  if (!hsl_type)
    hsl_type = g_boxed_type_register_static ("PicmanHSL",
                                              (GBoxedCopyFunc) picman_hsl_copy,
                                              (GBoxedFreeFunc) g_free);

  return hsl_type;
}

static PicmanHSL *
picman_hsl_copy (const PicmanHSL *hsl)
{
  return g_memdup (hsl, sizeof (PicmanHSL));
}


/*  HSL functions  */

/**
 * picman_hsl_set:
 * @hsl:
 * @h:
 * @s:
 * @l:
 *
 * Since: PICMAN 2.8
 **/
void
picman_hsl_set (PicmanHSL *hsl,
              gdouble  h,
              gdouble  s,
              gdouble  l)
{
  g_return_if_fail (hsl != NULL);

  hsl->h = h;
  hsl->s = s;
  hsl->l = l;
}
