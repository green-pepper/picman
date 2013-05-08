/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2003  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
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

#include <glib.h>

#include "picmanthumb-error.h"


/**
 * SECTION: picmanthumb-error
 * @title: PicmanThumb-error
 * @short_description: Error codes used by libpicmanthumb
 *
 * Error codes used by libpicmanthumb
 **/


/**
 * picman_thumb_error_quark:
 *
 * This function is never called directly. Use PICMAN_THUMB_ERROR() instead.
 *
 * Return value: the #GQuark that defines the PicmanThumb error domain.
 **/
GQuark
picman_thumb_error_quark (void)
{
  return g_quark_from_static_string ("picman-thumb-error-quark");
}
