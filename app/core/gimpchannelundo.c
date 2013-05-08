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
#include "picmanchannel.h"
#include "picmanchannelundo.h"


enum
{
  PROP_0,
  PROP_PREV_PARENT,
  PROP_PREV_POSITION,
  PROP_PREV_CHANNEL
};


static void    picman_channel_undo_constructed  (GObject             *object);
static void    picman_channel_undo_set_property (GObject             *object,
                                               guint                property_id,
                                               const GValue        *value,
                                               GParamSpec          *pspec);
static void    picman_channel_undo_get_property (GObject             *object,
                                               guint                property_id,
                                               GValue              *value,
                                               GParamSpec          *pspec);

static gint64  picman_channel_undo_get_memsize  (PicmanObject          *object,
                                               gint64              *gui_size);

static void    picman_channel_undo_pop          (PicmanUndo            *undo,
                                               PicmanUndoMode         undo_mode,
                                               PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanChannelUndo, picman_channel_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_channel_undo_parent_class


static void
picman_channel_undo_class_init (PicmanChannelUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_channel_undo_constructed;
  object_class->set_property     = picman_channel_undo_set_property;
  object_class->get_property     = picman_channel_undo_get_property;

  picman_object_class->get_memsize = picman_channel_undo_get_memsize;

  undo_class->pop                = picman_channel_undo_pop;

  g_object_class_install_property (object_class, PROP_PREV_PARENT,
                                   g_param_spec_object ("prev-parent",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CHANNEL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_POSITION,
                                   g_param_spec_int ("prev-position",
                                                     NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_CHANNEL,
                                   g_param_spec_object ("prev-channel",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CHANNEL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_channel_undo_init (PicmanChannelUndo *undo)
{
}

static void
picman_channel_undo_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_CHANNEL (PICMAN_ITEM_UNDO (object)->item));
}

static void
picman_channel_undo_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanChannelUndo *channel_undo = PICMAN_CHANNEL_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      channel_undo->prev_parent = g_value_get_object (value);
      break;
    case PROP_PREV_POSITION:
      channel_undo->prev_position = g_value_get_int (value);
      break;
    case PROP_PREV_CHANNEL:
      channel_undo->prev_channel = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_channel_undo_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanChannelUndo *channel_undo = PICMAN_CHANNEL_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      g_value_set_object (value, channel_undo->prev_parent);
      break;
    case PROP_PREV_POSITION:
      g_value_set_int (value, channel_undo->prev_position);
      break;
    case PROP_PREV_CHANNEL:
      g_value_set_object (value, channel_undo->prev_channel);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_channel_undo_get_memsize (PicmanObject *object,
                               gint64     *gui_size)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (object);
  gint64        memsize   = 0;

  if (! picman_item_is_attached (item_undo->item))
    memsize += picman_object_get_memsize (PICMAN_OBJECT (item_undo->item),
                                        gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_channel_undo_pop (PicmanUndo            *undo,
                       PicmanUndoMode         undo_mode,
                       PicmanUndoAccumulator *accum)
{
  PicmanChannelUndo *channel_undo = PICMAN_CHANNEL_UNDO (undo);
  PicmanChannel     *channel      = PICMAN_CHANNEL (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if ((undo_mode       == PICMAN_UNDO_MODE_UNDO &&
       undo->undo_type == PICMAN_UNDO_CHANNEL_ADD) ||
      (undo_mode       == PICMAN_UNDO_MODE_REDO &&
       undo->undo_type == PICMAN_UNDO_CHANNEL_REMOVE))
    {
      /*  remove channel  */

      /*  record the current parent and position  */
      channel_undo->prev_parent   = picman_channel_get_parent (channel);
      channel_undo->prev_position = picman_item_get_index (PICMAN_ITEM (channel));

      picman_image_remove_channel (undo->image, channel, FALSE,
                                 channel_undo->prev_channel);
    }
  else
    {
      /*  restore channel  */

      /*  record the active channel  */
      channel_undo->prev_channel = picman_image_get_active_channel (undo->image);

      picman_image_add_channel (undo->image, channel,
                              channel_undo->prev_parent,
                              channel_undo->prev_position, FALSE);
    }
}
