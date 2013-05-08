/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Object properties serialization routines
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

#ifndef __PICMAN_CONFIG_SERIALIZE_H__
#define __PICMAN_CONFIG_SERIALIZE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


gboolean  picman_config_serialize_properties         (PicmanConfig       *config,
                                                    PicmanConfigWriter *writer);
gboolean  picman_config_serialize_changed_properties (PicmanConfig       *config,
                                                    PicmanConfigWriter *writer);

gboolean  picman_config_serialize_property           (PicmanConfig       *config,
                                                    GParamSpec       *param_spec,
                                                    PicmanConfigWriter *writer);
gboolean  picman_config_serialize_property_by_name   (PicmanConfig       *config,
                                                    const gchar      *prop_name,
                                                    PicmanConfigWriter *writer);
gboolean  picman_config_serialize_value              (const GValue     *value,
                                                    GString          *str,
                                                    gboolean          escaped);


G_END_DECLS

#endif /* __PICMAN_CONFIG_SERIALIZE_H__ */
