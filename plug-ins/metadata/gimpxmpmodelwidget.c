/* picmanxmpmodelwidget.c - interface definition for XMPModel bound
 *                        GTKWidgets
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

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "xmp-schemas.h"
#include "xmp-model.h"

#include "picmanxmpmodelwidget.h"


#define PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE(obj) \
  (picman_xmp_model_widget_get_private (PICMAN_XMP_MODEL_WIDGET (obj)))


typedef struct _PicmanXmpModelWidgetPrivate PicmanXmpModelWidgetPrivate;

struct _PicmanXmpModelWidgetPrivate
{
  const gchar *schema_uri;
  const gchar *property_name;
  XMPModel    *xmp_model;
};


static void     picman_xmp_model_widget_iface_base_init   (PicmanXmpModelWidgetInterface *iface);

static PicmanXmpModelWidgetPrivate *
                picman_xmp_model_widget_get_private       (PicmanXmpModelWidget *widget);

static void     picman_xmp_model_widget_xmpmodel_changed  (XMPModel           *xmp_model,
                                                         GtkTreeIter        *iter,
                                                         gpointer           *user_data);

const gchar *   find_schema_prefix                      (const gchar        *schema_uri);

void            set_property_edit_icon                  (GtkWidget          *widget,
                                                         XMPModel           *xmp_model,
                                                         GtkTreeIter        *iter);


GType
picman_xmp_model_widget_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
   {
    const GTypeInfo iface_info =
     {
      sizeof (PicmanXmpModelWidgetInterface),
      (GBaseInitFunc)     picman_xmp_model_widget_iface_base_init,
      (GBaseFinalizeFunc) NULL,
     };

    iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                         "PicmanXmpModelWidgetInterface",
                                         &iface_info, 0);
   }

  return iface_type;
}

static void
picman_xmp_model_widget_iface_base_init (PicmanXmpModelWidgetInterface *iface)
{
   static gboolean initialized = FALSE;

   if (! initialized)
    {
      g_object_interface_install_property (iface,
                                           g_param_spec_string ("schema-uri",
                                                                NULL, NULL, NULL,
                                                                G_PARAM_CONSTRUCT_ONLY |
                                                                G_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_string ("property-name",
                                                                NULL, NULL, NULL,
                                                                G_PARAM_CONSTRUCT_ONLY |
                                                                G_PARAM_READWRITE));

      g_object_interface_install_property (iface,
                                           g_param_spec_object ("xmp-model",
                                                                NULL, NULL,
                                                                PICMAN_TYPE_XMP_MODEL,
                                                                G_PARAM_CONSTRUCT_ONLY |
                                                                PICMAN_PARAM_READWRITE));
      iface->widget_set_text = NULL;
      initialized = TRUE;
    }

}

/**
 * picman_xmp_model_widget_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 *
 * Installs the necessary properties for a class implementing
 * #PicmanXmpModelWidgetInterface. A #PicmanXmpModelWidgetProp property is
 * installed for each property, using the values from the
 * #PicmanXmpModelWidgetProp enumeration.
 **/
void
picman_xmp_model_widget_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
                                    PICMAN_XMP_MODEL_WIDGET_PROP_SCHEMA_URI,
                                    "schema-uri");

  g_object_class_override_property (klass,
                                    PICMAN_XMP_MODEL_WIDGET_PROP_XMPMODEL,
                                    "xmp-model");

  g_object_class_override_property (klass,
                                    PICMAN_XMP_MODEL_WIDGET_PROP_PROPERTY_NAME,
                                    "property-name");
}

void
picman_xmp_model_widget_constructor (GObject *object)
{
  PicmanXmpModelWidget        *widget = PICMAN_XMP_MODEL_WIDGET (object);
  PicmanXmpModelWidgetPrivate *priv;
  gchar                     *signal;

  priv = PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE (object);

  signal = g_strdup_printf ("property-changed::%s:%s",
                            find_schema_prefix (priv->schema_uri),
                            priv->property_name);

  g_signal_connect (priv->xmp_model, signal,
                    G_CALLBACK (picman_xmp_model_widget_xmpmodel_changed),
                    widget);

  g_free (signal);
}

void
picman_xmp_model_widget_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PicmanXmpModelWidgetPrivate *priv = PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE (object);

  switch (property_id)
    {
    case PICMAN_XMP_MODEL_WIDGET_PROP_SCHEMA_URI:
      priv->schema_uri = g_value_dup_string (value);
      break;

    case PICMAN_XMP_MODEL_WIDGET_PROP_PROPERTY_NAME:
      priv->property_name = g_value_dup_string (value);
      break;

    case PICMAN_XMP_MODEL_WIDGET_PROP_XMPMODEL:
      priv->xmp_model = g_value_dup_object (value);
      break;

    default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
       break;
    }
}

void
picman_xmp_model_widget_get_property (GObject      *object,
                                    guint         property_id,
                                    GValue       *value,
                                    GParamSpec   *pspec)
{
  PicmanXmpModelWidgetPrivate *priv = PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE (object);

  switch (property_id)
    {
    case PICMAN_XMP_MODEL_WIDGET_PROP_SCHEMA_URI:
      g_value_set_string (value, priv->schema_uri);
      break;

    case PICMAN_XMP_MODEL_WIDGET_PROP_PROPERTY_NAME:
      g_value_set_string (value, priv->property_name);
      break;

    case PICMAN_XMP_MODEL_WIDGET_PROP_XMPMODEL:
      g_value_set_object (value, priv->xmp_model);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_xmp_model_widget_private_finalize (PicmanXmpModelWidgetPrivate *private)
{
  g_slice_free (PicmanXmpModelWidgetPrivate, private);
}

static PicmanXmpModelWidgetPrivate *
picman_xmp_model_widget_get_private (PicmanXmpModelWidget *widget)
{
  const gchar *private_key = "picman-xmp-model-widget-private";

  PicmanXmpModelWidgetPrivate *private;

  private = g_object_get_data (G_OBJECT (widget), private_key);

  if (! private)
   {
     private = g_slice_new0 (PicmanXmpModelWidgetPrivate);

     g_object_set_data_full (G_OBJECT (widget), private_key, private,
                             (GDestroyNotify) picman_xmp_model_widget_private_finalize);
   }

  return private;
}

/**
 * picman_xmp_model_widget_xmpmodel_changed:
 * @xmp_model: XMPModel this widget is bound to.
 * @iter: The iter which points to the last change in the XMPModel
 * @user_data: which should be the GtkWidget displaying the value.
 *
 * If the XMPModel has been changed, the GtkWidget needs to be updated.
 * This method updates the corresponding GtkWidget with the new value
 * from the XMPModel.
 **/
static void
picman_xmp_model_widget_xmpmodel_changed (XMPModel     *xmp_model,
                                        GtkTreeIter  *iter,
                                        gpointer     *user_data)
{
  PicmanXmpModelWidget        *widget = PICMAN_XMP_MODEL_WIDGET (user_data);
  PicmanXmpModelWidgetPrivate *priv  = PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE (widget);
  const gchar               *tree_value;
  const gchar               *property_name;
  GdkPixbuf                 *icon;

  gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), iter,
                      COL_XMP_NAME,      &property_name,
                      COL_XMP_VALUE,     &tree_value,
                      COL_XMP_EDIT_ICON, &icon,
                      -1);

  if (! strcmp (priv->property_name, property_name))
    picman_xmp_model_widget_set_text (widget, tree_value);

  if (icon == NULL)
    set_property_edit_icon (GTK_WIDGET (widget), priv->xmp_model, iter);

  return;
}

/**
 * picman_xmp_model_widget_set_text:
 * @widget: The GtkWidget where the new value is set.
 * @tree_value: The new string which will be set on the widget.
 *
 * This method sets the new value on the GtkWidget implementing the
 * #PicmanXmpModelWidgetInterface.
 **/
void
picman_xmp_model_widget_set_text (PicmanXmpModelWidget  *widget,
                                const gchar         *tree_value)
{
  PicmanXmpModelWidgetInterface *iface;

  iface = PICMAN_XMP_MODEL_WIDGET_GET_INTERFACE (widget);

  if (iface->widget_set_text)
    iface->widget_set_text (widget, tree_value);
}

/**
 * picman_xmp_model_widget_changed:
 * @widget: The GtkWidget which was changed.
 * @value: The new string from the GtkWidget.
 *
 * If the GtkWidget was changed, a new value is set in the #XMPModel.
 **/
void
picman_xmp_model_widget_changed (PicmanXmpModelWidget *widget,
                               const gchar        *value)
{
  PicmanXmpModelWidgetPrivate *priv = PICMAN_XMP_MODEL_WIDGET_GET_PRIVATE (widget);

  xmp_model_set_scalar_property (priv->xmp_model,
                                 priv->schema_uri,
                                 priv->property_name,
                                 value);
}

/**
 * utility methods
 **/

/* find the schema prefix for the given URI */
const gchar*
find_schema_prefix (const gchar *schema_uri)
{
  int i;

  for (i = 0; xmp_schemas[i].uri != NULL; ++i)
  {
    if (! strcmp (xmp_schemas[i].uri, schema_uri))
      return xmp_schemas[i].prefix;
  }
  return NULL;
}

void
set_property_edit_icon (GtkWidget       *widget,
                        XMPModel        *xmp_model,
                        GtkTreeIter     *iter)
{
  GdkPixbuf                *icon;
  gboolean                  editable;

  gtk_tree_model_get (GTK_TREE_MODEL (xmp_model), iter,
                      COL_XMP_EDITABLE, &editable,
                      COL_XMP_EDIT_ICON, &icon,
                      -1);

  if (editable == XMP_AUTO_UPDATE)
    {
      icon = gtk_widget_render_icon (GTK_WIDGET (widget), PICMAN_STOCK_WILBER,
                                     GTK_ICON_SIZE_MENU, NULL);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), iter,
                          COL_XMP_EDIT_ICON, icon,
                          -1);
    }
  else if (editable == TRUE)
    {
      icon = gtk_widget_render_icon (GTK_WIDGET (widget), GTK_STOCK_EDIT,
                                     GTK_ICON_SIZE_MENU, NULL);
      gtk_tree_store_set (GTK_TREE_STORE (xmp_model), iter,
                          COL_XMP_EDIT_ICON, icon,
                          -1);
    }

  return;
}
