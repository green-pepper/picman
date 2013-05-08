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

#ifndef __VECTORS_IMPORT_DIALOG_H__
#define __VECTORS_IMPORT_DIALOG_H__


typedef struct _VectorsImportDialog VectorsImportDialog;

struct _VectorsImportDialog
{
  GtkWidget *dialog;

  PicmanImage *image;
  gboolean    merge_vectors;
  gboolean    scale_vectors;
};


VectorsImportDialog * vectors_import_dialog_new (PicmanImage *image,
                                                 GtkWidget *parent,
                                                 gboolean   merge_vectors,
                                                 gboolean   scale_vectors);


#endif /* __VECTORS_IMPORT_DIALOG_H__ */
