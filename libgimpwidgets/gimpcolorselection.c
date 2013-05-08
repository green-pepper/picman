/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselection.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "picmanwidgetstypes.h"

#include "picmancolorarea.h"
#include "picmancolornotebook.h"
#include "picmancolorscales.h"
#include "picmancolorselect.h"
#include "picmancolorselection.h"
#include "picmanhelpui.h"
#include "picmanstock.h"
#include "picmanwidgets.h"
#include "picmanwidgets-private.h"

#include "picmanwidgetsmarshal.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmancolorselection
 * @title: PicmanColorSelection
 * @short_description: Widget for doing a color selection.
 *
 * Widget for doing a color selection.
 **/


#define COLOR_AREA_SIZE  20


typedef enum
{
  UPDATE_NOTEBOOK  = 1 << 0,
  UPDATE_SCALES    = 1 << 1,
  UPDATE_ENTRY     = 1 << 2,
  UPDATE_COLOR     = 1 << 3
} UpdateType;

#define UPDATE_ALL (UPDATE_NOTEBOOK | \
                    UPDATE_SCALES   | \
                    UPDATE_ENTRY    | \
                    UPDATE_COLOR)

enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_CONFIG
};


static void   picman_color_selection_set_property      (GObject            *object,
                                                      guint               property_id,
                                                      const GValue       *value,
                                                      GParamSpec         *pspec);

static void   picman_color_selection_switch_page       (GtkWidget          *widget,
                                                      gpointer            page,
                                                      guint               page_num,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_notebook_changed  (PicmanColorSelector  *selector,
                                                      const PicmanRGB      *rgb,
                                                      const PicmanHSV      *hsv,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_scales_changed    (PicmanColorSelector  *selector,
                                                      const PicmanRGB      *rgb,
                                                      const PicmanHSV      *hsv,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_color_picked      (GtkWidget          *widget,
                                                      const PicmanRGB      *rgb,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_entry_changed     (PicmanColorHexEntry  *entry,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_channel_changed   (PicmanColorSelector  *selector,
                                                      PicmanColorSelectorChannel channel,
                                                      PicmanColorSelection *selection);
static void   picman_color_selection_new_color_changed (GtkWidget          *widget,
                                                      PicmanColorSelection *selection);

static void   picman_color_selection_update            (PicmanColorSelection *selection,
                                                      UpdateType          update);


G_DEFINE_TYPE (PicmanColorSelection, picman_color_selection, GTK_TYPE_BOX)

#define parent_class picman_color_selection_parent_class

static guint selection_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_selection_class_init (PicmanColorSelectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_color_selection_set_property;

  klass->color_changed       = NULL;

  g_object_class_install_property (object_class, PROP_CONFIG,
                                   g_param_spec_object ("config",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_COLOR_CONFIG,
                                                        G_PARAM_WRITABLE));

  selection_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorSelectionClass, color_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

}

static void
picman_color_selection_init (PicmanColorSelection *selection)
{
  GtkWidget    *main_hbox;
  GtkWidget    *hbox;
  GtkWidget    *vbox;
  GtkWidget    *frame;
  GtkWidget    *label;
  GtkWidget    *entry;
  GtkWidget    *button;
  GtkSizeGroup *new_group;
  GtkSizeGroup *old_group;

  selection->show_alpha = TRUE;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (selection),
                                  GTK_ORIENTATION_VERTICAL);

  picman_rgba_set (&selection->rgb, 0.0, 0.0, 0.0, 1.0);
  picman_rgb_to_hsv (&selection->rgb, &selection->hsv);

  selection->channel = PICMAN_COLOR_SELECTOR_HUE;

  main_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_add (GTK_CONTAINER (selection), main_hbox);
  gtk_widget_show (main_hbox);

  /*  The left vbox with the notebook  */
  selection->left_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (main_hbox), selection->left_vbox,
                      TRUE, TRUE, 0);
  gtk_widget_show (selection->left_vbox);

  if (_picman_ensure_modules_func)
    {
      g_type_class_ref (PICMAN_TYPE_COLOR_SELECT);
      _picman_ensure_modules_func ();
    }

  selection->notebook = picman_color_selector_new (PICMAN_TYPE_COLOR_NOTEBOOK,
                                                 &selection->rgb,
                                                 &selection->hsv,
                                                 selection->channel);

  if (_picman_ensure_modules_func)
    g_type_class_unref (g_type_class_peek (PICMAN_TYPE_COLOR_SELECT));

  picman_color_selector_set_toggles_visible
    (PICMAN_COLOR_SELECTOR (selection->notebook), FALSE);
  gtk_box_pack_start (GTK_BOX (selection->left_vbox), selection->notebook,
                      TRUE, TRUE, 0);
  gtk_widget_show (selection->notebook);

  g_signal_connect (selection->notebook, "color-changed",
                    G_CALLBACK (picman_color_selection_notebook_changed),
                    selection);
  g_signal_connect (PICMAN_COLOR_NOTEBOOK (selection->notebook)->notebook,
                    "switch-page",
                    G_CALLBACK (picman_color_selection_switch_page),
                    selection);

  /*  The hbox for the color_areas  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_end (GTK_BOX (selection->left_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  The labels  */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("Current:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  new_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
  gtk_size_group_add_widget (new_group, label);
  g_object_unref (new_group);

  label = gtk_label_new (_("Old:"));
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  old_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
  gtk_size_group_add_widget (old_group, label);
  g_object_unref (old_group);

  /*  The color areas  */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  selection->new_color = picman_color_area_new (&selection->rgb,
                                              selection->show_alpha ?
                                              PICMAN_COLOR_AREA_SMALL_CHECKS :
                                              PICMAN_COLOR_AREA_FLAT,
                                              GDK_BUTTON1_MASK |
                                              GDK_BUTTON2_MASK);
  gtk_size_group_add_widget (new_group, selection->new_color);
  gtk_box_pack_start (GTK_BOX (vbox), selection->new_color, FALSE, FALSE, 0);
  gtk_widget_show (selection->new_color);

  g_signal_connect (selection->new_color, "color-changed",
                    G_CALLBACK (picman_color_selection_new_color_changed),
                    selection);

  selection->old_color = picman_color_area_new (&selection->rgb,
                                              selection->show_alpha ?
                                              PICMAN_COLOR_AREA_SMALL_CHECKS :
                                              PICMAN_COLOR_AREA_FLAT,
                                              GDK_BUTTON1_MASK |
                                              GDK_BUTTON2_MASK);
  gtk_drag_dest_unset (selection->old_color);
  gtk_size_group_add_widget (old_group, selection->old_color);
  gtk_box_pack_start (GTK_BOX (vbox), selection->old_color, FALSE, FALSE, 0);
  gtk_widget_show (selection->old_color);

  /*  The right vbox with color scales  */
  selection->right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (main_hbox), selection->right_vbox,
                      TRUE, TRUE, 0);
  gtk_widget_show (selection->right_vbox);

  selection->scales = picman_color_selector_new (PICMAN_TYPE_COLOR_SCALES,
                                               &selection->rgb,
                                               &selection->hsv,
                                               selection->channel);
  picman_color_selector_set_toggles_visible
    (PICMAN_COLOR_SELECTOR (selection->scales), TRUE);
  picman_color_selector_set_show_alpha (PICMAN_COLOR_SELECTOR (selection->scales),
                                      selection->show_alpha);
  gtk_box_pack_start (GTK_BOX (selection->right_vbox), selection->scales,
                      TRUE, TRUE, 0);
  gtk_widget_show (selection->scales);

  g_signal_connect (selection->scales, "channel-changed",
                    G_CALLBACK (picman_color_selection_channel_changed),
                    selection);
  g_signal_connect (selection->scales, "color-changed",
                    G_CALLBACK (picman_color_selection_scales_changed),
                    selection);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (selection->right_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  The color picker  */
  button = picman_pick_button_new ();
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "color-picked",
                    G_CALLBACK (picman_color_selection_color_picked),
                    selection);

  /* The hex triplet entry */
  entry = picman_color_hex_entry_new ();
  picman_help_set_help_data (entry,
                           _("Hexadecimal color notation as used in HTML and "
                             "CSS.  This entry also accepts CSS color names."),
                           NULL);
  gtk_box_pack_end (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_widget_show (entry);

  label = gtk_label_new_with_mnemonic (_("HTML _notation:"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
  gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  g_object_set_data (G_OBJECT (selection), "color-hex-entry", entry);

  g_signal_connect (entry, "color-changed",
                    G_CALLBACK (picman_color_selection_entry_changed),
                    selection);
}

static void
picman_color_selection_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanColorSelection *selection = PICMAN_COLOR_SELECTION (object);

  switch (property_id)
    {
    case PROP_CONFIG:
      picman_color_selection_set_config (selection, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/**
 * picman_color_selection_new:
 *
 * Creates a new #PicmanColorSelection widget.
 *
 * Return value: The new #PicmanColorSelection widget.
 **/
GtkWidget *
picman_color_selection_new (void)
{
  return g_object_new (PICMAN_TYPE_COLOR_SELECTION, NULL);
}

/**
 * picman_color_selection_set_show_alpha:
 * @selection:  A #PicmanColorSelection widget.
 * @show_alpha: The new @show_alpha setting.
 *
 * Sets the @show_alpha property of the @selection widget.
 **/
void
picman_color_selection_set_show_alpha (PicmanColorSelection *selection,
                                     gboolean            show_alpha)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));

  if (show_alpha != selection->show_alpha)
    {
      selection->show_alpha = show_alpha ? TRUE : FALSE;

      picman_color_selector_set_show_alpha
        (PICMAN_COLOR_SELECTOR (selection->notebook), selection->show_alpha);
      picman_color_selector_set_show_alpha
        (PICMAN_COLOR_SELECTOR (selection->scales), selection->show_alpha);

      picman_color_area_set_type (PICMAN_COLOR_AREA (selection->new_color),
                                selection->show_alpha ?
                                PICMAN_COLOR_AREA_SMALL_CHECKS :
                                PICMAN_COLOR_AREA_FLAT);
      picman_color_area_set_type (PICMAN_COLOR_AREA (selection->old_color),
                                selection->show_alpha ?
                                PICMAN_COLOR_AREA_SMALL_CHECKS :
                                PICMAN_COLOR_AREA_FLAT);
    }
}

/**
 * picman_color_selection_get_show_alpha:
 * @selection: A #PicmanColorSelection widget.
 *
 * Returns the @selection's @show_alpha property.
 *
 * Return value: #TRUE if the #PicmanColorSelection has alpha controls.
 **/
gboolean
picman_color_selection_get_show_alpha (PicmanColorSelection *selection)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_SELECTION (selection), FALSE);

  return selection->show_alpha;
}

/**
 * picman_color_selection_set_color:
 * @selection: A #PicmanColorSelection widget.
 * @color:     The @color to set as current color.
 *
 * Sets the #PicmanColorSelection's current color to the new @color.
 **/
void
picman_color_selection_set_color (PicmanColorSelection *selection,
                                const PicmanRGB      *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));
  g_return_if_fail (color != NULL);

  selection->rgb = *color;
  picman_rgb_to_hsv (&selection->rgb, &selection->hsv);

  picman_color_selection_update (selection, UPDATE_ALL);

  picman_color_selection_color_changed (selection);
}

/**
 * picman_color_selection_get_color:
 * @selection: A #PicmanColorSelection widget.
 * @color:     Return location for the @selection's current @color.
 *
 * This function returns the #PicmanColorSelection's current color.
 **/
void
picman_color_selection_get_color (PicmanColorSelection *selection,
                                PicmanRGB            *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));
  g_return_if_fail (color != NULL);

  *color = selection->rgb;
}

/**
 * picman_color_selection_set_old_color:
 * @selection: A #PicmanColorSelection widget.
 * @color:     The @color to set as old color.
 *
 * Sets the #PicmanColorSelection's old color.
 **/
void
picman_color_selection_set_old_color (PicmanColorSelection *selection,
                                    const PicmanRGB      *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));
  g_return_if_fail (color != NULL);

  picman_color_area_set_color (PICMAN_COLOR_AREA (selection->old_color), color);
}

/**
 * picman_color_selection_get_old_color:
 * @selection: A #PicmanColorSelection widget.
 * @color:     Return location for the @selection's old @color.
 *
 * This function returns the #PicmanColorSelection's old color.
 **/
void
picman_color_selection_get_old_color (PicmanColorSelection *selection,
                                    PicmanRGB            *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));
  g_return_if_fail (color != NULL);

  picman_color_area_get_color (PICMAN_COLOR_AREA (selection->old_color), color);
}

/**
 * picman_color_selection_reset:
 * @selection: A #PicmanColorSelection widget.
 *
 * Sets the #PicmanColorSelection's current color to its old color.
 **/
void
picman_color_selection_reset (PicmanColorSelection *selection)
{
  PicmanRGB color;

  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));

  picman_color_area_get_color (PICMAN_COLOR_AREA (selection->old_color), &color);
  picman_color_selection_set_color (selection, &color);
}

/**
 * picman_color_selection_color_changed:
 * @selection: A #PicmanColorSelection widget.
 *
 * Emits the "color-changed" signal.
 **/
void
picman_color_selection_color_changed (PicmanColorSelection *selection)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));

  g_signal_emit (selection, selection_signals[COLOR_CHANGED], 0);
}

/**
 * picman_color_selection_set_config:
 * @selection: A #PicmanColorSelection widget.
 * @config:    A #PicmanColorConfig object.
 *
 * Sets the color management configuration to use with this color selection.
 *
 * Since: PICMAN 2.4
 */
void
picman_color_selection_set_config (PicmanColorSelection *selection,
                                 PicmanColorConfig    *config)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTION (selection));
  g_return_if_fail (config == NULL || PICMAN_IS_COLOR_CONFIG (config));

  picman_color_selector_set_config (PICMAN_COLOR_SELECTOR (selection->notebook),
                                  config);
  picman_color_selector_set_config (PICMAN_COLOR_SELECTOR (selection->scales),
                                  config);
}

/*  private functions  */

static void
picman_color_selection_switch_page (GtkWidget          *widget,
                                  gpointer            page,
                                  guint               page_num,
                                  PicmanColorSelection *selection)
{
  PicmanColorNotebook *notebook = PICMAN_COLOR_NOTEBOOK (selection->notebook);
  gboolean           sensitive;

  sensitive =
    (PICMAN_COLOR_SELECTOR_GET_CLASS (notebook->cur_page)->set_channel != NULL);

  picman_color_selector_set_toggles_sensitive
    (PICMAN_COLOR_SELECTOR (selection->scales), sensitive);
}

static void
picman_color_selection_notebook_changed (PicmanColorSelector  *selector,
                                       const PicmanRGB      *rgb,
                                       const PicmanHSV      *hsv,
                                       PicmanColorSelection *selection)
{
  selection->hsv = *hsv;
  selection->rgb = *rgb;

  picman_color_selection_update (selection,
                               UPDATE_SCALES | UPDATE_ENTRY | UPDATE_COLOR);
  picman_color_selection_color_changed (selection);
}

static void
picman_color_selection_scales_changed (PicmanColorSelector  *selector,
                                     const PicmanRGB      *rgb,
                                     const PicmanHSV      *hsv,
                                     PicmanColorSelection *selection)
{
  selection->rgb = *rgb;
  selection->hsv = *hsv;

  picman_color_selection_update (selection,
                               UPDATE_ENTRY | UPDATE_NOTEBOOK | UPDATE_COLOR);
  picman_color_selection_color_changed (selection);
}

static void
picman_color_selection_color_picked (GtkWidget          *widget,
                                   const PicmanRGB      *rgb,
                                   PicmanColorSelection *selection)
{
  picman_color_selection_set_color (selection, rgb);
}

static void
picman_color_selection_entry_changed (PicmanColorHexEntry  *entry,
                                    PicmanColorSelection *selection)
{
  picman_color_hex_entry_get_color (entry, &selection->rgb);

  picman_rgb_to_hsv (&selection->rgb, &selection->hsv);

  picman_color_selection_update (selection,
                               UPDATE_NOTEBOOK | UPDATE_SCALES | UPDATE_COLOR);
  picman_color_selection_color_changed (selection);
}

static void
picman_color_selection_channel_changed (PicmanColorSelector        *selector,
                                      PicmanColorSelectorChannel  channel,
                                      PicmanColorSelection       *selection)
{
  selection->channel = channel;

  picman_color_selector_set_channel (PICMAN_COLOR_SELECTOR (selection->notebook),
                                   selection->channel);
}

static void
picman_color_selection_new_color_changed (GtkWidget          *widget,
                                        PicmanColorSelection *selection)
{
  picman_color_area_get_color (PICMAN_COLOR_AREA (widget), &selection->rgb);
  picman_rgb_to_hsv (&selection->rgb, &selection->hsv);

  picman_color_selection_update (selection,
                               UPDATE_NOTEBOOK | UPDATE_SCALES | UPDATE_ENTRY);
  picman_color_selection_color_changed (selection);
}

static void
picman_color_selection_update (PicmanColorSelection *selection,
                             UpdateType          update)
{
  if (update & UPDATE_NOTEBOOK)
    {
      g_signal_handlers_block_by_func (selection->notebook,
                                       picman_color_selection_notebook_changed,
                                       selection);

      picman_color_selector_set_color (PICMAN_COLOR_SELECTOR (selection->notebook),
                                     &selection->rgb,
                                     &selection->hsv);

      g_signal_handlers_unblock_by_func (selection->notebook,
                                         picman_color_selection_notebook_changed,
                                         selection);
    }

  if (update & UPDATE_SCALES)
    {
      g_signal_handlers_block_by_func (selection->scales,
                                       picman_color_selection_scales_changed,
                                       selection);

      picman_color_selector_set_color (PICMAN_COLOR_SELECTOR (selection->scales),
                                     &selection->rgb,
                                     &selection->hsv);

      g_signal_handlers_unblock_by_func (selection->scales,
                                         picman_color_selection_scales_changed,
                                         selection);
    }

  if (update & UPDATE_ENTRY)
    {
      PicmanColorHexEntry *entry;

      entry = g_object_get_data (G_OBJECT (selection), "color-hex-entry");

      g_signal_handlers_block_by_func (entry,
                                       picman_color_selection_entry_changed,
                                       selection);

      picman_color_hex_entry_set_color (entry, &selection->rgb);

      g_signal_handlers_unblock_by_func (entry,
                                         picman_color_selection_entry_changed,
                                         selection);
    }

  if (update & UPDATE_COLOR)
    {
      g_signal_handlers_block_by_func (selection->new_color,
                                       picman_color_selection_new_color_changed,
                                       selection);

      picman_color_area_set_color (PICMAN_COLOR_AREA (selection->new_color),
                                 &selection->rgb);

      g_signal_handlers_unblock_by_func (selection->new_color,
                                         picman_color_selection_new_color_changed,
                                         selection);
    }
}
