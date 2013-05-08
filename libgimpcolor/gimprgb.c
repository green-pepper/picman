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

#include "config.h"

#include <babl/babl.h>
#include <glib-object.h>

#define PICMAN_DISABLE_DEPRECATION_WARNINGS /*  for PICMAN_RGB_INTENSITY()  */
#include "libpicmanmath/picmanmath.h"

#include "picmancolortypes.h"

#undef PICMAN_DISABLE_DEPRECATED  /*  for PICMAN_RGB_INTENSITY()  */
#include "picmanrgb.h"


/**
 * SECTION: picmanrgb
 * @title: PicmanRGB
 * @short_description: Definitions and Functions relating to RGB colors.
 *
 * Definitions and Functions relating to RGB colors.
 **/


/*
 * PICMAN_TYPE_RGB
 */

static PicmanRGB * picman_rgb_copy (const PicmanRGB *rgb);


GType
picman_rgb_get_type (void)
{
  static GType rgb_type = 0;

  if (!rgb_type)
    rgb_type = g_boxed_type_register_static ("PicmanRGB",
                                             (GBoxedCopyFunc) picman_rgb_copy,
                                             (GBoxedFreeFunc) g_free);

  return rgb_type;
}

void
picman_value_get_rgb (const GValue *value,
                    PicmanRGB      *rgb)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_RGB (value));
  g_return_if_fail (rgb != NULL);

  if (value->data[0].v_pointer)
    *rgb = *((PicmanRGB *) value->data[0].v_pointer);
  else
    picman_rgba_set (rgb, 0.0, 0.0, 0.0, 1.0);
}

void
picman_value_set_rgb (GValue        *value,
                    const PicmanRGB *rgb)
{
  g_return_if_fail (PICMAN_VALUE_HOLDS_RGB (value));
  g_return_if_fail (rgb != NULL);

  g_value_set_boxed (value, rgb);
}

static PicmanRGB *
picman_rgb_copy (const PicmanRGB *rgb)
{
  return g_memdup (rgb, sizeof (PicmanRGB));
}


/*  RGB functions  */

/**
 * picman_rgb_set:
 * @rgb:   a #PicmanRGB struct
 * @red:   the red component
 * @green: the green component
 * @blue:  the blue component
 *
 * Sets the red, green and blue components of @rgb and leaves the
 * alpha component unchanged. The color values should be between 0.0
 * and 1.0 but there is no check to enforce this and the values are
 * set exactly as they are passed in.
 **/
void
picman_rgb_set (PicmanRGB *rgb,
              gdouble  r,
              gdouble  g,
              gdouble  b)
{
  g_return_if_fail (rgb != NULL);

  rgb->r = r;
  rgb->g = g;
  rgb->b = b;
}

/**
 * picman_rgb_set_alpha:
 * @rgb:   a #PicmanRGB struct
 * @alpha: the alpha component
 *
 * Sets the alpha component of @rgb and leaves the RGB components unchanged.
 **/
void
picman_rgb_set_alpha (PicmanRGB *rgb,
                    gdouble  a)
{
  g_return_if_fail (rgb != NULL);

  rgb->a = a;
}

/**
 * picman_rgb_set:
 * @rgb:    a #PicmanRGB struct
 * @format: a Babl format
 * @pixel:  pointer to the source pixel
 *
 * Sets the red, green and blue components of @rgb from the color
 * stored in @pixel. The pixel format of @pixel is determined by
 * @format.
 *
 * Since: PICMAN 2.10
 **/
void
picman_rgb_set_pixel (PicmanRGB       *rgb,
                    const Babl    *format,
                    gconstpointer  pixel)
{
  g_return_if_fail (rgb != NULL);
  g_return_if_fail (format != NULL);
  g_return_if_fail (pixel != NULL);

  babl_process (babl_fish (format,
                           babl_format ("R'G'B' double")),
                pixel, rgb, 1);
}

/**
 * picman_rgb_get:
 * @rgb:    a #PicmanRGB struct
 * @format: a Babl format
 * @pixel:  pointer to the destination pixel
 *
 * Writes the red, green, blue and alpha components of @rgb to the
 * color stored in @pixel. The pixel format of @pixel is determined by
 * @format.
 *
 * Since: PICMAN 2.10
 **/
void
picman_rgb_get_pixel (const PicmanRGB *rgb,
                    const Babl    *format,
                    gpointer       pixel)
{
  g_return_if_fail (rgb != NULL);
  g_return_if_fail (format != NULL);
  g_return_if_fail (pixel != NULL);

  babl_process (babl_fish (babl_format ("R'G'B' double"),
                           format),
                rgb, pixel, 1);
}

/**
 * picman_rgb_set_uchar:
 * @rgb:   a #PicmanRGB struct
 * @red:   the red component
 * @green: the green component
 * @blue:  the blue component
 *
 * Sets the red, green and blue components of @rgb from 8bit values
 * (0 to 255) and leaves the alpha component unchanged.
 **/
void
picman_rgb_set_uchar (PicmanRGB *rgb,
                    guchar   r,
                    guchar   g,
                    guchar   b)
{
  g_return_if_fail (rgb != NULL);

  rgb->r = (gdouble) r / 255.0;
  rgb->g = (gdouble) g / 255.0;
  rgb->b = (gdouble) b / 255.0;
}

void
picman_rgb_get_uchar (const PicmanRGB *rgb,
                    guchar        *r,
                    guchar        *g,
                    guchar        *b)
{
  g_return_if_fail (rgb != NULL);

  if (r) *r = ROUND (CLAMP (rgb->r, 0.0, 1.0) * 255.0);
  if (g) *g = ROUND (CLAMP (rgb->g, 0.0, 1.0) * 255.0);
  if (b) *b = ROUND (CLAMP (rgb->b, 0.0, 1.0) * 255.0);
}

void
picman_rgb_add (PicmanRGB       *rgb1,
              const PicmanRGB *rgb2)
{
  g_return_if_fail (rgb1 != NULL);
  g_return_if_fail (rgb2 != NULL);

  rgb1->r += rgb2->r;
  rgb1->g += rgb2->g;
  rgb1->b += rgb2->b;
}

void
picman_rgb_subtract (PicmanRGB       *rgb1,
                   const PicmanRGB *rgb2)
{
  g_return_if_fail (rgb1 != NULL);
  g_return_if_fail (rgb2 != NULL);

  rgb1->r -= rgb2->r;
  rgb1->g -= rgb2->g;
  rgb1->b -= rgb2->b;
}

void
picman_rgb_multiply (PicmanRGB *rgb,
                   gdouble  factor)
{
  g_return_if_fail (rgb != NULL);

  rgb->r *= factor;
  rgb->g *= factor;
  rgb->b *= factor;
}

gdouble
picman_rgb_distance (const PicmanRGB *rgb1,
                   const PicmanRGB *rgb2)
{
  g_return_val_if_fail (rgb1 != NULL, 0.0);
  g_return_val_if_fail (rgb2 != NULL, 0.0);

  return (fabs (rgb1->r - rgb2->r) +
          fabs (rgb1->g - rgb2->g) +
          fabs (rgb1->b - rgb2->b));
}

gdouble
picman_rgb_max (const PicmanRGB *rgb)
{
  g_return_val_if_fail (rgb != NULL, 0.0);

  if (rgb->r > rgb->g)
    return (rgb->r > rgb->b) ? rgb->r : rgb->b;
  else
    return (rgb->g > rgb->b) ? rgb->g : rgb->b;
}

gdouble
picman_rgb_min (const PicmanRGB *rgb)
{
  g_return_val_if_fail (rgb != NULL, 0.0);

  if (rgb->r < rgb->g)
    return (rgb->r < rgb->b) ? rgb->r : rgb->b;
  else
    return (rgb->g < rgb->b) ? rgb->g : rgb->b;
}

void
picman_rgb_clamp (PicmanRGB *rgb)
{
  g_return_if_fail (rgb != NULL);

  rgb->r = CLAMP (rgb->r, 0.0, 1.0);
  rgb->g = CLAMP (rgb->g, 0.0, 1.0);
  rgb->b = CLAMP (rgb->b, 0.0, 1.0);
  rgb->a = CLAMP (rgb->a, 0.0, 1.0);
}

void
picman_rgb_gamma (PicmanRGB *rgb,
                gdouble  gamma)
{
  gdouble ig;

  g_return_if_fail (rgb != NULL);

  if (gamma != 0.0)
    ig = 1.0 / gamma;
  else
    ig = 0.0;

  rgb->r = pow (rgb->r, ig);
  rgb->g = pow (rgb->g, ig);
  rgb->b = pow (rgb->b, ig);
}

/**
 * picman_rgb_luminance:
 * @rgb: a #PicmanRGB struct
 *
 * Return value: the luminous intensity of the range from 0.0 to 1.0.
 *
 * Since: PICMAN 2.4
 **/
gdouble
picman_rgb_luminance (const PicmanRGB *rgb)
{
  gdouble luminance;

  g_return_val_if_fail (rgb != NULL, 0.0);

  luminance = PICMAN_RGB_LUMINANCE (rgb->r, rgb->g, rgb->b);

  return CLAMP (luminance, 0.0, 1.0);
}

/**
 * picman_rgb_luminance_uchar:
 * @rgb: a #PicmanRGB struct
 *
 * Return value: the luminous intensity in the range from 0 to 255.
 *
 * Since: PICMAN 2.4
 **/
guchar
picman_rgb_luminance_uchar (const PicmanRGB *rgb)
{
  g_return_val_if_fail (rgb != NULL, 0);

  return ROUND (picman_rgb_luminance (rgb) * 255.0);
}

/**
 * picman_rgb_intensity:
 * @rgb: a #PicmanRGB struct
 *
 * This function is deprecated! Use picman_rgb_luminance() instead.
 *
 * Return value: the intensity in the range from 0.0 to 1.0.
 **/
gdouble
picman_rgb_intensity (const PicmanRGB *rgb)
{
  gdouble intensity;

  g_return_val_if_fail (rgb != NULL, 0.0);

  intensity = PICMAN_RGB_INTENSITY (rgb->r, rgb->g, rgb->b);

  return CLAMP (intensity, 0.0, 1.0);
}

/**
 * picman_rgb_intensity_uchar:
 * @rgb: a #PicmanRGB struct
 *
 * This function is deprecated! Use picman_rgb_luminance_uchar() instead.
 *
 * Return value: the intensity in the range from 0 to 255.
 **/
guchar
picman_rgb_intensity_uchar (const PicmanRGB *rgb)
{
  g_return_val_if_fail (rgb != NULL, 0);

  return ROUND (picman_rgb_intensity (rgb) * 255.0);
}

void
picman_rgb_composite (PicmanRGB              *color1,
                    const PicmanRGB        *color2,
                    PicmanRGBCompositeMode  mode)
{
  g_return_if_fail (color1 != NULL);
  g_return_if_fail (color2 != NULL);

  switch (mode)
    {
    case PICMAN_RGB_COMPOSITE_NONE:
      break;

    case PICMAN_RGB_COMPOSITE_NORMAL:
      /*  put color2 on top of color1  */
      if (color2->a == 1.0)
        {
          *color1 = *color2;
        }
      else
        {
          gdouble factor = color1->a * (1.0 - color2->a);

          color1->r = color1->r * factor + color2->r * color2->a;
          color1->g = color1->g * factor + color2->g * color2->a;
          color1->b = color1->b * factor + color2->b * color2->a;
          color1->a = factor + color2->a;
        }
      break;

    case PICMAN_RGB_COMPOSITE_BEHIND:
      /*  put color2 below color1  */
      if (color1->a < 1.0)
        {
          gdouble factor = color2->a * (1.0 - color1->a);

          color1->r = color2->r * factor + color1->r * color1->a;
          color1->g = color2->g * factor + color1->g * color1->a;
          color1->b = color2->b * factor + color1->b * color1->a;
          color1->a = factor + color1->a;
        }
      break;
    }
}

/*  RGBA functions  */

/**
 * picman_rgba_set:
 * @rgba:   a #PicmanRGB struct
 * @format: a Babl format
 * @pixel:  pointer to the source pixel
 *
 * Sets the red, green, blue and alpha components of @rgba from the
 * color stored in @pixel. The pixel format of @pixel is determined
 * by @format.
 *
 * Since: PICMAN 2.10
 **/
void
picman_rgba_set_pixel (PicmanRGB       *rgba,
                     const Babl    *format,
                     gconstpointer  pixel)
{
  g_return_if_fail (rgba != NULL);
  g_return_if_fail (format != NULL);
  g_return_if_fail (pixel != NULL);

  babl_process (babl_fish (format,
                           babl_format ("R'G'B'A double")),
                pixel, rgba, 1);
}

/**
 * picman_rgba_get:
 * @rgba:   a #PicmanRGB struct
 * @format: a Babl format
 * @pixel:  pointer to the destination pixel
 *
 * Writes the red, green, blue and alpha components of @rgba to the
 * color stored in @pixel. The pixel format of @pixel is determined by
 * @format.
 *
 * Since: PICMAN 2.10
 **/
void
picman_rgba_get_pixel (const PicmanRGB *rgba,
                     const Babl    *format,
                     gpointer       pixel)
{
  g_return_if_fail (rgba != NULL);
  g_return_if_fail (format != NULL);
  g_return_if_fail (pixel != NULL);

  babl_process (babl_fish (babl_format ("R'G'B'A double"),
                           format),
                rgba, pixel, 1);
}

/**
 * picman_rgba_set:
 * @rgba:  a #PicmanRGB struct
 * @red:   the red component
 * @green: the green component
 * @blue:  the blue component
 * @alpha: the alpha component
 *
 * Sets the red, green, blue and alpha components of @rgb. The values
 * should be between 0.0 and 1.0 but there is no check to enforce this
 * and the values are set exactly as they are passed in.
 **/
void
picman_rgba_set (PicmanRGB *rgba,
               gdouble  r,
               gdouble  g,
               gdouble  b,
               gdouble  a)
{
  g_return_if_fail (rgba != NULL);

  rgba->r = r;
  rgba->g = g;
  rgba->b = b;
  rgba->a = a;
}

/**
 * picman_rgba_set_uchar:
 * @rgba:  a #PicmanRGB struct
 * @red:   the red component
 * @green: the green component
 * @blue:  the blue component
 * @alpha: the alpha component
 *
 * Sets the red, green, blue and alpha components of @rgb from 8bit
 * values (0 to 255).
 **/
void
picman_rgba_set_uchar (PicmanRGB *rgba,
                     guchar   r,
                     guchar   g,
                     guchar   b,
                     guchar   a)
{
  g_return_if_fail (rgba != NULL);

  rgba->r = (gdouble) r / 255.0;
  rgba->g = (gdouble) g / 255.0;
  rgba->b = (gdouble) b / 255.0;
  rgba->a = (gdouble) a / 255.0;
}

void
picman_rgba_get_uchar (const PicmanRGB *rgba,
                     guchar        *r,
                     guchar        *g,
                     guchar        *b,
                     guchar        *a)
{
  g_return_if_fail (rgba != NULL);

  if (r) *r = ROUND (CLAMP (rgba->r, 0.0, 1.0) * 255.0);
  if (g) *g = ROUND (CLAMP (rgba->g, 0.0, 1.0) * 255.0);
  if (b) *b = ROUND (CLAMP (rgba->b, 0.0, 1.0) * 255.0);
  if (a) *a = ROUND (CLAMP (rgba->a, 0.0, 1.0) * 255.0);
}

void
picman_rgba_add (PicmanRGB       *rgba1,
               const PicmanRGB *rgba2)
{
  g_return_if_fail (rgba1 != NULL);
  g_return_if_fail (rgba2 != NULL);

  rgba1->r += rgba2->r;
  rgba1->g += rgba2->g;
  rgba1->b += rgba2->b;
  rgba1->a += rgba2->a;
}

void
picman_rgba_subtract (PicmanRGB       *rgba1,
                    const PicmanRGB *rgba2)
{
  g_return_if_fail (rgba1 != NULL);
  g_return_if_fail (rgba2 != NULL);

  rgba1->r -= rgba2->r;
  rgba1->g -= rgba2->g;
  rgba1->b -= rgba2->b;
  rgba1->a -= rgba2->a;
}

void
picman_rgba_multiply (PicmanRGB *rgba,
                    gdouble  factor)
{
  g_return_if_fail (rgba != NULL);

  rgba->r *= factor;
  rgba->g *= factor;
  rgba->b *= factor;
  rgba->a *= factor;
}

gdouble
picman_rgba_distance (const PicmanRGB *rgba1,
                    const PicmanRGB *rgba2)
{
  g_return_val_if_fail (rgba1 != NULL, 0.0);
  g_return_val_if_fail (rgba2 != NULL, 0.0);

  return (fabs (rgba1->r - rgba2->r) +
          fabs (rgba1->g - rgba2->g) +
          fabs (rgba1->b - rgba2->b) +
          fabs (rgba1->a - rgba2->a));
}


/*
 * PICMAN_TYPE_PARAM_RGB
 */

#define PICMAN_PARAM_SPEC_RGB(pspec)    (G_TYPE_CHECK_INSTANCE_CAST ((pspec), PICMAN_TYPE_PARAM_RGB, PicmanParamSpecRGB))

typedef struct _PicmanParamSpecRGB PicmanParamSpecRGB;

struct _PicmanParamSpecRGB
{
  GParamSpecBoxed  parent_instance;

  gboolean         has_alpha;
  PicmanRGB          default_value;
};

static void       picman_param_rgb_class_init  (GParamSpecClass *class);
static void       picman_param_rgb_init        (GParamSpec      *pspec);
static void       picman_param_rgb_set_default (GParamSpec      *pspec,
                                              GValue          *value);
static gboolean   picman_param_rgb_validate    (GParamSpec      *pspec,
                                              GValue          *value);
static gint       picman_param_rgb_values_cmp  (GParamSpec      *pspec,
                                              const GValue    *value1,
                                              const GValue    *value2);

/**
 * picman_param_rgb_get_type:
 *
 * Reveals the object type
 *
 * Returns: the #GType for a PicmanParamRGB object
 *
 * Since: PICMAN 2.4
 **/
GType
picman_param_rgb_get_type (void)
{
  static GType spec_type = 0;

  if (! spec_type)
    {
      const GTypeInfo type_info =
      {
        sizeof (GParamSpecClass),
        NULL, NULL,
        (GClassInitFunc) picman_param_rgb_class_init,
        NULL, NULL,
        sizeof (PicmanParamSpecRGB),
        0,
        (GInstanceInitFunc) picman_param_rgb_init
      };

      spec_type = g_type_register_static (G_TYPE_PARAM_BOXED,
                                          "PicmanParamRGB",
                                          &type_info, 0);
    }

  return spec_type;
}

static void
picman_param_rgb_class_init (GParamSpecClass *class)
{
  class->value_type        = PICMAN_TYPE_RGB;
  class->value_set_default = picman_param_rgb_set_default;
  class->value_validate    = picman_param_rgb_validate;
  class->values_cmp        = picman_param_rgb_values_cmp;
}

static void
picman_param_rgb_init (GParamSpec *pspec)
{
  PicmanParamSpecRGB *cspec = PICMAN_PARAM_SPEC_RGB (pspec);

  picman_rgba_set (&cspec->default_value, 0.0, 0.0, 0.0, 1.0);
}

static void
picman_param_rgb_set_default (GParamSpec *pspec,
                            GValue     *value)
{
  PicmanParamSpecRGB *cspec = PICMAN_PARAM_SPEC_RGB (pspec);

  g_value_set_static_boxed (value, &cspec->default_value);
}

static gboolean
picman_param_rgb_validate (GParamSpec *pspec,
                         GValue     *value)
{
  PicmanRGB *rgb = value->data[0].v_pointer;

  if (rgb)
    {
      PicmanRGB oval = *rgb;

      picman_rgb_clamp (rgb);

      return (oval.r != rgb->r ||
              oval.g != rgb->g ||
              oval.b != rgb->b ||
              (PICMAN_PARAM_SPEC_RGB (pspec)->has_alpha && oval.a != rgb->a));
    }

  return FALSE;
}

static gint
picman_param_rgb_values_cmp (GParamSpec   *pspec,
                           const GValue *value1,
                           const GValue *value2)
{
  PicmanRGB *rgb1 = value1->data[0].v_pointer;
  PicmanRGB *rgb2 = value2->data[0].v_pointer;

  /*  try to return at least *something*, it's useless anyway...  */

  if (! rgb1)
    {
      return rgb2 != NULL ? -1 : 0;
    }
  else if (! rgb2)
    {
      return rgb1 != NULL;
    }
  else
    {
      guint32 int1 = 0;
      guint32 int2 = 0;

      if (PICMAN_PARAM_SPEC_RGB (pspec)->has_alpha)
        {
          picman_rgba_get_uchar (rgb1,
                               ((guchar *) &int1) + 0,
                               ((guchar *) &int1) + 1,
                               ((guchar *) &int1) + 2,
                               ((guchar *) &int1) + 3);
          picman_rgba_get_uchar (rgb2,
                               ((guchar *) &int2) + 0,
                               ((guchar *) &int2) + 1,
                               ((guchar *) &int2) + 2,
                               ((guchar *) &int2) + 3);
        }
      else
        {
          picman_rgb_get_uchar (rgb1,
                              ((guchar *) &int1) + 0,
                              ((guchar *) &int1) + 1,
                              ((guchar *) &int1) + 2);
          picman_rgb_get_uchar (rgb2,
                              ((guchar *) &int2) + 0,
                              ((guchar *) &int2) + 1,
                              ((guchar *) &int2) + 2);
        }

      return int1 - int2;
    }
}

/**
 * picman_param_spec_rgb:
 * @name:          Canonical name of the param
 * @nick:          Nickname of the param
 * @blurb:         Brief desciption of param.
 * @has_alpha:     %TRUE if the alpha channel has relevance.
 * @default_value: Value to use if none is assigned.
 * @flags:         a combination of #GParamFlags
 *
 * Creates a param spec to hold an #PicmanRGB value.
 * See g_param_spec_internal() for more information.
 *
 * Returns: a newly allocated #GParamSpec instance
 *
 * Since: PICMAN 2.4
 **/
GParamSpec *
picman_param_spec_rgb (const gchar   *name,
                     const gchar   *nick,
                     const gchar   *blurb,
                     gboolean       has_alpha,
                     const PicmanRGB *default_value,
                     GParamFlags    flags)
{
  PicmanParamSpecRGB *cspec;

  cspec = g_param_spec_internal (PICMAN_TYPE_PARAM_RGB,
                                 name, nick, blurb, flags);

  cspec->has_alpha = has_alpha;

  if (default_value)
    cspec->default_value = *default_value;

  return G_PARAM_SPEC (cspec);
}

/**
 * picman_param_spec_rgb_has_alpha:
 * @pspec: a #GParamSpec to hold an #PicmanRGB value.
 *
 * Returns: %TRUE if the alpha channel is relevant.
 *
 * Since: PICMAN 2.4
 **/
gboolean
picman_param_spec_rgb_has_alpha (GParamSpec *pspec)
{
  g_return_val_if_fail (PICMAN_IS_PARAM_SPEC_RGB (pspec), FALSE);

  return PICMAN_PARAM_SPEC_RGB (pspec)->has_alpha;
}
