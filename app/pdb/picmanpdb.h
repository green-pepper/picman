/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_PDB_H__
#define __PICMAN_PDB_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_PDB            (picman_pdb_get_type ())
#define PICMAN_PDB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PDB, PicmanPDB))
#define PICMAN_PDB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PDB, PicmanPDBClass))
#define PICMAN_IS_PDB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PDB))
#define PICMAN_IS_PDB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PDB))
#define PICMAN_PDB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PDB, PicmanPDBClass))


typedef struct _PicmanPDBClass PicmanPDBClass;

struct _PicmanPDB
{
  PicmanObject  parent_instance;

  Picman       *picman;

  GHashTable *procedures;
  GHashTable *compat_proc_names;
};

struct _PicmanPDBClass
{
  PicmanObjectClass parent_class;

  void (* register_procedure)   (PicmanPDB       *pdb,
                                 PicmanProcedure *procedure);
  void (* unregister_procedure) (PicmanPDB       *pdb,
                                 PicmanProcedure *procedure);
};


GType            picman_pdb_get_type                       (void) G_GNUC_CONST;

PicmanPDB        * picman_pdb_new                            (Picman           *picman);

void             picman_pdb_register_procedure             (PicmanPDB        *pdb,
                                                          PicmanProcedure  *procedure);
void             picman_pdb_unregister_procedure           (PicmanPDB        *pdb,
                                                          PicmanProcedure  *procedure);

PicmanProcedure  * picman_pdb_lookup_procedure               (PicmanPDB        *pdb,
                                                          const gchar    *name);

void             picman_pdb_register_compat_proc_name      (PicmanPDB        *pdb,
                                                          const gchar    *old_name,
                                                          const gchar    *new_name);
const gchar    * picman_pdb_lookup_compat_proc_name        (PicmanPDB        *pdb,
                                                          const gchar    *old_name);

PicmanValueArray * picman_pdb_execute_procedure_by_name_args (PicmanPDB        *pdb,
                                                          PicmanContext    *context,
                                                          PicmanProgress   *progress,
                                                          GError        **error,
                                                          const gchar    *name,
                                                          PicmanValueArray *args);
PicmanValueArray * picman_pdb_execute_procedure_by_name      (PicmanPDB        *pdb,
                                                          PicmanContext    *context,
                                                          PicmanProgress   *progress,
                                                          GError        **error,
                                                          const gchar    *name,
                                                          ...);

GList          * picman_pdb_get_deprecated_procedures      (PicmanPDB        *pdb);


#endif  /*  __PICMAN_PDB_H__  */
