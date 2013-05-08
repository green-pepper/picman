/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * Config file serialization and deserialization interface
 * Copyright (C) 2001-2003  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_CONFIG_TYPES_H__
#define __PICMAN_CONFIG_TYPES_H__


#include <libpicmanbase/picmanbasetypes.h>


typedef struct _PicmanConfig        PicmanConfig; /* dummy typedef */
typedef struct _PicmanConfigWriter  PicmanConfigWriter;
typedef gchar *                   PicmanConfigPath; /* to satisfy docs */


#include <libpicmanconfig/picmancolorconfig-enums.h>

typedef struct _PicmanColorConfig   PicmanColorConfig;


#endif  /* __PICMAN_CONFIG_TYPES_H__ */
