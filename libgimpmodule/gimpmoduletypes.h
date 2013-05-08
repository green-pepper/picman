/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#ifndef __PICMAN_MODULE_TYPES_H__
#define __PICMAN_MODULE_TYPES_H__


#include <libpicmanbase/picmanbasetypes.h>


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#ifndef PICMAN_DISABLE_DEPRECATED
/*
 * PICMAN_MODULE_PARAM_SERIALIZE is deprecated, use
 * PICMAN_CONFIG_PARAM_SERIALIZE instead.
 */
#define PICMAN_MODULE_PARAM_SERIALIZE (1 << (0 + G_PARAM_USER_SHIFT))
#endif


typedef struct _PicmanModule     PicmanModule;
typedef struct _PicmanModuleInfo PicmanModuleInfo;
typedef struct _PicmanModuleDB   PicmanModuleDB;


G_END_DECLS

#endif  /* __PICMAN_MODULE_TYPES_H__ */
