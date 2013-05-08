/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "config.h"

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "picmanwidgets-constructors.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean   picman_paint_mode_menu_separator_func (GtkTreeModel *model,
                                                       GtkTreeIter  *iter,
                                                       gpointer      data);


/*  public functions  */

static void
picman_enum_store_insert_value_after (PicmanEnumStore *store,
                                    gint           after,
                                    gint           insert_value)
{
  GtkTreeIter iter;

  g_return_if_fail (PICMAN_IS_ENUM_STORE (store));

  if (picman_int_store_lookup_by_value (GTK_TREE_MODEL (store),
                                      after, &iter))
    {
      GEnumValue *enum_value;

      enum_value = g_enum_get_value (store->enum_class, insert_value);

      if (enum_value)
        {
          GtkTreeIter  value_iter;
          const gchar *desc;

          gtk_list_store_insert_after (GTK_LIST_STORE (store),
                                       &value_iter, &iter);

          desc = picman_enum_value_get_desc (store->enum_class, enum_value);

          gtk_list_store_set (GTK_LIST_STORE (store), &value_iter,
                              PICMAN_INT_STORE_VALUE, enum_value->value,
                              PICMAN_INT_STORE_LABEL, desc,
                              -1);
        }
    }
}

static void
picman_int_store_insert_separator_after (PicmanIntStore *store,
                                       gint          after,
                                       gint          separator_value)
{
  GtkTreeIter iter;

  g_return_if_fail (PICMAN_IS_INT_STORE (store));

  if (picman_int_store_lookup_by_value (GTK_TREE_MODEL (store),
                                      after, &iter))
    {
      GtkTreeIter sep_iter;

      gtk_list_store_insert_after (GTK_LIST_STORE (store),
                                   &sep_iter, &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &sep_iter,
                          PICMAN_INT_STORE_VALUE, separator_value,
                          -1);
    }
}

GtkWidget *
picman_paint_mode_menu_new (gboolean with_behind_mode,
                          gboolean with_replace_modes)
{
  GtkListStore *store;
  GtkWidget    *combo;

  store = picman_enum_store_new_with_values (PICMAN_TYPE_LAYER_MODE_EFFECTS,
                                           21,
                                           PICMAN_NORMAL_MODE,
                                           PICMAN_DISSOLVE_MODE,

                                           PICMAN_LIGHTEN_ONLY_MODE,
                                           PICMAN_SCREEN_MODE,
                                           PICMAN_DODGE_MODE,
                                           PICMAN_ADDITION_MODE,

                                           PICMAN_DARKEN_ONLY_MODE,
                                           PICMAN_MULTIPLY_MODE,
                                           PICMAN_BURN_MODE,

                                           PICMAN_OVERLAY_MODE,
                                           PICMAN_SOFTLIGHT_MODE,
                                           PICMAN_HARDLIGHT_MODE,

                                           PICMAN_DIFFERENCE_MODE,
                                           PICMAN_SUBTRACT_MODE,
                                           PICMAN_GRAIN_EXTRACT_MODE,
                                           PICMAN_GRAIN_MERGE_MODE,
                                           PICMAN_DIVIDE_MODE,

                                           PICMAN_HUE_MODE,
                                           PICMAN_SATURATION_MODE,
                                           PICMAN_COLOR_MODE,
                                           PICMAN_VALUE_MODE);

  picman_int_store_insert_separator_after (PICMAN_INT_STORE (store),
                                         PICMAN_DISSOLVE_MODE, -1);

  picman_int_store_insert_separator_after (PICMAN_INT_STORE (store),
                                         PICMAN_ADDITION_MODE, -1);

  picman_int_store_insert_separator_after (PICMAN_INT_STORE (store),
                                         PICMAN_BURN_MODE, -1);

  picman_int_store_insert_separator_after (PICMAN_INT_STORE (store),
                                         PICMAN_HARDLIGHT_MODE, -1);

  picman_int_store_insert_separator_after (PICMAN_INT_STORE (store),
                                         PICMAN_DIVIDE_MODE, -1);

  if (with_behind_mode)
    {
      picman_enum_store_insert_value_after (PICMAN_ENUM_STORE (store),
                                          PICMAN_DISSOLVE_MODE,
                                          PICMAN_BEHIND_MODE);
      picman_enum_store_insert_value_after (PICMAN_ENUM_STORE (store),
                                          PICMAN_BEHIND_MODE,
                                          PICMAN_COLOR_ERASE_MODE);
    }

  if (with_replace_modes)
    {
      picman_enum_store_insert_value_after (PICMAN_ENUM_STORE (store),
                                          PICMAN_NORMAL_MODE,
                                          PICMAN_REPLACE_MODE);
      picman_enum_store_insert_value_after (PICMAN_ENUM_STORE (store),
                                          PICMAN_COLOR_ERASE_MODE,
                                          PICMAN_ERASE_MODE);
      picman_enum_store_insert_value_after (PICMAN_ENUM_STORE (store),
                                          PICMAN_ERASE_MODE,
                                          PICMAN_ANTI_ERASE_MODE);
    }

  combo = picman_enum_combo_box_new_with_model (PICMAN_ENUM_STORE (store));
  g_object_unref (store);

  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo),
                                        picman_paint_mode_menu_separator_func,
                                        GINT_TO_POINTER (-1),
                                        NULL);

  return combo;
}

GtkWidget *
picman_stock_button_new (const gchar *stock_id,
                       const gchar *label)
{
  GtkWidget *button;
  GtkWidget *image;

  button = gtk_button_new ();

  if (label)
    {
      GtkWidget *hbox;
      GtkWidget *lab;

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_container_add (GTK_CONTAINER (button), hbox);
      gtk_widget_show (hbox);

      image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
      gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
      gtk_widget_show (image);

      lab = gtk_label_new_with_mnemonic (label);
      gtk_label_set_mnemonic_widget (GTK_LABEL (lab), button);
      gtk_box_pack_start (GTK_BOX (hbox), lab, TRUE, TRUE, 0);
      gtk_widget_show (lab);
    }
  else
    {
      image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
      gtk_container_add (GTK_CONTAINER (button), image);
      gtk_widget_show (image);
    }

  return button;
}


/*  private functions  */

static gboolean
picman_paint_mode_menu_separator_func (GtkTreeModel *model,
                                     GtkTreeIter  *iter,
                                     gpointer      data)
{
  gint value;

  gtk_tree_model_get (model, iter, PICMAN_INT_STORE_VALUE, &value, -1);

  return value == GPOINTER_TO_INT (data);
}
