/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasgroup.c
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "picmancanvasgroup.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_GROUP_STROKING,
  PROP_GROUP_FILLING
};


typedef struct _PicmanCanvasGroupPrivate PicmanCanvasGroupPrivate;

struct _PicmanCanvasGroupPrivate
{
  GList    *items;
  gboolean  group_stroking;
  gboolean  group_filling;
};

#define GET_PRIVATE(group) \
        G_TYPE_INSTANCE_GET_PRIVATE (group, \
                                     PICMAN_TYPE_CANVAS_GROUP, \
                                     PicmanCanvasGroupPrivate)


/*  local function prototypes  */

static void             picman_canvas_group_dispose      (GObject         *object);
static void             picman_canvas_group_set_property (GObject         *object,
                                                        guint            property_id,
                                                        const GValue    *value,
                                                        GParamSpec      *pspec);
static void             picman_canvas_group_get_property (GObject         *object,
                                                        guint            property_id,
                                                        GValue          *value,
                                                        GParamSpec      *pspec);
static void             picman_canvas_group_draw         (PicmanCanvasItem  *item,
                                                        cairo_t         *cr);
static cairo_region_t * picman_canvas_group_get_extents  (PicmanCanvasItem  *item);
static gboolean         picman_canvas_group_hit          (PicmanCanvasItem  *item,
                                                        gdouble          x,
                                                        gdouble          y);

static void             picman_canvas_group_child_update (PicmanCanvasItem  *item,
                                                        cairo_region_t  *region,
                                                        PicmanCanvasGroup *group);


G_DEFINE_TYPE (PicmanCanvasGroup, picman_canvas_group, PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_group_parent_class


static void
picman_canvas_group_class_init (PicmanCanvasGroupClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->dispose      = picman_canvas_group_dispose;
  object_class->set_property = picman_canvas_group_set_property;
  object_class->get_property = picman_canvas_group_get_property;

  item_class->draw           = picman_canvas_group_draw;
  item_class->get_extents    = picman_canvas_group_get_extents;
  item_class->hit            = picman_canvas_group_hit;

  g_object_class_install_property (object_class, PROP_GROUP_STROKING,
                                   g_param_spec_boolean ("group-stroking",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_GROUP_FILLING,
                                   g_param_spec_boolean ("group-filling",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasGroupPrivate));
}

static void
picman_canvas_group_init (PicmanCanvasGroup *group)
{
}

static void
picman_canvas_group_dispose (GObject *object)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (object);

  if (private->items)
    {
      g_list_free_full (private->items, (GDestroyNotify) g_object_unref);
      private->items = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_canvas_group_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_GROUP_STROKING:
      private->group_stroking = g_value_get_boolean (value);
      break;
    case PROP_GROUP_FILLING:
      private->group_filling = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_group_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_GROUP_STROKING:
      g_value_set_boolean (value, private->group_stroking);
      break;
    case PROP_GROUP_FILLING:
      g_value_set_boolean (value, private->group_filling);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_group_draw (PicmanCanvasItem *item,
                        cairo_t        *cr)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (item);
  GList                  *list;

  for (list = private->items; list; list = g_list_next (list))
    {
      PicmanCanvasItem *sub_item = list->data;

      picman_canvas_item_draw (sub_item, cr);
    }

  if (private->group_stroking)
    _picman_canvas_item_stroke (item, cr);

  if (private->group_filling)
    _picman_canvas_item_fill (item, cr);
}

static cairo_region_t *
picman_canvas_group_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (item);
  cairo_region_t         *region  = NULL;
  GList                  *list;

  for (list = private->items; list; list = g_list_next (list))
    {
      PicmanCanvasItem *sub_item   = list->data;
      cairo_region_t *sub_region = picman_canvas_item_get_extents (sub_item);

      if (! region)
        {
          region = sub_region;
        }
      else if (sub_region)
        {
          cairo_region_union (region, sub_region);
          cairo_region_destroy (sub_region);
        }
    }

  return region;
}

static gboolean
picman_canvas_group_hit (PicmanCanvasItem *item,
                       gdouble         x,
                       gdouble         y)
{
  PicmanCanvasGroupPrivate *private = GET_PRIVATE (item);
  GList                  *list;

  for (list = private->items; list; list = g_list_next (list))
    {
      if (picman_canvas_item_hit (list->data, x, y))
        return TRUE;
    }

  return FALSE;
}

static void
picman_canvas_group_child_update (PicmanCanvasItem  *item,
                                cairo_region_t  *region,
                                PicmanCanvasGroup *group)
{
  if (_picman_canvas_item_needs_update (PICMAN_CANVAS_ITEM (group)))
    _picman_canvas_item_update (PICMAN_CANVAS_ITEM (group), region);
}


/*  public functions  */

PicmanCanvasItem *
picman_canvas_group_new (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_GROUP,
                       "shell", shell,
                       NULL);
}

void
picman_canvas_group_add_item (PicmanCanvasGroup *group,
                            PicmanCanvasItem  *item)
{
  PicmanCanvasGroupPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));
  g_return_if_fail (PICMAN_CANVAS_ITEM (group) != item);

  private = GET_PRIVATE (group);

  if (private->group_stroking)
    picman_canvas_item_suspend_stroking (item);

  if (private->group_filling)
    picman_canvas_item_suspend_filling (item);

  private->items = g_list_append (private->items, g_object_ref (item));

  if (_picman_canvas_item_needs_update (PICMAN_CANVAS_ITEM (group)))
    {
      cairo_region_t *region = picman_canvas_item_get_extents (item);

      if (region)
        {
          _picman_canvas_item_update (PICMAN_CANVAS_ITEM (group), region);
          cairo_region_destroy (region);
        }
    }

  g_signal_connect (item, "update",
                    G_CALLBACK (picman_canvas_group_child_update),
                    group);
}

void
picman_canvas_group_remove_item (PicmanCanvasGroup *group,
                               PicmanCanvasItem  *item)
{
  PicmanCanvasGroupPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  private = GET_PRIVATE (group);

  g_return_if_fail (g_list_find (private->items, item));

  private->items = g_list_remove (private->items, item);

  if (private->group_stroking)
    picman_canvas_item_resume_stroking (item);

  if (private->group_filling)
    picman_canvas_item_resume_filling (item);

  if (_picman_canvas_item_needs_update (PICMAN_CANVAS_ITEM (group)))
    {
      cairo_region_t *region = picman_canvas_item_get_extents (item);

      if (region)
        {
          _picman_canvas_item_update (PICMAN_CANVAS_ITEM (group), region);
          cairo_region_destroy (region);
        }
    }

  g_signal_handlers_disconnect_by_func (item,
                                        picman_canvas_group_child_update,
                                        group);

  g_object_unref (item);
}

void
picman_canvas_group_set_group_stroking (PicmanCanvasGroup *group,
                                      gboolean         group_stroking)
{
  PicmanCanvasGroupPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));

  private = GET_PRIVATE (group);

  if (private->group_stroking != group_stroking)
    {
      GList *list;

      picman_canvas_item_begin_change (PICMAN_CANVAS_ITEM (group));

      g_object_set (group,
                    "group-stroking", group_stroking ? TRUE : FALSE,
                    NULL);

      for (list = private->items; list; list = g_list_next (list))
        {
          if (private->group_stroking)
            picman_canvas_item_suspend_stroking (list->data);
          else
            picman_canvas_item_resume_stroking (list->data);
        }

      picman_canvas_item_end_change (PICMAN_CANVAS_ITEM (group));
    }
}

void
picman_canvas_group_set_group_filling (PicmanCanvasGroup *group,
                                     gboolean         group_filling)
{
  PicmanCanvasGroupPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));

  private = GET_PRIVATE (group);

  if (private->group_filling != group_filling)
    {
      GList *list;

      picman_canvas_item_begin_change (PICMAN_CANVAS_ITEM (group));

      g_object_set (group,
                    "group-filling", group_filling ? TRUE : FALSE,
                    NULL);

      for (list = private->items; list; list = g_list_next (list))
        {
          if (private->group_filling)
            picman_canvas_item_suspend_filling (list->data);
          else
            picman_canvas_item_resume_filling (list->data);
        }

      picman_canvas_item_end_change (PICMAN_CANVAS_ITEM (group));
    }
}
