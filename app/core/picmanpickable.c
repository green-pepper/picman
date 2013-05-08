/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpickable.c
 * Copyright (C) 2004  Michael Natterer <mitch@picman.org>
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

/* This file contains an interface for pixel objects that their color at
 * a given position can be picked. Also included is a utility for
 * sampling an average area (which uses the implemented picking
 * functions).
 */

#include "config.h"

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picmanobject.h"
#include "picmanimage.h"
#include "picmanpickable.h"


GType
picman_pickable_interface_get_type (void)
{
  static GType pickable_iface_type = 0;

  if (! pickable_iface_type)
    {
      const GTypeInfo pickable_iface_info =
      {
        sizeof (PicmanPickableInterface),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
      };

      pickable_iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                                    "PicmanPickableInterface",
                                                    &pickable_iface_info,
                                                    0);

      g_type_interface_add_prerequisite (pickable_iface_type, PICMAN_TYPE_OBJECT);
    }

  return pickable_iface_type;
}

void
picman_pickable_flush (PicmanPickable *pickable)
{
  PicmanPickableInterface *pickable_iface;

  g_return_if_fail (PICMAN_IS_PICKABLE (pickable));

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->flush)
    pickable_iface->flush (pickable);
}

PicmanImage *
picman_pickable_get_image (PicmanPickable *pickable)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), NULL);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_image)
    return pickable_iface->get_image (pickable);

  return NULL;
}

const Babl *
picman_pickable_get_format (PicmanPickable *pickable)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), NULL);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_format)
    return pickable_iface->get_format (pickable);

  return NULL;
}

const Babl *
picman_pickable_get_format_with_alpha (PicmanPickable *pickable)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), NULL);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_format_with_alpha)
    return pickable_iface->get_format_with_alpha (pickable);

  return NULL;
}

GeglBuffer *
picman_pickable_get_buffer (PicmanPickable *pickable)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), NULL);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_buffer)
    return pickable_iface->get_buffer (pickable);

  return NULL;
}

gboolean
picman_pickable_get_pixel_at (PicmanPickable *pickable,
                            gint          x,
                            gint          y,
                            const Babl   *format,
                            gpointer      pixel)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), FALSE);
  g_return_val_if_fail (pixel != NULL, FALSE);

  if (! format)
    format = picman_pickable_get_format (pickable);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_pixel_at)
    return pickable_iface->get_pixel_at (pickable, x, y, format, pixel);

  return FALSE;
}

gboolean
picman_pickable_get_color_at (PicmanPickable *pickable,
                            gint          x,
                            gint          y,
                            PicmanRGB      *color)
{
  guchar pixel[32];

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  if (! picman_pickable_get_pixel_at (pickable, x, y, NULL, pixel))
    return FALSE;

  picman_rgba_set_pixel (color, picman_pickable_get_format (pickable), pixel);

  return TRUE;
}

gdouble
picman_pickable_get_opacity_at (PicmanPickable *pickable,
                              gint          x,
                              gint          y)
{
  PicmanPickableInterface *pickable_iface;

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), PICMAN_OPACITY_TRANSPARENT);

  pickable_iface = PICMAN_PICKABLE_GET_INTERFACE (pickable);

  if (pickable_iface->get_opacity_at)
    return pickable_iface->get_opacity_at (pickable, x, y);

  return PICMAN_OPACITY_TRANSPARENT;
}

gboolean
picman_pickable_pick_color (PicmanPickable *pickable,
                          gint          x,
                          gint          y,
                          gboolean      sample_average,
                          gdouble       average_radius,
                          PicmanRGB      *color,
                          gint         *color_index)
{
  const Babl *format;
  guchar      pixel[4];

  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), FALSE);

  format = babl_format ("R'G'B'A u8");

  if (! picman_pickable_get_pixel_at (pickable, x, y, format, pixel))
    return FALSE;

  if (sample_average)
    {
      gint count        = 0;
      gint color_avg[4] = { 0, 0, 0, 0 };
      gint radius       = (gint) average_radius;
      gint i, j;

      for (i = x - radius; i <= x + radius; i++)
        for (j = y - radius; j <= y + radius; j++)
          if (picman_pickable_get_pixel_at (pickable, i, j, format, pixel))
            {
              count++;

              color_avg[RED]   += pixel[RED];
              color_avg[GREEN] += pixel[GREEN];
              color_avg[BLUE]  += pixel[BLUE];
              color_avg[ALPHA] += pixel[ALPHA];
            }

      pixel[RED]   = (guchar) ((color_avg[RED]   + count / 2) / count);
      pixel[GREEN] = (guchar) ((color_avg[GREEN] + count / 2) / count);
      pixel[BLUE]  = (guchar) ((color_avg[BLUE]  + count / 2) / count);
      pixel[ALPHA] = (guchar) ((color_avg[ALPHA] + count / 2) / count);
    }

  picman_rgba_set_uchar (color,
                       pixel[RED],
                       pixel[GREEN],
                       pixel[BLUE],
                       pixel[ALPHA]);

  if (color_index)
    {
      format = picman_pickable_get_format (pickable);

      if (babl_format_is_palette (format) && ! sample_average)
        {
          picman_pickable_get_pixel_at (pickable, x, y, format, pixel);

          *color_index = pixel[0];
        }
      else
        {
          *color_index = -1;
        }
    }

  return TRUE;
}
