/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmantaggedcontainer.c
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#include <glib-object.h>

#include "core-types.h"

#include "picman.h"
#include "picmanmarshal.h"
#include "picmantag.h"
#include "picmantagged.h"
#include "picmantaggedcontainer.h"


enum
{
  TAG_COUNT_CHANGED,
  LAST_SIGNAL
};


static void      picman_tagged_container_dispose            (GObject               *object);
static gint64    picman_tagged_container_get_memsize        (PicmanObject            *object,
                                                           gint64                *gui_size);

static void      picman_tagged_container_clear              (PicmanContainer         *container);

static void      picman_tagged_container_src_add            (PicmanFilteredContainer *filtered_container,
                                                           PicmanObject            *object);
static void      picman_tagged_container_src_remove         (PicmanFilteredContainer *filtered_container,
                                                           PicmanObject            *object);
static void      picman_tagged_container_src_freeze         (PicmanFilteredContainer *filtered_container);
static void      picman_tagged_container_src_thaw           (PicmanFilteredContainer *filtered_container);

static gboolean  picman_tagged_container_object_matches     (PicmanTaggedContainer   *tagged_container,
                                                           PicmanObject            *object);

static void      picman_tagged_container_tag_added          (PicmanTagged            *tagged,
                                                           PicmanTag               *tag,
                                                           PicmanTaggedContainer   *tagged_container);
static void      picman_tagged_container_tag_removed        (PicmanTagged            *tagged,
                                                           PicmanTag               *tag,
                                                           PicmanTaggedContainer   *tagged_container);
static void      picman_tagged_container_ref_tag            (PicmanTaggedContainer   *tagged_container,
                                                           PicmanTag               *tag);
static void      picman_tagged_container_unref_tag          (PicmanTaggedContainer   *tagged_container,
                                                           PicmanTag               *tag);
static void      picman_tagged_container_tag_count_changed  (PicmanTaggedContainer   *tagged_container,
                                                           gint                   tag_count);


G_DEFINE_TYPE (PicmanTaggedContainer, picman_tagged_container,
               PICMAN_TYPE_FILTERED_CONTAINER)

#define parent_class picman_tagged_container_parent_class

static guint picman_tagged_container_signals[LAST_SIGNAL] = { 0, };


static void
picman_tagged_container_class_init (PicmanTaggedContainerClass *klass)
{
  GObjectClass               *g_object_class    = G_OBJECT_CLASS (klass);
  PicmanObjectClass            *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanContainerClass         *container_class   = PICMAN_CONTAINER_CLASS (klass);
  PicmanFilteredContainerClass *filtered_class    = PICMAN_FILTERED_CONTAINER_CLASS (klass);

  g_object_class->dispose        = picman_tagged_container_dispose;

  picman_object_class->get_memsize = picman_tagged_container_get_memsize;

  container_class->clear         = picman_tagged_container_clear;

  filtered_class->src_add        = picman_tagged_container_src_add;
  filtered_class->src_remove     = picman_tagged_container_src_remove;
  filtered_class->src_freeze     = picman_tagged_container_src_freeze;
  filtered_class->src_thaw       = picman_tagged_container_src_thaw;

  klass->tag_count_changed       = picman_tagged_container_tag_count_changed;

  picman_tagged_container_signals[TAG_COUNT_CHANGED] =
    g_signal_new ("tag-count-changed",
                  PICMAN_TYPE_TAGGED_CONTAINER,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanTaggedContainerClass, tag_count_changed),
                  NULL, NULL,
                  picman_marshal_VOID__INT,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);
}

static void
picman_tagged_container_init (PicmanTaggedContainer *tagged_container)
{
  tagged_container->tag_ref_counts =
    g_hash_table_new ((GHashFunc) picman_tag_get_hash,
                      (GEqualFunc) picman_tag_equals);
}

static void
picman_tagged_container_dispose (GObject *object)
{
  PicmanTaggedContainer *tagged_container = PICMAN_TAGGED_CONTAINER (object);

  if (tagged_container->filter)
    {
      g_list_free_full (tagged_container->filter,
                        (GDestroyNotify) picman_tag_or_null_unref);
      tagged_container->filter = NULL;
    }

  if (tagged_container->tag_ref_counts)
    {
      g_hash_table_unref (tagged_container->tag_ref_counts);
      tagged_container->tag_ref_counts = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gint64
picman_tagged_container_get_memsize (PicmanObject *object,
                                   gint64     *gui_size)
{
  gint64 memsize = 0;

  /* FIXME take members into account */

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_tagged_container_clear (PicmanContainer *container)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (container);
  PicmanTaggedContainer   *tagged_container   = PICMAN_TAGGED_CONTAINER (container);
  GList                 *list;

  for (list = PICMAN_LIST (filtered_container->src_container)->list;
       list;
       list = g_list_next (list))
    {
      g_signal_handlers_disconnect_by_func (list->data,
                                            picman_tagged_container_tag_added,
                                            tagged_container);
      g_signal_handlers_disconnect_by_func (list->data,
                                            picman_tagged_container_tag_removed,
                                            tagged_container);
    }

  if (tagged_container->tag_ref_counts)
    {
      g_hash_table_remove_all (tagged_container->tag_ref_counts);
      tagged_container->tag_count = 0;
    }

  PICMAN_CONTAINER_CLASS (parent_class)->clear (container);
}

static void
picman_tagged_container_src_add (PicmanFilteredContainer *filtered_container,
                               PicmanObject            *object)
{
  PicmanTaggedContainer *tagged_container = PICMAN_TAGGED_CONTAINER (filtered_container);
  GList               *list;

  for (list = picman_tagged_get_tags (PICMAN_TAGGED (object));
       list;
       list = g_list_next (list))
    {
      picman_tagged_container_ref_tag (tagged_container, list->data);
    }

  g_signal_connect (object, "tag-added",
                    G_CALLBACK (picman_tagged_container_tag_added),
                    tagged_container);
  g_signal_connect (object, "tag-removed",
                    G_CALLBACK (picman_tagged_container_tag_removed),
                    tagged_container);

  if (picman_tagged_container_object_matches (tagged_container, object))
    {
      picman_container_add (PICMAN_CONTAINER (tagged_container), object);
    }
}

static void
picman_tagged_container_src_remove (PicmanFilteredContainer *filtered_container,
                                  PicmanObject            *object)
{
  PicmanTaggedContainer *tagged_container = PICMAN_TAGGED_CONTAINER (filtered_container);
  GList               *list;

  g_signal_handlers_disconnect_by_func (object,
                                        picman_tagged_container_tag_added,
                                        tagged_container);
  g_signal_handlers_disconnect_by_func (object,
                                        picman_tagged_container_tag_removed,
                                        tagged_container);

  for (list = picman_tagged_get_tags (PICMAN_TAGGED (object));
       list;
       list = g_list_next (list))
    {
      picman_tagged_container_unref_tag (tagged_container, list->data);
    }

  if (picman_tagged_container_object_matches (tagged_container, object))
    {
      picman_container_remove (PICMAN_CONTAINER (tagged_container), object);
    }
}

static void
picman_tagged_container_src_freeze (PicmanFilteredContainer *filtered_container)
{
  picman_container_clear (PICMAN_CONTAINER (filtered_container));
}

static void
picman_tagged_container_src_thaw (PicmanFilteredContainer *filtered_container)
{
  GList *list;

  for (list = PICMAN_LIST (filtered_container->src_container)->list;
       list;
       list = g_list_next (list))
    {
      picman_tagged_container_src_add (filtered_container, list->data);
    }
}

/**
 * picman_tagged_container_new:
 * @src_container: container to be filtered.
 *
 * Creates a new #PicmanTaggedContainer object which creates filtered
 * data view of #PicmanTagged objects. It filters @src_container for
 * objects containing all of the filtering tags. Synchronization with
 * @src_container data is performed automatically.
 *
 * Return value: a new #PicmanTaggedContainer object.
 **/
PicmanContainer *
picman_tagged_container_new (PicmanContainer *src_container)
{
  PicmanTaggedContainer *tagged_container;
  GType                children_type;
  GCompareFunc         sort_func;

  g_return_val_if_fail (PICMAN_IS_LIST (src_container), NULL);

  children_type = picman_container_get_children_type (src_container);
  sort_func     = PICMAN_LIST (src_container)->sort_func;

  tagged_container = g_object_new (PICMAN_TYPE_TAGGED_CONTAINER,
                                   "sort-func",     sort_func,
                                   "children-type", children_type,
                                   "policy",        PICMAN_CONTAINER_POLICY_WEAK,
                                   "unique-names",  FALSE,
                                   "src-container", src_container,
                                   NULL);

  return PICMAN_CONTAINER (tagged_container);
}

/**
 * picman_tagged_container_set_filter:
 * @tagged_container: a #PicmanTaggedContainer object.
 * @tags:             list of #PicmanTag objects.
 *
 * Sets list of tags to be used for filtering. Only objects which have
 * all of the tags assigned match filtering criteria.
 **/
void
picman_tagged_container_set_filter (PicmanTaggedContainer *tagged_container,
                                  GList               *tags)
{
  GList *new_filter;

  g_return_if_fail (PICMAN_IS_TAGGED_CONTAINER (tagged_container));

  if (tags)
    {
      GList *list;

      for (list = tags; list; list = g_list_next (list))
        g_return_if_fail (list->data == NULL || PICMAN_IS_TAG (list->data));
    }

  if (! picman_container_frozen (PICMAN_FILTERED_CONTAINER (tagged_container)->src_container))
    {
      picman_tagged_container_src_freeze (PICMAN_FILTERED_CONTAINER (tagged_container));
    }

  /*  ref new tags first, they could be the same as the old ones  */
  new_filter = g_list_copy (tags);
  g_list_foreach (new_filter, (GFunc) picman_tag_or_null_ref, NULL);

  g_list_free_full (tagged_container->filter,
                    (GDestroyNotify) picman_tag_or_null_unref);
  tagged_container->filter = new_filter;

  if (! picman_container_frozen (PICMAN_FILTERED_CONTAINER (tagged_container)->src_container))
    {
      picman_tagged_container_src_thaw (PICMAN_FILTERED_CONTAINER (tagged_container));
    }
}

/**
 * picman_tagged_container_get_filter:
 * @tagged_container: a #PicmanTaggedContainer object.
 *
 * Returns current tag filter. Tag filter is a list of PicmanTag objects, which
 * must be contained by each object matching filter criteria.
 *
 * Return value: a list of PicmanTag objects used as filter. This value should
 *               not be modified or freed.
 **/
const GList *
picman_tagged_container_get_filter (PicmanTaggedContainer *tagged_container)
{
  g_return_val_if_fail (PICMAN_IS_TAGGED_CONTAINER (tagged_container), NULL);

  return tagged_container->filter;
}

static gboolean
picman_tagged_container_object_matches (PicmanTaggedContainer *tagged_container,
                                      PicmanObject          *object)
{
  GList *filter_tags;

  for (filter_tags = tagged_container->filter;
       filter_tags;
       filter_tags = g_list_next (filter_tags))
    {
      if (! filter_tags->data)
        {
          /* invalid tag - does not match */
          return FALSE;
        }

      if (! picman_tagged_has_tag (PICMAN_TAGGED (object),
                                 filter_tags->data))
        {
          /* match for the tag was not found.
           * since query is of type AND, it whole fails.
           */
          return FALSE;
        }
    }

  return TRUE;
}

static void
picman_tagged_container_tag_added (PicmanTagged          *tagged,
                                 PicmanTag             *tag,
                                 PicmanTaggedContainer *tagged_container)
{
  picman_tagged_container_ref_tag (tagged_container, tag);

  if (picman_tagged_container_object_matches (tagged_container,
                                            PICMAN_OBJECT (tagged)) &&
      ! picman_container_have (PICMAN_CONTAINER (tagged_container),
                             PICMAN_OBJECT (tagged)))
    {
      picman_container_add (PICMAN_CONTAINER (tagged_container),
                          PICMAN_OBJECT (tagged));
    }
}

static void
picman_tagged_container_tag_removed (PicmanTagged          *tagged,
                                   PicmanTag             *tag,
                                   PicmanTaggedContainer *tagged_container)
{
  picman_tagged_container_unref_tag (tagged_container, tag);

  if (! picman_tagged_container_object_matches (tagged_container,
                                              PICMAN_OBJECT (tagged)) &&
      picman_container_have (PICMAN_CONTAINER (tagged_container),
                           PICMAN_OBJECT (tagged)))
    {
      picman_container_remove (PICMAN_CONTAINER (tagged_container),
                             PICMAN_OBJECT (tagged));
    }
}

static void
picman_tagged_container_ref_tag (PicmanTaggedContainer *tagged_container,
                               PicmanTag             *tag)
{
  gint ref_count;

  ref_count = GPOINTER_TO_INT (g_hash_table_lookup (tagged_container->tag_ref_counts,
                                                    tag));
  ref_count++;
  g_hash_table_insert (tagged_container->tag_ref_counts,
                       tag, GINT_TO_POINTER (ref_count));
  if (ref_count == 1)
    {
      tagged_container->tag_count++;
      g_signal_emit (tagged_container,
                     picman_tagged_container_signals[TAG_COUNT_CHANGED], 0,
                     tagged_container->tag_count);
    }
}

static void
picman_tagged_container_unref_tag (PicmanTaggedContainer *tagged_container,
                                 PicmanTag             *tag)
{
  gint ref_count;

  ref_count = GPOINTER_TO_INT (g_hash_table_lookup (tagged_container->tag_ref_counts,
                                                    tag));
  ref_count--;

  if (ref_count > 0)
    {
      g_hash_table_insert (tagged_container->tag_ref_counts,
                           tag, GINT_TO_POINTER (ref_count));
    }
  else
    {
      if (g_hash_table_remove (tagged_container->tag_ref_counts, tag))
        {
          tagged_container->tag_count--;
          g_signal_emit (tagged_container,
                         picman_tagged_container_signals[TAG_COUNT_CHANGED], 0,
                         tagged_container->tag_count);
        }
    }
}

static void
picman_tagged_container_tag_count_changed (PicmanTaggedContainer *container,
                                         gint                 tag_count)
{
}

/**
 * picman_tagged_container_get_tag_count:
 * @container:  a #PicmanTaggedContainer object.
 *
 * Get number of distinct tags that are currently assigned to all
 * objects in the container. The count is independent of currently
 * used filter, it is provided for all available objects (ie. empty
 * filter).
 *
 * Return value: number of distinct tags assigned to all objects in the
 *               container.
 **/
gint
picman_tagged_container_get_tag_count (PicmanTaggedContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_TAGGED_CONTAINER (container), 0);

  return container->tag_count;
}
