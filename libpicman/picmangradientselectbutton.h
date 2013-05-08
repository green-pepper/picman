/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmangradientselectbutton.h
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

#if !defined (__PICMAN_UI_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picmanui.h> can be included directly."
#endif

#ifndef __PICMAN_GRADIENT_SELECT_BUTTON_H__
#define __PICMAN_GRADIENT_SELECT_BUTTON_H__

#include <libpicman/picmanselectbutton.h>

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_GRADIENT_SELECT_BUTTON            (picman_gradient_select_button_get_type ())
#define PICMAN_GRADIENT_SELECT_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GRADIENT_SELECT_BUTTON, PicmanGradientSelectButton))
#define PICMAN_GRADIENT_SELECT_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GRADIENT_SELECT_BUTTON, PicmanGradientSelectButtonClass))
#define PICMAN_IS_GRADIENT_SELECT_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GRADIENT_SELECT_BUTTON))
#define PICMAN_IS_GRADIENT_SELECT_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GRADIENT_SELECT_BUTTON))
#define PICMAN_GRADIENT_SELECT_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GRADIENT_SELECT_BUTTON, PicmanGradientSelectButtonClass))


typedef struct _PicmanGradientSelectButtonClass   PicmanGradientSelectButtonClass;

struct _PicmanGradientSelectButton
{
  PicmanSelectButton  parent_instance;
};

struct _PicmanGradientSelectButtonClass
{
  PicmanSelectButtonClass  parent_class;

  /* gradient_set signal is emitted when gradient is chosen */
  void (* gradient_set) (PicmanGradientSelectButton *button,
                         const gchar              *gradient_name,
                         gint                      width,
                         const gdouble            *gradient_data,
                         gboolean                  dialog_closing);

  /* Padding for future expansion */
  void (*_picman_reserved1) (void);
  void (*_picman_reserved2) (void);
  void (*_picman_reserved3) (void);
  void (*_picman_reserved4) (void);
};


GType         picman_gradient_select_button_get_type     (void) G_GNUC_CONST;

GtkWidget   * picman_gradient_select_button_new          (const gchar *title,
                                                        const gchar *gradient_name);

const gchar * picman_gradient_select_button_get_gradient (PicmanGradientSelectButton *button);
void          picman_gradient_select_button_set_gradient (PicmanGradientSelectButton *button,
                                                        const gchar              *gradient_name);


G_END_DECLS

#endif /* __PICMAN_GRADIENT_SELECT_BUTTON_H__ */
