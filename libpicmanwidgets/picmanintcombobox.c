/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanintcombobox.c
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#include <libintl.h>

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanintcombobox.h"
#include "picmanintstore.h"


/**
 * SECTION: picmanintcombobox
 * @title: PicmanIntComboBox
 * @short_description: A widget providing a popup menu of integer
 *                     values (e.g. enums).
 *
 * A widget providing a popup menu of integer values (e.g. enums).
 **/


enum
{
  PROP_0,
  PROP_ELLIPSIZE
};


typedef struct
{
  GtkCellRenderer        *pixbuf_renderer;
  GtkCellRenderer        *text_renderer;

  PicmanIntSensitivityFunc  sensitivity_func;
  gpointer                sensitivity_data;
  GDestroyNotify          sensitivity_destroy;
} PicmanIntComboBoxPrivate;

#define PICMAN_INT_COMBO_BOX_GET_PRIVATE(obj) \
  ((PicmanIntComboBoxPrivate *) ((PicmanIntComboBox *) (obj))->priv)


static void  picman_int_combo_box_finalize     (GObject         *object);
static void  picman_int_combo_box_set_property (GObject         *object,
                                              guint            property_id,
                                              const GValue    *value,
                                              GParamSpec      *pspec);
static void  picman_int_combo_box_get_property (GObject         *object,
                                              guint            property_id,
                                              GValue          *value,
                                              GParamSpec      *pspec);

static void  picman_int_combo_box_data_func    (GtkCellLayout   *layout,
                                              GtkCellRenderer *cell,
                                              GtkTreeModel    *model,
                                              GtkTreeIter     *iter,
                                              gpointer         data);


G_DEFINE_TYPE (PicmanIntComboBox, picman_int_combo_box, GTK_TYPE_COMBO_BOX)

#define parent_class picman_int_combo_box_parent_class


static void
picman_int_combo_box_class_init (PicmanIntComboBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_int_combo_box_set_property;
  object_class->get_property = picman_int_combo_box_get_property;
  object_class->finalize     = picman_int_combo_box_finalize;

  /**
   * PicmanIntComboBox:ellipsize:
   *
   * Specifies the preferred place to ellipsize text in the combo-box,
   * if the cell renderer does not have enough room to display the
   * entire string.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class,
                                   PROP_ELLIPSIZE,
                                   g_param_spec_enum ("ellipsize", NULL, NULL,
                                                      PANGO_TYPE_ELLIPSIZE_MODE,
                                                      PANGO_ELLIPSIZE_NONE,
                                                      PICMAN_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (PicmanIntComboBoxPrivate));
}

static void
picman_int_combo_box_init (PicmanIntComboBox *combo_box)
{
  PicmanIntComboBoxPrivate *priv;
  GtkListStore           *store;
  GtkCellRenderer        *cell;

  combo_box->priv = G_TYPE_INSTANCE_GET_PRIVATE (combo_box,
                                                 PICMAN_TYPE_INT_COMBO_BOX,
                                                 PicmanIntComboBoxPrivate);

  priv = PICMAN_INT_COMBO_BOX_GET_PRIVATE (combo_box);

  store = picman_int_store_new ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo_box), GTK_TREE_MODEL (store));
  g_object_unref (store);

  priv->pixbuf_renderer = cell = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                  "stock-id", PICMAN_INT_STORE_STOCK_ID,
                                  "pixbuf",   PICMAN_INT_STORE_PIXBUF,
                                  NULL);

  priv->text_renderer = cell = gtk_cell_renderer_text_new ();

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                  "text", PICMAN_INT_STORE_LABEL,
                                  NULL);
}

static void
picman_int_combo_box_finalize (GObject *object)
{
  PicmanIntComboBoxPrivate *priv = PICMAN_INT_COMBO_BOX_GET_PRIVATE (object);

  if (priv->sensitivity_destroy)
    {
      GDestroyNotify d = priv->sensitivity_destroy;

      priv->sensitivity_destroy = NULL;
      d (priv->sensitivity_data);
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_int_combo_box_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanIntComboBoxPrivate *priv = PICMAN_INT_COMBO_BOX_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ELLIPSIZE:
      g_object_set_property (G_OBJECT (priv->text_renderer),
                             pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_int_combo_box_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanIntComboBoxPrivate *priv = PICMAN_INT_COMBO_BOX_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ELLIPSIZE:
      g_object_get_property (G_OBJECT (priv->text_renderer),
                             pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/**
 * picman_int_combo_box_new:
 * @first_label: the label of the first item
 * @first_value: the value of the first item
 * @...: a %NULL terminated list of more label, value pairs
 *
 * Creates a GtkComboBox that has integer values associated with each
 * item. The items to fill the combo box with are specified as a %NULL
 * terminated list of label/value pairs.
 *
 * If you need to construct an empty #PicmanIntComboBox, it's best to use
 * g_object_new (PICMAN_TYPE_INT_COMBO_BOX, NULL).
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_int_combo_box_new (const gchar *first_label,
                        gint         first_value,
                        ...)
{
  GtkWidget *combo_box;
  va_list    args;

  va_start (args, first_value);

  combo_box = picman_int_combo_box_new_valist (first_label, first_value, args);

  va_end (args);

  return combo_box;
}

/**
 * picman_int_combo_box_new_valist:
 * @first_label: the label of the first item
 * @first_value: the value of the first item
 * @values: a va_list with more values
 *
 * A variant of picman_int_combo_box_new() that takes a va_list of
 * label/value pairs. Probably only useful for language bindings.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_int_combo_box_new_valist (const gchar *first_label,
                               gint         first_value,
                               va_list      values)
{
  GtkWidget    *combo_box;
  GtkListStore *store;
  const gchar  *label;
  gint          value;

  combo_box = g_object_new (PICMAN_TYPE_INT_COMBO_BOX, NULL);

  store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));

  for (label = first_label, value = first_value;
       label;
       label = va_arg (values, const gchar *), value = va_arg (values, gint))
    {
      GtkTreeIter  iter = { 0, };

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          PICMAN_INT_STORE_VALUE, value,
                          PICMAN_INT_STORE_LABEL, label,
                          -1);
    }

  return combo_box;
}

/**
 * picman_int_combo_box_new_array:
 * @n_values: the number of values
 * @labels:   an array of labels (array length must be @n_values)
 *
 * A variant of picman_int_combo_box_new() that takes an array of labels.
 * The array indices are used as values.
 *
 * Return value: a new #PicmanIntComboBox.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_int_combo_box_new_array (gint         n_values,
                              const gchar *labels[])
{
  GtkWidget    *combo_box;
  GtkListStore *store;
  gint          i;

  g_return_val_if_fail (n_values >= 0, NULL);
  g_return_val_if_fail (labels != NULL || n_values == 0, NULL);

  combo_box = g_object_new (PICMAN_TYPE_INT_COMBO_BOX, NULL);

  store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));

  for (i = 0; i < n_values; i++)
    {
      GtkTreeIter  iter;

      if (labels[i])
        {
          gtk_list_store_append (store, &iter);
          gtk_list_store_set (store, &iter,
                              PICMAN_INT_STORE_VALUE, i,
                              PICMAN_INT_STORE_LABEL, gettext (labels[i]),
                              -1);
        }
    }

  return combo_box;
}

/**
 * picman_int_combo_box_prepend:
 * @combo_box: a #PicmanIntComboBox
 * @...:       pairs of column number and value, terminated with -1
 *
 * This function provides a convenient way to prepend items to a
 * #PicmanIntComboBox. It prepends a row to the @combo_box's list store
 * and calls gtk_list_store_set() for you.
 *
 * The column number must be taken from the enum #PicmanIntStoreColumns.
 *
 * Since: PICMAN 2.2
 **/
void
picman_int_combo_box_prepend (PicmanIntComboBox *combo_box,
                            ...)
{
  GtkListStore *store;
  GtkTreeIter   iter;
  va_list       args;

  g_return_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box));

  store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));

  va_start (args, combo_box);

  gtk_list_store_prepend (store, &iter);
  gtk_list_store_set_valist (store, &iter, args);

  va_end (args);
}

/**
 * picman_int_combo_box_append:
 * @combo_box: a #PicmanIntComboBox
 * @...:       pairs of column number and value, terminated with -1
 *
 * This function provides a convenient way to append items to a
 * #PicmanIntComboBox. It appends a row to the @combo_box's list store
 * and calls gtk_list_store_set() for you.
 *
 * The column number must be taken from the enum #PicmanIntStoreColumns.
 *
 * Since: PICMAN 2.2
 **/
void
picman_int_combo_box_append (PicmanIntComboBox *combo_box,
                           ...)
{
  GtkListStore *store;
  GtkTreeIter   iter;
  va_list       args;

  g_return_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box));

  store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));

  va_start (args, combo_box);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set_valist (store, &iter, args);

  va_end (args);
}

/**
 * picman_int_combo_box_set_active:
 * @combo_box: a #PicmanIntComboBox
 * @value:     an integer value
 *
 * Looks up the item that belongs to the given @value and makes it the
 * selected item in the @combo_box.
 *
 * Return value: %TRUE on success or %FALSE if there was no item for
 *               this value.
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_int_combo_box_set_active (PicmanIntComboBox *combo_box,
                               gint             value)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  g_return_val_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box), FALSE);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

  if (picman_int_store_lookup_by_value (model, value, &iter))
    {
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo_box), &iter);
      return TRUE;
    }

  return FALSE;
}

/**
 * picman_int_combo_box_get_active:
 * @combo_box: a #PicmanIntComboBox
 * @value:     return location for the integer value
 *
 * Retrieves the value of the selected (active) item in the @combo_box.
 *
 * Return value: %TRUE if @value has been set or %FALSE if no item was
 *               active.
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_int_combo_box_get_active (PicmanIntComboBox *combo_box,
                               gint            *value)
{
  GtkTreeIter  iter;

  g_return_val_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box), FALSE);
  g_return_val_if_fail (value != NULL, FALSE);

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_box), &iter))
    {
      gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)),
                          &iter,
                          PICMAN_INT_STORE_VALUE, value,
                          -1);
      return TRUE;
    }

  return FALSE;
}

/**
 * picman_int_combo_box_connect:
 * @combo_box: a #PicmanIntComboBox
 * @value:     the value to set
 * @callback:  a callback to connect to the @combo_box's "changed" signal
 * @data:      a pointer passed as data to g_signal_connect()
 *
 * A convenience function that sets the initial @value of a
 * #PicmanIntComboBox and connects @callback to the "changed"
 * signal.
 *
 * This function also calls the @callback once after setting the
 * initial @value. This is often convenient when working with combo
 * boxes that select a default active item, like for example
 * picman_drawable_combo_box_new(). If you pass an invalid initial
 * @value, the @callback will be called with the default item active.
 *
 * Return value: the signal handler ID as returned by g_signal_connect()
 *
 * Since: PICMAN 2.2
 **/
gulong
picman_int_combo_box_connect (PicmanIntComboBox *combo_box,
                            gint             value,
                            GCallback        callback,
                            gpointer         data)
{
  gulong handler = 0;

  g_return_val_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box), 0);

  if (callback)
    handler = g_signal_connect (combo_box, "changed", callback, data);

  if (! picman_int_combo_box_set_active (combo_box, value))
    g_signal_emit_by_name (combo_box, "changed", NULL);

  return handler;
}

/**
 * picman_int_combo_box_set_sensitivity:
 * @combo_box: a #PicmanIntComboBox
 * @func: a function that returns a boolean value, or %NULL to unset
 * @data: data to pass to @func
 * @destroy: destroy notification for @data
 *
 * Sets a function that is used to decide about the sensitivity of
 * rows in the @combo_box. Use this if you want to set certain rows
 * insensitive.
 *
 * Calling gtk_widget_queue_draw() on the @combo_box will cause the
 * sensitivity to be updated.
 *
 * Since: PICMAN 2.4
 **/
void
picman_int_combo_box_set_sensitivity (PicmanIntComboBox        *combo_box,
                                    PicmanIntSensitivityFunc  func,
                                    gpointer                data,
                                    GDestroyNotify          destroy)
{
  PicmanIntComboBoxPrivate *priv;

  g_return_if_fail (PICMAN_IS_INT_COMBO_BOX (combo_box));

  priv = PICMAN_INT_COMBO_BOX_GET_PRIVATE (combo_box);

  if (priv->sensitivity_destroy)
    {
      GDestroyNotify d = priv->sensitivity_destroy;

      priv->sensitivity_destroy = NULL;
      d (priv->sensitivity_data);
    }

  priv->sensitivity_func    = func;
  priv->sensitivity_data    = data;
  priv->sensitivity_destroy = destroy;

  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo_box),
                                      priv->pixbuf_renderer,
                                      func ?
                                      picman_int_combo_box_data_func : NULL,
                                      priv, NULL);

  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo_box),
                                      priv->text_renderer,
                                      func ?
                                      picman_int_combo_box_data_func : NULL,
                                      priv, NULL);
}


static void
picman_int_combo_box_data_func (GtkCellLayout   *layout,
                              GtkCellRenderer *cell,
                              GtkTreeModel    *model,
                              GtkTreeIter     *iter,
                              gpointer         data)
{
  PicmanIntComboBoxPrivate *priv = data;

  if (priv->sensitivity_func)
    {
      gint      value;
      gboolean  sensitive;

      gtk_tree_model_get (model, iter,
                          PICMAN_INT_STORE_VALUE, &value,
                          -1);

      sensitive = priv->sensitivity_func (value, priv->sensitivity_data);

      g_object_set (cell,
                    "sensitive", sensitive,
                    NULL);
    }
}
