/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanunitmenu.h
 * Copyright (C) 1999 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version
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

#ifndef PICMAN_DISABLE_DEPRECATED

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_UNIT_MENU_H__
#define __PICMAN_UNIT_MENU_H__

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#include <gtk/gtkoptionmenu.h>
#define GTK_DISABLE_DEPRECATED
#endif

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_UNIT_MENU            (picman_unit_menu_get_type ())
#define PICMAN_UNIT_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNIT_MENU, PicmanUnitMenu))
#define PICMAN_UNIT_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNIT_MENU, PicmanUnitMenuClass))
#define PICMAN_IS_UNIT_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_UNIT_MENU))
#define PICMAN_IS_UNIT_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNIT_MENU))
#define PICMAN_UNIT_MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNIT_MENU, PicmanUnitMenuClass))


typedef struct _PicmanUnitMenuClass  PicmanUnitMenuClass;

struct _PicmanUnitMenu
{
  GtkOptionMenu  parent_instance;

  /* public (read only) */
  gchar         *format;
  PicmanUnit       unit;
  gint           pixel_digits;

  gboolean       show_pixels;
  gboolean       show_percent;

  /* private */
  GtkWidget     *selection;
  GtkWidget     *tv;
};

struct _PicmanUnitMenuClass
{
  GtkOptionMenuClass  parent_class;

  void (* unit_changed) (PicmanUnitMenu *menu);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType       picman_unit_menu_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_unit_menu_new              (const gchar  *format,
                                             PicmanUnit      unit,
                                             gboolean      show_pixels,
                                             gboolean      show_percent,
                                             gboolean      show_custom);

void        picman_unit_menu_set_unit         (PicmanUnitMenu *menu,
                                             PicmanUnit      unit);

PicmanUnit    picman_unit_menu_get_unit         (PicmanUnitMenu *menu);

void        picman_unit_menu_set_pixel_digits (PicmanUnitMenu *menu,
                                             gint          digits);
gint        picman_unit_menu_get_pixel_digits (PicmanUnitMenu *menu);


G_END_DECLS

#endif /* __PICMAN_UNIT_MENU_H__ */

#endif /* PICMAN_DISABLE_DEPRECATED */
