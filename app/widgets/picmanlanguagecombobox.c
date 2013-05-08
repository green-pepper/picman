/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlanguagecombobox.c
 * Copyright (C) 2009  Sven Neumann <sven@picman.org>
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

/* PicmanLanguageComboBox is a combo-box widget to select the user
 * interface language.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "picmanlanguagecombobox.h"
#include "picmantranslationstore.h"


struct _PicmanLanguageComboBox
{
  GtkComboBox parent_instance;
};


G_DEFINE_TYPE (PicmanLanguageComboBox,
               picman_language_combo_box, GTK_TYPE_COMBO_BOX)

#define parent_class picman_language_combo_box_parent_class


static void
picman_language_combo_box_class_init (PicmanLanguageComboBoxClass *klass)
{
}

static void
picman_language_combo_box_init (PicmanLanguageComboBox *combo)
{
  GtkCellRenderer *renderer;

  renderer = gtk_cell_renderer_text_new ();

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                  "text",  PICMAN_LANGUAGE_STORE_LABEL,
                                  NULL);
}

GtkWidget *
picman_language_combo_box_new (void)
{
  GtkWidget    *combo;
  GtkListStore *store;

  store = picman_translation_store_new ();

  combo = g_object_new (PICMAN_TYPE_LANGUAGE_COMBO_BOX,
                        "model", store,
                        NULL);

  g_object_unref (store);

  return combo;
}

gchar *
picman_language_combo_box_get_code (PicmanLanguageComboBox *combo)
{
  GtkTreeIter  iter;
  gchar       *code;

  g_return_val_if_fail (PICMAN_IS_LANGUAGE_COMBO_BOX (combo), NULL);

  if (! gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
    return NULL;

  gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)), &iter,
                      PICMAN_LANGUAGE_STORE_CODE, &code,
                      -1);

  return code;
}

gboolean
picman_language_combo_box_set_code (PicmanLanguageComboBox *combo,
                                  const gchar          *code)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  g_return_val_if_fail (PICMAN_IS_LANGUAGE_COMBO_BOX (combo), FALSE);

  if (! code || ! strlen (code))
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
      return TRUE;
    }

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (picman_language_store_lookup (PICMAN_LANGUAGE_STORE (model), code, &iter))
    {
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
      return TRUE;
    }

  return FALSE;
}
