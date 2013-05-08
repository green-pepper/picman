/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfontselect.c
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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanparamspecs.h"

#include "text/picmanfont.h"

#include "pdb/picmanpdb.h"

#include "picmancontainerbox.h"
#include "picmanfontselect.h"
#include "picmanfontview.h"


static void             picman_font_select_constructed  (GObject        *object);

static PicmanValueArray * picman_font_select_run_callback (PicmanPdbDialog  *dialog,
                                                       PicmanObject     *object,
                                                       gboolean        closing,
                                                       GError        **error);


G_DEFINE_TYPE (PicmanFontSelect, picman_font_select, PICMAN_TYPE_PDB_DIALOG)

#define parent_class picman_font_select_parent_class


static void
picman_font_select_class_init (PicmanFontSelectClass *klass)
{
  GObjectClass       *object_class = G_OBJECT_CLASS (klass);
  PicmanPdbDialogClass *pdb_class    = PICMAN_PDB_DIALOG_CLASS (klass);

  object_class->constructed = picman_font_select_constructed;

  pdb_class->run_callback   = picman_font_select_run_callback;
}

static void
picman_font_select_init (PicmanFontSelect *select)
{
}

static void
picman_font_select_constructed (GObject *object)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (object);
  GtkWidget     *content_area;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  dialog->view = picman_font_view_new (PICMAN_VIEW_TYPE_LIST,
                                     dialog->context->picman->fonts,
                                     dialog->context,
                                     PICMAN_VIEW_SIZE_MEDIUM, 1,
                                     dialog->menu_factory);

  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (PICMAN_CONTAINER_EDITOR (dialog->view)->view),
                                       6 * (PICMAN_VIEW_SIZE_MEDIUM + 2),
                                       6 * (PICMAN_VIEW_SIZE_MEDIUM + 2));

  gtk_container_set_border_width (GTK_CONTAINER (dialog->view), 12);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_box_pack_start (GTK_BOX (content_area), dialog->view, TRUE, TRUE, 0);
  gtk_widget_show (dialog->view);
}

static PicmanValueArray *
picman_font_select_run_callback (PicmanPdbDialog  *dialog,
                               PicmanObject     *object,
                               gboolean        closing,
                               GError        **error)
{
  return picman_pdb_execute_procedure_by_name (dialog->pdb,
                                             dialog->caller_context,
                                             NULL, error,
                                             dialog->callback_name,
                                             G_TYPE_STRING,   picman_object_get_name (object),
                                             PICMAN_TYPE_INT32, closing,
                                             G_TYPE_NONE);
}
