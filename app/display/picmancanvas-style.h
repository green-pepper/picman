/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandisplayshell-style.h
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

#ifndef __PICMAN_CANVAS_STYLE_H__
#define __PICMAN_CANVAS_STYLE_H__


void   picman_canvas_set_guide_style         (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gboolean       active);
void   picman_canvas_set_sample_point_style  (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gboolean       active);
void   picman_canvas_set_grid_style          (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            PicmanGrid      *grid);
void   picman_canvas_set_pen_style           (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            const PicmanRGB *color,
                                            gint           width);
void   picman_canvas_set_layer_style         (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            PicmanLayer     *layer);
void   picman_canvas_set_selection_out_style (GtkWidget     *canvas,
                                            cairo_t       *cr);
void   picman_canvas_set_selection_in_style  (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gint           index);
void   picman_canvas_set_vectors_bg_style    (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gboolean       active);
void   picman_canvas_set_vectors_fg_style    (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gboolean       active);
void   picman_canvas_set_outline_bg_style    (GtkWidget     *canvas,
                                            cairo_t       *cr);
void   picman_canvas_set_outline_fg_style    (GtkWidget     *canvas,
                                            cairo_t       *cr);
void   picman_canvas_set_passe_partout_style (GtkWidget     *canvas,
                                            cairo_t       *cr);

void   picman_canvas_set_tool_bg_style       (GtkWidget     *canvas,
                                            cairo_t       *cr);
void   picman_canvas_set_tool_fg_style       (GtkWidget     *canvas,
                                            cairo_t       *cr,
                                            gboolean       highlight);


#endif /* __PICMAN_CANVAS_STYLE_H__ */
