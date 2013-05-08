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

#include "gegl/picman-gegl-utils.h"

#include "picman-utils.h"
#include "picmanimage.h"
#include "picmandrawable.h"
#include "picmandrawablemodundo.h"


enum
{
  PROP_0,
  PROP_COPY_BUFFER
};


static void     picman_drawable_mod_undo_constructed  (GObject             *object);
static void     picman_drawable_mod_undo_set_property (GObject             *object,
                                                     guint                property_id,
                                                     const GValue        *value,
                                                     GParamSpec          *pspec);
static void     picman_drawable_mod_undo_get_property (GObject             *object,
                                                     guint                property_id,
                                                     GValue              *value,
                                                     GParamSpec          *pspec);

static gint64   picman_drawable_mod_undo_get_memsize  (PicmanObject          *object,
                                                     gint64              *gui_size);

static void     picman_drawable_mod_undo_pop          (PicmanUndo            *undo,
                                                     PicmanUndoMode         undo_mode,
                                                     PicmanUndoAccumulator *accum);
static void     picman_drawable_mod_undo_free         (PicmanUndo            *undo,
                                                     PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanDrawableModUndo, picman_drawable_mod_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_drawable_mod_undo_parent_class


static void
picman_drawable_mod_undo_class_init (PicmanDrawableModUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_drawable_mod_undo_constructed;
  object_class->set_property     = picman_drawable_mod_undo_set_property;
  object_class->get_property     = picman_drawable_mod_undo_get_property;

  picman_object_class->get_memsize = picman_drawable_mod_undo_get_memsize;

  undo_class->pop                = picman_drawable_mod_undo_pop;
  undo_class->free               = picman_drawable_mod_undo_free;

  g_object_class_install_property (object_class, PROP_COPY_BUFFER,
                                   g_param_spec_boolean ("copy-buffer",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_drawable_mod_undo_init (PicmanDrawableModUndo *undo)
{
}

static void
picman_drawable_mod_undo_constructed (GObject *object)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (object);
  PicmanItem            *item;
  PicmanDrawable        *drawable;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_DRAWABLE (PICMAN_ITEM_UNDO (object)->item));

  item     = PICMAN_ITEM_UNDO (object)->item;
  drawable = PICMAN_DRAWABLE (item);

  if (drawable_mod_undo->copy_buffer)
    {
      drawable_mod_undo->buffer =
        gegl_buffer_dup (picman_drawable_get_buffer (drawable));
    }
  else
    {
      drawable_mod_undo->buffer =
        g_object_ref (picman_drawable_get_buffer (drawable));
    }

  picman_item_get_offset (item,
                        &drawable_mod_undo->offset_x,
                        &drawable_mod_undo->offset_y);
}

static void
picman_drawable_mod_undo_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (object);

  switch (property_id)
    {
    case PROP_COPY_BUFFER:
      drawable_mod_undo->copy_buffer = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_drawable_mod_undo_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (object);

  switch (property_id)
    {
    case PROP_COPY_BUFFER:
      g_value_set_boolean (value, drawable_mod_undo->copy_buffer);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_drawable_mod_undo_get_memsize (PicmanObject *object,
                                    gint64     *gui_size)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (object);
  gint64               memsize           = 0;

  memsize += picman_gegl_buffer_get_memsize (drawable_mod_undo->buffer);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_drawable_mod_undo_pop (PicmanUndo            *undo,
                            PicmanUndoMode         undo_mode,
                            PicmanUndoAccumulator *accum)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (undo);
  PicmanDrawable        *drawable          = PICMAN_DRAWABLE (PICMAN_ITEM_UNDO (undo)->item);
  GeglBuffer          *buffer;
  gint                 offset_x;
  gint                 offset_y;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  buffer   = drawable_mod_undo->buffer;
  offset_x = drawable_mod_undo->offset_x;
  offset_y = drawable_mod_undo->offset_y;

  drawable_mod_undo->buffer = g_object_ref (picman_drawable_get_buffer (drawable));

  picman_item_get_offset (PICMAN_ITEM (drawable),
                        &drawable_mod_undo->offset_x,
                        &drawable_mod_undo->offset_y);

  picman_drawable_set_buffer_full (drawable, FALSE, NULL,
                                 buffer, offset_x, offset_y);
  g_object_unref (buffer);
}

static void
picman_drawable_mod_undo_free (PicmanUndo     *undo,
                             PicmanUndoMode  undo_mode)
{
  PicmanDrawableModUndo *drawable_mod_undo = PICMAN_DRAWABLE_MOD_UNDO (undo);

  if (drawable_mod_undo->buffer)
    {
      g_object_unref (drawable_mod_undo->buffer);
      drawable_mod_undo->buffer = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
