/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmananchor.c
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#include <glib-object.h>

#include "vectors-types.h"

#include "picmananchor.h"


GType
picman_anchor_get_type (void)
{
  static GType anchor_type = 0;

  if (!anchor_type)
    anchor_type = g_boxed_type_register_static ("PicmanAnchor",
                                                (GBoxedCopyFunc) picman_anchor_copy,
                                                (GBoxedFreeFunc) picman_anchor_free);

  return anchor_type;
}

PicmanAnchor *
picman_anchor_new (PicmanAnchorType    type,
                 const PicmanCoords *position)
{
  PicmanAnchor *anchor = g_slice_new0 (PicmanAnchor);

  anchor->type = type;

  if (position)
    anchor->position = *position;

  return anchor;
}

PicmanAnchor *
picman_anchor_copy (const PicmanAnchor *anchor)
{
  g_return_val_if_fail (anchor != NULL, NULL);

  return g_slice_dup (PicmanAnchor, anchor);
}

void
picman_anchor_free (PicmanAnchor *anchor)
{
  g_return_if_fail (anchor != NULL);

  g_slice_free (PicmanAnchor, anchor);
}
