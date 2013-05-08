/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmantypes.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_TYPES_H__
#define __PICMAN_TYPES_H__

#include <libpicmanbase/picmanbasetypes.h>

G_BEGIN_DECLS

/* For information look into the html documentation */


typedef struct _PicmanPlugInInfo  PicmanPlugInInfo;
typedef struct _PicmanTile        PicmanTile;
typedef struct _PicmanDrawable    PicmanDrawable;
typedef struct _PicmanPixelRgn    PicmanPixelRgn;
typedef struct _PicmanParamDef    PicmanParamDef;
typedef struct _PicmanParamRegion PicmanParamRegion;
typedef union  _PicmanParamData   PicmanParamData;
typedef struct _PicmanParam       PicmanParam;

G_END_DECLS

#endif /* __PICMAN_TYPES_H__ */
