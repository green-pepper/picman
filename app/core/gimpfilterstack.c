/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanfilterstack.c
 * Copyright (C) 2008-2013 Michael Natterer <mitch@picman.org>
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

#include "picmanfilter.h"
#include "picmanfilterstack.h"


/*  local function prototypes  */

static void   picman_filter_stack_constructed (GObject         *object);
static void   picman_filter_stack_finalize    (GObject         *object);

static void   picman_filter_stack_add         (PicmanContainer   *container,
                                             PicmanObject      *object);
static void   picman_filter_stack_remove      (PicmanContainer   *container,
                                             PicmanObject      *object);
static void   picman_filter_stack_reorder     (PicmanContainer   *container,
                                             PicmanObject      *object,
                                             gint             new_index);

static void   picman_filter_stack_add_node    (PicmanFilterStack *stack,
                                             PicmanFilter      *filter);
static void   picman_filter_stack_remove_node (PicmanFilterStack *stack,
                                             PicmanFilter      *filter);


G_DEFINE_TYPE (PicmanFilterStack, picman_filter_stack, PICMAN_TYPE_LIST);

#define parent_class picman_filter_stack_parent_class


static void
picman_filter_stack_class_init (PicmanFilterStackClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  PicmanContainerClass *container_class = PICMAN_CONTAINER_CLASS (klass);

  object_class->constructed = picman_filter_stack_constructed;
  object_class->finalize    = picman_filter_stack_finalize;

  container_class->add      = picman_filter_stack_add;
  container_class->remove   = picman_filter_stack_remove;
  container_class->reorder  = picman_filter_stack_reorder;
}

static void
picman_filter_stack_init (PicmanFilterStack *stack)
{
}

static void
picman_filter_stack_constructed (GObject *object)
{
  PicmanContainer *container = PICMAN_CONTAINER (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (g_type_is_a (picman_container_get_children_type (container),
                         PICMAN_TYPE_FILTER));
}

static void
picman_filter_stack_finalize (GObject *object)
{
  PicmanFilterStack *stack = PICMAN_FILTER_STACK (object);

  if (stack->graph)
    {
      g_object_unref (stack->graph);
      stack->graph = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_filter_stack_add (PicmanContainer *container,
                       PicmanObject    *object)
{
  PicmanFilterStack *stack = PICMAN_FILTER_STACK (container);

  PICMAN_CONTAINER_CLASS (parent_class)->add (container, object);

  if (stack->graph)
    {
      PicmanFilter *filter = PICMAN_FILTER (object);

      gegl_node_add_child (stack->graph, picman_filter_get_node (filter));
      picman_filter_stack_add_node (stack, filter);
    }
}

static void
picman_filter_stack_remove (PicmanContainer *container,
                          PicmanObject    *object)
{
  PicmanFilterStack *stack = PICMAN_FILTER_STACK (container);

  if (stack->graph)
    {
      PicmanFilter *filter = PICMAN_FILTER (object);

      picman_filter_stack_remove_node (stack, filter);
      gegl_node_remove_child (stack->graph, picman_filter_get_node (filter));
    }

  PICMAN_CONTAINER_CLASS (parent_class)->remove (container, object);
}

static void
picman_filter_stack_reorder (PicmanContainer *container,
                           PicmanObject    *object,
                           gint           new_index)
{
  PicmanFilterStack *stack  = PICMAN_FILTER_STACK (container);
  PicmanFilter      *filter = PICMAN_FILTER (object);

  if (stack->graph)
    picman_filter_stack_remove_node (stack, filter);

  PICMAN_CONTAINER_CLASS (parent_class)->reorder (container, object, new_index);

  if (stack->graph)
    picman_filter_stack_add_node (stack, filter);
}


/*  public functions  */

PicmanContainer *
picman_filter_stack_new (GType filter_type)
{
  g_return_val_if_fail (g_type_is_a (filter_type, PICMAN_TYPE_FILTER), NULL);

  return g_object_new (PICMAN_TYPE_FILTER_STACK,
                       "name",          g_type_name (filter_type),
                       "children-type", filter_type,
                       "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                       NULL);
}

GeglNode *
picman_filter_stack_get_graph (PicmanFilterStack *stack)
{
  GList    *list;
  GList    *reverse_list = NULL;
  GeglNode *first        = NULL;
  GeglNode *previous     = NULL;
  GeglNode *input;
  GeglNode *output;

  g_return_val_if_fail (PICMAN_IS_FILTER_STACK (stack), NULL);

  if (stack->graph)
    return stack->graph;

  for (list = PICMAN_LIST (stack)->list;
       list;
       list = g_list_next (list))
    {
      PicmanFilter *filter = list->data;

      reverse_list = g_list_prepend (reverse_list, filter);

      if (! g_list_next (list))
        picman_filter_set_is_last_node (filter, TRUE);
    }

  stack->graph = gegl_node_new ();

  for (list = reverse_list; list; list = g_list_next (list))
    {
      PicmanFilter *filter = list->data;
      GeglNode   *node   = picman_filter_get_node (filter);

      if (! first)
        first = node;

      gegl_node_add_child (stack->graph, node);

      if (previous)
        gegl_node_connect_to (previous, "output",
                              node,     "input");

      previous = node;
    }

  g_list_free (reverse_list);

  input  = gegl_node_get_input_proxy  (stack->graph, "input");
  output = gegl_node_get_output_proxy (stack->graph, "output");

  if (first && previous)
    {
      gegl_node_connect_to (input, "output",
                            first, "input");
      gegl_node_connect_to (previous, "output",
                            output,   "input");
    }
  else
    {
      gegl_node_connect_to (input,  "output",
                            output, "input");
    }

  return stack->graph;
}


/*  private functions  */

static void
picman_filter_stack_add_node (PicmanFilterStack *stack,
                              PicmanFilter        *filter)
{
  PicmanFilter *filter_above = NULL;
  PicmanFilter *filter_below;
  GeglNode   *node_above;
  GeglNode   *node_below;
  GeglNode   *node;
  gint        index;

  node = picman_filter_get_node (filter);

  index = picman_container_get_child_index (PICMAN_CONTAINER (stack),
                                          PICMAN_OBJECT (filter));

  if (index == 0)
    {
      node_above = gegl_node_get_output_proxy (stack->graph, "output");
    }
  else
    {
      filter_above = (PicmanFilter *)
        picman_container_get_child_by_index (PICMAN_CONTAINER (stack), index - 1);

      node_above = picman_filter_get_node (filter_above);
    }

  gegl_node_connect_to (node,       "output",
                        node_above, "input");

  filter_below = (PicmanFilter *)
    picman_container_get_child_by_index (PICMAN_CONTAINER (stack), index + 1);

  if (filter_below)
    {
      node_below = picman_filter_get_node (filter_below);
    }
  else
    {
      node_below = gegl_node_get_input_proxy (stack->graph, "input");

      if (filter_above)
        picman_filter_set_is_last_node (filter_above, FALSE);

      picman_filter_set_is_last_node (filter, TRUE);
    }

  gegl_node_connect_to (node_below, "output",
                        node,       "input");
}

static void
picman_filter_stack_remove_node (PicmanFilterStack *stack,
                                 PicmanFilter        *filter)
{
  PicmanFilter *filter_above = NULL;
  PicmanFilter *filter_below;
  GeglNode   *node_above;
  GeglNode   *node_below;
  GeglNode   *node;
  gint        index;

  node = picman_filter_get_node (filter);

  gegl_node_disconnect (node, "input");

  index = picman_container_get_child_index (PICMAN_CONTAINER (stack),
                                          PICMAN_OBJECT (filter));

  if (index == 0)
    {
      node_above = gegl_node_get_output_proxy (stack->graph, "output");
    }
  else
    {
      filter_above = (PicmanFilter *)
        picman_container_get_child_by_index (PICMAN_CONTAINER (stack), index - 1);

      node_above = picman_filter_get_node (filter_above);
    }

  filter_below = (PicmanFilter *)
    picman_container_get_child_by_index (PICMAN_CONTAINER (stack), index + 1);

  if (filter_below)
    {
      node_below = picman_filter_get_node (filter_below);
    }
  else
    {
      node_below = gegl_node_get_input_proxy (stack->graph, "input");

      picman_filter_set_is_last_node (filter, FALSE);

      if (filter_above)
        picman_filter_set_is_last_node (filter_above, TRUE);
    }

  gegl_node_connect_to (node_below, "output",
                        node_above, "input");
}
