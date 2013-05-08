/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#if !defined (__PICMAN_CONFIG_H_INSIDE__) && !defined (PICMAN_CONFIG_COMPILATION)
#error "Only <libpicmanconfig/picmanconfig.h> can be included directly."
#endif

#ifndef __PICMAN_CONFIG_ERROR_H__
#define __PICMAN_CONFIG_ERROR_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/**
 * PicmanConfigError:
 * @PICMAN_CONFIG_ERROR_OPEN:        open failed
 * @PICMAN_CONFIG_ERROR_OPEN_ENOENT: file does not exist
 * @PICMAN_CONFIG_ERROR_WRITE:       write failed
 * @PICMAN_CONFIG_ERROR_PARSE:       parser error
 * @PICMAN_CONFIG_ERROR_VERSION:     parser failed due to version mismatch
 *
 * The possible values of a #GError thrown by libpicmanconfig.
 **/
typedef enum
{
  PICMAN_CONFIG_ERROR_OPEN,
  PICMAN_CONFIG_ERROR_OPEN_ENOENT,
  PICMAN_CONFIG_ERROR_WRITE,
  PICMAN_CONFIG_ERROR_PARSE,
  PICMAN_CONFIG_ERROR_VERSION
} PicmanConfigError;

#define PICMAN_CONFIG_ERROR (picman_config_error_quark ())

GQuark        picman_config_error_quark   (void) G_GNUC_CONST;


G_END_DECLS

#endif  /* __PICMAN_CONFIG_ERROR_H__ */
