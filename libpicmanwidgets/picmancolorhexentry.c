/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorhexentry.c
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmancolor/picmancolor.h"

#include "picmanwidgetstypes.h"

#include "picmancellrenderercolor.h"
#include "picmancolorhexentry.h"


/**
 * SECTION: picmancolorhexentry
 * @title: PicmanColorHexEntry
 * @short_description: Widget for entering a color's hex triplet.
 *
 * Widget for entering a color's hex triplet.
 **/


enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};

enum
{
  COLUMN_NAME,
  COLUMN_COLOR,
  NUM_COLUMNS
};


static gboolean  picman_color_hex_entry_events     (GtkWidget          *widget,
                                                  GdkEvent           *event);

static gboolean  picman_color_hex_entry_matched    (GtkEntryCompletion *completion,
                                                  GtkTreeModel       *model,
                                                  GtkTreeIter        *iter,
                                                  PicmanColorHexEntry  *entry);


G_DEFINE_TYPE (PicmanColorHexEntry, picman_color_hex_entry, GTK_TYPE_ENTRY)

#define parent_class picman_color_hex_entry_parent_class

static guint entry_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_hex_entry_class_init (PicmanColorHexEntryClass *klass)
{
  entry_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorHexEntryClass, color_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  klass->color_changed = NULL;
}

static void
picman_color_hex_entry_init (PicmanColorHexEntry *entry)
{
  GtkEntryCompletion  *completion;
  GtkCellRenderer     *cell;
  GtkListStore        *store;
  PicmanRGB             *colors;
  const gchar        **names;
  gint                 num_colors;
  gint                 i;

  /* GtkEntry's minimum size is way too large, set a reasonable one
   * for our use case
   */
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 8);

  picman_rgba_set (&entry->color, 0.0, 0.0, 0.0, 1.0);

  store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, PICMAN_TYPE_RGB);

  num_colors = picman_rgb_list_names (&names, &colors);

  for (i = 0; i < num_colors; i++)
    {
      GtkTreeIter  iter;

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          COLUMN_NAME,  names[i],
                          COLUMN_COLOR, colors + i,
                          -1);
    }

  g_free (colors);
  g_free (names);

  completion = g_object_new (GTK_TYPE_ENTRY_COMPLETION,
                             "model", store,
                             NULL);
  g_object_unref (store);

  cell = picman_cell_renderer_color_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (completion), cell, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (completion), cell,
                                  "color", COLUMN_COLOR,
                                  NULL);

  gtk_entry_completion_set_text_column (completion, COLUMN_NAME);

  gtk_entry_set_completion (GTK_ENTRY (entry), completion);
  g_object_unref (completion);

  gtk_entry_set_text (GTK_ENTRY (entry), "000000");

  g_signal_connect (entry, "focus-out-event",
                    G_CALLBACK (picman_color_hex_entry_events),
                    NULL);
  g_signal_connect (entry, "key-press-event",
                    G_CALLBACK (picman_color_hex_entry_events),
                    NULL);

  g_signal_connect (completion, "match-selected",
                    G_CALLBACK (picman_color_hex_entry_matched),
                    entry);
}

/**
 * picman_color_hex_entry_new:
 *
 * Return value: a new #PicmanColorHexEntry widget
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_color_hex_entry_new (void)
{
  return g_object_new (PICMAN_TYPE_COLOR_HEX_ENTRY, NULL);
}

/**
 * picman_color_hex_entry_set_color:
 * @entry: a #PicmanColorHexEntry widget
 * @color: pointer to a #PicmanRGB
 *
 * Sets the color displayed by a #PicmanColorHexEntry. If the new color
 * is different to the previously set color, the "color-changed"
 * signal is emitted.
 *
 * Since: PICMAN 2.2
 **/
void
picman_color_hex_entry_set_color (PicmanColorHexEntry *entry,
                                const PicmanRGB     *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_HEX_ENTRY (entry));
  g_return_if_fail (color != NULL);

  if (picman_rgb_distance (&entry->color, color) > 0.0)
    {
      gchar   buffer[8];
      guchar  r, g, b;

      picman_rgb_set (&entry->color, color->r, color->g, color->b);
      picman_rgb_clamp (&entry->color);

      picman_rgb_get_uchar (&entry->color, &r, &g, &b);
      g_snprintf (buffer, sizeof (buffer), "%.2x%.2x%.2x", r, g, b);

      gtk_entry_set_text (GTK_ENTRY (entry), buffer);

      /* move cursor to the end */
      gtk_editable_set_position (GTK_EDITABLE (entry), -1);

     g_signal_emit (entry, entry_signals[COLOR_CHANGED], 0);
    }
}

/**
 * picman_color_hex_entry_get_color:
 * @entry: a #PicmanColorHexEntry widget
 * @color: pointer to a #PicmanRGB
 *
 * Retrieves the color value displayed by a #PicmanColorHexEntry.
 *
 * Since: PICMAN 2.2
 **/
void
picman_color_hex_entry_get_color (PicmanColorHexEntry *entry,
                                PicmanRGB           *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_HEX_ENTRY (entry));
  g_return_if_fail (color != NULL);

  *color = entry->color;
}

static gboolean
picman_color_hex_entry_events (GtkWidget *widget,
                             GdkEvent  *event)
{
  PicmanColorHexEntry *entry = PICMAN_COLOR_HEX_ENTRY (widget);

  switch (event->type)
    {
    case GDK_KEY_PRESS:
      {
        GdkEventKey *kevent = (GdkEventKey *) event;

        if (kevent->keyval != GDK_KEY_Return   &&
            kevent->keyval != GDK_KEY_KP_Enter &&
            kevent->keyval != GDK_KEY_ISO_Enter)
          break;
        /*  else fall through  */
      }

    case GDK_FOCUS_CHANGE:
      {
        const gchar *text;
        gchar        buffer[8];
        guchar       r, g, b;

        text = gtk_entry_get_text (GTK_ENTRY (widget));

        picman_rgb_get_uchar (&entry->color, &r, &g, &b);
        g_snprintf (buffer, sizeof (buffer), "%.2x%.2x%.2x", r, g, b);

        if (g_ascii_strcasecmp (buffer, text) != 0)
          {
            PicmanRGB  color;
            gsize    len = strlen (text);

            if (len > 0 &&
                (picman_rgb_parse_hex (&color, text, len) ||
                 picman_rgb_parse_name (&color, text, -1)))
              {
                picman_color_hex_entry_set_color (entry, &color);
              }
            else
              {
                gtk_entry_set_text (GTK_ENTRY (entry), buffer);
              }
          }
      }
      break;

    default:
      /*  do nothing  */
      break;
    }

  return FALSE;
}

static gboolean
picman_color_hex_entry_matched (GtkEntryCompletion *completion,
                              GtkTreeModel       *model,
                              GtkTreeIter        *iter,
                              PicmanColorHexEntry  *entry)
{
  gchar   *name;
  PicmanRGB  color;

  gtk_tree_model_get (model, iter,
                      COLUMN_NAME, &name,
                      -1);

  if (picman_rgb_parse_name (&color, name, -1))
    picman_color_hex_entry_set_color (entry, &color);

  g_free (name);

  return TRUE;
}
