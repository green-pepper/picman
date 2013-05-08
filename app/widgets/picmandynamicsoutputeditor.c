/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmandynamicsoutputeditor.c
 * Copyright (C) 2010 Alexia Death
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancurve.h"
#include "core/picmandynamicsoutput.h"

#include "picmancurveview.h"
#include "picmandynamicsoutputeditor.h"

#include "picman-intl.h"


#define CURVE_SIZE   185
#define CURVE_BORDER   4


enum
{
  PROP_0,
  PROP_OUTPUT
};

enum
{
  INPUT_COLUMN_INDEX,
  INPUT_COLUMN_USE_INPUT,
  INPUT_COLUMN_NAME,
  INPUT_COLUMN_COLOR,
  INPUT_N_COLUMNS
};


struct
{
  const gchar   *use_property;
  const gchar   *curve_property;
  const gchar   *label;
  const PicmanRGB  color;
}
inputs[] =
{
  { "use-pressure",  "pressure-curve",  N_("Pressure"),  { 1.0, 0.0, 0.0, 1.0 } },
  { "use-velocity",  "velocity-curve",  N_("Velocity"),  { 0.0, 1.0, 0.0, 1.0 } },
  { "use-direction", "direction-curve", N_("Direction"), { 0.0, 0.0, 1.0, 1.0 } },
  { "use-tilt",      "tilt-curve",      N_("Tilt"),      { 1.0, 0.5, 0.0, 1.0 } },
  { "use-wheel",     "wheel-curve",     N_("Wheel"),     { 1.0, 0.0, 1.0, 1.0 } },
  { "use-random",    "random-curve",    N_("Random"),    { 0.0, 1.0, 1.0, 1.0 } },
  { "use-fade",      "fade-curve",      N_("Fade"),      { 0.2, 0.2, 0.2, 1.0 } }
};


typedef struct _PicmanDynamicsOutputEditorPrivate PicmanDynamicsOutputEditorPrivate;

struct _PicmanDynamicsOutputEditorPrivate
{
  PicmanDynamicsOutput *output;

  GtkListStore       *input_list;
  GtkTreeIter         input_iters[G_N_ELEMENTS (inputs)];

  GtkWidget          *curve_view;
  GtkWidget          *input_view;

  PicmanCurve          *active_curve;
};

#define GET_PRIVATE(editor) \
        G_TYPE_INSTANCE_GET_PRIVATE (editor, \
                                     PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR, \
                                     PicmanDynamicsOutputEditorPrivate)


static void   picman_dynamics_output_editor_constructed    (GObject                  *object);
static void   picman_dynamics_output_editor_finalize       (GObject                  *object);
static void   picman_dynamics_output_editor_set_property   (GObject                  *object,
                                                          guint                     property_id,
                                                          const GValue             *value,
                                                          GParamSpec               *pspec);
static void   picman_dynamics_output_editor_get_property   (GObject                  *object,
                                                          guint                     property_id,
                                                          GValue                   *value,
                                                          GParamSpec               *pspec);

static void   picman_dynamics_output_editor_curve_reset    (GtkWidget                *button,
                                                          PicmanDynamicsOutputEditor *editor);

static void   picman_dynamics_output_editor_input_selected (GtkTreeSelection         *selection,
                                                          PicmanDynamicsOutputEditor *editor);

static void   picman_dynamics_output_editor_input_toggled  (GtkCellRenderer          *cell,
                                                          gchar                    *path,
                                                          PicmanDynamicsOutputEditor *editor);

static void   picman_dynamics_output_editor_activate_input (PicmanDynamicsOutputEditor *editor,
                                                          gint                      input);

static void   picman_dynamics_output_editor_notify_output  (PicmanDynamicsOutput       *output,
                                                          const GParamSpec         *pspec,
                                                          PicmanDynamicsOutputEditor *editor);


G_DEFINE_TYPE (PicmanDynamicsOutputEditor, picman_dynamics_output_editor,
               GTK_TYPE_BOX)

#define parent_class picman_dynamics_output_editor_parent_class


static void
picman_dynamics_output_editor_class_init (PicmanDynamicsOutputEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_dynamics_output_editor_constructed;
  object_class->finalize     = picman_dynamics_output_editor_finalize;
  object_class->set_property = picman_dynamics_output_editor_set_property;
  object_class->get_property = picman_dynamics_output_editor_get_property;

  g_object_class_install_property (object_class, PROP_OUTPUT,
                                   g_param_spec_object ("output",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DYNAMICS_OUTPUT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (object_class,
                            sizeof (PicmanDynamicsOutputEditorPrivate));
}

static void
picman_dynamics_output_editor_init (PicmanDynamicsOutputEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (editor), 6);
}

static void
picman_dynamics_output_editor_constructed (GObject *object)
{
  PicmanDynamicsOutputEditor        *editor;
  PicmanDynamicsOutputEditorPrivate *private;
  GtkWidget                       *view;
  GtkWidget                       *button;
  GtkCellRenderer                 *cell;
  GtkTreeSelection                *tree_sel;
  gint                             i;
  PicmanDynamicsOutputType           output_type;
  const gchar                     *type_desc;

  editor  = PICMAN_DYNAMICS_OUTPUT_EDITOR (object);
  private = GET_PRIVATE (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_DYNAMICS_OUTPUT (private->output));

  private->curve_view = picman_curve_view_new ();
  g_object_set (private->curve_view,
                "border-width", CURVE_BORDER,
                NULL);

  g_object_get (private->output,
               "type", &output_type,
               NULL);

  if (picman_enum_get_value (PICMAN_TYPE_DYNAMICS_OUTPUT_TYPE, output_type,
                           NULL, NULL, &type_desc, NULL))
    g_object_set (private->curve_view,
                  "y-axis-label", type_desc,
                  NULL);

  gtk_widget_set_size_request (private->curve_view,
                               CURVE_SIZE + CURVE_BORDER * 2,
                               CURVE_SIZE + CURVE_BORDER * 2);
  gtk_box_pack_start (GTK_BOX (editor), private->curve_view, TRUE, TRUE, 0);
  gtk_widget_show (private->curve_view);

  picman_dynamics_output_editor_activate_input (editor, 0);

  button = gtk_button_new_with_mnemonic (_("_Reset Curve"));
  gtk_box_pack_start (GTK_BOX (editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (picman_dynamics_output_editor_curve_reset),
                    editor);

  private->input_list = gtk_list_store_new (INPUT_N_COLUMNS,
                                            G_TYPE_INT,
                                            G_TYPE_BOOLEAN,
                                            G_TYPE_STRING,
                                            PICMAN_TYPE_RGB);

  for (i = 0; i < G_N_ELEMENTS (inputs); i++)
    {
      gboolean use_input;

      g_object_get (private->output,
                    inputs[i].use_property, &use_input,
                    NULL);

      gtk_list_store_insert_with_values (private->input_list,
                                         &private->input_iters[i], -1,
                                         INPUT_COLUMN_INDEX,     i,
                                         INPUT_COLUMN_USE_INPUT, use_input,
                                         INPUT_COLUMN_NAME,      gettext (inputs[i].label),
                                         INPUT_COLUMN_COLOR,     &inputs[i].color,
                                         -1);
    }

  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (private->input_list));
  g_object_unref (private->input_list);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

  cell = gtk_cell_renderer_toggle_new ();

  g_object_set (cell,
                "mode",        GTK_CELL_RENDERER_MODE_ACTIVATABLE,
                "activatable", TRUE,
                NULL);

  g_signal_connect (G_OBJECT (cell), "toggled",
                    G_CALLBACK (picman_dynamics_output_editor_input_toggled),
                    editor);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1, NULL,
                                               picman_cell_renderer_color_new (),
                                               "color", INPUT_COLUMN_COLOR,
                                               NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1, NULL,
                                               cell,
                                               "active", INPUT_COLUMN_USE_INPUT,
                                               NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", INPUT_COLUMN_NAME,
                                               NULL);

  gtk_box_pack_start (GTK_BOX (editor), view, FALSE, FALSE, 0);
  gtk_widget_show (view);

  private->input_view = view;

  tree_sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_BROWSE);

  gtk_tree_selection_select_iter (tree_sel, &private->input_iters[0]);

  g_signal_connect (G_OBJECT (tree_sel), "changed",
                    G_CALLBACK (picman_dynamics_output_editor_input_selected),
                    editor);

  g_signal_connect (private->output, "notify",
                    G_CALLBACK (picman_dynamics_output_editor_notify_output),
                    editor);
}

static void
picman_dynamics_output_editor_finalize (GObject *object)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (object);

  if (private->output)
    {
      g_object_unref (private->output);
      private->output = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_dynamics_output_editor_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_OUTPUT:
      private->output = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dynamics_output_editor_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_OUTPUT:
      g_value_set_object (value, private->output);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


static void
picman_dynamics_output_editor_curve_reset (GtkWidget                *button,
                                         PicmanDynamicsOutputEditor *editor)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (editor);

  if (private->active_curve)
    picman_curve_reset (private->active_curve, TRUE);
}

static void
picman_dynamics_output_editor_input_selected (GtkTreeSelection         *selection,
                                            PicmanDynamicsOutputEditor *editor)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gint input;

      gtk_tree_model_get (model, &iter,
                          INPUT_COLUMN_INDEX, &input,
                          -1);

      picman_dynamics_output_editor_activate_input (editor, input);
    }
}

static void
picman_dynamics_output_editor_input_toggled (GtkCellRenderer          *cell,
                                           gchar                    *path,
                                           PicmanDynamicsOutputEditor *editor)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (editor);
  GtkTreeModel                    *model;
  GtkTreeIter                      iter;

  model = GTK_TREE_MODEL (private->input_list);

  if (gtk_tree_model_get_iter_from_string (model, &iter, path))
    {
      gint     input;
      gboolean use;

      gtk_tree_model_get (model, &iter,
                          INPUT_COLUMN_INDEX,     &input,
                          INPUT_COLUMN_USE_INPUT, &use,
                          -1);

      g_object_set (private->output,
                    inputs[input].use_property, ! use,
                    NULL);
    }
}

static void
picman_dynamics_output_editor_activate_input (PicmanDynamicsOutputEditor *editor,
                                            gint                      input)
{
  PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (editor);
  gint                             i;

  picman_curve_view_set_curve (PICMAN_CURVE_VIEW (private->curve_view), NULL, NULL);
  picman_curve_view_remove_all_backgrounds (PICMAN_CURVE_VIEW (private->curve_view));

  for (i = 0; i < G_N_ELEMENTS (inputs); i++)
    {
      gboolean   use_input;
      PicmanCurve *input_curve;

      g_object_get (private->output,
                    inputs[i].use_property,   &use_input,
                    inputs[i].curve_property, &input_curve,
                    NULL);

      if (input == i)
        {
          picman_curve_view_set_curve (PICMAN_CURVE_VIEW (private->curve_view),
                                     input_curve, &inputs[i].color);
          private->active_curve = input_curve;

          picman_curve_view_set_x_axis_label (PICMAN_CURVE_VIEW (private->curve_view),
                                            inputs[i].label);
        }
      else if (use_input)
        {
          picman_curve_view_add_background (PICMAN_CURVE_VIEW (private->curve_view),
                                          input_curve,
                                          &inputs[i].color);
        }

      g_object_unref (input_curve);
    }
}

static void
picman_dynamics_output_editor_notify_output (PicmanDynamicsOutput       *output,
                                           const GParamSpec         *pspec,
                                           PicmanDynamicsOutputEditor *editor)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (inputs); i++)
    {
      if (! strcmp (pspec->name, inputs[i].use_property))
        {
          PicmanDynamicsOutputEditorPrivate *private = GET_PRIVATE (editor);
          GtkTreeSelection                *sel;
          gboolean                         use_input;
          PicmanCurve                       *input_curve;

          sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (private->input_view));

          g_object_get (output,
                        pspec->name,              &use_input,
                        inputs[i].curve_property, &input_curve,
                        NULL);

          gtk_list_store_set (private->input_list, &private->input_iters[i],
                              INPUT_COLUMN_USE_INPUT, use_input,
                              -1);

          if (! gtk_tree_selection_iter_is_selected (sel,
                                                     &private->input_iters[i]))
            {
              if (use_input)
                {
                  picman_curve_view_add_background (PICMAN_CURVE_VIEW (private->curve_view),
                                                  input_curve,
                                                  &inputs[i].color);
                }
              else
                {
                  picman_curve_view_remove_background (PICMAN_CURVE_VIEW (private->curve_view),
                                                     input_curve);
                }

              g_object_unref (input_curve);
            }

          break;
        }
    }
}


/*  public functions  */

GtkWidget *
picman_dynamics_output_editor_new (PicmanDynamicsOutput *output)
{
  g_return_val_if_fail (PICMAN_IS_DYNAMICS_OUTPUT (output), NULL);

  return g_object_new (PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR,
                       "output", output,
                       NULL);
}
