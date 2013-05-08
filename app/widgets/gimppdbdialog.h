/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpdbdialog.h
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

#ifndef __PICMAN_PDB_DIALOG_H__
#define __PICMAN_PDB_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_PDB_DIALOG            (picman_pdb_dialog_get_type ())
#define PICMAN_PDB_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PDB_DIALOG, PicmanPdbDialog))
#define PICMAN_PDB_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PDB_DIALOG, PicmanPdbDialogClass))
#define PICMAN_IS_PDB_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PDB_DIALOG))
#define PICMAN_IS_PDB_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PDB_DIALOG))
#define PICMAN_PDB_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PDB_DIALOG, PicmanPdbDialogClass))


typedef struct _PicmanPdbDialogClass  PicmanPdbDialogClass;

struct _PicmanPdbDialog
{
  PicmanDialog       parent_instance;

  PicmanPDB         *pdb;

  /*  The context we were created with. This is the context the plug-in
   *  exists in and must be used when calling the plug-in.
   */
  PicmanContext     *caller_context;

  /*  The dialog's private context, serves just as model for the
   *  select widgets and must not be used when calling the plug-in.
   */
  PicmanContext     *context;

  GType            select_type;
  PicmanObject      *initial_object;
  gchar           *callback_name;
  gboolean         callback_busy;

  PicmanMenuFactory *menu_factory;
  GtkWidget       *view;
};

struct _PicmanPdbDialogClass
{
  PicmanDialogClass  parent_class;

  GList           *dialogs;

  PicmanValueArray * (* run_callback) (PicmanPdbDialog  *dialog,
                                     PicmanObject     *object,
                                     gboolean        closing,
                                     GError        **error);
};


GType           picman_pdb_dialog_get_type        (void) G_GNUC_CONST;

void            picman_pdb_dialog_run_callback    (PicmanPdbDialog      *dialog,
                                                 gboolean            closing);

PicmanPdbDialog * picman_pdb_dialog_get_by_callback (PicmanPdbDialogClass *klass,
                                                 const gchar        *callback_name);


G_END_DECLS

#endif /* __PICMAN_PDB_DIALOG_H__ */
