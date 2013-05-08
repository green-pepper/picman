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
#include "picmanimage-guides.h"
#include "picmanguide.h"
#include "picmanguideundo.h"


enum
{
  PROP_0,
  PROP_GUIDE
};


static void   picman_guide_undo_constructed  (GObject            *object);
static void   picman_guide_undo_set_property (GObject             *object,
                                            guint                property_id,
                                            const GValue        *value,
                                            GParamSpec          *pspec);
static void   picman_guide_undo_get_property (GObject             *object,
                                            guint                property_id,
                                            GValue              *value,
                                            GParamSpec          *pspec);

static void   picman_guide_undo_pop          (PicmanUndo            *undo,
                                            PicmanUndoMode         undo_mode,
                                            PicmanUndoAccumulator *accum);
static void   picman_guide_undo_free         (PicmanUndo            *undo,
                                            PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanGuideUndo, picman_guide_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_guide_undo_parent_class


static void
picman_guide_undo_class_init (PicmanGuideUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed  = picman_guide_undo_constructed;
  object_class->set_property = picman_guide_undo_set_property;
  object_class->get_property = picman_guide_undo_get_property;

  undo_class->pop            = picman_guide_undo_pop;
  undo_class->free           = picman_guide_undo_free;

  g_object_class_install_property (object_class, PROP_GUIDE,
                                   g_param_spec_object ("guide", NULL, NULL,
                                                        PICMAN_TYPE_GUIDE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_guide_undo_init (PicmanGuideUndo *undo)
{
}

static void
picman_guide_undo_constructed (GObject *object)
{
  PicmanGuideUndo *guide_undo = PICMAN_GUIDE_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_GUIDE (guide_undo->guide));

  guide_undo->orientation = picman_guide_get_orientation (guide_undo->guide);
  guide_undo->position    = picman_guide_get_position (guide_undo->guide);
}

static void
picman_guide_undo_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanGuideUndo *guide_undo = PICMAN_GUIDE_UNDO (object);

  switch (property_id)
    {
    case PROP_GUIDE:
      guide_undo->guide = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_guide_undo_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanGuideUndo *guide_undo = PICMAN_GUIDE_UNDO (object);

  switch (property_id)
    {
    case PROP_GUIDE:
      g_value_set_object (value, guide_undo->guide);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_guide_undo_pop (PicmanUndo              *undo,
                     PicmanUndoMode           undo_mode,
                     PicmanUndoAccumulator   *accum)
{
  PicmanGuideUndo       *guide_undo = PICMAN_GUIDE_UNDO (undo);
  PicmanOrientationType  orientation;
  gint                 position;
  gboolean             moved = FALSE;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  orientation = picman_guide_get_orientation (guide_undo->guide);
  position    = picman_guide_get_position (guide_undo->guide);

  if (position == -1)
    {
      picman_image_add_guide (undo->image,
                            guide_undo->guide, guide_undo->position);
    }
  else if (guide_undo->position == -1)
    {
      picman_image_remove_guide (undo->image, guide_undo->guide, FALSE);
    }
  else
    {
      picman_guide_set_position (guide_undo->guide, guide_undo->position);

      moved = TRUE;
    }

  picman_guide_set_orientation (guide_undo->guide, guide_undo->orientation);

  if (moved || guide_undo->orientation != orientation)
    picman_image_guide_moved (undo->image, guide_undo->guide);

  guide_undo->position    = position;
  guide_undo->orientation = orientation;
}

static void
picman_guide_undo_free (PicmanUndo     *undo,
                      PicmanUndoMode  undo_mode)
{
  PicmanGuideUndo *guide_undo = PICMAN_GUIDE_UNDO (undo);

  if (guide_undo->guide)
    {
      g_object_unref (guide_undo->guide);
      guide_undo->guide = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
