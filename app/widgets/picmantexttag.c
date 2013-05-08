/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextTag
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#include "libpicmancolor/picmancolor.h"

#include "widgets-types.h"

#include "picmantexttag.h"
#include "picmanwidgets-utils.h"


gint
picman_text_tag_get_size (GtkTextTag *tag)
{
  gint size;

  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), 0);

  g_object_get (tag,
                PICMAN_TEXT_PROP_NAME_SIZE, &size,
                NULL);

  return size;
}

gint
picman_text_tag_get_baseline (GtkTextTag *tag)
{
  gint baseline;

  g_object_get (tag,
                PICMAN_TEXT_PROP_NAME_BASELINE, &baseline,
                NULL);

  return baseline;
}

gint
picman_text_tag_get_kerning (GtkTextTag *tag)
{
  gint kerning;

  g_object_get (tag,
                PICMAN_TEXT_PROP_NAME_KERNING, &kerning,
                NULL);

  return kerning;
}

gchar *
picman_text_tag_get_font (GtkTextTag *tag)
{
  gchar *font;

  g_object_get (tag,
                PICMAN_TEXT_PROP_NAME_FONT, &font,
                NULL);

  return font;
}

void
picman_text_tag_get_color (GtkTextTag *tag,
                         PicmanRGB    *color)
{
  GdkColor *gdk_color;

  g_object_get (tag,
                PICMAN_TEXT_PROP_NAME_COLOR, &gdk_color,
                NULL);

  picman_rgb_set_gdk_color (color, gdk_color);

  gdk_color_free (gdk_color);
}
