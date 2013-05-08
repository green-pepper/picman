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

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanselectionoptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_OPERATION,
  PROP_ANTIALIAS,
  PROP_FEATHER,
  PROP_FEATHER_RADIUS
};


static void   picman_selection_options_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);
static void   picman_selection_options_get_property (GObject      *object,
                                                   guint         property_id,
                                                   GValue       *value,
                                                   GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanSelectionOptions, picman_selection_options,
               PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_selection_options_parent_class


static void
picman_selection_options_class_init (PicmanSelectionOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_selection_options_set_property;
  object_class->get_property = picman_selection_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_OPERATION,
                                 "operation", NULL,
                                 PICMAN_TYPE_CHANNEL_OPS,
                                 PICMAN_CHANNEL_OP_REPLACE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ANTIALIAS,
                                    "antialias",
                                    N_("Smooth edges"),
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FEATHER,
                                    "feather",
                                    N_("Enable feathering of selection edges"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_FEATHER_RADIUS,
                                   "feather-radius",
                                   N_("Radius of feathering"),
                                   0.0, 100.0, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_selection_options_init (PicmanSelectionOptions *options)
{
}

static void
picman_selection_options_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanSelectionOptions *options = PICMAN_SELECTION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OPERATION:
      options->operation = g_value_get_enum (value);
      break;

    case PROP_ANTIALIAS:
      options->antialias = g_value_get_boolean (value);
      break;

    case PROP_FEATHER:
      options->feather = g_value_get_boolean (value);
      break;

    case PROP_FEATHER_RADIUS:
      options->feather_radius = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_selection_options_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanSelectionOptions *options = PICMAN_SELECTION_OPTIONS (object);

  switch (property_id)
    {
    case PROP_OPERATION:
      g_value_set_enum (value, options->operation);
      break;

    case PROP_ANTIALIAS:
      g_value_set_boolean (value, options->antialias);
      break;

    case PROP_FEATHER:
      g_value_set_boolean (value, options->feather);
      break;

    case PROP_FEATHER_RADIUS:
      g_value_set_double (value, options->feather_radius);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static const gchar *
picman_selection_options_get_modifiers (PicmanChannelOps operation)
{
  GdkModifierType extend_mask;
  GdkModifierType modify_mask;
  GdkModifierType modifiers = 0;

  extend_mask = picman_get_extend_selection_mask ();
  modify_mask = picman_get_modify_selection_mask ();

  switch (operation)
    {
    case PICMAN_CHANNEL_OP_ADD:
      modifiers = extend_mask;
      break;

    case PICMAN_CHANNEL_OP_SUBTRACT:
      modifiers = modify_mask;
      break;

    case PICMAN_CHANNEL_OP_REPLACE:
      modifiers = 0;
      break;

    case PICMAN_CHANNEL_OP_INTERSECT:
      modifiers = extend_mask | modify_mask;
      break;
    }

  return picman_get_mod_string (modifiers);
}

GtkWidget *
picman_selection_options_gui (PicmanToolOptions *tool_options)
{
  GObject              *config  = G_OBJECT (tool_options);
  PicmanSelectionOptions *options = PICMAN_SELECTION_OPTIONS (tool_options);
  GtkWidget            *vbox    = picman_tool_options_gui (tool_options);
  GtkWidget            *button;

  /*  the selection operation radio buttons  */
  {
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *box;
    GList     *children;
    GList     *list;
    gint       i;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    label = gtk_label_new (_("Mode:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    box = picman_prop_enum_stock_box_new (config, "operation",
                                        "picman-selection", 0, 0);
    gtk_box_pack_start (GTK_BOX (hbox), box, FALSE, FALSE, 0);
    gtk_widget_show (box);

    children = gtk_container_get_children (GTK_CONTAINER (box));

    /*  add modifier keys to the tooltips  */
    for (list = children, i = 0; list; list = list->next, i++)
      {
        GtkWidget   *button   = list->data;
        const gchar *modifier = picman_selection_options_get_modifiers (i);
        gchar       *tooltip;

        if (! modifier)
          continue;

        tooltip = gtk_widget_get_tooltip_text (button);

        if (tooltip)
          {
            gchar *tip = g_strdup_printf ("%s  <b>%s</b>", tooltip, modifier);

            picman_help_set_help_data_with_markup (button, tip, NULL);

            g_free (tip);
            g_free (tooltip);
          }
        else
          {
            picman_help_set_help_data (button, modifier, NULL);
          }
      }

    /*  move PICMAN_CHANNEL_OP_REPLACE to the front  */
    gtk_box_reorder_child (GTK_BOX (box),
                           GTK_WIDGET (children->next->next->data), 0);

    g_list_free (children);
  }

  /*  the antialias toggle button  */
  button = picman_prop_check_button_new (config, "antialias",
                                       _("Antialiasing"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  options->antialias_toggle = button;

  /*  the feather frame  */
  {
    GtkWidget *frame;
    GtkWidget *scale;

    /*  the feather radius scale  */
    scale = picman_prop_spin_scale_new (config, "feather-radius",
                                      _("Radius"),
                                      1.0, 10.0, 1);

    frame = picman_prop_expanding_frame_new (config, "feather",
                                           _("Feather edges"),
                                           scale, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
    gtk_widget_show (frame);
  }

  return vbox;
}
