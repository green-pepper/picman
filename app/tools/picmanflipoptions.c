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

#include "widgets/picmanwidgets-utils.h"

#include "picmanflipoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_FLIP_TYPE
};


static void   picman_flip_options_set_property (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec);
static void   picman_flip_options_get_property (GObject      *object,
                                              guint         property_id,
                                              GValue       *value,
                                              GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanFlipOptions, picman_flip_options,
               PICMAN_TYPE_TRANSFORM_OPTIONS)


static void
picman_flip_options_class_init (PicmanFlipOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_flip_options_set_property;
  object_class->get_property = picman_flip_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FLIP_TYPE,
                                 "flip-type",
                                 N_("Direction of flipping"),
                                 PICMAN_TYPE_ORIENTATION_TYPE,
                                 PICMAN_ORIENTATION_HORIZONTAL,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_flip_options_init (PicmanFlipOptions *options)
{
}

static void
picman_flip_options_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanFlipOptions *options = PICMAN_FLIP_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FLIP_TYPE:
      options->flip_type = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_flip_options_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanFlipOptions *options = PICMAN_FLIP_OPTIONS (object);

  switch (property_id)
    {
    case PROP_FLIP_TYPE:
      g_value_set_enum (value, options->flip_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_flip_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_tool_options_gui (tool_options);
  GtkWidget       *hbox;
  GtkWidget       *box;
  GtkWidget       *label;
  GtkWidget       *frame;
  gchar           *str;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Affect:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  box = picman_prop_enum_stock_box_new (config, "type", "picman", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  /*  tool toggle  */
  str = g_strdup_printf (_("Flip Type  (%s)"),
                         picman_get_mod_string (toggle_mask));

  frame = picman_prop_enum_radio_frame_new (config, "flip-type",
                                          str,
                                          PICMAN_ORIENTATION_HORIZONTAL,
                                          PICMAN_ORIENTATION_VERTICAL);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_free (str);

  return vbox;
}
