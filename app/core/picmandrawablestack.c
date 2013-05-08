/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmandrawablestack.c
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

#include <gegl.h>

#include "core-types.h"

#include "picmandrawable.h"
#include "picmandrawablestack.h"
#include "picmanmarshal.h"


enum
{
  UPDATE,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void   picman_drawable_stack_constructed      (GObject           *object);

static void   picman_drawable_stack_add              (PicmanContainer     *container,
                                                    PicmanObject        *object);
static void   picman_drawable_stack_remove           (PicmanContainer     *container,
                                                    PicmanObject        *object);
static void   picman_drawable_stack_reorder          (PicmanContainer     *container,
                                                    PicmanObject        *object,
                                                    gint               new_index);

static void   picman_drawable_stack_update           (PicmanDrawableStack *stack,
                                                    gint               x,
                                                    gint               y,
                                                    gint               width,
                                                    gint               height);
static void   picman_drawable_stack_drawable_update  (PicmanItem          *item,
                                                    gint               x,
                                                    gint               y,
                                                    gint               width,
                                                    gint               height,
                                                    PicmanDrawableStack *stack);
static void   picman_drawable_stack_drawable_visible (PicmanItem          *item,
                                                    PicmanDrawableStack *stack);


G_DEFINE_TYPE (PicmanDrawableStack, picman_drawable_stack, PICMAN_TYPE_ITEM_STACK)

#define parent_class picman_drawable_stack_parent_class

static guint stack_signals[LAST_SIGNAL] = { 0 };


static void
picman_drawable_stack_class_init (PicmanDrawableStackClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  PicmanContainerClass *container_class = PICMAN_CONTAINER_CLASS (klass);

  stack_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDrawableStackClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__INT_INT_INT_INT,
                  G_TYPE_NONE, 4,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT);

  object_class->constructed = picman_drawable_stack_constructed;

  container_class->add      = picman_drawable_stack_add;
  container_class->remove   = picman_drawable_stack_remove;
  container_class->reorder  = picman_drawable_stack_reorder;
}

static void
picman_drawable_stack_init (PicmanDrawableStack *stack)
{
}

static void
picman_drawable_stack_constructed (GObject *object)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (g_type_is_a (picman_container_get_children_type (container),
                         PICMAN_TYPE_DRAWABLE));

  picman_container_add_handler (container, "update",
                              G_CALLBACK (picman_drawable_stack_drawable_update),
                              container);
  picman_container_add_handler (container, "visibility-changed",
                              G_CALLBACK (picman_drawable_stack_drawable_visible),
                              container);
}

static void
picman_drawable_stack_add (PicmanContainer *container,
                         PicmanObject    *object)
{
  PicmanDrawableStack *stack = PICMAN_DRAWABLE_STACK (container);

  PICMAN_CONTAINER_CLASS (parent_class)->add (container, object);

  if (picman_item_get_visible (PICMAN_ITEM (object)))
    picman_drawable_stack_drawable_visible (PICMAN_ITEM (object), stack);
}

static void
picman_drawable_stack_remove (PicmanContainer *container,
                            PicmanObject    *object)
{
  PicmanDrawableStack *stack = PICMAN_DRAWABLE_STACK (container);

  PICMAN_CONTAINER_CLASS (parent_class)->remove (container, object);

  if (picman_item_get_visible (PICMAN_ITEM (object)))
    picman_drawable_stack_drawable_visible (PICMAN_ITEM (object), stack);
}

static void
picman_drawable_stack_reorder (PicmanContainer *container,
                             PicmanObject    *object,
                             gint           new_index)
{
  PicmanDrawableStack *stack  = PICMAN_DRAWABLE_STACK (container);

  PICMAN_CONTAINER_CLASS (parent_class)->reorder (container, object, new_index);

  if (picman_item_get_visible (PICMAN_ITEM (object)))
    picman_drawable_stack_drawable_visible (PICMAN_ITEM (object), stack);
}


/*  public functions  */

PicmanContainer *
picman_drawable_stack_new (GType drawable_type)
{
  g_return_val_if_fail (g_type_is_a (drawable_type, PICMAN_TYPE_DRAWABLE), NULL);

  return g_object_new (PICMAN_TYPE_DRAWABLE_STACK,
                       "name",          g_type_name (drawable_type),
                       "children-type", drawable_type,
                       "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                       NULL);
}


/*  private functions  */

static void
picman_drawable_stack_update (PicmanDrawableStack *stack,
                            gint               x,
                            gint               y,
                            gint               width,
                            gint               height)
{
  g_signal_emit (stack, stack_signals[UPDATE], 0,
                 x, y, width, height);
}

static void
picman_drawable_stack_drawable_update (PicmanItem          *item,
                                     gint               x,
                                     gint               y,
                                     gint               width,
                                     gint               height,
                                     PicmanDrawableStack *stack)
{
  if (picman_item_get_visible (item))
    {
      gint offset_x;
      gint offset_y;

      picman_item_get_offset (item, &offset_x, &offset_y);

      picman_drawable_stack_update (stack,
                                  x + offset_x, y + offset_y,
                                  width, height);
    }
}

static void
picman_drawable_stack_drawable_visible (PicmanItem          *item,
                                      PicmanDrawableStack *stack)
{
  gint offset_x;
  gint offset_y;

  picman_item_get_offset (item, &offset_x, &offset_y);

  picman_drawable_stack_update (stack,
                              offset_x, offset_y,
                              picman_item_get_width  (item),
                              picman_item_get_height (item));
}
