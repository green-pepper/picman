/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanuiconfigurer.h
 * Copyright (C) 2009 Martin Nordholts <martinn@src.gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_UI_CONFIGURER_H__
#define __PICMAN_UI_CONFIGURER_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_UI_CONFIGURER              (picman_ui_configurer_get_type ())
#define PICMAN_UI_CONFIGURER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UI_CONFIGURER, PicmanUIConfigurer))
#define PICMAN_UI_CONFIGURER_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), PICMAN_TYPE_UI_CONFIGURER, PicmanUIConfigurerClass))
#define PICMAN_IS_UI_CONFIGURER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UI_CONFIGURER))
#define PICMAN_IS_UI_CONFIGURER_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), PICMAN_TYPE_UI_CONFIGURER))
#define PICMAN_UI_CONFIGURER_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((inst), PICMAN_TYPE_UI_CONFIGURER, PicmanUIConfigurerClass))


typedef struct _PicmanUIConfigurerClass   PicmanUIConfigurerClass;
typedef struct _PicmanUIConfigurerPrivate PicmanUIConfigurerPrivate;

struct _PicmanUIConfigurer
{
  PicmanObject parent_instance;

  PicmanUIConfigurerPrivate *p;
};

struct _PicmanUIConfigurerClass
{
  PicmanObjectClass parent_class;
};


GType         picman_ui_configurer_get_type  (void) G_GNUC_CONST;
void          picman_ui_configurer_configure (PicmanUIConfigurer *ui_configurer,
                                            gboolean          single_window_mode);


#endif  /* __PICMAN_UI_CONFIGURER_H__ */
