/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanhistogram module Copyright (C) 1999 Jay Cox <jaycox@picman.org>
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

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "gegl/picman-babl.h"

#include "picmanhistogram.h"


struct _PicmanHistogram
{
  gint     ref_count;
  gint     n_channels;
  gdouble *values;
};


/*  local function prototypes  */

static void   picman_histogram_alloc_values (PicmanHistogram *histogram,
                                           gint           bytes);


/*  public functions  */

PicmanHistogram *
picman_histogram_new (void)
{
  PicmanHistogram *histogram = g_slice_new0 (PicmanHistogram);

  histogram->ref_count = 1;

  return histogram;
}

PicmanHistogram *
picman_histogram_ref (PicmanHistogram *histogram)
{
  g_return_val_if_fail (histogram != NULL, NULL);

  histogram->ref_count++;

  return histogram;
}

void
picman_histogram_unref (PicmanHistogram *histogram)
{
  g_return_if_fail (histogram != NULL);

  histogram->ref_count--;

  if (histogram->ref_count == 0)
    {
      picman_histogram_clear_values (histogram);
      g_slice_free (PicmanHistogram, histogram);
    }
}

/**
 * picman_histogram_duplicate:
 * @histogram: a %PicmanHistogram
 *
 * Creates a duplicate of @histogram. The duplicate has a reference
 * count of 1 and contains the values from @histogram.
 *
 * Return value: a newly allocated %PicmanHistogram
 **/
PicmanHistogram *
picman_histogram_duplicate (PicmanHistogram *histogram)
{
  PicmanHistogram *dup;

  g_return_val_if_fail (histogram != NULL, NULL);

  dup = picman_histogram_new ();

  dup->n_channels = histogram->n_channels;
  dup->values     = g_memdup (histogram->values,
                              sizeof (gdouble) * dup->n_channels * 256);

  return dup;
}

void
picman_histogram_calculate (PicmanHistogram       *histogram,
                          GeglBuffer          *buffer,
                          const GeglRectangle *buffer_rect,
                          GeglBuffer          *mask,
                          const GeglRectangle *mask_rect)
{
  GeglBufferIterator *iter;
  const Babl         *format;
  gint                n_components;

  g_return_if_fail (histogram != NULL);
  g_return_if_fail (GEGL_IS_BUFFER (buffer));
  g_return_if_fail (buffer_rect != NULL);

  format = gegl_buffer_get_format (buffer);

  if (babl_format_is_palette (format))
    format = picman_babl_format (PICMAN_RGB, PICMAN_PRECISION_U8,
                               babl_format_has_alpha (format));
  else
    format = picman_babl_format (picman_babl_format_get_base_type (format),
                               PICMAN_PRECISION_U8,
                               babl_format_has_alpha (format));

  n_components = babl_format_get_n_components (format);

  picman_histogram_alloc_values (histogram, n_components);

  iter = gegl_buffer_iterator_new (buffer, buffer_rect, 0, format,
                                   GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

  if (mask)
    gegl_buffer_iterator_add (iter, mask, mask_rect, 0,
                              babl_format ("Y float"),
                              GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

#define VALUE(c,i) (histogram->values[(c) * 256 + (i)])

  while (gegl_buffer_iterator_next (iter))
    {
      const guchar *data = iter->data[0];
      gint          max;

      if (mask)
        {
          const gfloat *mask_data = iter->data[1];

          switch (n_components)
            {
            case 1:
              while (iter->length--)
                {
                  const gdouble masked = *mask_data;

                  VALUE (0, data[0]) += masked;

                  data += n_components;
                  mask_data += 1;
                }
              break;

            case 2:
              while (iter->length--)
                {
                  const gdouble masked = *mask_data;
                  const gdouble weight = data[1] / 255.0;

                  VALUE (0, data[0]) += weight * masked;
                  VALUE (1, data[1]) += masked;

                  data += n_components;
                  mask_data += 1;
                }
              break;

            case 3: /* calculate separate value values */
              while (iter->length--)
                {
                  const gdouble masked = *mask_data;

                  VALUE (1, data[0]) += masked;
                  VALUE (2, data[1]) += masked;
                  VALUE (3, data[2]) += masked;

                  max = MAX (data[0], data[1]);
                  max = MAX (data[2], max);

                  VALUE (0, max) += masked;

                  data += n_components;
                  mask_data += 1;
                }
              break;

            case 4: /* calculate separate value values */
              while (iter->length--)
                {
                  const gdouble masked = *mask_data;
                  const gdouble weight = data[3] / 255.0;

                  VALUE (1, data[0]) += weight * masked;
                  VALUE (2, data[1]) += weight * masked;
                  VALUE (3, data[2]) += weight * masked;
                  VALUE (4, data[3]) += masked;

                  max = MAX (data[0], data[1]);
                  max = MAX (data[2], max);

                  VALUE (0, max) += weight * masked;

                  data += n_components;
                  mask_data += 1;
                }
              break;
            }
        }
      else /* no mask */
        {
          switch (n_components)
            {
            case 1:
              while (iter->length--)
                {
                  VALUE (0, data[0]) += 1.0;

                  data += n_components;
                }
              break;

            case 2:
              while (iter->length--)
                {
                  const gdouble weight = data[1] / 255.0;

                  VALUE (0, data[0]) += weight;
                  VALUE (1, data[1]) += 1.0;

                  data += n_components;
                }
              break;

            case 3: /* calculate separate value values */
              while (iter->length--)
                {
                  VALUE (1, data[0]) += 1.0;
                  VALUE (2, data[1]) += 1.0;
                  VALUE (3, data[2]) += 1.0;

                  max = MAX (data[0], data[1]);
                  max = MAX (data[2], max);

                  VALUE (0, max) += 1.0;

                  data += n_components;
                }
              break;

            case 4: /* calculate separate value values */
              while (iter->length--)
                {
                  const gdouble weight = data[3] / 255.0;

                  VALUE (1, data[0]) += weight;
                  VALUE (2, data[1]) += weight;
                  VALUE (3, data[2]) += weight;
                  VALUE (4, data[3]) += 1.0;

                  max = MAX (data[0], data[1]);
                  max = MAX (data[2], max);

                  VALUE (0, max) += weight;

                  data += n_components;
                }
              break;
            }
        }
    }

#undef VALUE
}

void
picman_histogram_clear_values (PicmanHistogram *histogram)
{
  g_return_if_fail (histogram != NULL);

  if (histogram->values)
    {
      g_free (histogram->values);
      histogram->values = NULL;
    }

  histogram->n_channels = 0;
}


#define HISTOGRAM_VALUE(c,i) (histogram->values[(c) * 256 + (i)])


gdouble
picman_histogram_get_maximum (PicmanHistogram        *histogram,
                            PicmanHistogramChannel  channel)
{
  gdouble max = 0.0;
  gint    x;

  g_return_val_if_fail (histogram != NULL, 0.0);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0.0;

  if (channel == PICMAN_HISTOGRAM_RGB)
    for (x = 0; x < 256; x++)
      {
        max = MAX (max, HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED,   x));
        max = MAX (max, HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, x));
        max = MAX (max, HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE,  x));
      }
  else
    for (x = 0; x < 256; x++)
      {
        max = MAX (max, HISTOGRAM_VALUE (channel, x));
      }

  return max;
}

gdouble
picman_histogram_get_value (PicmanHistogram        *histogram,
                          PicmanHistogramChannel  channel,
                          gint                  bin)
{
  g_return_val_if_fail (histogram != NULL, 0.0);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      bin < 0 || bin >= 256 ||
      (channel == PICMAN_HISTOGRAM_RGB && histogram->n_channels < 4) ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0.0;

  if (channel == PICMAN_HISTOGRAM_RGB)
    {
      gdouble min = HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED, bin);

      min = MIN (min, HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, bin));

      return MIN (min, HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE, bin));
    }
  else
    {
      return HISTOGRAM_VALUE (channel, bin);
    }
}

gdouble
picman_histogram_get_channel (PicmanHistogram        *histogram,
                            PicmanHistogramChannel  channel,
                            gint                  bin)
{
  g_return_val_if_fail (histogram != NULL, 0.0);

  if (histogram->n_channels > 3)
    channel++;

  return picman_histogram_get_value (histogram, channel, bin);
}

gint
picman_histogram_n_channels (PicmanHistogram *histogram)
{
  g_return_val_if_fail (histogram != NULL, 0);

  return histogram->n_channels - 1;
}

gdouble
picman_histogram_get_count (PicmanHistogram        *histogram,
                          PicmanHistogramChannel  channel,
                          gint                  start,
                          gint                  end)
{
  gint    i;
  gdouble count = 0.0;

  g_return_val_if_fail (histogram != NULL, 0.0);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (channel == PICMAN_HISTOGRAM_RGB)
    return (picman_histogram_get_count (histogram,
                                      PICMAN_HISTOGRAM_RED, start, end)   +
            picman_histogram_get_count (histogram,
                                      PICMAN_HISTOGRAM_GREEN, start, end) +
            picman_histogram_get_count (histogram,
                                      PICMAN_HISTOGRAM_BLUE, start, end));

  if (! histogram->values ||
      start > end ||
      channel >= histogram->n_channels)
    return 0.0;

  start = CLAMP (start, 0, 255);
  end   = CLAMP (end, 0, 255);

  for (i = start; i <= end; i++)
    count += HISTOGRAM_VALUE (channel, i);

  return count;
}

gdouble
picman_histogram_get_mean (PicmanHistogram        *histogram,
                         PicmanHistogramChannel  channel,
                         gint                  start,
                         gint                  end)
{
  gint    i;
  gdouble mean = 0.0;
  gdouble count;

  g_return_val_if_fail (histogram != NULL, 0.0);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      start > end ||
      (channel == PICMAN_HISTOGRAM_RGB && histogram->n_channels < 4) ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0.0;

  start = CLAMP (start, 0, 255);
  end = CLAMP (end, 0, 255);

  if (channel == PICMAN_HISTOGRAM_RGB)
    {
      for (i = start; i <= end; i++)
        mean += (i * HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED,   i) +
                 i * HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, i) +
                 i * HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE,  i));
    }
  else
    {
      for (i = start; i <= end; i++)
        mean += i * HISTOGRAM_VALUE (channel, i);
    }

  count = picman_histogram_get_count (histogram, channel, start, end);

  if (count > 0.0)
    return mean / count;

  return mean;
}

gint
picman_histogram_get_median (PicmanHistogram         *histogram,
                           PicmanHistogramChannel   channel,
                           gint                   start,
                           gint                   end)
{
  gint    i;
  gdouble sum = 0.0;
  gdouble count;

  g_return_val_if_fail (histogram != NULL, -1);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      start > end ||
      (channel == PICMAN_HISTOGRAM_RGB && histogram->n_channels < 4) ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0;

  start = CLAMP (start, 0, 255);
  end = CLAMP (end, 0, 255);

  count = picman_histogram_get_count (histogram, channel, start, end);

  if (channel == PICMAN_HISTOGRAM_RGB)
    for (i = start; i <= end; i++)
      {
        sum += (HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED,   i) +
                HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, i) +
                HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE,  i));

        if (sum * 2 > count)
          return i;
      }
  else
    for (i = start; i <= end; i++)
      {
        sum += HISTOGRAM_VALUE (channel, i);

        if (sum * 2 > count)
          return i;
      }

  return -1;
}

/*
 * adapted from GNU ocrad 0.14 : page_image_io.cc : otsu_th
 *
 *  N. Otsu, "A threshold selection method from gray-level histograms,"
 *  IEEE Trans. Systems, Man, and Cybernetics, vol. 9, no. 1, pp. 62-66, 1979.
 */
gdouble
picman_histogram_get_threshold (PicmanHistogram        *histogram,
                              PicmanHistogramChannel  channel,
                              gint                  start,
                              gint                  end)
{
  gint     i;
  gint     maxval;
  gdouble *hist      = NULL;
  gdouble *chist     = NULL;
  gdouble *cmom      = NULL;
  gdouble  hist_max  = 0.0;
  gdouble  chist_max = 0.0;
  gdouble  cmom_max  = 0.0;
  gdouble  bvar_max  = 0.0;
  gint     threshold = 127;

  g_return_val_if_fail (histogram != NULL, -1);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      start > end ||
      (channel == PICMAN_HISTOGRAM_RGB && histogram->n_channels < 4) ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0;

  start = CLAMP (start, 0, 255);
  end = CLAMP (end, 0, 255);

  maxval = end - start;

  hist  = g_newa (gdouble, maxval + 1);
  chist = g_newa (gdouble, maxval + 1);
  cmom  = g_newa (gdouble, maxval + 1);

  if (channel == PICMAN_HISTOGRAM_RGB)
    {
      for (i = start; i <= end; i++)
        hist[i - start] = (HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED,   i) +
                           HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, i) +
                           HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE,  i));
    }
  else
    {
      for (i = start; i <= end; i++)
        hist[i - start] = HISTOGRAM_VALUE (channel, i);
    }

  hist_max = hist[0];
  chist[0] = hist[0];
  cmom[0] = 0;

  for (i = 1; i <= maxval; i++)
    {
      if (hist[i] > hist_max)
	hist_max = hist[i];

      chist[i] = chist[i-1] + hist[i];
      cmom[i] = cmom[i-1] + i * hist[i];
    }

  chist_max = chist[maxval];
  cmom_max = cmom[maxval];
  bvar_max = 0;

  for (i = 0; i < maxval; ++i)
    if (chist[i] > 0 && chist[i] < chist_max)
      {
	gdouble bvar;

	bvar = (gdouble) cmom[i] / chist[i];
	bvar -= (cmom_max - cmom[i]) / (chist_max - chist[i]);
	bvar *= bvar;
	bvar *= chist[i];
	bvar *= chist_max - chist[i];

	if (bvar > bvar_max)
	  {
	    bvar_max = bvar;
	    threshold = start + i;
	  }
      }

  return threshold;
}

gdouble
picman_histogram_get_std_dev (PicmanHistogram        *histogram,
                            PicmanHistogramChannel  channel,
                            gint                  start,
                            gint                  end)
{
  gint    i;
  gdouble dev = 0.0;
  gdouble count;
  gdouble mean;

  g_return_val_if_fail (histogram != NULL, 0.0);

  /*  the gray alpha channel is in slot 1  */
  if (histogram->n_channels == 3 && channel == PICMAN_HISTOGRAM_ALPHA)
    channel = 1;

  if (! histogram->values ||
      start > end ||
      (channel == PICMAN_HISTOGRAM_RGB && histogram->n_channels < 4) ||
      (channel != PICMAN_HISTOGRAM_RGB && channel >= histogram->n_channels))
    return 0.0;

  mean  = picman_histogram_get_mean  (histogram, channel, start, end);
  count = picman_histogram_get_count (histogram, channel, start, end);

  if (count == 0.0)
    count = 1.0;

  for (i = start; i <= end; i++)
    {
      gdouble value;

      if (channel == PICMAN_HISTOGRAM_RGB)
        {
          value = (HISTOGRAM_VALUE (PICMAN_HISTOGRAM_RED,   i) +
                   HISTOGRAM_VALUE (PICMAN_HISTOGRAM_GREEN, i) +
                   HISTOGRAM_VALUE (PICMAN_HISTOGRAM_BLUE,  i));
        }
      else
        {
          value = picman_histogram_get_value (histogram, channel, i);
        }

      dev += value * SQR (i - mean);
    }

  return sqrt (dev / count);
}


/*  private functions  */

static void
picman_histogram_alloc_values (PicmanHistogram *histogram,
                             gint           bytes)
{
  if (bytes + 1 != histogram->n_channels)
    {
      picman_histogram_clear_values (histogram);

      histogram->n_channels = bytes + 1;

      histogram->values = g_new0 (gdouble, histogram->n_channels * 256);
    }
  else
    {
      memset (histogram->values,
              0, histogram->n_channels * 256 * sizeof (gdouble));
    }
}
