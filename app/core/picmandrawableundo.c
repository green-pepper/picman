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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmanimage.h"
#include "picmandrawable.h"
#include "picmandrawableundo.h"


enum
{
  PROP_0,
  PROP_BUFFER,
  PROP_X,
  PROP_Y
};


static void     picman_drawable_undo_constructed  (GObject             *object);
static void     picman_drawable_undo_set_property (GObject             *object,
                                                 guint                property_id,
                                                 const GValue        *value,
                                                 GParamSpec          *pspec);
static void     picman_drawable_undo_get_property (GObject             *object,
                                                 guint                property_id,
                                                 GValue              *value,
                                                 GParamSpec          *pspec);

static gint64   picman_drawable_undo_get_memsize  (PicmanObject          *object,
                                                 gint64              *gui_size);

static void     picman_drawable_undo_pop          (PicmanUndo            *undo,
                                                 PicmanUndoMode         undo_mode,
                                                 PicmanUndoAccumulator *accum);
static void     picman_drawable_undo_free         (PicmanUndo            *undo,
                                                 PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanDrawableUndo, picman_drawable_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_drawable_undo_parent_class


static void
picman_drawable_undo_class_init (PicmanDrawableUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_drawable_undo_constructed;
  object_class->set_property     = picman_drawable_undo_set_property;
  object_class->get_property     = picman_drawable_undo_get_property;

  picman_object_class->get_memsize = picman_drawable_undo_get_memsize;

  undo_class->pop                = picman_drawable_undo_pop;
  undo_class->free               = picman_drawable_undo_free;

  g_object_class_install_property (object_class, PROP_BUFFER,
                                   g_param_spec_object ("buffer", NULL, NULL,
                                                        GEGL_TYPE_BUFFER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_X,
                                   g_param_spec_int ("x", NULL, NULL,
                                                     0, PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_Y,
                                   g_param_spec_int ("y", NULL, NULL,
                                                     0, PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_drawable_undo_init (PicmanDrawableUndo *undo)
{
}

static void
picman_drawable_undo_constructed (GObject *object)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_DRAWABLE (PICMAN_ITEM_UNDO (object)->item));
  g_assert (drawable_undo->buffer != NULL);
}

static void
picman_drawable_undo_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (object);

  switch (property_id)
    {
    case PROP_BUFFER:
      drawable_undo->buffer = g_value_dup_object (value);
      break;
    case PROP_X:
      drawable_undo->x = g_value_get_int (value);
      break;
    case PROP_Y:
      drawable_undo->y = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_drawable_undo_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (object);

  switch (property_id)
    {
    case PROP_BUFFER:
      g_value_set_object (value, drawable_undo->buffer);
      break;
    case PROP_X:
      g_value_set_int (value, drawable_undo->x);
      break;
    case PROP_Y:
      g_value_set_int (value, drawable_undo->y);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_drawable_undo_get_memsize (PicmanObject *object,
                                gint64     *gui_size)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (object);
  gint64            memsize       = 0;

  memsize += picman_gegl_buffer_get_memsize (drawable_undo->buffer);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_drawable_undo_pop (PicmanUndo            *undo,
                        PicmanUndoMode         undo_mode,
                        PicmanUndoAccumulator *accum)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (undo);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  picman_drawable_swap_pixels (PICMAN_DRAWABLE (PICMAN_ITEM_UNDO (undo)->item),
                             drawable_undo->buffer,
                             drawable_undo->x,
                             drawable_undo->y);
}

static void
picman_drawable_undo_free (PicmanUndo     *undo,
                         PicmanUndoMode  undo_mode)
{
  PicmanDrawableUndo *drawable_undo = PICMAN_DRAWABLE_UNDO (undo);

  if (drawable_undo->buffer)
    {
      g_object_unref (drawable_undo->buffer);
      drawable_undo->buffer = NULL;
    }

  if (drawable_undo->applied_buffer)
    {
      g_object_unref (drawable_undo->applied_buffer);
      drawable_undo->applied_buffer = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
