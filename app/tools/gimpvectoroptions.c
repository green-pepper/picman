/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectoroptions.c
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

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanvectoroptions.h"
#include "picmantooloptions-gui.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_VECTORS_EDIT_MODE,
  PROP_VECTORS_POLYGONAL
};


static void   picman_vector_options_set_property (GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);
static void   picman_vector_options_get_property (GObject      *object,
                                                guint         property_id,
                                                GValue       *value,
                                                GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanVectorOptions, picman_vector_options, PICMAN_TYPE_TOOL_OPTIONS)


static void
picman_vector_options_class_init (PicmanVectorOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_vector_options_set_property;
  object_class->get_property = picman_vector_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_VECTORS_EDIT_MODE,
                                 "vectors-edit-mode", NULL,
                                 PICMAN_TYPE_VECTOR_MODE,
                                 PICMAN_VECTOR_MODE_DESIGN,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_VECTORS_POLYGONAL,
                                    "vectors-polygonal",
                                    N_("Restrict editing to polygons"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_vector_options_init (PicmanVectorOptions *options)
{
}

static void
picman_vector_options_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanVectorOptions *options = PICMAN_VECTOR_OPTIONS (object);

  switch (property_id)
    {
    case PROP_VECTORS_EDIT_MODE:
      options->edit_mode = g_value_get_enum (value);
      break;
    case PROP_VECTORS_POLYGONAL:
      options->polygonal = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


static void
picman_vector_options_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanVectorOptions *options = PICMAN_VECTOR_OPTIONS (object);

  switch (property_id)
    {
    case PROP_VECTORS_EDIT_MODE:
      g_value_set_enum (value, options->edit_mode);
      break;
    case PROP_VECTORS_POLYGONAL:
      g_value_set_boolean (value, options->polygonal);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
button_append_modifier (GtkWidget       *button,
                        GdkModifierType  modifiers)
{
  gchar *str = g_strdup_printf ("%s (%s)",
                                gtk_button_get_label (GTK_BUTTON (button)),
                                picman_get_mod_string (modifiers));

  gtk_button_set_label (GTK_BUTTON (button), str);
  g_free (str);
}

GtkWidget *
picman_vector_options_gui (PicmanToolOptions *tool_options)
{
  GObject           *config  = G_OBJECT (tool_options);
  PicmanVectorOptions *options = PICMAN_VECTOR_OPTIONS (tool_options);
  GtkWidget         *vbox    = picman_tool_options_gui (tool_options);
  GtkWidget         *frame;
  GtkWidget         *button;
  gchar             *str;

  /*  tool toggle  */
  frame = picman_prop_enum_radio_frame_new (config, "vectors-edit-mode",
                                          _("Edit Mode"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = g_object_get_data (G_OBJECT (frame), "radio-button");

  if (GTK_IS_RADIO_BUTTON (button))
    {
      GSList *list = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

      /* PICMAN_VECTOR_MODE_MOVE  */
      button_append_modifier (list->data, GDK_MOD1_MASK);

      if (list->next)   /* PICMAN_VECTOR_MODE_EDIT  */
        button_append_modifier (list->next->data,
                                picman_get_toggle_behavior_mask ());
    }

  button = picman_prop_check_button_new (config, "vectors-polygonal",
                                       _("Polygonal"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  str = g_strdup_printf (_("Path to Selection\n"
                           "%s  Add\n"
                           "%s  Subtract\n"
                           "%s  Intersect"),
                         picman_get_mod_string (picman_get_extend_selection_mask ()),
                         picman_get_mod_string (picman_get_modify_selection_mask ()),
                         picman_get_mod_string (picman_get_extend_selection_mask () |
                                              picman_get_modify_selection_mask ()));

  button = picman_button_new ();
  /*  Create a selection from the current path  */
  gtk_button_set_label (GTK_BUTTON (button), _("Selection from Path"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  picman_help_set_help_data (button, str, PICMAN_HELP_PATH_SELECTION_REPLACE);
  gtk_widget_show (button);

  g_free (str);

  options->to_selection_button = button;

  button = gtk_button_new_with_label (_("Stroke Path"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (button, FALSE);
  picman_help_set_help_data (button, NULL, PICMAN_HELP_PATH_STROKE);
  gtk_widget_show (button);

  options->stroke_button = button;

  return vbox;
}
