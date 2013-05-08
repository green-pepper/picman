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

#include "paint-types.h"

#include "picmanpaintcore.h"
#include "picmanpaintcoreundo.h"


enum
{
  PROP_0,
  PROP_PAINT_CORE
};


static void   picman_paint_core_undo_constructed  (GObject             *object);
static void   picman_paint_core_undo_set_property (GObject             *object,
                                                 guint                property_id,
                                                 const GValue        *value,
                                                 GParamSpec          *pspec);
static void   picman_paint_core_undo_get_property (GObject             *object,
                                                 guint                property_id,
                                                 GValue              *value,
                                                 GParamSpec          *pspec);

static void   picman_paint_core_undo_pop          (PicmanUndo            *undo,
                                                 PicmanUndoMode         undo_mode,
                                                 PicmanUndoAccumulator *accum);
static void   picman_paint_core_undo_free         (PicmanUndo            *undo,
                                                 PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanPaintCoreUndo, picman_paint_core_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_paint_core_undo_parent_class


static void
picman_paint_core_undo_class_init (PicmanPaintCoreUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed  = picman_paint_core_undo_constructed;
  object_class->set_property = picman_paint_core_undo_set_property;
  object_class->get_property = picman_paint_core_undo_get_property;

  undo_class->pop            = picman_paint_core_undo_pop;
  undo_class->free           = picman_paint_core_undo_free;

  g_object_class_install_property (object_class, PROP_PAINT_CORE,
                                   g_param_spec_object ("paint-core", NULL, NULL,
                                                        PICMAN_TYPE_PAINT_CORE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_paint_core_undo_init (PicmanPaintCoreUndo *undo)
{
}

static void
picman_paint_core_undo_constructed (GObject *object)
{
  PicmanPaintCoreUndo *paint_core_undo = PICMAN_PAINT_CORE_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PAINT_CORE (paint_core_undo->paint_core));

  paint_core_undo->last_coords = paint_core_undo->paint_core->start_coords;

  g_object_add_weak_pointer (G_OBJECT (paint_core_undo->paint_core),
                             (gpointer) &paint_core_undo->paint_core);
}

static void
picman_paint_core_undo_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanPaintCoreUndo *paint_core_undo = PICMAN_PAINT_CORE_UNDO (object);

  switch (property_id)
    {
    case PROP_PAINT_CORE:
      paint_core_undo->paint_core = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_paint_core_undo_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanPaintCoreUndo *paint_core_undo = PICMAN_PAINT_CORE_UNDO (object);

  switch (property_id)
    {
    case PROP_PAINT_CORE:
      g_value_set_object (value, paint_core_undo->paint_core);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_paint_core_undo_pop (PicmanUndo              *undo,
                          PicmanUndoMode           undo_mode,
                          PicmanUndoAccumulator   *accum)
{
  PicmanPaintCoreUndo *paint_core_undo = PICMAN_PAINT_CORE_UNDO (undo);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  /*  only pop if the core still exists  */
  if (paint_core_undo->paint_core)
    {
      PicmanCoords tmp_coords;

      tmp_coords = paint_core_undo->paint_core->last_coords;
      paint_core_undo->paint_core->last_coords = paint_core_undo->last_coords;
      paint_core_undo->last_coords = tmp_coords;
    }
}

static void
picman_paint_core_undo_free (PicmanUndo     *undo,
                           PicmanUndoMode  undo_mode)
{
  PicmanPaintCoreUndo *paint_core_undo = PICMAN_PAINT_CORE_UNDO (undo);

  if (paint_core_undo->paint_core)
    {
      g_object_remove_weak_pointer (G_OBJECT (paint_core_undo->paint_core),
                                    (gpointer) &paint_core_undo->paint_core);
      paint_core_undo->paint_core = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
