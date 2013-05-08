/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfilter.c
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

#include "picman.h"
#include "picman-utils.h"
#include "picmanfilter.h"


enum
{
  PROP_0,
  PROP_IS_LAST_NODE
};


typedef struct _PicmanFilterPrivate PicmanFilterPrivate;

struct _PicmanFilterPrivate
{
  GeglNode       *node;
  gboolean        is_last_node;

  PicmanApplicator *applicator;
};

#define GET_PRIVATE(filter) G_TYPE_INSTANCE_GET_PRIVATE (filter, \
                                                         PICMAN_TYPE_FILTER, \
                                                         PicmanFilterPrivate)


/*  local function prototypes  */

static void       picman_filter_finalize      (GObject      *object);
static void       picman_filter_set_property  (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void       picman_filter_get_property  (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);

static gint64     picman_filter_get_memsize   (PicmanObject   *object,
                                             gint64       *gui_size);

static GeglNode * picman_filter_real_get_node (PicmanFilter *filter);


G_DEFINE_TYPE (PicmanFilter, picman_filter, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_filter_parent_class


static void
picman_filter_class_init (PicmanFilterClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->finalize         = picman_filter_finalize;
  object_class->set_property     = picman_filter_set_property;
  object_class->get_property     = picman_filter_get_property;

  picman_object_class->get_memsize = picman_filter_get_memsize;

  klass->get_node                = picman_filter_real_get_node;

  g_object_class_install_property (object_class, PROP_IS_LAST_NODE,
                                   g_param_spec_boolean ("is-last-node",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanFilterPrivate));
}

static void
picman_filter_init (PicmanFilter *filter)
{
}

static void
picman_filter_finalize (GObject *object)
{
  PicmanFilterPrivate *private = GET_PRIVATE (object);

  if (private->node)
    {
      g_object_unref (private->node);
      private->node = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_filter_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PicmanFilterPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IS_LAST_NODE:
      private->is_last_node = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_filter_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PicmanFilterPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_IS_LAST_NODE:
      g_value_set_boolean (value, private->is_last_node);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_filter_get_memsize (PicmanObject *object,
                         gint64     *gui_size)
{
  PicmanFilterPrivate *private = GET_PRIVATE (object);
  gint64             memsize = 0;

  memsize += picman_g_object_get_memsize (G_OBJECT (private->node));

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static GeglNode *
picman_filter_real_get_node (PicmanFilter *filter)
{
  PicmanFilterPrivate *private = GET_PRIVATE (filter);

  private->node = gegl_node_new ();

  return private->node;
}


/*  public functions  */

PicmanFilter *
picman_filter_new (const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);

  return g_object_new (PICMAN_TYPE_FILTER,
                       "name", name,
                       NULL);
}

GeglNode *
picman_filter_get_node (PicmanFilter *filter)
{
  PicmanFilterPrivate *private;

  g_return_val_if_fail (PICMAN_IS_FILTER (filter), NULL);

  private = GET_PRIVATE (filter);

  if (private->node)
    return private->node;

  return PICMAN_FILTER_GET_CLASS (filter)->get_node (filter);
}

GeglNode *
picman_filter_peek_node (PicmanFilter *filter)
{
  g_return_val_if_fail (PICMAN_IS_FILTER (filter), NULL);

  return GET_PRIVATE (filter)->node;
}

void
picman_filter_set_is_last_node (PicmanFilter *filter,
                              gboolean    is_last_node)
{
  PicmanFilterPrivate *private;

  g_return_if_fail (PICMAN_IS_FILTER (filter));

  private = GET_PRIVATE (filter);

  if (is_last_node != private->is_last_node)
    {
      g_object_set (filter,
                    "is-last-node", is_last_node ? TRUE : FALSE,
                    NULL);
    }
}

gboolean
picman_filter_get_is_last_node (PicmanFilter *filter)
{
  g_return_val_if_fail (PICMAN_IS_FILTER (filter), FALSE);

  return GET_PRIVATE (filter)->is_last_node;
}

void
picman_filter_set_applicator (PicmanFilter     *filter,
                            PicmanApplicator *applicator)
{
  PicmanFilterPrivate *private;

  g_return_if_fail (PICMAN_IS_FILTER (filter));

  private = GET_PRIVATE (filter);

  private->applicator = applicator;
}

PicmanApplicator *
picman_filter_get_applicator (PicmanFilter *filter)
{
  g_return_val_if_fail (PICMAN_IS_FILTER (filter), NULL);

  return GET_PRIVATE (filter)->applicator;
}
