/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#if !defined (__PICMAN_MODULE_H_INSIDE__) && !defined (PICMAN_MODULE_COMPILATION)
#error "Only <libpicmanmodule/picmanmodule.h> can be included directly."
#endif

#ifndef __PICMAN_MODULE_DB_H__
#define __PICMAN_MODULE_DB_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_MODULE_DB            (picman_module_db_get_type ())
#define PICMAN_MODULE_DB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MODULE_DB, PicmanModuleDB))
#define PICMAN_MODULE_DB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MODULE_DB, PicmanModuleDBClass))
#define PICMAN_IS_MODULE_DB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MODULE_DB))
#define PICMAN_IS_MODULE_DB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MODULE_DB))
#define PICMAN_MODULE_DB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MODULE_DB, PicmanModuleDBClass))


typedef struct _PicmanModuleDBClass PicmanModuleDBClass;

struct _PicmanModuleDB
{
  GObject   parent_instance;

  /*< private >*/
  GList    *modules;

  gchar    *load_inhibit;
  gboolean  verbose;
};

struct _PicmanModuleDBClass
{
  GObjectClass  parent_class;

  void (* add)             (PicmanModuleDB *db,
                            PicmanModule   *module);
  void (* remove)          (PicmanModuleDB *db,
                            PicmanModule   *module);
  void (* module_modified) (PicmanModuleDB *db,
                            PicmanModule   *module);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


GType          picman_module_db_get_type         (void) G_GNUC_CONST;
PicmanModuleDB * picman_module_db_new              (gboolean      verbose);

void           picman_module_db_set_load_inhibit (PicmanModuleDB *db,
                                                const gchar  *load_inhibit);
const gchar  * picman_module_db_get_load_inhibit (PicmanModuleDB *db);

void           picman_module_db_load             (PicmanModuleDB *db,
                                                const gchar  *module_path);
void           picman_module_db_refresh          (PicmanModuleDB *db,
                                                const gchar  *module_path);


G_END_DECLS

#endif  /* __PICMAN_MODULE_DB_H__ */
