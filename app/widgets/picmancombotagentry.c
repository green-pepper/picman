/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancombotagentry.c
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmantag.h"
#include "core/picmantagged.h"
#include "core/picmantaggedcontainer.h"
#include "core/picmanviewable.h"

#include "picmantagentry.h"
#include "picmantagpopup.h"
#include "picmancombotagentry.h"


static void     picman_combo_tag_entry_constructed       (GObject              *object);
static void     picman_combo_tag_entry_dispose           (GObject              *object);

static gboolean picman_combo_tag_entry_expose            (GtkWidget            *widget,
                                                        GdkEventExpose       *event);
static void     picman_combo_tag_entry_style_set         (GtkWidget            *widget,
                                                        GtkStyle             *previous_style);

static void     picman_combo_tag_entry_icon_press        (GtkWidget            *widget,
                                                        GtkEntryIconPosition  icon_pos,
                                                        GdkEvent             *event,
                                                        gpointer              user_data);

static void     picman_combo_tag_entry_popup_destroy     (GtkObject            *object,
                                                        PicmanComboTagEntry    *entry);

static void     picman_combo_tag_entry_tag_count_changed (PicmanTaggedContainer  *container,
                                                        gint                  tag_count,
                                                        PicmanComboTagEntry    *entry);


G_DEFINE_TYPE (PicmanComboTagEntry, picman_combo_tag_entry, PICMAN_TYPE_TAG_ENTRY);

#define parent_class picman_combo_tag_entry_parent_class


static void
picman_combo_tag_entry_class_init (PicmanComboTagEntryClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed  = picman_combo_tag_entry_constructed;
  object_class->dispose      = picman_combo_tag_entry_dispose;

  widget_class->expose_event = picman_combo_tag_entry_expose;
  widget_class->style_set    = picman_combo_tag_entry_style_set;
}

static void
picman_combo_tag_entry_init (PicmanComboTagEntry *entry)
{
  entry->popup                 = NULL;
  entry->normal_item_attr      = NULL;
  entry->selected_item_attr    = NULL;
  entry->insensitive_item_attr = NULL;

  gtk_widget_add_events (GTK_WIDGET (entry),
                         GDK_BUTTON_PRESS_MASK |
                         GDK_POINTER_MOTION_MASK);

  gtk_entry_set_icon_from_stock (GTK_ENTRY (entry),
                                 GTK_ENTRY_ICON_SECONDARY,
                                 GTK_STOCK_GO_DOWN);

  g_signal_connect (entry, "icon-press",
                    G_CALLBACK (picman_combo_tag_entry_icon_press),
                    NULL);
}

static void
picman_combo_tag_entry_constructed (GObject *object)
{
  PicmanComboTagEntry *entry = PICMAN_COMBO_TAG_ENTRY (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_signal_connect_object (PICMAN_TAG_ENTRY (entry)->container,
                           "tag-count-changed",
                           G_CALLBACK (picman_combo_tag_entry_tag_count_changed),
                           entry, 0);
}

static void
picman_combo_tag_entry_dispose (GObject *object)
{
  PicmanComboTagEntry *combo_entry = PICMAN_COMBO_TAG_ENTRY (object);

  if (combo_entry->arrow_pixbuf)
    {
      g_object_unref (combo_entry->arrow_pixbuf);
      combo_entry->arrow_pixbuf = NULL;
    }

  if (combo_entry->normal_item_attr)
    {
      pango_attr_list_unref (combo_entry->normal_item_attr);
      combo_entry->normal_item_attr = NULL;
    }

  if (combo_entry->selected_item_attr)
    {
      pango_attr_list_unref (combo_entry->selected_item_attr);
      combo_entry->selected_item_attr = NULL;
    }

  if (combo_entry->insensitive_item_attr)
    {
      pango_attr_list_unref (combo_entry->insensitive_item_attr);
      combo_entry->insensitive_item_attr = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
picman_combo_tag_entry_expose (GtkWidget      *widget,
                             GdkEventExpose *event)
{
  PicmanComboTagEntry *entry = PICMAN_COMBO_TAG_ENTRY (widget);

  if (! entry->arrow_pixbuf)
    {
      GtkStyle  *style = gtk_widget_get_style (widget);
      GdkPixmap *pixmap;
      cairo_t   *cr;

      pixmap = gdk_pixmap_new (gtk_widget_get_window (widget), 8, 8, -1);

      cr = gdk_cairo_create (pixmap);
      gdk_cairo_set_source_color (cr, &style->base[GTK_STATE_NORMAL]);
      cairo_paint (cr);
      cairo_destroy (cr);

      gtk_paint_arrow (style, pixmap,
                       GTK_STATE_NORMAL,
                       GTK_SHADOW_NONE, NULL, widget, NULL,
                       GTK_ARROW_DOWN, TRUE,
                       0, 0, 8, 8);

      entry->arrow_pixbuf = gdk_pixbuf_get_from_drawable (NULL, pixmap, NULL,
                                                          0, 0, 0, 0, 8, 8);

      g_object_unref (pixmap);

      gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry),
                                      GTK_ENTRY_ICON_SECONDARY,
                                      entry->arrow_pixbuf);
    }

  return GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);
}

static void
picman_combo_tag_entry_style_set (GtkWidget *widget,
                                GtkStyle  *previous_style)
{
  PicmanComboTagEntry *entry = PICMAN_COMBO_TAG_ENTRY (widget);
  GtkStyle          *style = gtk_widget_get_style (widget);
  GdkColor           color;
  PangoAttribute    *attribute;

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);

  if (entry->normal_item_attr)
    pango_attr_list_unref (entry->normal_item_attr);
  entry->normal_item_attr = pango_attr_list_new ();

  if (style->font_desc)
    {
      attribute = pango_attr_font_desc_new (style->font_desc);
      pango_attr_list_insert (entry->normal_item_attr, attribute);
    }
  color = style->text[GTK_STATE_NORMAL];
  attribute = pango_attr_foreground_new (color.red, color.green, color.blue);
  pango_attr_list_insert (entry->normal_item_attr, attribute);

  if (entry->selected_item_attr)
    pango_attr_list_unref (entry->selected_item_attr);
  entry->selected_item_attr = pango_attr_list_copy (entry->normal_item_attr);

  color = style->text[GTK_STATE_SELECTED];
  attribute = pango_attr_foreground_new (color.red, color.green, color.blue);
  pango_attr_list_insert (entry->selected_item_attr, attribute);
  color = style->base[GTK_STATE_SELECTED];
  attribute = pango_attr_background_new (color.red, color.green, color.blue);
  pango_attr_list_insert (entry->selected_item_attr, attribute);

  if (entry->insensitive_item_attr)
    pango_attr_list_unref (entry->insensitive_item_attr);
  entry->insensitive_item_attr = pango_attr_list_copy (entry->normal_item_attr);

  color = style->text[GTK_STATE_INSENSITIVE];
  attribute = pango_attr_foreground_new (color.red, color.green, color.blue);
  pango_attr_list_insert (entry->insensitive_item_attr, attribute);
  color = style->base[GTK_STATE_INSENSITIVE];
  attribute = pango_attr_background_new (color.red, color.green, color.blue);
  pango_attr_list_insert (entry->insensitive_item_attr, attribute);

  entry->selected_item_color = style->base[GTK_STATE_SELECTED];

  if (entry->arrow_pixbuf)
    {
      g_object_unref (entry->arrow_pixbuf);
      entry->arrow_pixbuf = NULL;
    }
}

/**
 * picman_combo_tag_entry_new:
 * @container: a tagged container to be used.
 * @mode:      tag entry mode to work in.
 *
 * Creates a new #PicmanComboTagEntry widget which extends #PicmanTagEntry by
 * adding ability to pick tags using popup window (similar to combo box).
 *
 * Return value: a new #PicmanComboTagEntry widget.
 **/
GtkWidget *
picman_combo_tag_entry_new (PicmanTaggedContainer *container,
                          PicmanTagEntryMode     mode)
{
  g_return_val_if_fail (PICMAN_IS_TAGGED_CONTAINER (container), NULL);

  return g_object_new (PICMAN_TYPE_COMBO_TAG_ENTRY,
                       "container", container,
                       "mode",      mode,
                       NULL);
}

static void
picman_combo_tag_entry_icon_press (GtkWidget            *widget,
                                 GtkEntryIconPosition  icon_pos,
                                 GdkEvent             *event,
                                 gpointer              user_data)
{
  PicmanComboTagEntry *entry = PICMAN_COMBO_TAG_ENTRY (widget);

  if (! entry->popup)
    {
      PicmanTaggedContainer *container = PICMAN_TAG_ENTRY (entry)->container;
      gint                 tag_count;

      tag_count = picman_tagged_container_get_tag_count (container);

      if (tag_count > 0 && ! PICMAN_TAG_ENTRY (entry)->has_invalid_tags)
        {
          entry->popup = picman_tag_popup_new (entry);
          g_signal_connect (entry->popup, "destroy",
                            G_CALLBACK (picman_combo_tag_entry_popup_destroy),
                            entry);
          picman_tag_popup_show (PICMAN_TAG_POPUP (entry->popup));
        }
    }
  else
    {
      gtk_widget_destroy (entry->popup);
    }
}

static void
picman_combo_tag_entry_popup_destroy (GtkObject         *object,
                                    PicmanComboTagEntry *entry)
{
  entry->popup = NULL;
  gtk_widget_grab_focus (GTK_WIDGET (entry));
}

static void
picman_combo_tag_entry_tag_count_changed (PicmanTaggedContainer *container,
                                        gint                 tag_count,
                                        PicmanComboTagEntry   *entry)
{
  gboolean sensitive;

  sensitive = tag_count > 0 && ! PICMAN_TAG_ENTRY (entry)->has_invalid_tags;

  gtk_entry_set_icon_sensitive (GTK_ENTRY (entry),
                                GTK_ENTRY_ICON_SECONDARY,
                                sensitive);
}
