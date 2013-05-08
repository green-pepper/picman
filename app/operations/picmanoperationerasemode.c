/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationerasemode.c
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
 *               2012 Ville Sokk <ville.sokk@gmail.com>
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

#include "picmanoperationerasemode.h"


static void prepare (GeglOperation *operation);
static gboolean picman_operation_erase_mode_process (GeglOperation       *operation,
                                                   void                *in_buf,
                                                   void                *aux_buf,
                                                   void                *aux2_buf,
                                                   void                *out_buf,
                                                   glong                samples,
                                                   const GeglRectangle *roi,
                                                   gint                 level);


G_DEFINE_TYPE (PicmanOperationEraseMode, picman_operation_erase_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)


static void
picman_operation_erase_mode_class_init (PicmanOperationEraseModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:erase-mode",
                                 "description", "PICMAN erase mode operation",
                                 NULL);

  operation_class->prepare = prepare;
  point_class->process         = picman_operation_erase_mode_process;
}

static void
picman_operation_erase_mode_init (PicmanOperationEraseMode *self)
{
}

static void
prepare (GeglOperation *operation)
{
  const Babl *format = babl_format ("RGBA float");

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "aux",    format);
  gegl_operation_set_format (operation, "aux2",   babl_format ("Y float"));
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
picman_operation_erase_mode_process (GeglOperation       *operation,
                                   void                *in_buf,
                                   void                *aux_buf,
                                   void                *aux2_buf,
                                   void                *out_buf,
                                   glong                samples,
                                   const GeglRectangle *roi,
                                   gint                 level)
{
  PicmanOperationPointLayerMode *point    = PICMAN_OPERATION_POINT_LAYER_MODE (operation);
  gdouble                      opacity  = point->opacity;
  gfloat                      *in       = in_buf;
  gfloat                      *layer    = aux_buf;
  gfloat                      *mask     = aux2_buf;
  gfloat                      *out      = out_buf;
  const gboolean               has_mask = mask != NULL;

  while (samples--)
    {
      gint    b;
      gdouble value = opacity;

      if (has_mask)
        value *= (*mask);

      for (b = RED; b < ALPHA; b++)
        {
          out[b] = in[b];
        }

      out[ALPHA] = in[ALPHA] - in[ALPHA] * layer[ALPHA] * value;

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask ++;
    }

  return TRUE;
}
