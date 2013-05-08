/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanColorConfig class
 * Copyright (C) 2004  Stefan Döhla <stefan@doehla.de>
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

#ifndef __PICMAN_COLOR_CONFIG_H__
#define __PICMAN_COLOR_CONFIG_H__


#define PICMAN_TYPE_COLOR_CONFIG            (picman_color_config_get_type ())
#define PICMAN_COLOR_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_CONFIG, PicmanColorConfig))
#define PICMAN_COLOR_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_CONFIG, PicmanColorConfigClass))
#define PICMAN_IS_COLOR_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_CONFIG))
#define PICMAN_IS_COLOR_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_CONFIG))


typedef struct _PicmanColorConfigClass PicmanColorConfigClass;

struct _PicmanColorConfig
{
  GObject                     parent_instance;

  /*< public >*/
  PicmanColorManagementMode     mode;
  gchar                      *rgb_profile;
  gchar                      *cmyk_profile;
  gchar                      *display_profile;
  gboolean                    display_profile_from_gdk;
  gchar                      *printer_profile;
  PicmanColorRenderingIntent    display_intent;
  PicmanColorRenderingIntent    simulation_intent;

  gchar                      *display_module;

  gboolean                    simulation_gamut_check;
  PicmanRGB                     out_of_gamut_color;

  /*< private >*/
  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
  void (* _picman_reserved5) (void);
  void (* _picman_reserved6) (void);
  void (* _picman_reserved7) (void);
  void (* _picman_reserved8) (void);
};

struct _PicmanColorConfigClass
{
  GObjectClass                parent_class;
};


GType  picman_color_config_get_type (void) G_GNUC_CONST;


#endif /* PICMAN_COLOR_CONFIG_H__ */
