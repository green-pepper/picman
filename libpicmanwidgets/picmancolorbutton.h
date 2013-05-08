/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorbutton.h
 * Copyright (C) 1999-2001 Sven Neumann
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

/* This provides a button with a color preview. The preview
 * can handle transparency by showing the checkerboard.
 * On click, a color selector is opened, which is already
 * fully functional wired to the preview button.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_COLOR_BUTTON_H__
#define __PICMAN_COLOR_BUTTON_H__

#include <libpicmanwidgets/picmanbutton.h>

G_BEGIN_DECLS


#define PICMAN_TYPE_COLOR_BUTTON            (picman_color_button_get_type ())
#define PICMAN_COLOR_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_BUTTON, PicmanColorButton))
#define PICMAN_COLOR_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_BUTTON, PicmanColorButtonClass))
#define PICMAN_IS_COLOR_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_BUTTON))
#define PICMAN_IS_COLOR_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_BUTTON))
#define PICMAN_COLOR_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_BUTTON, PicmanColorButtonClass))


typedef struct _PicmanColorButtonClass  PicmanColorButtonClass;

struct _PicmanColorButton
{
  PicmanButton      parent_instance;

  gchar          *title;
  gboolean        continuous_update;

  GtkWidget      *color_area;
  GtkWidget      *dialog;

  /*< private >*/
  gpointer        popup_menu;
};

struct _PicmanColorButtonClass
{
  PicmanButtonClass  parent_class;

  /*  signals  */
  void  (* color_changed)   (PicmanColorButton *button);

  /*  virtual functions  */
  GType (* get_action_type) (PicmanColorButton *button);

  /* Padding for future expansion */
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_color_button_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_color_button_new        (const gchar       *title,
                                          gint               width,
                                          gint               height,
                                          const PicmanRGB     *color,
                                          PicmanColorAreaType  type);

void        picman_color_button_set_color  (PicmanColorButton   *button,
                                          const PicmanRGB     *color);
void        picman_color_button_get_color  (PicmanColorButton   *button,
                                          PicmanRGB           *color);

gboolean    picman_color_button_has_alpha  (PicmanColorButton   *button);
void        picman_color_button_set_type   (PicmanColorButton   *button,
                                          PicmanColorAreaType  type);

gboolean    picman_color_button_get_update (PicmanColorButton   *button);
void        picman_color_button_set_update (PicmanColorButton   *button,
                                          gboolean           continuous);


G_END_DECLS

#endif /* __PICMAN_COLOR_BUTTON_H__ */
