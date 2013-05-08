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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-colormap.h"

#include "widgets/picmancolordialog.h"
#include "widgets/picmancolormapeditor.h"
#include "widgets/picmandialogfactory.h"

#include "actions.h"
#include "colormap-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   colormap_edit_color_update (PicmanColorDialog      *dialog,
                                          const PicmanRGB        *color,
                                          PicmanColorDialogState  state,
                                          PicmanColormapEditor   *editor);


/*  public functions  */

void
colormap_edit_color_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanColormapEditor *editor;
  PicmanImage          *image;
  const guchar       *colormap;
  PicmanRGB             color;
  gchar              *desc;
  return_if_no_image (image, data);

  editor = PICMAN_COLORMAP_EDITOR (data);

  colormap = picman_image_get_colormap (image);

  picman_rgba_set_uchar (&color,
                       colormap[editor->col_index * 3],
                       colormap[editor->col_index * 3 + 1],
                       colormap[editor->col_index * 3 + 2],
                       255);

  desc = g_strdup_printf (_("Edit colormap entry #%d"), editor->col_index);

  if (! editor->color_dialog)
    {
      editor->color_dialog =
        picman_color_dialog_new (PICMAN_VIEWABLE (image),
                               action_data_get_context (data),
                               _("Edit Colormap Entry"),
                               PICMAN_STOCK_COLORMAP,
                               desc,
                               GTK_WIDGET (editor),
                               picman_dialog_factory_get_singleton (),
                               "picman-colormap-editor-color-dialog",
                               (const PicmanRGB *) &color,
                               FALSE, FALSE);

      g_signal_connect (editor->color_dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &editor->color_dialog);

      g_signal_connect (editor->color_dialog, "update",
                        G_CALLBACK (colormap_edit_color_update),
                        editor);
    }
  else
    {
      picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (editor->color_dialog),
                                         PICMAN_VIEWABLE (image),
                                         action_data_get_context (data));
      g_object_set (editor->color_dialog, "description", desc, NULL);
      picman_color_dialog_set_color (PICMAN_COLOR_DIALOG (editor->color_dialog),
                                   &color);
    }

  g_free (desc);

  gtk_window_present (GTK_WINDOW (editor->color_dialog));
}

void
colormap_add_color_cmd_callback (GtkAction *action,
                                 gint       value,
                                 gpointer   data)
{
  PicmanContext *context;
  PicmanImage   *image;
  return_if_no_context (context, data);
  return_if_no_image (image, data);

  if (picman_image_get_colormap_size (image) < 256)
    {
      PicmanRGB color;

      if (value)
        picman_context_get_background (context, &color);
      else
        picman_context_get_foreground (context, &color);

      picman_image_add_colormap_entry (image, &color);
      picman_image_flush (image);
    }
}


/*  private functions  */

static void
colormap_edit_color_update (PicmanColorDialog      *dialog,
                            const PicmanRGB        *color,
                            PicmanColorDialogState  state,
                            PicmanColormapEditor   *editor)
{
  PicmanImage *image = PICMAN_IMAGE_EDITOR (editor)->image;

  switch (state)
    {
    case PICMAN_COLOR_DIALOG_UPDATE:
      break;

    case PICMAN_COLOR_DIALOG_OK:
      picman_image_set_colormap_entry (image, editor->col_index, color, TRUE);
      picman_image_flush (image);
      /* Fall through */

    case PICMAN_COLOR_DIALOG_CANCEL:
      gtk_widget_hide (editor->color_dialog);
      break;
    }
}
