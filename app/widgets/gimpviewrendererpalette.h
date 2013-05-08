/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererpalette.h
 * Copyright (C) 2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEW_RENDERER_PALETTE_H__
#define __PICMAN_VIEW_RENDERER_PALETTE_H__

#include "picmanviewrenderer.h"

#define PICMAN_TYPE_VIEW_RENDERER_PALETTE            (picman_view_renderer_palette_get_type ())
#define PICMAN_VIEW_RENDERER_PALETTE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEW_RENDERER_PALETTE, PicmanViewRendererPalette))
#define PICMAN_VIEW_RENDERER_PALETTE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEW_RENDERER_PALETTE, PicmanViewRendererPaletteClass))
#define PICMAN_IS_VIEW_RENDERER_PALETTE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_VIEW_RENDERER_PALETTE))
#define PICMAN_IS_VIEW_RENDERER_PALETTE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEW_RENDERER_PALETTE))
#define PICMAN_VIEW_RENDERER_PALETTE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEW_RENDERER_PALETTE, PicmanViewRendererPaletteClass))


typedef struct _PicmanViewRendererPaletteClass  PicmanViewRendererPaletteClass;

struct _PicmanViewRendererPalette
{
  PicmanViewRenderer  parent_instance;

  gint              cell_size;
  gboolean          draw_grid;

  gint              cell_width;
  gint              cell_height;
  gint              columns;
  gint              rows;
};

struct _PicmanViewRendererPaletteClass
{
  PicmanViewRendererClass  parent_class;
};


GType   picman_view_renderer_palette_get_type    (void) G_GNUC_CONST;

void    picman_view_renderer_palette_set_cell_size (PicmanViewRendererPalette *renderer,
                                                  gint                     cell_size);
void    picman_view_renderer_palette_set_draw_grid (PicmanViewRendererPalette *renderer,
                                                  gboolean                 draw_grid);


#endif /* __PICMAN_VIEW_RENDERER_PALETTE_H__ */
