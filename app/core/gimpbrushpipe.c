/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1999 Adrian Likins and Tor Lillqvist
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

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmanbrushpipe.h"
#include "picmanbrushpipe-load.h"


static void        picman_brush_pipe_finalize         (GObject          *object);

static gint64      picman_brush_pipe_get_memsize      (PicmanObject       *object,
                                                     gint64           *gui_size);

static gboolean    picman_brush_pipe_get_popup_size   (PicmanViewable     *viewable,
                                                     gint              width,
                                                     gint              height,
                                                     gboolean          dot_for_dot,
                                                     gint             *popup_width,
                                                     gint             *popup_height);

static void        picman_brush_pipe_begin_use        (PicmanBrush        *brush);
static void        picman_brush_pipe_end_use          (PicmanBrush        *brush);
static PicmanBrush * picman_brush_pipe_select_brush     (PicmanBrush        *brush,
                                                     const PicmanCoords *last_coords,
                                                     const PicmanCoords *current_coords);
static gboolean    picman_brush_pipe_want_null_motion (PicmanBrush        *brush,
                                                     const PicmanCoords *last_coords,
                                                     const PicmanCoords *current_coords);


G_DEFINE_TYPE (PicmanBrushPipe, picman_brush_pipe, PICMAN_TYPE_BRUSH);

#define parent_class picman_brush_pipe_parent_class


static void
picman_brush_pipe_class_init (PicmanBrushPipeClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanBrushClass    *brush_class       = PICMAN_BRUSH_CLASS (klass);

  object_class->finalize         = picman_brush_pipe_finalize;

  picman_object_class->get_memsize = picman_brush_pipe_get_memsize;

  viewable_class->get_popup_size = picman_brush_pipe_get_popup_size;

  brush_class->begin_use         = picman_brush_pipe_begin_use;
  brush_class->end_use           = picman_brush_pipe_end_use;
  brush_class->select_brush      = picman_brush_pipe_select_brush;
  brush_class->want_null_motion  = picman_brush_pipe_want_null_motion;
}

static void
picman_brush_pipe_init (PicmanBrushPipe *pipe)
{
  pipe->current   = NULL;
  pipe->dimension = 0;
  pipe->rank      = NULL;
  pipe->stride    = NULL;
  pipe->n_brushes = 0;
  pipe->brushes   = NULL;
  pipe->select    = NULL;
  pipe->index     = NULL;
}

static void
picman_brush_pipe_finalize (GObject *object)
{
  PicmanBrushPipe *pipe = PICMAN_BRUSH_PIPE (object);

  if (pipe->rank)
    {
      g_free (pipe->rank);
      pipe->rank = NULL;
    }
  if (pipe->stride)
    {
      g_free (pipe->stride);
      pipe->stride = NULL;
    }

  if (pipe->brushes)
    {
      gint i;

      for (i = 0; i < pipe->n_brushes; i++)
        if (pipe->brushes[i])
          g_object_unref (pipe->brushes[i]);

      g_free (pipe->brushes);
      pipe->brushes = NULL;
    }

  if (pipe->select)
    {
      g_free (pipe->select);
      pipe->select = NULL;
    }
  if (pipe->index)
    {
      g_free (pipe->index);
      pipe->index = NULL;
    }

  PICMAN_BRUSH (pipe)->mask   = NULL;
  PICMAN_BRUSH (pipe)->pixmap = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_brush_pipe_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanBrushPipe *pipe    = PICMAN_BRUSH_PIPE (object);
  gint64         memsize = 0;
  gint           i;

  memsize += pipe->dimension * (sizeof (gint) /* rank   */ +
                                sizeof (gint) /* stride */ +
                                sizeof (PipeSelectModes));

  for (i = 0; i < pipe->n_brushes; i++)
    memsize += picman_object_get_memsize (PICMAN_OBJECT (pipe->brushes[i]),
                                        gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_brush_pipe_get_popup_size (PicmanViewable *viewable,
                                gint          width,
                                gint          height,
                                gboolean      dot_for_dot,
                                gint         *popup_width,
                                gint         *popup_height)
{
  return picman_viewable_get_size (viewable, popup_width, popup_height);
}

static void
picman_brush_pipe_begin_use (PicmanBrush *brush)
{
  PicmanBrushPipe *pipe = PICMAN_BRUSH_PIPE (brush);
  gint           i;

  PICMAN_BRUSH_CLASS (parent_class)->begin_use (brush);

  for (i = 0; i < pipe->n_brushes; i++)
    if (pipe->brushes[i])
      picman_brush_begin_use (pipe->brushes[i]);
}

static void
picman_brush_pipe_end_use (PicmanBrush *brush)
{
  PicmanBrushPipe *pipe = PICMAN_BRUSH_PIPE (brush);
  gint           i;

  PICMAN_BRUSH_CLASS (parent_class)->end_use (brush);

  for (i = 0; i < pipe->n_brushes; i++)
    if (pipe->brushes[i])
      picman_brush_end_use (pipe->brushes[i]);
}

static PicmanBrush *
picman_brush_pipe_select_brush (PicmanBrush        *brush,
                              const PicmanCoords *last_coords,
                              const PicmanCoords *current_coords)
{
  PicmanBrushPipe *pipe = PICMAN_BRUSH_PIPE (brush);
  gint           i, brushix, ix;
  gdouble        velocity;

  if (pipe->n_brushes == 1)
    return PICMAN_BRUSH (pipe->current);

  brushix = 0;
  for (i = 0; i < pipe->dimension; i++)
    {
      switch (pipe->select[i])
        {
        case PIPE_SELECT_INCREMENTAL:
          ix = (pipe->index[i] + 1) % pipe->rank[i];
          break;

        case PIPE_SELECT_ANGULAR:
          /* Coords angle is already nomalized,
           * offset by 90 degrees is still needed
           * because hoses were made PS compatible*/
          ix = (gint) RINT ((1.0 - current_coords->direction + 0.25) * pipe->rank[i]) % pipe->rank[i];
          break;

        case PIPE_SELECT_VELOCITY:
          velocity = current_coords->velocity;

          /* Max velocity is 3.0, picking stamp as a ratio*/
          ix = ROUND ((3.0 / velocity) * pipe->rank[i]);
          break;

        case PIPE_SELECT_RANDOM:
          /* This probably isn't the right way */
          ix = g_random_int_range (0, pipe->rank[i]);
          break;

        case PIPE_SELECT_PRESSURE:
          ix = RINT (current_coords->pressure * (pipe->rank[i] - 1));
          break;

        case PIPE_SELECT_TILT_X:
          ix = RINT (current_coords->xtilt / 2.0 * pipe->rank[i]) + pipe->rank[i] / 2;
          break;

        case PIPE_SELECT_TILT_Y:
          ix = RINT (current_coords->ytilt / 2.0 * pipe->rank[i]) + pipe->rank[i] / 2;
          break;

        case PIPE_SELECT_CONSTANT:
        default:
          ix = pipe->index[i];
          break;
        }

      pipe->index[i] = CLAMP (ix, 0, pipe->rank[i] - 1);
      brushix += pipe->stride[i] * pipe->index[i];
    }

  /* Make sure is inside bounds */
  brushix = CLAMP (brushix, 0, pipe->n_brushes - 1);

  pipe->current = pipe->brushes[brushix];

  return PICMAN_BRUSH (pipe->current);
}

static gboolean
picman_brush_pipe_want_null_motion (PicmanBrush        *brush,
                                  const PicmanCoords *last_coords,
                                  const PicmanCoords *current_coords)
{
  PicmanBrushPipe *pipe = PICMAN_BRUSH_PIPE (brush);
  gint           i;

  if (pipe->n_brushes == 1)
    return TRUE;

  for (i = 0; i < pipe->dimension; i++)
    if (pipe->select[i] == PIPE_SELECT_ANGULAR)
      return FALSE;

  return TRUE;
}
