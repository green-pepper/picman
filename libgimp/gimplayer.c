/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * picmanlayer.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"


/**
 * picman_layer_new:
 * @image_ID: The image to which to add the layer.
 * @name: The layer name.
 * @width: The layer width.
 * @height: The layer height.
 * @type: The layer type.
 * @opacity: The layer opacity.
 * @mode: The layer combination mode.
 *
 * Create a new layer.
 *
 * This procedure creates a new layer with the specified width, height,
 * and type. Name, opacity, and mode are also supplied parameters. The
 * new layer still needs to be added to the image, as this is not
 * automatic. Add the new layer with the picman_image_insert_layer()
 * command. Other attributes such as layer mask modes, and offsets
 * should be set with explicit procedure calls.
 *
 * Returns: The newly created layer.
 */
gint32
picman_layer_new (gint32                image_ID,
                const gchar          *name,
                gint                  width,
                gint                  height,
                PicmanImageType         type,
                gdouble               opacity,
                PicmanLayerModeEffects  mode)
{
  return _picman_layer_new (image_ID,
                          width,
                          height,
                          type,
                          name,
                          opacity,
                          mode);
}

/**
 * picman_layer_copy:
 * @layer_ID: The layer to copy.
 *
 * Copy a layer.
 *
 * This procedure copies the specified layer and returns the copy. The
 * newly copied layer is for use within the original layer's image. It
 * should not be subsequently added to any other image.
 *
 * Returns: The newly copied layer.
 */
gint32
picman_layer_copy (gint32  layer_ID)
{
  return _picman_layer_copy (layer_ID, FALSE);
}

/**
 * picman_layer_new_from_pixbuf:
 * @image_ID:       The RGB image to which to add the layer.
 * @name:           The layer name.
 * @pixbuf:         A GdkPixbuf.
 * @opacity:        The layer opacity.
 * @mode:           The layer combination mode.
 * @progress_start: start of progress
 * @progress_end:   end of progress
 *
 * Create a new layer from a %GdkPixbuf.
 *
 * This procedure creates a new layer from the given %GdkPixbuf.  The
 * image has to be an RGB image and just like with picman_layer_new()
 * you will still need to add the layer to it.
 *
 * If you pass @progress_end > @progress_start to this function,
 * picman_progress_update() will be called for. You have to call
 * picman_progress_init() beforehand then.
 *
 * Returns: The newly created layer.
 *
 * Since: PICMAN 2.4
 */
gint32
picman_layer_new_from_pixbuf (gint32                image_ID,
                            const gchar          *name,
                            GdkPixbuf            *pixbuf,
                            gdouble               opacity,
                            PicmanLayerModeEffects  mode,
                            gdouble               progress_start,
                            gdouble               progress_end)
{
  gint32  layer;
  gint    width;
  gint    height;
  gint    bpp;
  gdouble range = progress_end - progress_start;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), -1);

  if (picman_image_base_type (image_ID) != PICMAN_RGB)
    {
      g_warning ("picman_layer_new_from_pixbuf() needs an RGB image");
      return -1;
    }

  if (gdk_pixbuf_get_colorspace (pixbuf) != GDK_COLORSPACE_RGB)
    {
      g_warning ("picman_layer_new_from_pixbuf() assumes that GdkPixbuf is RGB");
      return -1;
    }

  width     = gdk_pixbuf_get_width (pixbuf);
  height    = gdk_pixbuf_get_height (pixbuf);
  bpp       = gdk_pixbuf_get_n_channels (pixbuf);

  layer = picman_layer_new (image_ID, name, width, height,
                          bpp == 3 ? PICMAN_RGB_IMAGE : PICMAN_RGBA_IMAGE,
                          opacity, mode);

  if (layer == -1)
    return -1;

  if (picman_plugin_precision_enabled ())
    {
      GeglBuffer *src_buffer;
      GeglBuffer *dest_buffer;

      src_buffer = picman_pixbuf_create_buffer (pixbuf);
      dest_buffer = picman_drawable_get_buffer (layer);

      gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);

      g_object_unref (src_buffer);
      g_object_unref (dest_buffer);
    }
  else
    {
      PicmanDrawable *drawable;
      PicmanPixelRgn  rgn;
      gpointer      pr;
      const guchar *pixels;
      gint          rowstride;
      guint         done  = 0;
      guint         count = 0;

      drawable = picman_drawable_get (layer);

      picman_pixel_rgn_init (&rgn, drawable, 0, 0, width, height, TRUE, FALSE);

      g_assert (bpp == rgn.bpp);

      rowstride = gdk_pixbuf_get_rowstride (pixbuf);
      pixels    = gdk_pixbuf_get_pixels (pixbuf);

      for (pr = picman_pixel_rgns_register (1, &rgn);
           pr != NULL;
           pr = picman_pixel_rgns_process (pr))
        {
          const guchar *src  = pixels + rgn.y * rowstride + rgn.x * bpp;
          guchar       *dest = rgn.data;
          gint          y;

          for (y = 0; y < rgn.h; y++)
            {
              memcpy (dest, src, rgn.w * rgn.bpp);

              src  += rowstride;
              dest += rgn.rowstride;
            }

          if (range > 0.0)
            {
              done += rgn.h * rgn.w;

              if (count++ % 32 == 0)
                picman_progress_update (progress_start +
                                      (gdouble) done / (width * height) * range);
            }
        }

      picman_drawable_detach (drawable);
    }

  if (range > 0.0)
    picman_progress_update (progress_end);

  return layer;
}

/**
 * picman_layer_new_from_surface:
 * @image_ID:        The RGB image to which to add the layer.
 * @name:            The layer name.
 * @surface:         A Cairo image surface.
 * @progress_start:  start of progress
 * @progress_end:    end of progress
 *
 * Create a new layer from a #cairo_surface_t.
 *
 * This procedure creates a new layer from the given
 * #cairo_surface_t. The image has to be an RGB image and just like
 * with picman_layer_new() you will still need to add the layer to it.
 *
 * If you pass @progress_end > @progress_start to this function,
 * picman_progress_update() will be called for. You have to call
 * picman_progress_init() beforehand then.
 *
 * Returns: The newly created layer.
 *
 * Since: PICMAN 2.8
 */
gint32
picman_layer_new_from_surface (gint32                image_ID,
                             const gchar          *name,
                             cairo_surface_t      *surface,
                             gdouble               progress_start,
                             gdouble               progress_end)
{
  gint32         layer;
  gint           width;
  gint           height;
  cairo_format_t format;
  gdouble        range = progress_end - progress_start;

  g_return_val_if_fail (surface != NULL, -1);
  g_return_val_if_fail (cairo_surface_get_type (surface) ==
                        CAIRO_SURFACE_TYPE_IMAGE, -1);

  if (picman_image_base_type (image_ID) != PICMAN_RGB)
    {
      g_warning ("picman_layer_new_from_surface() needs an RGB image");
      return -1;
    }

  width  = cairo_image_surface_get_width (surface);
  height = cairo_image_surface_get_height (surface);
  format = cairo_image_surface_get_format (surface);

  if (format != CAIRO_FORMAT_ARGB32 &&
      format != CAIRO_FORMAT_RGB24)
    {
      g_warning ("picman_layer_new_from_surface() assumes that surface is RGB");
      return -1;
    }

  layer = picman_layer_new (image_ID, name, width, height,
                          format == CAIRO_FORMAT_RGB24 ?
                          PICMAN_RGB_IMAGE : PICMAN_RGBA_IMAGE,
                          100.0, PICMAN_NORMAL_MODE);

  if (layer == -1)
    return -1;

  if (picman_plugin_precision_enabled ())
    {
      GeglBuffer *src_buffer;
      GeglBuffer *dest_buffer;

      src_buffer = picman_cairo_surface_create_buffer (surface);
      dest_buffer = picman_drawable_get_buffer (layer);

      gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);

      g_object_unref (src_buffer);
      g_object_unref (dest_buffer);
    }
  else
    {
      PicmanDrawable   *drawable;
      PicmanPixelRgn    rgn;
      const guchar   *pixels;
      gpointer        pr;
      gint            rowstride;
      guint           count = 0;
      guint           done  = 0;

      drawable = picman_drawable_get (layer);

      picman_pixel_rgn_init (&rgn, drawable, 0, 0, width, height, TRUE, FALSE);

      rowstride = cairo_image_surface_get_stride (surface);
      pixels    = cairo_image_surface_get_data (surface);

      for (pr = picman_pixel_rgns_register (1, &rgn);
           pr != NULL;
           pr = picman_pixel_rgns_process (pr))
        {
          const guchar *src  = pixels + rgn.y * rowstride + rgn.x * 4;
          guchar       *dest = rgn.data;
          gint          y;

          switch (format)
            {
            case CAIRO_FORMAT_RGB24:
              for (y = 0; y < rgn.h; y++)
                {
                  const guchar *s = src;
                  guchar       *d = dest;
                  gint          w = rgn.w;

                  while (w--)
                    {
                      PICMAN_CAIRO_RGB24_GET_PIXEL (s, d[0], d[1], d[2]);

                      s += 4;
                      d += 3;
                    }

                  src  += rowstride;
                  dest += rgn.rowstride;
                }
              break;

            case CAIRO_FORMAT_ARGB32:
              for (y = 0; y < rgn.h; y++)
                {
                  const guchar *s = src;
                  guchar       *d = dest;
                  gint          w = rgn.w;

                  while (w--)
                    {
                      PICMAN_CAIRO_ARGB32_GET_PIXEL (s, d[0], d[1], d[2], d[3]);

                      s += 4;
                      d += 4;
                    }

                  src  += rowstride;
                  dest += rgn.rowstride;
                }
              break;

            default:
              break;
            }

          if (range > 0.0)
            {
              done += rgn.h * rgn.w;

              if (count++ % 32 == 0)
                picman_progress_update (progress_start +
                                      (gdouble) done / (width * height) * range);
            }
        }

      picman_drawable_detach (drawable);
   }

  if (range > 0.0)
    picman_progress_update (progress_end);

  return layer;
}

/**
 * picman_layer_get_preserve_trans:
 * @layer_ID: The layer.
 *
 * This procedure is deprecated! Use picman_layer_get_lock_alpha() instead.
 *
 * Returns: The layer's preserve transperancy setting.
 */
gboolean
picman_layer_get_preserve_trans (gint32 layer_ID)
{
  return picman_layer_get_lock_alpha (layer_ID);
}

/**
 * picman_layer_set_preserve_trans:
 * @layer_ID: The layer.
 * @preserve_trans: The new layer's preserve transperancy setting.
 *
 * This procedure is deprecated! Use picman_layer_set_lock_alpha() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_layer_set_preserve_trans (gint32   layer_ID,
                               gboolean preserve_trans)
{
  return picman_layer_set_lock_alpha (layer_ID, preserve_trans);
}
