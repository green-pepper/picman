/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanitemtree.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#include "core-types.h"

#include "picmanimage.h"
#include "picmanimage-undo-push.h"
#include "picmanitem.h"
#include "picmanitemstack.h"
#include "picmanitemtree.h"


enum
{
  PROP_0,
  PROP_IMAGE,
  PROP_CONTAINER_TYPE,
  PROP_ITEM_TYPE,
  PROP_ACTIVE_ITEM
};


typedef struct _PicmanItemTreePrivate PicmanItemTreePrivate;

struct _PicmanItemTreePrivate
{
  PicmanImage  *image;

  GType       container_type;
  GType       item_type;

  PicmanItem   *active_item;

  GHashTable *name_hash;
};

#define PICMAN_ITEM_TREE_GET_PRIVATE(object) \
        G_TYPE_INSTANCE_GET_PRIVATE (object, \
                                     PICMAN_TYPE_ITEM_TREE, \
                                     PicmanItemTreePrivate)


/*  local function prototypes  */

static void     picman_item_tree_constructed   (GObject      *object);
static void     picman_item_tree_finalize      (GObject      *object);
static void     picman_item_tree_set_property  (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec);
static void     picman_item_tree_get_property  (GObject      *object,
                                              guint         property_id,
                                              GValue       *value,
                                              GParamSpec   *pspec);

static gint64   picman_item_tree_get_memsize   (PicmanObject   *object,
                                              gint64       *gui_size);

static void     picman_item_tree_uniquefy_name (PicmanItemTree *tree,
                                              PicmanItem     *item,
                                              const gchar  *new_name);


G_DEFINE_TYPE (PicmanItemTree, picman_item_tree, PICMAN_TYPE_OBJECT)

#define parent_class picman_item_tree_parent_class


static void
picman_item_tree_class_init (PicmanItemTreeClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->constructed      = picman_item_tree_constructed;
  object_class->finalize         = picman_item_tree_finalize;
  object_class->set_property     = picman_item_tree_set_property;
  object_class->get_property     = picman_item_tree_get_property;

  picman_object_class->get_memsize = picman_item_tree_get_memsize;

  g_object_class_install_property (object_class, PROP_IMAGE,
                                   g_param_spec_object ("image",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTAINER_TYPE,
                                   g_param_spec_gtype ("container-type",
                                                       NULL, NULL,
                                                       PICMAN_TYPE_ITEM_STACK,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_ITEM_TYPE,
                                   g_param_spec_gtype ("item-type",
                                                       NULL, NULL,
                                                       PICMAN_TYPE_ITEM,
                                                       PICMAN_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_ACTIVE_ITEM,
                                   g_param_spec_object ("active-item",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_ITEM,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanItemTreePrivate));
}

static void
picman_item_tree_init (PicmanItemTree *tree)
{
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  private->name_hash = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
picman_item_tree_constructed (GObject *object)
{
  PicmanItemTree        *tree    = PICMAN_ITEM_TREE (object);
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_IMAGE (private->image));
  g_assert (g_type_is_a (private->container_type, PICMAN_TYPE_ITEM_STACK));
  g_assert (g_type_is_a (private->item_type,      PICMAN_TYPE_ITEM));
  g_assert (private->item_type != PICMAN_TYPE_ITEM);

  tree->container = g_object_new (private->container_type,
                                  "name",          g_type_name (private->item_type),
                                  "children-type", private->item_type,
                                  "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                                  NULL);
}

static void
picman_item_tree_finalize (GObject *object)
{
  PicmanItemTree        *tree    = PICMAN_ITEM_TREE (object);
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  if (private->name_hash)
    {
      g_hash_table_unref (private->name_hash);
      private->name_hash = NULL;
    }

  if (tree->container)
    {
      g_object_unref (tree->container);
      tree->container = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_item_tree_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      private->image = g_value_get_object (value); /* don't ref */
      break;
    case PROP_CONTAINER_TYPE:
      private->container_type = g_value_get_gtype (value);
      break;
    case PROP_ITEM_TYPE:
      private->item_type = g_value_get_gtype (value);
      break;
    case PROP_ACTIVE_ITEM:
      private->active_item = g_value_get_object (value); /* don't ref */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_item_tree_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IMAGE:
      g_value_set_object (value, private->image);
      break;
    case PROP_CONTAINER_TYPE:
      g_value_set_gtype (value, private->container_type);
      break;
    case PROP_ITEM_TYPE:
      g_value_set_gtype (value, private->item_type);
      break;
    case PROP_ACTIVE_ITEM:
      g_value_set_object (value, private->active_item);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_item_tree_get_memsize (PicmanObject *object,
                            gint64     *gui_size)
{
  PicmanItemTree *tree    = PICMAN_ITEM_TREE (object);
  gint64        memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (tree->container), gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}


/*  public functions  */

PicmanItemTree *
picman_item_tree_new (PicmanImage *image,
                    GType      container_type,
                    GType      item_type)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (g_type_is_a (container_type, PICMAN_TYPE_ITEM_STACK), NULL);
  g_return_val_if_fail (g_type_is_a (item_type, PICMAN_TYPE_ITEM), NULL);

  return g_object_new (PICMAN_TYPE_ITEM_TREE,
                       "image",          image,
                       "container-type", container_type,
                       "item-type",      item_type,
                       NULL);
}

PicmanItem *
picman_item_tree_get_active_item (PicmanItemTree *tree)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE (tree), NULL);

  return PICMAN_ITEM_TREE_GET_PRIVATE (tree)->active_item;
}

void
picman_item_tree_set_active_item (PicmanItemTree *tree,
                                PicmanItem     *item)
{
  PicmanItemTreePrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM_TREE (tree));

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_if_fail (item == NULL ||
                    G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type));
  g_return_if_fail (item == NULL || picman_item_get_tree (item) == tree);

  if (item != private->active_item)
    {
      private->active_item = item;

      g_object_notify (G_OBJECT (tree), "active-item");
    }
}

PicmanItem *
picman_item_tree_get_item_by_name (PicmanItemTree *tree,
                                 const gchar  *name)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE (tree), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return g_hash_table_lookup (PICMAN_ITEM_TREE_GET_PRIVATE (tree)->name_hash,
                              name);
}

gboolean
picman_item_tree_get_insert_pos (PicmanItemTree  *tree,
                               PicmanItem      *item,
                               PicmanItem     **parent,
                               gint          *position)
{
  PicmanItemTreePrivate *private;
  PicmanContainer       *container;

  g_return_val_if_fail (PICMAN_IS_ITEM_TREE (tree), FALSE);
  g_return_val_if_fail (parent != NULL, FALSE);

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type),
                        FALSE);
  g_return_val_if_fail (! picman_item_is_attached (item), FALSE);
  g_return_val_if_fail (picman_item_get_image (item) == private->image, FALSE);
  g_return_val_if_fail (*parent == NULL ||
                        *parent == PICMAN_IMAGE_ACTIVE_PARENT ||
                        G_TYPE_CHECK_INSTANCE_TYPE (*parent, private->item_type),
                        FALSE);
  g_return_val_if_fail (*parent == NULL ||
                        *parent == PICMAN_IMAGE_ACTIVE_PARENT ||
                        picman_item_get_tree (*parent) == tree, FALSE);
  g_return_val_if_fail (*parent == NULL ||
                        *parent == PICMAN_IMAGE_ACTIVE_PARENT ||
                        picman_viewable_get_children (PICMAN_VIEWABLE (*parent)),
                        FALSE);
  g_return_val_if_fail (position != NULL, FALSE);

  /*  if we want to insert in the active item's parent container  */
  if (*parent == PICMAN_IMAGE_ACTIVE_PARENT)
    {
      if (private->active_item)
        {
          /*  if the active item is a branch, add to the top of that
           *  branch; add to the active item's parent container
           *  otherwise
           */
          if (picman_viewable_get_children (PICMAN_VIEWABLE (private->active_item)))
            {
              *parent   = private->active_item;
              *position = 0;
            }
          else
            {
              *parent = picman_item_get_parent (private->active_item);
            }
        }
      else
        {
          /*  use the toplevel container if there is no active item  */
          *parent = NULL;
        }
    }

  if (*parent)
    container = picman_viewable_get_children (PICMAN_VIEWABLE (*parent));
  else
    container = tree->container;

  /*  if we want to add on top of the active item  */
  if (*position == -1)
    {
      if (private->active_item)
        *position =
          picman_container_get_child_index (container,
                                          PICMAN_OBJECT (private->active_item));

      /*  if the active item is not in the specified parent container,
       *  fall back to index 0
       */
      if (*position == -1)
        *position = 0;
    }

  /*  don't add at a non-existing index  */
  *position = CLAMP (*position, 0, picman_container_get_n_children (container));

  return TRUE;
}

void
picman_item_tree_add_item (PicmanItemTree *tree,
                         PicmanItem     *item,
                         PicmanItem     *parent,
                         gint          position)
{
  PicmanItemTreePrivate *private;
  PicmanContainer       *container;
  PicmanContainer       *children;

  g_return_if_fail (PICMAN_IS_ITEM_TREE (tree));

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type));
  g_return_if_fail (! picman_item_is_attached (item));
  g_return_if_fail (picman_item_get_image (item) == private->image);
  g_return_if_fail (parent == NULL ||
                    G_TYPE_CHECK_INSTANCE_TYPE (parent, private->item_type));
  g_return_if_fail (parent == NULL || picman_item_get_tree (parent) == tree);
  g_return_if_fail (parent == NULL ||
                    picman_viewable_get_children (PICMAN_VIEWABLE (parent)));

  picman_item_tree_uniquefy_name (tree, item, NULL);

  children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

  if (children)
    {
      GList *list = picman_item_stack_get_item_list (PICMAN_ITEM_STACK (children));

      while (list)
        {
          picman_item_tree_uniquefy_name (tree, list->data, NULL);

          list = g_list_remove (list, list->data);
        }
    }

  if (parent)
    container = picman_viewable_get_children (PICMAN_VIEWABLE (parent));
  else
    container = tree->container;

  if (parent)
    picman_viewable_set_parent (PICMAN_VIEWABLE (item),
                              PICMAN_VIEWABLE (parent));

  picman_container_insert (container, PICMAN_OBJECT (item), position);

  /*  if the item came from the undo stack, reset its "removed" state  */
  if (picman_item_is_removed (item))
    picman_item_unset_removed (item);
}

PicmanItem *
picman_item_tree_remove_item (PicmanItemTree *tree,
                            PicmanItem     *item,
                            PicmanItem     *new_active)
{
  PicmanItemTreePrivate *private;
  PicmanItem            *parent;
  PicmanContainer       *container;
  PicmanContainer       *children;
  gint                 index;

  g_return_val_if_fail (PICMAN_IS_ITEM_TREE (tree), NULL);

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type),
                        NULL);
  g_return_val_if_fail (picman_item_get_tree (item) == tree, NULL);

  parent    = picman_item_get_parent (item);
  container = picman_item_get_container (item);
  index     = picman_item_get_index (item);

  g_object_ref (item);

  g_hash_table_remove (private->name_hash,
                       picman_object_get_name (item));

  children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

  if (children)
    {
      GList *list = picman_item_stack_get_item_list (PICMAN_ITEM_STACK (children));

      while (list)
        {
          g_hash_table_remove (private->name_hash,
                               picman_object_get_name (list->data));

          list = g_list_remove (list, list->data);
        }
    }

  picman_container_remove (container, PICMAN_OBJECT (item));

  if (parent)
    picman_viewable_set_parent (PICMAN_VIEWABLE (item), NULL);

  picman_item_removed (item);

  if (! new_active)
    {
      gint n_children = picman_container_get_n_children (container);

      if (n_children > 0)
        {
          index = CLAMP (index, 0, n_children - 1);

          new_active =
            PICMAN_ITEM (picman_container_get_child_by_index (container, index));
        }
      else if (parent)
        {
          new_active = parent;
        }
    }

  g_object_unref (item);

  return new_active;
}

gboolean
picman_item_tree_reorder_item (PicmanItemTree *tree,
                             PicmanItem     *item,
                             PicmanItem     *new_parent,
                             gint          new_index,
                             gboolean      push_undo,
                             const gchar  *undo_desc)
{
  PicmanItemTreePrivate *private;
  PicmanContainer       *container;
  PicmanContainer       *new_container;
  gint                 n_items;

  g_return_val_if_fail (PICMAN_IS_ITEM_TREE (tree), FALSE);

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type),
                        FALSE);
  g_return_val_if_fail (picman_item_get_tree (item) == tree, FALSE);
  g_return_val_if_fail (new_parent == NULL ||
                        G_TYPE_CHECK_INSTANCE_TYPE (new_parent,
                                                    private->item_type),
                        FALSE);
  g_return_val_if_fail (new_parent == NULL ||
                        picman_item_get_tree (new_parent) == tree, FALSE);
  g_return_val_if_fail (new_parent == NULL ||
                        picman_viewable_get_children (PICMAN_VIEWABLE (new_parent)),
                        FALSE);
  g_return_val_if_fail (item != new_parent, FALSE);
  g_return_val_if_fail (new_parent == NULL ||
                        ! picman_viewable_is_ancestor (PICMAN_VIEWABLE (item),
                                                     PICMAN_VIEWABLE (new_parent)),
                        FALSE);

  container = picman_item_get_container (item);

  if (new_parent)
    new_container = picman_viewable_get_children (PICMAN_VIEWABLE (new_parent));
  else
    new_container = tree->container;

  n_items = picman_container_get_n_children (new_container);

  if (new_container == container)
    n_items--;

  new_index = CLAMP (new_index, 0, n_items);

  if (new_container != container ||
      new_index     != picman_item_get_index (item))
    {
      if (push_undo)
        picman_image_undo_push_item_reorder (private->image, undo_desc, item);

      if (new_container != container)
        {
          g_object_ref (item);

          picman_container_remove (container, PICMAN_OBJECT (item));

          picman_viewable_set_parent (PICMAN_VIEWABLE (item),
                                    PICMAN_VIEWABLE (new_parent));

          picman_container_insert (new_container, PICMAN_OBJECT (item), new_index);

          g_object_unref (item);
        }
      else
        {
          picman_container_reorder (container, PICMAN_OBJECT (item), new_index);
        }
    }

  return TRUE;
}

void
picman_item_tree_rename_item (PicmanItemTree *tree,
                            PicmanItem     *item,
                            const gchar  *new_name,
                            gboolean      push_undo,
                            const gchar  *undo_desc)
{
  PicmanItemTreePrivate *private;

  g_return_if_fail (PICMAN_IS_ITEM_TREE (tree));

  private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  g_return_if_fail (G_TYPE_CHECK_INSTANCE_TYPE (item, private->item_type));
  g_return_if_fail (picman_item_get_tree (item) == tree);
  g_return_if_fail (new_name != NULL);

  if (strcmp (new_name, picman_object_get_name (item)))
    {
      if (push_undo)
        picman_image_undo_push_item_rename (picman_item_get_image (item),
                                          undo_desc, item);

      picman_item_tree_uniquefy_name (tree, item, new_name);
    }
}


/*  private functions  */

static void
picman_item_tree_uniquefy_name (PicmanItemTree *tree,
                              PicmanItem     *item,
                              const gchar  *new_name)
{
  PicmanItemTreePrivate *private = PICMAN_ITEM_TREE_GET_PRIVATE (tree);

  if (new_name)
    {
      g_hash_table_remove (private->name_hash,
                           picman_object_get_name (item));

      picman_object_set_name (PICMAN_OBJECT (item), new_name);
    }

  if (g_hash_table_lookup (private->name_hash,
                           picman_object_get_name (item)))
    {
      gchar *name     = g_strdup (picman_object_get_name (item));
      gchar *ext      = strrchr (name, '#');
      gchar *new_name = NULL;
      gint   number   = 0;

      if (ext)
        {
          gchar ext_str[8];

          number = atoi (ext + 1);

          g_snprintf (ext_str, sizeof (ext_str), "%d", number);

          /*  check if the extension really is of the form "#<n>"  */
          if (! strcmp (ext_str, ext + 1))
            {
              if (ext > name && *(ext - 1) == ' ')
                ext--;

              *ext = '\0';
            }
          else
            {
              number = 0;
            }
        }

      do
        {
          number++;

          g_free (new_name);

          new_name = g_strdup_printf ("%s #%d", name, number);
        }
      while (g_hash_table_lookup (private->name_hash, new_name));

      g_free (name);

      picman_object_take_name (PICMAN_OBJECT (item), new_name);
    }

  g_hash_table_insert (private->name_hash,
                       (gpointer) picman_object_get_name (item),
                       item);
}
