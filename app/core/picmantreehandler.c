/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTreeHandler
 * Copyright (C) 2009  Michael Natterer <mitch@picman.org>
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

#include "picmancontainer.h"
#include "picmantreehandler.h"
#include "picmanviewable.h"


static void   picman_tree_handler_dispose          (GObject         *object);

static void   picman_tree_handler_freeze           (PicmanTreeHandler *handler,
                                                  PicmanContainer   *container);
static void   picman_tree_handler_thaw             (PicmanTreeHandler *handler,
                                                  PicmanContainer   *container);

static void   picman_tree_handler_add_container    (PicmanTreeHandler *handler,
                                                  PicmanContainer   *container);
static void   picman_tree_handler_add_foreach      (PicmanViewable    *viewable,
                                                  PicmanTreeHandler *handler);
static void   picman_tree_handler_add              (PicmanTreeHandler *handler,
                                                  PicmanViewable    *viewable,
                                                  PicmanContainer   *container);

static void   picman_tree_handler_remove_container (PicmanTreeHandler *handler,
                                                  PicmanContainer   *container);
static void   picman_tree_handler_remove_foreach   (PicmanViewable    *viewable,
                                                  PicmanTreeHandler *handler);
static void   picman_tree_handler_remove           (PicmanTreeHandler *handler,
                                                  PicmanViewable    *viewable,
                                                  PicmanContainer   *container);


G_DEFINE_TYPE (PicmanTreeHandler, picman_tree_handler, PICMAN_TYPE_OBJECT)

#define parent_class picman_tree_handler_parent_class


static void
picman_tree_handler_class_init (PicmanTreeHandlerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_tree_handler_dispose;
}

static void
picman_tree_handler_init (PicmanTreeHandler *handler)
{
}

static void
picman_tree_handler_dispose (GObject *object)
{
  PicmanTreeHandler *handler = PICMAN_TREE_HANDLER (object);

  if (handler->container)
    {
      g_signal_handlers_disconnect_by_func (handler->container,
                                            picman_tree_handler_freeze,
                                            handler);
      g_signal_handlers_disconnect_by_func (handler->container,
                                            picman_tree_handler_thaw,
                                            handler);

      if (! picman_container_frozen (handler->container))
        picman_tree_handler_remove_container (handler, handler->container);

      g_object_unref (handler->container);
      handler->container = NULL;

      g_free (handler->signal_name);
      handler->signal_name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}


/*  public functions  */

PicmanTreeHandler *
picman_tree_handler_connect (PicmanContainer *container,
                           const gchar   *signal_name,
                           GCallback      callback,
                           gpointer       user_data)
{
  PicmanTreeHandler *handler;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (signal_name != NULL, NULL);

  handler = g_object_new (PICMAN_TYPE_TREE_HANDLER, NULL);

  handler->container   = g_object_ref (container);
  handler->signal_name = g_strdup (signal_name);
  handler->callback    = callback;
  handler->user_data   = user_data;

  if (! picman_container_frozen (container))
    picman_tree_handler_add_container (handler, container);

  g_signal_connect_object (container, "freeze",
                           G_CALLBACK (picman_tree_handler_freeze),
                           handler,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_tree_handler_thaw),
                           handler,
                           G_CONNECT_SWAPPED);

  return handler;
}

void
picman_tree_handler_disconnect (PicmanTreeHandler *handler)
{
  g_return_if_fail (PICMAN_IS_TREE_HANDLER (handler));

  g_object_run_dispose (G_OBJECT (handler));
  g_object_unref (handler);
}


/*  private functions  */

static void
picman_tree_handler_freeze (PicmanTreeHandler *handler,
                          PicmanContainer   *container)
{
  picman_tree_handler_remove_container (handler, container);
}

static void
picman_tree_handler_thaw (PicmanTreeHandler *handler,
                        PicmanContainer   *container)
{
  picman_tree_handler_add_container (handler, container);
}

static void
picman_tree_handler_add_container (PicmanTreeHandler *handler,
                                 PicmanContainer   *container)
{
  picman_container_foreach (container,
                          (GFunc) picman_tree_handler_add_foreach,
                          handler);

  g_signal_connect_object (container, "add",
                           G_CALLBACK (picman_tree_handler_add),
                           handler,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_tree_handler_remove),
                           handler,
                           G_CONNECT_SWAPPED);
}

static void
picman_tree_handler_add_foreach (PicmanViewable    *viewable,
                               PicmanTreeHandler *handler)
{
  picman_tree_handler_add (handler, viewable, NULL);
}

static void
picman_tree_handler_add (PicmanTreeHandler *handler,
                       PicmanViewable    *viewable,
                       PicmanContainer   *unused)
{
  PicmanContainer *children = picman_viewable_get_children (viewable);

  g_signal_connect (viewable,
                    handler->signal_name,
                    handler->callback,
                    handler->user_data);

  if (children)
    picman_tree_handler_add_container (handler, children);
}

static void
picman_tree_handler_remove_container (PicmanTreeHandler *handler,
                                    PicmanContainer   *container)
{
  g_signal_handlers_disconnect_by_func (container,
                                        picman_tree_handler_add,
                                        handler);
  g_signal_handlers_disconnect_by_func (container,
                                        picman_tree_handler_remove,
                                        handler);

  picman_container_foreach (container,
                          (GFunc) picman_tree_handler_remove_foreach,
                          handler);
}

static void
picman_tree_handler_remove_foreach (PicmanViewable    *viewable,
                                  PicmanTreeHandler *handler)
{
  picman_tree_handler_remove (handler, viewable, NULL);
}

static void
picman_tree_handler_remove (PicmanTreeHandler *handler,
                          PicmanViewable    *viewable,
                          PicmanContainer   *unused)
{
  PicmanContainer *children = picman_viewable_get_children (viewable);

  if (children)
    picman_tree_handler_remove_container (handler, children);

  g_signal_handlers_disconnect_by_func (viewable,
                                        handler->callback,
                                        handler->user_data);
}
