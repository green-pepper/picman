/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanchainbutton.h
 * Copyright (C) 1999-2000 Sven Neumann <sven@picman.org>
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

/*
 * This implements a widget derived from GtkTable that visualizes
 * it's state with two different pixmaps showing a closed and a
 * broken chain. It's intended to be used with the PicmanSizeEntry
 * widget. The usage is quite similar to the one the GtkToggleButton
 * provides.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_CHAIN_BUTTON_H__
#define __PICMAN_CHAIN_BUTTON_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_CHAIN_BUTTON            (picman_chain_button_get_type ())
#define PICMAN_CHAIN_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CHAIN_BUTTON, PicmanChainButton))
#define PICMAN_CHAIN_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CHAIN_BUTTON, PicmanChainButtonClass))
#define PICMAN_IS_CHAIN_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CHAIN_BUTTON))
#define PICMAN_IS_CHAIN_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CHAIN_BUTTON))
#define PICMAN_CHAIN_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CHAIN_BUTTON, PicmanChainButtonClass))


typedef struct _PicmanChainButtonClass  PicmanChainButtonClass;

struct _PicmanChainButton
{
  GtkTable           parent_instance;

  PicmanChainPosition  position;
  gboolean           active;

  GtkWidget         *button;
  GtkWidget         *line1;
  GtkWidget         *line2;
  GtkWidget         *image;
};

struct _PicmanChainButtonClass
{
  GtkTableClass  parent_class;

  void (* toggled)  (PicmanChainButton *button);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_chain_button_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_chain_button_new        (PicmanChainPosition  position);

void        picman_chain_button_set_active (PicmanChainButton   *button,
                                          gboolean           active);
gboolean    picman_chain_button_get_active (PicmanChainButton   *button);


G_END_DECLS

#endif /* __PICMAN_CHAIN_BUTTON_H__ */
