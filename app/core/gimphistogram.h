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

#ifndef __PICMAN_HISTOGRAM_H__
#define __PICMAN_HISTOGRAM_H__


PicmanHistogram * picman_histogram_new           (void);

PicmanHistogram * picman_histogram_ref           (PicmanHistogram        *histogram);
void            picman_histogram_unref         (PicmanHistogram        *histogram);

PicmanHistogram * picman_histogram_duplicate     (PicmanHistogram        *histogram);

void            picman_histogram_calculate     (PicmanHistogram        *histogram,
                                              GeglBuffer           *buffer,
                                              const GeglRectangle  *buffer_rect,
                                              GeglBuffer           *mask,
                                              const GeglRectangle  *mask_rect);

void            picman_histogram_clear_values  (PicmanHistogram        *histogram);

gdouble         picman_histogram_get_maximum   (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel);
gdouble         picman_histogram_get_count     (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  start,
                                              gint                  end);
gdouble         picman_histogram_get_mean      (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  start,
                                              gint                  end);
gint            picman_histogram_get_median    (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  start,
                                              gint                  end);
gdouble         picman_histogram_get_std_dev   (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  start,
                                              gint                  end);
gdouble         picman_histogram_get_threshold (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  start,
                                              gint                  end);
gdouble         picman_histogram_get_value     (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  bin);
gdouble         picman_histogram_get_channel   (PicmanHistogram        *histogram,
                                              PicmanHistogramChannel  channel,
                                              gint                  bin);
gint            picman_histogram_n_channels    (PicmanHistogram        *histogram);


#endif /* __PICMAN_HISTOGRAM_H__ */
