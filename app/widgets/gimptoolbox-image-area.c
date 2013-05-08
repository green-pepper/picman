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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "picmandialogfactory.h"
#include "picmandnd.h"
#include "picmanview.h"
#include "picmantoolbox.h"
#include "picmantoolbox-image-area.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


static void
image_preview_clicked (GtkWidget       *widget,
                       GdkModifierType  state,
                       PicmanToolbox     *toolbox)
{
  PicmanContext *context = picman_toolbox_get_context (toolbox);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                             context->picman,
                                             picman_dock_get_dialog_factory (PICMAN_DOCK (toolbox)),
                                             gtk_widget_get_screen (widget),
                                             "picman-image-list|picman-image-grid");
}

static void
image_preview_drop_image (GtkWidget    *widget,
                          gint          x,
                          gint          y,
                          PicmanViewable *viewable,
                          gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  picman_context_set_image (context, PICMAN_IMAGE (viewable));
}

static void
image_preview_set_viewable (PicmanView     *view,
                            PicmanViewable *old_viewable,
                            PicmanViewable *new_viewable)
{
  if (! old_viewable && new_viewable)
    {
      picman_dnd_xds_source_add (GTK_WIDGET (view),
                               (PicmanDndDragViewableFunc) picman_view_get_viewable,
                               NULL);
    }
  else if (old_viewable && ! new_viewable)
    {
      picman_dnd_xds_source_remove (GTK_WIDGET (view));
    }
}

/*  public functions  */

GtkWidget *
picman_toolbox_image_area_create (PicmanToolbox *toolbox,
                                gint         width,
                                gint         height)
{
  PicmanContext *context;
  GtkWidget   *image_view;
  gchar       *tooltip;

  g_return_val_if_fail (PICMAN_IS_TOOLBOX (toolbox), NULL);

  context = picman_toolbox_get_context (toolbox);

  image_view = picman_view_new_full_by_types (context,
                                            PICMAN_TYPE_VIEW, PICMAN_TYPE_IMAGE,
                                            width, height, 0,
                                            FALSE, TRUE, TRUE);

  g_signal_connect (image_view, "set-viewable",
                    G_CALLBACK (image_preview_set_viewable),
                    NULL);

  picman_view_set_viewable (PICMAN_VIEW (image_view),
                          PICMAN_VIEWABLE (picman_context_get_image (context)));

  gtk_widget_show (image_view);

#ifdef GDK_WINDOWING_X11
  tooltip = g_strdup_printf ("%s\n%s",
                             _("The active image.\n"
                               "Click to open the Image Dialog."),
                             _("Drag to an XDS enabled file-manager to "
                               "save the image."));
#else
  tooltip = g_strdup (_("The active image.\n"
                        "Click to open the Image Dialog."));
#endif

  picman_help_set_help_data (image_view, tooltip, NULL);
  g_free (tooltip);

  g_signal_connect_object (context, "image-changed",
                           G_CALLBACK (picman_view_set_viewable),
                           image_view, G_CONNECT_SWAPPED);

  g_signal_connect (image_view, "clicked",
                    G_CALLBACK (image_preview_clicked),
                    toolbox);

  picman_dnd_viewable_dest_add (image_view,
                              PICMAN_TYPE_IMAGE,
                              image_preview_drop_image,
                              context);

  return image_view;
}
