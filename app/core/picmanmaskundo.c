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
#include "picmanchannel.h"
#include "picmanmaskundo.h"


enum
{
  PROP_0,
  PROP_CONVERT_FORMAT
};


static void     picman_mask_undo_constructed  (GObject             *object);
static void     picman_mask_undo_set_property (GObject             *object,
                                             guint                property_id,
                                             const GValue        *value,
                                             GParamSpec          *pspec);
static void     picman_mask_undo_get_property (GObject             *object,
                                             guint                property_id,
                                             GValue              *value,
                                             GParamSpec          *pspec);

static gint64   picman_mask_undo_get_memsize  (PicmanObject          *object,
                                             gint64              *gui_size);

static void     picman_mask_undo_pop          (PicmanUndo            *undo,
                                             PicmanUndoMode         undo_mode,
                                             PicmanUndoAccumulator *accum);
static void     picman_mask_undo_free         (PicmanUndo            *undo,
                                             PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanMaskUndo, picman_mask_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_mask_undo_parent_class


static void
picman_mask_undo_class_init (PicmanMaskUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_mask_undo_constructed;
  object_class->set_property     = picman_mask_undo_set_property;
  object_class->get_property     = picman_mask_undo_get_property;

  picman_object_class->get_memsize = picman_mask_undo_get_memsize;

  undo_class->pop                = picman_mask_undo_pop;
  undo_class->free               = picman_mask_undo_free;

  g_object_class_install_property (object_class, PROP_CONVERT_FORMAT,
                                   g_param_spec_boolean ("convert-format",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_mask_undo_init (PicmanMaskUndo *undo)
{
}

static void
picman_mask_undo_constructed (GObject *object)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (object);
  PicmanChannel  *channel;
  PicmanDrawable *drawable;
  gint          x1, y1, x2, y2;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_CHANNEL (PICMAN_ITEM_UNDO (object)->item));

  channel  = PICMAN_CHANNEL (PICMAN_ITEM_UNDO (object)->item);
  drawable = PICMAN_DRAWABLE (channel);

  if (picman_channel_bounds (channel, &x1, &y1, &x2, &y2))
    {
      mask_undo->buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                           x2 - x1, y2 - y1),
                                           picman_drawable_get_format (drawable));

      gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1),
                        mask_undo->buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));

      mask_undo->x = x1;
      mask_undo->y = y1;
    }

  mask_undo->format = picman_drawable_get_format (drawable);
}

static void
picman_mask_undo_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (object);

  switch (property_id)
    {
    case PROP_CONVERT_FORMAT:
      mask_undo->convert_format = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_mask_undo_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (object);

  switch (property_id)
    {
    case PROP_CONVERT_FORMAT:
      g_value_set_boolean (value, mask_undo->convert_format);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_mask_undo_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (object);
  gint64        memsize   = 0;

  memsize += picman_gegl_buffer_get_memsize (mask_undo->buffer);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_mask_undo_pop (PicmanUndo            *undo,
                    PicmanUndoMode         undo_mode,
                    PicmanUndoAccumulator *accum)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (undo);
  PicmanChannel  *channel   = PICMAN_CHANNEL (PICMAN_ITEM_UNDO (undo)->item);
  PicmanDrawable *drawable  = PICMAN_DRAWABLE (channel);
  GeglBuffer   *new_buffer;
  const Babl   *format;
  gint          x1, y1, x2, y2;
  gint          width  = 0;
  gint          height = 0;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if (picman_channel_bounds (channel, &x1, &y1, &x2, &y2))
    {
      new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, x2 - x1, y2 - y1),
                                    picman_drawable_get_format (drawable));

      gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1),
                        new_buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));

      gegl_buffer_clear (picman_drawable_get_buffer (drawable),
                         GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1));
    }
  else
    {
      new_buffer = NULL;
    }

  format = picman_drawable_get_format (drawable);

  if (mask_undo->convert_format)
    {
      GeglBuffer *buffer;
      gint        width  = picman_item_get_width  (PICMAN_ITEM (channel));
      gint        height = picman_item_get_height (PICMAN_ITEM (channel));

      buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                mask_undo->format);
      gegl_buffer_clear (buffer, NULL);

      picman_drawable_set_buffer (drawable, FALSE, NULL, buffer);
      g_object_unref (buffer);
    }

  if (mask_undo->buffer)
    {
      width  = gegl_buffer_get_width  (mask_undo->buffer);
      height = gegl_buffer_get_height (mask_undo->buffer);

      gegl_buffer_copy (mask_undo->buffer,
                        NULL,
                        picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (mask_undo->x, mask_undo->y, 0, 0));

      g_object_unref (mask_undo->buffer);
    }

  /* invalidate the current bounds and boundary of the mask */
  picman_drawable_invalidate_boundary (drawable);

  if (mask_undo->buffer)
    {
      channel->empty = FALSE;
      channel->x1    = mask_undo->x;
      channel->y1    = mask_undo->y;
      channel->x2    = mask_undo->x + width;
      channel->y2    = mask_undo->y + height;
    }
  else
    {
      channel->empty = TRUE;
      channel->x1    = 0;
      channel->y1    = 0;
      channel->x2    = picman_item_get_width  (PICMAN_ITEM (channel));
      channel->y2    = picman_item_get_height (PICMAN_ITEM (channel));
    }

  /* we know the bounds */
  channel->bounds_known = TRUE;

  /*  set the new mask undo parameters  */
  mask_undo->buffer = new_buffer;
  mask_undo->x      = x1;
  mask_undo->y      = y1;
  mask_undo->format = format;

  picman_drawable_update (PICMAN_DRAWABLE (channel),
                        0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_mask_undo_free (PicmanUndo     *undo,
                     PicmanUndoMode  undo_mode)
{
  PicmanMaskUndo *mask_undo = PICMAN_MASK_UNDO (undo);

  if (mask_undo->buffer)
    {
      g_object_unref (mask_undo->buffer);
      mask_undo->buffer = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
