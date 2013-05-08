/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_HISTOGRAM_VIEW_H__
#define __PICMAN_HISTOGRAM_VIEW_H__


#define PICMAN_TYPE_HISTOGRAM_VIEW            (picman_histogram_view_get_type ())
#define PICMAN_HISTOGRAM_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HISTOGRAM_VIEW, PicmanHistogramView))
#define PICMAN_HISTOGRAM_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HISTOGRAM_VIEW, PicmanHistogramViewClass))
#define PICMAN_IS_HISTOGRAM_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HISTOGRAM_VIEW))
#define PICMAN_IS_HISTOGRAM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HISTOGRAM_VIEW))
#define PICMAN_HISTOGRAM_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HISTOGRAM_VIEW, PicmanHistogramViewClass))


typedef struct _PicmanHistogramViewClass  PicmanHistogramViewClass;

struct _PicmanHistogramView
{
  GtkDrawingArea         parent_instance;

  PicmanHistogram         *histogram;
  PicmanHistogram         *bg_histogram;
  PicmanHistogramChannel   channel;
  PicmanHistogramScale     scale;
  gint                   start;
  gint                   end;

  gint                   border_width;
  gint                   subdivisions;
};

struct _PicmanHistogramViewClass
{
  GtkDrawingAreaClass  parent_class;

  void (* range_changed) (PicmanHistogramView *view,
                          gint               start,
                          gint               end);
};


GType           picman_histogram_view_get_type       (void) G_GNUC_CONST;

GtkWidget     * picman_histogram_view_new            (gboolean             range);

void            picman_histogram_view_set_histogram  (PicmanHistogramView   *view,
                                                    PicmanHistogram       *histogram);
PicmanHistogram * picman_histogram_view_get_histogram  (PicmanHistogramView   *view);

void            picman_histogram_view_set_background (PicmanHistogramView   *view,
                                                    PicmanHistogram       *histogram);
PicmanHistogram * picman_histogram_view_get_background (PicmanHistogramView   *view);

void            picman_histogram_view_set_channel    (PicmanHistogramView   *view,
                                                    PicmanHistogramChannel channel);
PicmanHistogramChannel
                picman_histogram_view_get_channel    (PicmanHistogramView   *view);

void            picman_histogram_view_set_scale      (PicmanHistogramView   *view,
                                                    PicmanHistogramScale   scale);
PicmanHistogramScale
                picman_histogram_view_get_scale      (PicmanHistogramView   *view);

void            picman_histogram_view_set_range      (PicmanHistogramView   *view,
                                                    gint                 start,
                                                    gint                 end);
void            picman_histogram_view_get_range      (PicmanHistogramView   *view,
                                                    gint                *start,
                                                    gint                *end);


#endif /* __PICMAN_HISTOGRAM_VIEW_H__ */
