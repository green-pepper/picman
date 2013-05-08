/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanfilleditor.c
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include <gegl.h>
#include "widgets-types.h"

#include "core/picmanfilloptions.h"

#include "picmancolorpanel.h"
#include "picmanfilleditor.h"
#include "picmanpropwidgets.h"
#include "picmanviewablebox.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_OPTIONS,
  PROP_EDIT_CONTEXT
};


static void   picman_fill_editor_constructed  (GObject      *object);
static void   picman_fill_editor_finalize     (GObject      *object);
static void   picman_fill_editor_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void   picman_fill_editor_get_property (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanFillEditor, picman_fill_editor, GTK_TYPE_BOX)

#define parent_class picman_fill_editor_parent_class


static void
picman_fill_editor_class_init (PicmanFillEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_fill_editor_constructed;
  object_class->finalize     = picman_fill_editor_finalize;
  object_class->set_property = picman_fill_editor_set_property;
  object_class->get_property = picman_fill_editor_get_property;

  g_object_class_install_property (object_class, PROP_OPTIONS,
                                   g_param_spec_object ("options",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_FILL_OPTIONS,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_EDIT_CONTEXT,
                                   g_param_spec_boolean ("edit-context",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_fill_editor_init (PicmanFillEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (editor), 6);
}

static void
picman_fill_editor_constructed (GObject *object)
{
  PicmanFillEditor *editor = PICMAN_FILL_EDITOR (object);
  GtkWidget      *box;
  GtkWidget      *button;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_FILL_OPTIONS (editor->options));

  box = picman_prop_enum_radio_box_new (G_OBJECT (editor->options), "style",
                                      0, 0);
  gtk_box_pack_start (GTK_BOX (editor), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  if (editor->edit_context)
    {
      GtkWidget *color_button;
      GtkWidget *pattern_box;

      color_button = picman_prop_color_button_new (G_OBJECT (editor->options),
                                                 "foreground",
                                                 _("Fill Color"),
                                                 -1, 24,
                                                 PICMAN_COLOR_AREA_SMALL_CHECKS);
      picman_color_panel_set_context (PICMAN_COLOR_PANEL (color_button),
                                    PICMAN_CONTEXT (editor->options));
      picman_enum_radio_box_add (GTK_BOX (box), color_button,
                               PICMAN_FILL_STYLE_SOLID, FALSE);

      pattern_box = picman_prop_pattern_box_new (NULL,
                                               PICMAN_CONTEXT (editor->options),
                                               NULL, 2,
                                               "pattern-view-type",
                                               "pattern-view-size");
      picman_enum_radio_box_add (GTK_BOX (box), pattern_box,
                               PICMAN_FILL_STYLE_PATTERN, FALSE);
    }

  button = picman_prop_check_button_new (G_OBJECT (editor->options),
                                       "antialias",
                                       _("_Antialiasing"));
  gtk_box_pack_start (GTK_BOX (editor), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
}

static void
picman_fill_editor_finalize (GObject *object)
{
  PicmanFillEditor *editor = PICMAN_FILL_EDITOR (object);

  if (editor->options)
    {
      g_object_unref (editor->options);
      editor->options = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_fill_editor_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanFillEditor *editor = PICMAN_FILL_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      if (editor->options)
        g_object_unref (editor->options);
      editor->options = g_value_dup_object (value);
      break;

    case PROP_EDIT_CONTEXT:
      editor->edit_context = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_fill_editor_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanFillEditor *editor = PICMAN_FILL_EDITOR (object);

  switch (property_id)
    {
    case PROP_OPTIONS:
      g_value_set_object (value, editor->options);
      break;

    case PROP_EDIT_CONTEXT:
      g_value_set_boolean (value, editor->edit_context);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_fill_editor_new (PicmanFillOptions *options,
                      gboolean         edit_context)
{
  g_return_val_if_fail (PICMAN_IS_FILL_OPTIONS (options), NULL);

  return g_object_new (PICMAN_TYPE_FILL_EDITOR,
                       "options",      options,
                       "edit-context", edit_context ? TRUE : FALSE,
                       NULL);
}
