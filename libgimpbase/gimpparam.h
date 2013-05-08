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

#ifndef __PICMAN_PARAM_H__
#define __PICMAN_PARAM_H__


/**
 * SECTION: picmanparam
 * @title: picmanparam
 * @short_description: Definitions of useful #GParamFlags.
 *
 * Definitions of useful #GParamFlags.
 **/


/**
 * PICMAN_PARAM_STATIC_STRINGS:
 *
 * Since: PICMAN 2.4
 **/
#define PICMAN_PARAM_STATIC_STRINGS (G_PARAM_STATIC_NAME | \
                                   G_PARAM_STATIC_NICK | \
                                   G_PARAM_STATIC_BLURB)

/**
 * PICMAN_PARAM_READABLE:
 *
 * Since: PICMAN 2.4
 **/
#define PICMAN_PARAM_READABLE       (G_PARAM_READABLE    | \
                                   PICMAN_PARAM_STATIC_STRINGS)

/**
 * PICMAN_PARAM_WRITABLE:
 *
 * Since: PICMAN 2.4
 **/
#define PICMAN_PARAM_WRITABLE       (G_PARAM_WRITABLE    | \
                                   PICMAN_PARAM_STATIC_STRINGS)

/**
 * PICMAN_PARAM_READWRITE:
 *
 * Since: PICMAN 2.4
 **/
#define PICMAN_PARAM_READWRITE      (G_PARAM_READWRITE   | \
                                   PICMAN_PARAM_STATIC_STRINGS)


#endif  /*  __PICMAN_PARAM_H__  */
