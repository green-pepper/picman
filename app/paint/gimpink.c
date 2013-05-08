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

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "gegl/picman-gegl-utils.h"

#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmantempbuf.h"

#include "picmaninkoptions.h"
#include "picmanink.h"
#include "picmanink-blob.h"
#include "picmaninkundo.h"

#include "picman-intl.h"


#define SUBSAMPLE 8


/*  local function prototypes  */

static void         picman_ink_finalize         (GObject          *object);

static void         picman_ink_paint            (PicmanPaintCore    *paint_core,
                                               PicmanDrawable     *drawable,
                                               PicmanPaintOptions *paint_options,
                                               const PicmanCoords *coords,
                                               PicmanPaintState    paint_state,
                                               guint32           time);
static GeglBuffer * picman_ink_get_paint_buffer (PicmanPaintCore    *paint_core,
                                               PicmanDrawable     *drawable,
                                               PicmanPaintOptions *paint_options,
                                               const PicmanCoords *coords,
                                               gint             *paint_buffer_x,
                                               gint             *paint_buffer_y);
static PicmanUndo   * picman_ink_push_undo        (PicmanPaintCore    *core,
                                               PicmanImage        *image,
                                               const gchar      *undo_desc);

static void         picman_ink_motion           (PicmanPaintCore    *paint_core,
                                               PicmanDrawable     *drawable,
                                               PicmanPaintOptions *paint_options,
                                               const PicmanCoords *coords,
                                               guint32           time);

static PicmanBlob   * ink_pen_ellipse           (PicmanInkOptions   *options,
                                               gdouble           x_center,
                                               gdouble           y_center,
                                               gdouble           pressure,
                                               gdouble           xtilt,
                                               gdouble           ytilt,
                                               gdouble           velocity);

static void         render_blob               (GeglBuffer       *buffer,
                                               GeglRectangle    *rect,
                                               PicmanBlob         *blob);


G_DEFINE_TYPE (PicmanInk, picman_ink, PICMAN_TYPE_PAINT_CORE)

#define parent_class picman_ink_parent_class


void
picman_ink_register (Picman                      *picman,
                   PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_INK,
                PICMAN_TYPE_INK_OPTIONS,
                "picman-ink",
                _("Ink"),
                "picman-tool-ink");
}

static void
picman_ink_class_init (PicmanInkClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);

  object_class->finalize             = picman_ink_finalize;

  paint_core_class->paint            = picman_ink_paint;
  paint_core_class->get_paint_buffer = picman_ink_get_paint_buffer;
  paint_core_class->push_undo        = picman_ink_push_undo;
}

static void
picman_ink_init (PicmanInk *ink)
{
}

static void
picman_ink_finalize (GObject *object)
{
  PicmanInk *ink = PICMAN_INK (object);

  if (ink->start_blob)
    {
      g_free (ink->start_blob);
      ink->start_blob = NULL;
    }

  if (ink->last_blob)
    {
      g_free (ink->last_blob);
      ink->last_blob = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_ink_paint (PicmanPaintCore    *paint_core,
                PicmanDrawable     *drawable,
                PicmanPaintOptions *paint_options,
                const PicmanCoords *coords,
                PicmanPaintState    paint_state,
                guint32           time)
{
  PicmanInk *ink = PICMAN_INK (paint_core);
  PicmanCoords last_coords;

  picman_paint_core_get_last_coords (paint_core, &last_coords);

  switch (paint_state)
    {

    case PICMAN_PAINT_STATE_INIT:

      if (coords->x == last_coords.x &&
          coords->y == last_coords.y)
        {
          /*  start with new blobs if we're not interpolating  */

          if (ink->start_blob)
            {
              g_free (ink->start_blob);
              ink->start_blob = NULL;
            }

          if (ink->last_blob)
            {
              g_free (ink->last_blob);
              ink->last_blob = NULL;
            }
        }
      else if (ink->last_blob)
        {
          /*  save the start blob of the line for undo otherwise  */

          if (ink->start_blob)
            g_free (ink->start_blob);

          ink->start_blob = picman_blob_duplicate (ink->last_blob);
        }
      break;

    case PICMAN_PAINT_STATE_MOTION:
      picman_ink_motion (paint_core, drawable, paint_options, coords, time);
      break;

    case PICMAN_PAINT_STATE_FINISH:
      break;
    }
}

static GeglBuffer *
picman_ink_get_paint_buffer (PicmanPaintCore    *paint_core,
                           PicmanDrawable     *drawable,
                           PicmanPaintOptions *paint_options,
                           const PicmanCoords *coords,
                           gint             *paint_buffer_x,
                           gint             *paint_buffer_y)
{
  PicmanInk *ink = PICMAN_INK (paint_core);
  gint     x, y;
  gint     width, height;
  gint     dwidth, dheight;
  gint     x1, y1, x2, y2;

  picman_blob_bounds (ink->cur_blob, &x, &y, &width, &height);

  dwidth  = picman_item_get_width  (PICMAN_ITEM (drawable));
  dheight = picman_item_get_height (PICMAN_ITEM (drawable));

  x1 = CLAMP (x / SUBSAMPLE - 1,            0, dwidth);
  y1 = CLAMP (y / SUBSAMPLE - 1,            0, dheight);
  x2 = CLAMP ((x + width)  / SUBSAMPLE + 2, 0, dwidth);
  y2 = CLAMP ((y + height) / SUBSAMPLE + 2, 0, dheight);

  /*  configure the canvas buffer  */
  if ((x2 - x1) && (y2 - y1))
    {
      PicmanTempBuf *temp_buf;

      temp_buf = picman_temp_buf_new ((x2 - x1), (y2 - y1),
                                    babl_format ("RGBA float"));

      *paint_buffer_x = x1;
      *paint_buffer_y = y1;

      if (paint_core->paint_buffer)
        g_object_unref (paint_core->paint_buffer);

      paint_core->paint_buffer = picman_temp_buf_create_buffer (temp_buf);

      picman_temp_buf_unref (temp_buf);

      return paint_core->paint_buffer;
    }

  return NULL;
}

static PicmanUndo *
picman_ink_push_undo (PicmanPaintCore *core,
                    PicmanImage     *image,
                    const gchar   *undo_desc)
{
  return picman_image_undo_push (image, PICMAN_TYPE_INK_UNDO,
                               PICMAN_UNDO_INK, undo_desc,
                               0,
                               "paint-core", core,
                               NULL);
}

static void
picman_ink_motion (PicmanPaintCore    *paint_core,
                 PicmanDrawable     *drawable,
                 PicmanPaintOptions *paint_options,
                 const PicmanCoords *coords,
                 guint32           time)
{
  PicmanInk        *ink        = PICMAN_INK (paint_core);
  PicmanInkOptions *options    = PICMAN_INK_OPTIONS (paint_options);
  PicmanContext    *context    = PICMAN_CONTEXT (paint_options);
  PicmanBlob       *blob_union = NULL;
  PicmanBlob       *blob_to_render;
  GeglBuffer     *paint_buffer;
  gint            paint_buffer_x;
  gint            paint_buffer_y;
  PicmanRGB         foreground;
  GeglColor      *color;

  if (! ink->last_blob)
    {
      ink->last_blob = ink_pen_ellipse (options,
                                        coords->x,
                                        coords->y,
                                        coords->pressure,
                                        coords->xtilt,
                                        coords->ytilt,
                                        100);

      if (ink->start_blob)
        g_free (ink->start_blob);

      ink->start_blob = picman_blob_duplicate (ink->last_blob);

      blob_to_render = ink->last_blob;
    }
  else
    {
      PicmanBlob *blob = ink_pen_ellipse (options,
                                        coords->x,
                                        coords->y,
                                        coords->pressure,
                                        coords->xtilt,
                                        coords->ytilt,
                                        coords->velocity * 100);

      blob_union = picman_blob_convex_union (ink->last_blob, blob);

      g_free (ink->last_blob);
      ink->last_blob = blob;

      blob_to_render = blob_union;
    }

  /* Get the buffer */
  ink->cur_blob = blob_to_render;
  paint_buffer = picman_paint_core_get_paint_buffer (paint_core, drawable,
                                                   paint_options, coords,
                                                   &paint_buffer_x,
                                                   &paint_buffer_y);
  ink->cur_blob = NULL;

  if (! paint_buffer)
    return;

  picman_context_get_foreground (context, &foreground);
  color = picman_gegl_color_new (&foreground);

  gegl_buffer_set_color (paint_buffer, NULL, color);
  g_object_unref (color);

  /*  draw the blob directly to the canvas_buffer  */
  render_blob (paint_core->canvas_buffer,
               GEGL_RECTANGLE (paint_core->paint_buffer_x,
                               paint_core->paint_buffer_y,
                               gegl_buffer_get_width  (paint_core->paint_buffer),
                               gegl_buffer_get_height (paint_core->paint_buffer)),
               blob_to_render);

  /*  draw the paint_area using the just rendered canvas_buffer as mask */
  picman_paint_core_paste (paint_core,
                         paint_core->canvas_buffer,
                         GEGL_RECTANGLE (paint_core->paint_buffer_x,
                                         paint_core->paint_buffer_y,
                                         gegl_buffer_get_width  (paint_core->paint_buffer),
                                         gegl_buffer_get_height (paint_core->paint_buffer)),
                         drawable,
                         PICMAN_OPACITY_OPAQUE,
                         picman_context_get_opacity (context),
                         picman_context_get_paint_mode (context),
                         PICMAN_PAINT_CONSTANT);

  if (blob_union)
    g_free (blob_union);
}

static PicmanBlob *
ink_pen_ellipse (PicmanInkOptions *options,
                 gdouble         x_center,
                 gdouble         y_center,
                 gdouble         pressure,
                 gdouble         xtilt,
                 gdouble         ytilt,
                 gdouble         velocity)
{
  PicmanBlobFunc blob_function;
  gdouble      size;
  gdouble      tsin, tcos;
  gdouble      aspect, radmin;
  gdouble      x,y;
  gdouble      tscale;
  gdouble      tscale_c;
  gdouble      tscale_s;

  /* Adjust the size depending on pressure. */

  size = options->size * (1.0 + options->size_sensitivity *
                          (2.0 * pressure - 1.0));

  /* Adjust the size further depending on pointer velocity and
   * velocity-sensitivity.  These 'magic constants' are 'feels
   * natural' tigert-approved. --ADM
   */

  if (velocity < 3.0)
    velocity = 3.0;

#ifdef VERBOSE
  g_printerr ("%g (%g) -> ", size, velocity);
#endif

  size = (options->vel_sensitivity *
          ((4.5 * size) / (1.0 + options->vel_sensitivity * (2.0 * velocity)))
          + (1.0 - options->vel_sensitivity) * size);

#ifdef VERBOSE
  g_printerr ("%g\n", (gfloat) size);
#endif

  /* Clamp resulting size to sane limits */

  if (size > options->size * (1.0 + options->size_sensitivity))
    size = options->size * (1.0 + options->size_sensitivity);

  if (size * SUBSAMPLE < 1.0)
    size = 1.0 / SUBSAMPLE;

  /* Add brush angle/aspect to tilt vectorially */

  /* I'm not happy with the way the brush widget info is combined with
   * tilt info from the brush. My personal feeling is that
   * representing both as affine transforms would make the most
   * sense. -RLL
   */

  tscale   = options->tilt_sensitivity * 10.0;
  tscale_c = tscale * cos (picman_deg_to_rad (options->tilt_angle));
  tscale_s = tscale * sin (picman_deg_to_rad (options->tilt_angle));

  x = (options->blob_aspect * cos (options->blob_angle) +
       xtilt * tscale_c - ytilt * tscale_s);
  y = (options->blob_aspect * sin (options->blob_angle) +
       ytilt * tscale_c + xtilt * tscale_s);

#ifdef VERBOSE
  g_printerr ("angle %g aspect %g; %g %g; %g %g\n",
              options->blob_angle, options->blob_aspect,
              tscale_c, tscale_s, x, y);
#endif

  aspect = sqrt (SQR (x) + SQR (y));

  if (aspect != 0)
    {
      tcos = x / aspect;
      tsin = y / aspect;
    }
  else
    {
      tsin = sin (options->blob_angle);
      tcos = cos (options->blob_angle);
    }

  aspect = CLAMP (aspect, 1.0, 10.0);

  radmin = MAX (1.0, SUBSAMPLE * size / aspect);

  switch (options->blob_type)
    {
    case PICMAN_INK_BLOB_TYPE_CIRCLE:
      blob_function = picman_blob_ellipse;
      break;

    case PICMAN_INK_BLOB_TYPE_SQUARE:
      blob_function = picman_blob_square;
      break;

    case PICMAN_INK_BLOB_TYPE_DIAMOND:
      blob_function = picman_blob_diamond;
      break;

    default:
      g_return_val_if_reached (NULL);
      break;
    }

  return (* blob_function) (x_center * SUBSAMPLE,
                            y_center * SUBSAMPLE,
                            radmin * aspect * tcos,
                            radmin * aspect * tsin,
                            -radmin * tsin,
                            radmin * tcos);
}


/*********************************/
/*  Rendering functions          */
/*********************************/

/* Some of this stuff should probably be combined with the
 * code it was copied from in paint_core.c; but I wanted
 * to learn this stuff, so I've kept it simple.
 *
 * The following only supports CONSTANT mode. Incremental
 * would, I think, interact strangely with the way we
 * do things. But it wouldn't be hard to implement at all.
 */

enum
{
  ROW_START,
  ROW_STOP
};

/* The insertion sort here, for SUBSAMPLE = 8, tends to beat out
 * qsort() by 4x with CFLAGS=-O2, 2x with CFLAGS=-g
 */
static void
insert_sort (gint *data,
             gint  n)
{
  gint i, j, k;

  for (i = 2; i < 2 * n; i += 2)
    {
      gint tmp1 = data[i];
      gint tmp2 = data[i + 1];

      j = 0;

      while (data[j] < tmp1)
        j += 2;

      for (k = i; k > j; k -= 2)
        {
          data[k]     = data[k - 2];
          data[k + 1] = data[k - 1];
        }

      data[j]     = tmp1;
      data[j + 1] = tmp2;
    }
}

static void
fill_run (gfloat *dest,
          gfloat  alpha,
          gint    w)
{
  if (alpha == 1.0)
    {
      while (w--)
        {
          *dest = 1.0;
          dest++;
        }
    }
  else
    {
      while (w--)
        {
          *dest = MAX (*dest, alpha);
          dest++;
        }
    }
}

static void
render_blob_line (PicmanBlob *blob,
                  gfloat   *dest,
                  gint      x,
                  gint      y,
                  gint      width)
{
  gint  buf[4 * SUBSAMPLE];
  gint *data    = buf;
  gint  n       = 0;
  gint  i, j;
  gint  current = 0;  /* number of filled rows at this point
                       * in the scan line
                       */
  gint last_x;

  /* Sort start and ends for all lines */

  j = y * SUBSAMPLE - blob->y;
  for (i = 0; i < SUBSAMPLE; i++)
    {
      if (j >= blob->height)
        break;

      if ((j > 0) && (blob->data[j].left <= blob->data[j].right))
        {
          data[2 * n]                     = blob->data[j].left;
          data[2 * n + 1]                 = ROW_START;
          data[2 * SUBSAMPLE + 2 * n]     = blob->data[j].right;
          data[2 * SUBSAMPLE + 2 * n + 1] = ROW_STOP;
          n++;
        }
      j++;
    }

  /*   If we have less than SUBSAMPLE rows, compress */
  if (n < SUBSAMPLE)
    {
      for (i = 0; i < 2 * n; i++)
        data[2 * n + i] = data[2 * SUBSAMPLE + i];
    }

  /*   Now count start and end separately */
  n *= 2;

  insert_sort (data, n);

  /* Discard portions outside of tile */

  while ((n > 0) && (data[0] < SUBSAMPLE*x))
    {
      if (data[1] == ROW_START)
        current++;
      else
        current--;
      data += 2;
      n--;
    }

  while ((n > 0) && (data[2*(n-1)] >= SUBSAMPLE*(x+width)))
    n--;

  /* Render the row */

  last_x = 0;
  for (i = 0; i < n;)
    {
      gint cur_x = data[2 * i] / SUBSAMPLE - x;
      gint pixel;

      /* Fill in portion leading up to this pixel */
      if (current && cur_x != last_x)
        fill_run (dest + last_x, (gfloat) current / SUBSAMPLE, cur_x - last_x);

      /* Compute the value for this pixel */
      pixel = current * SUBSAMPLE;

      while (i<n)
        {
          gint tmp_x = data[2 * i] / SUBSAMPLE;

          if (tmp_x - x != cur_x)
            break;

          if (data[2 * i + 1] == ROW_START)
            {
              current++;
              pixel += ((tmp_x + 1) * SUBSAMPLE) - data[2 * i];
            }
          else
            {
              current--;
              pixel -= ((tmp_x + 1) * SUBSAMPLE) - data[2 * i];
            }

          i++;
        }

      dest[cur_x] = MAX (dest[cur_x], (gfloat) pixel / (SUBSAMPLE * SUBSAMPLE));

      last_x = cur_x + 1;
    }

  if (current != 0)
    fill_run (dest + last_x, (gfloat) current / SUBSAMPLE, width - last_x);
}

static void
render_blob (GeglBuffer    *buffer,
             GeglRectangle *rect,
             PicmanBlob      *blob)
{
  GeglBufferIterator *iter;
  GeglRectangle      *roi;

  iter = gegl_buffer_iterator_new (buffer, rect, 0, babl_format ("Y float"),
                                   GEGL_BUFFER_READWRITE, GEGL_ABYSS_NONE);
  roi = &iter->roi[0];

  while (gegl_buffer_iterator_next (iter))
    {
      gfloat *d = iter->data[0];
      gint    h = roi->height;
      gint    y;

      for (y = 0; y < h; y++, d += roi->width * 1)
        {
          render_blob_line (blob, d, roi->x, roi->y + y, roi->width);
        }
    }
}
