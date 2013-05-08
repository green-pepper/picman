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

#ifndef __PICMAN_COLOR_FRAME_H__
#define __PICMAN_COLOR_FRAME_H__


#define PICMAN_COLOR_FRAME_ROWS 5


#define PICMAN_TYPE_COLOR_FRAME            (picman_color_frame_get_type ())
#define PICMAN_COLOR_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_FRAME, PicmanColorFrame))
#define PICMAN_COLOR_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_FRAME, PicmanColorFrameClass))
#define PICMAN_IS_COLOR_FRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_FRAME))
#define PICMAN_IS_COLOR_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_FRAME))
#define PICMAN_COLOR_FRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_FRAME, PicmanColorFrameClass))


typedef struct _PicmanColorFrameClass PicmanColorFrameClass;

struct _PicmanColorFrame
{
  PicmanFrame           parent_instance;

  gboolean            sample_valid;
  const Babl         *sample_format;
  PicmanRGB             color;
  gint                color_index;

  PicmanColorFrameMode  frame_mode;

  gboolean            has_number;
  gint                number;

  gboolean            has_color_area;

  GtkWidget          *menu;
  GtkWidget          *color_area;
  GtkWidget          *name_labels[PICMAN_COLOR_FRAME_ROWS];
  GtkWidget          *value_labels[PICMAN_COLOR_FRAME_ROWS];

  PangoLayout        *number_layout;
};

struct _PicmanColorFrameClass
{
  PicmanFrameClass      parent_class;
};


GType       picman_color_frame_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_color_frame_new         (void);

void        picman_color_frame_set_mode           (PicmanColorFrame     *frame,
                                                 PicmanColorFrameMode  mode);
void        picman_color_frame_set_has_number     (PicmanColorFrame     *frame,
                                                 gboolean            has_number);
void        picman_color_frame_set_number         (PicmanColorFrame     *frame,
                                                 gint                number);


void        picman_color_frame_set_has_color_area (PicmanColorFrame     *frame,
                                                 gboolean            has_color_area);

void        picman_color_frame_set_color          (PicmanColorFrame     *frame,
                                                 const Babl         *format,
                                                 const PicmanRGB      *color,
                                                 gint                color_index);
void        picman_color_frame_set_invalid        (PicmanColorFrame     *frame);


#endif  /*  __PICMAN_COLOR_FRAME_H__  */
