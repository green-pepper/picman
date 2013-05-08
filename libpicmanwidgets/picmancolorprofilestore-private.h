/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanprofilestore-private.h
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_COLOR_PROFILE_STORE_PRIVATE_H__
#define __PICMAN_COLOR_PROFILE_STORE_PRIVATE_H__


typedef enum
{
  PICMAN_COLOR_PROFILE_STORE_ITEM_FILE,
  PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_TOP,
  PICMAN_COLOR_PROFILE_STORE_ITEM_SEPARATOR_BOTTOM,
  PICMAN_COLOR_PROFILE_STORE_ITEM_DIALOG
} PicmanColorProfileStoreItemType;

typedef enum
{
  PICMAN_COLOR_PROFILE_STORE_ITEM_TYPE,
  PICMAN_COLOR_PROFILE_STORE_LABEL,
  PICMAN_COLOR_PROFILE_STORE_FILENAME,
  PICMAN_COLOR_PROFILE_STORE_INDEX
} PicmanColorProfileStoreColumns;


G_GNUC_INTERNAL gboolean  _picman_color_profile_store_history_add     (PicmanColorProfileStore *store,
                                                                     const gchar           *filename,
                                                                     const gchar           *label,
                                                                     GtkTreeIter           *iter);

G_GNUC_INTERNAL void      _picman_color_profile_store_history_reorder (PicmanColorProfileStore *store,
                                                                     GtkTreeIter           *iter);


#endif  /* __PICMAN_COLOR_PROFILE_STORE_PRIVATE_H__ */
