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

#ifndef __PICMAN_BASE_H__
#define __PICMAN_BASE_H__

#define __PICMAN_BASE_H_INSIDE__

#include <libpicmanbase/picmanbasetypes.h>

#include <libpicmanbase/picmanchecks.h>
#include <libpicmanbase/picmancpuaccel.h>
#include <libpicmanbase/picmandatafiles.h>
#include <libpicmanbase/picmanenv.h>
#include <libpicmanbase/picmanlimits.h>
#include <libpicmanbase/picmanmemsize.h>
#include <libpicmanbase/picmanparasite.h>
#include <libpicmanbase/picmanrectangle.h>
#include <libpicmanbase/picmanunit.h>
#include <libpicmanbase/picmanutils.h>
#include <libpicmanbase/picmanversion.h>
#include <libpicmanbase/picmanvaluearray.h>

#ifndef G_OS_WIN32
#include <libpicmanbase/picmansignal.h>
#endif

#undef __PICMAN_BASE_H_INSIDE__

#endif  /* __PICMAN_BASE_H__ */
