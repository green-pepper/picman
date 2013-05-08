/* picmanxmpmodelwidget.h - interface definition for xmpmodel gtkwidgets
 *
 * Copyright (C) 2010, Róman Joost <romanofski@picman.org>
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

#ifndef __PICMAN_XMP_MODEL_WIDGET_H__
#define __PICMAN_XMP_MODEL_WIDGET_H__

G_BEGIN_DECLS


typedef enum
{
  PICMAN_XMP_MODEL_WIDGET_PROP_0,
  PICMAN_XMP_MODEL_WIDGET_PROP_SCHEMA_URI,
  PICMAN_XMP_MODEL_WIDGET_PROP_PROPERTY_NAME,
  PICMAN_XMP_MODEL_WIDGET_PROP_XMPMODEL
} PicmanXmpModelWidgetProp;



#define PICMAN_TYPE_XMP_MODEL_WIDGET                  (picman_xmp_model_widget_interface_get_type ())
#define PICMAN_IS_XMP_MODEL_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_XMP_MODEL_WIDGET))
#define PICMAN_XMP_MODEL_WIDGET(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_XMP_MODEL_WIDGET, PicmanXmpModelWidget))
#define PICMAN_XMP_MODEL_WIDGET_GET_INTERFACE(obj)    (G_TYPE_INSTANCE_GET_INTERFACE ((obj), PICMAN_TYPE_XMP_MODEL_WIDGET, PicmanXmpModelWidgetInterface))


typedef struct _PicmanXmpModelWidget              PicmanXmpModelWidget;
typedef struct _PicmanXmpModelWidgetInterface     PicmanXmpModelWidgetInterface;


struct _PicmanXmpModelWidgetInterface
{
  GTypeInterface base_iface;

  void          (*widget_set_text) (PicmanXmpModelWidget  *widget,
                                    const gchar         *value);
};


GType           picman_xmp_model_widget_interface_get_type    (void) G_GNUC_CONST;

void            picman_xmp_model_widget_install_properties    (GObjectClass *klass);

void            picman_xmp_model_widget_constructor           (GObject *object);

void            picman_xmp_model_widget_set_text              (PicmanXmpModelWidget *widget,
                                                             const gchar        *value);

void            picman_xmp_model_widget_set_property          (GObject            *object,
                                                             guint               property_id,
                                                             const GValue       *value,
                                                             GParamSpec         *pspec);

void            picman_xmp_model_widget_get_property          (GObject            *object,
                                                             guint               property_id,
                                                             GValue             *value,
                                                             GParamSpec         *pspec);

void            picman_xmp_model_widget_changed               (PicmanXmpModelWidget *widget,
                                                             const gchar        *value);


G_END_DECLS

#endif /* __PICMAN_XMP_MODEL_WIDGET_H__ */
