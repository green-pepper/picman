/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextLayer
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#include "text-types.h"

#include "core/picman-transform-utils.h"
#include "core/picmanimage-undo.h"

#include "picmantext.h"
#include "picmantextlayer.h"
#include "picmantextlayer-transform.h"


static PicmanItemClass * picman_text_layer_parent_class (void) G_GNUC_CONST;

static gboolean  picman_text_layer_get_transformation  (PicmanTextLayer *layer,
                                                      PicmanMatrix3   *matrix);
static gboolean  picman_text_layer_set_transformation  (PicmanTextLayer *layer,
                                                      PicmanMatrix3   *matrix);


void
picman_text_layer_scale (PicmanItem               *item,
                       gint                    new_width,
                       gint                    new_height,
                       gint                    new_offset_x,
                       gint                    new_offset_y,
                       PicmanInterpolationType   interpolation_type,
                       PicmanProgress           *progress)
{
  /*  TODO  */
}

static gboolean
picman_text_layer_transform_flip (PicmanTextLayer       *layer,
                                PicmanOrientationType  flip_type,
                                gdouble              axis)
{
  PicmanMatrix3  matrix;

  if (! picman_text_layer_get_transformation (layer, &matrix))
    return FALSE;

  picman_transform_matrix_flip (&matrix, flip_type, axis);

  return picman_text_layer_set_transformation (layer, &matrix);
}

void
picman_text_layer_flip (PicmanItem            *item,
                      PicmanContext         *context,
                      PicmanOrientationType  flip_type,
                      gdouble              axis,
                      gboolean             clip_result)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (item);

  if (picman_text_layer_transform_flip (layer, flip_type, axis))
    {
      PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (layer));

      if (mask)
        picman_item_flip (PICMAN_ITEM (mask), context,
                        flip_type, axis, clip_result);
    }
  else
    {
      picman_text_layer_parent_class ()->flip (item, context,
                                             flip_type, axis, clip_result);
    }
}

static gboolean
picman_text_layer_transform_rotate (PicmanTextLayer    *layer,
                                  PicmanRotationType  rotate_type,
                                  gdouble           center_x,
                                  gdouble           center_y)
{
  PicmanMatrix3  matrix;

  if (! picman_text_layer_get_transformation (layer, &matrix))
    return FALSE;

  picman_transform_matrix_rotate (&matrix, rotate_type, center_x, center_y);

  return picman_text_layer_set_transformation (layer, &matrix);
}

void
picman_text_layer_rotate (PicmanItem         *item,
                        PicmanContext      *context,
                        PicmanRotationType  rotate_type,
                        gdouble           center_x,
                        gdouble           center_y,
                        gboolean          clip_result)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (item);

  if (! picman_text_layer_transform_rotate (layer,
                                          rotate_type, center_x, center_y))
    {
      PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (layer));

      if (mask)
        picman_item_rotate (PICMAN_ITEM (mask), context,
                          rotate_type, center_x, center_y, clip_result);
    }
  else
    {
      picman_text_layer_parent_class ()->rotate (item, context,
                                               rotate_type, center_x, center_y,
                                               clip_result);
    }
}

void
picman_text_layer_transform (PicmanItem               *item,
                           PicmanContext            *context,
                           const PicmanMatrix3      *matrix,
                           PicmanTransformDirection  direction,
                           PicmanInterpolationType   interpolation_type,
                           gboolean                supersample,
                           gint                    recursion_level,
                           PicmanTransformResize     clip_result,
                           PicmanProgress           *progress)
{
  /*  TODO  */
}

static PicmanItemClass *
picman_text_layer_parent_class (void)
{
  static PicmanItemClass *parent_class = NULL;

  if (! parent_class)
    {
      gpointer klass = g_type_class_peek (PICMAN_TYPE_TEXT_LAYER);

      parent_class = g_type_class_peek_parent (klass);
    }

  return parent_class;
}

static gboolean
picman_text_layer_get_transformation (PicmanTextLayer *layer,
                                    PicmanMatrix3   *matrix)
{
  if (! layer->text || layer->modified)
    return FALSE;

  picman_text_get_transformation (layer->text, matrix);

  return TRUE;
}

static gboolean
picman_text_layer_set_transformation (PicmanTextLayer *layer,
                                    PicmanMatrix3   *matrix)
{
  PicmanMatrix2  trafo;

  if (! picman_matrix3_is_affine (matrix))
    return FALSE;

  trafo.coeff[0][0] = matrix->coeff[0][0];
  trafo.coeff[0][1] = matrix->coeff[0][1];
  trafo.coeff[1][0] = matrix->coeff[1][0];
  trafo.coeff[1][1] = matrix->coeff[1][1];

  picman_text_layer_set (PICMAN_TEXT_LAYER (layer), NULL,
                       "transformation", &trafo,
                       "offset-x",       matrix->coeff[0][2],
                       "offset-y",       matrix->coeff[1][2],
                       NULL);

  return TRUE;
}
