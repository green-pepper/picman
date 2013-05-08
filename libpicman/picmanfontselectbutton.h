/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanfontselectbutton.h
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

#ifndef __PICMAN_FONT_SELECT_BUTTON_H__
#define __PICMAN_FONT_SELECT_BUTTON_H__

#include <libpicman/picmanselectbutton.h>

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_FONT_SELECT_BUTTON            (picman_font_select_button_get_type ())
#define PICMAN_FONT_SELECT_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FONT_SELECT_BUTTON, PicmanFontSelectButton))
#define PICMAN_FONT_SELECT_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FONT_SELECT_BUTTON, PicmanFontSelectButtonClass))
#define PICMAN_IS_FONT_SELECT_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FONT_SELECT_BUTTON))
#define PICMAN_IS_FONT_SELECT_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FONT_SELECT_BUTTON))
#define PICMAN_FONT_SELECT_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FONT_SELECT_BUTTON, PicmanFontSelectButtonClass))


typedef struct _PicmanFontSelectButtonClass  PicmanFontSelectButtonClass;

struct _PicmanFontSelectButton
{
  PicmanSelectButton  parent_instance;
};

struct _PicmanFontSelectButtonClass
{
  PicmanSelectButtonClass  parent_class;

  /* font_set signal is emitted when font is chosen */
  void (* font_set) (PicmanFontSelectButton *button,
                     const gchar          *font_name,
                     gboolean              dialog_closing);

  /* Padding for future expansion */
  void (*_picman_reserved1) (void);
  void (*_picman_reserved2) (void);
  void (*_picman_reserved3) (void);
  void (*_picman_reserved4) (void);
};


GType         picman_font_select_button_get_type (void) G_GNUC_CONST;

GtkWidget   * picman_font_select_button_new      (const gchar          *title,
                                                const gchar          *font_name);

const gchar * picman_font_select_button_get_font (PicmanFontSelectButton *button);
void          picman_font_select_button_set_font (PicmanFontSelectButton *button,
                                                const gchar          *font_name);


G_END_DECLS

#endif /* __PICMAN_FONT_SELECT_BUTTON_H__ */
