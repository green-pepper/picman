/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmandeviceinfoeditor.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancurve.h"

#include "picmancurveview.h"
#include "picmandeviceinfo.h"
#include "picmandeviceinfoeditor.h"

#include "picman-intl.h"


#define CURVE_SIZE   256
#define CURVE_BORDER   4


enum
{
  PROP_0,
  PROP_INFO
};

enum
{
  AXIS_COLUMN_INDEX,
  AXIS_COLUMN_NAME,
  AXIS_COLUMN_INPUT_NAME,
  AXIS_N_COLUMNS
};

enum
{
  INPUT_COLUMN_INDEX,
  INPUT_COLUMN_NAME,
  INPUT_N_COLUMNS
};

enum
{
  KEY_COLUMN_INDEX,
  KEY_COLUMN_NAME,
  KEY_COLUMN_KEY,
  KEY_COLUMN_MASK,
  KEY_N_COLUMNS
};


typedef struct _PicmanDeviceInfoEditorPrivate PicmanDeviceInfoEditorPrivate;

struct _PicmanDeviceInfoEditorPrivate
{
  PicmanDeviceInfo *info;

  GtkWidget      *vbox;

  GtkListStore   *input_store;

  GtkListStore   *axis_store;
  GtkTreeIter     axis_iters[GDK_AXIS_LAST - GDK_AXIS_X];

  GtkListStore   *key_store;

  GtkWidget      *notebook;
};

#define PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE(editor) \
        G_TYPE_INSTANCE_GET_PRIVATE (editor, \
                                     PICMAN_TYPE_DEVICE_INFO_EDITOR, \
                                     PicmanDeviceInfoEditorPrivate)


static void   picman_device_info_editor_constructed   (GObject              *object);
static void   picman_device_info_editor_finalize      (GObject              *object);
static void   picman_device_info_editor_set_property  (GObject              *object,
                                                     guint                 property_id,
                                                     const GValue         *value,
                                                     GParamSpec           *pspec);
static void   picman_device_info_editor_get_property  (GObject              *object,
                                                     guint                 property_id,
                                                     GValue               *value,
                                                     GParamSpec           *pspec);

static void   picman_device_info_editor_set_axes      (PicmanDeviceInfoEditor *editor);

static void   picman_device_info_editor_axis_changed  (GtkCellRendererCombo *combo,
                                                     const gchar          *path_string,
                                                     GtkTreeIter          *new_iter,
                                                     PicmanDeviceInfoEditor *editor);
static void   picman_device_info_editor_axis_selected (GtkTreeSelection     *selection,
                                                     PicmanDeviceInfoEditor *editor);

static void   picman_device_info_editor_key_edited    (GtkCellRendererAccel *accel,
                                                     const char           *path_string,
                                                     guint                 accel_key,
                                                     GdkModifierType       accel_mask,
                                                     guint                 hardware_keycode,
                                                     PicmanDeviceInfoEditor *editor);
static void   picman_device_info_editor_key_cleared   (GtkCellRendererAccel *accel,
                                                     const char           *path_string,
                                                     PicmanDeviceInfoEditor *editor);

static void   picman_device_info_editor_curve_reset   (GtkWidget            *button,
                                                     PicmanCurve            *curve);


G_DEFINE_TYPE (PicmanDeviceInfoEditor, picman_device_info_editor, GTK_TYPE_BOX)

#define parent_class picman_device_info_editor_parent_class


static const gchar *const axis_use_strings[] =
{
  N_("X"),
  N_("Y"),
  N_("Pressure"),
  N_("X tilt"),
  N_("Y tilt"),
  /* Wheel as in mouse or input device wheel */
  N_("Wheel")
};


static void
picman_device_info_editor_class_init (PicmanDeviceInfoEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_device_info_editor_constructed;
  object_class->finalize     = picman_device_info_editor_finalize;
  object_class->set_property = picman_device_info_editor_set_property;
  object_class->get_property = picman_device_info_editor_get_property;

  g_object_class_install_property (object_class, PROP_INFO,
                                   g_param_spec_object ("info",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DEVICE_INFO,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (object_class, sizeof (PicmanDeviceInfoEditorPrivate));
}

static void
picman_device_info_editor_init (PicmanDeviceInfoEditor *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  GtkWidget                   *frame;
  GtkWidget                   *frame2;
  GtkWidget                   *view;
  GtkTreeSelection            *sel;
  GtkWidget                   *scrolled_win;
  GtkWidget                   *key_view;
  GtkCellRenderer             *cell;
  gint                         i;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_HORIZONTAL);

  gtk_box_set_spacing (GTK_BOX (editor), 12);

  private->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (editor), private->vbox, TRUE, TRUE, 0);
  gtk_widget_show (private->vbox);

  /*  the axes  */

  /* The axes of an input device */
  frame = picman_frame_new (_("Axes"));
  gtk_box_pack_start (GTK_BOX (private->vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), frame2);
  gtk_widget_show (frame2);

  private->axis_store = gtk_list_store_new (AXIS_N_COLUMNS,
                                            G_TYPE_INT,
                                            G_TYPE_STRING,
                                            G_TYPE_STRING);

  for (i = GDK_AXIS_X; i < GDK_AXIS_LAST; i++)
    {
      const gchar *string = gettext (axis_use_strings[i - 1]);

      gtk_list_store_insert_with_values (private->axis_store,
                                         &private->axis_iters[i - 1], -1,
                                         AXIS_COLUMN_INDEX, i,
                                         AXIS_COLUMN_NAME,  string,
                                         -1);
    }

  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (private->axis_store));
  g_object_unref (private->axis_store);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", AXIS_COLUMN_NAME,
                                               NULL);

  private->input_store = gtk_list_store_new (INPUT_N_COLUMNS,
                                             G_TYPE_INT,
                                             G_TYPE_STRING);

  cell = gtk_cell_renderer_combo_new ();
  g_object_set (cell,
                "mode",        GTK_CELL_RENDERER_MODE_EDITABLE,
                "editable",    TRUE,
                "model",       private->input_store,
                "text-column", INPUT_COLUMN_NAME,
                "has-entry",   FALSE,
                NULL);

  g_object_unref (private->input_store);

  g_signal_connect (cell, "changed",
                    G_CALLBACK (picman_device_info_editor_axis_changed),
                    editor);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1, NULL,
                                               cell,
                                               "text", AXIS_COLUMN_INPUT_NAME,
                                               NULL);

  gtk_container_add (GTK_CONTAINER (frame2), view);
  gtk_widget_show (view);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  gtk_tree_selection_set_mode (sel, GTK_SELECTION_BROWSE);
  gtk_tree_selection_select_iter (sel, &private->axis_iters[0]);

  g_signal_connect (sel, "changed",
                    G_CALLBACK (picman_device_info_editor_axis_selected),
                    editor);

  /*  the keys  */

  frame = picman_frame_new (_("Keys"));
  gtk_box_pack_end (GTK_BOX (private->vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  private->key_store = gtk_list_store_new (KEY_N_COLUMNS,
                                           G_TYPE_INT,
                                           G_TYPE_STRING,
                                           G_TYPE_UINT,
                                           GDK_TYPE_MODIFIER_TYPE);
  key_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (private->key_store));
  g_object_unref (private->key_store);

  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (key_view), FALSE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (key_view),
                                               -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", KEY_COLUMN_NAME,
                                               NULL);

  cell = gtk_cell_renderer_accel_new ();
  g_object_set (cell,
                "mode",     GTK_CELL_RENDERER_MODE_EDITABLE,
                "editable", TRUE,
                NULL);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (key_view),
                                               -1, NULL,
                                               cell,
                                               "accel-key",  KEY_COLUMN_KEY,
                                               "accel-mods", KEY_COLUMN_MASK,
                                               NULL);

  g_signal_connect (cell, "accel-edited",
                    G_CALLBACK (picman_device_info_editor_key_edited),
                    editor);
  g_signal_connect (cell, "accel-cleared",
                    G_CALLBACK (picman_device_info_editor_key_cleared),
                    editor);

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_win),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scrolled_win);
  gtk_widget_show (scrolled_win);

  gtk_container_add (GTK_CONTAINER (scrolled_win), key_view);
  gtk_widget_show (key_view);
}

static void
picman_device_info_editor_constructed (GObject *object)
{
  PicmanDeviceInfoEditor        *editor  = PICMAN_DEVICE_INFO_EDITOR (object);
  PicmanDeviceInfoEditorPrivate *private;
  GtkWidget                   *hbox;
  GtkWidget                   *label;
  GtkWidget                   *combo;
  gint                         n_axes;
  gint                         n_keys;
  gint                         i;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_DEVICE_INFO (private->info));

  /*  the mode menu  */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (private->vbox), hbox, FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (private->vbox), hbox, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Mode:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (G_OBJECT (private->info), "mode",
                                        0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

  /*  the axes  */

  n_axes = picman_device_info_get_n_axes (private->info);

  for (i = -1; i < n_axes; i++)
    {
      gchar name[16];

      if (i == -1)
        g_snprintf (name, sizeof (name), _("none"));
      else
        g_snprintf (name, sizeof (name), "%d", i + 1);

      gtk_list_store_insert_with_values (private->input_store, NULL, -1,
                                         INPUT_COLUMN_INDEX, i,
                                         INPUT_COLUMN_NAME,  name,
                                         -1);
    }

  picman_device_info_editor_set_axes (editor);

  /*  the keys  */

  n_keys = picman_device_info_get_n_keys (private->info);

  for (i = 0; i < n_keys; i++)
    {
      gchar           string[16];
      guint           keyval;
      GdkModifierType modifiers;

      g_snprintf (string, sizeof (string), "%d", i + 1);

      picman_device_info_get_key (private->info, i, &keyval, &modifiers);

      gtk_list_store_insert_with_values (private->key_store, NULL, -1,
                                         KEY_COLUMN_INDEX, i,
                                         KEY_COLUMN_NAME,  string,
                                         KEY_COLUMN_KEY,   keyval,
                                         KEY_COLUMN_MASK,  modifiers,
                                         -1);
    }

  /*  the curves  */

  private->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (editor), private->notebook, TRUE, TRUE, 0);
  gtk_widget_show (private->notebook);

  for (i = GDK_AXIS_X; i < GDK_AXIS_LAST; i++)
    {
      GtkWidget *frame;
      PicmanCurve *curve;
      gchar     *title;

      /* e.g. "Pressure Curve" for mapping input device axes */
      title = g_strdup_printf (_("%s Curve"), axis_use_strings[i - 1]);

      frame = picman_frame_new (title);
      gtk_notebook_append_page (GTK_NOTEBOOK (private->notebook), frame, NULL);
      gtk_widget_show (frame);

      g_free (title);

      curve = picman_device_info_get_curve (private->info, i);

      if (curve)
        {
          GtkWidget *vbox;
          GtkWidget *view;
          GtkWidget *label;
          GtkWidget *combo;
          GtkWidget *button;

          vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
          gtk_box_set_spacing (GTK_BOX (vbox), 6);
          gtk_container_add (GTK_CONTAINER (frame), vbox);
          gtk_widget_show (vbox);

          frame = gtk_frame_new (NULL);
          gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
          gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
          gtk_widget_show (frame);

          view = picman_curve_view_new ();
          g_object_set (view,
                        "picman",         PICMAN_CONTEXT (private->info)->picman,
                        "border-width", CURVE_BORDER,
                        NULL);
          gtk_widget_set_size_request (view,
                                       CURVE_SIZE + CURVE_BORDER * 2,
                                       CURVE_SIZE + CURVE_BORDER * 2);
          gtk_container_add (GTK_CONTAINER (frame), view);
          gtk_widget_show (view);

          picman_curve_view_set_curve (PICMAN_CURVE_VIEW (view), curve, NULL);

          hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
          gtk_box_set_spacing (GTK_BOX (hbox), 6);
          gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
          gtk_widget_show (hbox);

          label = gtk_label_new_with_mnemonic (_("Curve _type:"));
          gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
          gtk_widget_show (label);

          combo = picman_prop_enum_combo_box_new (G_OBJECT (curve),
                                                "curve-type", 0, 0);
          picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (combo),
                                                "picman-curve");
          gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
          gtk_widget_show (combo);

          gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

          button = gtk_button_new_with_mnemonic (_("_Reset Curve"));
          gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
          gtk_widget_show (button);

          g_signal_connect (button, "clicked",
                            G_CALLBACK (picman_device_info_editor_curve_reset),
                            curve);
        }
      else
        {
          GtkWidget *label;
          gchar     *string;

          string = g_strdup_printf (_("The axis '%s' has no curve"),
                                    axis_use_strings[i - 1]);

          label = gtk_label_new (string);
          gtk_container_add (GTK_CONTAINER (frame), label);
          gtk_widget_show (label);

          g_free (string);
        }
    }
}

static void
picman_device_info_editor_finalize (GObject *object)
{
  PicmanDeviceInfoEditorPrivate *private;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (object);

  if (private->info)
    {
      g_object_unref (private->info);
      private->info = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_device_info_editor_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PicmanDeviceInfoEditorPrivate *private;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_INFO:
      private->info = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_info_editor_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PicmanDeviceInfoEditorPrivate *private;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_INFO:
      g_value_set_object (value, private->info);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_info_editor_set_axes (PicmanDeviceInfoEditor *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  gint                         n_axes;
  gint                         i;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  n_axes = picman_device_info_get_n_axes (private->info);

  for (i = GDK_AXIS_X; i < GDK_AXIS_LAST; i++)
    {
      gchar input_name[16];
      gint  j;

      for (j = 0; j < n_axes; j++)
        {
          if (picman_device_info_get_axis_use (private->info, j) == i)
            break;
        }

      if (j == n_axes)
        j = -1;

      if (j == -1)
        g_snprintf (input_name, sizeof (input_name), _("none"));
      else
        g_snprintf (input_name, sizeof (input_name), "%d", j + 1);

      gtk_list_store_set (private->axis_store,
                          &private->axis_iters[i - 1],
                          AXIS_COLUMN_INPUT_NAME, input_name,
                          -1);
    }
}

static void
picman_device_info_editor_axis_changed (GtkCellRendererCombo  *combo,
                                      const gchar           *path_string,
                                      GtkTreeIter           *new_iter,
                                      PicmanDeviceInfoEditor  *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  GtkTreePath                 *path;
  GtkTreeIter                  new_use_iter;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  path = gtk_tree_path_new_from_string (path_string);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (private->axis_store),
                               &new_use_iter, path))
    {
      GdkAxisUse new_use  = GDK_AXIS_IGNORE;
      GdkAxisUse old_use  = GDK_AXIS_IGNORE;
      gint       new_axis = -1;
      gint       old_axis = -1;
      gint       n_axes;
      gint       i;

      gtk_tree_model_get (GTK_TREE_MODEL (private->axis_store), &new_use_iter,
                          AXIS_COLUMN_INDEX, &new_use,
                          -1);

      gtk_tree_model_get (GTK_TREE_MODEL (private->input_store), new_iter,
                          INPUT_COLUMN_INDEX, &new_axis,
                          -1);

      n_axes = picman_device_info_get_n_axes (private->info);

      for (i = 0; i < n_axes; i++)
        if (picman_device_info_get_axis_use (private->info, i) == new_use)
          {
            old_axis = i;
            break;
          }

      if (new_axis == old_axis)
        goto out;

      if (new_axis != -1)
        old_use = picman_device_info_get_axis_use (private->info, new_axis);

      /* we must always have an x and a y axis */
      if ((new_axis == -1 && (new_use == GDK_AXIS_X ||
                              new_use == GDK_AXIS_Y)) ||
          (old_axis == -1 && (old_use == GDK_AXIS_X ||
                              old_use == GDK_AXIS_Y)))
        {
          /* do nothing */
        }
      else
        {
          if (new_axis != -1)
            picman_device_info_set_axis_use (private->info, new_axis, new_use);

          if (old_axis != -1)
            picman_device_info_set_axis_use (private->info, old_axis, old_use);

          picman_device_info_editor_set_axes (editor);
        }
    }

 out:
  gtk_tree_path_free (path);
}

static void
picman_device_info_editor_axis_selected (GtkTreeSelection     *selection,
                                       PicmanDeviceInfoEditor *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  GtkTreeIter                  iter;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      GdkAxisUse use;

      gtk_tree_model_get (GTK_TREE_MODEL (private->axis_store), &iter,
                          AXIS_COLUMN_INDEX, &use,
                          -1);

      gtk_notebook_set_current_page (GTK_NOTEBOOK (private->notebook),
                                     use - 1);
    }
}

static void
picman_device_info_editor_key_edited (GtkCellRendererAccel *accel,
                                    const char           *path_string,
                                    guint                 accel_key,
                                    GdkModifierType       accel_mask,
                                    guint                 hardware_keycode,
                                    PicmanDeviceInfoEditor *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  GtkTreePath                 *path;
  GtkTreeIter                  iter;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  path = gtk_tree_path_new_from_string (path_string);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (private->key_store), &iter, path))
    {
      gint index;

      gtk_tree_model_get (GTK_TREE_MODEL (private->key_store), &iter,
                          KEY_COLUMN_INDEX, &index,
                          -1);

      gtk_list_store_set (private->key_store, &iter,
                          KEY_COLUMN_KEY,  accel_key,
                          KEY_COLUMN_MASK, accel_mask,
                          -1);

      picman_device_info_set_key (private->info, index, accel_key, accel_mask);
    }

  gtk_tree_path_free (path);
}

static void
picman_device_info_editor_key_cleared (GtkCellRendererAccel *accel,
                                     const char           *path_string,
                                     PicmanDeviceInfoEditor *editor)
{
  PicmanDeviceInfoEditorPrivate *private;
  GtkTreePath                 *path;
  GtkTreeIter                  iter;

  private = PICMAN_DEVICE_INFO_EDITOR_GET_PRIVATE (editor);

  path = gtk_tree_path_new_from_string (path_string);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (private->key_store), &iter, path))
    {
      gint index;

      gtk_tree_model_get (GTK_TREE_MODEL (private->key_store), &iter,
                          KEY_COLUMN_INDEX, &index,
                          -1);

      gtk_list_store_set (private->key_store, &iter,
                          KEY_COLUMN_KEY,  0,
                          KEY_COLUMN_MASK, 0,
                          -1);

      picman_device_info_set_key (private->info, index, 0, 0);
    }

  gtk_tree_path_free (path);
}

static void
picman_device_info_editor_curve_reset (GtkWidget *button,
                                     PicmanCurve *curve)
{
  picman_curve_reset (curve, TRUE);
}


/*  public functions  */

GtkWidget *
picman_device_info_editor_new (PicmanDeviceInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_INFO (info), NULL);

  return g_object_new (PICMAN_TYPE_DEVICE_INFO_EDITOR,
                       "info", info,
                       NULL);
}
