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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmandatafactory.h"
#include "core/picmantoolinfo.h"

#include "display/picmandisplay.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanbucketfilloptions.h"
#include "picmanpaintoptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_FILL_MODE,
  PROP_FILL_SELECTION,
  PROP_FILL_TRANSPARENT,
  PROP_SAMPLE_MERGED,
  PROP_THRESHOLD,
  PROP_FILL_CRITERION
};


static void   picman_bucket_fill_options_set_property (GObject         *object,
                                                     guint            property_id,
                                                     const GValue    *value,
                                                     GParamSpec      *pspec);
static void   picman_bucket_fill_options_get_property (GObject         *object,
                                                     guint            property_id,
                                                     GValue          *value,
                                                     GParamSpec      *pspec);

static void   picman_bucket_fill_options_reset        (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanBucketFillOptions, picman_bucket_fill_options,
               PICMAN_TYPE_PAINT_OPTIONS)

#define parent_class picman_bucket_fill_options_parent_class


static void
picman_bucket_fill_options_class_init (PicmanBucketFillOptionsClass *klass)
{
  GObjectClass         *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolOptionsClass *options_class = PICMAN_TOOL_OPTIONS_CLASS (klass);

  object_class->set_property = picman_bucket_fill_options_set_property;
  object_class->get_property = picman_bucket_fill_options_get_property;

  options_class->reset       = picman_bucket_fill_options_reset;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FILL_MODE,
                                 "fill-mode", NULL,
                                 PICMAN_TYPE_BUCKET_FILL_MODE,
                                 PICMAN_FG_BUCKET_FILL,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FILL_SELECTION,
                                    "fill-selection",
                                    N_("Which area will be filled"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FILL_TRANSPARENT,
                                    "fill-transparent",
                                    N_("Allow completely transparent regions "
                                       "to be filled"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SAMPLE_MERGED,
                                    "sample-merged",
                                    N_("Base filled area on all visible "
                                       "layers"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_THRESHOLD,
                                   "threshold",
                                   N_("Maximum color difference"),
                                   0.0, 255.0, 15.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FILL_CRITERION,
                                 "fill-criterion",
                                 N_("Criterion used for determining color similarity"),
                                 PICMAN_TYPE_SELECT_CRITERION,
                                 PICMAN_SELECT_CRITERION_COMPOSITE,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_bucket_fill_options_init (PicmanBucketFillOptions *options)
{
}

static void
picman_bucket_fill_options_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanBucketFillOptions *options = PICMAN_BUCKET_FILL_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FILL_MODE:
      options->fill_mode = g_value_get_enum (value);
      break;
    case PROP_FILL_SELECTION:
      options->fill_selection = g_value_get_boolean (value);
      break;
    case PROP_FILL_TRANSPARENT:
      options->fill_transparent = g_value_get_boolean (value);
      break;
    case PROP_SAMPLE_MERGED:
      options->sample_merged = g_value_get_boolean (value);
      break;
    case PROP_THRESHOLD:
      options->threshold = g_value_get_double (value);
      break;
    case PROP_FILL_CRITERION:
      options->fill_criterion = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_bucket_fill_options_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanBucketFillOptions *options = PICMAN_BUCKET_FILL_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FILL_MODE:
      g_value_set_enum (value, options->fill_mode);
      break;
    case PROP_FILL_SELECTION:
      g_value_set_boolean (value, options->fill_selection);
      break;
    case PROP_FILL_TRANSPARENT:
      g_value_set_boolean (value, options->fill_transparent);
      break;
    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, options->sample_merged);
      break;
    case PROP_THRESHOLD:
      g_value_set_double (value, options->threshold);
      break;
    case PROP_FILL_CRITERION:
      g_value_set_enum (value, options->fill_criterion);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_bucket_fill_options_reset (PicmanToolOptions *tool_options)
{
  GParamSpec *pspec;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (tool_options),
                                        "threshold");

  if (pspec)
    G_PARAM_SPEC_DOUBLE (pspec)->default_value =
      tool_options->tool_info->picman->config->default_threshold;

  PICMAN_TOOL_OPTIONS_CLASS (parent_class)->reset (tool_options);
}

GtkWidget *
picman_bucket_fill_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget       *vbox2;
  GtkWidget       *table;
  GtkWidget       *frame;
  GtkWidget       *hbox;
  GtkWidget       *button;
  GtkWidget       *scale;
  GtkWidget       *combo;
  gchar           *str;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  /*  fill type  */
  str = g_strdup_printf (_("Fill Type  (%s)"),
                         picman_get_mod_string (toggle_mask)),
  frame = picman_prop_enum_radio_frame_new (config, "fill-mode", str, 0, 0);
  g_free (str);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  hbox = picman_prop_pattern_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                    NULL, 2,
                                    "pattern-view-type", "pattern-view-size");
  picman_enum_radio_frame_add (GTK_FRAME (frame), hbox,
                             PICMAN_PATTERN_BUCKET_FILL, TRUE);

  /*  fill selection  */
  str = g_strdup_printf (_("Affected Area  (%s)"),
                         picman_get_mod_string (GDK_SHIFT_MASK));
  frame = picman_prop_boolean_radio_frame_new (config, "fill-selection",
                                             str,
                                             _("Fill whole selection"),
                                             _("Fill similar colors"));
  g_free (str);
  gtk_box_reorder_child (GTK_BOX (gtk_bin_get_child (GTK_BIN (frame))),
                         g_object_get_data (G_OBJECT (frame), "radio-button"),
                         1);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  frame = picman_frame_new (_("Finding Similar Colors"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_object_bind_property (config, "fill-selection",
                          frame,  "sensitive",
                          G_BINDING_SYNC_CREATE |
                          G_BINDING_INVERT_BOOLEAN);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox2);
  gtk_widget_show (vbox2);

  /*  the fill transparent areas toggle  */
  button = picman_prop_check_button_new (config, "fill-transparent",
                                       _("Fill transparent areas"));
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  the sample merged toggle  */
  button = picman_prop_check_button_new (config, "sample-merged",
                                       _("Sample merged"));
  gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  the threshold scale  */
  scale = picman_prop_spin_scale_new (config, "threshold",
                                    _("Threshold"),
                                    1.0, 16.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox2), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  the fill criterion combo  */
  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox2), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  combo = picman_prop_enum_combo_box_new (config, "fill-criterion", 0, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Fill by:"), 0.0, 0.5,
                             combo, 2, FALSE);

  return vbox;
}
