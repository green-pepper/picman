/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselectorpalette.h
 * Copyright (C) 2006 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_COLOR_SELECTOR_PALETTE_H__
#define __PICMAN_COLOR_SELECTOR_PALETTE_H__


#define PICMAN_TYPE_COLOR_SELECTOR_PALETTE            (picman_color_selector_palette_get_type ())
#define PICMAN_COLOR_SELECTOR_PALETTE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_SELECTOR_PALETTE, PicmanColorSelectorPalette))
#define PICMAN_IS_COLOR_SELECTOR_PALETTE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_SELECTOR_PALETTE))
#define PICMAN_COLOR_SELECTOR_PALETTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_SELECTOR_PALETTE, PicmanColorSelectorPaletteClass))
#define PICMAN_IS_COLOR_SELECTOR_PALETTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_SELECTOR_PALETTE))
#define PICMAN_COLOR_SELECTOR_PALETTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_SELECTOR_PALETTE, PicmanColorSelectorPaletteClass))


typedef struct _PicmanColorSelectorPalette      PicmanColorSelectorPalette;
typedef struct _PicmanColorSelectorPaletteClass PicmanColorSelectorPaletteClass;

struct _PicmanColorSelectorPalette
{
  PicmanColorSelector  parent_instance;

  PicmanContext       *context;
  GtkWidget         *view;
};

struct _PicmanColorSelectorPaletteClass
{
  PicmanColorSelectorClass  parent_class;
};


GType   picman_color_selector_palette_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_COLOR_SELECTOR_PALETTE_H__ */
