/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfileprocview.h
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_FILE_PROC_VIEW_H__
#define __PICMAN_FILE_PROC_VIEW_H__


#define PICMAN_TYPE_FILE_PROC_VIEW            (picman_file_proc_view_get_type ())
#define PICMAN_FILE_PROC_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILE_PROC_VIEW, PicmanFileProcView))
#define PICMAN_FILE_PROC_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILE_PROC_VIEW, PicmanFileProcViewClass))
#define PICMAN_IS_FILE_PROC_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILE_PROC_VIEW))
#define PICMAN_IS_FILE_PROC_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILE_PROC_VIEW))
#define PICMAN_FILE_PROC_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILE_PROC_VIEW, PicmanFileProcViewClass))


typedef struct _PicmanFileProcViewClass PicmanFileProcViewClass;

struct _PicmanFileProcView
{
  GtkTreeView        parent_instance;

  GList             *meta_extensions;
};

struct _PicmanFileProcViewClass
{
  GtkTreeViewClass   parent_class;

  void (* changed) (PicmanFileProcView *view);
};


GType                 picman_file_proc_view_get_type    (void) G_GNUC_CONST;

GtkWidget           * picman_file_proc_view_new         (Picman                 *picman,
                                                       GSList               *procedures,
                                                       const gchar          *automatic,
                                                       const gchar          *automatic_help_id);

PicmanPlugInProcedure * picman_file_proc_view_get_proc    (PicmanFileProcView     *view,
                                                       gchar               **label);
gboolean              picman_file_proc_view_set_proc    (PicmanFileProcView     *view,
                                                       PicmanPlugInProcedure  *proc);

gchar               * picman_file_proc_view_get_help_id (PicmanFileProcView     *view);


#endif  /*  __PICMAN_FILE_PROC_VIEW_H__  */
