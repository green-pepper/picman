/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpdbprogress.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_PDB_PROGRESS_H__
#define __PICMAN_PDB_PROGRESS_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_PDB_PROGRESS            (picman_pdb_progress_get_type ())
#define PICMAN_PDB_PROGRESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PDB_PROGRESS, PicmanPdbProgress))
#define PICMAN_PDB_PROGRESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PDB_PROGRESS, PicmanPdbProgressClass))
#define PICMAN_IS_PDB_PROGRESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PDB_PROGRESS))
#define PICMAN_IS_PDB_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PDB_PROGRESS))
#define PICMAN_PDB_PROGRESS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PDB_PROGRESS, PicmanPdbProgressClass))


typedef struct _PicmanPdbProgressClass  PicmanPdbProgressClass;

struct _PicmanPdbProgress
{
  GObject      object;

  gboolean     active;
  gdouble      value;

  PicmanPDB     *pdb;
  PicmanContext *context;
  gchar       *callback_name;
  gboolean     callback_busy;
};

struct _PicmanPdbProgressClass
{
  GObjectClass  parent_class;

  GList        *progresses;
};


GType             picman_pdb_progress_get_type        (void) G_GNUC_CONST;

PicmanPdbProgress * picman_pdb_progress_get_by_callback (PicmanPdbProgressClass *klass,
                                                     const gchar          *callback_name);


G_END_DECLS

#endif /* __PICMAN_PDB_PROGRESS_H__ */
