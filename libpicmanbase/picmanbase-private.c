/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanbase-private.c
 * Copyright (C) 2003 Sven Neumann <sven@picman.org>
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

#include "config.h"

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanbasetypes.h"

#include "picmanbase-private.h"


PicmanUnitVtable _picman_unit_vtable = { NULL, };


void
picman_base_init (PicmanUnitVtable *vtable)
{
  static gboolean picman_base_initialized = FALSE;

  g_return_if_fail (vtable != NULL);

  if (picman_base_initialized)
    g_error ("picman_base_init() must only be called once!");

  _picman_unit_vtable = *vtable;

  picman_base_initialized = TRUE;
}
