/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmandynamics.h"

#include "picmandocked.h"
#include "picmandynamicseditor.h"
#include "picmandynamicsoutputeditor.h"
#include "picmanmenufactory.h"
#include "picmanpropwidgets.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_dynamics_editor_constructed     (GObject            *object);
static void   picman_dynamics_editor_finalize        (GObject            *object);

static void   picman_dynamics_editor_set_data        (PicmanDataEditor     *editor,
                                                    PicmanData           *data);

static void   picman_dynamics_editor_notify_model    (PicmanDynamics       *options,
                                                    const GParamSpec   *pspec,
                                                    PicmanDynamicsEditor *editor);
static void   picman_dynamics_editor_notify_data     (PicmanDynamics       *options,
                                                    const GParamSpec   *pspec,
                                                    PicmanDynamicsEditor *editor);

static void   picman_dynamics_editor_add_output_row  (GObject     *config,
                                                    const gchar *row_label,
                                                    GtkTable    *table,
                                                    gint         row);

static void picman_dynamics_editor_init_output_editors (PicmanDynamics *dynamics,
                                                      GtkWidget    *view_selector,
                                                      GtkWidget    *notebook,
                                                      GtkWidget    *check_grid);

static GtkWidget * dynamics_check_button_new       (GObject     *config,
                                                    const gchar *property_name,
                                                    GtkTable    *table,
                                                    gint         column,
                                                    gint         row);

static void      picman_dynamics_editor_view_changed (GtkComboBox *combo,
                                                    GtkWidget   *notebook);

G_DEFINE_TYPE_WITH_CODE (PicmanDynamicsEditor, picman_dynamics_editor,
                         PICMAN_TYPE_DATA_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED, NULL))

#define parent_class picman_dynamics_editor_parent_class


static void
picman_dynamics_editor_class_init (PicmanDynamicsEditorClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanDataEditorClass *editor_class = PICMAN_DATA_EDITOR_CLASS (klass);

  object_class->constructed = picman_dynamics_editor_constructed;
  object_class->finalize    = picman_dynamics_editor_finalize;

  editor_class->set_data    = picman_dynamics_editor_set_data;
  editor_class->title       = _("Paint Dynamics Editor");
}

static void
picman_dynamics_editor_init (PicmanDynamicsEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);
  PicmanDynamics   *dynamics;
  GtkWidget      *input_labels[7];
  gint            n_inputs    = G_N_ELEMENTS (input_labels);
  gint            i;

  dynamics = editor->dynamics_model = g_object_new (PICMAN_TYPE_DYNAMICS, NULL);

  g_signal_connect (dynamics, "notify",
                    G_CALLBACK (picman_dynamics_editor_notify_model),
                    editor);

  editor->view_selector =
    picman_enum_combo_box_new (PICMAN_TYPE_DYNAMICS_OUTPUT_TYPE);
  gtk_box_pack_start (GTK_BOX (data_editor), editor->view_selector,
                      FALSE, FALSE, 0);
  gtk_widget_show (editor->view_selector);

  editor->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (editor->notebook), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (editor->notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (editor), editor->notebook, TRUE, TRUE, 0);
  gtk_widget_show (editor->notebook);

  editor->check_grid = gtk_table_new (10, n_inputs + 2, FALSE);

  picman_dynamics_editor_init_output_editors (dynamics,
                                            editor->view_selector,
                                            editor->notebook,
                                            editor->check_grid);

  gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
                            editor->check_grid, NULL);
  gtk_widget_show (editor->check_grid);

  picman_int_combo_box_prepend (PICMAN_INT_COMBO_BOX (editor->view_selector),
                              PICMAN_INT_STORE_VALUE,     -1,
                              PICMAN_INT_STORE_LABEL,     _("Mapping matrix"),
                              PICMAN_INT_STORE_USER_DATA, editor->check_grid,
                              -1);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (editor->view_selector), -1);

  input_labels[0] = gtk_label_new (_("Pressure"));
  input_labels[1] = gtk_label_new (_("Velocity"));
  input_labels[2] = gtk_label_new (_("Direction"));
  input_labels[3] = gtk_label_new (_("Tilt"));
  input_labels[4] = gtk_label_new (_("Wheel"));
  input_labels[5] = gtk_label_new (_("Random"));
  input_labels[6] = gtk_label_new (_("Fade"));

  for (i = 0; i < n_inputs; i++)
    {
      gtk_label_set_angle (GTK_LABEL (input_labels[i]), 90);
      gtk_misc_set_alignment (GTK_MISC (input_labels[i]), 0.5, 1.0);

      gtk_table_attach (GTK_TABLE (editor->check_grid), input_labels[i],
                        i + 1, i + 2, 0, 1,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
      gtk_widget_show (input_labels[i]);
    }
}

static void
picman_dynamics_editor_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_docked_set_show_button_bar (PICMAN_DOCKED (object), FALSE);
}

static void
picman_dynamics_editor_finalize (GObject *object)
{
  PicmanDynamicsEditor *editor = PICMAN_DYNAMICS_EDITOR (object);

  if (editor->dynamics_model)
    {
      g_object_unref (editor->dynamics_model);
      editor->dynamics_model = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_dynamics_editor_set_data (PicmanDataEditor *editor,
                               PicmanData       *data)
{
  PicmanDynamicsEditor *dynamics_editor = PICMAN_DYNAMICS_EDITOR (editor);
  GtkTreeModel       *model;
  GtkTreeIter         iter;
  gboolean            iter_valid;

  if (editor->data)
    g_signal_handlers_disconnect_by_func (editor->data,
                                          picman_dynamics_editor_notify_data,
                                          editor);

  PICMAN_DATA_EDITOR_CLASS (parent_class)->set_data (editor, data);

  if (editor->data)
    {
      g_signal_handlers_block_by_func (dynamics_editor->dynamics_model,
                                       picman_dynamics_editor_notify_model,
                                       editor);

      picman_config_copy (PICMAN_CONFIG (editor->data),
                        PICMAN_CONFIG (dynamics_editor->dynamics_model),
                        PICMAN_CONFIG_PARAM_SERIALIZE);

      g_signal_handlers_unblock_by_func (dynamics_editor->dynamics_model,
                                         picman_dynamics_editor_notify_model,
                                         editor);

      g_signal_connect (editor->data, "notify",
                        G_CALLBACK (picman_dynamics_editor_notify_data),
                        editor);
    }

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (dynamics_editor->view_selector));

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gpointer widget;

      gtk_tree_model_get (model, &iter,
                          PICMAN_INT_STORE_USER_DATA, &widget,
                          -1);

      gtk_widget_set_sensitive (widget, editor->data_editable);
    }
}


/*  public functions  */

GtkWidget *
picman_dynamics_editor_new (PicmanContext     *context,
                          PicmanMenuFactory *menu_factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_DYNAMICS_EDITOR,
                       "menu-factory",    menu_factory,
                       "menu-identifier", "<DynamicsEditor>",
                       "ui-path",         "/dynamics-editor-popup",
                       "data-factory",    context->picman->dynamics_factory,
                       "context",         context,
                       "data",            picman_context_get_dynamics (context),
                       NULL);
}


/*  private functions  */

static void
picman_dynamics_editor_notify_model (PicmanDynamics       *options,
                                   const GParamSpec   *pspec,
                                   PicmanDynamicsEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

  if (data_editor->data)
    {
      g_signal_handlers_block_by_func (data_editor->data,
                                       picman_dynamics_editor_notify_data,
                                       editor);

      picman_config_copy (PICMAN_CONFIG (editor->dynamics_model),
                        PICMAN_CONFIG (data_editor->data),
                        PICMAN_CONFIG_PARAM_SERIALIZE);

      g_signal_handlers_unblock_by_func (data_editor->data,
                                         picman_dynamics_editor_notify_data,
                                         editor);
    }
}

static void
picman_dynamics_editor_notify_data (PicmanDynamics       *options,
                                  const GParamSpec   *pspec,
                                  PicmanDynamicsEditor *editor)
{
  PicmanDataEditor *data_editor = PICMAN_DATA_EDITOR (editor);

  g_signal_handlers_block_by_func (editor->dynamics_model,
                                   picman_dynamics_editor_notify_model,
                                   editor);

  picman_config_copy (PICMAN_CONFIG (data_editor->data),
                    PICMAN_CONFIG (editor->dynamics_model),
                    PICMAN_CONFIG_PARAM_SERIALIZE);

  g_signal_handlers_unblock_by_func (editor->dynamics_model,
                                     picman_dynamics_editor_notify_model,
                                     editor);
}

static void
picman_dynamics_editor_add_output_row (GObject     *config,
                                     const gchar *row_label,
                                     GtkTable    *table,
                                     gint         row)
{
  GtkWidget *label;
  gint       column = 1;

  label = gtk_label_new (row_label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row + 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (label);

  dynamics_check_button_new (config, "use-pressure",
                             table, column, row);
  column++;

  dynamics_check_button_new (config, "use-velocity",
                             table, column, row);
  column++;

  dynamics_check_button_new (config, "use-direction",
                             table, column, row);
  column++;

  dynamics_check_button_new (config,  "use-tilt",
                             table, column, row);
  column++;

  dynamics_check_button_new (config,  "use-wheel",
                             table, column, row);
  column++;

  dynamics_check_button_new (config, "use-random",
                             table, column, row);
  column++;

  dynamics_check_button_new (config, "use-fade",
                             table, column, row);
  column++;
}

static GtkWidget *
dynamics_check_button_new (GObject     *config,
                           const gchar *property_name,
                           GtkTable    *table,
                           gint         column,
                           gint         row)
{
  GtkWidget *button;

  button = picman_prop_check_button_new (config, property_name, NULL);
  gtk_table_attach (table, button, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (button);

  return button;
}

static void
picman_dynamics_editor_init_output_editors (PicmanDynamics *dynamics,
                                          GtkWidget    *view_selector,
                                          GtkWidget    *notebook,
                                          GtkWidget    *check_grid)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view_selector));
  PicmanIntStore *list  = PICMAN_INT_STORE (model);
  GtkTreeIter   iter;
  gboolean      iter_valid;
  gint          i;

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter), i = 1;
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter), i++)
    {
      gint                output_type;
      gchar              *label;
      PicmanDynamicsOutput *output;
      GtkWidget          *output_editor;

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                          PICMAN_INT_STORE_VALUE, &output_type,
                          PICMAN_INT_STORE_LABEL, &label,
                          -1);

      output = picman_dynamics_get_output (dynamics, output_type);

      output_editor = picman_dynamics_output_editor_new (output);

      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), output_editor, NULL);
      gtk_widget_show (output_editor);

      gtk_list_store_set (GTK_LIST_STORE (list), &iter,
                          PICMAN_INT_STORE_USER_DATA, output_editor,
                          -1);

      picman_dynamics_editor_add_output_row (G_OBJECT (output),
                                           label,
                                           GTK_TABLE (check_grid),
                                           i);

      g_free (label);
  }

  g_signal_connect (G_OBJECT (view_selector), "changed",
                    G_CALLBACK (picman_dynamics_editor_view_changed),
                    notebook);
}

static void
picman_dynamics_editor_view_changed (GtkComboBox *combo,
                                   GtkWidget   *notebook)
{
  GtkTreeModel *model = gtk_combo_box_get_model (combo);
  GtkTreeIter   iter;
  gpointer      widget;
  gint          page;

  gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);

  gtk_tree_model_get (model, &iter,
                      PICMAN_INT_STORE_USER_DATA, &widget,
                      -1);
  page = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), widget);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);
}
