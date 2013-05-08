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

#include <time.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picmancontext.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanmarshal.h"
#include "picmantempbuf.h"
#include "picmanundo.h"
#include "picmanundostack.h"

#include "picman-intl.h"


enum
{
  POP,
  FREE,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_IMAGE,
  PROP_TIME,
  PROP_UNDO_TYPE,
  PROP_DIRTY_MASK
};


static void          picman_undo_constructed         (GObject             *object);
static void          picman_undo_finalize            (GObject             *object);
static void          picman_undo_set_property        (GObject             *object,
                                                    guint                property_id,
                                                    const GValue        *value,
                                                    GParamSpec          *pspec);
static void          picman_undo_get_property        (GObject             *object,
                                                    guint                property_id,
                                                    GValue              *value,
                                                    GParamSpec          *pspec);

static gint64        picman_undo_get_memsize         (PicmanObject          *object,
                                                    gint64              *gui_size);

static gboolean      picman_undo_get_popup_size      (PicmanViewable        *viewable,
                                                    gint                 width,
                                                    gint                 height,
                                                    gboolean             dot_for_dot,
                                                    gint                *popup_width,
                                                    gint                *popup_height);
static PicmanTempBuf * picman_undo_get_new_preview     (PicmanViewable        *viewable,
                                                    PicmanContext         *context,
                                                    gint                 width,
                                                    gint                 height);

static void          picman_undo_real_pop            (PicmanUndo            *undo,
                                                    PicmanUndoMode         undo_mode,
                                                    PicmanUndoAccumulator *accum);
static void          picman_undo_real_free           (PicmanUndo            *undo,
                                                    PicmanUndoMode         undo_mode);

static gboolean      picman_undo_create_preview_idle (gpointer             data);
static void       picman_undo_create_preview_private (PicmanUndo            *undo,
                                                    PicmanContext         *context);


G_DEFINE_TYPE (PicmanUndo, picman_undo, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_undo_parent_class

static guint undo_signals[LAST_SIGNAL] = { 0 };


static void
picman_undo_class_init (PicmanUndoClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);

  undo_signals[POP] =
    g_signal_new ("pop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanUndoClass, pop),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM_POINTER,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_UNDO_MODE,
                  G_TYPE_POINTER);

  undo_signals[FREE] =
    g_signal_new ("free",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanUndoClass, free),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_UNDO_MODE);

  object_class->constructed        = picman_undo_constructed;
  object_class->finalize           = picman_undo_finalize;
  object_class->set_property       = picman_undo_set_property;
  object_class->get_property       = picman_undo_get_property;

  picman_object_class->get_memsize   = picman_undo_get_memsize;

  viewable_class->default_stock_id = "gtk-undo";
  viewable_class->get_popup_size   = picman_undo_get_popup_size;
  viewable_class->get_new_preview  = picman_undo_get_new_preview;

  klass->pop                       = picman_undo_real_pop;
  klass->free                      = picman_undo_real_free;

  g_object_class_install_property (object_class, PROP_IMAGE,
                                   g_param_spec_object ("image", NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_TIME,
                                   g_param_spec_uint ("time", NULL, NULL,
                                                      0, G_MAXUINT, 0,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_UNDO_TYPE,
                                   g_param_spec_enum ("undo-type", NULL, NULL,
                                                      PICMAN_TYPE_UNDO_TYPE,
                                                      PICMAN_UNDO_GROUP_NONE,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DIRTY_MASK,
                                   g_param_spec_flags ("dirty-mask",
                                                       NULL, NULL,
                                                       PICMAN_TYPE_DIRTY_MASK,
                                                       PICMAN_DIRTY_NONE,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_undo_init (PicmanUndo *undo)
{
  undo->time = time (NULL);
}

static void
picman_undo_constructed (GObject *object)
{
  PicmanUndo *undo = PICMAN_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_IMAGE (undo->image));
}

static void
picman_undo_finalize (GObject *object)
{
  PicmanUndo *undo = PICMAN_UNDO (object);

  if (undo->preview_idle_id)
    {
      g_source_remove (undo->preview_idle_id);
      undo->preview_idle_id = 0;
    }

  if (undo->preview)
    {
      picman_temp_buf_unref (undo->preview);
      undo->preview = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_undo_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanUndo *undo = PICMAN_UNDO (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      /* don't ref */
      undo->image = g_value_get_object (value);
      break;
    case PROP_TIME:
      undo->time = g_value_get_uint (value);
      break;
    case PROP_UNDO_TYPE:
      undo->undo_type = g_value_get_enum (value);
      break;
    case PROP_DIRTY_MASK:
      undo->dirty_mask = g_value_get_flags (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_undo_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  PicmanUndo *undo = PICMAN_UNDO (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      g_value_set_object (value, undo->image);
      break;
    case PROP_TIME:
      g_value_set_uint (value, undo->time);
      break;
    case PROP_UNDO_TYPE:
      g_value_set_enum (value, undo->undo_type);
      break;
    case PROP_DIRTY_MASK:
      g_value_set_flags (value, undo->dirty_mask);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_undo_get_memsize (PicmanObject *object,
                       gint64     *gui_size)
{
  PicmanUndo *undo    = PICMAN_UNDO (object);
  gint64    memsize = 0;

  *gui_size += picman_temp_buf_get_memsize (undo->preview);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_undo_get_popup_size (PicmanViewable *viewable,
                          gint          width,
                          gint          height,
                          gboolean      dot_for_dot,
                          gint         *popup_width,
                          gint         *popup_height)
{
  PicmanUndo *undo = PICMAN_UNDO (viewable);

  if (undo->preview &&
      (picman_temp_buf_get_width  (undo->preview) > width ||
       picman_temp_buf_get_height (undo->preview) > height))
    {
      *popup_width  = picman_temp_buf_get_width  (undo->preview);
      *popup_height = picman_temp_buf_get_height (undo->preview);

      return TRUE;
    }

  return FALSE;
}

static PicmanTempBuf *
picman_undo_get_new_preview (PicmanViewable *viewable,
                           PicmanContext  *context,
                           gint          width,
                           gint          height)
{
  PicmanUndo *undo = PICMAN_UNDO (viewable);

  if (undo->preview)
    {
      gint preview_width;
      gint preview_height;

      picman_viewable_calc_preview_size (picman_temp_buf_get_width  (undo->preview),
                                       picman_temp_buf_get_height (undo->preview),
                                       width,
                                       height,
                                       TRUE, 1.0, 1.0,
                                       &preview_width,
                                       &preview_height,
                                       NULL);

      if (preview_width  < picman_temp_buf_get_width  (undo->preview) &&
          preview_height < picman_temp_buf_get_height (undo->preview))
        {
          return picman_temp_buf_scale (undo->preview,
                                      preview_width, preview_height);
        }

      return picman_temp_buf_copy (undo->preview);
    }

  return NULL;
}

static void
picman_undo_real_pop (PicmanUndo            *undo,
                    PicmanUndoMode         undo_mode,
                    PicmanUndoAccumulator *accum)
{
}

static void
picman_undo_real_free (PicmanUndo     *undo,
                     PicmanUndoMode  undo_mode)
{
}

void
picman_undo_pop (PicmanUndo            *undo,
               PicmanUndoMode         undo_mode,
               PicmanUndoAccumulator *accum)
{
  g_return_if_fail (PICMAN_IS_UNDO (undo));
  g_return_if_fail (accum != NULL);

  if (undo->dirty_mask != PICMAN_DIRTY_NONE)
    {
      switch (undo_mode)
        {
        case PICMAN_UNDO_MODE_UNDO:
          picman_image_clean (undo->image, undo->dirty_mask);
          break;

        case PICMAN_UNDO_MODE_REDO:
          picman_image_dirty (undo->image, undo->dirty_mask);
          break;
        }
    }

  g_signal_emit (undo, undo_signals[POP], 0, undo_mode, accum);
}

void
picman_undo_free (PicmanUndo     *undo,
                PicmanUndoMode  undo_mode)
{
  g_return_if_fail (PICMAN_IS_UNDO (undo));

  g_signal_emit (undo, undo_signals[FREE], 0, undo_mode);
}

typedef struct _PicmanUndoIdle PicmanUndoIdle;

struct _PicmanUndoIdle
{
  PicmanUndo    *undo;
  PicmanContext *context;
};

static void
picman_undo_idle_free (PicmanUndoIdle *idle)
{
  if (idle->context)
    g_object_unref (idle->context);

  g_slice_free (PicmanUndoIdle, idle);
}

void
picman_undo_create_preview (PicmanUndo    *undo,
                          PicmanContext *context,
                          gboolean     create_now)
{
  g_return_if_fail (PICMAN_IS_UNDO (undo));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (undo->preview || undo->preview_idle_id)
    return;

  if (create_now)
    {
      picman_undo_create_preview_private (undo, context);
    }
  else
    {
      PicmanUndoIdle *idle = g_slice_new0 (PicmanUndoIdle);

      idle->undo = undo;

      if (context)
        idle->context = g_object_ref (context);

      undo->preview_idle_id =
        g_idle_add_full (PICMAN_VIEWABLE_PRIORITY_IDLE,
                         picman_undo_create_preview_idle, idle,
                         (GDestroyNotify) picman_undo_idle_free);
    }
}

static gboolean
picman_undo_create_preview_idle (gpointer data)
{
  PicmanUndoIdle *idle   = data;
  PicmanUndoStack *stack = picman_image_get_undo_stack (idle->undo->image);

  if (idle->undo == picman_undo_stack_peek (stack))
    {
      picman_undo_create_preview_private (idle->undo, idle->context);
    }

  idle->undo->preview_idle_id = 0;

  return FALSE;
}

static void
picman_undo_create_preview_private (PicmanUndo    *undo,
                                  PicmanContext *context)
{
  PicmanImage    *image = undo->image;
  PicmanViewable *preview_viewable;
  PicmanViewSize  preview_size;
  gint          width;
  gint          height;

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK:
    case PICMAN_UNDO_GROUP_MASK:
    case PICMAN_UNDO_MASK:
      preview_viewable = PICMAN_VIEWABLE (picman_image_get_mask (image));
      break;

    default:
      preview_viewable = PICMAN_VIEWABLE (image);
      break;
    }

  preview_size = image->picman->config->undo_preview_size;

  if (picman_image_get_width  (image) <= preview_size &&
      picman_image_get_height (image) <= preview_size)
    {
      width  = picman_image_get_width  (image);
      height = picman_image_get_height (image);
    }
  else
    {
      if (picman_image_get_width (image) > picman_image_get_height (image))
        {
          width  = preview_size;
          height = MAX (1, (picman_image_get_height (image) * preview_size /
                            picman_image_get_width (image)));
        }
      else
        {
          height = preview_size;
          width  = MAX (1, (picman_image_get_width (image) * preview_size /
                            picman_image_get_height (image)));
        }
    }

  undo->preview = picman_viewable_get_new_preview (preview_viewable, context,
                                                 width, height);

  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (undo));
}

void
picman_undo_refresh_preview (PicmanUndo    *undo,
                           PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_UNDO (undo));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (undo->preview_idle_id)
    return;

  if (undo->preview)
    {
      picman_temp_buf_unref (undo->preview);
      undo->preview = NULL;
      picman_undo_create_preview (undo, context, FALSE);
    }
}

const gchar *
picman_undo_type_to_name (PicmanUndoType type)
{
  const gchar *desc;

  if (picman_enum_get_value (PICMAN_TYPE_UNDO_TYPE, type, NULL, NULL, &desc, NULL))
    return desc;
  else
    return "";
}

gboolean
picman_undo_is_weak (PicmanUndo *undo)
{
  if (! undo)
    return FALSE;

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_GROUP_ITEM_VISIBILITY:
    case PICMAN_UNDO_GROUP_ITEM_PROPERTIES:
    case PICMAN_UNDO_GROUP_LAYER_APPLY_MASK:
    case PICMAN_UNDO_ITEM_VISIBILITY:
    case PICMAN_UNDO_LAYER_MODE:
    case PICMAN_UNDO_LAYER_OPACITY:
    case PICMAN_UNDO_LAYER_MASK_APPLY:
    case PICMAN_UNDO_LAYER_MASK_SHOW:
      return TRUE;
      break;

    default:
      break;
    }

  return FALSE;
}

/**
 * picman_undo_get_age:
 * @undo:
 *
 * Return value: the time in seconds since this undo item was created
 */
gint
picman_undo_get_age (PicmanUndo *undo)
{
  guint now = time (NULL);

  g_return_val_if_fail (PICMAN_IS_UNDO (undo), 0);
  g_return_val_if_fail (now >= undo->time, 0);

  return now - undo->time;
}

/**
 * picman_undo_reset_age:
 * @undo:
 *
 * Changes the creation time of this undo item to the current time.
 */
void
picman_undo_reset_age (PicmanUndo *undo)
{
  g_return_if_fail (PICMAN_IS_UNDO (undo));

  undo->time = time (NULL);

  g_object_notify (G_OBJECT (undo), "time");
}
