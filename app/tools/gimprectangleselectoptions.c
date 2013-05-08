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

#include "core/picmantoolinfo.h"

#include "widgets/picmanpropwidgets.h"

#include "picmanrectangleoptions.h"
#include "picmanrectangleselectoptions.h"
#include "picmanrectangleselecttool.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_ROUND_CORNERS = PICMAN_RECTANGLE_OPTIONS_PROP_LAST + 1,
  PROP_CORNER_RADIUS
};


static void   picman_rectangle_select_options_set_property (GObject      *object,
                                                          guint         property_id,
                                                          const GValue *value,
                                                          GParamSpec   *pspec);
static void   picman_rectangle_select_options_get_property (GObject      *object,
                                                          guint         property_id,
                                                          GValue       *value,
                                                          GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanRectangleSelectOptions,
                         picman_rectangle_select_options,
                         PICMAN_TYPE_SELECTION_OPTIONS,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_RECTANGLE_OPTIONS,
                                                NULL))


static void
picman_rectangle_select_options_class_init (PicmanRectangleSelectOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_rectangle_select_options_set_property;
  object_class->get_property = picman_rectangle_select_options_get_property;

  /* The 'highlight' property is defined here because we want different
   * default values for the Crop and the Rectangle Select tools.
   */
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class,
                                    PICMAN_RECTANGLE_OPTIONS_PROP_HIGHLIGHT,
                                    "highlight",
                                    N_("Dim everything outside selection"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ROUND_CORNERS,
                                    "round-corners",
                                    N_("Round corners of selection"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_CORNER_RADIUS,
                                   "corner-radius",
                                   N_("Radius of rounding in pixels"),
                                   0.0, 100.0, 5.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  picman_rectangle_options_install_properties (object_class);
}

static void
picman_rectangle_select_options_init (PicmanRectangleSelectOptions *options)
{
}

static void
picman_rectangle_select_options_set_property (GObject      *object,
                                            guint         property_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  PicmanRectangleSelectOptions *options = PICMAN_RECTANGLE_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_ROUND_CORNERS:
      options->round_corners = g_value_get_boolean (value);
      break;

    case PROP_CORNER_RADIUS:
      options->corner_radius = g_value_get_double (value);
      break;

    default:
      picman_rectangle_options_set_property (object, property_id, value, pspec);
      break;
    }
}

static void
picman_rectangle_select_options_get_property (GObject    *object,
                                            guint       property_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  PicmanRectangleSelectOptions *options = PICMAN_RECTANGLE_SELECT_OPTIONS (object);

  switch (property_id)
    {
    case PROP_ROUND_CORNERS:
      g_value_set_boolean (value, options->round_corners);
      break;

    case PROP_CORNER_RADIUS:
      g_value_set_double (value, options->corner_radius);
      break;

    default:
      picman_rectangle_options_get_property (object, property_id, value, pspec);
      break;
    }
}

GtkWidget *
picman_rectangle_select_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_selection_options_gui (tool_options);

  /*  the round corners frame  */
  if (tool_options->tool_info->tool_type == PICMAN_TYPE_RECTANGLE_SELECT_TOOL)
    {
      GtkWidget *frame;
      GtkWidget *scale;
      GtkWidget *toggle;

      scale = picman_prop_spin_scale_new (config, "corner-radius",
                                        _("Radius"),
                                        1.0, 10.0, 1);

      frame = picman_prop_expanding_frame_new (config, "round-corners",
                                             _("Rounded corners"),
                                             scale, NULL);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);

      toggle = PICMAN_SELECTION_OPTIONS (tool_options)->antialias_toggle;

      g_object_bind_property (config, "round-corners",
                              toggle, "sensitive",
                              G_BINDING_SYNC_CREATE);
    }

  /*  the rectangle options  */
  {
    GtkWidget *vbox_rectangle;

    vbox_rectangle = picman_rectangle_options_gui (tool_options);
    gtk_box_pack_start (GTK_BOX (vbox), vbox_rectangle, FALSE, FALSE, 0);
    gtk_widget_show (vbox_rectangle);
  }

  return vbox;
}
