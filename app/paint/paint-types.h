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

#ifndef __PAINT_TYPES_H__
#define __PAINT_TYPES_H__


#include "core/core-types.h"
#include "paint/paint-enums.h"


/*  paint cores  */

typedef struct _PicmanPaintCore        PicmanPaintCore;
typedef struct _PicmanBrushCore        PicmanBrushCore;
typedef struct _PicmanSourceCore       PicmanSourceCore;

typedef struct _PicmanAirbrush         PicmanAirbrush;
typedef struct _PicmanClone            PicmanClone;
typedef struct _PicmanConvolve         PicmanConvolve;
typedef struct _PicmanDodgeBurn        PicmanDodgeBurn;
typedef struct _PicmanEraser           PicmanEraser;
typedef struct _PicmanHeal             PicmanHeal;
typedef struct _PicmanInk              PicmanInk;
typedef struct _PicmanPaintbrush       PicmanPaintbrush;
typedef struct _PicmanPencil           PicmanPencil;
typedef struct _PicmanPerspectiveClone PicmanPerspectiveClone;
typedef struct _PicmanSmudge           PicmanSmudge;


/*  paint options  */

typedef struct _PicmanPaintOptions            PicmanPaintOptions;
typedef struct _PicmanSourceOptions           PicmanSourceOptions;

typedef struct _PicmanAirbrushOptions         PicmanAirbrushOptions;
typedef struct _PicmanCloneOptions            PicmanCloneOptions;
typedef struct _PicmanConvolveOptions         PicmanConvolveOptions;
typedef struct _PicmanDodgeBurnOptions        PicmanDodgeBurnOptions;
typedef struct _PicmanEraserOptions           PicmanEraserOptions;
typedef struct _PicmanInkOptions              PicmanInkOptions;
typedef struct _PicmanPencilOptions           PicmanPencilOptions;
typedef struct _PicmanPerspectiveCloneOptions PicmanPerspectiveCloneOptions;
typedef struct _PicmanSmudgeOptions           PicmanSmudgeOptions;


/*  paint undos  */

typedef struct _PicmanPaintCoreUndo PicmanPaintCoreUndo;
typedef struct _PicmanInkUndo       PicmanInkUndo;


/*  functions  */

typedef void (* PicmanPaintRegisterCallback) (Picman        *picman,
                                            GType        paint_type,
                                            GType        paint_options_type,
                                            const gchar *identifier,
                                            const gchar *blurb,
                                            const gchar *stock_id);

typedef void (* PicmanPaintRegisterFunc)     (Picman                      *picman,
                                            PicmanPaintRegisterCallback  callback);


#endif /* __PAINT_TYPES_H__ */
