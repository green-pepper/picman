/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanplugin-cleanup.c
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

#include "plug-in-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmandrawable.h"
#include "core/picmandrawable-shadow.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanundostack.h"

#include "picmanplugin.h"
#include "picmanplugin-cleanup.h"
#include "picmanpluginmanager.h"
#include "picmanpluginprocedure.h"

#include "picman-log.h"


typedef struct _PicmanPlugInCleanupImage PicmanPlugInCleanupImage;

struct _PicmanPlugInCleanupImage
{
  PicmanImage *image;
  gint       image_ID;

  gint       undo_group_count;
};


typedef struct _PicmanPlugInCleanupItem PicmanPlugInCleanupItem;

struct _PicmanPlugInCleanupItem
{
  PicmanItem *item;
  gint      item_ID;

  gboolean  shadow_buffer;
};


/*  local function prototypes  */

static PicmanPlugInCleanupImage *
              picman_plug_in_cleanup_image_new  (PicmanImage              *image);
static void   picman_plug_in_cleanup_image_free (PicmanPlugInCleanupImage *cleanup);
static PicmanPlugInCleanupImage *
              picman_plug_in_cleanup_image_get  (PicmanPlugInProcFrame    *proc_frame,
                                               PicmanImage              *image);
static void   picman_plug_in_cleanup_image      (PicmanPlugInProcFrame    *proc_frame,
                                               PicmanPlugInCleanupImage *cleanup);

static PicmanPlugInCleanupItem *
              picman_plug_in_cleanup_item_new   (PicmanItem               *item);
static void   picman_plug_in_cleanup_item_free  (PicmanPlugInCleanupItem  *cleanup);
static PicmanPlugInCleanupItem *
              picman_plug_in_cleanup_item_get   (PicmanPlugInProcFrame    *proc_frame,
                                               PicmanItem               *item);
static void   picman_plug_in_cleanup_item       (PicmanPlugInProcFrame    *proc_frame,
                                               PicmanPlugInCleanupItem  *cleanup);


/*  public functions  */

gboolean
picman_plug_in_cleanup_undo_group_start (PicmanPlugIn *plug_in,
                                       PicmanImage  *image)
{
  PicmanPlugInProcFrame    *proc_frame;
  PicmanPlugInCleanupImage *cleanup;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);
  cleanup    = picman_plug_in_cleanup_image_get (proc_frame, image);

  if (! cleanup)
    {
      cleanup = picman_plug_in_cleanup_image_new (image);

      cleanup->undo_group_count = picman_image_get_undo_group_count (image);

      proc_frame->image_cleanups = g_list_prepend (proc_frame->image_cleanups,
                                                   cleanup);
    }

  return TRUE;
}

gboolean
picman_plug_in_cleanup_undo_group_end (PicmanPlugIn *plug_in,
                                     PicmanImage  *image)
{
  PicmanPlugInProcFrame    *proc_frame;
  PicmanPlugInCleanupImage *cleanup;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);
  cleanup    = picman_plug_in_cleanup_image_get (proc_frame, image);

  if (! cleanup)
    return FALSE;

  if (cleanup->undo_group_count == picman_image_get_undo_group_count (image) - 1)
    {
      proc_frame->image_cleanups = g_list_remove (proc_frame->image_cleanups,
                                                  cleanup);
      picman_plug_in_cleanup_image_free (cleanup);
    }

  return TRUE;
}

gboolean
picman_plug_in_cleanup_add_shadow (PicmanPlugIn   *plug_in,
                                 PicmanDrawable *drawable)
{
  PicmanPlugInProcFrame   *proc_frame;
  PicmanPlugInCleanupItem *cleanup;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);
  cleanup    = picman_plug_in_cleanup_item_get (proc_frame, PICMAN_ITEM (drawable));

  if (! cleanup)
    {
      cleanup = picman_plug_in_cleanup_item_new (PICMAN_ITEM (drawable));

      proc_frame->item_cleanups = g_list_prepend (proc_frame->item_cleanups,
                                                  cleanup);
    }

  cleanup->shadow_buffer = TRUE;

  return TRUE;
}

gboolean
picman_plug_in_cleanup_remove_shadow (PicmanPlugIn   *plug_in,
                                    PicmanDrawable *drawable)
{
  PicmanPlugInProcFrame   *proc_frame;
  PicmanPlugInCleanupItem *cleanup;

  g_return_val_if_fail (PICMAN_IS_PLUG_IN (plug_in), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  proc_frame = picman_plug_in_get_proc_frame (plug_in);
  cleanup    = picman_plug_in_cleanup_item_get (proc_frame, PICMAN_ITEM (drawable));

  if (! cleanup)
    return FALSE;

  if (! cleanup->shadow_buffer)
    return FALSE;

  proc_frame->item_cleanups = g_list_remove (proc_frame->item_cleanups,
                                             cleanup);
  picman_plug_in_cleanup_item_free (cleanup);

  return TRUE;
}

void
picman_plug_in_cleanup (PicmanPlugIn          *plug_in,
                      PicmanPlugInProcFrame *proc_frame)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));
  g_return_if_fail (proc_frame != NULL);

  for (list = proc_frame->image_cleanups; list; list = g_list_next (list))
    {
      PicmanPlugInCleanupImage *cleanup = list->data;

      if (picman_image_get_by_ID (plug_in->manager->picman,
                                cleanup->image_ID) == cleanup->image)
        {
          picman_plug_in_cleanup_image (proc_frame, cleanup);
        }

      picman_plug_in_cleanup_image_free (cleanup);
    }

  g_list_free (proc_frame->image_cleanups);
  proc_frame->image_cleanups = NULL;

  for (list = proc_frame->item_cleanups; list; list = g_list_next (list))
    {
      PicmanPlugInCleanupItem *cleanup = list->data;

      if (picman_item_get_by_ID (plug_in->manager->picman,
                               cleanup->item_ID) == cleanup->item)
        {
          picman_plug_in_cleanup_item (proc_frame, cleanup);
        }

      picman_plug_in_cleanup_item_free (cleanup);
    }

  g_list_free (proc_frame->item_cleanups);
  proc_frame->item_cleanups = NULL;
}


/*  private functions  */

static PicmanPlugInCleanupImage *
picman_plug_in_cleanup_image_new (PicmanImage *image)
{
  PicmanPlugInCleanupImage *cleanup = g_slice_new0 (PicmanPlugInCleanupImage);

  cleanup->image    = image;
  cleanup->image_ID = picman_image_get_ID (image);

  return cleanup;
}

static void
picman_plug_in_cleanup_image_free (PicmanPlugInCleanupImage *cleanup)
{
  g_slice_free (PicmanPlugInCleanupImage, cleanup);
}

static PicmanPlugInCleanupImage *
picman_plug_in_cleanup_image_get (PicmanPlugInProcFrame *proc_frame,
                                PicmanImage           *image)
{
  GList *list;

  for (list = proc_frame->image_cleanups; list; list = g_list_next (list))
    {
      PicmanPlugInCleanupImage *cleanup = list->data;

      if (cleanup->image == image)
        return cleanup;
    }

  return NULL;
}

static void
picman_plug_in_cleanup_image (PicmanPlugInProcFrame    *proc_frame,
                            PicmanPlugInCleanupImage *cleanup)
{
  PicmanImage *image = cleanup->image;

  if (picman_image_get_undo_group_count (image) == 0)
    return;

  if (cleanup->undo_group_count != picman_image_get_undo_group_count (image))
    {
      PicmanProcedure *proc = proc_frame->procedure;

      g_message ("Plug-In '%s' left image undo in inconsistent state, "
                 "closing open undo groups.",
                 picman_plug_in_procedure_get_label (PICMAN_PLUG_IN_PROCEDURE (proc)));

      while (cleanup->undo_group_count < picman_image_get_undo_group_count (image))
        {
          if (! picman_image_undo_group_end (image))
            break;
        }
    }
}

static PicmanPlugInCleanupItem *
picman_plug_in_cleanup_item_new (PicmanItem *item)
{
  PicmanPlugInCleanupItem *cleanup = g_slice_new0 (PicmanPlugInCleanupItem);

  cleanup->item    = item;
  cleanup->item_ID = picman_item_get_ID (item);

  return cleanup;
}

static void
picman_plug_in_cleanup_item_free (PicmanPlugInCleanupItem *cleanup)
{
  g_slice_free (PicmanPlugInCleanupItem, cleanup);
}

static PicmanPlugInCleanupItem *
picman_plug_in_cleanup_item_get (PicmanPlugInProcFrame *proc_frame,
                               PicmanItem            *item)
{
  GList *list;

  for (list = proc_frame->item_cleanups; list; list = g_list_next (list))
    {
      PicmanPlugInCleanupItem *cleanup = list->data;

      if (cleanup->item == item)
        return cleanup;
    }

  return NULL;
}

static void
picman_plug_in_cleanup_item (PicmanPlugInProcFrame   *proc_frame,
                           PicmanPlugInCleanupItem *cleanup)
{
  PicmanItem *item = cleanup->item;

  if (cleanup->shadow_buffer)
    {
      PicmanProcedure *proc = proc_frame->procedure;

      PICMAN_LOG (SHADOW_TILES,
                "Freeing shadow buffer of drawable '%s' on behalf of '%s'.",
                picman_object_get_name (item),
                picman_plug_in_procedure_get_label (PICMAN_PLUG_IN_PROCEDURE (proc)));

      picman_drawable_free_shadow_buffer (PICMAN_DRAWABLE (item));
    }
}
