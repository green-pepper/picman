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

#ifndef __PICMAN_HISTOGRAM_BOX_H__
#define __PICMAN_HISTOGRAM_BOX_H__


#define PICMAN_TYPE_HISTOGRAM_BOX            (picman_histogram_box_get_type ())
#define PICMAN_HISTOGRAM_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HISTOGRAM_BOX, PicmanHistogramBox))
#define PICMAN_HISTOGRAM_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HISTOGRAM_BOX, PicmanHistogramBoxClass))
#define PICMAN_IS_HISTOGRAM_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HISTOGRAM_BOX))
#define PICMAN_IS_HISTOGRAM_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HISTOGRAM_BOX))
#define PICMAN_HISTOGRAM_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HISTOGRAM_BOX, PicmanHistogramBoxClass))


typedef struct _PicmanHistogramBoxClass PicmanHistogramBoxClass;

struct _PicmanHistogramBox
{
  GtkBox             parent_instance;

  PicmanHistogramView *view;
  GtkWidget         *color_bar;
  GtkWidget         *slider_bar;

  GtkAdjustment     *low_adj;
  GtkAdjustment     *high_adj;
};

struct _PicmanHistogramBoxClass
{
  GtkBoxClass  parent_class;
};


GType       picman_histogram_box_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_histogram_box_new         (void);
void        picman_histogram_box_set_channel (PicmanHistogramBox     *box,
                                            PicmanHistogramChannel  channel);


#endif  /*  __PICMAN_HISTOGRAM_BOX_H__  */
