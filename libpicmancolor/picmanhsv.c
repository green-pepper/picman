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

#include "picmanhsv.h"


/**
 * SECTION: picmanhsv
 * @title: PicmanHSV
 * @short_description: Definitions and Functions relating to HSV colors.
 *
 * Definitions and Functions relating to HSV colors.
 **/


/*
 * PICMAN_TYPE_HSV
 */

static PicmanHSV * picman_hsv_copy (const PicmanHSV *hsv);


GType
picman_hsv_get_type (void)
{
  static GType hsv_type = 0;

  if (!hsv_type)
    hsv_type = g_boxed_type_register_static ("PicmanHSV",
                                              (GBoxedCopyFunc) picman_hsv_copy,
                                              (GBoxedFreeFunc) g_free);

  return hsv_type;
}

static PicmanHSV *
picman_hsv_copy (const PicmanHSV *hsv)
{
  return g_memdup (hsv, sizeof (PicmanHSV));
}


/*  HSV functions  */

void
picman_hsv_set (PicmanHSV *hsv,
              gdouble  h,
              gdouble  s,
              gdouble  v)
{
  g_return_if_fail (hsv != NULL);

  hsv->h = h;
  hsv->s = s;
  hsv->v = v;
}

void
picman_hsv_clamp (PicmanHSV *hsv)
{
  g_return_if_fail (hsv != NULL);

  hsv->h -= (gint) hsv->h;

  if (hsv->h < 0)
    hsv->h += 1.0;

  hsv->s = CLAMP (hsv->s, 0.0, 1.0);
  hsv->v = CLAMP (hsv->v, 0.0, 1.0);
  hsv->a = CLAMP (hsv->a, 0.0, 1.0);
}

void
picman_hsva_set (PicmanHSV *hsva,
               gdouble  h,
               gdouble  s,
               gdouble  v,
               gdouble  a)
{
  g_return_if_fail (hsva != NULL);

  hsva->h = h;
  hsva->s = s;
  hsva->v = v;
  hsva->a = a;
}
