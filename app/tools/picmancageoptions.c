/* PICMAN - The GNU Image Manipulation Program
 *
 * picmancageoptions.c
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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

#include "picmancageoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CAGE_MODE,
  PROP_FILL_PLAIN_COLOR
};


static void   picman_cage_options_set_property (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec);
static void   picman_cage_options_get_property (GObject      *object,
                                              guint         property_id,
                                              GValue       *value,
                                              GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanCageOptions, picman_cage_options,
               PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_cage_options_parent_class


static void
picman_cage_options_class_init (PicmanCageOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_cage_options_set_property;
  object_class->get_property = picman_cage_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CAGE_MODE,
                                 "cage-mode", NULL,
                                 PICMAN_TYPE_CAGE_MODE,
                                 PICMAN_CAGE_MODE_CAGE_CHANGE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FILL_PLAIN_COLOR,
                                    "fill-plain-color", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_cage_options_init (PicmanCageOptions *options)
{
}

static void
picman_cage_options_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanCageOptions *options = PICMAN_CAGE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CAGE_MODE:
      options->cage_mode = g_value_get_enum (value);
      break;
    case PROP_FILL_PLAIN_COLOR:
      options->fill_plain_color = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_cage_options_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanCageOptions *options = PICMAN_CAGE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CAGE_MODE:
      g_value_set_enum (value, options->cage_mode);
      break;
    case PROP_FILL_PLAIN_COLOR:
      g_value_set_boolean (value, options->fill_plain_color);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_cage_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget *mode;
  GtkWidget *button;

  mode = picman_prop_enum_radio_box_new (config, "cage-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), mode, FALSE, FALSE, 0);
  gtk_widget_show (mode);

  button = picman_prop_check_button_new (config, "fill-plain-color",
                                       _("Fill the original position\n"
                                         "of the cage with a color"));
  gtk_box_pack_start (GTK_BOX (vbox),  button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  return vbox;
}
