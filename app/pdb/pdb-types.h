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

#ifndef __PDB_TYPES_H__
#define __PDB_TYPES_H__


#include "core/core-types.h"


typedef struct _PicmanPDB                PicmanPDB;
typedef struct _PicmanProcedure          PicmanProcedure;
typedef struct _PicmanPlugInProcedure    PicmanPlugInProcedure;
typedef struct _PicmanTemporaryProcedure PicmanTemporaryProcedure;


typedef enum
{
  PICMAN_PDB_COMPAT_OFF,
  PICMAN_PDB_COMPAT_ON,
  PICMAN_PDB_COMPAT_WARN
} PicmanPDBCompatMode;


typedef enum
{
  PICMAN_PDB_ITEM_CONTENT  = 1 << 0,
  PICMAN_PDB_ITEM_POSITION = 1 << 1
} PicmanPDBItemModify;


#endif /* __PDB_TYPES_H__ */
