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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanregionselectoptions.h"
#include "picmanregionselecttool.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SELECT_TRANSPARENT,
  PROP_SAMPLE_MERGED,
  PROP_THRESHOLD,
  PROP_SELECT_CRITERION
};


static void   picman_region_select_options_set_property (GObject         *object,
                                                       guint            property_id,
                                                       const GValue    *value,
                                                       GParamSpec      *pspec);
static void   picman_region_select_options_get_property (GObject         *object,
                                                       guint            property_id,
                                                       GValue          *value,
                                                       GParamSpec      *pspec);

static void   picman_region_select_options_reset        (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanRegionSelectOptions, picman_region_select_options,
               PICMAN_TYPE_SELECTION_OPTIONS)

#define parent_class picman_region_select_options_parent_class


static void
picman_region_select_options_class_init (PicmanRegionSelectOptionsClass *klass)
{
  GObjectClass         *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolOptionsClass *options_class = PICMAN_TOOL_OPTIONS_CLASS (klass);

  object_class->set_property = picman_region_select_options_set_property;
  object_class->get_property = picman_region_select_options_get_property;

  options_class->reset       = picman_region_select_options_reset;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SELECT_TRANSPARENT,
                                    "select-transparent",
                                    N_("Allow completely transparent regions "
                                       "to be selected"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SAMPLE_MERGED,
                                    "sample-merged",
                                    N_("Base selection on all visible layers"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_THRESHOLD,
                                   "threshold",
                                   N_("Maximum color difference"),
                                   0.0, 255.0, 15.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_SELECT_CRITERION,
                                 "select-criterion",
                                 N_("Selection criterion"),
                                 PICMAN_TYPE_SELECT_CRITERION,
                                 PICMAN_SELECT_CRITERION_COMPOSITE,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_region_select_options_init (PicmanRegionSelectOptions *options)
{
}

static void
picman_region_select_options_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  PicmanRegionSelectOptions *options = PICMAN_REGION_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SELECT_TRANSPARENT:
      options->select_transparent = g_value_get_boolean (value);
      break;

    case PROP_SAMPLE_MERGED:
      options->sample_merged = g_value_get_boolean (value);
      break;

    case PROP_THRESHOLD:
      options->threshold = g_value_get_double (value);
      break;

    case PROP_SELECT_CRITERION:
      options->select_criterion = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_region_select_options_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  PicmanRegionSelectOptions *options = PICMAN_REGION_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SELECT_TRANSPARENT:
      g_value_set_boolean (value, options->select_transparent);
      break;

    case PROP_SAMPLE_MERGED:
      g_value_set_boolean (value, options->sample_merged);
      break;

    case PROP_THRESHOLD:
      g_value_set_double (value, options->threshold);
      break;

    case PROP_SELECT_CRITERION:
      g_value_set_enum (value, options->select_criterion);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_region_select_options_reset (PicmanToolOptions *tool_options)
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
picman_region_select_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config  = G_OBJECT (tool_options);
  GtkWidget *vbox    = picman_selection_options_gui (tool_options);
  GtkWidget *button;
  GtkWidget *scale;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;

  /*  the select transparent areas toggle  */
  button = picman_prop_check_button_new (config, "select-transparent",
                                       _("Select transparent areas"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  the sample merged toggle  */
  button = picman_prop_check_button_new (config, "sample-merged",
                                       _("Sample merged"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  the threshold scale  */
  scale = picman_prop_spin_scale_new (config, "threshold",
                                    _("Threshold"),
                                    1.0, 16.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  the select criterion combo  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Select by:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (config, "select-criterion", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  return vbox;
}
