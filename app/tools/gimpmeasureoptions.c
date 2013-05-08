/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmeasuretool.c
 * Copyright (C) 1999 Sven Neumann <sven@picman.org>
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

#include "picmanmeasureoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_USE_INFO_WINDOW
};


static void   picman_measure_options_set_property (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);
static void   picman_measure_options_get_property (GObject      *object,
                                                 guint         property_id,
                                                 GValue       *value,
                                                 GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanMeasureOptions, picman_measure_options,
               PICMAN_TYPE_TOOL_OPTIONS)


static void
picman_measure_options_class_init (PicmanMeasureOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_measure_options_set_property;
  object_class->get_property = picman_measure_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_INFO_WINDOW,
                                    "use-info-window",
                                    N_("Open a floating dialog to view details "
                                       "about measurements"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_measure_options_init (PicmanMeasureOptions *options)
{
}

static void
picman_measure_options_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanMeasureOptions *options = PICMAN_MEASURE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_USE_INFO_WINDOW:
      options->use_info_window = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_measure_options_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanMeasureOptions *options = PICMAN_MEASURE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_USE_INFO_WINDOW:
      g_value_set_boolean (value, options->use_info_window);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_measure_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget *button;

  /*  the use_info_window toggle button  */
  button = picman_prop_check_button_new (config, "use-info-window",
                                       _("Use info window"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  return vbox;
}
