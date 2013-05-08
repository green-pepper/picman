/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanregioniterator.c
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

#include <glib.h>

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"
#include "picmanregioniterator.h"


/**
 * SECTION: picmanregioniterator
 * @title: picmanregioniterator
 * @short_description: Functions to traverse a pixel regions.
 *
 * The PicmanRgnIterator functions provide a variety of common ways to
 * traverse a PixelRegion, using a pre-defined function pointer per
 * pixel.
 **/


struct _PicmanRgnIterator
{
  PicmanDrawable *drawable;
  gint          x1;
  gint          y1;
  gint          x2;
  gint          y2;
};


static void  picman_rgn_iterator_iter_single (PicmanRgnIterator    *iter,
                                            PicmanPixelRgn       *srcPR,
                                            PicmanRgnFuncSrc      func,
                                            gpointer            data);
static void  picman_rgn_render_row           (const guchar       *src,
                                            guchar             *dest,
                                            gint                col,
                                            gint                bpp,
                                            PicmanRgnFunc2        func,
                                            gpointer            data);
static void  picman_rgn_render_region        (const PicmanPixelRgn *srcPR,
                                            const PicmanPixelRgn *destPR,
                                            PicmanRgnFunc2        func,
                                            gpointer            data);


/**
 * picman_rgn_iterator_new:
 * @drawable: a #PicmanDrawable
 * @unused:   ignored
 *
 * Creates a new #PicmanRgnIterator for @drawable. The #PicmanRunMode
 * parameter is ignored. Use picman_rgn_iterator_free() to free this
 * iterator.
 *
 * Return value: a newly allocated #PicmanRgnIterator.
 **/
PicmanRgnIterator *
picman_rgn_iterator_new (PicmanDrawable *drawable,
                       PicmanRunMode   unused)
{
  PicmanRgnIterator *iter;

  g_return_val_if_fail (drawable != NULL, NULL);

  iter = g_slice_new (PicmanRgnIterator);

  iter->drawable = drawable;

  picman_drawable_mask_bounds (drawable->drawable_id,
                             &iter->x1, &iter->y1,
                             &iter->x2, &iter->y2);

  return iter;
}

/**
 * picman_rgn_iterator_free:
 * @iter: a #PicmanRgnIterator
 *
 * Frees the resources allocated for @iter.
 **/
void
picman_rgn_iterator_free (PicmanRgnIterator *iter)
{
  g_return_if_fail (iter != NULL);

  g_slice_free (PicmanRgnIterator, iter);
}

void
picman_rgn_iterator_src (PicmanRgnIterator *iter,
                       PicmanRgnFuncSrc   func,
                       gpointer         data)
{
  PicmanPixelRgn srcPR;

  g_return_if_fail (iter != NULL);

  picman_pixel_rgn_init (&srcPR, iter->drawable,
                       iter->x1, iter->y1,
                       iter->x2 - iter->x1, iter->y2 - iter->y1,
                       FALSE, FALSE);
  picman_rgn_iterator_iter_single (iter, &srcPR, func, data);
}

void
picman_rgn_iterator_src_dest (PicmanRgnIterator    *iter,
                            PicmanRgnFuncSrcDest  func,
                            gpointer            data)
{
  PicmanPixelRgn  srcPR, destPR;
  gint          x1, y1, x2, y2;
  gint          bpp;
  gint          count;
  gpointer      pr;
  gint          total_area;
  gint          area_so_far;

  g_return_if_fail (iter != NULL);

  x1 = iter->x1;
  y1 = iter->y1;
  x2 = iter->x2;
  y2 = iter->y2;

  total_area  = (x2 - x1) * (y2 - y1);
  area_so_far = 0;

  picman_pixel_rgn_init (&srcPR, iter->drawable, x1, y1, x2 - x1, y2 - y1,
                       FALSE, FALSE);
  picman_pixel_rgn_init (&destPR, iter->drawable, x1, y1, x2 - x1, y2 - y1,
                       TRUE, TRUE);

  bpp = srcPR.bpp;

  for (pr = picman_pixel_rgns_register (2, &srcPR, &destPR), count = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), count++)
    {
      const guchar *src  = srcPR.data;
      guchar       *dest = destPR.data;
      gint          y;

      for (y = srcPR.y; y < srcPR.y + srcPR.h; y++)
        {
          const guchar *s = src;
          guchar       *d = dest;
          gint          x;

          for (x = srcPR.x; x < srcPR.x + srcPR.w; x++)
            {
              func (x, y, s, d, bpp, data);

              s += bpp;
              d += bpp;
            }

          src  += srcPR.rowstride;
          dest += destPR.rowstride;
        }

      area_so_far += srcPR.w * srcPR.h;

      if ((count % 16) == 0)
        picman_progress_update ((gdouble) area_so_far / (gdouble) total_area);
    }

  picman_drawable_flush (iter->drawable);
  picman_drawable_merge_shadow (iter->drawable->drawable_id, TRUE);
  picman_drawable_update (iter->drawable->drawable_id,
                        x1, y1, x2 - x1, y2 - y1);
}

void
picman_rgn_iterator_dest (PicmanRgnIterator *iter,
                        PicmanRgnFuncDest  func,
                        gpointer         data)
{
  PicmanPixelRgn destPR;

  g_return_if_fail (iter != NULL);

  picman_pixel_rgn_init (&destPR, iter->drawable,
                       iter->x1, iter->y1,
                       iter->x2 - iter->x1, iter->y2 - iter->y1,
                       TRUE, TRUE);
  picman_rgn_iterator_iter_single (iter, &destPR, (PicmanRgnFuncSrc) func, data);

  /*  update the processed region  */
  picman_drawable_flush (iter->drawable);
  picman_drawable_merge_shadow (iter->drawable->drawable_id, TRUE);
  picman_drawable_update (iter->drawable->drawable_id,
                        iter->x1, iter->y1,
                        iter->x2 - iter->x1, iter->y2 - iter->y1);
}


void
picman_rgn_iterate1 (PicmanDrawable *drawable,
                   PicmanRunMode   unused,
                   PicmanRgnFunc1  func,
                   gpointer      data)
{
  PicmanPixelRgn  srcPR;
  gint          x1, y1, x2, y2;
  gpointer      pr;
  gint          total_area;
  gint          area_so_far;
  gint          count;

  g_return_if_fail (drawable != NULL);

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  total_area  = (x2 - x1) * (y2 - y1);
  area_so_far = 0;

  if (total_area <= 0)
    return;

  picman_pixel_rgn_init (&srcPR, drawable,
                       x1, y1, (x2 - x1), (y2 - y1), FALSE, FALSE);

  for (pr = picman_pixel_rgns_register (1, &srcPR), count = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), count++)
    {
      const guchar *src = srcPR.data;
      gint          y;

      for (y = 0; y < srcPR.h; y++)
        {
          const guchar *s = src;
          gint          x;

          for (x = 0; x < srcPR.w; x++)
            {
              func (s, srcPR.bpp, data);
              s += srcPR.bpp;
            }

          src += srcPR.rowstride;
        }

      area_so_far += srcPR.w * srcPR.h;

      if ((count % 16) == 0)
        picman_progress_update ((gdouble) area_so_far / (gdouble) total_area);
    }
}

void
picman_rgn_iterate2 (PicmanDrawable *drawable,
                   PicmanRunMode   unused,
                   PicmanRgnFunc2  func,
                   gpointer      data)
{
  PicmanPixelRgn  srcPR, destPR;
  gint          x1, y1, x2, y2;
  gpointer      pr;
  gint          total_area;
  gint          area_so_far;
  gint          count;

  g_return_if_fail (drawable != NULL);

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  total_area  = (x2 - x1) * (y2 - y1);
  area_so_far = 0;

  if (total_area <= 0)
    return;

  /* Initialize the pixel regions. */
  picman_pixel_rgn_init (&srcPR, drawable, x1, y1, (x2 - x1), (y2 - y1),
                       FALSE, FALSE);
  picman_pixel_rgn_init (&destPR, drawable, x1, y1, (x2 - x1), (y2 - y1),
                       TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (2, &srcPR, &destPR), count = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), count++)
    {
      picman_rgn_render_region (&srcPR, &destPR, func, data);

      area_so_far += srcPR.w * srcPR.h;

      if ((count % 16) == 0)
        picman_progress_update ((gdouble) area_so_far / (gdouble) total_area);
    }

  /*  update the processed region  */
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, (x2 - x1), (y2 - y1));
}

static void
picman_rgn_iterator_iter_single (PicmanRgnIterator *iter,
                               PicmanPixelRgn    *srcPR,
                               PicmanRgnFuncSrc   func,
                               gpointer         data)
{
  gpointer  pr;
  gint      total_area;
  gint      area_so_far;
  gint      count;

  total_area  = (iter->x2 - iter->x1) * (iter->y2 - iter->y1);
  area_so_far = 0;

  for (pr = picman_pixel_rgns_register (1, srcPR), count = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), count++)
    {
      const guchar *src = srcPR->data;
      gint          y;

      for (y = srcPR->y; y < srcPR->y + srcPR->h; y++)
        {
          const guchar *s = src;
          gint          x;

          for (x = srcPR->x; x < srcPR->x + srcPR->w; x++)
            {
              func (x, y, s, srcPR->bpp, data);
              s += srcPR->bpp;
            }

          src += srcPR->rowstride;
        }

      area_so_far += srcPR->w * srcPR->h;

      if ((count % 16) == 0)
        picman_progress_update ((gdouble) area_so_far / (gdouble) total_area);
    }
}

static void
picman_rgn_render_row (const guchar *src,
                     guchar       *dest,
                     gint          col,    /* row width in pixels */
                     gint          bpp,
                     PicmanRgnFunc2  func,
                     gpointer      data)
{
  while (col--)
    {
      func (src, dest, bpp, data);

      src  += bpp;
      dest += bpp;
    }
}

static void
picman_rgn_render_region (const PicmanPixelRgn *srcPR,
                        const PicmanPixelRgn *destPR,
                        PicmanRgnFunc2        func,
                        gpointer            data)
{
  const guchar *src  = srcPR->data;
  guchar       *dest = destPR->data;
  gint          row;

  for (row = 0; row < srcPR->h; row++)
    {
      picman_rgn_render_row (src, dest, srcPR->w, srcPR->bpp, func, data);

      src  += srcPR->rowstride;
      dest += destPR->rowstride;
    }
}
