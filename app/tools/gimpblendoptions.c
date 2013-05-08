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

#include "core/picmandatafactory.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanviewablebox.h"

#include "picmanblendoptions.h"
#include "picmanpaintoptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_OFFSET,
  PROP_GRADIENT_TYPE,
  PROP_GRADIENT_REPEAT,  /*  overrides a PicmanPaintOptions property  */
  PROP_SUPERSAMPLE,
  PROP_SUPERSAMPLE_DEPTH,
  PROP_SUPERSAMPLE_THRESHOLD,
  PROP_DITHER
};


static void   picman_blend_options_set_property    (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void   picman_blend_options_get_property    (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);

static void   blend_options_gradient_type_notify (PicmanBlendOptions *options,
                                                  GParamSpec       *pspec,
                                                  GtkWidget        *repeat_combo);


G_DEFINE_TYPE (PicmanBlendOptions, picman_blend_options, PICMAN_TYPE_PAINT_OPTIONS)


static void
picman_blend_options_class_init (PicmanBlendOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_blend_options_set_property;
  object_class->get_property = picman_blend_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_OFFSET,
                                   "offset", NULL,
                                   0.0, 100.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRADIENT_TYPE,
                                 "gradient-type", NULL,
                                 PICMAN_TYPE_GRADIENT_TYPE,
                                 PICMAN_GRADIENT_LINEAR,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRADIENT_REPEAT,
                                 "gradient-repeat", NULL,
                                 PICMAN_TYPE_REPEAT_MODE,
                                 PICMAN_REPEAT_NONE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SUPERSAMPLE,
                                    "supersample", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_SUPERSAMPLE_DEPTH,
                                "supersample-depth", NULL,
                                0, 6, 3,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SUPERSAMPLE_THRESHOLD,
                                   "supersample-threshold", NULL,
                                   0.0, 4.0, 0.2,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DITHER,
                                    "dither", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_blend_options_init (PicmanBlendOptions *options)
{
}

static void
picman_blend_options_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanBlendOptions *options = PICMAN_BLEND_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OFFSET:
      options->offset = g_value_get_double (value);
      break;
    case PROP_GRADIENT_TYPE:
      options->gradient_type = g_value_get_enum (value);
      break;
    case PROP_GRADIENT_REPEAT:
      PICMAN_PAINT_OPTIONS (options)->gradient_options->gradient_repeat =
        g_value_get_enum (value);
      break;

    case PROP_SUPERSAMPLE:
      options->supersample = g_value_get_boolean (value);
      break;
    case PROP_SUPERSAMPLE_DEPTH:
      options->supersample_depth = g_value_get_int (value);
      break;
    case PROP_SUPERSAMPLE_THRESHOLD:
      options->supersample_threshold = g_value_get_double (value);
      break;

    case PROP_DITHER:
      options->dither = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_blend_options_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanBlendOptions *options = PICMAN_BLEND_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OFFSET:
      g_value_set_double (value, options->offset);
      break;
    case PROP_GRADIENT_TYPE:
      g_value_set_enum (value, options->gradient_type);
      break;
    case PROP_GRADIENT_REPEAT:
      g_value_set_enum (value,
                        PICMAN_PAINT_OPTIONS (options)->gradient_options->gradient_repeat);
      break;

    case PROP_SUPERSAMPLE:
      g_value_set_boolean (value, options->supersample);
      break;
    case PROP_SUPERSAMPLE_DEPTH:
      g_value_set_int (value, options->supersample_depth);
      break;
    case PROP_SUPERSAMPLE_THRESHOLD:
      g_value_set_double (value, options->supersample_threshold);
      break;

    case PROP_DITHER:
      g_value_set_boolean (value, options->dither);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_blend_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *table;
  GtkWidget *vbox2;
  GtkWidget *frame;
  GtkWidget *scale;
  GtkWidget *combo;
  GtkWidget *button;

  /*  the gradient  */
  button = picman_prop_gradient_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                       _("Gradient"), 2,
                                       "gradient-view-type",
                                       "gradient-view-size",
                                       "gradient-reverse",
                                       "picman-gradient-editor");
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*  the gradient type menu  */
  combo = picman_prop_enum_combo_box_new (config, "gradient-type", 0, 0);
  g_object_set (combo, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  picman_enum_combo_box_set_stock_prefix (PICMAN_ENUM_COMBO_BOX (combo),
                                        "picman-gradient");
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("Shape:"), 0.0, 0.5,
                             combo, 2, FALSE);

  /*  the repeat option  */
  combo = picman_prop_enum_combo_box_new (config, "gradient-repeat", 0, 0);
  g_object_set (combo, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 1,
                             _("Repeat:"), 0.0, 0.5,
                             combo, 2, FALSE);

  g_signal_connect (config, "notify::gradient-type",
                    G_CALLBACK (blend_options_gradient_type_notify),
                    combo);

  /*  the offset scale  */
  scale = picman_prop_spin_scale_new (config, "offset",
                                    _("Offset"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  the dither toggle  */
  button = picman_prop_check_button_new (config, "dither",
                                       _("Dithering"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  supersampling options  */
  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  frame = picman_prop_expanding_frame_new (config, "supersample",
                                         _("Adaptive supersampling"),
                                         vbox2, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  max depth scale  */
  scale = picman_prop_spin_scale_new (config, "supersample-depth",
                                    _("Max depth"),
                                    1.0, 1.0, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  /*  threshold scale  */
  scale = picman_prop_spin_scale_new (config, "supersample-threshold",
                                    _("Threshold"),
                                    0.01, 0.1, 2);
  gtk_box_pack_start (GTK_BOX (vbox2), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return vbox;
}

static void
blend_options_gradient_type_notify (PicmanBlendOptions *options,
                                    GParamSpec       *pspec,
                                    GtkWidget        *repeat_combo)
{
  gtk_widget_set_sensitive (repeat_combo, options->gradient_type < 6);
}
