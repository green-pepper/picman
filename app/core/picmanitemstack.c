/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanitemstack.c
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#include <string.h>

#include <gegl.h>

#include "core-types.h"

#include "picmanitem.h"
#include "picmanitemstack.h"


/*  local function prototypes  */

static void   picman_item_stack_constructed (GObject       *object);

static void   picman_item_stack_add         (PicmanContainer *container,
                                           PicmanObject    *object);
static void   picman_item_stack_remove      (PicmanContainer *container,
                                           PicmanObject    *object);


G_DEFINE_TYPE (PicmanItemStack, picman_item_stack, PICMAN_TYPE_FILTER_STACK)

#define parent_class picman_item_stack_parent_class


static void
picman_item_stack_class_init (PicmanItemStackClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  PicmanContainerClass *container_class = PICMAN_CONTAINER_CLASS (klass);

  object_class->constructed = picman_item_stack_constructed;

  container_class->add      = picman_item_stack_add;
  container_class->remove   = picman_item_stack_remove;
}

static void
picman_item_stack_init (PicmanItemStack *stack)
{
}

static void
picman_item_stack_constructed (GObject *object)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (g_type_is_a (picman_container_get_children_type (container),
                         PICMAN_TYPE_ITEM));
}

static void
picman_item_stack_add (PicmanContainer *container,
                     PicmanObject    *object)
{
  g_object_ref_sink (object);

  PICMAN_CONTAINER_CLASS (parent_class)->add (container, object);

  g_object_unref (object);
}

static void
picman_item_stack_remove (PicmanContainer *container,
                        PicmanObject    *object)
{
  PICMAN_CONTAINER_CLASS (parent_class)->remove (container, object);
}


/*  public functions  */

PicmanContainer *
picman_item_stack_new (GType item_type)
{
  g_return_val_if_fail (g_type_is_a (item_type, PICMAN_TYPE_ITEM), NULL);

  return g_object_new (PICMAN_TYPE_ITEM_STACK,
                       "name",          g_type_name (item_type),
                       "children-type", item_type,
                       "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                       NULL);
}

gint
picman_item_stack_get_n_items (PicmanItemStack *stack)
{
  GList *list;
  gint   n_items = 0;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), 0);

  for (list = PICMAN_LIST (stack)->list; list; list = g_list_next (list))
    {
      PicmanItem      *item = list->data;
      PicmanContainer *children;

      n_items++;

      children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

      if (children)
        n_items += picman_item_stack_get_n_items (PICMAN_ITEM_STACK (children));
    }

  return n_items;
}

gboolean
picman_item_stack_is_flat (PicmanItemStack *stack)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), TRUE);

  for (list = PICMAN_LIST (stack)->list; list; list = g_list_next (list))
    {
      PicmanViewable *viewable = list->data;

      if (picman_viewable_get_children (viewable))
        return FALSE;
    }

  return TRUE;
}

GList *
picman_item_stack_get_item_iter (PicmanItemStack *stack)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), NULL);

  return PICMAN_LIST (stack)->list;
}

GList *
picman_item_stack_get_item_list (PicmanItemStack *stack)
{
  GList *list;
  GList *result = NULL;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), NULL);

  for (list = PICMAN_LIST (stack)->list;
       list;
       list = g_list_next (list))
    {
      PicmanViewable  *viewable = list->data;
      PicmanContainer *children;

      result = g_list_prepend (result, viewable);

      children = picman_viewable_get_children (viewable);

      if (children)
        {
          GList *child_list;

          child_list = picman_item_stack_get_item_list (PICMAN_ITEM_STACK (children));

          while (child_list)
            {
              result = g_list_prepend (result, child_list->data);

              child_list = g_list_remove (child_list, child_list->data);
            }
        }
    }

  return g_list_reverse (result);
}

PicmanItem *
picman_item_stack_get_item_by_tattoo (PicmanItemStack *stack,
                                    PicmanTattoo     tattoo)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), NULL);

  for (list = PICMAN_LIST (stack)->list; list; list = g_list_next (list))
    {
      PicmanItem      *item = list->data;
      PicmanContainer *children;

      if (picman_item_get_tattoo (item) == tattoo)
        return item;

      children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

      if (children)
        {
          item = picman_item_stack_get_item_by_tattoo (PICMAN_ITEM_STACK (children),
                                                     tattoo);

          if (item)
            return item;
        }
    }

  return NULL;
}

PicmanItem *
picman_item_stack_get_item_by_path (PicmanItemStack *stack,
                                  GList         *path)
{
  PicmanContainer *container;
  PicmanItem      *item = NULL;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), NULL);
  g_return_val_if_fail (path != NULL, NULL);

  container = PICMAN_CONTAINER (stack);

  while (path)
    {
      guint32 i = GPOINTER_TO_UINT (path->data);

      item = PICMAN_ITEM (picman_container_get_child_by_index (container, i));

      g_return_val_if_fail (PICMAN_IS_ITEM (item), item);

      if (path->next)
        {
          container = picman_viewable_get_children (PICMAN_VIEWABLE (item));

          g_return_val_if_fail (PICMAN_IS_ITEM_STACK (container), item);
        }

      path = path->next;
    }

  return item;
}

PicmanItem *
picman_item_stack_get_parent_by_path (PicmanItemStack *stack,
                                    GList         *path,
                                    gint          *index)
{
  PicmanItem *parent = NULL;
  guint32   i;

  g_return_val_if_fail (PICMAN_IS_ITEM_STACK (stack), NULL);
  g_return_val_if_fail (path != NULL, NULL);

  i = GPOINTER_TO_UINT (path->data);

  if (index)
    *index = i;

  while (path->next)
    {
      PicmanObject    *child;
      PicmanContainer *children;

      child = picman_container_get_child_by_index (PICMAN_CONTAINER (stack), i);

      g_return_val_if_fail (PICMAN_IS_ITEM (child), parent);

      children = picman_viewable_get_children (PICMAN_VIEWABLE (child));

      g_return_val_if_fail (PICMAN_IS_ITEM_STACK (children), parent);

      parent = PICMAN_ITEM (child);
      stack  = PICMAN_ITEM_STACK (children);

      path = path->next;

      i = GPOINTER_TO_UINT (path->data);

      if (index)
        *index = i;
    }

  return parent;
}

static void
picman_item_stack_invalidate_preview (PicmanViewable *viewable)
{
  PicmanContainer *children = picman_viewable_get_children (viewable);

  if (children)
    picman_item_stack_invalidate_previews (PICMAN_ITEM_STACK (children));

  picman_viewable_invalidate_preview (viewable);
}

void
picman_item_stack_invalidate_previews (PicmanItemStack *stack)
{
  g_return_if_fail (PICMAN_IS_ITEM_STACK (stack));

  picman_container_foreach (PICMAN_CONTAINER (stack),
                          (GFunc) picman_item_stack_invalidate_preview,
                          NULL);
}
