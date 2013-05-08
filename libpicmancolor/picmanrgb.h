/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_COLOR_H_INSIDE__) && !defined (PICMAN_COLOR_COMPILATION)
#error "Only <libpicmancolor/picmancolor.h> can be included directly."
#endif

#ifndef __PICMAN_RGB_H__
#define __PICMAN_RGB_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*
 * PICMAN_TYPE_RGB
 */

#define PICMAN_TYPE_RGB               (picman_rgb_get_type ())
#define PICMAN_VALUE_HOLDS_RGB(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_RGB))

GType   picman_rgb_get_type           (void) G_GNUC_CONST;

void    picman_value_get_rgb          (const GValue  *value,
                                     PicmanRGB       *rgb);
void    picman_value_set_rgb          (GValue        *value,
                                     const PicmanRGB *rgb);


/*
 * PICMAN_TYPE_PARAM_RGB
 */

#define PICMAN_TYPE_PARAM_RGB           (picman_param_rgb_get_type ())
#define PICMAN_IS_PARAM_SPEC_RGB(pspec) (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_RGB))


GType        picman_param_rgb_get_type       (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_rgb           (const gchar    *name,
                                            const gchar    *nick,
                                            const gchar    *blurb,
                                            gboolean        has_alpha,
                                            const PicmanRGB  *default_value,
                                            GParamFlags     flags);

gboolean     picman_param_spec_rgb_has_alpha (GParamSpec     *pspec);


/*  RGB and RGBA color types and operations taken from LibGCK  */

typedef enum
{
  PICMAN_RGB_COMPOSITE_NONE = 0,
  PICMAN_RGB_COMPOSITE_NORMAL,
  PICMAN_RGB_COMPOSITE_BEHIND
} PicmanRGBCompositeMode;


void      picman_rgb_set             (PicmanRGB       *rgb,
                                    gdouble        red,
                                    gdouble        green,
                                    gdouble        blue);
void      picman_rgb_set_alpha       (PicmanRGB       *rgb,
                                    gdouble        alpha);

void      picman_rgb_set_pixel       (PicmanRGB       *rgb,
                                    const Babl    *format,
                                    gconstpointer  pixel);
void      picman_rgb_get_pixel       (const PicmanRGB *rgb,
                                    const Babl    *format,
                                    gpointer       pixel);

void      picman_rgb_set_uchar       (PicmanRGB       *rgb,
                                    guchar         red,
                                    guchar         green,
                                    guchar         blue);
void      picman_rgb_get_uchar       (const PicmanRGB *rgb,
                                    guchar        *red,
                                    guchar        *green,
                                    guchar        *blue);

gboolean  picman_rgb_parse_name      (PicmanRGB       *rgb,
                                    const gchar   *name,
                                    gint           len);
gboolean  picman_rgb_parse_hex       (PicmanRGB       *rgb,
                                    const gchar   *hex,
                                    gint           len);
gboolean  picman_rgb_parse_css       (PicmanRGB       *rgb,
                                    const gchar   *css,
                                    gint           len);

void      picman_rgb_add             (PicmanRGB       *rgb1,
                                    const PicmanRGB *rgb2);
void      picman_rgb_subtract        (PicmanRGB       *rgb1,
                                    const PicmanRGB *rgb2);
void      picman_rgb_multiply        (PicmanRGB       *rgb1,
                                    gdouble        factor);
gdouble   picman_rgb_distance        (const PicmanRGB *rgb1,
                                    const PicmanRGB *rgb2);

gdouble   picman_rgb_max             (const PicmanRGB *rgb);
gdouble   picman_rgb_min             (const PicmanRGB *rgb);
void      picman_rgb_clamp           (PicmanRGB       *rgb);

void      picman_rgb_gamma           (PicmanRGB       *rgb,
                                    gdouble        gamma);

gdouble   picman_rgb_luminance       (const PicmanRGB *rgb);
guchar    picman_rgb_luminance_uchar (const PicmanRGB *rgb);

PICMAN_DEPRECATED_FOR(picman_rgb_luminance)
gdouble   picman_rgb_intensity       (const PicmanRGB *rgb);
PICMAN_DEPRECATED_FOR(picman_rgb_luminance_uchar)
guchar    picman_rgb_intensity_uchar (const PicmanRGB *rgb);

void      picman_rgb_composite       (PicmanRGB              *color1,
                                    const PicmanRGB        *color2,
                                    PicmanRGBCompositeMode  mode);

/*  access to the list of color names  */
gint      picman_rgb_list_names      (const gchar ***names,
                                    PicmanRGB      **colors);


void      picman_rgba_set            (PicmanRGB       *rgba,
                                    gdouble        red,
                                    gdouble        green,
                                    gdouble        blue,
                                    gdouble        alpha);

void      picman_rgba_set_pixel      (PicmanRGB       *rgba,
                                    const Babl    *format,
                                    gconstpointer  pixel);
void      picman_rgba_get_pixel      (const PicmanRGB *rgba,
                                    const Babl    *format,
                                    gpointer       pixel);

void      picman_rgba_set_uchar      (PicmanRGB       *rgba,
                                    guchar         red,
                                    guchar         green,
                                    guchar         blue,
                                    guchar         alpha);
void      picman_rgba_get_uchar      (const PicmanRGB *rgba,
                                    guchar        *red,
                                    guchar        *green,
                                    guchar        *blue,
                                    guchar        *alpha);

gboolean  picman_rgba_parse_css      (PicmanRGB       *rgba,
                                    const gchar   *css,
                                    gint           len);

void      picman_rgba_add            (PicmanRGB       *rgba1,
                                    const PicmanRGB *rgba2);
void      picman_rgba_subtract       (PicmanRGB       *rgba1,
                                    const PicmanRGB *rgba2);
void      picman_rgba_multiply       (PicmanRGB       *rgba,
                                    gdouble        factor);

gdouble   picman_rgba_distance       (const PicmanRGB *rgba1,
                                    const PicmanRGB *rgba2);



/*  Map RGB to intensity  */

/*
 * The weights to compute true CIE luminance from linear red, green
 * and blue, as defined by the ITU-R Recommendation BT.709, "Basic
 * Parameter Values for the HDTV Standard for the Studio and for
 * International Programme Exchange" (1990). Also suggested in the
 * sRGB colorspace specification by the W3C.
 */

#define PICMAN_RGB_LUMINANCE_RED    (0.2126)
#define PICMAN_RGB_LUMINANCE_GREEN  (0.7152)
#define PICMAN_RGB_LUMINANCE_BLUE   (0.0722)

#define PICMAN_RGB_LUMINANCE(r,g,b) ((r) * PICMAN_RGB_LUMINANCE_RED   + \
                                   (g) * PICMAN_RGB_LUMINANCE_GREEN + \
                                   (b) * PICMAN_RGB_LUMINANCE_BLUE)


#ifndef PICMAN_DISABLE_DEPRECATED

/*
 * The coefficients below properly computed luminance for monitors
 * having phosphors that were contemporary at the introduction of NTSC
 * television in 1953. They are still appropriate for computing video
 * luma. However, these coefficients do not accurately compute
 * luminance for contemporary monitors. The use of these definitions
 * is deprecated.
 */

#define PICMAN_RGB_INTENSITY_RED    (0.30)
#define PICMAN_RGB_INTENSITY_GREEN  (0.59)
#define PICMAN_RGB_INTENSITY_BLUE   (0.11)

#define PICMAN_RGB_INTENSITY(r,g,b) ((r) * PICMAN_RGB_INTENSITY_RED   + \
                                   (g) * PICMAN_RGB_INTENSITY_GREEN + \
                                   (b) * PICMAN_RGB_INTENSITY_BLUE)

#endif


G_END_DECLS

#endif  /* __PICMAN_RGB_H__ */
