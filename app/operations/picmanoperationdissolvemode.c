/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationdissolvemode.c
 * Copyright (C) 2012 Ville Sokk <ville.sokk@gmail.com>
 *               2012 Øyvind Kolås <pippin@picman.org>
 *               2003 Helvetix Victorinox
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gegl-plugin.h>

#include "operations-types.h"

#include "picmanoperationdissolvemode.h"


#define RANDOM_TABLE_SIZE 4096


static gboolean picman_operation_dissolve_mode_process (GeglOperation       *operation,
                                                      void                *in_buf,
                                                      void                *aux_buf,
                                                      void                *aux2_buf,
                                                      void                *out_buf,
                                                      glong                samples,
                                                      const GeglRectangle *result,
                                                      gint                 level);


G_DEFINE_TYPE (PicmanOperationDissolveMode, picman_operation_dissolve_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)

static gint32 random_table[RANDOM_TABLE_SIZE];


static void
picman_operation_dissolve_mode_class_init (PicmanOperationDissolveModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_composer_class;
  GRand                            *gr;
  gint                              i;

  operation_class      = GEGL_OPERATION_CLASS (klass);
  point_composer_class = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:dissolve-mode",
                                 "description", "PICMAN dissolve mode operation",
                                 "categories",  "compositors",
                                 NULL);

  point_composer_class->process = picman_operation_dissolve_mode_process;

  /* generate a table of random seeds */
  gr = g_rand_new_with_seed (314159265);
  for (i = 0; i < RANDOM_TABLE_SIZE; i++)
    random_table[i] = g_rand_int (gr);

  g_rand_free (gr);
}

static void
picman_operation_dissolve_mode_init (PicmanOperationDissolveMode *self)
{
}

static gboolean
picman_operation_dissolve_mode_process (GeglOperation       *operation,
                                      void                *in_buf,
                                      void                *aux_buf,
                                      void                *aux2_buf,
                                      void                *out_buf,
                                      glong                samples,
                                      const GeglRectangle *result,
                                      gint                 level)
{
  gdouble         opacity  = PICMAN_OPERATION_POINT_LAYER_MODE (operation)->opacity;
  gfloat         *in       = in_buf;
  gfloat         *out      = out_buf;
  gfloat         *aux      = aux_buf;
  gfloat         *mask     = aux2_buf;
  const gboolean  has_mask = mask != NULL;
  gint            x, y;

  for (y = result->y; y < result->y + result->height; y++)
    {
      GRand *gr = g_rand_new_with_seed (random_table[y % RANDOM_TABLE_SIZE]);

      /* fast forward through the rows pseudo random sequence */
      for (x = 0; x < result->x; x++)
        g_rand_int (gr);

      for (x = result->x; x < result->x + result->width; x++)
        {
          gfloat value = aux[ALPHA] * opacity * 255;

          if (has_mask)
            value *= *mask;

          if (g_rand_int_range (gr, 0, 255) >= value)
            {
              out[0] = in[0];
              out[1] = in[1];
              out[2] = in[2];
              out[3] = in[3];
            }
          else
            {
              out[0] = aux[0];
              out[1] = aux[1];
              out[2] = aux[2];
              out[3] = 1.0;
            }

          in   += 4;
          out  += 4;
          aux  += 4;

          if (has_mask)
            mask ++;
        }
      g_rand_free (gr);
    }

  return TRUE;
}
