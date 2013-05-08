/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasproxygroup.c
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

#include "picmancanvas.h"
#include "picmancanvasproxygroup.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0
};


typedef struct _PicmanCanvasProxyGroupPrivate PicmanCanvasProxyGroupPrivate;

struct _PicmanCanvasProxyGroupPrivate
{
  GHashTable *proxy_hash;
};

#define GET_PRIVATE(proxy_group) \
        G_TYPE_INSTANCE_GET_PRIVATE (proxy_group, \
                                     PICMAN_TYPE_CANVAS_PROXY_GROUP, \
                                     PicmanCanvasProxyGroupPrivate)


/*  local function prototypes  */

static void        picman_canvas_proxy_group_finalize     (GObject          *object);
static void        picman_canvas_proxy_group_set_property (GObject          *object,
                                                         guint             property_id,
                                                         const GValue     *value,
                                                         GParamSpec       *pspec);
static void        picman_canvas_proxy_group_get_property (GObject          *object,
                                                         guint             property_id,
                                                         GValue           *value,
                                                         GParamSpec       *pspec);


G_DEFINE_TYPE (PicmanCanvasProxyGroup, picman_canvas_proxy_group,
               PICMAN_TYPE_CANVAS_GROUP)

#define parent_class picman_canvas_proxy_group_parent_class


static void
picman_canvas_proxy_group_class_init (PicmanCanvasProxyGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_canvas_proxy_group_finalize;
  object_class->set_property = picman_canvas_proxy_group_set_property;
  object_class->get_property = picman_canvas_proxy_group_get_property;

  g_type_class_add_private (klass, sizeof (PicmanCanvasProxyGroupPrivate));
}

static void
picman_canvas_proxy_group_init (PicmanCanvasProxyGroup *proxy_group)
{
  PicmanCanvasProxyGroupPrivate *private = GET_PRIVATE (proxy_group);

  private->proxy_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
picman_canvas_proxy_group_finalize (GObject *object)
{
  PicmanCanvasProxyGroupPrivate *private = GET_PRIVATE (object);

  if (private->proxy_hash)
    {
      g_hash_table_unref (private->proxy_hash);
      private->proxy_hash = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_proxy_group_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  /* PicmanCanvasProxyGroupPrivate *private = GET_PRIVATE (object); */

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_proxy_group_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  /* PicmanCanvasProxyGroupPrivate *private = GET_PRIVATE (object); */

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

PicmanCanvasItem *
picman_canvas_proxy_group_new (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_PROXY_GROUP,
                       "shell", shell,
                       NULL);
}

void
picman_canvas_proxy_group_add_item (PicmanCanvasProxyGroup *group,
                                  gpointer              object,
                                  PicmanCanvasItem       *proxy_item)
{
  PicmanCanvasProxyGroupPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));
  g_return_if_fail (object != NULL);
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (proxy_item));
  g_return_if_fail (PICMAN_CANVAS_ITEM (group) != proxy_item);

  private = GET_PRIVATE (group);

  g_return_if_fail (g_hash_table_lookup (private->proxy_hash, object) ==
                    NULL);

  g_hash_table_insert (private->proxy_hash, object, proxy_item);

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (group), proxy_item);
}

void
picman_canvas_proxy_group_remove_item (PicmanCanvasProxyGroup *group,
                                     gpointer              object)
{
  PicmanCanvasProxyGroupPrivate *private;
  PicmanCanvasItem              *proxy_item;

  g_return_if_fail (PICMAN_IS_CANVAS_GROUP (group));
  g_return_if_fail (object != NULL);

  private = GET_PRIVATE (group);

  proxy_item = g_hash_table_lookup (private->proxy_hash, object);

  g_return_if_fail (proxy_item != NULL);

  g_hash_table_remove (private->proxy_hash, object);

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (group), proxy_item);
}

PicmanCanvasItem *
picman_canvas_proxy_group_get_item (PicmanCanvasProxyGroup *group,
                                  gpointer              object)
{
  PicmanCanvasProxyGroupPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CANVAS_GROUP (group), NULL);
  g_return_val_if_fail (object != NULL, NULL);

  private = GET_PRIVATE (group);

  return g_hash_table_lookup (private->proxy_hash, object);
}
