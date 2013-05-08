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

#include <gegl.h>

#include "core-types.h"

#include "picmanimage.h"
#include "picmanimage-sample-points.h"
#include "picmansamplepoint.h"
#include "picmansamplepointundo.h"


enum
{
  PROP_0,
  PROP_SAMPLE_POINT
};


static void   picman_sample_point_undo_constructed  (GObject             *object);
static void   picman_sample_point_undo_set_property (GObject             *object,
                                                   guint                property_id,
                                                   const GValue        *value,
                                                   GParamSpec          *pspec);
static void   picman_sample_point_undo_get_property (GObject             *object,
                                                   guint                property_id,
                                                   GValue              *value,
                                                   GParamSpec          *pspec);

static void   picman_sample_point_undo_pop          (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode,
                                                   PicmanUndoAccumulator *accum);
static void   picman_sample_point_undo_free         (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanSamplePointUndo, picman_sample_point_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_sample_point_undo_parent_class


static void
picman_sample_point_undo_class_init (PicmanSamplePointUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed  = picman_sample_point_undo_constructed;
  object_class->set_property = picman_sample_point_undo_set_property;
  object_class->get_property = picman_sample_point_undo_get_property;

  undo_class->pop            = picman_sample_point_undo_pop;
  undo_class->free           = picman_sample_point_undo_free;

  g_object_class_install_property (object_class, PROP_SAMPLE_POINT,
                                   g_param_spec_boxed ("sample-point", NULL, NULL,
                                                       PICMAN_TYPE_SAMPLE_POINT,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_sample_point_undo_init (PicmanSamplePointUndo *undo)
{
}

static void
picman_sample_point_undo_constructed (GObject *object)
{
  PicmanSamplePointUndo *sample_point_undo = PICMAN_SAMPLE_POINT_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (sample_point_undo->sample_point != NULL);

  sample_point_undo->x = sample_point_undo->sample_point->x;
  sample_point_undo->y = sample_point_undo->sample_point->y;
}

static void
picman_sample_point_undo_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanSamplePointUndo *sample_point_undo = PICMAN_SAMPLE_POINT_UNDO (object);

  switch (property_id)
    {
    case PROP_SAMPLE_POINT:
      sample_point_undo->sample_point = g_value_dup_boxed (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_sample_point_undo_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanSamplePointUndo *sample_point_undo = PICMAN_SAMPLE_POINT_UNDO (object);

  switch (property_id)
    {
    case PROP_SAMPLE_POINT:
      g_value_set_boxed (value, sample_point_undo->sample_point);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_sample_point_undo_pop (PicmanUndo              *undo,
                            PicmanUndoMode           undo_mode,
                            PicmanUndoAccumulator   *accum)
{
  PicmanSamplePointUndo *sample_point_undo = PICMAN_SAMPLE_POINT_UNDO (undo);
  gint                 x;
  gint                 y;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  x = sample_point_undo->sample_point->x;
  y = sample_point_undo->sample_point->y;

  if (x == -1)
    {
      picman_image_add_sample_point (undo->image,
                                   sample_point_undo->sample_point,
                                   sample_point_undo->x,
                                   sample_point_undo->y);
    }
  else if (sample_point_undo->x == -1)
    {
      picman_image_remove_sample_point (undo->image,
                                      sample_point_undo->sample_point, FALSE);
    }
  else
    {
      sample_point_undo->sample_point->x = sample_point_undo->x;
      sample_point_undo->sample_point->y = sample_point_undo->y;

      picman_image_sample_point_moved (undo->image,
                                     sample_point_undo->sample_point);
    }

  sample_point_undo->x = x;
  sample_point_undo->y = y;
}

static void
picman_sample_point_undo_free (PicmanUndo     *undo,
                             PicmanUndoMode  undo_mode)
{
  PicmanSamplePointUndo *sample_point_undo = PICMAN_SAMPLE_POINT_UNDO (undo);

  if (sample_point_undo->sample_point)
    {
      picman_sample_point_unref (sample_point_undo->sample_point);
      sample_point_undo->sample_point = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
