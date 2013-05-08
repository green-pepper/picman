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

#ifndef __PICMAN_HANDLE_BAR_H__
#define __PICMAN_HANDLE_BAR_H__


#define PICMAN_TYPE_HANDLE_BAR            (picman_handle_bar_get_type ())
#define PICMAN_HANDLE_BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HANDLE_BAR, PicmanHandleBar))
#define PICMAN_HANDLE_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HANDLE_BAR, PicmanHandleBarClass))
#define PICMAN_IS_HANDLE_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HANDLE_BAR))
#define PICMAN_IS_HANDLE_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HANDLE_BAR))
#define PICMAN_HANDLE_BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HANDLE_BAR, PicmanHandleBarClass))


typedef struct _PicmanHandleBarClass  PicmanHandleBarClass;

struct _PicmanHandleBar
{
  GtkEventBox     parent_class;

  GtkOrientation  orientation;

  GtkAdjustment  *slider_adj[3];
  gdouble         lower;
  gdouble         upper;

  gint            slider_pos[3];
  gint            active_slider;
};

struct _PicmanHandleBarClass
{
  GtkEventBoxClass   parent_class;
};


GType       picman_handle_bar_get_type       (void) G_GNUC_CONST;

GtkWidget * picman_handle_bar_new            (GtkOrientation  orientation);

void        picman_handle_bar_set_adjustment (PicmanHandleBar  *bar,
                                            gint            handle_no,
                                            GtkAdjustment  *adjustment);


#endif  /*  __PICMAN_HANDLE_BAR_H__  */
