/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorprofilecombobox.c
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmancolorprofilecombobox.h"
#include "picmancolorprofilestore.h"
#include "picmancolorprofilestore-private.h"


/**
 * SECTION: picmancolorprofilecombobox
 * @title: PicmanColorProfileComboBox
 * @short_description: A combo box for selecting color profiles.
 *
 * A combo box for selecting color profiles.
 **/


enum
{
  PROP_0,
  PROP_DIALOG,
  PROP_MODEL
};


typedef struct
{
  GtkTreePath *last_path;
} PicmanColorProfileComboBoxPrivate;

#define PICMAN_COLOR_PROFILE_COMBO_BOX_GET_PRIVATE(obj) \
  G_TYPE_INSTANCE_GET_PRIVATE (obj, \
                               PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX, \
                               PicmanColorProfileComboBoxPrivate)


static void  picman_color_profile_combo_box_finalize     (GObject      *object);
static void  picman_color_profile_combo_box_set_property (GObject      *object,
                                                        guint         property_id,
                                                        const GValue *value,
                                                        GParamSpec   *pspec);
static void  picman_color_profile_combo_box_get_property (GObject      *object,
                                                        guint         property_id,
                                                        GValue       *value,
                                                        GParamSpec   *pspec);
static void  picman_color_profile_combo_box_changed      (GtkComboBox  *combo);

static gboolean  picman_color_profile_row_separator_func (GtkTreeModel *model,
                                                        GtkTreeIter  *iter,
                                                        gpointer      data);


G_DEFINE_TYPE (PicmanColorProfileComboBox,
               picman_color_profile_combo_box, GTK_TYPE_COMBO_BOX)

#define parent_class picman_color_profile_combo_box_parent_class


static void
picman_color_profile_combo_box_class_init (PicmanColorProfileComboBoxClass *klass)
{
  GObjectClass     *object_class = G_OBJECT_CLASS (klass);
  GtkComboBoxClass *combo_class  = GTK_COMBO_BOX_CLASS (klass);

  object_class->set_property = picman_color_profile_combo_box_set_property;
  object_class->get_property = picman_color_profile_combo_box_get_property;
  object_class->finalize     = picman_color_profile_combo_box_finalize;

  combo_class->changed       = picman_color_profile_combo_box_changed;

  /**
   * PicmanColorProfileComboBox:dialog:
   *
   * #GtkDialog to present when the user selects the
   * "Select color profile from disk..." item.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class,
                                   PROP_DIALOG,
                                   g_param_spec_object ("dialog", NULL, NULL,
                                                        GTK_TYPE_DIALOG,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        PICMAN_PARAM_READWRITE));
  /**
   * PicmanColorProfileComboBox:model:
   *
   * Overrides the "model" property of the #GtkComboBox class.
   * #PicmanColorProfileComboBox requires the model to be a
   * #PicmanColorProfileStore.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model", NULL, NULL,
                                                        PICMAN_TYPE_COLOR_PROFILE_STORE,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (object_class,
                            sizeof (PicmanColorProfileComboBoxPrivate));
}

static void
picman_color_profile_combo_box_init (PicmanColorProfileComboBox *combo_box)
{
  GtkCellRenderer *cell = gtk_cell_renderer_text_new ();

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                  "text", PICMAN_COLOR_PROFILE_STORE_LABEL,
                                  NULL);

  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo_box),
                                        picman_color_profile_row_separator_func,
                                        NULL, NULL);
}

static void
picman_color_profile_combo_box_finalize (GObject *object)
{
  PicmanColorProfileComboBox        *combo;
  PicmanColorProfileComboBoxPrivate *priv;

  combo = PICMAN_COLOR_PROFILE_COMBO_BOX (object);

  if (combo->dialog)
    {
      g_object_unref (combo->dialog);
      combo->dialog = NULL;
    }

  priv = PICMAN_COLOR_PROFILE_COMBO_BOX_GET_PRIVATE (combo);

  if (priv->last_path)
    {
      gtk_tree_path_free (priv->last_path);
      priv->last_path = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_profile_combo_box_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  PicmanColorProfileComboBox *combo_box = PICMAN_COLOR_PROFILE_COMBO_BOX (object);

  switch (property_id)
    {
    case PROP_DIALOG:
      g_return_if_fail (combo_box->dialog == NULL);
      combo_box->dialog = g_value_dup_object (value);
      break;

    case PROP_MODEL:
      gtk_combo_box_set_model (GTK_COMBO_BOX (combo_box),
                               g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_profile_combo_box_get_property (GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  PicmanColorProfileComboBox *combo_box = PICMAN_COLOR_PROFILE_COMBO_BOX (object);

  switch (property_id)
    {
    case PROP_DIALOG:
      g_value_set_object (value, combo_box->dialog);
      break;

    case PROP_MODEL:
      g_value_set_object (value,
                          gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_profile_combo_box_changed (GtkComboBox *combo)
{
  PicmanColorProfileComboBoxPrivate *priv;

  GtkTreeModel *model = gtk_combo_box_get_model (combo);
  GtkTreeIter   iter;
  gint          type;

  if (! gtk_combo_box_get_active_iter (combo, &iter))
    return;

  gtk_tree_model_get (model, &iter,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                      -1);

  priv = PICMAN_COLOR_PROFILE_COMBO_BOX_GET_PRIVATE (combo);

  switch (type)
    {
    case PICMAN_COLOR_PROFILE_STORE_ITEM_DIALOG:
      {
        GtkWidget *dialog = PICMAN_COLOR_PROFILE_COMBO_BOX (combo)->dialog;
        GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (combo));

        if (GTK_IS_WINDOW (parent))
          gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                        GTK_WINDOW (parent));

        gtk_window_present (GTK_WINDOW (dialog));

        if (priv->last_path &&
            gtk_tree_model_get_iter (model, &iter, priv->last_path))
          {
            gtk_combo_box_set_active_iter (combo, &iter);
          }
      }
      break;

    case PICMAN_COLOR_PROFILE_STORE_ITEM_FILE:
      if (priv->last_path)
        gtk_tree_path_free (priv->last_path);

      priv->last_path = gtk_tree_model_get_path (model, &iter);

      _picman_color_profile_store_history_reorder (PICMAN_COLOR_PROFILE_STORE (model),
                                                 &iter);
      break;

    default:
      break;
    }
}


/**
 * picman_color_profile_combo_box_new:
 * @dialog:  a #GtkDialog to present when the user selects the
 *           "Select color profile from disk..." item
 * @history: filename of the profilerc (or %NULL for no history)
 *
 * Create a combo-box widget for selecting color profiles. The combo-box
 * is populated from the file specified as @history. This filename is
 * typically created using the following code snippet:
 * <informalexample><programlisting>
 *  gchar *history = picman_personal_rc_file ("profilerc");
 * </programlisting></informalexample>
 *
 * Return value: a new #PicmanColorProfileComboBox.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_color_profile_combo_box_new (GtkWidget   *dialog,
                                  const gchar *history)
{
  GtkWidget    *combo;
  GtkListStore *store;

  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);

  store = picman_color_profile_store_new (history);
  combo = picman_color_profile_combo_box_new_with_model (dialog,
                                                       GTK_TREE_MODEL (store));
  g_object_unref (store);

  return combo;
}

/**
 * picman_color_profile_combo_box_new_with_model:
 * @dialog: a #GtkDialog to present when the user selects the
 *          "Select color profile from disk..." item
 * @model:  a #PicmanColorProfileStore object
 *
 * This constructor is useful when you want to create several
 * combo-boxes for profile selection that all share the same
 * #PicmanColorProfileStore. This is for example done in the
 * PICMAN Preferences dialog.
 *
 * See also picman_color_profile_combo_box_new().
 *
 * Return value: a new #PicmanColorProfileComboBox.
 *
 * Since: PICMAN 2.4
 **/
GtkWidget *
picman_color_profile_combo_box_new_with_model (GtkWidget    *dialog,
                                             GtkTreeModel *model)
{
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
  g_return_val_if_fail (PICMAN_IS_COLOR_PROFILE_STORE (model), NULL);

  return g_object_new (PICMAN_TYPE_COLOR_PROFILE_COMBO_BOX,
                       "dialog", dialog,
                       "model",  model,
                       NULL);
}

/**
 * picman_color_profile_combo_box_add:
 * @combo:    a #PicmanColorProfileComboBox
 * @filename: filename of the profile to add (or %NULL)
 * @label:    label to use for the profile
 *            (may only be %NULL if @filename is %NULL)
 *
 * This function delegates to the underlying
 * #PicmanColorProfileStore. Please refer to the documentation of
 * picman_color_profile_store_add() for details.
 *
 * Since: PICMAN 2.4
 **/
void
picman_color_profile_combo_box_add (PicmanColorProfileComboBox *combo,
                                  const gchar              *filename,
                                  const gchar              *label)
{
  GtkTreeModel *model;

  g_return_if_fail (PICMAN_IS_COLOR_PROFILE_COMBO_BOX (combo));
  g_return_if_fail (label != NULL || filename == NULL);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  picman_color_profile_store_add (PICMAN_COLOR_PROFILE_STORE (model),
                                filename, label);
}

/**
 * picman_color_profile_combo_box_set_active:
 * @combo:    a #PicmanColorProfileComboBox
 * @filename: filename of the profile to select
 * @label:    label to use when adding a new entry (can be %NULL)
 *
 * Selects a color profile from the @combo and makes it the active
 * item.  If the profile is not listed in the @combo, then it is added
 * with the given @label (or @filename in case that @label is %NULL).
 *
 * Since: PICMAN 2.4
 **/
void
picman_color_profile_combo_box_set_active (PicmanColorProfileComboBox *combo,
                                         const gchar              *filename,
                                         const gchar              *label)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  g_return_if_fail (PICMAN_IS_COLOR_PROFILE_COMBO_BOX (combo));

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (_picman_color_profile_store_history_add (PICMAN_COLOR_PROFILE_STORE (model),
                                             filename, label, &iter))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
}

/**
 * picman_color_profile_combo_box_get_active:
 * @combo: a #PicmanColorProfileComboBox
 *
 * Return value: The filename of the currently selected color profile.
 *               This is a newly allocated string and should be released
 *               using g_free() when it is not any longer needed.
 *
 * Since: PICMAN 2.4
 **/
gchar *
picman_color_profile_combo_box_get_active (PicmanColorProfileComboBox *combo)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  g_return_val_if_fail (PICMAN_IS_COLOR_PROFILE_COMBO_BOX (combo), NULL);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
    {
      gchar *filename;
      gint   type;

      gtk_tree_model_get (model, &iter,
                          PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                          PICMAN_COLOR_PROFILE_STORE_FILENAME,  &filename,
                          -1);

      if (type == PICMAN_COLOR_PROFILE_STORE_ITEM_FILE)
        return filename;

      g_free (filename);
    }

  return NULL;
}

static gboolean
picman_color_profile_row_separator_func (GtkTreeModel *model,
                                       GtkTreeIter  *iter,
                                       gpointer      data)
{
  gint type;

  gtk_tree_model_get (model, iter,
                      PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE, &type,
                      -1);

  switch (type)
    {
    case PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_TOP:
    case PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_BOTTOM:
      return TRUE;

    default:
      return FALSE;
    }
}
