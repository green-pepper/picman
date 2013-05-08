/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantoolpalette.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TOOL_PALETTE_H__
#define __PICMAN_TOOL_PALETTE_H__


#define PICMAN_TYPE_TOOL_PALETTE            (picman_tool_palette_get_type ())
#define PICMAN_TOOL_PALETTE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_PALETTE, PicmanToolPalette))
#define PICMAN_TOOL_PALETTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_PALETTE, PicmanToolPaletteClass))
#define PICMAN_IS_TOOL_PALETTE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_PALETTE))
#define PICMAN_IS_TOOL_PALETTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_PALETTE))
#define PICMAN_TOOL_PALETTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_PALETTE, PicmanToolPaletteClass))


typedef struct _PicmanToolPaletteClass PicmanToolPaletteClass;

struct _PicmanToolPalette
{
  GtkToolPalette  parent_instance;
};

struct _PicmanToolPaletteClass
{
  GtkToolPaletteClass  parent_class;
};


GType       picman_tool_palette_get_type        (void) G_GNUC_CONST;

GtkWidget * picman_tool_palette_new             (void);
void        picman_tool_palette_set_toolbox     (PicmanToolPalette   *palette,
                                               PicmanToolbox       *toolbox);
gboolean    picman_tool_palette_get_button_size (PicmanToolPalette   *palette,
                                               gint              *width,
                                               gint              *height);


#endif /* __PICMAN_TOOL_PALETTE_H__ */
