/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * operations-types.h
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

#ifndef __OPERATIONS_TYPES_H__
#define __OPERATIONS_TYPES_H__


#include "gegl/picman-gegl-types.h"


/*  operations  */

typedef struct _PicmanOperationPointFilter        PicmanOperationPointFilter;
typedef struct _PicmanOperationPointLayerMode     PicmanOperationPointLayerMode;


/*  operation config objects  */

typedef struct _PicmanBrightnessContrastConfig    PicmanBrightnessContrastConfig;
typedef struct _PicmanCageConfig                  PicmanCageConfig;
typedef struct _PicmanColorBalanceConfig          PicmanColorBalanceConfig;
typedef struct _PicmanColorizeConfig              PicmanColorizeConfig;
typedef struct _PicmanCurvesConfig                PicmanCurvesConfig;
typedef struct _PicmanDesaturateConfig            PicmanDesaturateConfig;
typedef struct _PicmanHueSaturationConfig         PicmanHueSaturationConfig;
typedef struct _PicmanLevelsConfig                PicmanLevelsConfig;
typedef struct _PicmanPosterizeConfig             PicmanPosterizeConfig;
typedef struct _PicmanThresholdConfig             PicmanThresholdConfig;


/*  non-object types  */

typedef struct _PicmanCagePoint                   PicmanCagePoint;


#endif /* __OPERATIONS_TYPES_H__ */
