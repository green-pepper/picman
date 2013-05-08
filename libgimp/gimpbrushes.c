/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * picmanbrushes.c
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

#include <string.h>

#include "picman.h"
#include "picmanbrushes.h"

/**
 * picman_brushes_get_opacity:
 *
 * This procedure is deprecated! Use picman_context_get_opacity() instead.
 *
 * Returns: The brush opacity.
 */
gdouble
picman_brushes_get_opacity (void)
{
  return picman_context_get_opacity ();
}

/**
 * picman_brushes_set_opacity:
 * @opacity: The brush opacity.
 *
 * This procedure is deprecated! Use picman_context_set_opacity() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_brushes_set_opacity (gdouble opacity)
{
  return picman_context_set_opacity (opacity);
}

/**
 * picman_brushes_get_paint_mode:
 *
 * This procedure is deprecated! Use picman_context_get_paint_mode() instead.
 *
 * Returns: The paint mode.
 */
PicmanLayerModeEffects
picman_brushes_get_paint_mode (void)
{
  return picman_context_get_paint_mode ();
}

/**
 * picman_brushes_set_paint_mode:
 * @paint_mode: The paint mode.
 *
 * This procedure is deprecated! Use picman_context_set_paint_mode() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_brushes_set_paint_mode (PicmanLayerModeEffects paint_mode)
{
  return picman_context_set_paint_mode (paint_mode);
}

/**
 * picman_brushes_set_brush:
 * @name: The brush name.
 *
 * This procedure is deprecated! Use picman_context_set_brush() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_brushes_set_brush (const gchar *name)
{
  return picman_context_set_brush (name);
}
