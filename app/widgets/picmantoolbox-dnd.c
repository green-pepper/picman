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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanbuffer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-new.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmantoolinfo.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "picmandnd.h"
#include "picmantoolbox.h"
#include "picmantoolbox-dnd.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_toolbox_drop_uri_list  (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           GList           *uri_list,
                                           gpointer         data);
static void   picman_toolbox_drop_drawable  (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           PicmanViewable    *viewable,
                                           gpointer         data);
static void   picman_toolbox_drop_tool      (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           PicmanViewable    *viewable,
                                           gpointer         data);
static void   picman_toolbox_drop_buffer    (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           PicmanViewable    *viewable,
                                           gpointer         data);
static void   picman_toolbox_drop_component (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           PicmanImage       *image,
                                           PicmanChannelType  component,
                                           gpointer         data);
static void   picman_toolbox_drop_pixbuf    (GtkWidget       *widget,
                                           gint             x,
                                           gint             y,
                                           GdkPixbuf       *pixbuf,
                                           gpointer         data);


/*  public functions  */

void
picman_toolbox_dnd_init (PicmanToolbox *toolbox,
                       GtkWidget   *vbox)
{
  PicmanContext *context = NULL;

  g_return_if_fail (PICMAN_IS_TOOLBOX (toolbox));
  g_return_if_fail (GTK_IS_BOX (vbox));

  context = picman_toolbox_get_context (toolbox);

  /* Before caling any dnd helper functions, setup the drag
   * destination manually since we want to handle all drag events
   * manually, otherwise we would not be able to give the drag handler
   * a chance to handle drag events
   */
  gtk_drag_dest_set (vbox,
                     0, NULL, 0,
                     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  picman_dnd_viewable_dest_add  (vbox,
                               PICMAN_TYPE_LAYER,
                               picman_toolbox_drop_drawable,
                               context);
  picman_dnd_viewable_dest_add  (vbox,
                               PICMAN_TYPE_LAYER_MASK,
                               picman_toolbox_drop_drawable,
                               context);
  picman_dnd_viewable_dest_add  (vbox,
                               PICMAN_TYPE_CHANNEL,
                               picman_toolbox_drop_drawable,
                               context);
  picman_dnd_viewable_dest_add  (vbox,
                               PICMAN_TYPE_TOOL_INFO,
                               picman_toolbox_drop_tool,
                               context);
  picman_dnd_viewable_dest_add  (vbox,
                               PICMAN_TYPE_BUFFER,
                               picman_toolbox_drop_buffer,
                               context);
  picman_dnd_component_dest_add (vbox,
                               picman_toolbox_drop_component,
                               context);
  picman_dnd_uri_list_dest_add  (vbox,
                               picman_toolbox_drop_uri_list,
                               context);
  picman_dnd_pixbuf_dest_add    (vbox,
                               picman_toolbox_drop_pixbuf,
                               context);
}


/*  private functions  */

static void
picman_toolbox_drop_uri_list (GtkWidget *widget,
                            gint       x,
                            gint       y,
                            GList     *uri_list,
                            gpointer   data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);
  GList       *list;

  if (context->picman->busy)
    return;

  for (list = uri_list; list; list = g_list_next (list))
    {
      const gchar       *uri   = list->data;
      PicmanImage         *image;
      PicmanPDBStatusType  status;
      GError            *error = NULL;

      image = file_open_with_display (context->picman, context, NULL,
                                      uri, FALSE, &status, &error);

      if (! image && status != PICMAN_PDB_CANCEL)
        {
          gchar *filename = file_utils_uri_display_name (uri);

          picman_message (context->picman, G_OBJECT (widget), PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed:\n\n%s"),
                        filename, error->message);

          g_clear_error (&error);
          g_free (filename);
        }
    }
}

static void
picman_toolbox_drop_drawable (GtkWidget    *widget,
                            gint          x,
                            gint          y,
                            PicmanViewable *viewable,
                            gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);
  PicmanImage   *new_image;

  if (context->picman->busy)
    return;

  new_image = picman_image_new_from_drawable (context->picman,
                                            PICMAN_DRAWABLE (viewable));
  picman_create_display (context->picman, new_image, PICMAN_UNIT_PIXEL, 1.0);
  g_object_unref (new_image);
}

static void
picman_toolbox_drop_tool (GtkWidget    *widget,
                        gint          x,
                        gint          y,
                        PicmanViewable *viewable,
                        gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  if (context->picman->busy)
    return;

  picman_context_set_tool (context, PICMAN_TOOL_INFO (viewable));
}

static void
picman_toolbox_drop_buffer (GtkWidget    *widget,
                          gint          x,
                          gint          y,
                          PicmanViewable *viewable,
                          gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);
  PicmanImage   *image;

  if (context->picman->busy)
    return;

  image = picman_image_new_from_buffer (context->picman, NULL,
                                      PICMAN_BUFFER (viewable));
  picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0);
  g_object_unref (image);
}

static void
picman_toolbox_drop_component (GtkWidget       *widget,
                             gint             x,
                             gint             y,
                             PicmanImage       *image,
                             PicmanChannelType  component,
                             gpointer         data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);
  PicmanImage   *new_image;

  if (context->picman->busy)
    return;

  new_image = picman_image_new_from_component (context->picman,
                                             image, component);
  picman_create_display (new_image->picman, new_image, PICMAN_UNIT_PIXEL, 1.0);
  g_object_unref (new_image);
}

static void
picman_toolbox_drop_pixbuf (GtkWidget *widget,
                          gint       x,
                          gint       y,
                          GdkPixbuf *pixbuf,
                          gpointer   data)
{
  PicmanContext   *context = PICMAN_CONTEXT (data);
  PicmanImage     *new_image;

  if (context->picman->busy)
    return;

  new_image = picman_image_new_from_pixbuf (context->picman, pixbuf,
                                          _("Dropped Buffer"));
  picman_create_display (new_image->picman, new_image, PICMAN_UNIT_PIXEL, 1.0);
  g_object_unref (new_image);
}
