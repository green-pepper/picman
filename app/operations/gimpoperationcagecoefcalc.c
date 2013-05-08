/* PICMAN - The GNU Image Manipulation Program
 *
 * picmanoperationcagecoefcalc.c
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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

#include "libpicmanmath/picmanmath.h"

#include "operations-types.h"

#include "picmanoperationcagecoefcalc.h"
#include "picmancageconfig.h"

#include "picman-intl.h"


static void           picman_operation_cage_coef_calc_finalize         (GObject              *object);
static void           picman_operation_cage_coef_calc_get_property     (GObject              *object,
                                                                      guint                 property_id,
                                                                      GValue               *value,
                                                                      GParamSpec           *pspec);
static void           picman_operation_cage_coef_calc_set_property     (GObject              *object,
                                                                      guint                 property_id,
                                                                      const GValue         *value,
                                                                      GParamSpec           *pspec);

static void           picman_operation_cage_coef_calc_prepare          (GeglOperation        *operation);
static GeglRectangle  picman_operation_cage_coef_calc_get_bounding_box (GeglOperation        *operation);
static gboolean       picman_operation_cage_coef_calc_process          (GeglOperation        *operation,
                                                                      GeglBuffer           *output,
                                                                      const GeglRectangle  *roi,
                                                                      gint                  level);


G_DEFINE_TYPE (PicmanOperationCageCoefCalc, picman_operation_cage_coef_calc,
               GEGL_TYPE_OPERATION_SOURCE)

#define parent_class picman_operation_cage_coef_calc_parent_class


static void
picman_operation_cage_coef_calc_class_init (PicmanOperationCageCoefCalcClass *klass)
{
  GObjectClass             *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationSourceClass *source_class    = GEGL_OPERATION_SOURCE_CLASS (klass);
  GeglOperationClass       *operation_class = GEGL_OPERATION_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:cage-coef-calc",
                                 "categories",  "transform",
                                 "description", _("Compute a set of coefficient buffer for the PICMAN cage tool"),
                                 NULL);

  operation_class->prepare            = picman_operation_cage_coef_calc_prepare;
  operation_class->get_bounding_box   = picman_operation_cage_coef_calc_get_bounding_box;
  operation_class->no_cache           = FALSE;
  operation_class->get_cached_region  = NULL;

  source_class->process               = picman_operation_cage_coef_calc_process;

  object_class->get_property          = picman_operation_cage_coef_calc_get_property;
  object_class->set_property          = picman_operation_cage_coef_calc_set_property;
  object_class->finalize              = picman_operation_cage_coef_calc_finalize;

  g_object_class_install_property (object_class,
                                   PICMAN_OPERATION_CAGE_COEF_CALC_PROP_CONFIG,
                                   g_param_spec_object ("config",
                                                        "Config",
                                                        "A PicmanCageConfig object, that define the transformation",
                                                        PICMAN_TYPE_CAGE_CONFIG,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_operation_cage_coef_calc_init (PicmanOperationCageCoefCalc *self)
{
}

static void
picman_operation_cage_coef_calc_finalize (GObject *object)
{
  PicmanOperationCageCoefCalc *self = PICMAN_OPERATION_CAGE_COEF_CALC (object);

  if (self->config)
    {
      g_object_unref (self->config);
      self->config = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_operation_cage_coef_calc_get_property (GObject    *object,
                                            guint       property_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  PicmanOperationCageCoefCalc *self = PICMAN_OPERATION_CAGE_COEF_CALC (object);

  switch (property_id)
    {
    case PICMAN_OPERATION_CAGE_COEF_CALC_PROP_CONFIG:
      g_value_set_object (value, self->config);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_cage_coef_calc_set_property (GObject      *object,
                                            guint         property_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  PicmanOperationCageCoefCalc *self = PICMAN_OPERATION_CAGE_COEF_CALC (object);

  switch (property_id)
    {
    case PICMAN_OPERATION_CAGE_COEF_CALC_PROP_CONFIG:
      if (self->config)
        g_object_unref (self->config);
      self->config = g_value_dup_object (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_operation_cage_coef_calc_is_on_straight (PicmanVector2 *d1,
                                              PicmanVector2 *d2,
                                              PicmanVector2 *p)
{
  PicmanVector2 v1, v2;
  gfloat      deter;

  v1.x = p->x - d1->x;
  v1.y = p->y - d1->y;
  v2.x = d2->x - d1->x;
  v2.y = d2->y - d1->y;

  picman_vector2_normalize (&v1);
  picman_vector2_normalize (&v2);

  deter = v1.x * v2.y - v2.x * v1.y;

  return (deter < 0.000000001) && (deter > -0.000000001);
}

static void
picman_operation_cage_coef_calc_prepare (GeglOperation *operation)
{
  PicmanOperationCageCoefCalc *occc   = PICMAN_OPERATION_CAGE_COEF_CALC (operation);
  PicmanCageConfig            *config = PICMAN_CAGE_CONFIG (occc->config);

  gegl_operation_set_format (operation,
                             "output",
                             babl_format_n (babl_type ("float"),
                                            2 * picman_cage_config_get_n_points (config)));
}

static GeglRectangle
picman_operation_cage_coef_calc_get_bounding_box (GeglOperation *operation)
{
  PicmanOperationCageCoefCalc *occc   = PICMAN_OPERATION_CAGE_COEF_CALC (operation);
  PicmanCageConfig            *config = PICMAN_CAGE_CONFIG (occc->config);

  return picman_cage_config_get_bounding_box (config);
}

static gboolean
picman_operation_cage_coef_calc_process (GeglOperation       *operation,
                                       GeglBuffer          *output,
                                       const GeglRectangle *roi,
                                       gint                 level)
{
  PicmanOperationCageCoefCalc *occc   = PICMAN_OPERATION_CAGE_COEF_CALC (operation);
  PicmanCageConfig            *config = PICMAN_CAGE_CONFIG (occc->config);

  const Babl *format = babl_format_n (babl_type ("float"), 2 * picman_cage_config_get_n_points (config));

  GeglBufferIterator *it;
  guint               n_cage_vertices;
  PicmanCagePoint      *current, *last;

  if (! config)
    return FALSE;

  n_cage_vertices   = picman_cage_config_get_n_points (config);

  it = gegl_buffer_iterator_new (output, roi, 0, format,
                                 GEGL_BUFFER_READWRITE, GEGL_ABYSS_NONE);

  while (gegl_buffer_iterator_next (it))
    {
      /* iterate inside the roi */
      gint  n_pixels = it->length;
      gint  x = it->roi->x; /* initial x                   */
      gint  y = it->roi->y; /*           and y coordinates */
      gint  j;

      gfloat      *coef = it->data[0];

      while(n_pixels--)
        {
          if (picman_cage_config_point_inside(config, x, y))
            {
              last = &(g_array_index (config->cage_points, PicmanCagePoint, 0));

              for( j = 0; j < n_cage_vertices; j++)
                {
                  PicmanVector2 v1,v2,a,b,p;
                  gdouble BA,SRT,L0,L1,A0,A1,A10,L10, Q,S,R, absa;

                  current = &(g_array_index (config->cage_points, PicmanCagePoint, (j+1) % n_cage_vertices));
                  v1 = last->src_point;
                  v2 = current->src_point;
                  p.x = x;
                  p.y = y;
                  a.x = v2.x - v1.x;
                  a.y = v2.y - v1.y;
                  absa = picman_vector2_length (&a);

                  b.x = v1.x - x;
                  b.y = v1.y - y;
                  Q = a.x * a.x + a.y * a.y;
                  S = b.x * b.x + b.y * b.y;
                  R = 2.0 * (a.x * b.x + a.y * b.y);
                  BA = b.x * a.y - b.y * a.x;
                  SRT = sqrt(4.0 * S * Q - R * R);

                  L0 = log(S);
                  L1 = log(S + Q + R);
                  A0 = atan2(R, SRT) / SRT;
                  A1 = atan2(2.0 * Q + R, SRT) / SRT;
                  A10 = A1 - A0;
                  L10 = L1 - L0;

                  /* edge coef */
                  coef[j + n_cage_vertices] = (-absa / (4.0 * G_PI)) * ((4.0*S-(R*R)/Q) * A10 + (R / (2.0 * Q)) * L10 + L1 - 2.0);

                  if (isnan(coef[j + n_cage_vertices]))
                    {
                      coef[j + n_cage_vertices] = 0.0;
                    }

                  /* vertice coef */
                  if (!picman_operation_cage_coef_calc_is_on_straight (&v1, &v2, &p))
                    {
                      coef[j] += (BA / (2.0 * G_PI)) * (L10 /(2.0*Q) - A10 * (2.0 + R / Q));
                      coef[(j+1)%n_cage_vertices] -= (BA / (2.0 * G_PI)) * (L10 / (2.0 * Q) - A10 * (R / Q));
                    }

                  last = current;
                }
            }

          coef += 2 * n_cage_vertices;

          /* update x and y coordinates */
          x++;
          if (x >= (it->roi->x + it->roi->width))
            {
              x = it->roi->x;
              y++;
            }
        }
    }

  return TRUE;
}
