/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanConfig typedefs
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
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

#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__


#include "libpicmanconfig/picmanconfigtypes.h"

#include "config/config-enums.h"


#define PICMAN_OPACITY_TRANSPARENT      0.0
#define PICMAN_OPACITY_OPAQUE           1.0


typedef struct _PicmanGeglConfig       PicmanGeglConfig;
typedef struct _PicmanCoreConfig       PicmanCoreConfig;
typedef struct _PicmanDisplayConfig    PicmanDisplayConfig;
typedef struct _PicmanGuiConfig        PicmanGuiConfig;
typedef struct _PicmanPluginConfig     PicmanPluginConfig;
typedef struct _PicmanRc               PicmanRc;

typedef struct _PicmanXmlParser        PicmanXmlParser;

typedef struct _PicmanDisplayOptions   PicmanDisplayOptions;

/* should be in core/core-types.h */
typedef struct _PicmanGrid             PicmanGrid;
typedef struct _PicmanTemplate         PicmanTemplate;


#endif /* __CONFIG_TYPES_H__ */
