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

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmanmagnifyoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_AUTO_RESIZE,
  PROP_ZOOM_TYPE
};


static void   picman_magnify_options_set_property (GObject         *object,
                                                 guint            property_id,
                                                 const GValue    *value,
                                                 GParamSpec      *pspec);
static void   picman_magnify_options_get_property (GObject         *object,
                                                 guint            property_id,
                                                 GValue          *value,
                                                 GParamSpec      *pspec);

static void   picman_magnify_options_reset        (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanMagnifyOptions, picman_magnify_options,
               PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_magnify_options_parent_class


static void
picman_magnify_options_class_init (PicmanMagnifyOptionsClass *klass)
{
  GObjectClass         *object_class  = G_OBJECT_CLASS (klass);
  PicmanToolOptionsClass *options_class = PICMAN_TOOL_OPTIONS_CLASS (klass);

  object_class->set_property = picman_magnify_options_set_property;
  object_class->get_property = picman_magnify_options_get_property;

  options_class->reset       = picman_magnify_options_reset;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_AUTO_RESIZE,
                                    "auto-resize",
                                    N_("Resize image window to accommodate "
                                       "new zoom level"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_ZOOM_TYPE,
                                 "zoom-type",
                                 N_("Direction of magnification"),
                                 PICMAN_TYPE_ZOOM_TYPE,
                                 PICMAN_ZOOM_IN,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_magnify_options_init (PicmanMagnifyOptions *options)
{
}

static void
picman_magnify_options_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanMagnifyOptions *options = PICMAN_MAGNIFY_OPTIONS (object);

  switch (property_id)
    {
    case PROP_AUTO_RESIZE:
      options->auto_resize = g_value_get_boolean (value);
      break;
    case PROP_ZOOM_TYPE:
      options->zoom_type = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_magnify_options_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanMagnifyOptions *options = PICMAN_MAGNIFY_OPTIONS (object);

  switch (property_id)
    {
    case PROP_AUTO_RESIZE:
      g_value_set_boolean (value, options->auto_resize);
      break;
    case PROP_ZOOM_TYPE:
      g_value_set_enum (value, options->zoom_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_magnify_options_reset (PicmanToolOptions *tool_options)
{
  GParamSpec *pspec;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (tool_options),
                                        "auto-resize");

  if (pspec)
    G_PARAM_SPEC_BOOLEAN (pspec)->default_value =
      PICMAN_DISPLAY_CONFIG (tool_options->tool_info->picman->config)->resize_windows_on_zoom;

  PICMAN_TOOL_OPTIONS_CLASS (parent_class)->reset (tool_options);
}

GtkWidget *
picman_magnify_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget       *frame;
  GtkWidget       *button;
  gchar           *str;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  /*  the auto_resize toggle button  */
  button = picman_prop_check_button_new (config, "auto-resize",
                                       _("Auto-resize window"));
  gtk_box_pack_start (GTK_BOX (vbox),  button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /*  tool toggle  */
  str = g_strdup_printf (_("Direction  (%s)"),
                         picman_get_mod_string (toggle_mask));

  frame = picman_prop_enum_radio_frame_new (config, "zoom-type",
                                          str, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_free (str);

  return vbox;
}
