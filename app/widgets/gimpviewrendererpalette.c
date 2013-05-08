/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererpalette.c
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

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanpalette.h"

#include "picmanviewrendererpalette.h"


#define COLUMNS 16


static void   picman_view_renderer_palette_finalize (GObject          *object);

static void   picman_view_renderer_palette_render   (PicmanViewRenderer *renderer,
                                                   GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererPalette, picman_view_renderer_palette,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_palette_parent_class


static void
picman_view_renderer_palette_class_init (PicmanViewRendererPaletteClass *klass)
{
  GObjectClass          *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  object_class->finalize = picman_view_renderer_palette_finalize;

  renderer_class->render = picman_view_renderer_palette_render;
}

static void
picman_view_renderer_palette_init (PicmanViewRendererPalette *renderer)
{
  renderer->cell_size = 4;
  renderer->draw_grid = FALSE;
  renderer->columns   = COLUMNS;
}

static void
picman_view_renderer_palette_finalize (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_view_renderer_palette_render (PicmanViewRenderer *renderer,
                                   GtkWidget        *widget)
{
  PicmanViewRendererPalette *renderpal = PICMAN_VIEW_RENDERER_PALETTE (renderer);
  PicmanPalette             *palette;
  guchar                  *row;
  guchar                  *dest;
  GList                   *list;
  gdouble                  cell_width;
  gint                     grid_width;
  gint                     dest_stride;
  gint                     y;

  palette = PICMAN_PALETTE (renderer->viewable);

  if (picman_palette_get_n_colors (palette) == 0)
    return;

  grid_width = renderpal->draw_grid ? 1 : 0;

  if (renderpal->cell_size > 0)
    {
      gint n_columns = picman_palette_get_columns (palette);

      if (n_columns > 0)
        cell_width = MAX ((gdouble) renderpal->cell_size,
                          (gdouble) (renderer->width - grid_width) /
                          (gdouble) n_columns);
      else
        cell_width = renderpal->cell_size;
    }
  else
    {
      gint n_columns = picman_palette_get_columns (palette);

      if (n_columns > 0)
        cell_width = ((gdouble) (renderer->width - grid_width) /
                      (gdouble) n_columns);
      else
        cell_width = (gdouble) (renderer->width - grid_width) / 16.0;
    }

  cell_width = MAX (4.0, cell_width);

  renderpal->cell_width = cell_width;

  renderpal->columns = (gdouble) (renderer->width - grid_width) / cell_width;

  renderpal->rows = picman_palette_get_n_colors (palette) / renderpal->columns;
  if (picman_palette_get_n_colors (palette) % renderpal->columns)
    renderpal->rows += 1;

  renderpal->cell_height = MAX (4, ((renderer->height - grid_width) /
                                    renderpal->rows));

  if (! renderpal->draw_grid)
    renderpal->cell_height = MIN (renderpal->cell_height,
                                  renderpal->cell_width);

  list = picman_palette_get_colors (palette);

  if (! renderer->surface)
    renderer->surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                    renderer->width,
                                                    renderer->height);

  cairo_surface_flush (renderer->surface);

  row = g_new (guchar, renderer->width * 4);

  dest        = cairo_image_surface_get_data (renderer->surface);
  dest_stride = cairo_image_surface_get_stride (renderer->surface);

  for (y = 0; y < renderer->height; y++)
    {
      if ((y % renderpal->cell_height) == 0)
        {
          guchar  r, g, b;
          gint    x;
          gint    n = 0;
          guchar *d = row;

          memset (row, renderpal->draw_grid ? 0 : 255, renderer->width * 4);

          r = g = b = (renderpal->draw_grid ? 0 : 255);

          for (x = 0; x < renderer->width; x++, d += 4)
            {
              if ((x % renderpal->cell_width) == 0)
                {
                  if (list && n < renderpal->columns &&
                      renderer->width - x >= renderpal->cell_width)
                    {
                      PicmanPaletteEntry *entry = list->data;

                      list = g_list_next (list);
                      n++;

                      picman_rgb_get_uchar (&entry->color, &r, &g, &b);
                    }
                  else
                    {
                      r = g = b = (renderpal->draw_grid ? 0 : 255);
                    }
                }

              if (renderpal->draw_grid && (x % renderpal->cell_width) == 0)
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (d, 0, 0, 0);
                }
              else
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (d, r, g, b);
                }
            }
        }

      if (renderpal->draw_grid && (y % renderpal->cell_height) == 0)
        {
          memset (dest, 0, renderer->width * 4);
        }
      else
        {
          memcpy (dest, row, renderer->width * 4);
        }

      dest += dest_stride;
    }

  g_free (row);

  cairo_surface_mark_dirty (renderer->surface);

  renderer->needs_render = FALSE;
}


/*  public functions  */

void
picman_view_renderer_palette_set_cell_size (PicmanViewRendererPalette *renderer,
                                          gint                     cell_size)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER_PALETTE (renderer));

  if (cell_size != renderer->cell_size)
    {
      renderer->cell_size = cell_size;

      picman_view_renderer_invalidate (PICMAN_VIEW_RENDERER (renderer));
    }
}

void
picman_view_renderer_palette_set_draw_grid (PicmanViewRendererPalette *renderer,
                                          gboolean                 draw_grid)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER_PALETTE (renderer));

  if (draw_grid != renderer->draw_grid)
    {
      renderer->draw_grid = draw_grid ? TRUE : FALSE;

      picman_view_renderer_invalidate (PICMAN_VIEW_RENDERER (renderer));
    }
}
