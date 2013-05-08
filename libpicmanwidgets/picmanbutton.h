/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanbutton.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_BUTTON_H__
#define __PICMAN_BUTTON_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_BUTTON            (picman_button_get_type ())
#define PICMAN_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BUTTON, PicmanButton))
#define PICMAN_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BUTTON, PicmanButtonClass))
#define PICMAN_IS_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BUTTON))
#define PICMAN_IS_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BUTTON))
#define PICMAN_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BUTTON, PicmanButtonClass))


typedef struct _PicmanButtonClass  PicmanButtonClass;

struct _PicmanButton
{
  GtkButton        parent_instance;

  /*< private >*/
  GdkModifierType  press_state;
};

struct _PicmanButtonClass
{
  GtkButtonClass  parent_class;

  void (* extended_clicked) (PicmanButton      *button,
                             GdkModifierType  modifier_state);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_button_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_button_new              (void);

void        picman_button_extended_clicked (PicmanButton      *button,
                                          GdkModifierType  state);


G_END_DECLS

#endif /* __PICMAN_BUTTON_H__ */
