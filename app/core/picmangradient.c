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

#include "config.h"

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmancontext.h"
#include "picmangradient.h"
#include "picmangradient-load.h"
#include "picmangradient-save.h"
#include "picmantagged.h"
#include "picmantempbuf.h"


#define EPSILON 1e-10


static void          picman_gradient_tagged_iface_init (PicmanTaggedInterface *iface);
static void          picman_gradient_finalize          (GObject             *object);

static gint64        picman_gradient_get_memsize       (PicmanObject          *object,
                                                      gint64              *gui_size);

static void          picman_gradient_get_preview_size  (PicmanViewable        *viewable,
                                                      gint                 size,
                                                      gboolean             popup,
                                                      gboolean             dot_for_dot,
                                                      gint                *width,
                                                      gint                *height);
static gboolean      picman_gradient_get_popup_size    (PicmanViewable        *viewable,
                                                      gint                 width,
                                                      gint                 height,
                                                      gboolean             dot_for_dot,
                                                      gint                *popup_width,
                                                      gint                *popup_height);
static PicmanTempBuf * picman_gradient_get_new_preview   (PicmanViewable        *viewable,
                                                      PicmanContext         *context,
                                                      gint                 width,
                                                      gint                 height);
static const gchar * picman_gradient_get_extension     (PicmanData            *data);
static PicmanData    * picman_gradient_duplicate         (PicmanData            *data);

static gchar       * picman_gradient_get_checksum      (PicmanTagged          *tagged);

static PicmanGradientSegment *
              picman_gradient_get_segment_at_internal  (PicmanGradient        *gradient,
                                                      PicmanGradientSegment *seg,
                                                      gdouble              pos);


static inline gdouble  picman_gradient_calc_linear_factor            (gdouble  middle,
                                                                    gdouble  pos);
static inline gdouble  picman_gradient_calc_curved_factor            (gdouble  middle,
                                                                    gdouble  pos);
static inline gdouble  picman_gradient_calc_sine_factor              (gdouble  middle,
                                                                    gdouble  pos);
static inline gdouble  picman_gradient_calc_sphere_increasing_factor (gdouble  middle,
                                                                    gdouble  pos);
static inline gdouble  picman_gradient_calc_sphere_decreasing_factor (gdouble  middle,
                                                                    gdouble  pos);


G_DEFINE_TYPE_WITH_CODE (PicmanGradient, picman_gradient, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_TAGGED,
                                                picman_gradient_tagged_iface_init))

#define parent_class picman_gradient_parent_class


static void
picman_gradient_class_init (PicmanGradientClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanDataClass     *data_class        = PICMAN_DATA_CLASS (klass);

  object_class->finalize           = picman_gradient_finalize;

  picman_object_class->get_memsize   = picman_gradient_get_memsize;

  viewable_class->default_stock_id = "picman-gradient";
  viewable_class->get_preview_size = picman_gradient_get_preview_size;
  viewable_class->get_popup_size   = picman_gradient_get_popup_size;
  viewable_class->get_new_preview  = picman_gradient_get_new_preview;

  data_class->save                 = picman_gradient_save;
  data_class->get_extension        = picman_gradient_get_extension;
  data_class->duplicate            = picman_gradient_duplicate;
}

static void
picman_gradient_tagged_iface_init (PicmanTaggedInterface *iface)
{
  iface->get_checksum = picman_gradient_get_checksum;
}

static void
picman_gradient_init (PicmanGradient *gradient)
{
  gradient->segments = NULL;
}

static void
picman_gradient_finalize (GObject *object)
{
  PicmanGradient *gradient = PICMAN_GRADIENT (object);

  if (gradient->segments)
    {
      picman_gradient_segments_free (gradient->segments);
      gradient->segments = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_gradient_get_memsize (PicmanObject *object,
                           gint64     *gui_size)
{
  PicmanGradient        *gradient = PICMAN_GRADIENT (object);
  PicmanGradientSegment *segment;
  gint64               memsize  = 0;

  for (segment = gradient->segments; segment; segment = segment->next)
    memsize += sizeof (PicmanGradientSegment);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_gradient_get_preview_size (PicmanViewable *viewable,
                                gint          size,
                                gboolean      popup,
                                gboolean      dot_for_dot,
                                gint         *width,
                                gint         *height)
{
  *width  = size;
  *height = 1 + size / 2;
}

static gboolean
picman_gradient_get_popup_size (PicmanViewable *viewable,
                              gint          width,
                              gint          height,
                              gboolean      dot_for_dot,
                              gint         *popup_width,
                              gint         *popup_height)
{
  if (width < 128 || height < 32)
    {
      *popup_width  = 128;
      *popup_height =  32;

      return TRUE;
    }

  return FALSE;
}

static PicmanTempBuf *
picman_gradient_get_new_preview (PicmanViewable *viewable,
                               PicmanContext  *context,
                               gint          width,
                               gint          height)
{
  PicmanGradient        *gradient = PICMAN_GRADIENT (viewable);
  PicmanGradientSegment *seg      = NULL;
  PicmanTempBuf         *temp_buf;
  guchar              *buf;
  guchar              *p;
  guchar              *row;
  gint                 x, y;
  gdouble              dx, cur_x;
  PicmanRGB              color;

  dx    = 1.0 / (width - 1);
  cur_x = 0.0;
  p     = row = g_malloc (width * 4);

  /* Create lines to fill the image */

  for (x = 0; x < width; x++)
    {
      seg = picman_gradient_get_color_at (gradient, context, seg, cur_x,
                                        FALSE, &color);

      *p++ = ROUND (color.r * 255.0);
      *p++ = ROUND (color.g * 255.0);
      *p++ = ROUND (color.b * 255.0);
      *p++ = ROUND (color.a * 255.0);

      cur_x += dx;
    }

  temp_buf = picman_temp_buf_new (width, height, babl_format ("R'G'B'A u8"));

  buf = picman_temp_buf_get_data (temp_buf);

  for (y = 0; y < height; y++)
    memcpy (buf + (width * y * 4), row, width * 4);

  g_free (row);

  return temp_buf;
}

static PicmanData *
picman_gradient_duplicate (PicmanData *data)
{
  PicmanGradient        *gradient;
  PicmanGradientSegment *head, *prev, *cur, *orig;

  gradient = g_object_new (PICMAN_TYPE_GRADIENT, NULL);

  prev = NULL;
  orig = PICMAN_GRADIENT (data)->segments;
  head = NULL;

  while (orig)
    {
      cur = picman_gradient_segment_new ();

      *cur = *orig;  /* Copy everything */

      cur->prev = prev;
      cur->next = NULL;

      if (prev)
        prev->next = cur;
      else
        head = cur;  /* Remember head */

      prev = cur;
      orig = orig->next;
    }

  gradient->segments = head;

  return PICMAN_DATA (gradient);
}

static gchar *
picman_gradient_get_checksum (PicmanTagged *tagged)
{
  PicmanGradient *gradient        = PICMAN_GRADIENT (tagged);
  gchar        *checksum_string = NULL;

  if (gradient->segments)
    {
      GChecksum           *checksum = g_checksum_new (G_CHECKSUM_MD5);
      PicmanGradientSegment *segment  = gradient->segments;

      while (segment)
        {
          g_checksum_update (checksum,
                             (const guchar *) &segment->left,
                             sizeof (segment->left));
          g_checksum_update (checksum,
                             (const guchar *) &segment->middle,
                             sizeof (segment->middle));
          g_checksum_update (checksum,
                             (const guchar *) &segment->right,
                             sizeof (segment->right));
          g_checksum_update (checksum,
                             (const guchar *) &segment->left_color_type,
                             sizeof (segment->left_color_type));
          g_checksum_update (checksum,
                             (const guchar *) &segment->left_color,
                             sizeof (segment->left_color));
          g_checksum_update (checksum,
                             (const guchar *) &segment->right_color_type,
                             sizeof (segment->right_color_type));
          g_checksum_update (checksum,
                             (const guchar *) &segment->right_color,
                             sizeof (segment->right_color));
          g_checksum_update (checksum,
                             (const guchar *) &segment->type,
                             sizeof (segment->type));
          g_checksum_update (checksum,
                             (const guchar *) &segment->color,
                             sizeof (segment->color));

          segment = segment->next;
        }

      checksum_string = g_strdup (g_checksum_get_string (checksum));

      g_checksum_free (checksum);
    }

  return checksum_string;
}


/*  public functions  */

PicmanData *
picman_gradient_new (PicmanContext *context,
                   const gchar *name)
{
  PicmanGradient *gradient;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (*name != '\0', NULL);

  gradient = g_object_new (PICMAN_TYPE_GRADIENT,
                           "name", name,
                           NULL);

  gradient->segments = picman_gradient_segment_new ();

  return PICMAN_DATA (gradient);
}

PicmanData *
picman_gradient_get_standard (PicmanContext *context)
{
  static PicmanData *standard_gradient = NULL;

  if (! standard_gradient)
    {
      standard_gradient = picman_gradient_new (context, "Standard");

      picman_data_clean (standard_gradient);
      picman_data_make_internal (standard_gradient, "picman-gradient-standard");

      g_object_add_weak_pointer (G_OBJECT (standard_gradient),
                                 (gpointer *) &standard_gradient);
    }

  return standard_gradient;
}

static const gchar *
picman_gradient_get_extension (PicmanData *data)
{
  return PICMAN_GRADIENT_FILE_EXTENSION;
}

/**
 * picman_gradient_get_color_at:
 * @gradient: a gradient
 * @context:  a context
 * @seg:      a segment to seed the search with (or %NULL)
 * @pos:      position in the gradient (between 0.0 and 1.0)
 * @reverse:  when %TRUE, use the reversed gradient
 * @color:    returns the color
 *
 * If you are iterating over an gradient, you should pass the the
 * return value from the last call for @seg.
 *
 * Return value: the gradient segment the color is from
 **/
PicmanGradientSegment *
picman_gradient_get_color_at (PicmanGradient        *gradient,
                            PicmanContext         *context,
                            PicmanGradientSegment *seg,
                            gdouble              pos,
                            gboolean             reverse,
                            PicmanRGB             *color)
{
  gdouble  factor = 0.0;
  gdouble  seg_len;
  gdouble  middle;
  PicmanRGB  left_color;
  PicmanRGB  right_color;
  PicmanRGB  rgb;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (color != NULL, NULL);

  pos = CLAMP (pos, 0.0, 1.0);

  if (reverse)
    pos = 1.0 - pos;

  seg = picman_gradient_get_segment_at_internal (gradient, seg, pos);

  seg_len = seg->right - seg->left;

  if (seg_len < EPSILON)
    {
      middle = 0.5;
      pos    = 0.5;
    }
  else
    {
      middle = (seg->middle - seg->left) / seg_len;
      pos    = (pos - seg->left) / seg_len;
    }

  switch (seg->type)
    {
    case PICMAN_GRADIENT_SEGMENT_LINEAR:
      factor = picman_gradient_calc_linear_factor (middle, pos);
      break;

    case PICMAN_GRADIENT_SEGMENT_CURVED:
      factor = picman_gradient_calc_curved_factor (middle, pos);
      break;

    case PICMAN_GRADIENT_SEGMENT_SINE:
      factor = picman_gradient_calc_sine_factor (middle, pos);
      break;

    case PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING:
      factor = picman_gradient_calc_sphere_increasing_factor (middle, pos);
      break;

    case PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING:
      factor = picman_gradient_calc_sphere_decreasing_factor (middle, pos);
      break;

    default:
      g_warning ("%s: Unknown gradient type %d.", G_STRFUNC, seg->type);
      break;
    }

  /* Get left/right colors */

  switch (seg->left_color_type)
    {
    case PICMAN_GRADIENT_COLOR_FIXED:
      left_color = seg->left_color;
      break;

    case PICMAN_GRADIENT_COLOR_FOREGROUND:
    case PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT:
      picman_context_get_foreground (context, &left_color);

      if (seg->left_color_type == PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT)
        picman_rgb_set_alpha (&left_color, 0.0);
      break;

    case PICMAN_GRADIENT_COLOR_BACKGROUND:
    case PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT:
      picman_context_get_background (context, &left_color);

      if (seg->left_color_type == PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT)
        picman_rgb_set_alpha (&left_color, 0.0);
      break;
    }

  switch (seg->right_color_type)
    {
    case PICMAN_GRADIENT_COLOR_FIXED:
      right_color = seg->right_color;
      break;

    case PICMAN_GRADIENT_COLOR_FOREGROUND:
    case PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT:
      picman_context_get_foreground (context, &right_color);

      if (seg->right_color_type == PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT)
        picman_rgb_set_alpha (&right_color, 0.0);
      break;

    case PICMAN_GRADIENT_COLOR_BACKGROUND:
    case PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT:
      picman_context_get_background (context, &right_color);

      if (seg->right_color_type == PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT)
        picman_rgb_set_alpha (&right_color, 0.0);
      break;
    }

  /* Calculate color components */

  if (seg->color == PICMAN_GRADIENT_SEGMENT_RGB)
    {
      rgb.r = left_color.r + (right_color.r - left_color.r) * factor;
      rgb.g = left_color.g + (right_color.g - left_color.g) * factor;
      rgb.b = left_color.b + (right_color.b - left_color.b) * factor;
    }
  else
    {
      PicmanHSV left_hsv;
      PicmanHSV right_hsv;

      picman_rgb_to_hsv (&left_color,  &left_hsv);
      picman_rgb_to_hsv (&right_color, &right_hsv);

      left_hsv.s = left_hsv.s + (right_hsv.s - left_hsv.s) * factor;
      left_hsv.v = left_hsv.v + (right_hsv.v - left_hsv.v) * factor;

      switch (seg->color)
        {
        case PICMAN_GRADIENT_SEGMENT_HSV_CCW:
          if (left_hsv.h < right_hsv.h)
            {
              left_hsv.h += (right_hsv.h - left_hsv.h) * factor;
            }
          else
            {
              left_hsv.h += (1.0 - (left_hsv.h - right_hsv.h)) * factor;

              if (left_hsv.h > 1.0)
                left_hsv.h -= 1.0;
            }
          break;

        case PICMAN_GRADIENT_SEGMENT_HSV_CW:
          if (right_hsv.h < left_hsv.h)
            {
              left_hsv.h -= (left_hsv.h - right_hsv.h) * factor;
            }
          else
            {
              left_hsv.h -= (1.0 - (right_hsv.h - left_hsv.h)) * factor;

              if (left_hsv.h < 0.0)
                left_hsv.h += 1.0;
            }
          break;

        default:
          g_warning ("%s: Unknown coloring mode %d",
                     G_STRFUNC, (gint) seg->color);
          break;
        }

      picman_hsv_to_rgb (&left_hsv, &rgb);
    }

  /* Calculate alpha */

  rgb.a = left_color.a + (right_color.a - left_color.a) * factor;

  *color = rgb;

  return seg;
}

PicmanGradientSegment *
picman_gradient_get_segment_at (PicmanGradient *gradient,
                              gdouble       pos)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), NULL);

  return picman_gradient_get_segment_at_internal (gradient, NULL, pos);
}

gboolean
picman_gradient_has_fg_bg_segments (PicmanGradient *gradient)
{
  PicmanGradientSegment *segment;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), FALSE);

  for (segment = gradient->segments; segment; segment = segment->next)
    if (segment->left_color_type  != PICMAN_GRADIENT_COLOR_FIXED ||
        segment->right_color_type != PICMAN_GRADIENT_COLOR_FIXED)
      return TRUE;

  return FALSE;
}

PicmanGradient *
picman_gradient_flatten (PicmanGradient *gradient,
                       PicmanContext  *context)
{
  PicmanGradient        *flat;
  PicmanGradientSegment *seg;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  flat = PICMAN_GRADIENT (picman_data_duplicate (PICMAN_DATA (gradient)));

  for (seg = flat->segments; seg; seg = seg->next)
    {
      switch (seg->left_color_type)
        {
        case PICMAN_GRADIENT_COLOR_FIXED:
          break;

        case PICMAN_GRADIENT_COLOR_FOREGROUND:
        case PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT:
          picman_context_get_foreground (context, &seg->left_color);

          if (seg->left_color_type == PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT)
            picman_rgb_set_alpha (&seg->left_color, 0.0);
          break;

        case PICMAN_GRADIENT_COLOR_BACKGROUND:
        case PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT:
          picman_context_get_background (context, &seg->left_color);

          if (seg->left_color_type == PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT)
            picman_rgb_set_alpha (&seg->left_color, 0.0);
          break;
        }

      seg->left_color_type = PICMAN_GRADIENT_COLOR_FIXED;

      switch (seg->right_color_type)
        {
        case PICMAN_GRADIENT_COLOR_FIXED:
          break;

        case PICMAN_GRADIENT_COLOR_FOREGROUND:
        case PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT:
          picman_context_get_foreground (context, &seg->right_color);

          if (seg->right_color_type == PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT)
            picman_rgb_set_alpha (&seg->right_color, 0.0);
          break;

        case PICMAN_GRADIENT_COLOR_BACKGROUND:
        case PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT:
          picman_context_get_background (context, &seg->right_color);

          if (seg->right_color_type == PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT)
            picman_rgb_set_alpha (&seg->right_color, 0.0);
          break;
        }

      seg->right_color_type = PICMAN_GRADIENT_COLOR_FIXED;
    }

  return flat;
}


/*  gradient segment functions  */

PicmanGradientSegment *
picman_gradient_segment_new (void)
{
  PicmanGradientSegment *seg;

  seg = g_slice_new (PicmanGradientSegment);

  seg->left   = 0.0;
  seg->middle = 0.5;
  seg->right  = 1.0;

  seg->left_color_type = PICMAN_GRADIENT_COLOR_FIXED;
  picman_rgba_set (&seg->left_color,  0.0, 0.0, 0.0, 1.0);

  seg->right_color_type = PICMAN_GRADIENT_COLOR_FIXED;
  picman_rgba_set (&seg->right_color, 1.0, 1.0, 1.0, 1.0);

  seg->type  = PICMAN_GRADIENT_SEGMENT_LINEAR;
  seg->color = PICMAN_GRADIENT_SEGMENT_RGB;

  seg->prev = seg->next = NULL;

  return seg;
}


void
picman_gradient_segment_free (PicmanGradientSegment *seg)
{
  g_return_if_fail (seg != NULL);

  g_slice_free (PicmanGradientSegment, seg);
}

void
picman_gradient_segments_free (PicmanGradientSegment *seg)
{
  g_return_if_fail (seg != NULL);

  g_slice_free_chain (PicmanGradientSegment, seg, next);
}

PicmanGradientSegment *
picman_gradient_segment_get_last (PicmanGradientSegment *seg)
{
  if (! seg)
    return NULL;

  while (seg->next)
    seg = seg->next;

  return seg;
}

PicmanGradientSegment *
picman_gradient_segment_get_first (PicmanGradientSegment *seg)
{
  if (! seg)
    return NULL;

  while (seg->prev)
    seg = seg->prev;

  return seg;
}

PicmanGradientSegment *
picman_gradient_segment_get_nth (PicmanGradientSegment *seg,
                               gint                 index)
{
  gint i = 0;

  g_return_val_if_fail (index >= 0, NULL);

  if (! seg)
    return NULL;

  while (seg && (i < index))
    {
      seg = seg->next;
      i++;
    }

  if (i == index)
    return seg;

  return NULL;
}

void
picman_gradient_segment_split_midpoint (PicmanGradient         *gradient,
                                      PicmanContext          *context,
                                      PicmanGradientSegment  *lseg,
                                      PicmanGradientSegment **newl,
                                      PicmanGradientSegment **newr)
{
  PicmanRGB              color;
  PicmanGradientSegment *newseg;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (lseg != NULL);
  g_return_if_fail (newl != NULL);
  g_return_if_fail (newr != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  /* Get color at original segment's midpoint */
  picman_gradient_get_color_at (gradient, context, lseg, lseg->middle,
                              FALSE, &color);

  /* Create a new segment and insert it in the list */

  newseg = picman_gradient_segment_new ();

  newseg->prev = lseg;
  newseg->next = lseg->next;

  lseg->next = newseg;

  if (newseg->next)
    newseg->next->prev = newseg;

  /* Set coordinates of new segment */

  newseg->left   = lseg->middle;
  newseg->right  = lseg->right;
  newseg->middle = (newseg->left + newseg->right) / 2.0;

  /* Set coordinates of original segment */

  lseg->right  = newseg->left;
  lseg->middle = (lseg->left + lseg->right) / 2.0;

  /* Set colors of both segments */

  newseg->right_color_type = lseg->right_color_type;
  newseg->right_color      = lseg->right_color;

  lseg->right_color_type = newseg->left_color_type = PICMAN_GRADIENT_COLOR_FIXED;
  lseg->right_color      = newseg->left_color      = color;

  /* Set parameters of new segment */

  newseg->type  = lseg->type;
  newseg->color = lseg->color;

  /* Done */

  *newl = lseg;
  *newr = newseg;

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_split_uniform (PicmanGradient         *gradient,
                                     PicmanContext          *context,
                                     PicmanGradientSegment  *lseg,
                                     gint                  parts,
                                     PicmanGradientSegment **newl,
                                     PicmanGradientSegment **newr)
{
  PicmanGradientSegment *seg, *prev, *tmp;
  gdouble              seg_len;
  gint                 i;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (lseg != NULL);
  g_return_if_fail (newl != NULL);
  g_return_if_fail (newr != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  seg_len = (lseg->right - lseg->left) / parts; /* Length of divisions */

  seg  = NULL;
  prev = NULL;
  tmp  = NULL;

  for (i = 0; i < parts; i++)
    {
      seg = picman_gradient_segment_new ();

      if (i == 0)
        tmp = seg; /* Remember first segment */

      seg->left   = lseg->left + i * seg_len;
      seg->right  = lseg->left + (i + 1) * seg_len;
      seg->middle = (seg->left + seg->right) / 2.0;

      seg->left_color_type  = PICMAN_GRADIENT_COLOR_FIXED;
      seg->right_color_type = PICMAN_GRADIENT_COLOR_FIXED;

      picman_gradient_get_color_at (gradient, context, lseg,
                                  seg->left,  FALSE, &seg->left_color);
      picman_gradient_get_color_at (gradient, context, lseg,
                                  seg->right, FALSE, &seg->right_color);

      seg->type  = lseg->type;
      seg->color = lseg->color;

      seg->prev = prev;
      seg->next = NULL;

      if (prev)
        prev->next = seg;

      prev = seg;
    }

  /* Fix edges */

  tmp->left_color_type = lseg->left_color_type;
  tmp->left_color      = lseg->left_color;

  seg->right_color_type = lseg->right_color_type;
  seg->right_color      = lseg->right_color;

  tmp->left  = lseg->left;
  seg->right = lseg->right; /* To squish accumulative error */

  /* Link in list */

  tmp->prev = lseg->prev;
  seg->next = lseg->next;

  if (lseg->prev)
    lseg->prev->next = tmp;
  else
    gradient->segments = tmp; /* We are on leftmost segment */

  if (lseg->next)
    lseg->next->prev = seg;

  /* Done */
  *newl = tmp;
  *newr = seg;

  /* Delete old segment */
  picman_gradient_segment_free (lseg);

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_get_left_color (PicmanGradient        *gradient,
                                      PicmanGradientSegment *seg,
                                      PicmanRGB             *color)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);
  g_return_if_fail (color != NULL);

  *color = seg->left_color;
}

void
picman_gradient_segment_set_left_color (PicmanGradient        *gradient,
                                      PicmanGradientSegment *seg,
                                      const PicmanRGB       *color)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);
  g_return_if_fail (color != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  picman_gradient_segment_range_blend (gradient, seg, seg,
                                     color, &seg->right_color,
                                     TRUE, TRUE);

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_get_right_color (PicmanGradient        *gradient,
                                       PicmanGradientSegment *seg,
                                       PicmanRGB             *color)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);
  g_return_if_fail (color != NULL);

  *color = seg->right_color;
}

void
picman_gradient_segment_set_right_color (PicmanGradient        *gradient,
                                       PicmanGradientSegment *seg,
                                       const PicmanRGB       *color)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);
  g_return_if_fail (color != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  picman_gradient_segment_range_blend (gradient, seg, seg,
                                     &seg->left_color, color,
                                     TRUE, TRUE);

  picman_data_thaw (PICMAN_DATA (gradient));
}

PicmanGradientColor
picman_gradient_segment_get_left_color_type (PicmanGradient        *gradient,
                                           PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0);
  g_return_val_if_fail (seg != NULL, 0);

  return seg->left_color_type;
}

void
picman_gradient_segment_set_left_color_type (PicmanGradient        *gradient,
                                           PicmanGradientSegment *seg,
                                           PicmanGradientColor    color_type)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  seg->left_color_type = color_type;

  picman_data_thaw (PICMAN_DATA (gradient));
}

PicmanGradientColor
picman_gradient_segment_get_right_color_type (PicmanGradient        *gradient,
                                            PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0);
  g_return_val_if_fail (seg != NULL, 0);

  return seg->right_color_type;
}

void
picman_gradient_segment_set_right_color_type (PicmanGradient        *gradient,
                                            PicmanGradientSegment *seg,
                                            PicmanGradientColor    color_type)
{
  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (seg != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  seg->right_color_type = color_type;

  picman_data_thaw (PICMAN_DATA (gradient));
}

gdouble
picman_gradient_segment_get_left_pos (PicmanGradient        *gradient,
                                    PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  return seg->left;
}

gdouble
picman_gradient_segment_set_left_pos (PicmanGradient        *gradient,
                                    PicmanGradientSegment *seg,
                                    gdouble              pos)
{
  gdouble final_pos;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  if (seg->prev == NULL)
    {
      final_pos = 0;
    }
  else
    {
      picman_data_freeze (PICMAN_DATA (gradient));

      final_pos = seg->prev->right = seg->left =
          CLAMP (pos,
                 seg->prev->middle + EPSILON,
                 seg->middle - EPSILON);

      picman_data_thaw (PICMAN_DATA (gradient));
    }

  return final_pos;
}

gdouble
picman_gradient_segment_get_right_pos (PicmanGradient        *gradient,
                                     PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  return seg->right;
}

gdouble
picman_gradient_segment_set_right_pos (PicmanGradient        *gradient,
                                     PicmanGradientSegment *seg,
                                     gdouble              pos)
{
  gdouble final_pos;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  if (seg->next == NULL)
    {
      final_pos = 1.0;
    }
  else
    {
      picman_data_freeze (PICMAN_DATA (gradient));

      final_pos = seg->next->left = seg->right =
          CLAMP (pos,
                 seg->middle + EPSILON,
                 seg->next->middle - EPSILON);

      picman_data_thaw (PICMAN_DATA (gradient));
    }

  return final_pos;
}

gdouble
picman_gradient_segment_get_middle_pos (PicmanGradient        *gradient,
                                      PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  return seg->middle;
}

gdouble
picman_gradient_segment_set_middle_pos (PicmanGradient        *gradient,
                                      PicmanGradientSegment *seg,
                                      gdouble              pos)
{
  gdouble final_pos;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);
  g_return_val_if_fail (seg != NULL, 0.0);

  picman_data_freeze (PICMAN_DATA (gradient));

  final_pos = seg->middle =
      CLAMP (pos,
             seg->left + EPSILON,
             seg->right - EPSILON);

  picman_data_thaw (PICMAN_DATA (gradient));

  return final_pos;
}

PicmanGradientSegmentType
picman_gradient_segment_get_blending_function (PicmanGradient        *gradient,
                                             PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0);

  return seg->type;
}

PicmanGradientSegmentColor
picman_gradient_segment_get_coloring_type (PicmanGradient        *gradient,
                                         PicmanGradientSegment *seg)
{
  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0);

  return seg->color;
}

void
picman_gradient_segment_range_compress (PicmanGradient        *gradient,
                                      PicmanGradientSegment *range_l,
                                      PicmanGradientSegment *range_r,
                                      gdouble              new_l,
                                      gdouble              new_r)
{
  gdouble              orig_l, orig_r;
  gdouble              scale;
  PicmanGradientSegment *seg, *aseg;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (range_l != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! range_r)
    range_r = picman_gradient_segment_get_last (range_l);

  orig_l = range_l->left;
  orig_r = range_r->right;

  scale = (new_r - new_l) / (orig_r - orig_l);

  seg = range_l;

  do
    {
      if (seg->prev)
        seg->left   = new_l + (seg->left - orig_l) * scale;
      seg->middle = new_l + (seg->middle - orig_l) * scale;
      if (seg->next)
        seg->right  = new_l + (seg->right - orig_l) * scale;

      /* Next */

      aseg = seg;
      seg  = seg->next;
    }
  while (aseg != range_r);

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_blend (PicmanGradient        *gradient,
                                   PicmanGradientSegment *lseg,
                                   PicmanGradientSegment *rseg,
                                   const PicmanRGB       *rgb1,
                                   const PicmanRGB       *rgb2,
                                   gboolean             blend_colors,
                                   gboolean             blend_opacity)
{
  PicmanRGB              d;
  gdouble              left, len;
  PicmanGradientSegment *seg;
  PicmanGradientSegment *aseg;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (lseg != NULL);

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! rseg)
    rseg = picman_gradient_segment_get_last (lseg);

  d.r = rgb2->r - rgb1->r;
  d.g = rgb2->g - rgb1->g;
  d.b = rgb2->b - rgb1->b;
  d.a = rgb2->a - rgb1->a;

  left  = lseg->left;
  len   = rseg->right - left;

  seg = lseg;

  do
    {
      if (blend_colors)
        {
          seg->left_color.r  = rgb1->r + (seg->left - left) / len * d.r;
          seg->left_color.g  = rgb1->g + (seg->left - left) / len * d.g;
          seg->left_color.b  = rgb1->b + (seg->left - left) / len * d.b;

          seg->right_color.r = rgb1->r + (seg->right - left) / len * d.r;
          seg->right_color.g = rgb1->g + (seg->right - left) / len * d.g;
          seg->right_color.b = rgb1->b + (seg->right - left) / len * d.b;
        }

      if (blend_opacity)
        {
          seg->left_color.a  = rgb1->a + (seg->left - left) / len * d.a;
          seg->right_color.a = rgb1->a + (seg->right - left) / len * d.a;
        }

      aseg = seg;
      seg = seg->next;
    }
  while (aseg != rseg);
  picman_data_thaw (PICMAN_DATA (gradient));

}

void
picman_gradient_segment_range_set_blending_function (PicmanGradient            *gradient,
                                                   PicmanGradientSegment     *start_seg,
                                                   PicmanGradientSegment     *end_seg,
                                                   PicmanGradientSegmentType  new_type)
{
  PicmanGradientSegment *seg;
  gboolean             reached_last_segment = FALSE;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  picman_data_freeze (PICMAN_DATA (gradient));

  seg = start_seg;
  while (seg && ! reached_last_segment)
    {
      if (seg == end_seg)
        reached_last_segment = TRUE;

      seg->type = new_type;
      seg = seg->next;
    }

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_set_coloring_type (PicmanGradient             *gradient,
                                               PicmanGradientSegment      *start_seg,
                                               PicmanGradientSegment      *end_seg,
                                               PicmanGradientSegmentColor  new_color)
{
  PicmanGradientSegment *seg;
  gboolean             reached_last_segment = FALSE;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  picman_data_freeze (PICMAN_DATA (gradient));

  seg = start_seg;
  while (seg && ! reached_last_segment)
    {
      if (seg == end_seg)
        reached_last_segment = TRUE;

      seg->color = new_color;
      seg = seg->next;
    }

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_flip (PicmanGradient         *gradient,
                                  PicmanGradientSegment  *start_seg,
                                  PicmanGradientSegment  *end_seg,
                                  PicmanGradientSegment **final_start_seg,
                                  PicmanGradientSegment **final_end_seg)
{
  PicmanGradientSegment *oseg, *oaseg;
  PicmanGradientSegment *seg, *prev, *tmp;
  PicmanGradientSegment *lseg, *rseg;
  gdouble              left, right;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  left  = start_seg->left;
  right = end_seg->right;

  /* Build flipped segments */

  prev = NULL;
  oseg = end_seg;
  tmp  = NULL;

  do
    {
      seg = picman_gradient_segment_new ();

      if (prev == NULL)
        {
          seg->left = left;
          tmp = seg; /* Remember first segment */
        }
      else
        seg->left = left + right - oseg->right;

      seg->middle = left + right - oseg->middle;
      seg->right  = left + right - oseg->left;

      seg->left_color_type = oseg->right_color_type;
      seg->left_color      = oseg->right_color;

      seg->right_color_type = oseg->left_color_type;
      seg->right_color      = oseg->left_color;

      switch (oseg->type)
        {
        case PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING:
          seg->type = PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING;
          break;

        case PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING:
          seg->type = PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING;
          break;

        default:
          seg->type = oseg->type;
        }

      switch (oseg->color)
        {
        case PICMAN_GRADIENT_SEGMENT_HSV_CCW:
          seg->color = PICMAN_GRADIENT_SEGMENT_HSV_CW;
          break;

        case PICMAN_GRADIENT_SEGMENT_HSV_CW:
          seg->color = PICMAN_GRADIENT_SEGMENT_HSV_CCW;
          break;

        default:
          seg->color = oseg->color;
        }

      seg->prev = prev;
      seg->next = NULL;

      if (prev)
        prev->next = seg;

      prev = seg;

      oaseg = oseg;
      oseg  = oseg->prev; /* Move backwards! */
    }
  while (oaseg != start_seg);

  seg->right = right; /* Squish accumulative error */

  /* Free old segments */

  lseg = start_seg->prev;
  rseg = end_seg->next;

  oseg = start_seg;

  do
    {
      oaseg = oseg->next;
      picman_gradient_segment_free (oseg);
      oseg = oaseg;
    }
  while (oaseg != rseg);

  /* Link in new segments */

  if (lseg)
    lseg->next = tmp;
  else
    gradient->segments = tmp;

  tmp->prev = lseg;

  seg->next = rseg;

  if (rseg)
    rseg->prev = seg;

  /* Reset selection */

  if (final_start_seg)
    *final_start_seg = tmp;

  if (final_end_seg)
    *final_end_seg = seg;

  /* Done */

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_replicate (PicmanGradient         *gradient,
                                       PicmanGradientSegment  *start_seg,
                                       PicmanGradientSegment  *end_seg,
                                       gint                  replicate_times,
                                       PicmanGradientSegment **final_start_seg,
                                       PicmanGradientSegment **final_end_seg)
{
  gdouble              sel_left, sel_right, sel_len;
  gdouble              new_left;
  gdouble              factor;
  PicmanGradientSegment *prev, *seg, *tmp;
  PicmanGradientSegment *oseg, *oaseg;
  PicmanGradientSegment *lseg, *rseg;
  gint                 i;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  if (replicate_times < 2)
    {
      *final_start_seg = start_seg;
      *final_end_seg   = end_seg;
      return;
    }

  picman_data_freeze (PICMAN_DATA (gradient));

  /* Remember original parameters */
  sel_left  = start_seg->left;
  sel_right = end_seg->right;
  sel_len   = sel_right - sel_left;

  factor = 1.0 / replicate_times;

  /* Build replicated segments */

  prev = NULL;
  seg  = NULL;
  tmp  = NULL;

  for (i = 0; i < replicate_times; i++)
    {
      /* Build one cycle */

      new_left  = sel_left + i * factor * sel_len;

      oseg = start_seg;

      do
        {
          seg = picman_gradient_segment_new ();

          if (prev == NULL)
            {
              seg->left = sel_left;
              tmp = seg; /* Remember first segment */
            }
          else
            {
              seg->left = new_left + factor * (oseg->left - sel_left);
            }

          seg->middle = new_left + factor * (oseg->middle - sel_left);
          seg->right  = new_left + factor * (oseg->right - sel_left);

          seg->left_color_type = oseg->left_color_type;
          seg->left_color      = oseg->left_color;

          seg->right_color_type = oseg->right_color_type;
          seg->right_color      = oseg->right_color;

          seg->type  = oseg->type;
          seg->color = oseg->color;

          seg->prev = prev;
          seg->next = NULL;

          if (prev)
            prev->next = seg;

          prev = seg;

          oaseg = oseg;
          oseg  = oseg->next;
        }
      while (oaseg != end_seg);
    }

  seg->right = sel_right; /* Squish accumulative error */

  /* Free old segments */

  lseg = start_seg->prev;
  rseg = end_seg->next;

  oseg = start_seg;

  do
    {
      oaseg = oseg->next;
      picman_gradient_segment_free (oseg);
      oseg = oaseg;
    }
  while (oaseg != rseg);

  /* Link in new segments */

  if (lseg)
    lseg->next = tmp;
  else
    gradient->segments = tmp;

  tmp->prev = lseg;

  seg->next = rseg;

  if (rseg)
    rseg->prev = seg;

  /* Reset selection */

  if (final_start_seg)
    *final_start_seg = tmp;

  if (final_end_seg)
    *final_end_seg = seg;

  /* Done */

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_split_midpoint (PicmanGradient         *gradient,
                                            PicmanContext          *context,
                                            PicmanGradientSegment  *start_seg,
                                            PicmanGradientSegment  *end_seg,
                                            PicmanGradientSegment **final_start_seg,
                                            PicmanGradientSegment **final_end_seg)
{
  PicmanGradientSegment *seg, *lseg, *rseg;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  seg = start_seg;

  do
    {
      picman_gradient_segment_split_midpoint (gradient, context,
                                            seg, &lseg, &rseg);
      seg = rseg->next;
    }
  while (lseg != end_seg);

  if (final_start_seg)
    *final_start_seg = start_seg;

  if (final_end_seg)
    *final_end_seg = rseg;

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_split_uniform (PicmanGradient         *gradient,
                                           PicmanContext          *context,
                                           PicmanGradientSegment  *start_seg,
                                           PicmanGradientSegment  *end_seg,
                                           gint                  parts,
                                           PicmanGradientSegment **final_start_seg,
                                           PicmanGradientSegment **final_end_seg)
{
  PicmanGradientSegment *seg, *aseg, *lseg, *rseg, *lsel;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  if (parts < 2)
    {
      *final_start_seg = start_seg;
      *final_end_seg   = end_seg;
      return;
    }

  picman_data_freeze (PICMAN_DATA (gradient));

  seg = start_seg;
  lsel = NULL;

  do
    {
      aseg = seg;

      picman_gradient_segment_split_uniform (gradient, context, seg,
                                           parts,
                                           &lseg, &rseg);

      if (seg == start_seg)
        lsel = lseg;

      seg = rseg->next;
    }
  while (aseg != end_seg);

  if (final_start_seg)
    *final_start_seg = lsel;

  if (final_end_seg)
    *final_end_seg = rseg;

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_delete (PicmanGradient         *gradient,
                                    PicmanGradientSegment  *start_seg,
                                    PicmanGradientSegment  *end_seg,
                                    PicmanGradientSegment **final_start_seg,
                                    PicmanGradientSegment **final_end_seg)
{
  PicmanGradientSegment *lseg, *rseg, *seg, *aseg, *next;
  gdouble              join;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  /* Remember segments to the left and to the right of the selection */

  lseg = start_seg->prev;
  rseg = end_seg->next;

  /* Cannot delete all the segments in the gradient */

  if ((lseg == NULL) && (rseg == NULL))
    goto premature_return;

  picman_data_freeze (PICMAN_DATA (gradient));

  /* Calculate join point */

  join = (start_seg->left +
          end_seg->right) / 2.0;

  if (lseg == NULL)
    join = 0.0;
  else if (rseg == NULL)
    join = 1.0;

  /* Move segments */

  if (lseg != NULL)
    picman_gradient_segment_range_compress (gradient, lseg, lseg,
                                          lseg->left, join);

  if (rseg != NULL)
    picman_gradient_segment_range_compress (gradient, rseg, rseg,
                                          join, rseg->right);

  /* Link */

  if (lseg)
    lseg->next = rseg;

  if (rseg)
    rseg->prev = lseg;

  /* Delete old segments */

  seg = start_seg;

  do
    {
      next = seg->next;
      aseg = seg;

      picman_gradient_segment_free (seg);

      seg = next;
    }
  while (aseg != end_seg);

  /* Change selection */

  if (rseg)
    {
      if (final_start_seg)
        *final_start_seg = rseg;

      if (final_end_seg)
        *final_end_seg = rseg;
    }
  else
    {
      if (final_start_seg)
        *final_start_seg = lseg;

      if (final_end_seg)
        *final_end_seg = lseg;
    }

  if (lseg == NULL)
    gradient->segments = rseg;

  picman_data_thaw (PICMAN_DATA (gradient));

  return;

 premature_return:
  if (final_start_seg)
    *final_start_seg = start_seg;
  if (final_end_seg)
    *final_end_seg = end_seg;
}

void
picman_gradient_segment_range_recenter_handles (PicmanGradient        *gradient,
                                              PicmanGradientSegment *start_seg,
                                              PicmanGradientSegment *end_seg)
{
  PicmanGradientSegment *seg, *aseg;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  seg = start_seg;

  do
    {
      seg->middle = (seg->left + seg->right) / 2.0;

      aseg = seg;
      seg  = seg->next;
    }
  while (aseg != end_seg);

  picman_data_thaw (PICMAN_DATA (gradient));
}

void
picman_gradient_segment_range_redistribute_handles (PicmanGradient        *gradient,
                                                  PicmanGradientSegment *start_seg,
                                                  PicmanGradientSegment *end_seg)
{
  PicmanGradientSegment *seg, *aseg;
  gdouble              left, right, seg_len;
  gint                 num_segs;
  gint                 i;

  g_return_if_fail (PICMAN_IS_GRADIENT (gradient));

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! end_seg)
    end_seg = picman_gradient_segment_get_last (start_seg);

  /* Count number of segments in selection */

  num_segs = 0;
  seg      = start_seg;

  do
    {
      num_segs++;
      aseg = seg;
      seg  = seg->next;
    }
  while (aseg != end_seg);

  /* Calculate new segment length */

  left    = start_seg->left;
  right   = end_seg->right;
  seg_len = (right - left) / num_segs;

  /* Redistribute */

  seg = start_seg;

  for (i = 0; i < num_segs; i++)
    {
      seg->left   = left + i * seg_len;
      seg->right  = left + (i + 1) * seg_len;
      seg->middle = (seg->left + seg->right) / 2.0;

      seg = seg->next;
    }

  /* Fix endpoints to squish accumulative error */

  start_seg->left  = left;
  end_seg->right = right;

  picman_data_thaw (PICMAN_DATA (gradient));
}

gdouble
picman_gradient_segment_range_move (PicmanGradient        *gradient,
                                  PicmanGradientSegment *range_l,
                                  PicmanGradientSegment *range_r,
                                  gdouble              delta,
                                  gboolean             control_compress)
{
  gdouble              lbound, rbound;
  gint                 is_first, is_last;
  PicmanGradientSegment *seg, *aseg;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), 0.0);

  picman_data_freeze (PICMAN_DATA (gradient));

  if (! range_l)
    range_r = picman_gradient_segment_get_last (range_l);

  /* First or last segments in gradient? */

  is_first = (range_l->prev == NULL);
  is_last  = (range_r->next == NULL);

  /* Calculate drag bounds */

  if (! control_compress)
    {
      if (!is_first)
        lbound = range_l->prev->middle + EPSILON;
      else
        lbound = range_l->left + EPSILON;

      if (!is_last)
        rbound = range_r->next->middle - EPSILON;
      else
        rbound = range_r->right - EPSILON;
    }
  else
    {
      if (!is_first)
        lbound = range_l->prev->left + 2.0 * EPSILON;
      else
        lbound = range_l->left + EPSILON;

      if (!is_last)
        rbound = range_r->next->right - 2.0 * EPSILON;
      else
        rbound = range_r->right - EPSILON;
    }

  /* Fix the delta if necessary */

  if (delta < 0.0)
    {
      if (!is_first)
        {
          if (range_l->left + delta < lbound)
            delta = lbound - range_l->left;
        }
      else
        if (range_l->middle + delta < lbound)
          delta = lbound - range_l->middle;
    }
  else
    {
      if (!is_last)
        {
          if (range_r->right + delta > rbound)
            delta = rbound - range_r->right;
        }
      else
        if (range_r->middle + delta > rbound)
          delta = rbound - range_r->middle;
    }

  /* Move all the segments inside the range */

  seg = range_l;

  do
    {
      if (!((seg == range_l) && is_first))
        seg->left  += delta;

      seg->middle  += delta;

      if (!((seg == range_r) && is_last))
        seg->right += delta;

      /* Next */

      aseg = seg;
      seg  = seg->next;
    }
  while (aseg != range_r);

  /* Fix the segments that surround the range */

  if (!is_first)
    {
      if (! control_compress)
        range_l->prev->right = range_l->left;
      else
        picman_gradient_segment_range_compress (gradient,
                                              range_l->prev, range_l->prev,
                                              range_l->prev->left, range_l->left);
    }

  if (!is_last)
    {
      if (! control_compress)
        range_r->next->left = range_r->right;
      else
        picman_gradient_segment_range_compress (gradient,
                                              range_r->next, range_r->next,
                                              range_r->right, range_r->next->right);
    }

  picman_data_thaw (PICMAN_DATA (gradient));

  return delta;
}


/*  private functions  */

static PicmanGradientSegment *
picman_gradient_get_segment_at_internal (PicmanGradient        *gradient,
                                       PicmanGradientSegment *seg,
                                       gdouble              pos)
{
  /* handle FP imprecision at the edges of the gradient */
  pos = CLAMP (pos, 0.0, 1.0);

  if (! seg)
    seg = gradient->segments;

  while (seg)
    {
      if (pos >= seg->left)
        {
          if (pos <= seg->right)
            {
              return seg;
            }
          else
            {
              seg = seg->next;
            }
        }
      else
        {
          seg = seg->prev;
        }
    }

  /* Oops: we should have found a segment, but we didn't */
  g_warning ("%s: no matching segment for position %0.15f", G_STRFUNC, pos);

  return NULL;
}

static inline gdouble
picman_gradient_calc_linear_factor (gdouble middle,
                                  gdouble pos)
{
  if (pos <= middle)
    {
      if (middle < EPSILON)
        return 0.0;
      else
        return 0.5 * pos / middle;
    }
  else
    {
      pos -= middle;
      middle = 1.0 - middle;

      if (middle < EPSILON)
        return 1.0;
      else
        return 0.5 + 0.5 * pos / middle;
    }
}

static inline gdouble
picman_gradient_calc_curved_factor (gdouble middle,
                                  gdouble pos)
{
  if (middle < EPSILON)
    middle = EPSILON;

  return pow (pos, log (0.5) / log (middle));
}

static inline gdouble
picman_gradient_calc_sine_factor (gdouble middle,
                                gdouble pos)
{
  pos = picman_gradient_calc_linear_factor (middle, pos);

  return (sin ((-G_PI / 2.0) + G_PI * pos) + 1.0) / 2.0;
}

static inline gdouble
picman_gradient_calc_sphere_increasing_factor (gdouble middle,
                                             gdouble pos)
{
  pos = picman_gradient_calc_linear_factor (middle, pos) - 1.0;

  /* Works for convex increasing and concave decreasing */
  return sqrt (1.0 - pos * pos);
}

static inline gdouble
picman_gradient_calc_sphere_decreasing_factor (gdouble middle,
                                             gdouble pos)
{
  pos = picman_gradient_calc_linear_factor (middle, pos);

  /* Works for convex decreasing and concave increasing */
  return 1.0 - sqrt(1.0 - pos * pos);
}
