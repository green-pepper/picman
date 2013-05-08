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

#ifndef __PICMAN_CANVAS_H__
#define __PICMAN_CANVAS_H__


#include "widgets/picmanoverlaybox.h"


#define PICMAN_CANVAS_EVENT_MASK (GDK_EXPOSURE_MASK            | \
                                GDK_POINTER_MOTION_MASK      | \
                                GDK_BUTTON_PRESS_MASK        | \
                                GDK_BUTTON_RELEASE_MASK      | \
                                GDK_STRUCTURE_MASK           | \
                                GDK_ENTER_NOTIFY_MASK        | \
                                GDK_LEAVE_NOTIFY_MASK        | \
                                GDK_FOCUS_CHANGE_MASK        | \
                                GDK_KEY_PRESS_MASK           | \
                                GDK_KEY_RELEASE_MASK         | \
                                GDK_PROXIMITY_OUT_MASK)


#define PICMAN_TYPE_CANVAS            (picman_canvas_get_type ())
#define PICMAN_CANVAS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS, PicmanCanvas))
#define PICMAN_CANVAS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS, PicmanCanvasClass))
#define PICMAN_IS_CANVAS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS))
#define PICMAN_IS_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS))
#define PICMAN_CANVAS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS, PicmanCanvasClass))


typedef struct _PicmanCanvasClass PicmanCanvasClass;

struct _PicmanCanvas
{
  PicmanOverlayBox     parent_instance;

  PicmanDisplayConfig *config;
  PangoLayout       *layout;
};

struct _PicmanCanvasClass
{
  PicmanOverlayBoxClass  parent_class;
};


GType         picman_canvas_get_type     (void) G_GNUC_CONST;

GtkWidget   * picman_canvas_new          (PicmanDisplayConfig *config);

PangoLayout * picman_canvas_get_layout   (PicmanCanvas        *canvas,
                                        const gchar       *format,
                                        ...) G_GNUC_PRINTF (2, 3);

void          picman_canvas_set_bg_color (PicmanCanvas        *canvas,
                                        PicmanRGB           *color);


#endif /*  __PICMAN_CANVAS_H__  */
