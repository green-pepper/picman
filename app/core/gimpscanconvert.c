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

#include "config.h"

#include <string.h>

#include <gegl.h>

#include <cairo.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmanbezierdesc.h"
#include "picmanscanconvert.h"


struct _PicmanScanConvert
{
  gdouble         ratio_xy;

  gboolean        clip;
  gint            clip_x;
  gint            clip_y;
  gint            clip_w;
  gint            clip_h;

  /* stroking options */
  gboolean        do_stroke;
  gdouble         width;
  PicmanJoinStyle   join;
  PicmanCapStyle    cap;
  gdouble         miter;
  gdouble         dash_offset;
  GArray         *dash_info;

  GArray         *path_data;
};


/*  public functions  */

/**
 * picman_scan_convert_new:
 *
 * Create a new scan conversion context.
 *
 * Return value: a newly allocated #PicmanScanConvert context.
 */
PicmanScanConvert *
picman_scan_convert_new (void)
{
  PicmanScanConvert *sc = g_slice_new0 (PicmanScanConvert);

  sc->path_data = g_array_new (FALSE, FALSE, sizeof (cairo_path_data_t));
  sc->ratio_xy = 1.0;

  return sc;
}

/**
 * picman_scan_convert_free:
 * @sc: a #PicmanScanConvert context
 *
 * Frees the resources allocated for @sc.
 */
void
picman_scan_convert_free (PicmanScanConvert *sc)
{
  g_return_if_fail (sc != NULL);

  if (sc->path_data)
    g_array_free (sc->path_data, TRUE);

  if (sc->dash_info)
    g_array_free (sc->dash_info, TRUE);

  g_slice_free (PicmanScanConvert, sc);
}

/**
 * picman_scan_convert_set_pixel_ratio:
 * @sc:       a #PicmanScanConvert context
 * @ratio_xy: the aspect ratio of the major coordinate axes
 *
 * Sets the pixel aspect ratio.
 */
void
picman_scan_convert_set_pixel_ratio (PicmanScanConvert *sc,
                                   gdouble          ratio_xy)
{
  g_return_if_fail (sc != NULL);

  /* we only need the relative resolution */
  sc->ratio_xy = ratio_xy;
}

/**
 * picman_scan_convert_set_clip_rectangle
 * @sc:     a #PicmanScanConvert context
 * @x:      horizontal offset of clip rectangle
 * @y:      vertical offset of clip rectangle
 * @width:  width of clip rectangle
 * @height: height of clip rectangle
 *
 * Sets a clip rectangle on @sc. Subsequent render operations will be
 * restricted to this area.
 */
void
picman_scan_convert_set_clip_rectangle (PicmanScanConvert *sc,
                                      gint             x,
                                      gint             y,
                                      gint             width,
                                      gint             height)
{
  g_return_if_fail (sc != NULL);

  sc->clip   = TRUE;
  sc->clip_x = x;
  sc->clip_y = y;
  sc->clip_w = width;
  sc->clip_h = height;
}

/**
 * picman_scan_convert_add_polyline:
 * @sc:       a #PicmanScanConvert context
 * @n_points: number of points to add
 * @points:   array of points to add
 * @closed:   whether to close the polyline and make it a polygon
 *
 * Add a polyline with @n_points @points that may be open or closed.
 *
 * Please note that you should use picman_scan_convert_stroke() if you
 * specify open polygons.
 */
void
picman_scan_convert_add_polyline (PicmanScanConvert   *sc,
                                guint              n_points,
                                const PicmanVector2 *points,
                                gboolean           closed)
{
  PicmanVector2        prev = { 0.0, 0.0, };
  cairo_path_data_t  pd;
  gint               i;

  g_return_if_fail (sc != NULL);
  g_return_if_fail (points != NULL);
  g_return_if_fail (n_points > 0);

  for (i = 0; i < n_points; i++)
    {
      /* compress multiple identical coordinates */
      if (i == 0 ||
          prev.x != points[i].x ||
          prev.y != points[i].y)
        {
          pd.header.type = (i == 0) ? CAIRO_PATH_MOVE_TO : CAIRO_PATH_LINE_TO;
          pd.header.length = 2;
          sc->path_data = g_array_append_val (sc->path_data, pd);

          pd.point.x = points[i].x;
          pd.point.y = points[i].y;
          sc->path_data = g_array_append_val (sc->path_data, pd);
          prev = points[i];
        }
    }

  /* close the polyline when needed */
  if (closed)
    {
      pd.header.type = CAIRO_PATH_CLOSE_PATH;
      pd.header.length = 1;
      sc->path_data = g_array_append_val (sc->path_data, pd);
    }
}

/**
 * picman_scan_convert_add_polyline:
 * @sc:     a #PicmanScanConvert context
 * @bezier: a #PicmanBezierDesc
 *
 * Adds a @bezier path to @sc.
 *
 * Please note that you should use picman_scan_convert_stroke() if you
 * specify open paths.
 **/
void
picman_scan_convert_add_bezier (PicmanScanConvert       *sc,
                              const PicmanBezierDesc  *bezier)
{
  g_return_if_fail (sc != NULL);
  g_return_if_fail (bezier != NULL);

  sc->path_data = g_array_append_vals (sc->path_data,
                                       bezier->data, bezier->num_data);
}

/**
 * picman_scan_convert_stroke:
 * @sc:          a #PicmanScanConvert context
 * @width:       line width in pixels
 * @join:        how lines should be joined
 * @cap:         how to render the end of lines
 * @miter:       convert a mitered join to a bevelled join if the miter would
 *               extend to a distance of more than @miter times @width from
 *               the actual join point
 * @dash_offset: offset to apply on the dash pattern
 * @dash_info:   dash pattern or %NULL for a solid line
 *
 * Stroke the content of a PicmanScanConvert. The next
 * picman_scan_convert_render() will result in the outline of the
 * polygon defined with the commands above.
 *
 * You cannot add additional polygons after this command.
 *
 * Note that if you have nonstandard resolution, "width" gives the
 * width (in pixels) for a vertical stroke, i.e. use the X resolution
 * to calculate the width of a stroke when operating with real world
 * units.
 */
void
picman_scan_convert_stroke (PicmanScanConvert *sc,
                          gdouble          width,
                          PicmanJoinStyle    join,
                          PicmanCapStyle     cap,
                          gdouble          miter,
                          gdouble          dash_offset,
                          GArray          *dash_info)
{
  sc->do_stroke = TRUE;
  sc->width = width;
  sc->join  = join;
  sc->cap   = cap;
  sc->miter = miter;
  if (sc->dash_info)
    {
      g_array_free (sc->dash_info, TRUE);
      sc->dash_info = NULL;
    }

  if (dash_info && dash_info->len >= 2)
    {
      gint          n_dashes;
      gdouble      *dashes;
      gint          i;

      dash_offset = dash_offset * MAX (width, 1.0);

      n_dashes = dash_info->len;
      dashes = g_new (gdouble, dash_info->len);

      for (i = 0; i < dash_info->len ; i++)
        dashes[i] = MAX (width, 1.0) * g_array_index (dash_info, gdouble, i);

      /* correct 0.0 in the first element (starts with a gap) */

      if (dashes[0] == 0.0)
        {
          gdouble first;

          first = dashes[1];

          /* shift the pattern to really starts with a dash and
           * use the offset to skip into it.
           */
          for (i = 0; i < dash_info->len - 2; i++)
            {
              dashes[i] = dashes[i+2];
              dash_offset += dashes[i];
            }

          if (dash_info->len % 2 == 1)
            {
              dashes[dash_info->len - 2] = first;
              n_dashes --;
            }
          else if (dash_info->len > 2)
           {
             dashes [dash_info->len - 3] += first;
             n_dashes -= 2;
           }
        }

      /* correct odd number of dash specifiers */

      if (n_dashes % 2 == 1)
        {
          gdouble last;

          last = dashes[n_dashes - 1];
          dashes[0]   += last;
          dash_offset += last;
          n_dashes --;
        }

      if (n_dashes >= 2)
        {
          sc->dash_info = g_array_sized_new (FALSE, FALSE,
                                             sizeof (gdouble), n_dashes);
          sc->dash_info = g_array_append_vals (sc->dash_info, dashes, n_dashes);
          sc->dash_offset = dash_offset;
        }

      g_free (dashes);
    }
}


/**
 * picman_scan_convert_render:
 * @sc:        a #PicmanScanConvert context
 * @bufferr:   the #GeglBuffer to render to
 * @off_x:     horizontal offset into the @buffer
 * @off_y:     vertical offset into the @buffer
 * @antialias: whether to apply antialiasiing
 *
 * This is a wrapper around picman_scan_convert_render_full() that replaces the
 * content of the @buffer with a rendered form of the path passed in.
 *
 * You cannot add additional polygons after this command.
 */
void
picman_scan_convert_render (PicmanScanConvert *sc,
                          GeglBuffer      *buffer,
                          gint             off_x,
                          gint             off_y,
                          gboolean         antialias)
{
  picman_scan_convert_render_full (sc, buffer, off_x, off_y,
                                 TRUE, antialias, 1.0);
}

/**
 * picman_scan_convert_render_value:
 * @sc:     a #PicmanScanConvert context
 * @buffer: the #GeglBuffer to render to
 * @off_x:  horizontal offset into the @buffer
 * @off_y:  vertical offset into the @buffer
 * @value:  value to use for covered pixels
 *
 * This is a wrapper around picman_scan_convert_render_full() that
 * doesn't do antialiasing but gives control over the value that
 * should be used for pixels covered by the scan conversion. Uncovered
 * pixels are set to zero.
 *
 * You cannot add additional polygons after this command.
 */
void
picman_scan_convert_render_value (PicmanScanConvert *sc,
                                GeglBuffer      *buffer,
                                gint             off_x,
                                gint             off_y,
                                gdouble          value)
{
  picman_scan_convert_render_full (sc, buffer, off_x, off_y,
                                 TRUE, FALSE, value);
}

/**
 * picman_scan_convert_compose:
 * @sc:     a #PicmanScanConvert context
 * @buffer: the #GeglBuffer to render to
 * @off_x:  horizontal offset into the @buffer
 * @off_y:  vertical offset into the @buffer
 *
 * This is a wrapper around of picman_scan_convert_render_full() that composes
 * the (aliased) scan conversion on top of the content of the @buffer.
 *
 * You cannot add additional polygons after this command.
 */
void
picman_scan_convert_compose (PicmanScanConvert *sc,
                           GeglBuffer      *buffer,
                           gint             off_x,
                           gint             off_y)
{
  picman_scan_convert_render_full (sc, buffer, off_x, off_y,
                                 FALSE, FALSE, 1.0);
}

/**
 * picman_scan_convert_compose_value:
 * @sc:     a #PicmanScanConvert context
 * @buffer: the #GeglBuffer to render to
 * @off_x:  horizontal offset into the @buffer
 * @off_y:  vertical offset into the @buffer
 * @value:  value to use for covered pixels
 *
 * This is a wrapper around picman_scan_convert_render_full() that
 * composes the (aliased) scan conversion with value @value on top of the
 * content of the @buffer.
 *
 * You cannot add additional polygons after this command.
 */
void
picman_scan_convert_compose_value (PicmanScanConvert *sc,
                                 GeglBuffer      *buffer,
                                 gint             off_x,
                                 gint             off_y,
                                 gdouble          value)
{
  picman_scan_convert_render_full (sc, buffer, off_x, off_y,
                                 FALSE, FALSE, value);
}

/**
 * picman_scan_convert_render_full:
 * @sc:        a #PicmanScanConvert context
 * @buffer:    the #GeglBuffer to render to
 * @off_x:     horizontal offset into the @buffer
 * @off_y:     vertical offset into the @buffer
 * @replace:   if true the original content of the @buffer gets estroyed
 * @antialias: if true the rendering happens antialiased
 * @value:     value to use for covered pixels
 *
 * This function renders the area described by the path to the
 * @buffer, taking the offset @off_x and @off_y in the buffer into
 * account.  The rendering can happen antialiased and be rendered on
 * top of existing content or replacing it completely. The @value
 * specifies the opacity value to be used for the objects in the @sc.
 *
 * You cannot add additional polygons after this command.
 */
void
picman_scan_convert_render_full (PicmanScanConvert *sc,
                               GeglBuffer      *buffer,
                               gint             off_x,
                               gint             off_y,
                               gboolean         replace,
                               gboolean         antialias,
                               gdouble          value)
{
  const Babl         *format;
  GeglBufferIterator *iter;
  GeglRectangle      *roi;
  cairo_t            *cr;
  cairo_surface_t    *surface;
  cairo_path_t        path;
  gint                bpp;
  gint                x, y;
  gint                width, height;

  g_return_if_fail (sc != NULL);
  g_return_if_fail (GEGL_IS_BUFFER (buffer));

  x      = 0;
  y      = 0;
  width  = gegl_buffer_get_width  (buffer);
  height = gegl_buffer_get_height (buffer);

  if (sc->clip && ! picman_rectangle_intersect (x, y, width, height,
                                              sc->clip_x, sc->clip_y,
                                              sc->clip_w, sc->clip_h,
                                              &x, &y, &width, &height))
    return;

  path.status   = CAIRO_STATUS_SUCCESS;
  path.data     = (cairo_path_data_t *) sc->path_data->data;
  path.num_data = sc->path_data->len;

  format = babl_format ("Y u8");
  bpp    = babl_format_get_bytes_per_pixel (format);

  iter = gegl_buffer_iterator_new (buffer, NULL, 0, format,
                                   GEGL_BUFFER_READWRITE, GEGL_ABYSS_NONE);
  roi = &iter->roi[0];

  while (gegl_buffer_iterator_next (iter))
    {
      guchar     *data    = iter->data[0];
      guchar     *tmp_buf = NULL;
      const gint  stride  = cairo_format_stride_for_width (CAIRO_FORMAT_A8,
                                                           roi->width);

      /*  cairo rowstrides are always multiples of 4, whereas
       *  maskPR.rowstride can be anything, so to be able to create an
       *  image surface, we maybe have to create our own temporary
       *  buffer
       */
      if (roi->width * bpp != stride)
        {
          tmp_buf = g_alloca (stride * roi->height);

          if (! replace)
            {
              const guchar *src  = data;
              guchar       *dest = tmp_buf;
              gint          i;

              for (i = 0; i < roi->height; i++)
                {
                  memcpy (dest, src, roi->width * bpp);

                  src  += roi->width * bpp;
                  dest += stride;
                }
            }
        }

      surface = cairo_image_surface_create_for_data (tmp_buf ?
                                                     tmp_buf : data,
                                                     CAIRO_FORMAT_A8,
                                                     roi->width, roi->height,
                                                     stride);

      cairo_surface_set_device_offset (surface,
                                       -off_x - roi->x,
                                       -off_y - roi->y);
      cr = cairo_create (surface);
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

      if (replace)
        {
          cairo_set_source_rgba (cr, 0, 0, 0, 0);
          cairo_paint (cr);
        }

      cairo_set_source_rgba (cr, 0, 0, 0, value);
      cairo_append_path (cr, &path);

      cairo_set_antialias (cr, antialias ?
                           CAIRO_ANTIALIAS_GRAY : CAIRO_ANTIALIAS_NONE);
      cairo_set_miter_limit (cr, sc->miter);

      if (sc->do_stroke)
        {
          cairo_set_line_cap (cr,
                              sc->cap == PICMAN_CAP_BUTT ? CAIRO_LINE_CAP_BUTT :
                              sc->cap == PICMAN_CAP_ROUND ? CAIRO_LINE_CAP_ROUND :
                              CAIRO_LINE_CAP_SQUARE);
          cairo_set_line_join (cr,
                               sc->join == PICMAN_JOIN_MITER ? CAIRO_LINE_JOIN_MITER :
                               sc->join == PICMAN_JOIN_ROUND ? CAIRO_LINE_JOIN_ROUND :
                               CAIRO_LINE_JOIN_BEVEL);

          cairo_set_line_width (cr, sc->width);

          if (sc->dash_info)
            cairo_set_dash (cr,
                            (double *) sc->dash_info->data,
                            sc->dash_info->len,
                            sc->dash_offset);

          cairo_scale (cr, 1.0, sc->ratio_xy);
          cairo_stroke (cr);
        }
      else
        {
          cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
          cairo_fill (cr);
        }

      cairo_destroy (cr);
      cairo_surface_destroy (surface);

      if (tmp_buf)
        {
          const guchar *src  = tmp_buf;
          guchar       *dest = data;
          gint          i;

          for (i = 0; i < roi->height; i++)
            {
              memcpy (dest, src, roi->width * bpp);

              src  += stride;
              dest += roi->width * bpp;
            }
        }
    }
}
