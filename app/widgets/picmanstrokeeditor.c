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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmandashpattern.h"
#include "core/picmanstrokeoptions.h"

#include "picmancellrendererdashes.h"
#include "picmandasheditor.h"
#include "picmanstrokeeditor.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_OPTIONS,
  PROP_RESOLUTION
};


static void      picman_stroke_editor_constructed  (GObject           *object);
static void      picman_stroke_editor_set_property (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);
static void      picman_stroke_editor_get_property (GObject           *object,
                                                  guint              property_id,
                                                  GValue            *value,
                                                  GParamSpec        *pspec);

static gboolean  picman_stroke_editor_paint_button (GtkWidget         *widget,
                                                  GdkEventExpose    *event,
                                                  gpointer           data);
static void      picman_stroke_editor_dash_preset  (GtkWidget         *widget,
                                                  PicmanStrokeOptions *options);

static void      picman_stroke_editor_combo_fill   (PicmanStrokeOptions *options,
                                                  GtkComboBox       *box);


G_DEFINE_TYPE (PicmanStrokeEditor, picman_stroke_editor, PICMAN_TYPE_FILL_EDITOR)

#define parent_class picman_stroke_editor_parent_class


static void
picman_stroke_editor_class_init (PicmanStrokeEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_stroke_editor_constructed;
  object_class->set_property = picman_stroke_editor_set_property;
  object_class->get_property = picman_stroke_editor_get_property;

  g_object_class_install_property (object_class, PROP_OPTIONS,
                                   g_param_spec_object ("options", NULL, NULL,
                                                        PICMAN_TYPE_STROKE_OPTIONS,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_RESOLUTION,
                                   g_param_spec_double ("resolution", NULL, NULL,
                                                        PICMAN_MIN_RESOLUTION,
                                                        PICMAN_MAX_RESOLUTION,
                                                        72.0,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_stroke_editor_init (PicmanStrokeEditor *editor)
{
}

static void
picman_stroke_editor_constructed (GObject *object)
{
  PicmanFillEditor    *fill_editor = PICMAN_FILL_EDITOR (object);
  PicmanStrokeEditor  *editor      = PICMAN_STROKE_EDITOR (object);
  PicmanStrokeOptions *options;
  PicmanEnumStore     *store;
  GEnumValue        *value;
  GtkWidget         *box;
  GtkWidget         *size;
  GtkWidget         *label;
  GtkWidget         *frame;
  GtkWidget         *table;
  GtkWidget         *expander;
  GtkWidget         *dash_editor;
  GtkWidget         *button;
  GtkCellRenderer   *cell;
  gint               row = 0;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_STROKE_OPTIONS (fill_editor->options));

  options = PICMAN_STROKE_OPTIONS (fill_editor->options);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (editor), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  label = gtk_label_new (_("Line width:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  size = picman_prop_size_entry_new (G_OBJECT (options),
                                   "width", FALSE, "unit",
                                   "%a", PICMAN_SIZE_ENTRY_UPDATE_SIZE,
                                   editor->resolution);
  picman_size_entry_set_pixel_digits (PICMAN_SIZE_ENTRY (size), 1);
  gtk_box_pack_start (GTK_BOX (box), size, FALSE, FALSE, 0);
  gtk_widget_show (size);

  expander = gtk_expander_new_with_mnemonic (_("_Line Style"));
  gtk_box_pack_start (GTK_BOX (editor), expander, FALSE, FALSE, 0);
  gtk_widget_show (expander);

  frame = picman_frame_new ("<expander>");
  gtk_container_add (GTK_CONTAINER (expander), frame);
  gtk_widget_show (frame);

  table = gtk_table_new (5, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_row_spacing (GTK_TABLE (table), 2, 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 4, 6);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  box = picman_prop_enum_stock_box_new (G_OBJECT (options), "cap-style",
                                      "picman-cap", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("_Cap style:"), 0.0, 0.5,
                             box, 2, TRUE);

  box = picman_prop_enum_stock_box_new (G_OBJECT (options), "join-style",
                                      "picman-join", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("_Join style:"), 0.0, 0.5,
                             box, 2, TRUE);

  picman_prop_scale_entry_new (G_OBJECT (options), "miter-limit",
                             GTK_TABLE (table), 0, row++,
                             _("_Miter limit:"),
                             1.0, 1.0, 1,
                             FALSE, 0.0, 0.0);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Dash pattern:"), 0.0, 0.5,
                             frame, 2, FALSE);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), box);
  gtk_widget_show (box);

  dash_editor = picman_dash_editor_new (options);

  button = g_object_new (GTK_TYPE_BUTTON,
                         "width-request", 14,
                         NULL);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect_object (button, "clicked",
                           G_CALLBACK (picman_dash_editor_shift_left),
                           dash_editor, G_CONNECT_SWAPPED);
  g_signal_connect_after (button, "expose-event",
                          G_CALLBACK (picman_stroke_editor_paint_button),
                          button);

  gtk_box_pack_start (GTK_BOX (box), dash_editor, TRUE, TRUE, 0);
  gtk_widget_show (dash_editor);

  button = g_object_new (GTK_TYPE_BUTTON,
                         "width-request", 14,
                         NULL);
  gtk_box_pack_start (GTK_BOX (box), button, FALSE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect_object (button, "clicked",
                           G_CALLBACK (picman_dash_editor_shift_right),
                           dash_editor, G_CONNECT_SWAPPED);
  g_signal_connect_after (button, "expose-event",
                          G_CALLBACK (picman_stroke_editor_paint_button),
                          NULL);


  store = g_object_new (PICMAN_TYPE_ENUM_STORE,
                        "enum-type",      PICMAN_TYPE_DASH_PRESET,
                        "user-data-type", PICMAN_TYPE_DASH_PATTERN,
                        NULL);

  for (value = store->enum_class->values; value->value_name; value++)
    {
      GtkTreeIter  iter = { 0, };
      const gchar *desc;

      desc = picman_enum_value_get_desc (store->enum_class, value);

      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                          PICMAN_INT_STORE_VALUE, value->value,
                          PICMAN_INT_STORE_LABEL, desc,
                          -1);
    }

  box = picman_enum_combo_box_new_with_model (store);
  g_object_unref (store);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (box), PICMAN_DASH_CUSTOM);
  picman_table_attach_aligned (GTK_TABLE (table), 0, row++,
                             _("Dash _preset:"), 0.0, 0.5,
                             box, 2, FALSE);

  cell = g_object_new (PICMAN_TYPE_CELL_RENDERER_DASHES,
                       "xpad", 2,
                       NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (box), cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (box), cell,
                                 "pattern", PICMAN_INT_STORE_USER_DATA);

  picman_stroke_editor_combo_fill (options, GTK_COMBO_BOX (box));

  g_signal_connect (box, "changed",
                    G_CALLBACK (picman_stroke_editor_dash_preset),
                    options);
  g_signal_connect_object (options, "dash-info-changed",
                           G_CALLBACK (picman_int_combo_box_set_active),
                           box, G_CONNECT_SWAPPED);
}

static void
picman_stroke_editor_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanFillEditor   *fill_editor = PICMAN_FILL_EDITOR (object);
  PicmanStrokeEditor *editor      = PICMAN_STROKE_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      if (fill_editor->options)
        g_object_unref (fill_editor->options);
      fill_editor->options = g_value_dup_object (value);
      break;

    case PROP_RESOLUTION:
      editor->resolution = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_stroke_editor_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanFillEditor   *fill_editor = PICMAN_FILL_EDITOR (object);
  PicmanStrokeEditor *editor      = PICMAN_STROKE_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      g_value_set_object (value, fill_editor->options);
      break;

    case PROP_RESOLUTION:
      g_value_set_double (value, editor->resolution);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_stroke_editor_new (PicmanStrokeOptions *options,
                        gdouble            resolution,
                        gboolean           edit_context)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), NULL);

  return g_object_new (PICMAN_TYPE_STROKE_EDITOR,
                       "options",      options,
                       "resolution",   resolution,
                       "edit-context", edit_context ? TRUE : FALSE,
                       NULL);
}

static gboolean
picman_stroke_editor_paint_button (GtkWidget       *widget,
                                 GdkEventExpose  *event,
                                 gpointer         data)
{
  GtkStyle      *style = gtk_widget_get_style (widget);
  GtkAllocation  allocation;
  gint           w;

  gtk_widget_get_allocation (widget, &allocation);

  w = MIN (allocation.width, allocation.height) * 2 / 3;

  gtk_paint_arrow (style,
                   gtk_widget_get_window (widget),
                   gtk_widget_get_state (widget),
                   GTK_SHADOW_IN,
                   &event->area, widget, NULL,
                   data ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT, TRUE,
                   allocation.x + (allocation.width - w) / 2,
                   allocation.y + (allocation.height - w) / 2,
                   w, w);
  return FALSE;
}

static void
picman_stroke_editor_dash_preset (GtkWidget         *widget,
                                PicmanStrokeOptions *options)
{
  gint value;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value) &&
      value != PICMAN_DASH_CUSTOM)
    {
      picman_stroke_options_take_dash_pattern (options, value, NULL);
    }
}

static void
picman_stroke_editor_combo_update (GtkTreeModel      *model,
                                 GParamSpec        *pspec,
                                 PicmanStrokeOptions *options)
{
  GtkTreeIter iter;

  if (picman_int_store_lookup_by_value (model, PICMAN_DASH_CUSTOM, &iter))
    {
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          PICMAN_INT_STORE_USER_DATA,
                          picman_stroke_options_get_dash_info (options),
                          -1);
    }
}

static void
picman_stroke_editor_combo_fill (PicmanStrokeOptions *options,
                               GtkComboBox       *box)
{
  GtkTreeModel *model = gtk_combo_box_get_model (box);
  GtkTreeIter   iter;
  gboolean      iter_valid;

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter))
    {
      gint value;

      gtk_tree_model_get (model, &iter,
                          PICMAN_INT_STORE_VALUE, &value,
                          -1);

      if (value == PICMAN_DASH_CUSTOM)
        {
          gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                              PICMAN_INT_STORE_USER_DATA,
                              picman_stroke_options_get_dash_info (options),
                              -1);

          g_signal_connect_object (options, "notify::dash-info",
                                   G_CALLBACK (picman_stroke_editor_combo_update),
                                   model, G_CONNECT_SWAPPED);
        }
      else
        {
          GArray *pattern = picman_dash_pattern_new_from_preset (value);

          gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                              PICMAN_INT_STORE_USER_DATA, pattern,
                              -1);
          picman_dash_pattern_free (pattern);
        }
    }
}
