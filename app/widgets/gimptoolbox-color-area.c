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

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "picmancolordialog.h"
#include "picmandialogfactory.h"
#include "picmanfgbgeditor.h"
#include "picmantoolbox.h"
#include "picmantoolbox-color-area.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   color_area_color_clicked (PicmanFgBgEditor       *editor,
                                        PicmanActiveColor       active_color,
                                        PicmanContext          *context);
static void   color_area_dialog_update (PicmanColorDialog      *dialog,
                                        const PicmanRGB        *color,
                                        PicmanColorDialogState  state,
                                        PicmanContext          *context);


/*  local variables  */

static GtkWidget       *color_area          = NULL;
static GtkWidget       *color_dialog        = NULL;
static gboolean         color_dialog_active = FALSE;
static PicmanActiveColor  edit_color;
static PicmanRGB          revert_fg;
static PicmanRGB          revert_bg;


/*  public functions  */

GtkWidget *
picman_toolbox_color_area_create (PicmanToolbox *toolbox,
                                gint         width,
                                gint         height)
{
  PicmanContext *context;

  g_return_val_if_fail (PICMAN_IS_TOOLBOX (toolbox), NULL);

  context = picman_toolbox_get_context (toolbox);

  color_area = picman_fg_bg_editor_new (context);
  gtk_widget_set_size_request (color_area, width, height);
  gtk_widget_add_events (color_area,
                         GDK_ENTER_NOTIFY_MASK |
                         GDK_LEAVE_NOTIFY_MASK);

  picman_help_set_help_data
    (color_area, _("Foreground & background colors.\n"
                   "The black and white squares reset colors.\n"
                   "The arrows swap colors.\n"
                   "Click to open the color selection dialog."), NULL);

  g_signal_connect (color_area, "color-clicked",
                    G_CALLBACK (color_area_color_clicked),
                    context);

  return color_area;
}


/*  private functions  */

static void
color_area_dialog_update (PicmanColorDialog      *dialog,
                          const PicmanRGB        *color,
                          PicmanColorDialogState  state,
                          PicmanContext          *context)
{
  switch (state)
    {
    case PICMAN_COLOR_DIALOG_OK:
      gtk_widget_hide (color_dialog);
      color_dialog_active = FALSE;
      /* Fallthrough */

    case PICMAN_COLOR_DIALOG_UPDATE:
      if (edit_color == PICMAN_ACTIVE_COLOR_FOREGROUND)
        picman_context_set_foreground (context, color);
      else
        picman_context_set_background (context, color);
      break;

    case PICMAN_COLOR_DIALOG_CANCEL:
      gtk_widget_hide (color_dialog);
      color_dialog_active = FALSE;
      picman_context_set_foreground (context, &revert_fg);
      picman_context_set_background (context, &revert_bg);
      break;
    }
}

static void
color_area_color_clicked (PicmanFgBgEditor  *editor,
                          PicmanActiveColor  active_color,
                          PicmanContext     *context)
{
  PicmanRGB      color;
  const gchar *title;

  if (! color_dialog_active)
    {
      picman_context_get_foreground (context, &revert_fg);
      picman_context_get_background (context, &revert_bg);
    }

  if (active_color == PICMAN_ACTIVE_COLOR_FOREGROUND)
    {
      picman_context_get_foreground (context, &color);
      title = _("Change Foreground Color");
    }
  else
    {
      picman_context_get_background (context, &color);
      title = _("Change Background Color");
    }

  edit_color = active_color;

  if (! color_dialog)
    {
      color_dialog = picman_color_dialog_new (NULL, context,
                                            NULL, NULL, NULL,
                                            GTK_WIDGET (editor),
                                            picman_dialog_factory_get_singleton (),
                                            "picman-toolbox-color-dialog",
                                            &color,
                                            TRUE, FALSE);

      g_signal_connect (color_dialog, "update",
                        G_CALLBACK (color_area_dialog_update),
                        context);
    }

  gtk_window_set_title (GTK_WINDOW (color_dialog), title);
  picman_color_dialog_set_color (PICMAN_COLOR_DIALOG (color_dialog), &color);

  gtk_window_present (GTK_WINDOW (color_dialog));
  color_dialog_active = TRUE;
}
