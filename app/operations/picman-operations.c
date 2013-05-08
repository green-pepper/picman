/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-operations.c
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

#include "operations-types.h"

#include "core/picman.h"

#include "picman-operations.h"

#include "picmanoperationborder.h"
#include "picmanoperationcagecoefcalc.h"
#include "picmanoperationcagetransform.h"
#include "picmanoperationequalize.h"
#include "picmanoperationgrow.h"
#include "picmanoperationhistogramsink.h"
#include "picmanoperationmaskcomponents.h"
#include "picmanoperationsemiflatten.h"
#include "picmanoperationsetalpha.h"
#include "picmanoperationshapeburst.h"
#include "picmanoperationshrink.h"
#include "picmanoperationthresholdalpha.h"

#include "picmanoperationbrightnesscontrast.h"
#include "picmanoperationcolorbalance.h"
#include "picmanoperationcolorize.h"
#include "picmanoperationcurves.h"
#include "picmanoperationdesaturate.h"
#include "picmanoperationhuesaturation.h"
#include "picmanoperationlevels.h"
#include "picmanoperationposterize.h"
#include "picmanoperationthreshold.h"

#include "picmanoperationpointlayermode.h"
#include "picmanoperationnormalmode.h"
#include "picmanoperationdissolvemode.h"
#include "picmanoperationbehindmode.h"
#include "picmanoperationmultiplymode.h"
#include "picmanoperationscreenmode.h"
#include "picmanoperationoverlaymode.h"
#include "picmanoperationdifferencemode.h"
#include "picmanoperationadditionmode.h"
#include "picmanoperationsubtractmode.h"
#include "picmanoperationdarkenonlymode.h"
#include "picmanoperationlightenonlymode.h"
#include "picmanoperationhuemode.h"
#include "picmanoperationsaturationmode.h"
#include "picmanoperationcolormode.h"
#include "picmanoperationvaluemode.h"
#include "picmanoperationdividemode.h"
#include "picmanoperationdodgemode.h"
#include "picmanoperationburnmode.h"
#include "picmanoperationhardlightmode.h"
#include "picmanoperationsoftlightmode.h"
#include "picmanoperationgrainextractmode.h"
#include "picmanoperationgrainmergemode.h"
#include "picmanoperationcolorerasemode.h"
#include "picmanoperationerasemode.h"
#include "picmanoperationreplacemode.h"
#include "picmanoperationantierasemode.h"


void
picman_operations_init (void)
{
  g_type_class_ref (PICMAN_TYPE_OPERATION_BORDER);
  g_type_class_ref (PICMAN_TYPE_OPERATION_CAGE_COEF_CALC);
  g_type_class_ref (PICMAN_TYPE_OPERATION_CAGE_TRANSFORM);
  g_type_class_ref (PICMAN_TYPE_OPERATION_EQUALIZE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_GROW);
  g_type_class_ref (PICMAN_TYPE_OPERATION_HISTOGRAM_SINK);
  g_type_class_ref (PICMAN_TYPE_OPERATION_MASK_COMPONENTS);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SEMI_FLATTEN);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SET_ALPHA);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SHAPEBURST);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SHRINK);
  g_type_class_ref (PICMAN_TYPE_OPERATION_THRESHOLD_ALPHA);

  g_type_class_ref (PICMAN_TYPE_OPERATION_BRIGHTNESS_CONTRAST);
  g_type_class_ref (PICMAN_TYPE_OPERATION_COLOR_BALANCE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_COLORIZE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_CURVES);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DESATURATE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_HUE_SATURATION);
  g_type_class_ref (PICMAN_TYPE_OPERATION_LEVELS);
  g_type_class_ref (PICMAN_TYPE_OPERATION_POSTERIZE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_THRESHOLD);

  g_type_class_ref (PICMAN_TYPE_OPERATION_POINT_LAYER_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_NORMAL_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DISSOLVE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_BEHIND_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_MULTIPLY_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SCREEN_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_OVERLAY_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DIFFERENCE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_ADDITION_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SUBTRACT_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DARKEN_ONLY_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_LIGHTEN_ONLY_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_HUE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SATURATION_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_COLOR_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_VALUE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DIVIDE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_DODGE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_BURN_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_HARDLIGHT_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_SOFTLIGHT_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_GRAIN_EXTRACT_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_GRAIN_MERGE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_COLOR_ERASE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_ERASE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_REPLACE_MODE);
  g_type_class_ref (PICMAN_TYPE_OPERATION_ANTI_ERASE_MODE);
}
