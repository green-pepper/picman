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
#include <gtk/gtk.h>

#include "tools-types.h"

#include "picmantoolcontrol.h"
#include "picmantransformtool.h"
#include "picmantransformtoolundo.h"


enum
{
  PROP_0,
  PROP_TRANSFORM_TOOL
};


static void   picman_transform_tool_undo_constructed  (GObject             *object);
static void   picman_transform_tool_undo_set_property (GObject             *object,
                                                     guint                property_id,
                                                     const GValue        *value,
                                                     GParamSpec          *pspec);
static void   picman_transform_tool_undo_get_property (GObject             *object,
                                                     guint                property_id,
                                                     GValue              *value,
                                                     GParamSpec          *pspec);

static void   picman_transform_tool_undo_pop          (PicmanUndo            *undo,
                                                     PicmanUndoMode         undo_mode,
                                                     PicmanUndoAccumulator *accum);
static void   picman_transform_tool_undo_free         (PicmanUndo            *undo,
                                                     PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanTransformToolUndo, picman_transform_tool_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_transform_tool_undo_parent_class


static void
picman_transform_tool_undo_class_init (PicmanTransformToolUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed  = picman_transform_tool_undo_constructed;
  object_class->set_property = picman_transform_tool_undo_set_property;
  object_class->get_property = picman_transform_tool_undo_get_property;

  undo_class->pop            = picman_transform_tool_undo_pop;
  undo_class->free           = picman_transform_tool_undo_free;

  g_object_class_install_property (object_class, PROP_TRANSFORM_TOOL,
                                   g_param_spec_object ("transform-tool",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TRANSFORM_TOOL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_transform_tool_undo_init (PicmanTransformToolUndo *undo)
{
}

static void
picman_transform_tool_undo_constructed (GObject *object)
{
  PicmanTransformToolUndo *transform_tool_undo = PICMAN_TRANSFORM_TOOL_UNDO (object);
  PicmanTransformTool     *transform_tool;
  gint                   i;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_TRANSFORM_TOOL (transform_tool_undo->transform_tool));

  transform_tool = transform_tool_undo->transform_tool;

  for (i = 0; i < TRANS_INFO_SIZE; i++)
    transform_tool_undo->trans_info[i] = (*transform_tool->old_trans_info)[i];

#if 0
  if (transform_tool->original)
    transform_tool_undo->original = tile_manager_ref (transform_tool->original);
#endif

  g_object_add_weak_pointer (G_OBJECT (transform_tool_undo->transform_tool),
                             (gpointer) &transform_tool_undo->transform_tool);
}

static void
picman_transform_tool_undo_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanTransformToolUndo *transform_tool_undo = PICMAN_TRANSFORM_TOOL_UNDO (object);

  switch (property_id)
    {
    case PROP_TRANSFORM_TOOL:
      transform_tool_undo->transform_tool = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_transform_tool_undo_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanTransformToolUndo *transform_tool_undo = PICMAN_TRANSFORM_TOOL_UNDO (object);

  switch (property_id)
    {
    case PROP_TRANSFORM_TOOL:
      g_value_set_object (value, transform_tool_undo->transform_tool);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_transform_tool_undo_pop (PicmanUndo              *undo,
                              PicmanUndoMode           undo_mode,
                              PicmanUndoAccumulator   *accum)
{
  PicmanTransformToolUndo *transform_tool_undo = PICMAN_TRANSFORM_TOOL_UNDO (undo);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if (transform_tool_undo->transform_tool)
    {
      PicmanTransformTool *transform_tool;
#if 0
      TileManager       *temp;
#endif
      gdouble            d;
      gint               i;

      transform_tool = transform_tool_undo->transform_tool;

      /*  swap the transformation information arrays  */
      for (i = 0; i < TRANS_INFO_SIZE; i++)
        {
          d = transform_tool_undo->trans_info[i];
          transform_tool_undo->trans_info[i] = transform_tool->trans_info[i];
          transform_tool->trans_info[i] = d;
        }

#if 0
      /*  swap the original buffer--the source buffer for repeated transforms
       */
      temp                          = transform_tool_undo->original;
      transform_tool_undo->original = transform_tool->original;
      transform_tool->original      = temp;
#endif

#if 0
      /*  If we're re-implementing the first transform, reactivate tool  */
      if (undo_mode == PICMAN_UNDO_MODE_REDO && transform_tool->original)
        {
          picman_tool_control_activate (PICMAN_TOOL (transform_tool)->control);

          picman_draw_tool_resume (PICMAN_DRAW_TOOL (transform_tool));
        }
#endif
    }
 }

static void
picman_transform_tool_undo_free (PicmanUndo     *undo,
                               PicmanUndoMode  undo_mode)
{
  PicmanTransformToolUndo *transform_tool_undo = PICMAN_TRANSFORM_TOOL_UNDO (undo);

  if (transform_tool_undo->transform_tool)
    {
      g_object_remove_weak_pointer (G_OBJECT (transform_tool_undo->transform_tool),
                                    (gpointer) &transform_tool_undo->transform_tool);
      transform_tool_undo->transform_tool = NULL;
    }

#if 0
  if (transform_tool_undo->original)
    {
      tile_manager_unref (transform_tool_undo->original);
      transform_tool_undo->original = NULL;
    }
#endif

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
