/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolordisplaystack.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"

#include "picmanwidgetstypes.h"

#undef PICMAN_DISABLE_DEPRECATED
#include "picmancolordisplay.h"
#include "picmancolordisplaystack.h"
#include "picmanwidgetsmarshal.h"


/**
 * SECTION: picmancolordisplaystack
 * @title: PicmanColorDisplayStack
 * @short_description: A stack of color correction modules.
 * @see_also: #PicmanColorDisplay
 *
 * A stack of color correction modules.
 **/


enum
{
  CHANGED,
  ADDED,
  REMOVED,
  REORDERED,
  LAST_SIGNAL
};


static void   picman_color_display_stack_dispose         (GObject               *object);

static void   picman_color_display_stack_display_changed (PicmanColorDisplay      *display,
                                                        PicmanColorDisplayStack *stack);
static void   picman_color_display_stack_display_enabled (PicmanColorDisplay      *display,
                                                        GParamSpec            *pspec,
                                                        PicmanColorDisplayStack *stack);
static void   picman_color_display_stack_disconnect      (PicmanColorDisplayStack *stack,
                                                        PicmanColorDisplay      *display);


G_DEFINE_TYPE (PicmanColorDisplayStack, picman_color_display_stack, G_TYPE_OBJECT)

#define parent_class picman_color_display_stack_parent_class

static guint stack_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_display_stack_class_init (PicmanColorDisplayStackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  stack_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorDisplayStackClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  stack_signals[ADDED] =
    g_signal_new ("added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorDisplayStackClass, added),
                  NULL, NULL,
                  _picman_widgets_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_COLOR_DISPLAY,
                  G_TYPE_INT);

  stack_signals[REMOVED] =
    g_signal_new ("removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorDisplayStackClass, removed),
                  NULL, NULL,
                  _picman_widgets_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_COLOR_DISPLAY);

  stack_signals[REORDERED] =
    g_signal_new ("reordered",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorDisplayStackClass, reordered),
                  NULL, NULL,
                  _picman_widgets_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_COLOR_DISPLAY,
                  G_TYPE_INT);

  object_class->dispose = picman_color_display_stack_dispose;

  klass->changed        = NULL;
  klass->added          = NULL;
  klass->removed        = NULL;
  klass->reordered      = NULL;
}

static void
picman_color_display_stack_init (PicmanColorDisplayStack *stack)
{
  stack->filters = NULL;
}

static void
picman_color_display_stack_dispose (GObject *object)
{
  PicmanColorDisplayStack *stack = PICMAN_COLOR_DISPLAY_STACK (object);

  if (stack->filters)
    {
      GList *list;

      for (list = stack->filters; list; list = g_list_next (list))
        {
          PicmanColorDisplay *display = list->data;

          picman_color_display_stack_disconnect (stack, display);
          g_object_unref (display);
        }

      g_list_free (stack->filters);
      stack->filters = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

PicmanColorDisplayStack *
picman_color_display_stack_new (void)
{
  return g_object_new (PICMAN_TYPE_COLOR_DISPLAY_STACK, NULL);
}

PicmanColorDisplayStack *
picman_color_display_stack_clone (PicmanColorDisplayStack *stack)
{
  PicmanColorDisplayStack *clone;
  GList                 *list;

  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack), NULL);

  clone = g_object_new (PICMAN_TYPE_COLOR_DISPLAY_STACK, NULL);

  for (list = stack->filters; list; list = g_list_next (list))
    {
      PicmanColorDisplay *display;

      display = picman_color_display_clone (list->data);

      picman_color_display_stack_add (clone, display);
      g_object_unref (display);
    }

  return clone;
}

void
picman_color_display_stack_changed (PicmanColorDisplayStack *stack)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));

  g_signal_emit (stack, stack_signals[CHANGED], 0);
}

void
picman_color_display_stack_add (PicmanColorDisplayStack *stack,
                              PicmanColorDisplay      *display)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));
  g_return_if_fail (g_list_find (stack->filters, display) == NULL);

  stack->filters = g_list_append (stack->filters, g_object_ref (display));

  g_signal_connect (display, "changed",
                    G_CALLBACK (picman_color_display_stack_display_changed),
                    G_OBJECT (stack));
  g_signal_connect (display, "notify::enabled",
                    G_CALLBACK (picman_color_display_stack_display_enabled),
                    G_OBJECT (stack));

  g_signal_emit (stack, stack_signals[ADDED], 0,
                 display, g_list_length (stack->filters) - 1);

  picman_color_display_stack_changed (stack);
}

void
picman_color_display_stack_remove (PicmanColorDisplayStack *stack,
                                 PicmanColorDisplay      *display)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));
  g_return_if_fail (g_list_find (stack->filters, display) != NULL);

  picman_color_display_stack_disconnect (stack, display);

  stack->filters = g_list_remove (stack->filters, display);

  g_signal_emit (stack, stack_signals[REMOVED], 0, display);

  picman_color_display_stack_changed (stack);

  g_object_unref (display);
}

void
picman_color_display_stack_reorder_up (PicmanColorDisplayStack *stack,
                                     PicmanColorDisplay      *display)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  list = g_list_find (stack->filters, display);

  g_return_if_fail (list != NULL);

  if (list->prev)
    {
      list->data       = list->prev->data;
      list->prev->data = display;

      g_signal_emit (stack, stack_signals[REORDERED], 0,
                     display, g_list_position (stack->filters, list->prev));

      picman_color_display_stack_changed (stack);
    }
}

void
picman_color_display_stack_reorder_down (PicmanColorDisplayStack *stack,
                                       PicmanColorDisplay      *display)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  list = g_list_find (stack->filters, display);

  g_return_if_fail (list != NULL);

  if (list->next)
    {
      list->data       = list->next->data;
      list->next->data = display;

      g_signal_emit (stack, stack_signals[REORDERED], 0,
                     display, g_list_position (stack->filters, list->next));

      picman_color_display_stack_changed (stack);
    }
}

/**
 * picman_color_display_stack_convert_surface:
 * @stack: a #PicmanColorDisplayStack
 * @surface: a #cairo_image_surface_t of type ARGB32
 *
 * Runs all the stack's filters on all pixels in @surface.
 *
 * Since: PICMAN 2.8
 **/
void
picman_color_display_stack_convert_surface (PicmanColorDisplayStack *stack,
                                          cairo_surface_t       *surface)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));
  g_return_if_fail (surface != NULL);
  g_return_if_fail (cairo_surface_get_type (surface) ==
                    CAIRO_SURFACE_TYPE_IMAGE);

  for (list = stack->filters; list; list = g_list_next (list))
    {
      PicmanColorDisplay *display = list->data;

      picman_color_display_convert_surface (display, surface);
    }
}

/**
 * picman_color_display_stack_convert:
 * @stack: a #PicmanColorDisplayStack
 * @buf: the pixel buffer to convert
 * @width: the width of the buffer
 * @height: the height of the buffer
 * @bpp: the number of bytes per pixel
 * @bpl: the buffer's rowstride
 *
 * Converts all pixels in @buf.
 *
 * Deprecated: PICMAN 2.8: Use picman_color_display_stack_convert_surface() instead.
 **/
void
picman_color_display_stack_convert (PicmanColorDisplayStack *stack,
                                  guchar                *buf,
                                  gint                   width,
                                  gint                   height,
                                  gint                   bpp,
                                  gint                   bpl)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY_STACK (stack));

  for (list = stack->filters; list; list = g_list_next (list))
    {
      PicmanColorDisplay *display = list->data;

      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      picman_color_display_convert (display, buf, width, height, bpp, bpl);
      G_GNUC_END_IGNORE_DEPRECATIONS
    }
}


/*  private functions  */

static void
picman_color_display_stack_display_changed (PicmanColorDisplay      *display,
                                          PicmanColorDisplayStack *stack)
{
  if (display->enabled)
    picman_color_display_stack_changed (stack);
}

static void
picman_color_display_stack_display_enabled (PicmanColorDisplay      *display,
                                          GParamSpec            *pspec,
                                          PicmanColorDisplayStack *stack)
{
  picman_color_display_stack_changed (stack);
}

static void
picman_color_display_stack_disconnect (PicmanColorDisplayStack *stack,
                                     PicmanColorDisplay      *display)
{
  g_signal_handlers_disconnect_by_func (display,
                                        picman_color_display_stack_display_changed,
                                        stack);
  g_signal_handlers_disconnect_by_func (display,
                                        picman_color_display_stack_display_enabled,
                                        stack);
}
