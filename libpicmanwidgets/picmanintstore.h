/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanintstore.c
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

#ifndef __PICMAN_INT_STORE_H__
#define __PICMAN_INT_STORE_H__

G_BEGIN_DECLS


/**
 * PicmanIntStoreColumns:
 * @PICMAN_INT_STORE_VALUE:       the integer value
 * @PICMAN_INT_STORE_LABEL:       a human-readable label
 * @PICMAN_INT_STORE_STOCK_ID:    a stock ID
 * @PICMAN_INT_STORE_PIXBUF:      a #GdkPixbuf
 * @PICMAN_INT_STORE_USER_DATA:   arbitrary user data
 * @PICMAN_INT_STORE_NUM_COLUMNS: the number of columns
 *
 * The column types of #PicmanIntStore.
 **/
typedef enum
{
  PICMAN_INT_STORE_VALUE,
  PICMAN_INT_STORE_LABEL,
  PICMAN_INT_STORE_STOCK_ID,
  PICMAN_INT_STORE_PIXBUF,
  PICMAN_INT_STORE_USER_DATA,
  PICMAN_INT_STORE_NUM_COLUMNS
} PicmanIntStoreColumns;


#define PICMAN_TYPE_INT_STORE            (picman_int_store_get_type ())
#define PICMAN_INT_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INT_STORE, PicmanIntStore))
#define PICMAN_INT_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INT_STORE, PicmanIntStoreClass))
#define PICMAN_IS_INT_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INT_STORE))
#define PICMAN_IS_INT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INT_STORE))
#define PICMAN_INT_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INT_STORE, PicmanIntStoreClass))


typedef struct _PicmanIntStoreClass  PicmanIntStoreClass;

struct _PicmanIntStore
{
  GtkListStore  parent_instance;

  /*< private >*/
  GtkTreeIter  *empty_iter;
};

struct _PicmanIntStoreClass
{
  GtkListStoreClass  parent_class;

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType          picman_int_store_get_type        (void) G_GNUC_CONST;

GtkListStore * picman_int_store_new             (void);

gboolean       picman_int_store_lookup_by_value (GtkTreeModel  *model,
                                               gint           value,
                                               GtkTreeIter   *iter);


G_END_DECLS

#endif  /* __PICMAN_INT_STORE_H__ */
