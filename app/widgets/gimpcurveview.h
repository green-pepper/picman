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

#ifndef __PICMAN_CURVE_VIEW_H__
#define __PICMAN_CURVE_VIEW_H__


#include "picmanhistogramview.h"


#define PICMAN_TYPE_CURVE_VIEW            (picman_curve_view_get_type ())
#define PICMAN_CURVE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CURVE_VIEW, PicmanCurveView))
#define PICMAN_CURVE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CURVE_VIEW, PicmanCurveViewClass))
#define PICMAN_IS_CURVE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CURVE_VIEW))
#define PICMAN_IS_CURVE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CURVE_VIEW))
#define PICMAN_CURVE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CURVE_VIEW, PicmanCurveViewClass))


typedef struct _PicmanCurveViewClass  PicmanCurveViewClass;

struct _PicmanCurveView
{
  PicmanHistogramView  parent_instance;

  Picman              *picman; /* only needed for copy & paste */

  PicmanCurve         *curve;
  PicmanRGB           *curve_color;

  GList             *bg_curves;

  gboolean           draw_base_line;
  gint               grid_rows;
  gint               grid_columns;

  gint               selected;
  gdouble            last_x;
  gdouble            last_y;
  gdouble            leftmost;
  gdouble            rightmost;
  gboolean           grabbed;

  GdkCursorType      cursor_type;

  gdouble            xpos;

  PangoLayout       *layout;

  gdouble            range_x_min;
  gdouble            range_x_max;
  gdouble            range_y_min;
  gdouble            range_y_max;

  gdouble            cursor_x;
  gdouble            cursor_y;
  PangoLayout       *cursor_layout;
  PangoRectangle     cursor_rect;

  gchar             *x_axis_label;
  gchar             *y_axis_label;
};

struct _PicmanCurveViewClass
{
  PicmanHistogramViewClass  parent_class;

  void (* cut_clipboard)   (PicmanCurveView *view);
  void (* copy_clipboard)  (PicmanCurveView *view);
  void (* paste_clipboard) (PicmanCurveView *view);
};


GType       picman_curve_view_get_type          (void) G_GNUC_CONST;

GtkWidget * picman_curve_view_new               (void);

void        picman_curve_view_set_curve         (PicmanCurveView *view,
                                               PicmanCurve     *curve,
                                               const PicmanRGB *color);
PicmanCurve * picman_curve_view_get_curve         (PicmanCurveView *view);

void        picman_curve_view_add_background    (PicmanCurveView *view,
                                               PicmanCurve     *curve,
                                               const PicmanRGB *color);
void        picman_curve_view_remove_background (PicmanCurveView *view,
                                               PicmanCurve     *curve);

void   picman_curve_view_remove_all_backgrounds (PicmanCurveView *view);

void        picman_curve_view_set_selected      (PicmanCurveView *view,
                                               gint           selected);
void        picman_curve_view_set_range_x       (PicmanCurveView *view,
                                               gdouble        min,
                                               gdouble        max);
void        picman_curve_view_set_range_y       (PicmanCurveView *view,
                                               gdouble        min,
                                               gdouble        max);
void        picman_curve_view_set_xpos          (PicmanCurveView *view,
                                               gdouble        x);

void        picman_curve_view_set_x_axis_label  (PicmanCurveView *view,
                                               const gchar   *label);
void        picman_curve_view_set_y_axis_label  (PicmanCurveView *view,
                                               const gchar   *label);


#endif /* __PICMAN_CURVE_VIEW_H__ */
