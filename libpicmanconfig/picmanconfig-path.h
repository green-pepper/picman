/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanconfig-path.h
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CONFIG_PATH_H__
#define __PICMAN_CONFIG_PATH_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*
 * PICMAN_TYPE_CONFIG_PATH
 */

#define PICMAN_TYPE_CONFIG_PATH               (picman_config_path_get_type ())
#define PICMAN_VALUE_HOLDS_CONFIG_PATH(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_CONFIG_PATH))

GType               picman_config_path_get_type        (void) G_GNUC_CONST;



/*
 * PICMAN_TYPE_PARAM_CONFIG_PATH
 */

typedef enum
{
  PICMAN_CONFIG_PATH_FILE,
  PICMAN_CONFIG_PATH_FILE_LIST,
  PICMAN_CONFIG_PATH_DIR,
  PICMAN_CONFIG_PATH_DIR_LIST
} PicmanConfigPathType;


#define PICMAN_TYPE_PARAM_CONFIG_PATH            (picman_param_config_path_get_type ())
#define PICMAN_IS_PARAM_SPEC_CONFIG_PATH(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_CONFIG_PATH))

GType               picman_param_config_path_get_type  (void) G_GNUC_CONST;

GParamSpec        * picman_param_spec_config_path      (const gchar  *name,
                                                      const gchar  *nick,
                                                      const gchar  *blurb,
                                                      PicmanConfigPathType  type,
                                                      const gchar  *default_value,
                                                      GParamFlags   flags);

PicmanConfigPathType  picman_param_spec_config_path_type (GParamSpec   *pspec);


/*
 * PicmanConfigPath utilities
 */

gchar             * picman_config_path_expand          (const gchar  *path,
                                                      gboolean      recode,
                                                      GError      **error) G_GNUC_MALLOC;

gchar             * picman_config_build_data_path      (const gchar  *name) G_GNUC_MALLOC;
gchar             * picman_config_build_writable_path  (const gchar  *name) G_GNUC_MALLOC;
gchar             * picman_config_build_plug_in_path   (const gchar  *name) G_GNUC_MALLOC;


G_END_DECLS

#endif /* __PICMAN_CONFIG_PATH_H__ */
