/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixelrgn.c
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
#include <stdarg.h>

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"


/**
 * SECTION: picmanpixelrgn
 * @title: picmanpixelrgn
 * @short_description: Functions for operating on pixel regions.
 *
 * Functions for operating on pixel regions. These functions provide
 * fast ways of accessing and modifying portions of a drawable.
 **/


#define TILE_WIDTH  picman_tile_width()
#define TILE_HEIGHT picman_tile_height()


typedef struct _PicmanPixelRgnHolder    PicmanPixelRgnHolder;
typedef struct _PicmanPixelRgnIterator  PicmanPixelRgnIterator;

struct _PicmanPixelRgnHolder
{
  PicmanPixelRgn *pr;
  guchar       *original_data;
  gint          startx;
  gint          starty;
  gint          count;
};

struct _PicmanPixelRgnIterator
{
  GSList *pixel_regions;
  gint    region_width;
  gint    region_height;
  gint    portion_width;
  gint    portion_height;
  gint    process_count;
};


static gint     picman_get_portion_width    (PicmanPixelRgnIterator *pri);
static gint     picman_get_portion_height   (PicmanPixelRgnIterator *pri);
static gpointer picman_pixel_rgns_configure (PicmanPixelRgnIterator *pri);
static void     picman_pixel_rgn_configure  (PicmanPixelRgnHolder   *prh,
                                           PicmanPixelRgnIterator *pri);

/**
 * picman_pixel_rgn_init:
 * @pr:        a pointer to a #PicmanPixelRgn variable.
 * @drawable:  the #PicmanDrawable the new region will be attached to.
 * @x:         the x coordinate of the top-left pixel of the region in the
 *             @drawable.
 * @y:         the y coordinate of the top-left pixel of the region in the
 *             @drawable.
 * @width:     the width of the region.
 * @height:    the height of the region.
 * @dirty:     a #gboolean indicating whether the @drawable should be marked
 *             as "dirty".
 * @shadow:    a #gboolean indicating whether the region is attached to the
 *             shadow tiles or the real @drawable tiles.
 *
 * Initialize the pixel region pointed by @pr with the specified parameters.
 *
 * The @dirty and @shadow flags can be used as follows:
 *
 * - @dirty = FALSE, @shadow = FALSE: the region will be used to read
 *                                    the actual drawable datas.  This
 *                                    is useful for save plug-ins or for
 *                                    filters.
 *
 * - @dirty = FALSE, @shadow = TRUE:  the region will be used to read the
 *                                    shadow tiles.  This is used in
 *                                    some filter plug-ins which operate
 *                                    in two passes such as gaussian
 *                                    blur.  The first pass reads the
 *                                    actual drawable data and writes to
 *                                    the shadow tiles, and the second
 *                                    one reads from and writes to the
 *                                    shadow tiles.
 *
 * - @dirty = TRUE, @shadow = TRUE:   the region will be used to write to
 *                                    the shadow tiles. It is common
 *                                    practice to write to the shadow
 *                                    tiles and then use
 *                                    picman_drawable_merge_shadow() to
 *                                    merge the changes from the shadow
 *                                    tiles using the current selection
 *                                    as a mask.
 *
 * - @dirty = TRUE, @shadow = FALSE:  the region will be used to directly
 *                                    change the drawable content. Don't
 *                                    do this, since this could prevent
 *                                    the Undo-System from working as
 *                                    expected.
 **/
void
picman_pixel_rgn_init (PicmanPixelRgn *pr,
                     PicmanDrawable *drawable,
                     gint          x,
                     gint          y,
                     gint          width,
                     gint          height,
                     gboolean      dirty,
                     gboolean      shadow)
{
  g_return_if_fail (pr != NULL);
  g_return_if_fail (drawable != NULL);
  g_return_if_fail (x >= 0 && x + width  <= drawable->width);
  g_return_if_fail (y >= 0 && y + height <= drawable->height);

  pr->data      = NULL;
  pr->drawable  = drawable;
  pr->bpp       = drawable->bpp;
  pr->rowstride = 0;
  pr->x         = x;
  pr->y         = y;
  pr->w         = width;
  pr->h         = height;
  pr->dirty     = dirty;
  pr->shadow    = shadow;
}

/**
 * picman_pixel_rgn_resize:
 * @pr:      a pointer to a previously initialized #PicmanPixelRgn.
 * @x:       the x coordinate of the new position of the region's
 *           top-left corner.
 * @y:       the y coordinate of the new position of the region's
 *           top-left corner.
 * @width:   the new width of the region.
 * @height:  the new height of the region.
 *
 * Change the position and size of a previously initialized pixel region.
 **/
void
picman_pixel_rgn_resize (PicmanPixelRgn *pr,
                       gint          x,
                       gint          y,
                       gint          width,
                       gint          height)
{
  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (x >= 0 && x + width  <= pr->drawable->width);
  g_return_if_fail (y >= 0 && y + height <= pr->drawable->height);

  pr->x = x;
  pr->y = y;
  pr->w = width;
  pr->h = height;
}

/**
 * picman_pixel_rgn_get_pixel:
 * @pr:    a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:   a pointer to an array of #guchar
 * @x:     the x coordinate of the wanted pixel (relative to the drawable)
 * @y:     the y coordinate of the wanted pixel (relative to the drawable)
 *
 * Fill the buffer pointed by @buf with the value of the pixel at (@x, @y)
 * in the region @pr. @buf should be large enough to hold the pixel value
 * (1 #guchar for an indexed or grayscale drawable, 2 #guchar for
 * indexed with alpha or grayscale with alpha drawable, 3 #guchar for
 * rgb drawable and 4 #guchar for rgb with alpha drawable.
 **/
void
picman_pixel_rgn_get_pixel (PicmanPixelRgn *pr,
                          guchar       *buf,
                          gint          x,
                          gint          y)
{
  PicmanTile     *tile;
  const guchar *tile_data;
  gint          b;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (x >= 0 && x < pr->drawable->width);
  g_return_if_fail (y >= 0 && y < pr->drawable->height);

  tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
  picman_tile_ref (tile);

  tile_data = (tile->data +
               tile->bpp * (tile->ewidth * (y % TILE_HEIGHT) + (x % TILE_WIDTH)));

  for (b = 0; b < tile->bpp; b++)
    *buf++ = *tile_data++;

  picman_tile_unref (tile, FALSE);
}

/**
 * picman_pixel_rgn_get_row:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @width:  the number of pixels to get.
 *
 * Get several pixels of a region in a row. This function fills the buffer
 * @buf with the values of the pixels from (@x, @y) to (@x+@width-1, @y).
 * @buf should be large enough to hold all these values.
 **/
void
picman_pixel_rgn_get_row (PicmanPixelRgn *pr,
                        guchar       *buf,
                        gint          x,
                        gint          y,
                        gint          width)
{
  gint end;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x + width <= pr->drawable->width);
  g_return_if_fail (y >= 0 && y < pr->drawable->height);
  g_return_if_fail (width >= 0);

  end = x + width;

  while (x < end)
    {
      PicmanTile     *tile;
      const guchar *tile_data;
      gint          inc, min;
      gint          boundary;

      tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
      picman_tile_ref (tile);

      tile_data = (tile->data +
                   tile->bpp * (tile->ewidth * (y % TILE_HEIGHT) + (x % TILE_WIDTH)));

      boundary = x + (tile->ewidth - (x % TILE_WIDTH));

      min = MIN (end, boundary);
      inc = tile->bpp * (min - x);

      memcpy (buf, tile_data, inc);

      x = min;
      buf += inc;

      picman_tile_unref (tile, FALSE);
    }
}

/**
 * picman_pixel_rgn_get_col:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @height: the number of pixels to get.
 *
 * Get several pixels of a region's column. This function fills the buffer
 * @buf with the values of the pixels from (@x, @y) to (@x, @y+@height-1).
 * @buf should be large enough to hold all these values.
 *
 **/
void
picman_pixel_rgn_get_col (PicmanPixelRgn *pr,
                        guchar       *buf,
                        gint          x,
                        gint          y,
                        gint          height)
{
  gint end;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x < pr->drawable->width);
  g_return_if_fail (y >= 0 && y + height <= pr->drawable->height);
  g_return_if_fail (height >= 0);

  end = y + height;

  while (y < end)
    {
      PicmanTile     *tile;
      const guchar *tile_data;
      gint          inc;
      gint          boundary;
      gint          b;

      tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
      picman_tile_ref (tile);

      tile_data = (tile->data +
                   tile->bpp * (tile->ewidth * (y % TILE_HEIGHT) + (x % TILE_WIDTH)));

      boundary = y + (tile->eheight - (y % TILE_HEIGHT));
      inc = tile->bpp * tile->ewidth;

      for ( ; y < end && y < boundary; y++)
        {
          for (b = 0; b < tile->bpp; b++)
            *buf++ = tile_data[b];

          tile_data += inc;
        }

      picman_tile_unref (tile, FALSE);
    }
}

/**
 * picman_pixel_rgn_get_rect:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @width:  the width of the rectangle.
 * @height: the height of the rectangle.
 *
 * Get all the pixel values from the rectangle defined by @x, @y, @width and
 * @height. This function fills the buffer @buf with the values of the pixels
 * from (@x, @y) to (@x+@width-1, @y+@height-1).
 * @buf should be large enough to hold all these values (@width*@height*bpp).
 **/
void
picman_pixel_rgn_get_rect (PicmanPixelRgn *pr,
                         guchar       *buf,
                         gint          x,
                         gint          y,
                         gint          width,
                         gint          height)
{
  gulong  bufstride;
  gint    xstart, ystart;
  gint    xend, yend;
  gint    xboundary;
  gint    yboundary;
  gint    xstep, ystep;
  gint    ty, bpp;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x + width  <= pr->drawable->width);
  g_return_if_fail (y >= 0 && y + height <= pr->drawable->height);
  g_return_if_fail (width >= 0);
  g_return_if_fail (height >= 0);

  bpp = pr->bpp;
  bufstride = bpp * width;

  xstart = x;
  ystart = y;
  xend = x + width;
  yend = y + height;
  ystep = 0;

  while (y < yend)
    {
      x = xstart;

      while (x < xend)
        {
          PicmanTile *tile;

          tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
          picman_tile_ref (tile);

          xstep = tile->ewidth - (x % TILE_WIDTH);
          ystep = tile->eheight - (y % TILE_HEIGHT);
          xboundary = x + xstep;
          yboundary = y + ystep;
          xboundary = MIN (xboundary, xend);
          yboundary = MIN (yboundary, yend);

          for (ty = y; ty < yboundary; ty++)
            {
              const guchar *src;
              guchar       *dest;

              src = (tile->data +
                     tile->bpp * (tile->ewidth * (ty % TILE_HEIGHT) + (x % TILE_WIDTH)));
              dest = buf + bufstride * (ty - ystart) + bpp * (x - xstart);

              memcpy (dest, src, (xboundary - x) * bpp);
            }

          picman_tile_unref (tile, FALSE);
          x += xstep;
        }

      y += ystep;
    }
}

/**
 * picman_pixel_rgn_set_pixel:
 * @pr:   a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:  a pointer to an array of #guchar.
 * @x:    the x coordinate of the pixel (relative to the drawable).
 * @y:    the y coordinate of the pixel (relative to the drawable).
 *
 * Set the pixel at (@x, @y) to the values from @buf.
 **/
void
picman_pixel_rgn_set_pixel (PicmanPixelRgn *pr,
                          const guchar *buf,
                          gint          x,
                          gint          y)
{
  PicmanTile *tile;
  guchar   *tile_data;
  gint      b;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x < pr->drawable->width);
  g_return_if_fail (y >= 0 && y < pr->drawable->height);

  tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
  picman_tile_ref (tile);

  tile_data = tile->data + tile->bpp * (tile->ewidth *
                                        (y % TILE_HEIGHT) + (x % TILE_WIDTH));

  for (b = 0; b < tile->bpp; b++)
    *tile_data++ = *buf++;

  picman_tile_unref (tile, TRUE);
}

/**
 * picman_pixel_rgn_set_row:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @width:  the number of pixels to set.
 *
 * Set several pixels of a region in a row. This function draws the pixels
 * from (@x, @y) to (@x+@width-1, @y) using the values of the buffer @buf.
 * @buf should be large enough to hold all these values.
 **/
void
picman_pixel_rgn_set_row (PicmanPixelRgn *pr,
                        const guchar *buf,
                        gint          x,
                        gint          y,
                        gint          width)
{
  PicmanTile *tile;
  guchar   *tile_data;
  gint      inc, min;
  gint      end;
  gint      boundary;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x + width <= pr->drawable->width);
  g_return_if_fail (y >= 0 && y < pr->drawable->height);
  g_return_if_fail (width >= 0);

  end = x + width;

  while (x < end)
    {
      tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
      picman_tile_ref (tile);

      tile_data = (tile->data +
                   tile->bpp * (tile->ewidth * (y % TILE_HEIGHT) + (x % TILE_WIDTH)));

      boundary = x + (tile->ewidth - (x % TILE_WIDTH));

      min = MIN (end, boundary);
      inc = tile->bpp * (min - x);

      memcpy (tile_data, buf, inc);

      x = min;
      buf += inc;

      picman_tile_unref (tile, TRUE);
    }
}

/**
 * picman_pixel_rgn_set_col:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @height: the number of pixels to set.
 *
 * Set several pixels of a region's column. This function draws the pixels
 * from (@x, @y) to (@x, @y+@height-1) using the values from the buffer @buf.
 * @buf should be large enough to hold all these values.
 **/
void
picman_pixel_rgn_set_col (PicmanPixelRgn *pr,
                        const guchar *buf,
                        gint          x,
                        gint          y,
                        gint          height)
{
  gint      end;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x < pr->drawable->width);
  g_return_if_fail (y >= 0 && y + height <= pr->drawable->height);
  g_return_if_fail (height >= 0);

  end = y + height;

  while (y < end)
    {
      PicmanTile *tile;
      guchar   *tile_data;
      gint      inc;
      gint      boundary;

      tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
      picman_tile_ref (tile);

      tile_data = (tile->data +
                   tile->bpp * (tile->ewidth * (y % TILE_HEIGHT) + (x % TILE_WIDTH)));

      boundary = y + (tile->eheight - (y % TILE_HEIGHT));
      inc = tile->bpp * tile->ewidth;

      for ( ; y < end && y < boundary; y++)
        {
          gint b;

          for (b = 0; b < tile->bpp; b++)
            tile_data[b] = *buf++;

          tile_data += inc;
        }

      picman_tile_unref (tile, TRUE);
    }
}

/**
 * picman_pixel_rgn_set_rect:
 * @pr:     a pointer to a previously initialized #PicmanPixelRgn.
 * @buf:    a pointer to an array of #guchar
 * @x:      the x coordinate of the first pixel (relative to the drawable).
 * @y:      the y coordinate of the first pixel (relative to the drawable).
 * @width:  the width of the rectangle.
 * @height: the height of the rectangle.
 *
 * Set all the pixel of the rectangle defined by @x, @y, @width and
 * @height. This function draws the rectangle from (@x, @y) to
 * (@x+@width-1, @y+@height-1), using the pixel values from the buffer @buf.
 * @buf should be large enough to hold all these values (@width*@height*bpp).
 **/
void
picman_pixel_rgn_set_rect (PicmanPixelRgn *pr,
                         const guchar *buf,
                         gint          x,
                         gint          y,
                         gint          width,
                         gint          height)
{
  gulong  bufstride;
  gint    xstart, ystart;
  gint    xend, yend;
  gint    xboundary;
  gint    yboundary;
  gint    xstep, ystep;
  gint    ty, bpp;

  g_return_if_fail (pr != NULL && pr->drawable != NULL);
  g_return_if_fail (buf != NULL);
  g_return_if_fail (x >= 0 && x + width  <= pr->drawable->width);
  g_return_if_fail (y >= 0 && y + height <= pr->drawable->height);
  g_return_if_fail (width >= 0);
  g_return_if_fail (height >= 0);

  bpp = pr->bpp;
  bufstride = bpp * width;

  xstart = x;
  ystart = y;
  xend = x + width;
  yend = y + height;
  ystep = 0;

  while (y < yend)
    {
      x = xstart;

      while (x < xend)
        {
          PicmanTile *tile;

          tile = picman_drawable_get_tile2 (pr->drawable, pr->shadow, x, y);
          picman_tile_ref (tile);

          xstep = tile->ewidth - (x % TILE_WIDTH);
          ystep = tile->eheight - (y % TILE_HEIGHT);
          xboundary = x + xstep;
          yboundary = y + ystep;
          xboundary = MIN (xboundary, xend);
          yboundary = MIN (yboundary, yend);

          for (ty = y; ty < yboundary; ty++)
            {
              const guchar *src;
              guchar       *dest;

              src = buf + bufstride * (ty - ystart) + bpp * (x - xstart);
              dest = tile->data + tile->bpp * (tile->ewidth *
                                               (ty % TILE_HEIGHT) + (x % TILE_WIDTH));

              memcpy (dest, src, (xboundary - x) * bpp);
            }

          picman_tile_unref (tile, TRUE);
          x += xstep;
        }

      y += ystep;
    }
}

/**
 * picman_pixel_rgns_register2:
 * @nrgns: the number of regions to register.
 * @prs:   an array of @nrgns pointers to initialized #PicmanPixelRgn.
 *
 * It takes a number of initialized regions of the same size and provides a
 * pixel region iterator the iterator can be used to iterate over the
 * registered pixel regions.  While iterating the registered pixel regions will
 * cover subsets of the original pixel regions, chosen for optimized access to
 * the image data.
 *
 * Note that the given regions themselves are changed by this function, so
 * they are resized to the first subsets.
 *
 * This function has to be used together with picman_pixel_rgns_process in a loop.
 *
 * Returns: a #gpointer to a regions iterator.
 **/
gpointer
picman_pixel_rgns_register2 (gint           nrgns,
                           PicmanPixelRgn **prs)
{
  PicmanPixelRgnIterator *pri;
  gboolean              found;

  g_return_val_if_fail (nrgns > 0, NULL);
  g_return_val_if_fail (prs != NULL, NULL);

  pri = g_slice_new0 (PicmanPixelRgnIterator);

  found = FALSE;
  while (nrgns --)
    {
      PicmanPixelRgn       *pr  = prs[nrgns];
      PicmanPixelRgnHolder *prh = g_slice_new0 (PicmanPixelRgnHolder);

      prh->pr = pr;

      if (pr != NULL)
        {
          /*  If there is a defined value for data, make sure tiles is NULL  */
          if (pr->data)
            pr->drawable = NULL;

          prh->original_data     = pr->data;
          prh->startx            = pr->x;
          prh->starty            = pr->y;
          prh->pr->process_count = 0;

          if (! found)
            {
              found = TRUE;
              pri->region_width  = pr->w;
              pri->region_height = pr->h;
            }
        }

      /*  Add the pixel Rgn holder to the list  */
      pri->pixel_regions = g_slist_prepend (pri->pixel_regions, prh);
    }

  return picman_pixel_rgns_configure (pri);
}

/**
 * picman_pixel_rgns_register:
 * @nrgns: the number of regions to register.
 * @...:   @nrgns pointers to #PicmanPixelRgn.
 *
 * This is the varargs version of #picman_pixel_rgns_register2.
 *
 * Returns: a #gpointer to a regions iterator.
 **/
gpointer
picman_pixel_rgns_register (gint nrgns,
                          ...)
{
  PicmanPixelRgn **prs;
  gint           n;
  va_list        ap;

  g_return_val_if_fail (nrgns > 0, NULL);

  prs = g_newa (PicmanPixelRgn *, nrgns);

  va_start (ap, nrgns);

  for (n = nrgns; n--; )
    prs[n] = va_arg (ap, PicmanPixelRgn *);

  va_end (ap);

  return picman_pixel_rgns_register2 (nrgns, prs);
}

/**
 * picman_pixel_rgns_process:
 * @pri_ptr: a regions iterator returned by #picman_pixel_rgns_register,
 *           #picman_pixel_rgns_register2 or #picman_pixel_rgns_process.
 *
 * This function update the regions registered previously with one of the
 * #picman_pixel_rgns_register* functions to their next tile.
 *
 * Returns: a #gpointer to a new regions iterator or #NULL if there isn't
 * any tiles left.
 **/
gpointer
picman_pixel_rgns_process (gpointer pri_ptr)
{
  PicmanPixelRgnIterator *pri;
  GSList               *list;

  g_return_val_if_fail (pri_ptr != NULL, NULL);

  pri = (PicmanPixelRgnIterator*) pri_ptr;
  pri->process_count++;

  /*  Unref all referenced tiles and increment the offsets  */

  for (list = pri->pixel_regions; list; list = list->next)
    {
      PicmanPixelRgnHolder *prh = list->data;

      if ((prh->pr != NULL) && (prh->pr->process_count != pri->process_count))
        {
          /*  This eliminates the possibility of incrementing the
           *  same region twice
           */
          prh->pr->process_count++;

          /*  Unref the last referenced tile if the underlying region
           *  is a tile manager
           */
          if (prh->pr->drawable)
            {
              PicmanTile *tile = picman_drawable_get_tile2 (prh->pr->drawable,
                                                        prh->pr->shadow,
                                                        prh->pr->x,
                                                        prh->pr->y);
              picman_tile_unref (tile, prh->pr->dirty);
            }

          prh->pr->x += pri->portion_width;

          if ((prh->pr->x - prh->startx) >= pri->region_width)
            {
              prh->pr->x  = prh->startx;
              prh->pr->y += pri->portion_height;
            }
        }
    }

  return picman_pixel_rgns_configure (pri);
}


static gint
picman_get_portion_width (PicmanPixelRgnIterator *pri)
{
  GSList *list;
  gint    min_width = G_MAXINT;
  gint    width;

  /* Find the minimum width to the next vertical tile (in the case of
   * a tile manager) or to the end of the pixel region (in the case of
   * no tile manager)
   */

  for (list = pri->pixel_regions; list; list = list->next)
    {
      PicmanPixelRgnHolder *prh = list->data;

      if (prh->pr)
        {
          /* Check if we're past the point of no return  */
          if ((prh->pr->x - prh->startx) >= pri->region_width)
            return 0;

          if (prh->pr->drawable)
            {
              width = TILE_WIDTH - (prh->pr->x % TILE_WIDTH);
              width = CLAMP (width,
                             0,
                             (pri->region_width - (prh->pr->x - prh->startx)));
            }
          else
            {
              width = (pri->region_width - (prh->pr->x - prh->startx));
            }

          if (width < min_width)
            min_width = width;
        }
    }

  return min_width;
}

static gint
picman_get_portion_height (PicmanPixelRgnIterator *pri)
{
  GSList *list;
  gint    min_height = G_MAXINT;
  gint    height;

  /* Find the minimum height to the next vertical tile (in the case of
   * a tile manager) or to the end of the pixel region (in the case of
   * no tile manager)
   */

  for (list = pri->pixel_regions; list; list = list->next)
    {
      PicmanPixelRgnHolder *prh = list->data;

      if (prh->pr)
        {
          /* Check if we're past the point of no return  */
          if ((prh->pr->y - prh->starty) >= pri->region_height)
            return 0;

          if (prh->pr->drawable)
            {
              height = TILE_HEIGHT - (prh->pr->y % TILE_HEIGHT);
              height = CLAMP (height,
                              0,
                              (pri->region_height - (prh->pr->y - prh->starty)));
            }
          else
            {
              height = (pri->region_height - (prh->pr->y - prh->starty));
            }

          if (height < min_height)
            min_height = height;
        }
    }

  return min_height;
}

static gpointer
picman_pixel_rgns_configure (PicmanPixelRgnIterator *pri)
{
  GSList *list;

  /*  Determine the portion width and height  */
  pri->portion_width  = picman_get_portion_width (pri);
  pri->portion_height = picman_get_portion_height (pri);

  if (pri->portion_width  == 0 ||
      pri->portion_height == 0)
    {
      /*  free the pixel regions list  */
      for (list = pri->pixel_regions; list; list = list->next)
        g_slice_free (PicmanPixelRgnHolder, list->data);

      g_slist_free (pri->pixel_regions);
      g_slice_free (PicmanPixelRgnIterator, pri);

      return NULL;
    }

  pri->process_count++;

  for (list = pri->pixel_regions; list; list = list->next)
    {
      PicmanPixelRgnHolder *prh = list->data;

      if ((prh->pr != NULL) && (prh->pr->process_count != pri->process_count))
        {
          prh->pr->process_count++;
          picman_pixel_rgn_configure (prh, pri);
        }
    }

  return pri;
}

static void
picman_pixel_rgn_configure (PicmanPixelRgnHolder   *prh,
                          PicmanPixelRgnIterator *pri)
{
  /* Configure the rowstride and data pointer for the pixel region
   * based on the current offsets into the region and whether the
   * region is represented by a tile manager or not
   */
  if (prh->pr->drawable)
    {
      PicmanTile *tile;
      gint      offx;
      gint      offy;

      tile = picman_drawable_get_tile2 (prh->pr->drawable,
                                      prh->pr->shadow,
                                      prh->pr->x,
                                      prh->pr->y);
      picman_tile_ref (tile);

      offx = prh->pr->x % TILE_WIDTH;
      offy = prh->pr->y % TILE_HEIGHT;

      prh->pr->rowstride = tile->ewidth * prh->pr->bpp;
      prh->pr->data = (tile->data +
                       offy * prh->pr->rowstride + offx * prh->pr->bpp);
    }
  else
    {
      prh->pr->data = (prh->original_data +
                       prh->pr->y * prh->pr->rowstride +
                       prh->pr->x * prh->pr->bpp);
    }

  prh->pr->w = pri->portion_width;
  prh->pr->h = pri->portion_height;
}
