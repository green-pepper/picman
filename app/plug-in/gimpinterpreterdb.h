/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaninterpreterdb.h
 * (C) 2005 Manish Singh <yosh@picman.org>
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

#ifndef __PICMAN_INTERPRETER_DB_H__
#define __PICMAN_INTERPRETER_DB_H__


#define PICMAN_TYPE_INTERPRETER_DB            (picman_interpreter_db_get_type ())
#define PICMAN_INTERPRETER_DB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INTERPRETER_DB, PicmanInterpreterDB))
#define PICMAN_INTERPRETER_DB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INTERPRETER_DB, PicmanInterpreterDBClass))
#define PICMAN_IS_INTERPRETER_DB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INTERPRETER_DB))
#define PICMAN_IS_INTERPRETER_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INTERPRETER_DB))
#define PICMAN_INTERPRETER_DB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INTERPRETER_DB, PicmanInterpreterDBClass))


typedef struct _PicmanInterpreterDBClass PicmanInterpreterDBClass;

struct _PicmanInterpreterDB
{
  GObject     parent_instance;

  GHashTable *programs;

  GSList     *magics;
  GHashTable *magic_names;

  GHashTable *extensions;
  GHashTable *extension_names;
};

struct _PicmanInterpreterDBClass
{
  GObjectClass  parent_class;
};


GType               picman_interpreter_db_get_type (void) G_GNUC_CONST;
PicmanInterpreterDB * picman_interpreter_db_new      (void);

void                picman_interpreter_db_load     (PicmanInterpreterDB  *db,
                                                  const gchar        *interp_path);

void                picman_interpreter_db_clear    (PicmanInterpreterDB  *db);

gchar             * picman_interpreter_db_resolve  (PicmanInterpreterDB  *db,
                                                  const gchar        *program_path,
                                                  gchar             **interp_arg);
gchar       * picman_interpreter_db_get_extensions (PicmanInterpreterDB  *db);


#endif /* __PICMAN_INTERPRETER_DB_H__ */
