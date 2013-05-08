/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanOverlayBox
 * Copyright (C) 2009 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OVERLAY_BOX_H__
#define __PICMAN_OVERLAY_BOX_H__


#define PICMAN_TYPE_OVERLAY_BOX            (picman_overlay_box_get_type ())
#define PICMAN_OVERLAY_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OVERLAY_BOX, PicmanOverlayBox))
#define PICMAN_OVERLAY_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_OVERLAY_BOX, PicmanOverlayBoxClass))
#define PICMAN_IS_OVERLAY_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OVERLAY_BOX))
#define PICMAN_IS_OVERLAY_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_OVERLAY_BOX))
#define PICMAN_OVERLAY_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_OVERLAY_BOX, PicmanOverlayBoxClass))


typedef struct _PicmanOverlayBoxClass PicmanOverlayBoxClass;

struct _PicmanOverlayBox
{
  GtkContainer  parent_instance;

  GList        *children;
};

struct _PicmanOverlayBoxClass
{
  GtkContainerClass  parent_class;
};


GType       picman_overlay_box_get_type            (void) G_GNUC_CONST;

GtkWidget * picman_overlay_box_new                 (void);

void        picman_overlay_box_add_child           (PicmanOverlayBox *box,
                                                  GtkWidget      *child,
                                                  gdouble         xalign,
                                                  gdouble         yalign);
void        picman_overlay_box_set_child_alignment (PicmanOverlayBox *box,
                                                  GtkWidget      *child,
                                                  gdouble         xalign,
                                                  gdouble         yalign);
void        picman_overlay_box_set_child_position  (PicmanOverlayBox *box,
                                                  GtkWidget      *child,
                                                  gdouble         x,
                                                  gdouble         y);
void        picman_overlay_box_set_child_angle     (PicmanOverlayBox *box,
                                                  GtkWidget      *child,
                                                  gdouble         angle);
void        picman_overlay_box_set_child_opacity   (PicmanOverlayBox *box,
                                                  GtkWidget      *child,
                                                  gdouble         opacity);

void        picman_overlay_box_scroll              (PicmanOverlayBox *box,
                                                  gint            offset_x,
                                                  gint            offset_y);


#endif /*  __PICMAN_OVERLAY_BOX_H__  */
