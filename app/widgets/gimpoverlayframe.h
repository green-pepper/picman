/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoverlayframe.h
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OVERLAY_FRAME_H__
#define __PICMAN_OVERLAY_FRAME_H__


#define PICMAN_TYPE_OVERLAY_FRAME            (picman_overlay_frame_get_type ())
#define PICMAN_OVERLAY_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OVERLAY_FRAME, PicmanOverlayFrame))
#define PICMAN_OVERLAY_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_OVERLAY_FRAME, PicmanOverlayFrameClass))
#define PICMAN_IS_OVERLAY_FRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OVERLAY_FRAME))
#define PICMAN_IS_OVERLAY_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_OVERLAY_FRAME))
#define PICMAN_OVERLAY_FRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_OVERLAY_FRAME, PicmanOverlayFrameClass))


typedef struct _PicmanOverlayFrame      PicmanOverlayFrame;
typedef struct _PicmanOverlayFrameClass PicmanOverlayFrameClass;

struct _PicmanOverlayFrame
{
  GtkBin  parent_instance;
};

struct _PicmanOverlayFrameClass
{
  GtkBinClass  parent_class;
};


GType       picman_overlay_frame_get_type (void) G_GNUC_CONST;

GtkWidget * picman_overlay_frame_new      (void);


#endif /* __PICMAN_OVERLAY_FRAME_H__ */
