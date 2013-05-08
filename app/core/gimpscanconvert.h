/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_SCAN_CONVERT_H__
#define __PICMAN_SCAN_CONVERT_H__


PicmanScanConvert * picman_scan_convert_new        (void);

void      picman_scan_convert_free               (PicmanScanConvert   *sc);
void      picman_scan_convert_set_pixel_ratio    (PicmanScanConvert   *sc,
                                                gdouble            ratio_xy);
void      picman_scan_convert_set_clip_rectangle (PicmanScanConvert   *sc,
                                                gint               x,
                                                gint               y,
                                                gint               width,
                                                gint               height);
void      picman_scan_convert_add_polyline       (PicmanScanConvert   *sc,
                                                guint              n_points,
                                                const PicmanVector2 *points,
                                                gboolean           closed);
void      picman_scan_convert_add_bezier         (PicmanScanConvert      *sc,
                                                const PicmanBezierDesc *bezier);
void      picman_scan_convert_stroke             (PicmanScanConvert   *sc,
                                                gdouble            width,
                                                PicmanJoinStyle      join,
                                                PicmanCapStyle       cap,
                                                gdouble            miter,
                                                gdouble            dash_offset,
                                                GArray            *dash_info);
void      picman_scan_convert_render_full        (PicmanScanConvert   *sc,
                                                GeglBuffer        *buffer,
                                                gint               off_x,
                                                gint               off_y,
                                                gboolean           replace,
                                                gboolean           antialias,
                                                gdouble            value);

void      picman_scan_convert_render             (PicmanScanConvert   *sc,
                                                GeglBuffer        *buffer,
                                                gint               off_x,
                                                gint               off_y,
                                                gboolean           antialias);
void      picman_scan_convert_render_value       (PicmanScanConvert   *sc,
                                                GeglBuffer        *buffer,
                                                gint               off_x,
                                                gint               off_y,
                                                gdouble            value);
void      picman_scan_convert_compose            (PicmanScanConvert   *sc,
                                                GeglBuffer        *buffer,
                                                gint               off_x,
                                                gint               off_y);
void      picman_scan_convert_compose_value      (PicmanScanConvert   *sc,
                                                GeglBuffer        *buffer,
                                                gint               off_x,
                                                gint               off_y,
                                                gdouble            value);


#endif /* __PICMAN_SCAN_CONVERT_H__ */
