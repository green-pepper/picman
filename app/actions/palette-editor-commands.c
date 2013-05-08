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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmandatafactory.h"
#include "core/picmanpalette.h"

#include "widgets/picmancolordialog.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanpaletteeditor.h"

#include "palette-editor-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void  palette_editor_edit_color_update (PicmanColorDialog      *dialog,
                                               const PicmanRGB        *color,
                                               PicmanColorDialogState  state,
                                               PicmanPaletteEditor    *editor);


/*  public functions  */

void
palette_editor_edit_color_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanPaletteEditor *editor      = PICMAN_PALETTE_EDITOR (data);
  PicmanDataEditor    *data_editor = PICMAN_DATA_EDITOR (data);
  PicmanPalette       *palette;

  if (! (data_editor->data_editable && editor->color))
    return;

  palette = PICMAN_PALETTE (data_editor->data);

  if (! editor->color_dialog)
    {
      editor->color_dialog =
        picman_color_dialog_new (PICMAN_VIEWABLE (palette),
                               data_editor->context,
                               _("Edit Palette Color"),
                               PICMAN_STOCK_PALETTE,
                               _("Edit Color Palette Entry"),
                               GTK_WIDGET (editor),
                               picman_dialog_factory_get_singleton (),
                               "picman-palette-editor-color-dialog",
                               &editor->color->color,
                               FALSE, FALSE);

      g_signal_connect (editor->color_dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &editor->color_dialog);

      g_signal_connect (editor->color_dialog, "update",
                        G_CALLBACK (palette_editor_edit_color_update),
                        editor);
    }
  else
    {
      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (editor->color_dialog),
                                         PICMAN_VIEWABLE (palette),
                                         data_editor->context);
      picman_color_dialog_set_color (PICMAN_COLOR_DIALOG (editor->color_dialog),
                                   &editor->color->color);
    }

  gtk_window_present (GTK_WINDOW (editor->color_dialog));
}

void
palette_editor_new_color_cmd_callback (GtkAction *action,
                                       gint       value,
                                       gpointer   data)
{
  PicmanPaletteEditor *editor      = PICMAN_PALETTE_EDITOR (data);
  PicmanDataEditor    *data_editor = PICMAN_DATA_EDITOR (data);

  if (data_editor->data_editable)
    {
      PicmanPalette *palette = PICMAN_PALETTE (data_editor->data);
      PicmanRGB      color;

      if (value)
        picman_context_get_background (data_editor->context, &color);
      else
        picman_context_get_foreground (data_editor->context, &color);

      editor->color = picman_palette_add_entry (palette, -1, NULL, &color);
    }
}

void
palette_editor_delete_color_cmd_callback (GtkAction *action,
                                          gpointer   data)
{
  PicmanPaletteEditor *editor      = PICMAN_PALETTE_EDITOR (data);
  PicmanDataEditor    *data_editor = PICMAN_DATA_EDITOR (data);

  if (data_editor->data_editable && editor->color)
    {
      PicmanPalette *palette = PICMAN_PALETTE (data_editor->data);

      picman_palette_delete_entry (palette, editor->color);
    }
}

void
palette_editor_zoom_cmd_callback (GtkAction *action,
                                  gint       value,
                                  gpointer   data)
{
  PicmanPaletteEditor *editor = PICMAN_PALETTE_EDITOR (data);

  picman_palette_editor_zoom (editor, (PicmanZoomType) value);
}


/*  private functions  */

static void
palette_editor_edit_color_update (PicmanColorDialog      *dialog,
                                  const PicmanRGB        *color,
                                  PicmanColorDialogState  state,
                                  PicmanPaletteEditor    *editor)
{
  PicmanPalette *palette = PICMAN_PALETTE (PICMAN_DATA_EDITOR (editor)->data);

  switch (state)
    {
    case PICMAN_COLOR_DIALOG_UPDATE:
      break;

    case PICMAN_COLOR_DIALOG_OK:
      if (editor->color)
        {
          editor->color->color = *color;
          picman_data_dirty (PICMAN_DATA (palette));
        }
      /* Fallthrough */

    case PICMAN_COLOR_DIALOG_CANCEL:
      gtk_widget_hide (editor->color_dialog);
      break;
    }
}
