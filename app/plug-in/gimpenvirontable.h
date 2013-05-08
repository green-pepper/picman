/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanenvirontable.h
 * (C) 2002 Manish Singh <yosh@picman.org>
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

#ifndef __PICMAN_ENVIRON_TABLE_H__
#define __PICMAN_ENVIRON_TABLE_H__


#define PICMAN_TYPE_ENVIRON_TABLE            (picman_environ_table_get_type ())
#define PICMAN_ENVIRON_TABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ENVIRON_TABLE, PicmanEnvironTable))
#define PICMAN_ENVIRON_TABLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ENVIRON_TABLE, PicmanEnvironTableClass))
#define PICMAN_IS_ENVIRON_TABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ENVIRON_TABLE))
#define PICMAN_IS_ENVIRON_TABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ENVIRON_TABLE))
#define PICMAN_ENVIRON_TABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ENVIRON_TABLE, PicmanEnvironTableClass))


typedef struct _PicmanEnvironTableClass PicmanEnvironTableClass;

struct _PicmanEnvironTable
{
  GObject      parent_instance;

  GHashTable  *vars;
  GHashTable  *internal;

  gchar      **envp;
};

struct _PicmanEnvironTableClass
{
  GObjectClass  parent_class;
};


GType               picman_environ_table_get_type  (void) G_GNUC_CONST;
PicmanEnvironTable  * picman_environ_table_new       (void);

void                picman_environ_table_load      (PicmanEnvironTable *environ_table,
                                                  const gchar      *env_path);

void                picman_environ_table_add       (PicmanEnvironTable *environ_table,
                                                  const gchar      *name,
                                                  const gchar      *value,
                                                  const gchar      *separator);
void                picman_environ_table_remove    (PicmanEnvironTable *environ_table,
                                                  const gchar      *name);

void                picman_environ_table_clear     (PicmanEnvironTable *environ_table);
void                picman_environ_table_clear_all (PicmanEnvironTable *environ_table);

gchar            ** picman_environ_table_get_envp  (PicmanEnvironTable *environ_table);


#endif /* __PICMAN_ENVIRON_TABLE_H__ */
