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

#include <glib-object.h>

#include "core-types.h"

#include "picmansamplepoint.h"


GType
picman_sample_point_get_type (void)
{
  static GType type = 0;

  if (! type)
    type = g_boxed_type_register_static ("PicmanSamplePoint",
                                         (GBoxedCopyFunc) picman_sample_point_ref,
                                         (GBoxedFreeFunc) picman_sample_point_unref);

  return type;
}

PicmanSamplePoint *
picman_sample_point_new (guint32 sample_point_ID)
{
  PicmanSamplePoint *sample_point;

  sample_point = g_slice_new0 (PicmanSamplePoint);

  sample_point->ref_count       = 1;
  sample_point->sample_point_ID = sample_point_ID;
  sample_point->x               = -1;
  sample_point->y               = -1;

  return sample_point;
}

PicmanSamplePoint *
picman_sample_point_ref (PicmanSamplePoint *sample_point)
{
  g_return_val_if_fail (sample_point != NULL, NULL);

  sample_point->ref_count++;

  return sample_point;
}

void
picman_sample_point_unref (PicmanSamplePoint *sample_point)
{
  g_return_if_fail (sample_point != NULL);

  sample_point->ref_count--;

  if (sample_point->ref_count < 1)
    g_slice_free (PicmanSamplePoint, sample_point);
}
