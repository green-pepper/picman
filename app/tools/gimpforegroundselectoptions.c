/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#if 0
#include "base/siox.h"
#endif

#include "widgets/picmanwidgets-utils.h"

#include "picmanforegroundselectoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_ANTIALIAS,
  PROP_CONTIGUOUS,
  PROP_BACKGROUND,
  PROP_STROKE_WIDTH,
  PROP_SMOOTHNESS,
  PROP_MASK_COLOR,
  PROP_EXPANDED,
  PROP_SENSITIVITY_L,
  PROP_SENSITIVITY_A,
  PROP_SENSITIVITY_B
};


static void   picman_foreground_select_options_set_property (GObject      *object,
                                                           guint         property_id,
                                                           const GValue *value,
                                                           GParamSpec   *pspec);
static void   picman_foreground_select_options_get_property (GObject      *object,
                                                           guint         property_id,
                                                           GValue       *value,
                                                           GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanForegroundSelectOptions, picman_foreground_select_options,
               PICMAN_TYPE_SELECTION_OPTIONS)


static void
picman_foreground_select_options_class_init (PicmanForegroundSelectOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_foreground_select_options_set_property;
  object_class->get_property = picman_foreground_select_options_get_property;

  /*  override the antialias default value from PicmanSelectionOptions  */
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ANTIALIAS,
                                    "antialias",
                                    N_("Smooth edges"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONTIGUOUS,
                                    "contiguous",
                                    N_("Select a single contiguous area"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_BACKGROUND,
                                    "background",
                                    N_("Paint over areas to mark color values for "
                                       "inclusion or exclusion from selection"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_STROKE_WIDTH,
                                "stroke-width",
                                N_("Size of the brush used for refinements"),
                                1, 80, 18,
                                PICMAN_PARAM_STATIC_STRINGS);

#if 0
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_SMOOTHNESS,
                                "smoothness",
                                N_("Smaller values give a more accurate "
                                  "selection border but may introduce holes "
                                  "in the selection"),
                                0, 8, SIOX_DEFAULT_SMOOTHNESS,
                                PICMAN_PARAM_STATIC_STRINGS);
#endif

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_MASK_COLOR,
                                 "mask-color",
                                 N_("Color of selection preview mask"),
                                 PICMAN_TYPE_CHANNEL_TYPE,
                                 PICMAN_BLUE_CHANNEL,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_EXPANDED,
                                    "expanded", NULL,
                                    FALSE,
                                    0);

#if 0
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SENSITIVITY_L,
                                   "sensitivity-l",
                                   N_("Sensitivity for brightness component"),
                                   0.0, 10.0, SIOX_DEFAULT_SENSITIVITY_L,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SENSITIVITY_A,
                                   "sensitivity-a",
                                   N_("Sensitivity for red/green component"),
                                   0.0, 10.0, SIOX_DEFAULT_SENSITIVITY_A,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SENSITIVITY_B,
                                   "sensitivity-b",
                                   N_("Sensitivity for yellow/blue component"),
                                   0.0, 10.0, SIOX_DEFAULT_SENSITIVITY_B,
                                   PICMAN_PARAM_STATIC_STRINGS);
#endif
}

static void
picman_foreground_select_options_init (PicmanForegroundSelectOptions *options)
{
}

static void
picman_foreground_select_options_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  PicmanForegroundSelectOptions *options = PICMAN_FOREGROUND_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_ANTIALIAS:
      PICMAN_SELECTION_OPTIONS (object)->antialias = g_value_get_boolean (value);
      break;

    case PROP_CONTIGUOUS:
      options->contiguous = g_value_get_boolean (value);
      break;

    case PROP_BACKGROUND:
      options->background = g_value_get_boolean (value);
      break;

    case PROP_STROKE_WIDTH:
      options->stroke_width = g_value_get_int (value);
      break;

    case PROP_SMOOTHNESS:
      options->smoothness = g_value_get_int (value);
      break;

    case PROP_MASK_COLOR:
      options->mask_color = g_value_get_enum (value);
      break;

    case PROP_EXPANDED:
      options->expanded = g_value_get_boolean (value);
      break;

    case PROP_SENSITIVITY_L:
      options->sensitivity[0] = g_value_get_double (value);
      break;

    case PROP_SENSITIVITY_A:
      options->sensitivity[1] = g_value_get_double (value);
      break;

    case PROP_SENSITIVITY_B:
      options->sensitivity[2] = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_foreground_select_options_get_property (GObject    *object,
                                             guint       property_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
  PicmanForegroundSelectOptions *options = PICMAN_FOREGROUND_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_ANTIALIAS:
      g_value_set_boolean (value, PICMAN_SELECTION_OPTIONS (object)->antialias);
      break;

    case PROP_CONTIGUOUS:
      g_value_set_boolean (value, options->contiguous);
      break;

    case PROP_BACKGROUND:
      g_value_set_boolean (value, options->background);
      break;

    case PROP_STROKE_WIDTH:
      g_value_set_int (value, options->stroke_width);
      break;

    case PROP_SMOOTHNESS:
      g_value_set_int (value, options->smoothness);
      break;

    case PROP_MASK_COLOR:
      g_value_set_enum (value, options->mask_color);
      break;

    case PROP_EXPANDED:
      g_value_set_boolean (value, options->expanded);
      break;

    case PROP_SENSITIVITY_L:
      g_value_set_double (value, options->sensitivity[0]);
      break;

    case PROP_SENSITIVITY_A:
      g_value_set_double (value, options->sensitivity[1]);
      break;

    case PROP_SENSITIVITY_B:
      g_value_set_double (value, options->sensitivity[2]);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_foreground_select_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_selection_options_gui (tool_options);
  GtkWidget       *hbox;
  GtkWidget       *button;
  GtkWidget       *frame;
  GtkWidget       *scale;
  GtkWidget       *label;
  GtkWidget       *menu;
  GtkWidget       *inner_frame;
  GtkWidget       *table;
  gchar           *title;
  gint             row = 0;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  gtk_widget_set_sensitive (PICMAN_SELECTION_OPTIONS (tool_options)->antialias_toggle,
                            FALSE);

  /*  single / multiple objects  */
  button = picman_prop_check_button_new (config, "contiguous", _("Contiguous"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  foreground / background  */
  title = g_strdup_printf (_("Interactive refinement  (%s)"),
                           picman_get_mod_string (toggle_mask));

  frame = picman_prop_boolean_radio_frame_new (config, "background", title,
                                             _("Mark background"),
                                             _("Mark foreground"));
  g_free (title);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  stroke width  */
  inner_frame = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (gtk_bin_get_child (GTK_BIN (frame))),
                      inner_frame, FALSE, FALSE, 2);
  gtk_widget_show (inner_frame);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (inner_frame), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Small brush"));
  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_("Large brush"));
  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  scale = picman_prop_hscale_new (config, "stroke-width", 1.0, 5.0, 0);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_box_pack_start (GTK_BOX (inner_frame), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  smoothness  */
  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

#if 0
  scale = picman_prop_hscale_new (config, "smoothness", 0.1, 1.0, 0);
  gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_RIGHT);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Smoothing:"), 0.0, 0.5, scale, 2, FALSE);
#endif

  /*  mask color */
  menu = picman_prop_enum_combo_box_new (config, "mask-color",
                                       PICMAN_RED_CHANNEL, PICMAN_BLUE_CHANNEL);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Preview color:"), 0.0, 0.5, menu, 2, FALSE);

#if 0
  /*  granularity  */
  frame = picman_prop_expander_new (config, "expanded", _("Color Sensitivity"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  inner_frame = picman_frame_new ("<expander>");
  gtk_container_add (GTK_CONTAINER (frame), inner_frame);
  gtk_widget_show (inner_frame);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_container_add (GTK_CONTAINER (inner_frame), table);
  gtk_widget_show (table);

  picman_prop_opacity_entry_new (config, "sensitivity-l",
                               GTK_TABLE (table), 0, row++, "L");

  picman_prop_opacity_entry_new (config, "sensitivity-a",
                               GTK_TABLE (table), 0, row++, "a");

  picman_prop_opacity_entry_new (config, "sensitivity-b",
                               GTK_TABLE (table), 0, row++, "b");
#endif

  return vbox;
}

void
picman_foreground_select_options_get_mask_color (PicmanForegroundSelectOptions *options,
                                               PicmanRGB                     *color)
{
  g_return_if_fail (PICMAN_IS_FOREGROUND_SELECT_OPTIONS (options));
  g_return_if_fail (color != NULL);

  switch (options->mask_color)
    {
    case PICMAN_RED_CHANNEL:
      picman_rgba_set (color, 1, 0, 0, 0.5);
      break;

    case PICMAN_GREEN_CHANNEL:
      picman_rgba_set (color, 0, 1, 0, 0.5);
      break;

    case PICMAN_BLUE_CHANNEL:
      picman_rgba_set (color, 0, 0, 1, 0.5);
      break;

    default:
      g_warn_if_reached ();
      break;
    }
}
