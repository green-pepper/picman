/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * vectors-types.h
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#ifndef __VECTORS_TYPES_H__
#define __VECTORS_TYPES_H__


#include "core/core-types.h"

#include "vectors/vectors-enums.h"


typedef struct _PicmanAnchor          PicmanAnchor;

typedef struct _PicmanVectors         PicmanVectors;
typedef struct _PicmanVectorsUndo     PicmanVectorsUndo;
typedef struct _PicmanVectorsModUndo  PicmanVectorsModUndo;
typedef struct _PicmanVectorsPropUndo PicmanVectorsPropUndo;
typedef struct _PicmanStroke          PicmanStroke;
typedef struct _PicmanBezierStroke    PicmanBezierStroke;


#endif /* __VECTORS_TYPES_H__ */
