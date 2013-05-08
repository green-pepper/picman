/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanitem-exclusive.c
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#include "picmancontext.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanitem.h"
#include "picmanitem-exclusive.h"
#include "picmanitemstack.h"
#include "picmanitemtree.h"
#include "picmanundostack.h"

#include "picman-intl.h"


static GList * picman_item_exclusive_get_ancestry (PicmanItem     *item);
static void    picman_item_exclusive_get_lists    (PicmanItem     *item,
                                                 const gchar  *property,
                                                 GList       **on,
                                                 GList       **off);


/*  public functions  */

void
picman_item_toggle_exclusive_visible (PicmanItem    *item,
                                    PicmanContext *context)
{
  GList *ancestry;
  GList *on;
  GList *off;
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  ancestry = picman_item_exclusive_get_ancestry (item);
  picman_item_exclusive_get_lists (item, "visible", &on, &off);

  if (on || off || ! picman_item_is_visible (item))
    {
      PicmanImage *image = picman_item_get_image (item);
      PicmanUndo  *undo;
      gboolean   push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_UNDO_STACK,
                                           PICMAN_UNDO_GROUP_ITEM_VISIBILITY);

      if (undo && (g_object_get_data (G_OBJECT (undo), "exclusive-visible-item") ==
                   (gpointer) item))
        push_undo = FALSE;

      if (push_undo)
        {
          if (picman_image_undo_group_start (image,
                                           PICMAN_UNDO_GROUP_ITEM_VISIBILITY,
                                           _("Set Item Exclusive Visible")))
            {
              undo = picman_image_undo_can_compress (image, PICMAN_TYPE_UNDO_STACK,
                                                   PICMAN_UNDO_GROUP_ITEM_VISIBILITY);

              if (undo)
                g_object_set_data (G_OBJECT (undo), "exclusive-visible-item",
                                   (gpointer) item);
            }

          for (list = ancestry; list; list = g_list_next (list))
            picman_image_undo_push_item_visibility (image, NULL, list->data);

          for (list = on; list; list = g_list_next (list))
            picman_image_undo_push_item_visibility (image, NULL, list->data);

          for (list = off; list; list = g_list_next (list))
            picman_image_undo_push_item_visibility (image, NULL, list->data);

          picman_image_undo_group_end (image);
        }
      else
        {
          picman_undo_refresh_preview (undo, context);
        }

      for (list = ancestry; list; list = g_list_next (list))
        picman_item_set_visible (list->data, TRUE, FALSE);

      if (on)
        {
          for (list = on; list; list = g_list_next (list))
            picman_item_set_visible (list->data, FALSE, FALSE);
        }
      else if (off)
        {
          for (list = off; list; list = g_list_next (list))
            picman_item_set_visible (list->data, TRUE, FALSE);
        }

      g_list_free (on);
      g_list_free (off);
    }

  g_list_free (ancestry);
}

void
picman_item_toggle_exclusive_linked (PicmanItem    *item,
                                   PicmanContext *context)
{
  GList *on  = NULL;
  GList *off = NULL;
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  for (list = picman_item_get_container_iter (item);
       list;
       list = g_list_next (list))
    {
      PicmanItem *other = list->data;

      if (other != item)
        {
          if (picman_item_get_linked (other))
            on = g_list_prepend (on, other);
          else
            off = g_list_prepend (off, other);
        }
    }

  if (on || off || ! picman_item_get_linked (item))
    {
      PicmanImage *image = picman_item_get_image (item);
      PicmanUndo  *undo;
      gboolean   push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_UNDO_STACK,
                                           PICMAN_UNDO_GROUP_ITEM_LINKED);

      if (undo && (g_object_get_data (G_OBJECT (undo), "exclusive-linked-item") ==
                   (gpointer) item))
        push_undo = FALSE;

      if (push_undo)
        {
          if (picman_image_undo_group_start (image,
                                           PICMAN_UNDO_GROUP_ITEM_LINKED,
                                           _("Set Item Exclusive Linked")))
            {
              undo = picman_image_undo_can_compress (image, PICMAN_TYPE_UNDO_STACK,
                                                   PICMAN_UNDO_GROUP_ITEM_LINKED);

              if (undo)
                g_object_set_data (G_OBJECT (undo), "exclusive-linked-item",
                                   (gpointer) item);
            }

          picman_image_undo_push_item_linked (image, NULL, item);

          for (list = on; list; list = g_list_next (list))
            picman_image_undo_push_item_linked (image, NULL, list->data);

          for (list = off; list; list = g_list_next (list))
            picman_image_undo_push_item_linked (image, NULL, list->data);

          picman_image_undo_group_end (image);
        }
      else
        {
          picman_undo_refresh_preview (undo, context);
        }

      if (off || ! picman_item_get_linked (item))
        {
          picman_item_set_linked (item, TRUE, FALSE);

          for (list = off; list; list = g_list_next (list))
            picman_item_set_linked (list->data, TRUE, FALSE);
        }
      else
        {
          for (list = on; list; list = g_list_next (list))
            picman_item_set_linked (list->data, FALSE, FALSE);
        }

      g_list_free (on);
      g_list_free (off);
    }
}


/*  private functions  */

static GList *
picman_item_exclusive_get_ancestry (PicmanItem *item)
{
  PicmanViewable *parent;
  GList        *ancestry = NULL;

  for (parent = PICMAN_VIEWABLE (item);
       parent;
       parent = picman_viewable_get_parent (parent))
    {
      ancestry = g_list_prepend (ancestry, parent);
    }

  return ancestry;
}

static void
picman_item_exclusive_get_lists (PicmanItem     *item,
                               const gchar  *property,
                               GList       **on,
                               GList       **off)
{
  PicmanItemTree *tree;
  GList        *items;
  GList        *list;

  *on  = NULL;
  *off = NULL;

  tree = picman_item_get_tree (item);

  items = picman_item_stack_get_item_list (PICMAN_ITEM_STACK (tree->container));

  for (list = items; list; list = g_list_next (list))
    {
      PicmanItem *other = list->data;

      if (other != item)
        {
          /* we are only interested in toplevel items that are not
           * item's ancestor
           */
          if (! picman_viewable_get_parent (PICMAN_VIEWABLE (other)) &&
              ! picman_viewable_is_ancestor (PICMAN_VIEWABLE (other),
                                           PICMAN_VIEWABLE (item)))
            {
              gboolean value;

              g_object_get (other, property, &value, NULL);

              if (value)
                *on = g_list_prepend (*on, other);
              else
                *off = g_list_prepend (*off, other);
            }
        }
    }

  g_list_free (items);
}
