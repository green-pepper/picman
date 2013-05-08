/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanidtable.h
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_ID_TABLE_H__
#define __PICMAN_ID_TABLE_H__


#include "picmanobject.h"


#define PICMAN_TYPE_ID_TABLE            (picman_id_table_get_type ())
#define PICMAN_ID_TABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ID_TABLE, PicmanIdTable))
#define PICMAN_ID_TABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ID_TABLE, PicmanIdTableClass))
#define PICMAN_IS_ID_TABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ID_TABLE))
#define PICMAN_IS_ID_TABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ID_TABLE))
#define PICMAN_ID_TABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ID_TABLE, PicmanIdTableClass))


typedef struct _PicmanIdTableClass  PicmanIdTableClass;
typedef struct _PicmanIdTablePriv   PicmanIdTablePriv;

struct _PicmanIdTable
{
  PicmanObject       parent_instance;

  PicmanIdTablePriv *priv;
};

struct _PicmanIdTableClass
{
  PicmanObjectClass  parent_class;
};


GType          picman_id_table_get_type       (void) G_GNUC_CONST;
PicmanIdTable *  picman_id_table_new            (void);
gint           picman_id_table_insert         (PicmanIdTable *id_table,
                                             gpointer     data);
gint           picman_id_table_insert_with_id (PicmanIdTable *id_table,
                                             gint         id,
                                             gpointer     data);
void           picman_id_table_replace        (PicmanIdTable *id_table,
                                             gint         id,
                                             gpointer     data);
gpointer       picman_id_table_lookup         (PicmanIdTable *id_table,
                                             gint         id);
gboolean       picman_id_table_remove         (PicmanIdTable *id_table,
                                             gint         id);


#endif  /*  __PICMAN_ID_TABLE_H__  */
