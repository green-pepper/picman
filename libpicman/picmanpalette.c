/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * picmanpalette.c
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

#include "picman.h"

/**
 * picman_palette_get_foreground:
 * @foreground: The foreground color.
 *
 * Get the current PICMAN foreground color.
 *
 * This procedure retrieves the current PICMAN foreground color. The
 * foreground color is used in a variety of tools such as paint tools,
 * blending, and bucket fill.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_get_foreground (PicmanRGB *foreground)
{
  return picman_context_get_foreground (foreground);
}

/**
 * picman_palette_get_background:
 * @background: The background color.
 *
 * Get the current PICMAN background color.
 *
 * This procedure retrieves the current PICMAN background color. The
 * background color is used in a variety of tools such as blending,
 * erasing (with non-alpha images), and image filling.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_get_background (PicmanRGB *background)
{
  return picman_context_get_background (background);
}

/**
 * picman_palette_set_foreground:
 * @foreground: The foreground color.
 *
 * Set the current PICMAN foreground color.
 *
 * This procedure sets the current PICMAN foreground color. After this is
 * set, operations which use foreground such as paint tools, blending,
 * and bucket fill will use the new value.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_set_foreground (const PicmanRGB *foreground)
{
  return picman_context_set_foreground (foreground);
}

/**
 * picman_palette_set_background:
 * @background: The background color.
 *
 * Set the current PICMAN background color.
 *
 * This procedure sets the current PICMAN background color. After this is
 * set, operations which use background such as blending, filling
 * images, clearing, and erasing (in non-alpha images) will use the new
 * value.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_set_background (const PicmanRGB *background)
{
  return picman_context_set_background (background);
}

/**
 * picman_palette_set_default_colors:
 *
 * Set the current PICMAN foreground and background colors to black and
 * white.
 *
 * This procedure sets the current PICMAN foreground and background
 * colors to their initial default values, black and white.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_set_default_colors (void)
{
  return picman_context_set_default_colors ();
}

/**
 * picman_palette_swap_colors:
 *
 * Swap the current PICMAN foreground and background colors.
 *
 * This procedure swaps the current PICMAN foreground and background
 * colors, so that the new foreground color becomes the old background
 * color and vice versa.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_palette_swap_colors (void)
{
  return picman_context_swap_colors ();
}
