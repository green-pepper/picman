/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixelfetcher.c
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

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"


/**
 * SECTION: picmanpixelfetcher
 * @title: picmanpixelfetcher
 * @short_description: Functions for operating on pixel regions.
 *
 * These functions provide neighbourhood-based algorithms which get
 * dramatically slower on region boundaries, to the point where a
 * special treatment for neighbourhoods which are completely inside a
 * tile is called for. It hides the special treatment of tile borders,
 * making plug-in code more readable and shorter.
 **/


struct _PicmanPixelFetcher
{
  gint                      col, row;
  gint                      img_width;
  gint                      img_height;
  gint                      sel_x1, sel_y1, sel_x2, sel_y2;
  gint                      img_bpp;
  gint                      tile_width, tile_height;
  guchar                    bg_color[4];
  PicmanPixelFetcherEdgeMode  mode;
  PicmanDrawable             *drawable;
  PicmanTile                 *tile;
  gboolean                  tile_dirty;
  gboolean                  shadow;
};


/*  local function prototypes  */

static guchar * picman_pixel_fetcher_provide_tile (PicmanPixelFetcher *pf,
                                                 gint              x,
                                                 gint              y);


/*  public functions  */

/**
 * picman_pixel_fetcher_new:
 * @drawable: the #PicmanDrawable the new region will be attached to.
 * @shadow:   a #gboolean indicating whether the region is attached to
 *            the shadow tiles or the real @drawable tiles.
 *
 * Initialize a pixel region from the drawable.
 *
 * Return value: a pointer to a #PicmanPixelRgn structure (or NULL).
 **/
PicmanPixelFetcher *
picman_pixel_fetcher_new (PicmanDrawable *drawable,
                        gboolean      shadow)
{
  PicmanPixelFetcher *pf;
  gint              width;
  gint              height;
  gint              bpp;

  g_return_val_if_fail (drawable != NULL, NULL);

  width  = picman_drawable_width  (drawable->drawable_id);
  height = picman_drawable_height (drawable->drawable_id);
  bpp    = picman_drawable_bpp    (drawable->drawable_id);

  g_return_val_if_fail (width > 0 && height > 0 && bpp > 0, NULL);

  pf = g_slice_new0 (PicmanPixelFetcher);

  picman_drawable_mask_bounds (drawable->drawable_id,
                             &pf->sel_x1, &pf->sel_y1,
                             &pf->sel_x2, &pf->sel_y2);

  pf->col           = -1;
  pf->row           = -1;
  pf->img_width     = width;
  pf->img_height    = height;
  pf->img_bpp       = bpp;
  pf->tile_width    = picman_tile_width ();
  pf->tile_height   = picman_tile_height ();
  pf->bg_color[0]   = 0;
  pf->bg_color[1]   = 0;
  pf->bg_color[2]   = 0;
  pf->bg_color[3]   = 255;
  pf->mode          = PICMAN_PIXEL_FETCHER_EDGE_NONE;
  pf->drawable      = drawable;
  pf->tile          = NULL;
  pf->tile_dirty    = FALSE;
  pf->shadow        = shadow;

  return pf;
}

/**
 * picman_pixel_fetcher_destroy:
 * @pf: a pointer to a previously initialized #PicmanPixelFetcher.
 *
 * Close a previously initializd pixel region.
 **/
void
picman_pixel_fetcher_destroy (PicmanPixelFetcher *pf)
{
  g_return_if_fail (pf != NULL);

  if (pf->tile)
    picman_tile_unref (pf->tile, pf->tile_dirty);

  g_slice_free (PicmanPixelFetcher, pf);
}

/**
 * picman_pixel_fetcher_set_edge_mode:
 * @pf:   a pointer to a previously initialized #PicmanPixelFetcher.
 * @mode: the new edge mode from #PicmanPixelFetcherEdgeMode.
 *
 * Change the edage mode of a previously initialized pixel region.
 **/
void
picman_pixel_fetcher_set_edge_mode (PicmanPixelFetcher         *pf,
                                  PicmanPixelFetcherEdgeMode  mode)
{
  g_return_if_fail (pf != NULL);

  pf->mode = mode;
}

/**
 * picman_pixel_fetcher_set_bg_color:
 * @pf:    a pointer to a previously initialized #PicmanPixelFetcher.
 * @color: the color to be used as bg color.
 *
 * Change the background color of a previously initialized pixel region.
 **/
void
picman_pixel_fetcher_set_bg_color (PicmanPixelFetcher *pf,
                                 const PicmanRGB    *color)
{
  g_return_if_fail (pf != NULL);
  g_return_if_fail (color != NULL);

  switch (pf->img_bpp)
    {
    case 2: pf->bg_color[1] = 255;
    case 1:
      pf->bg_color[0] = picman_rgb_luminance_uchar (color);
      break;

    case 4: pf->bg_color[3] = 255;
    case 3:
      picman_rgb_get_uchar (color,
                          pf->bg_color, pf->bg_color + 1, pf->bg_color + 2);
      break;
    }
}

/**
 * picman_pixel_fetcher_get_pixel:
 * @pf:    a pointer to a previously initialized #PicmanPixelFetcher.
 * @x:     the x coordinate of the pixel to get.
 * @y:     the y coordinate of the pixel to get.
 * @pixel: the memory location where to return the pixel.
 *
 * Get a pixel from the pixel region.
 **/
void
picman_pixel_fetcher_get_pixel (PicmanPixelFetcher *pf,
                              gint              x,
                              gint              y,
                              guchar           *pixel)
{
  guchar *p;
  gint    i;

  g_return_if_fail (pf != NULL);
  g_return_if_fail (pixel != NULL);

  if (pf->mode == PICMAN_PIXEL_FETCHER_EDGE_NONE &&
      (x < pf->sel_x1 || x >= pf->sel_x2 ||
       y < pf->sel_y1 || y >= pf->sel_y2))
    {
      return;
    }

  if (x < 0 || x >= pf->img_width ||
      y < 0 || y >= pf->img_height)
    {
      switch (pf->mode)
        {
        case PICMAN_PIXEL_FETCHER_EDGE_WRAP:
          if (x < 0 || x >= pf->img_width)
            {
              x %= pf->img_width;

              if (x < 0)
                x += pf->img_width;
            }

          if (y < 0 || y >= pf->img_height)
            {
              y %= pf->img_height;

              if (y < 0)
                y += pf->img_height;
            }
          break;

        case PICMAN_PIXEL_FETCHER_EDGE_SMEAR:
          x = CLAMP (x, 0, pf->img_width - 1);
          y = CLAMP (y, 0, pf->img_height - 1);
          break;

        case PICMAN_PIXEL_FETCHER_EDGE_BLACK:
          for (i = 0; i < pf->img_bpp; i++)
            pixel[i] = 0;
          return;

        case PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND:
          for (i = 0; i < pf->img_bpp; i++)
            pixel[i] = pf->bg_color[i];
          return;

        default:
          return;
        }
    }

  p = picman_pixel_fetcher_provide_tile (pf, x, y);

  i = pf->img_bpp;

  do
    {
      *pixel++ = *p++;
    }
  while (--i);
}

/**
 * picman_pixel_fetcher_put_pixel:
 * @pf:    a pointer to a previously initialized #PicmanPixelFetcher.
 * @x:     the x coordinate of the pixel to set.
 * @y:     the y coordinate of the pixel to set.
 * @pixel: the pixel to set.
 *
 * Set a pixel in the pixel region.
 **/
void
picman_pixel_fetcher_put_pixel (PicmanPixelFetcher *pf,
                              gint              x,
                              gint              y,
                              const guchar     *pixel)
{
  guchar *p;
  gint    i;

  g_return_if_fail (pf != NULL);
  g_return_if_fail (pixel != NULL);

  if (x < pf->sel_x1 || x >= pf->sel_x2 ||
      y < pf->sel_y1 || y >= pf->sel_y2)
    {
      return;
    }

  p = picman_pixel_fetcher_provide_tile (pf, x, y);

  i = pf->img_bpp;

  do
    {
      *p++ = *pixel++;
    }
  while (--i);

  pf->tile_dirty = TRUE;
}


/*  private functions  */

static guchar *
picman_pixel_fetcher_provide_tile (PicmanPixelFetcher *pf,
                                 gint              x,
                                 gint              y)
{
  gint col, row;
  gint coloff, rowoff;

  col    = x / pf->tile_width;
  coloff = x % pf->tile_width;
  row    = y / pf->tile_height;
  rowoff = y % pf->tile_height;

  if ((col != pf->col) || (row != pf->row) || (pf->tile == NULL))
    {
      if (pf->tile != NULL)
        picman_tile_unref (pf->tile, pf->tile_dirty);

      pf->tile = picman_drawable_get_tile (pf->drawable, pf->shadow, row, col);
      pf->tile_dirty = FALSE;
      picman_tile_ref (pf->tile);

      pf->col = col;
      pf->row = row;
    }

  return pf->tile->data + pf->img_bpp * (pf->tile->ewidth * rowoff + coloff);
}
