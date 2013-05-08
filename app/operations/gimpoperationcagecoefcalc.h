/* PICMAN - The GNU Image Manipulation Program
 *
 * picmanoperationcagecoefcalc.h
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

#ifndef __PICMAN_OPERATION_CAGE_COEF_CALC_H__
#define __PICMAN_OPERATION_CAGE_COEF_CALC_H__


#include <gegl-plugin.h>
#include <operation/gegl-operation-source.h>


enum
{
  PICMAN_OPERATION_CAGE_COEF_CALC_PROP_0,
  PICMAN_OPERATION_CAGE_COEF_CALC_PROP_CONFIG
};


#define PICMAN_TYPE_OPERATION_CAGE_COEF_CALC            (picman_operation_cage_coef_calc_get_type ())
#define PICMAN_OPERATION_CAGE_COEF_CALC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_CAGE_COEF_CALC, PicmanOperationCageCoefCalc))
#define PICMAN_OPERATION_CAGE_COEF_CALC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_CAGE_COEF_CALC, PicmanOperationCageCoefCalcClass))
#define PICMAN_IS_OPERATION_CAGE_COEF_CALC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_CAGE_COEF_CALC))
#define PICMAN_IS_OPERATION_CAGE_COEF_CALC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_CAGE_COEF_CALC))
#define PICMAN_OPERATION_CAGE_COEF_CALC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_CAGE_COEF_CALC, PicmanOperationCageCoefCalcClass))


typedef struct _PicmanOperationCageCoefCalc      PicmanOperationCageCoefCalc;
typedef struct _PicmanOperationCageCoefCalcClass PicmanOperationCageCoefCalcClass;

struct _PicmanOperationCageCoefCalc
{
  GeglOperationSource  parent_instance;

  PicmanCageConfig      *config;
};

struct _PicmanOperationCageCoefCalcClass
{
  GeglOperationSourceClass  parent_class;
};


GType   picman_operation_cage_coef_calc_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_CAGE_COEF_CALC_H__ */
