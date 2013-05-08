/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanfilteredcontainer.c
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
 *               2011 Michael Natterer <mitch@picman.org>
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

#include "picmanfilteredcontainer.h"


enum
{
  PROP_0,
  PROP_SRC_CONTAINER,
  PROP_FILTER_FUNC,
  PROP_FILTER_DATA
};


static void     picman_filtered_container_constructed     (GObject               *object);
static void     picman_filtered_container_dispose         (GObject               *object);
static void     picman_filtered_container_finalize        (GObject               *object);
static void     picman_filtered_container_set_property    (GObject               *object,
                                                         guint                  property_id,
                                                         const GValue          *value,
                                                         GParamSpec            *pspec);
static void     picman_filtered_container_get_property    (GObject               *object,
                                                         guint                  property_id,
                                                         GValue                *value,
                                                         GParamSpec            *pspec);

static void     picman_filtered_container_real_src_add    (PicmanFilteredContainer *filtered_container,
                                                         PicmanObject            *object);
static void     picman_filtered_container_real_src_remove (PicmanFilteredContainer *filtered_container,
                                                         PicmanObject            *object);
static void     picman_filtered_container_real_src_freeze (PicmanFilteredContainer *filtered_container);
static void     picman_filtered_container_real_src_thaw   (PicmanFilteredContainer *filtered_container);

static gboolean picman_filtered_container_object_matches  (PicmanFilteredContainer *filtered_container,
                                                         PicmanObject            *object);
static void     picman_filtered_container_src_add         (PicmanContainer         *src_container,
                                                         PicmanObject            *obj,
                                                         PicmanFilteredContainer *filtered_container);
static void     picman_filtered_container_src_remove      (PicmanContainer         *src_container,
                                                         PicmanObject            *obj,
                                                         PicmanFilteredContainer *filtered_container);
static void     picman_filtered_container_src_freeze      (PicmanContainer         *src_container,
                                                         PicmanFilteredContainer *filtered_container);
static void     picman_filtered_container_src_thaw        (PicmanContainer         *src_container,
                                                         PicmanFilteredContainer *filtered_container);


G_DEFINE_TYPE (PicmanFilteredContainer, picman_filtered_container, PICMAN_TYPE_LIST)

#define parent_class picman_filtered_container_parent_class


static void
picman_filtered_container_class_init (PicmanFilteredContainerClass *klass)
{
  GObjectClass               *g_object_class = G_OBJECT_CLASS (klass);
  PicmanFilteredContainerClass *filtered_class = PICMAN_FILTERED_CONTAINER_CLASS (klass);

  g_object_class->constructed  = picman_filtered_container_constructed;
  g_object_class->dispose      = picman_filtered_container_dispose;
  g_object_class->finalize     = picman_filtered_container_finalize;
  g_object_class->set_property = picman_filtered_container_set_property;
  g_object_class->get_property = picman_filtered_container_get_property;

  filtered_class->src_add      = picman_filtered_container_real_src_add;
  filtered_class->src_remove   = picman_filtered_container_real_src_remove;
  filtered_class->src_freeze   = picman_filtered_container_real_src_freeze;
  filtered_class->src_thaw     = picman_filtered_container_real_src_thaw;

  g_object_class_install_property (g_object_class, PROP_SRC_CONTAINER,
                                   g_param_spec_object ("src-container",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (g_object_class, PROP_FILTER_FUNC,
                                   g_param_spec_pointer ("filter-func",
                                                         NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (g_object_class, PROP_FILTER_DATA,
                                   g_param_spec_pointer ("filter-data",
                                                         NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_filtered_container_init (PicmanFilteredContainer *filtered_container)
{
}

static void
picman_filtered_container_constructed (GObject *object)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_CONTAINER (filtered_container->src_container));

  if (! picman_container_frozen (filtered_container->src_container))
    {
      /*  a freeze/thaw can't hurt on a newly created container because
       *  we can't have any views yet. This way we get away without
       *  having a virtual function for initializing the container.
       */
      picman_filtered_container_src_freeze (filtered_container->src_container,
                                          filtered_container);
      picman_filtered_container_src_thaw (filtered_container->src_container,
                                        filtered_container);
    }
}

static void
picman_filtered_container_dispose (GObject *object)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (object);

  if (filtered_container->src_container)
    {
      g_signal_handlers_disconnect_by_func (filtered_container->src_container,
                                            picman_filtered_container_src_add,
                                            filtered_container);
      g_signal_handlers_disconnect_by_func (filtered_container->src_container,
                                            picman_filtered_container_src_remove,
                                            filtered_container);
      g_signal_handlers_disconnect_by_func (filtered_container->src_container,
                                            picman_filtered_container_src_freeze,
                                            filtered_container);
      g_signal_handlers_disconnect_by_func (filtered_container->src_container,
                                            picman_filtered_container_src_thaw,
                                            filtered_container);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_filtered_container_finalize (GObject *object)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (object);

  if (filtered_container->src_container)
    {
      g_object_unref (filtered_container->src_container);
      filtered_container->src_container = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_filtered_container_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (object);

  switch (property_id)
    {
    case PROP_SRC_CONTAINER:
      filtered_container->src_container = g_value_dup_object (value);

      g_signal_connect (filtered_container->src_container, "add",
                        G_CALLBACK (picman_filtered_container_src_add),
                        filtered_container);
      g_signal_connect (filtered_container->src_container, "remove",
                        G_CALLBACK (picman_filtered_container_src_remove),
                        filtered_container);
      g_signal_connect (filtered_container->src_container, "freeze",
                        G_CALLBACK (picman_filtered_container_src_freeze),
                        filtered_container);
      g_signal_connect (filtered_container->src_container, "thaw",
                        G_CALLBACK (picman_filtered_container_src_thaw),
                        filtered_container);
      break;

    case PROP_FILTER_FUNC:
      filtered_container->filter_func = g_value_get_pointer (value);
      break;

    case PROP_FILTER_DATA:
      filtered_container->filter_data = g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_filtered_container_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PicmanFilteredContainer *filtered_container = PICMAN_FILTERED_CONTAINER (object);

  switch (property_id)
    {
    case PROP_SRC_CONTAINER:
      g_value_set_object (value, filtered_container->src_container);
      break;

    case PROP_FILTER_FUNC:
      g_value_set_pointer (value, filtered_container->filter_func);
      break;

    case PROP_FILTER_DATA:
      g_value_set_pointer (value, filtered_container->filter_data);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_filtered_container_real_src_add (PicmanFilteredContainer *filtered_container,
                                      PicmanObject            *object)
{
  if (picman_filtered_container_object_matches (filtered_container, object))
    {
      picman_container_add (PICMAN_CONTAINER (filtered_container), object);
    }
}

static void
picman_filtered_container_real_src_remove (PicmanFilteredContainer *filtered_container,
                                         PicmanObject            *object)
{
  if (picman_filtered_container_object_matches (filtered_container, object))
    {
      picman_container_remove (PICMAN_CONTAINER (filtered_container), object);
    }
}

static void
picman_filtered_container_real_src_freeze (PicmanFilteredContainer *filtered_container)
{
  picman_container_clear (PICMAN_CONTAINER (filtered_container));
}

static void
picman_filtered_container_real_src_thaw (PicmanFilteredContainer *filtered_container)
{
  GList *list;

  for (list = PICMAN_LIST (filtered_container->src_container)->list;
       list;
       list = g_list_next (list))
    {
      PicmanObject *object = list->data;

      if (picman_filtered_container_object_matches (filtered_container, object))
        {
          picman_container_add (PICMAN_CONTAINER (filtered_container), object);
        }
    }
}

/**
 * picman_filtered_container_new:
 * @src_container: container to be filtered.
 *
 * Creates a new #PicmanFilteredContainer object which creates filtered
 * data view of #PicmanTagged objects. It filters @src_container for objects
 * containing all of the filtering tags. Synchronization with @src_container
 * data is performed automatically.
 *
 * Return value: a new #PicmanFilteredContainer object.
 **/
PicmanContainer *
picman_filtered_container_new (PicmanContainer        *src_container,
                             PicmanObjectFilterFunc  filter_func,
                             gpointer              filter_data)
{
  GType        children_type;
  GCompareFunc sort_func;

  g_return_val_if_fail (PICMAN_IS_LIST (src_container), NULL);

  children_type = picman_container_get_children_type (src_container);
  sort_func     = PICMAN_LIST (src_container)->sort_func;

  return g_object_new (PICMAN_TYPE_FILTERED_CONTAINER,
                       "sort-func",     sort_func,
                       "children-type", children_type,
                       "policy",        PICMAN_CONTAINER_POLICY_WEAK,
                       "unique-names",  FALSE,
                       "src-container", src_container,
                       "filter-func",   filter_func,
                       "filter-data",   filter_data,
                       NULL);
}

static gboolean
picman_filtered_container_object_matches (PicmanFilteredContainer *filtered_container,
                                        PicmanObject            *object)
{
  return (! filtered_container->filter_func ||
          filtered_container->filter_func (object,
                                           filtered_container->filter_data));
}

static void
picman_filtered_container_src_add (PicmanContainer         *src_container,
                                 PicmanObject            *object,
                                 PicmanFilteredContainer *filtered_container)
{
  if (! picman_container_frozen (filtered_container->src_container))
    {
      PICMAN_FILTERED_CONTAINER_GET_CLASS (filtered_container)->src_add (filtered_container,
                                                                       object);
    }
}

static void
picman_filtered_container_src_remove (PicmanContainer         *src_container,
                                    PicmanObject            *object,
                                    PicmanFilteredContainer *filtered_container)
{
  if (! picman_container_frozen (filtered_container->src_container))
    {
      PICMAN_FILTERED_CONTAINER_GET_CLASS (filtered_container)->src_remove (filtered_container,
                                                                          object);
    }
}

static void
picman_filtered_container_src_freeze (PicmanContainer         *src_container,
                                    PicmanFilteredContainer *filtered_container)
{
  picman_container_freeze (PICMAN_CONTAINER (filtered_container));

  PICMAN_FILTERED_CONTAINER_GET_CLASS (filtered_container)->src_freeze (filtered_container);
}

static void
picman_filtered_container_src_thaw (PicmanContainer         *src_container,
                                  PicmanFilteredContainer *filtered_container)
{
  PICMAN_FILTERED_CONTAINER_GET_CLASS (filtered_container)->src_thaw (filtered_container);

  picman_container_thaw (PICMAN_CONTAINER (filtered_container));
}
