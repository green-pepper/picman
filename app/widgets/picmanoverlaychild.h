/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoverlaychild.h
 * Copyright (C) 2009 Michael Natterer <mitch@picman.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PICMAN_OVERLAY_CHILD_H__
#define __PICMAN_OVERLAY_CHILD_H__


typedef struct _PicmanOverlayChild PicmanOverlayChild;

struct _PicmanOverlayChild
{
  GtkWidget      *widget;
  GdkWindow      *window;

  gboolean        has_position;
  gdouble         xalign;
  gdouble         yalign;
  gdouble         x;
  gdouble         y;

  gdouble         angle;
  gdouble         opacity;

  /* updated in size_allocate */
  cairo_matrix_t  matrix;
};


PicmanOverlayChild * picman_overlay_child_new           (PicmanOverlayBox  *box,
                                                     GtkWidget       *widget,
                                                     gdouble          xalign,
                                                     gdouble          yalign,
                                                     gdouble          angle,
                                                     gdouble          opacity);
void               picman_overlay_child_free          (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);

PicmanOverlayChild * picman_overlay_child_find          (PicmanOverlayBox   *box,
                                                     GtkWidget        *widget);

void               picman_overlay_child_realize       (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);
void               picman_overlay_child_unrealize     (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);
void               picman_overlay_child_size_request  (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);
void               picman_overlay_child_size_allocate (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);
gboolean           picman_overlay_child_expose        (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child,
                                                     GdkEventExpose   *event);
gboolean           picman_overlay_child_damage        (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child,
                                                     GdkEventExpose   *event);

void               picman_overlay_child_invalidate    (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child);
gboolean           picman_overlay_child_pick          (PicmanOverlayBox   *box,
                                                     PicmanOverlayChild *child,
                                                     gdouble           box_x,
                                                     gdouble           box_y);


#endif /* __PICMAN_OVERLAY_CHILD_H__ */
