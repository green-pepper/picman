/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1999 Peter Mattis and Spencer Kimball
 *
 * picmanunitcombobox.c
 * Copyright (C) 2004, 2008  Sven Neumann <sven@picman.org>
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

#include "config.h"

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanunitcombobox.h"
#include "picmanunitstore.h"


/**
 * SECTION: picmanunitcombobox
 * @title: PicmanUnitComboBox
 * @short_description: A #GtkComboBox to select a #PicmanUnit.
 * @see_also: #PicmanUnit, #PicmanUnitStore
 *
 * #PicmanUnitComboBox selects units stored in a #PicmanUnitStore.
 * It replaces the deprecated #PicmanUnitMenu.
 **/


static void  picman_unit_combo_box_style_set (GtkWidget *widget,
                                            GtkStyle  *prev_style);


G_DEFINE_TYPE (PicmanUnitComboBox, picman_unit_combo_box, GTK_TYPE_COMBO_BOX)

#define parent_class picman_unit_combo_box_parent_class


static void
picman_unit_combo_box_class_init (PicmanUnitComboBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->style_set = picman_unit_combo_box_style_set;

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_double ("label-scale",
                                                                NULL, NULL,
                                                                0.0,
                                                                G_MAXDOUBLE,
                                                                1.0,
                                                                PICMAN_PARAM_READABLE));
}

static void
picman_unit_combo_box_init (PicmanUnitComboBox *combo)
{
  GtkCellLayout   *layout = GTK_CELL_LAYOUT (combo);
  GtkCellRenderer *cell;

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (layout, cell, TRUE);
  gtk_cell_layout_set_attributes (layout, cell,
                                  "text", PICMAN_UNIT_STORE_UNIT_LONG_FORMAT,
                                  NULL);
}

static void
picman_unit_combo_box_style_set (GtkWidget *widget,
                               GtkStyle  *prev_style)
{
  GtkCellLayout   *layout;
  GtkCellRenderer *cell;
  gdouble          scale;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget, "label-scale", &scale, NULL);

  /*  hackedehack ...  */
  layout = GTK_CELL_LAYOUT (gtk_bin_get_child (GTK_BIN (widget)));
  gtk_cell_layout_clear (layout);

  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                       "scale", scale,
                       NULL);
  gtk_cell_layout_pack_start (layout, cell, TRUE);
  gtk_cell_layout_set_attributes (layout, cell,
                                  "text",  PICMAN_UNIT_STORE_UNIT_SHORT_FORMAT,
                                  NULL);
}

/**
 * picman_unit_combo_box_new:
 *
 * Return value: a new #PicmanUnitComboBox.
 **/
GtkWidget *
picman_unit_combo_box_new (void)
{
  GtkWidget     *combo_box;
  PicmanUnitStore *store;

  store = picman_unit_store_new (0);

  combo_box = g_object_new (PICMAN_TYPE_UNIT_COMBO_BOX,
                            "model", store,
                            NULL);

  g_object_unref (store);

  return combo_box;
}

/**
 * picman_unit_combo_box_new_with_model:
 * @model: a PicmanUnitStore
 *
 * Return value: a new #PicmanUnitComboBox.
 **/
GtkWidget *
picman_unit_combo_box_new_with_model (PicmanUnitStore *model)
{
  return g_object_new (PICMAN_TYPE_UNIT_COMBO_BOX,
                       "model", model,
                       NULL);
}

PicmanUnit
picman_unit_combo_box_get_active (PicmanUnitComboBox *combo)
{
  GtkTreeIter iter;
  gint        unit;

  g_return_val_if_fail (PICMAN_IS_UNIT_COMBO_BOX (combo), -1);

  gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);

  gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter,
                      PICMAN_UNIT_STORE_UNIT, &unit,
                      -1);

  return (PicmanUnit) unit;
}

void
picman_unit_combo_box_set_active (PicmanUnitComboBox *combo,
                                PicmanUnit          unit)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gboolean      iter_valid;

  g_return_if_fail (PICMAN_IS_UNIT_COMBO_BOX (combo));

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gint iter_unit;

      gtk_tree_model_get (model, &iter,
                          PICMAN_UNIT_STORE_UNIT, &iter_unit,
                          -1);

      if (unit == (PicmanUnit) iter_unit)
        {
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
          break;
        }
    }

}
