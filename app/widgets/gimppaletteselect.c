/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpaletteselect.c
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
#include "core/picmanpalette.h"
#include "core/picmanparamspecs.h"

#include "pdb/picmanpdb.h"

#include "picmancontainerbox.h"
#include "picmandatafactoryview.h"
#include "picmanpaletteselect.h"


static void             picman_palette_select_constructed  (GObject        *object);

static PicmanValueArray * picman_palette_select_run_callback (PicmanPdbDialog  *dialog,
                                                          PicmanObject     *object,
                                                          gboolean        closing,
                                                          GError        **error);


G_DEFINE_TYPE (PicmanPaletteSelect, picman_palette_select, PICMAN_TYPE_PDB_DIALOG)

#define parent_class picman_palette_select_parent_class


static void
picman_palette_select_class_init (PicmanPaletteSelectClass *klass)
{
  GObjectClass       *object_class = G_OBJECT_CLASS (klass);
  PicmanPdbDialogClass *pdb_class    = PICMAN_PDB_DIALOG_CLASS (klass);

  object_class->constructed = picman_palette_select_constructed;

  pdb_class->run_callback   = picman_palette_select_run_callback;
}

static void
picman_palette_select_init (PicmanPaletteSelect *dialog)
{
}

static void
picman_palette_select_constructed (GObject *object)
{
  PicmanPdbDialog *dialog = PICMAN_PDB_DIALOG (object);
  GtkWidget     *content_area;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  dialog->view =
    picman_data_factory_view_new (PICMAN_VIEW_TYPE_LIST,
                                dialog->context->picman->palette_factory,
                                dialog->context,
                                PICMAN_VIEW_SIZE_MEDIUM, 1,
                                dialog->menu_factory, "<Palettes>",
                                "/palettes-popup",
                                "palettes");

  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (PICMAN_CONTAINER_EDITOR (dialog->view)->view),
                                       5 * (PICMAN_VIEW_SIZE_MEDIUM + 2),
                                       8 * (PICMAN_VIEW_SIZE_MEDIUM + 2));

  gtk_container_set_border_width (GTK_CONTAINER (dialog->view), 12);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_box_pack_start (GTK_BOX (content_area), dialog->view, TRUE, TRUE, 0);
  gtk_widget_show (dialog->view);
}

static PicmanValueArray *
picman_palette_select_run_callback (PicmanPdbDialog  *dialog,
                                  PicmanObject     *object,
                                  gboolean        closing,
                                  GError        **error)
{
  PicmanPalette *palette = PICMAN_PALETTE (object);

  return picman_pdb_execute_procedure_by_name (dialog->pdb,
                                             dialog->caller_context,
                                             NULL, error,
                                             dialog->callback_name,
                                             G_TYPE_STRING,   picman_object_get_name (object),
                                             PICMAN_TYPE_INT32, picman_palette_get_n_colors (palette),
                                             PICMAN_TYPE_INT32, closing,
                                             G_TYPE_NONE);
}
