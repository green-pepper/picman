/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanlist.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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
#include <string.h> /* strcmp */

#include <glib-object.h>

#include "core-types.h"

#include "picmanlist.h"


enum
{
  PROP_0,
  PROP_UNIQUE_NAMES,
  PROP_SORT_FUNC,
  PROP_APPEND
};


static void         picman_list_set_property       (GObject             *object,
                                                  guint                property_id,
                                                  const GValue        *value,
                                                  GParamSpec          *pspec);
static void         picman_list_get_property       (GObject             *object,
                                                  guint                property_id,
                                                  GValue              *value,
                                                  GParamSpec          *pspec);

static gint64       picman_list_get_memsize        (PicmanObject          *object,
                                                  gint64              *gui_size);

static void         picman_list_add                (PicmanContainer       *container,
                                                  PicmanObject          *object);
static void         picman_list_remove             (PicmanContainer       *container,
                                                  PicmanObject          *object);
static void         picman_list_reorder            (PicmanContainer       *container,
                                                  PicmanObject          *object,
                                                  gint                 new_index);
static void         picman_list_clear              (PicmanContainer       *container);
static gboolean     picman_list_have               (const PicmanContainer *container,
                                                  const PicmanObject    *object);
static void         picman_list_foreach            (const PicmanContainer *container,
                                                  GFunc                func,
                                                  gpointer             user_data);
static PicmanObject * picman_list_get_child_by_name  (const PicmanContainer *container,
                                                  const gchar         *name);
static PicmanObject * picman_list_get_child_by_index (const PicmanContainer *container,
                                                  gint                 index);
static gint         picman_list_get_child_index    (const PicmanContainer *container,
                                                  const PicmanObject    *object);

static void         picman_list_uniquefy_name      (PicmanList            *picman_list,
                                                  PicmanObject          *object);
static void         picman_list_object_renamed     (PicmanObject          *object,
                                                  PicmanList            *list);


G_DEFINE_TYPE (PicmanList, picman_list, PICMAN_TYPE_CONTAINER)

#define parent_class picman_list_parent_class


static void
picman_list_class_init (PicmanListClass *klass)
{
  GObjectClass       *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass    *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanContainerClass *container_class   = PICMAN_CONTAINER_CLASS (klass);

  object_class->set_property          = picman_list_set_property;
  object_class->get_property          = picman_list_get_property;

  picman_object_class->get_memsize      = picman_list_get_memsize;

  container_class->add                = picman_list_add;
  container_class->remove             = picman_list_remove;
  container_class->reorder            = picman_list_reorder;
  container_class->clear              = picman_list_clear;
  container_class->have               = picman_list_have;
  container_class->foreach            = picman_list_foreach;
  container_class->get_child_by_name  = picman_list_get_child_by_name;
  container_class->get_child_by_index = picman_list_get_child_by_index;
  container_class->get_child_index    = picman_list_get_child_index;

  g_object_class_install_property (object_class, PROP_UNIQUE_NAMES,
                                   g_param_spec_boolean ("unique-names",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_SORT_FUNC,
                                   g_param_spec_pointer ("sort-func",
                                                         NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_APPEND,
                                   g_param_spec_boolean ("append",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));
}

static void
picman_list_init (PicmanList *list)
{
  list->list         = NULL;
  list->unique_names = FALSE;
  list->sort_func    = NULL;
  list->append       = FALSE;
}

static void
picman_list_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanList *list = PICMAN_LIST (object);

  switch (property_id)
    {
    case PROP_UNIQUE_NAMES:
      list->unique_names = g_value_get_boolean (value);
      break;
    case PROP_SORT_FUNC:
      picman_list_set_sort_func (list, g_value_get_pointer (value));
      break;
    case PROP_APPEND:
      list->append = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_list_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  PicmanList *list = PICMAN_LIST (object);

  switch (property_id)
    {
    case PROP_UNIQUE_NAMES:
      g_value_set_boolean (value, list->unique_names);
      break;
    case PROP_SORT_FUNC:
      g_value_set_pointer (value, list->sort_func);
      break;
    case PROP_APPEND:
      g_value_set_boolean (value, list->append);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_list_get_memsize (PicmanObject *object,
                       gint64     *gui_size)
{
  PicmanList *list    = PICMAN_LIST (object);
  gint64    memsize = 0;

  memsize += (picman_container_get_n_children (PICMAN_CONTAINER (list)) *
              sizeof (GList));

  if (picman_container_get_policy (PICMAN_CONTAINER (list)) ==
      PICMAN_CONTAINER_POLICY_STRONG)
    {
      GList *glist;

      for (glist = list->list; glist; glist = g_list_next (glist))
        memsize += picman_object_get_memsize (PICMAN_OBJECT (glist->data), gui_size);
    }

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_list_add (PicmanContainer *container,
               PicmanObject    *object)
{
  PicmanList *list = PICMAN_LIST (container);

  if (list->unique_names)
    picman_list_uniquefy_name (list, object);

  if (list->unique_names || list->sort_func)
    g_signal_connect (object, "name-changed",
                      G_CALLBACK (picman_list_object_renamed),
                      list);

  if (list->sort_func)
    list->list = g_list_insert_sorted (list->list, object, list->sort_func);
  else if (list->append)
    list->list = g_list_append (list->list, object);
  else
    list->list = g_list_prepend (list->list, object);

  PICMAN_CONTAINER_CLASS (parent_class)->add (container, object);
}

static void
picman_list_remove (PicmanContainer *container,
                  PicmanObject    *object)
{
  PicmanList *list = PICMAN_LIST (container);

  if (list->unique_names || list->sort_func)
    g_signal_handlers_disconnect_by_func (object,
                                          picman_list_object_renamed,
                                          list);

  list->list = g_list_remove (list->list, object);

  PICMAN_CONTAINER_CLASS (parent_class)->remove (container, object);
}

static void
picman_list_reorder (PicmanContainer *container,
                   PicmanObject    *object,
                   gint           new_index)
{
  PicmanList *list = PICMAN_LIST (container);

  list->list = g_list_remove (list->list, object);

  if (new_index == -1 ||
      new_index == picman_container_get_n_children (container) - 1)
    list->list = g_list_append (list->list, object);
  else
    list->list = g_list_insert (list->list, object, new_index);
}

static void
picman_list_clear (PicmanContainer *container)
{
  PicmanList *list = PICMAN_LIST (container);

  while (list->list)
    picman_container_remove (container, list->list->data);
}

static gboolean
picman_list_have (const PicmanContainer *container,
                const PicmanObject    *object)
{
  PicmanList *list = PICMAN_LIST (container);

  return g_list_find (list->list, object) ? TRUE : FALSE;
}

static void
picman_list_foreach (const PicmanContainer *container,
                   GFunc                func,
                   gpointer             user_data)
{
  PicmanList *list = PICMAN_LIST (container);

  g_list_foreach (list->list, func, user_data);
}

static PicmanObject *
picman_list_get_child_by_name (const PicmanContainer *container,
                             const gchar         *name)
{
  PicmanList *list = PICMAN_LIST (container);
  GList    *glist;

  for (glist = list->list; glist; glist = g_list_next (glist))
    {
      PicmanObject *object = glist->data;

      if (! strcmp (picman_object_get_name (object), name))
        return object;
    }

  return NULL;
}

static PicmanObject *
picman_list_get_child_by_index (const PicmanContainer *container,
                              gint                 index)
{
  PicmanList *list = PICMAN_LIST (container);
  GList    *glist;

  glist = g_list_nth (list->list, index);

  if (glist)
    return (PicmanObject *) glist->data;

  return NULL;
}

static gint
picman_list_get_child_index (const PicmanContainer *container,
                           const PicmanObject    *object)
{
  PicmanList *list = PICMAN_LIST (container);

  return g_list_index (list->list, (gpointer) object);
}

/**
 * picman_list_new:
 * @children_type: the #GType of objects the list is going to hold
 * @unique_names:  if the list should ensure that all its children
 *                 have unique names.
 *
 * Creates a new #PicmanList object. Since #PicmanList is a #PicmanContainer
 * implementation, it holds PicmanObjects. Thus @children_type must be
 * PICMAN_TYPE_OBJECT or a type derived from it.
 *
 * The returned list has the #PICMAN_CONTAINER_POLICY_STRONG.
 *
 * Return value: a new #PicmanList object
 **/
PicmanContainer *
picman_list_new (GType    children_type,
               gboolean unique_names)
{
  PicmanList *list;

  g_return_val_if_fail (g_type_is_a (children_type, PICMAN_TYPE_OBJECT), NULL);

  list = g_object_new (PICMAN_TYPE_LIST,
                       "children-type", children_type,
                       "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                       "unique-names",  unique_names ? TRUE : FALSE,
                       NULL);

  /* for debugging purposes only */
  picman_object_set_static_name (PICMAN_OBJECT (list), g_type_name (children_type));

  return PICMAN_CONTAINER (list);
}

/**
 * picman_list_new_weak:
 * @children_type: the #GType of objects the list is going to hold
 * @unique_names:  if the list should ensure that all its children
 *                 have unique names.
 *
 * Creates a new #PicmanList object. Since #PicmanList is a #PicmanContainer
 * implementation, it holds PicmanObjects. Thus @children_type must be
 * PICMAN_TYPE_OBJECT or a type derived from it.
 *
 * The returned list has the #PICMAN_CONTAINER_POLICY_WEAK.
 *
 * Return value: a new #PicmanList object
 **/
PicmanContainer *
picman_list_new_weak (GType    children_type,
                    gboolean unique_names)
{
  PicmanList *list;

  g_return_val_if_fail (g_type_is_a (children_type, PICMAN_TYPE_OBJECT), NULL);

  list = g_object_new (PICMAN_TYPE_LIST,
                       "children-type", children_type,
                       "policy",        PICMAN_CONTAINER_POLICY_WEAK,
                       "unique-names",  unique_names ? TRUE : FALSE,
                       NULL);

  /* for debugging purposes only */
  picman_object_set_static_name (PICMAN_OBJECT (list), g_type_name (children_type));

  return PICMAN_CONTAINER (list);
}

/**
 * picman_list_reverse:
 * @list: a #PicmanList
 *
 * Reverses the order of elements in a #PicmanList.
 **/
void
picman_list_reverse (PicmanList *list)
{
  g_return_if_fail (PICMAN_IS_LIST (list));

  if (picman_container_get_n_children (PICMAN_CONTAINER (list)) > 1)
    {
      picman_container_freeze (PICMAN_CONTAINER (list));
      list->list = g_list_reverse (list->list);
      picman_container_thaw (PICMAN_CONTAINER (list));
    }
}

/**
 * picman_list_set_sort_func:
 * @list:      a #PicmanList
 * @sort_func: a #GCompareFunc
 *
 * Sorts the elements of @list using picman_list_sort() and remembers the
 * passed @sort_func in order to keep the list ordered across inserting
 * or renaming children.
 **/
void
picman_list_set_sort_func (PicmanList     *list,
                         GCompareFunc  sort_func)
{
  g_return_if_fail (PICMAN_IS_LIST (list));

  if (sort_func != list->sort_func)
    {
      if (sort_func)
        picman_list_sort (list, sort_func);

      list->sort_func = sort_func;
      g_object_notify (G_OBJECT (list), "sort-func");
    }
}

/**
 * picman_list_get_sort_func:
 * @list: a #PicmanList
 *
 * Returns the @list's sort function, see picman_list_set_sort_func().
 *
 * Return Value: The @list's sort function.
 **/
GCompareFunc
picman_list_get_sort_func (PicmanList*list)
{
  g_return_val_if_fail (PICMAN_IS_LIST (list), NULL);

  return list->sort_func;
}

/**
 * picman_list_sort:
 * @list: a #PicmanList
 * @sort_func: a #GCompareFunc
 *
 * Sorts the elements of a #PicmanList according to the given @sort_func.
 * See g_list_sort() for a detailed description of this function.
 **/
void
picman_list_sort (PicmanList     *list,
                GCompareFunc  sort_func)
{
  g_return_if_fail (PICMAN_IS_LIST (list));
  g_return_if_fail (sort_func != NULL);

  if (picman_container_get_n_children (PICMAN_CONTAINER (list)) > 1)
    {
      picman_container_freeze (PICMAN_CONTAINER (list));
      list->list = g_list_sort (list->list, sort_func);
      picman_container_thaw (PICMAN_CONTAINER (list));
    }
}

/**
 * picman_list_sort_by_name:
 * @list: a #PicmanList
 *
 * Sorts the #PicmanObject elements of a #PicmanList by their names.
 **/
void
picman_list_sort_by_name (PicmanList *list)
{
  g_return_if_fail (PICMAN_IS_LIST (list));

  picman_list_sort (list, (GCompareFunc) picman_object_name_collate);
}


/*  private functions  */

static void
picman_list_uniquefy_name (PicmanList   *picman_list,
                         PicmanObject *object)
{
  gchar *name = (gchar *) picman_object_get_name (object);
  GList *list;

  if (! name)
    return;

  for (list = picman_list->list; list; list = g_list_next (list))
    {
      PicmanObject  *object2 = list->data;
      const gchar *name2   = picman_object_get_name (object2);

      if (object != object2 &&
          name2             &&
          ! strcmp (name, name2))
        break;
    }

  if (list)
    {
      gchar *ext;
      gchar *new_name   = NULL;
      gint   unique_ext = 0;

      name = g_strdup (name);

      ext = strrchr (name, '#');

      if (ext)
        {
          gchar ext_str[8];

          unique_ext = atoi (ext + 1);

          g_snprintf (ext_str, sizeof (ext_str), "%d", unique_ext);

          /*  check if the extension really is of the form "#<n>"  */
          if (! strcmp (ext_str, ext + 1))
            {
              if (ext > name && *(ext - 1) == ' ')
                ext--;

              *ext = '\0';
            }
          else
            {
              unique_ext = 0;
            }
        }

      do
        {
          unique_ext++;

          g_free (new_name);

          new_name = g_strdup_printf ("%s #%d", name, unique_ext);

          for (list = picman_list->list; list; list = g_list_next (list))
            {
              PicmanObject  *object2 = list->data;
              const gchar *name2   = picman_object_get_name (object2);

              if (object != object2 &&
                  name2             &&
                  ! strcmp (new_name, name2))
                break;
            }
        }
      while (list);

      g_free (name);

      picman_object_take_name (object, new_name);
    }
}

static void
picman_list_object_renamed (PicmanObject *object,
                          PicmanList   *list)
{
  if (list->unique_names)
    {
      g_signal_handlers_block_by_func (object,
                                       picman_list_object_renamed,
                                       list);

      picman_list_uniquefy_name (list, object);

      g_signal_handlers_unblock_by_func (object,
                                         picman_list_object_renamed,
                                         list);
    }

  if (list->sort_func)
    {
      GList *glist;
      gint   old_index;
      gint   new_index = 0;

      old_index = g_list_index (list->list, object);

      for (glist = list->list; glist; glist = g_list_next (glist))
        {
          PicmanObject *object2 = PICMAN_OBJECT (glist->data);

          if (object == object2)
            continue;

          if (list->sort_func (object, object2) > 0)
            new_index++;
          else
            break;
        }

      if (new_index != old_index)
        picman_container_reorder (PICMAN_CONTAINER (list), object, new_index);
    }
}
