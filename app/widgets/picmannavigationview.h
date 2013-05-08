/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanNavigationView Widget
 * Copyright (C) 2001-2002 Michael Natterer <mitch@picman.org>
 *
 * partly based on app/nav_window
 * Copyright (C) 1999 Andy Thomas <alt@picman.org>
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

#ifndef __PICMAN_NAVIGATION_VIEW_H__
#define __PICMAN_NAVIGATION_VIEW_H__

#include "picmanview.h"


#define PICMAN_TYPE_NAVIGATION_VIEW            (picman_navigation_view_get_type ())
#define PICMAN_NAVIGATION_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_NAVIGATION_VIEW, PicmanNavigationView))
#define PICMAN_NAVIGATION_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_NAVIGATION_VIEW, PicmanNavigationViewClass))
#define PICMAN_IS_NAVIGATION_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_NAVIGATION_VIEW))
#define PICMAN_IS_NAVIGATION_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_NAVIGATION_VIEW))
#define PICMAN_NAVIGATION_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_NAVIGATION_VIEW, PicmanNavigationViewClass))


typedef struct _PicmanNavigationViewClass  PicmanNavigationViewClass;

struct _PicmanNavigationViewClass
{
  PicmanViewClass  parent_class;

  void (* marker_changed) (PicmanNavigationView *view,
                           gdouble             x,
                           gdouble             y);
  void (* zoom)           (PicmanNavigationView *view,
                           PicmanZoomType        direction);
  void (* scroll)         (PicmanNavigationView *view,
                           GdkScrollDirection  direction);
};


GType   picman_navigation_view_get_type     (void) G_GNUC_CONST;

void    picman_navigation_view_set_marker   (PicmanNavigationView *view,
                                           gdouble             x,
                                           gdouble             y,
                                           gdouble             width,
                                           gdouble             height);
void    picman_navigation_view_set_motion_offset
                                          (PicmanNavigationView *view,
                                           gint                motion_offset_x,
                                           gint                motion_offset_y);
void    picman_navigation_view_get_local_marker
                                          (PicmanNavigationView *view,
                                           gint               *x,
                                           gint               *y,
                                           gint               *width,
                                           gint               *height);
void    picman_navigation_view_grab_pointer (PicmanNavigationView *view);


#endif /* __PICMAN_NAVIGATION_VIEW_H__ */
