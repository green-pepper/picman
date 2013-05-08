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

#if !defined (__PICMAN_CONFIG_H_INSIDE__) && !defined (PICMAN_CONFIG_COMPILATION)
#error "Only <libpicmanconfig/picmanconfig.h> can be included directly."
#endif

#ifndef __PICMAN_CONFIG_IFACE_H__
#define __PICMAN_CONFIG_IFACE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_CONFIG               (picman_config_interface_get_type ())
#define PICMAN_IS_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONFIG))
#define PICMAN_CONFIG(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONFIG, PicmanConfig))
#define PICMAN_CONFIG_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_CONFIG, PicmanConfigInterface))


typedef struct _PicmanConfigInterface PicmanConfigInterface;

struct _PicmanConfigInterface
{
  GTypeInterface base_iface;

  gboolean     (* serialize)            (PicmanConfig       *config,
                                         PicmanConfigWriter *writer,
                                         gpointer          data);
  gboolean     (* deserialize)          (PicmanConfig       *config,
                                         GScanner         *scanner,
                                         gint              nest_level,
                                         gpointer          data);
  gboolean     (* serialize_property)   (PicmanConfig       *config,
                                         guint             property_id,
                                         const GValue     *value,
                                         GParamSpec       *pspec,
                                         PicmanConfigWriter *writer);
  gboolean     (* deserialize_property) (PicmanConfig       *config,
                                         guint             property_id,
                                         GValue           *value,
                                         GParamSpec       *pspec,
                                         GScanner         *scanner,
                                         GTokenType       *expected);
  PicmanConfig * (* duplicate)            (PicmanConfig       *config);
  gboolean     (* equal)                (PicmanConfig       *a,
                                         PicmanConfig       *b);
  void         (* reset)                (PicmanConfig       *config);
  gboolean     (* copy)                 (PicmanConfig       *src,
                                         PicmanConfig       *dest,
                                         GParamFlags       flags);
};


GType         picman_config_interface_get_type    (void) G_GNUC_CONST;

gboolean      picman_config_serialize_to_file     (PicmanConfig       *config,
                                                 const gchar      *filename,
                                                 const gchar      *header,
                                                 const gchar      *footer,
                                                 gpointer          data,
                                                 GError          **error);
gboolean      picman_config_serialize_to_fd       (PicmanConfig       *config,
                                                 gint              fd,
                                                 gpointer          data);
gchar       * picman_config_serialize_to_string   (PicmanConfig       *config,
                                                 gpointer          data);
gboolean      picman_config_deserialize_file      (PicmanConfig       *config,
                                                 const gchar      *filename,
                                                 gpointer          data,
                                                 GError          **error);
gboolean      picman_config_deserialize_string    (PicmanConfig       *config,
                                                 const gchar      *text,
                                                 gint              text_len,
                                                 gpointer          data,
                                                 GError          **error);
gboolean      picman_config_deserialize_return    (GScanner         *scanner,
                                                 GTokenType        expected_token,
                                                 gint              nest_level);

gboolean      picman_config_serialize             (PicmanConfig       *config,
                                                 PicmanConfigWriter *writer,
                                                 gpointer          data);
gboolean      picman_config_deserialize           (PicmanConfig       *config,
                                                 GScanner         *scanner,
                                                 gint              nest_level,
                                                 gpointer          data);
gpointer      picman_config_duplicate             (PicmanConfig       *config);
gboolean      picman_config_is_equal_to           (PicmanConfig       *a,
                                                 PicmanConfig       *b);
void          picman_config_reset                 (PicmanConfig       *config);
gboolean      picman_config_copy                  (PicmanConfig       *src,
                                                 PicmanConfig       *dest,
                                                 GParamFlags       flags);


G_END_DECLS

#endif  /* __PICMAN_CONFIG_IFACE_H__ */
