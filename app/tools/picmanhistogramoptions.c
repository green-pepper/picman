/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#include "config/picmanconfig-utils.h"

#include "widgets/picmanhistogramview.h"

#include "picmanhistogramoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SCALE
};


static void   picman_histogram_options_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);
static void   picman_histogram_options_get_property (GObject      *object,
                                                   guint         property_id,
                                                   GValue       *value,
                                                   GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanHistogramOptions, picman_histogram_options,
               PICMAN_TYPE_COLOR_OPTIONS)


static void
picman_histogram_options_class_init (PicmanHistogramOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_histogram_options_set_property;
  object_class->get_property = picman_histogram_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_SCALE,
                                 "histogram-scale", NULL,
                                 PICMAN_TYPE_HISTOGRAM_SCALE,
                                 PICMAN_HISTOGRAM_SCALE_LINEAR,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_histogram_options_init (PicmanHistogramOptions *options)
{
}

static void
picman_histogram_options_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanHistogramOptions *options = PICMAN_HISTOGRAM_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SCALE:
      options->scale = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_histogram_options_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanHistogramOptions *options = PICMAN_HISTOGRAM_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SCALE:
      g_value_set_enum (value, options->scale);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_histogram_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget *frame;

  frame = picman_prop_enum_radio_frame_new (config, "histogram-scale",
                                          _("Histogram Scale"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  return vbox;
}

void
picman_histogram_options_connect_view (PicmanHistogramOptions *options,
                                     PicmanHistogramView    *view)
{
  g_return_if_fail (PICMAN_IS_HISTOGRAM_OPTIONS (options));
  g_return_if_fail (PICMAN_IS_HISTOGRAM_VIEW (view));

  picman_config_connect (G_OBJECT (options), G_OBJECT (view), "histogram-scale");

  g_object_notify (G_OBJECT (options), "histogram-scale");
}
