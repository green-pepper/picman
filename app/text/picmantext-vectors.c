/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText-vectors
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#include <gegl.h>

#include <pango/pangocairo.h>

#include "text-types.h"

#include "core/picmanimage.h"

#include "vectors/picmanbezierstroke.h"
#include "vectors/picmanvectors.h"
#include "vectors/picmananchor.h"

#include "picmantext.h"
#include "picmantext-vectors.h"
#include "picmantextlayout.h"
#include "picmantextlayout-render.h"


typedef struct
{
  PicmanVectors *vectors;
  PicmanStroke  *stroke;
  PicmanAnchor  *anchor;
} RenderContext;


static void  picman_text_render_vectors (cairo_t       *cr,
                                       RenderContext *context);


PicmanVectors *
picman_text_vectors_new (PicmanImage *image,
                       PicmanText  *text)
{
  PicmanVectors   *vectors;
  RenderContext  context = { NULL, };

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT (text), NULL);

  vectors = picman_vectors_new (image, NULL);

  if (text->text || text->markup)
    {
      PicmanTextLayout  *layout;
      cairo_surface_t *surface;
      cairo_t         *cr;
      gdouble          xres;
      gdouble          yres;

      if (text->text)
        picman_object_set_name_safe (PICMAN_OBJECT (vectors), text->text);

      context.vectors = vectors;

      surface = cairo_recording_surface_create (CAIRO_CONTENT_ALPHA, NULL);
      cr = cairo_create (surface);

      picman_image_get_resolution (image, &xres, &yres);

      layout = picman_text_layout_new (text, xres, yres);
      picman_text_layout_render (layout, cr, text->base_dir, TRUE);
      g_object_unref (layout);

      picman_text_render_vectors (cr, &context);

      cairo_destroy (cr);
      cairo_surface_destroy (surface);

      if (context.stroke)
        picman_stroke_close (context.stroke);
    }

  return vectors;
}


static inline void
picman_text_vector_coords (const double  x,
                         const double  y,
                         PicmanCoords   *coords)
{
  const PicmanCoords default_values = PICMAN_COORDS_DEFAULT_VALUES;

  *coords = default_values;

  coords->x = x;
  coords->y = y;
}

static gint
moveto (RenderContext *context,
        const double   x,
        const double   y)
{
  PicmanCoords     start;

#if PICMAN_TEXT_DEBUG
  g_printerr ("moveto  %f, %f\n", x, y);
#endif

  picman_text_vector_coords (x, y, &start);

  if (context->stroke)
    picman_stroke_close (context->stroke);

  context->stroke = picman_bezier_stroke_new_moveto (&start);

  picman_vectors_stroke_add (context->vectors, context->stroke);
  g_object_unref (context->stroke);

  return 0;
}

static gint
lineto (RenderContext *context,
        const double   x,
        const double   y)
{
  PicmanCoords     end;

#if PICMAN_TEXT_DEBUG
  g_printerr ("lineto  %f, %f\n", x, y);
#endif

  if (! context->stroke)
    return 0;

  picman_text_vector_coords (x, y, &end);

  picman_bezier_stroke_lineto (context->stroke, &end);

  return 0;
}

static gint
cubicto (RenderContext *context,
         const double   x1,
         const double   y1,
         const double   x2,
         const double   y2,
         const double   x3,
         const double   y3)
{
  PicmanCoords     control1;
  PicmanCoords     control2;
  PicmanCoords     end;

#if PICMAN_TEXT_DEBUG
  g_printerr ("cubicto %f, %f\n", x3, y3);
#endif

  if (! context->stroke)
    return 0;

  picman_text_vector_coords (x1, y1, &control1);
  picman_text_vector_coords (x2, y2, &control2);
  picman_text_vector_coords (x3, y3, &end);

  picman_bezier_stroke_cubicto (context->stroke, &control1, &control2, &end);

  return 0;
}

static gint
closepath (RenderContext *context)
{
#if PICMAN_TEXT_DEBUG
  g_printerr ("moveto\n");
#endif

  if (! context->stroke)
    return 0;

  picman_stroke_close (context->stroke);

  context->stroke = NULL;

  return 0;
}

static void
picman_text_render_vectors (cairo_t       *cr,
                          RenderContext *context)
{
  cairo_path_t *path;
  gint          i;

  path = cairo_copy_path (cr);

  for (i = 0; i < path->num_data; i += path->data[i].header.length)
    {
      cairo_path_data_t *data = &path->data[i];

      /* if the drawing operation is the final moveto of the glyph,
       * break to avoid creating an empty point. This is because cairo
       * always adds a moveto after each closepath.
       */
      if (i + data->header.length >= path->num_data)
        break;

      switch (data->header.type)
        {
        case CAIRO_PATH_MOVE_TO:
          moveto (context, data[1].point.x, data[1].point.y);
          break;

        case CAIRO_PATH_LINE_TO:
          lineto (context, data[1].point.x, data[1].point.y);
          break;

        case CAIRO_PATH_CURVE_TO:
          cubicto (context,
                   data[1].point.x, data[1].point.y,
                   data[2].point.x, data[2].point.y,
                   data[3].point.x, data[3].point.y);
          break;

        case CAIRO_PATH_CLOSE_PATH:
          closepath (context);
          break;
        }
    }

  cairo_path_destroy (path);
}
