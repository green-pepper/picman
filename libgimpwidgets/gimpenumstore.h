/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenumstore.h
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_ENUM_STORE_H__
#define __PICMAN_ENUM_STORE_H__

#include <libpicmanwidgets/picmanintstore.h>


G_BEGIN_DECLS

#define PICMAN_TYPE_ENUM_STORE            (picman_enum_store_get_type ())
#define PICMAN_ENUM_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ENUM_STORE, PicmanEnumStore))
#define PICMAN_ENUM_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ENUM_STORE, PicmanEnumStoreClass))
#define PICMAN_IS_ENUM_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ENUM_STORE))
#define PICMAN_IS_ENUM_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ENUM_STORE))
#define PICMAN_ENUM_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ENUM_STORE, PicmanEnumStoreClass))


typedef struct _PicmanEnumStoreClass  PicmanEnumStoreClass;

struct _PicmanEnumStore
{
  PicmanIntStore       parent_instance;

  GEnumClass        *enum_class;
};

struct _PicmanEnumStoreClass
{
  PicmanIntStoreClass  parent_class;

  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType          picman_enum_store_get_type               (void) G_GNUC_CONST;

GtkListStore * picman_enum_store_new                    (GType    enum_type);
GtkListStore * picman_enum_store_new_with_range         (GType    enum_type,
                                                       gint     minimum,
                                                       gint     maximum);
GtkListStore * picman_enum_store_new_with_values        (GType    enum_type,
                                                       gint     n_values,
                                                       ...);
GtkListStore * picman_enum_store_new_with_values_valist (GType    enum_type,
                                                       gint     n_values,
                                                       va_list  args);

void           picman_enum_store_set_stock_prefix (PicmanEnumStore *store,
                                                 const gchar   *stock_prefix);


G_END_DECLS

#endif  /* __PICMAN_ENUM_STORE_H__ */
