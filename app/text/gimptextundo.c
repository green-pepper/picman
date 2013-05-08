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

#include "libpicmanconfig/picmanconfig.h"

#include "text-types.h"

#include "gegl/picman-babl.h"

#include "core/picmanitem.h"
#include "core/picmanitemundo.h"
#include "core/picman-utils.h"

#include "picmantext.h"
#include "picmantextlayer.h"
#include "picmantextundo.h"


enum
{
  PROP_0,
  PROP_PARAM
};


static void     picman_text_undo_constructed  (GObject             *object);
static void     picman_text_undo_set_property (GObject             *object,
                                             guint                property_id,
                                             const GValue        *value,
                                             GParamSpec          *pspec);
static void     picman_text_undo_get_property (GObject             *object,
                                             guint                property_id,
                                             GValue              *value,
                                             GParamSpec          *pspec);

static gint64   picman_text_undo_get_memsize  (PicmanObject          *object,
                                             gint64              *gui_size);

static void     picman_text_undo_pop          (PicmanUndo            *undo,
                                             PicmanUndoMode         undo_mode,
                                             PicmanUndoAccumulator *accum);
static void     picman_text_undo_free         (PicmanUndo            *undo,
                                             PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanTextUndo, picman_text_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_text_undo_parent_class


static void
picman_text_undo_class_init (PicmanTextUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_text_undo_constructed;
  object_class->set_property     = picman_text_undo_set_property;
  object_class->get_property     = picman_text_undo_get_property;

  picman_object_class->get_memsize = picman_text_undo_get_memsize;

  undo_class->pop                = picman_text_undo_pop;
  undo_class->free               = picman_text_undo_free;

  g_object_class_install_property (object_class, PROP_PARAM,
                                   g_param_spec_param ("param", NULL, NULL,
                                                       G_TYPE_PARAM,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_text_undo_init (PicmanTextUndo *undo)
{
}

static void
picman_text_undo_constructed (GObject *object)
{
  PicmanTextUndo  *text_undo = PICMAN_TEXT_UNDO (object);
  PicmanTextLayer *layer;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_TEXT_LAYER (PICMAN_ITEM_UNDO (text_undo)->item));

  layer = PICMAN_TEXT_LAYER (PICMAN_ITEM_UNDO (text_undo)->item);

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_TEXT_LAYER:
      if (text_undo->pspec)
        {
          g_assert (text_undo->pspec->owner_type == PICMAN_TYPE_TEXT);

          text_undo->value = g_slice_new0 (GValue);

          g_value_init (text_undo->value, text_undo->pspec->value_type);
          g_object_get_property (G_OBJECT (layer->text),
                                 text_undo->pspec->name, text_undo->value);
        }
      else if (layer->text)
        {
          text_undo->text = picman_config_duplicate (PICMAN_CONFIG (layer->text));
        }
      break;

    case PICMAN_UNDO_TEXT_LAYER_MODIFIED:
      text_undo->modified = layer->modified;
      break;

    case PICMAN_UNDO_TEXT_LAYER_CONVERT:
      text_undo->format = picman_drawable_get_format (PICMAN_DRAWABLE (layer));
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_text_undo_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanTextUndo *text_undo = PICMAN_TEXT_UNDO (object);

  switch (property_id)
    {
    case PROP_PARAM:
      text_undo->pspec = g_value_get_param (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_text_undo_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanTextUndo *text_undo = PICMAN_TEXT_UNDO (object);

  switch (property_id)
    {
    case PROP_PARAM:
      g_value_set_param (value, (GParamSpec *) text_undo->pspec);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_text_undo_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanTextUndo *undo    = PICMAN_TEXT_UNDO (object);
  gint64        memsize = 0;

  memsize += picman_g_value_get_memsize (undo->value);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (undo->text), NULL);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_text_undo_pop (PicmanUndo            *undo,
                    PicmanUndoMode         undo_mode,
                    PicmanUndoAccumulator *accum)
{
  PicmanTextUndo  *text_undo = PICMAN_TEXT_UNDO (undo);
  PicmanTextLayer *layer     = PICMAN_TEXT_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_TEXT_LAYER:
      if (text_undo->pspec)
        {
          GValue *value;

          g_return_if_fail (layer->text != NULL);

          value = g_slice_new0 (GValue);
          g_value_init (value, text_undo->pspec->value_type);

          g_object_get_property (G_OBJECT (layer->text),
                                 text_undo->pspec->name, value);

          g_object_set_property (G_OBJECT (layer->text),
                                 text_undo->pspec->name, text_undo->value);

          g_value_unset (text_undo->value);
          g_slice_free (GValue, text_undo->value);

          text_undo->value = value;
        }
      else
        {
          PicmanText *text;

          text = (layer->text ?
                  picman_config_duplicate (PICMAN_CONFIG (layer->text)) : NULL);

          if (layer->text && text_undo->text)
            picman_config_sync (G_OBJECT (text_undo->text),
                              G_OBJECT (layer->text), 0);
          else
            picman_text_layer_set_text (layer, text_undo->text);

          if (text_undo->text)
            g_object_unref (text_undo->text);

          text_undo->text = text;
        }
      break;

    case PICMAN_UNDO_TEXT_LAYER_MODIFIED:
      {
        gboolean modified;

#if 0
        g_print ("setting layer->modified from %s to %s\n",
                 layer->modified ? "TRUE" : "FALSE",
                 text_undo->modified ? "TRUE" : "FALSE");
#endif

        modified = layer->modified;
        g_object_set (layer, "modified", text_undo->modified, NULL);
        text_undo->modified = modified;

        picman_viewable_invalidate_preview (PICMAN_VIEWABLE (layer));
      }
      break;

    case PICMAN_UNDO_TEXT_LAYER_CONVERT:
      {
        const Babl *format;

        format = picman_drawable_get_format (PICMAN_DRAWABLE (layer));
        picman_drawable_convert_type (PICMAN_DRAWABLE (layer),
                                    picman_item_get_image (PICMAN_ITEM (layer)),
                                    picman_babl_format_get_base_type (text_undo->format),
                                    picman_babl_format_get_precision (text_undo->format),
                                    0, 0, FALSE);
        text_undo->format = format;
      }
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_text_undo_free (PicmanUndo     *undo,
                     PicmanUndoMode  undo_mode)
{
  PicmanTextUndo *text_undo = PICMAN_TEXT_UNDO (undo);

  if (text_undo->text)
    {
      g_object_unref (text_undo->text);
      text_undo->text = NULL;
    }

  if (text_undo->pspec)
    {
      g_value_unset (text_undo->value);
      g_slice_free (GValue, text_undo->value);

      text_undo->value = NULL;
      text_undo->pspec = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
