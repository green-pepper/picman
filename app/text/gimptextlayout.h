/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_LAYOUT_H__
#define __PICMAN_TEXT_LAYOUT_H__


#define PICMAN_TYPE_TEXT_LAYOUT    (picman_text_layout_get_type ())
#define PICMAN_TEXT_LAYOUT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_LAYOUT, PicmanTextLayout))
#define PICMAN_IS_TEXT_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_LAYOUT))


typedef struct _PicmanTextLayoutClass PicmanTextLayoutClass;

struct _PicmanTextLayoutClass
{
  GObjectClass   parent_class;
};


GType            picman_text_layout_get_type             (void) G_GNUC_CONST;

PicmanTextLayout * picman_text_layout_new                  (PicmanText       *text,
                                                        gdouble         xres,
                                                        gdouble         yres);
gboolean         picman_text_layout_get_size             (PicmanTextLayout *layout,
                                                        gint           *width,
                                                        gint           *heigth);
void             picman_text_layout_get_offsets          (PicmanTextLayout *layout,
                                                        gint           *x,
                                                        gint           *y);
void             picman_text_layout_get_resolution       (PicmanTextLayout *layout,
                                                        gdouble        *xres,
                                                        gdouble        *yres);

PicmanText       * picman_text_layout_get_text             (PicmanTextLayout *layout);
PangoLayout    * picman_text_layout_get_pango_layout     (PicmanTextLayout *layout);

void             picman_text_layout_get_transform        (PicmanTextLayout *layout,
                                                        cairo_matrix_t *matrix);

void             picman_text_layout_transform_rect       (PicmanTextLayout *layout,
                                                        PangoRectangle *rect);
void             picman_text_layout_transform_point      (PicmanTextLayout *layout,
                                                        gdouble        *x,
                                                        gdouble        *y);
void             picman_text_layout_transform_distance   (PicmanTextLayout *layout,
                                                        gdouble        *x,
                                                        gdouble        *y);

void             picman_text_layout_untransform_rect     (PicmanTextLayout *layout,
                                                        PangoRectangle *rect);
void             picman_text_layout_untransform_point    (PicmanTextLayout *layout,
                                                        gdouble        *x,
                                                        gdouble        *y);
void             picman_text_layout_untransform_distance (PicmanTextLayout *layout,
                                                        gdouble        *x,
                                                        gdouble        *y);


#endif /* __PICMAN_TEXT_LAYOUT_H__ */
