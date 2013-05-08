/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolordisplaystack.h
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_DISPLAY_STACK_H__
#define __PICMAN_COLOR_DISPLAY_STACK_H__

G_BEGIN_DECLS

/* For information look at the html documentation */


#define PICMAN_TYPE_COLOR_DISPLAY_STACK            (picman_color_display_stack_get_type ())
#define PICMAN_COLOR_DISPLAY_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_DISPLAY_STACK, PicmanColorDisplayStack))
#define PICMAN_COLOR_DISPLAY_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_DISPLAY_STACK, PicmanColorDisplayStackClass))
#define PICMAN_IS_COLOR_DISPLAY_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_DISPLAY_STACK))
#define PICMAN_IS_COLOR_DISPLAY_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_DISPLAY_STACK))
#define PICMAN_COLOR_DISPLAY_STACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_DISPLAY_STACK, PicmanColorDisplayStackClass))


typedef struct _PicmanColorDisplayStackClass PicmanColorDisplayStackClass;

struct _PicmanColorDisplayStack
{
  GObject  parent_instance;

  GList   *filters;
};

struct _PicmanColorDisplayStackClass
{
  GObjectClass  parent_class;

  void (* changed)   (PicmanColorDisplayStack *stack);

  void (* added)     (PicmanColorDisplayStack *stack,
                      PicmanColorDisplay      *display,
                      gint                   position);
  void (* removed)   (PicmanColorDisplayStack *stack,
                      PicmanColorDisplay      *display);
  void (* reordered) (PicmanColorDisplayStack *stack,
                      PicmanColorDisplay      *display,
                      gint                   position);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType                   picman_color_display_stack_get_type (void) G_GNUC_CONST;
PicmanColorDisplayStack * picman_color_display_stack_new      (void);
PicmanColorDisplayStack * picman_color_display_stack_clone    (PicmanColorDisplayStack *stack);

void   picman_color_display_stack_changed         (PicmanColorDisplayStack *stack);

void   picman_color_display_stack_add             (PicmanColorDisplayStack *stack,
                                                 PicmanColorDisplay      *display);
void   picman_color_display_stack_remove          (PicmanColorDisplayStack *stack,
                                                 PicmanColorDisplay      *display);
void   picman_color_display_stack_reorder_up      (PicmanColorDisplayStack *stack,
                                                 PicmanColorDisplay      *display);
void   picman_color_display_stack_reorder_down    (PicmanColorDisplayStack *stack,
                                                 PicmanColorDisplay      *display);
void   picman_color_display_stack_convert_surface (PicmanColorDisplayStack *stack,
                                                 cairo_surface_t       *surface);
PICMAN_DEPRECATED_FOR(picman_color_display_stack_convert_surface)
void   picman_color_display_stack_convert         (PicmanColorDisplayStack *stack,
                                                 guchar                *buf,
                                                 gint                   width,
                                                 gint                   height,
                                                 gint                   bpp,
                                                 gint                   bpl);

G_END_DECLS

#endif /* __PICMAN_COLOR_DISPLAY_STACK_H__ */
