/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGridEditor
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

#include "core/picmancontext.h"
#include "core/picmangrid.h"
#include "core/picmanmarshal.h"

#include "picmancolorpanel.h"
#include "picmangrideditor.h"
#include "picmanpropwidgets.h"

#include "picman-intl.h"


#define GRID_EDITOR_DEFAULT_RESOLUTION   72.0
#define GRID_EDITOR_COLOR_BUTTON_WIDTH   60
#define GRID_EDITOR_COLOR_BUTTON_HEIGHT  24


enum
{
  PROP_0,
  PROP_GRID,
  PROP_CONTEXT,
  PROP_XRESOLUTION,
  PROP_YRESOLUTION
};


static void   picman_grid_editor_constructed  (GObject      *object);
static void   picman_grid_editor_finalize     (GObject      *object);
static void   picman_grid_editor_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void   picman_grid_editor_get_property (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanGridEditor, picman_grid_editor, GTK_TYPE_BOX)

#define parent_class picman_grid_editor_parent_class


static void
picman_grid_editor_class_init (PicmanGridEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_grid_editor_constructed;
  object_class->set_property = picman_grid_editor_set_property;
  object_class->get_property = picman_grid_editor_get_property;
  object_class->finalize     = picman_grid_editor_finalize;

  g_object_class_install_property (object_class, PROP_GRID,
                                   g_param_spec_object ("grid", NULL, NULL,
                                                        PICMAN_TYPE_GRID,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context", NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_XRESOLUTION,
                                   g_param_spec_double ("xresolution", NULL, NULL,
                                                        PICMAN_MIN_RESOLUTION,
                                                        PICMAN_MAX_RESOLUTION,
                                                        GRID_EDITOR_DEFAULT_RESOLUTION,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_YRESOLUTION,
                                   g_param_spec_double ("yresolution", NULL, NULL,
                                                        PICMAN_MIN_RESOLUTION,
                                                        PICMAN_MAX_RESOLUTION,
                                                        GRID_EDITOR_DEFAULT_RESOLUTION,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_grid_editor_init (PicmanGridEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (editor), 12);
}

static void
picman_grid_editor_constructed (GObject *object)
{
  PicmanGridEditor *editor = PICMAN_GRID_EDITOR (object);
  GtkWidget      *frame;
  GtkWidget      *hbox;
  GtkWidget      *table;
  GtkWidget      *style;
  GtkWidget      *color_button;
  GtkWidget      *sizeentry;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (editor->grid != NULL);

  frame = picman_frame_new (_("Appearance"));
  gtk_box_pack_start (GTK_BOX (editor), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_container_add (GTK_CONTAINER (frame), table);

  style = picman_prop_enum_combo_box_new (G_OBJECT (editor->grid), "style",
                                        PICMAN_GRID_DOTS,
                                        PICMAN_GRID_SOLID);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Line _style:"), 0.0, 0.5,
                             style, 1, FALSE);

  color_button = picman_prop_color_button_new (G_OBJECT (editor->grid), "fgcolor",
                                             _("Change grid foreground color"),
                                             GRID_EDITOR_COLOR_BUTTON_WIDTH,
                                             GRID_EDITOR_COLOR_BUTTON_HEIGHT,
                                             PICMAN_COLOR_AREA_FLAT);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (color_button),
                                editor->context);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("_Foreground color:"), 0.0, 0.5,
                             color_button, 1, TRUE);

  color_button = picman_prop_color_button_new (G_OBJECT (editor->grid), "bgcolor",
                                             _("Change grid background color"),
                                             GRID_EDITOR_COLOR_BUTTON_WIDTH,
                                             GRID_EDITOR_COLOR_BUTTON_HEIGHT,
                                             PICMAN_COLOR_AREA_FLAT);
  picman_color_panel_set_context (PICMAN_COLOR_PANEL (color_button),
                                editor->context);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 2,
                             _("_Background color:"), 0.0, 0.5,
                             color_button, 1, TRUE);

  gtk_widget_show (table);

  frame = picman_frame_new (_("Spacing"));
  gtk_box_pack_start (GTK_BOX (editor), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  sizeentry = picman_prop_coordinates_new (G_OBJECT (editor->grid),
                                         "xspacing",
                                         "yspacing",
                                         "spacing-unit",
                                         "%a",
                                         PICMAN_SIZE_ENTRY_UPDATE_SIZE,
                                         editor->xresolution,
                                         editor->yresolution,
                                         TRUE);

  gtk_table_set_col_spacings (GTK_TABLE (sizeentry), 2);
  gtk_table_set_row_spacings (GTK_TABLE (sizeentry), 2);

  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Width"), 0, 1, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Height"), 0, 2, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Pixels"), 1, 4, 0.0);

  gtk_box_pack_start (GTK_BOX (hbox), sizeentry, FALSE, FALSE, 0);
  gtk_widget_show (sizeentry);

  gtk_widget_show (hbox);

  frame = picman_frame_new (_("Offset"));
  gtk_box_pack_start (GTK_BOX (editor), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  sizeentry = picman_prop_coordinates_new (G_OBJECT (editor->grid),
                                         "xoffset",
                                         "yoffset",
                                         "offset-unit",
                                         "%a",
                                         PICMAN_SIZE_ENTRY_UPDATE_SIZE,
                                         editor->xresolution,
                                         editor->yresolution,
                                         TRUE);

  gtk_table_set_col_spacings (GTK_TABLE (sizeentry), 2);
  gtk_table_set_row_spacings (GTK_TABLE (sizeentry), 2);

  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Width"), 0, 1, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Height"), 0, 2, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (sizeentry),
                                _("Pixels"), 1, 4, 0.0);

  gtk_box_pack_start (GTK_BOX (hbox), sizeentry, FALSE, FALSE, 0);
  gtk_widget_show (sizeentry);

  gtk_widget_show (hbox);
}

static void
picman_grid_editor_finalize (GObject *object)
{
  PicmanGridEditor *editor = PICMAN_GRID_EDITOR (object);

  if (editor->grid)
    {
      g_object_unref (editor->grid);
      editor->grid = NULL;
    }

  if (editor->context)
    {
      g_object_unref (editor->context);
      editor->context = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_grid_editor_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanGridEditor *editor = PICMAN_GRID_EDITOR (object);

  switch (property_id)
    {
    case PROP_GRID:
      editor->grid = g_value_dup_object (value);
      break;

    case PROP_CONTEXT:
      editor->context = g_value_dup_object (value);
      break;

    case PROP_XRESOLUTION:
      editor->xresolution = g_value_get_double (value);
      break;

    case PROP_YRESOLUTION:
      editor->yresolution = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_grid_editor_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanGridEditor *editor = PICMAN_GRID_EDITOR (object);

  switch (property_id)
    {
    case PROP_GRID:
      g_value_set_object (value, editor->grid);
      break;

    case PROP_CONTEXT:
      g_value_set_object (value, editor->context);
      break;

    case PROP_XRESOLUTION:
      g_value_set_double (value, editor->xresolution);
      break;

    case PROP_YRESOLUTION:
      g_value_set_double (value, editor->yresolution);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_grid_editor_new (PicmanGrid    *grid,
                      PicmanContext *context,
                      gdouble      xresolution,
                      gdouble      yresolution)
{
  g_return_val_if_fail (PICMAN_IS_GRID (grid), NULL);

  return g_object_new (PICMAN_TYPE_GRID_EDITOR,
                       "grid",        grid,
                       "context",     context,
                       "xresolution", xresolution,
                       "yresolution", yresolution,
                       NULL);
}
