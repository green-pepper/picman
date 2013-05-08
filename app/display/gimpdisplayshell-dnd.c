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

#include "libpicmanbase/picmanbase.h"

#include "display-types.h"

#include "core/picman.h"
#include "core/picman-edit.h"
#include "core/picmanbuffer.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-merge.h"
#include "core/picmanimage-new.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmanpattern.h"
#include "core/picmanprogress.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "text/picmantext.h"
#include "text/picmantextlayer.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanvectors-import.h"

#include "widgets/picmandnd.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-dnd.h"
#include "picmandisplayshell-transform.h"

#include "picman-log.h"
#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_display_shell_drop_drawable  (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 PicmanViewable    *viewable,
                                                 gpointer         data);
static void   picman_display_shell_drop_vectors   (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 PicmanViewable    *viewable,
                                                 gpointer         data);
static void   picman_display_shell_drop_svg       (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 const guchar    *svg_data,
                                                 gsize            svg_data_length,
                                                 gpointer         data);
static void   picman_display_shell_drop_pattern   (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 PicmanViewable    *viewable,
                                                 gpointer         data);
static void   picman_display_shell_drop_color     (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 const PicmanRGB   *color,
                                                 gpointer         data);
static void   picman_display_shell_drop_buffer    (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 PicmanViewable    *viewable,
                                                 gpointer         data);
static void   picman_display_shell_drop_uri_list  (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 GList           *uri_list,
                                                 gpointer         data);
static void   picman_display_shell_drop_component (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 PicmanImage       *image,
                                                 PicmanChannelType  component,
                                                 gpointer         data);
static void   picman_display_shell_drop_pixbuf    (GtkWidget       *widget,
                                                 gint             x,
                                                 gint             y,
                                                 GdkPixbuf       *pixbuf,
                                                 gpointer         data);


/*  public functions  */

void
picman_display_shell_dnd_init (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_LAYER,
                               picman_display_shell_drop_drawable,
                               shell);
  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_LAYER_MASK,
                               picman_display_shell_drop_drawable,
                               shell);
  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_CHANNEL,
                               picman_display_shell_drop_drawable,
                               shell);
  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_VECTORS,
                               picman_display_shell_drop_vectors,
                               shell);
  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_PATTERN,
                               picman_display_shell_drop_pattern,
                               shell);
  picman_dnd_viewable_dest_add  (shell->canvas, PICMAN_TYPE_BUFFER,
                               picman_display_shell_drop_buffer,
                               shell);
  picman_dnd_color_dest_add     (shell->canvas,
                               picman_display_shell_drop_color,
                               shell);
  picman_dnd_component_dest_add (shell->canvas,
                               picman_display_shell_drop_component,
                               shell);
  picman_dnd_uri_list_dest_add  (shell->canvas,
                               picman_display_shell_drop_uri_list,
                               shell);
  picman_dnd_svg_dest_add       (shell->canvas,
                               picman_display_shell_drop_svg,
                               shell);
  picman_dnd_pixbuf_dest_add    (shell->canvas,
                               picman_display_shell_drop_pixbuf,
                               shell);
}


/*  private functions  */

/*
 * Position the dropped item in the middle of the viewport.
 */
static void
picman_display_shell_dnd_position_item (PicmanDisplayShell *shell,
                                      PicmanImage        *image,
                                      PicmanItem         *item)
{
  gint item_width  = picman_item_get_width  (item);
  gint item_height = picman_item_get_height (item);
  gint off_x, off_y;

  if (item_width  >= picman_image_get_width  (image) &&
      item_height >= picman_image_get_height (image))
    {
      off_x = (picman_image_get_width  (image) - item_width)  / 2;
      off_y = (picman_image_get_height (image) - item_height) / 2;
    }
  else
    {
      gint x, y;
      gint width, height;

      picman_display_shell_untransform_viewport (shell, &x, &y, &width, &height);

      off_x = x + (width  - item_width)  / 2;
      off_y = y + (height - item_height) / 2;
    }

  picman_item_translate (item,
                       off_x - picman_item_get_offset_x (item),
                       off_y - picman_item_get_offset_y (item),
                       FALSE);
}

static void
picman_display_shell_dnd_flush (PicmanDisplayShell *shell,
                              PicmanImage        *image)
{
  picman_display_shell_present (shell);

  picman_image_flush (image);

  picman_context_set_display (picman_get_user_context (shell->display->picman),
                            shell->display);
}

static void
picman_display_shell_drop_drawable (GtkWidget    *widget,
                                  gint          x,
                                  gint          y,
                                  PicmanViewable *viewable,
                                  gpointer      data)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image     = picman_display_get_image (shell->display);
  GType             new_type;
  PicmanItem         *new_item;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! image)
    {
      image = picman_image_new_from_drawable (shell->display->picman,
                                            PICMAN_DRAWABLE (viewable));
      picman_create_display (shell->display->picman, image, PICMAN_UNIT_PIXEL, 1.0);
      g_object_unref (image);

      return;
    }

  if (PICMAN_IS_LAYER (viewable))
    new_type = G_TYPE_FROM_INSTANCE (viewable);
  else
    new_type = PICMAN_TYPE_LAYER;

  new_item = picman_item_convert (PICMAN_ITEM (viewable), image, new_type);

  if (new_item)
    {
      PicmanLayer *new_layer = PICMAN_LAYER (new_item);

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                                   _("Drop New Layer"));

      picman_display_shell_dnd_position_item (shell, image, new_item);

      picman_item_set_visible (new_item, TRUE, FALSE);
      picman_item_set_linked (new_item, FALSE, FALSE);

      picman_image_add_layer (image, new_layer,
                            PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_undo_group_end (image);

      picman_display_shell_dnd_flush (shell, image);
    }
}

static void
picman_display_shell_drop_vectors (GtkWidget    *widget,
                                 gint          x,
                                 gint          y,
                                 PicmanViewable *viewable,
                                 gpointer      data)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image = picman_display_get_image (shell->display);
  PicmanItem         *new_item;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! image)
    return;

  new_item = picman_item_convert (PICMAN_ITEM (viewable),
                                image, G_TYPE_FROM_INSTANCE (viewable));

  if (new_item)
    {
      PicmanVectors *new_vectors = PICMAN_VECTORS (new_item);

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                                   _("Drop New Path"));

      picman_image_add_vectors (image, new_vectors,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_undo_group_end (image);

      picman_display_shell_dnd_flush (shell, image);
    }
}

static void
picman_display_shell_drop_svg (GtkWidget     *widget,
                             gint           x,
                             gint           y,
                             const guchar  *svg_data,
                             gsize          svg_data_len,
                             gpointer       data)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image = picman_display_get_image (shell->display);
  GError           *error  = NULL;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! image)
    return;

  if (! picman_vectors_import_buffer (image,
                                    (const gchar *) svg_data, svg_data_len,
                                    TRUE, FALSE,
                                    PICMAN_IMAGE_ACTIVE_PARENT, -1,
                                    NULL, &error))
    {
      picman_message_literal (shell->display->picman, G_OBJECT (shell->display),
			    PICMAN_MESSAGE_ERROR,
			    error->message);
      g_clear_error (&error);
    }
  else
    {
      picman_display_shell_dnd_flush (shell, image);
    }
}

static void
picman_display_shell_dnd_bucket_fill (PicmanDisplayShell   *shell,
                                    PicmanBucketFillMode  fill_mode,
                                    const PicmanRGB      *color,
                                    PicmanPattern        *pattern)
{
  PicmanImage    *image = picman_display_get_image (shell->display);
  PicmanDrawable *drawable;

  if (shell->display->picman->busy)
    return;

  if (! image)
    return;

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    return;

  if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
    {
      picman_message_literal (shell->display->picman, G_OBJECT (shell->display),
                            PICMAN_MESSAGE_ERROR,
                            _("Cannot modify the pixels of layer groups."));
      return;
    }

  if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      picman_message_literal (shell->display->picman, G_OBJECT (shell->display),
                            PICMAN_MESSAGE_ERROR,
                            _("The active layer's pixels are locked."));
      return;
    }

  /* FIXME: there should be a virtual method for this that the
   *        PicmanTextLayer can override.
   */
  if (color && picman_item_is_text_layer (PICMAN_ITEM (drawable)))
    {
      picman_text_layer_set (PICMAN_TEXT_LAYER (drawable), NULL,
                           "color", color,
                           NULL);
    }
  else
    {
      picman_edit_fill_full (image, drawable,
                           color, pattern,
                           PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE,
                           pattern ?
                           C_("undo-type", "Drop pattern to layer") :
                           C_("undo-type", "Drop color to layer"));
    }

  picman_display_shell_dnd_flush (shell, image);
}

static void
picman_display_shell_drop_pattern (GtkWidget    *widget,
                                 gint          x,
                                 gint          y,
                                 PicmanViewable *viewable,
                                 gpointer      data)
{
  PICMAN_LOG (DND, NULL);

  if (PICMAN_IS_PATTERN (viewable))
    picman_display_shell_dnd_bucket_fill (PICMAN_DISPLAY_SHELL (data),
                                        PICMAN_PATTERN_BUCKET_FILL,
                                        NULL, PICMAN_PATTERN (viewable));
}

static void
picman_display_shell_drop_color (GtkWidget     *widget,
                               gint           x,
                               gint           y,
                               const PicmanRGB *color,
                               gpointer       data)
{
  PICMAN_LOG (DND, NULL);

  picman_display_shell_dnd_bucket_fill (PICMAN_DISPLAY_SHELL (data),
                                      PICMAN_FG_BUCKET_FILL,
                                      color, NULL);
}

static void
picman_display_shell_drop_buffer (GtkWidget    *widget,
                                gint          drop_x,
                                gint          drop_y,
                                PicmanViewable *viewable,
                                gpointer      data)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image = picman_display_get_image (shell->display);
  PicmanDrawable     *drawable;
  PicmanBuffer       *buffer;
  gint              x, y, width, height;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! image)
    {
      image = picman_image_new_from_buffer (shell->display->picman, NULL,
                                          PICMAN_BUFFER (viewable));
      picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0);
      g_object_unref (image);

      return;
    }

  drawable = picman_image_get_active_drawable (image);

  if (drawable)
    {
      if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
        {
          picman_message_literal (shell->display->picman, G_OBJECT (shell->display),
                                PICMAN_MESSAGE_ERROR,
                                _("Cannot modify the pixels of layer groups."));
          return;
        }

      if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
        {
          picman_message_literal (shell->display->picman, G_OBJECT (shell->display),
                                PICMAN_MESSAGE_ERROR,
                                _("The active layer's pixels are locked."));
          return;
        }
    }

  buffer = PICMAN_BUFFER (viewable);

  picman_display_shell_untransform_viewport (shell, &x, &y, &width, &height);

  /* FIXME: popup a menu for selecting "Paste Into" */

  picman_edit_paste (image, drawable, buffer, FALSE,
                   x, y, width, height);

  picman_display_shell_dnd_flush (shell, image);
}

static void
picman_display_shell_drop_uri_list (GtkWidget *widget,
                                  gint       x,
                                  gint       y,
                                  GList     *uri_list,
                                  gpointer   data)
{
  PicmanDisplayShell *shell   = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image;
  PicmanContext      *context;
  GList            *list;
  gboolean          open_as_layers;

  /* If the app is already being torn down, shell->display might be NULL here.
   * Play it safe. */
  if (! shell->display)
    {
      return;
    }

  image = picman_display_get_image (shell->display);
  context = picman_get_user_context (shell->display->picman);

  PICMAN_LOG (DND, NULL);

  open_as_layers = (image != NULL);

  for (list = uri_list; list; list = g_list_next (list))
    {
      const gchar       *uri   = list->data;
      PicmanPDBStatusType  status;
      GError            *error = NULL;
      gboolean           warn  = FALSE;

      if (! shell->display)
        {
          /* It seems as if PICMAN is being torn down for quitting. Bail out. */
          return;
        }

      if (open_as_layers)
        {
          GList *new_layers;

          new_layers = file_open_layers (shell->display->picman, context,
                                         PICMAN_PROGRESS (shell->display),
                                         image, FALSE,
                                         uri, PICMAN_RUN_INTERACTIVE, NULL,
                                         &status, &error);

          if (new_layers)
            {
              gint x, y;
              gint width, height;

              picman_display_shell_untransform_viewport (shell, &x, &y,
                                                       &width, &height);

              picman_image_add_layers (image, new_layers,
                                     PICMAN_IMAGE_ACTIVE_PARENT, -1,
                                     x, y, width, height,
                                     _("Drop layers"));

              g_list_free (new_layers);
            }
          else if (status != PICMAN_PDB_CANCEL)
            {
              warn = TRUE;
            }
        }
      else if (picman_display_get_image (shell->display))
        {
          /*  open any subsequent images in a new display  */
          PicmanImage *new_image;

          new_image = file_open_with_display (shell->display->picman, context,
                                              NULL,
                                              uri, FALSE,
                                              &status, &error);

          if (! new_image && status != PICMAN_PDB_CANCEL)
            warn = TRUE;
        }
      else
        {
          /*  open the first image in the empty display  */
          image = file_open_with_display (shell->display->picman, context,
                                          PICMAN_PROGRESS (shell->display),
                                          uri, FALSE,
                                          &status, &error);

          if (! image && status != PICMAN_PDB_CANCEL)
            warn = TRUE;
        }

      /* Something above might have run a few rounds of the main loop. Check
       * that shell->display is still there, otherwise ignore this as the app
       * is being torn down for quitting. */
      if (warn && shell->display)
        {
          gchar *filename = file_utils_uri_display_name (uri);

          picman_message (shell->display->picman, G_OBJECT (shell->display),
                        PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed:\n\n%s"),
                        filename, error->message);

          g_clear_error (&error);
          g_free (filename);
        }
    }

  if (image)
    picman_display_shell_dnd_flush (shell, image);
}

static void
picman_display_shell_drop_component (GtkWidget       *widget,
                                   gint             x,
                                   gint             y,
                                   PicmanImage       *image,
                                   PicmanChannelType  component,
                                   gpointer         data)
{
  PicmanDisplayShell *shell      = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *dest_image = picman_display_get_image (shell->display);
  PicmanChannel      *channel;
  PicmanItem         *new_item;
  const gchar      *desc;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! dest_image)
    {
      dest_image = picman_image_new_from_component (image->picman,
                                                  image, component);
      picman_create_display (dest_image->picman, dest_image, PICMAN_UNIT_PIXEL, 1.0);
      g_object_unref (dest_image);

      return;
    }

  channel = picman_channel_new_from_component (image, component, NULL, NULL);

  new_item = picman_item_convert (PICMAN_ITEM (channel),
                                dest_image, PICMAN_TYPE_LAYER);
  g_object_unref (channel);

  if (new_item)
    {
      PicmanLayer *new_layer = PICMAN_LAYER (new_item);

      picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                           NULL, NULL, &desc, NULL);
      picman_object_take_name (PICMAN_OBJECT (new_layer),
                             g_strdup_printf (_("%s Channel Copy"), desc));

      picman_image_undo_group_start (dest_image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                                   _("Drop New Layer"));

      picman_display_shell_dnd_position_item (shell, image, new_item);

      picman_image_add_layer (dest_image, new_layer,
                            PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_undo_group_end (dest_image);

      picman_display_shell_dnd_flush (shell, dest_image);
    }
}

static void
picman_display_shell_drop_pixbuf (GtkWidget *widget,
                                gint       x,
                                gint       y,
                                GdkPixbuf *pixbuf,
                                gpointer   data)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image = picman_display_get_image (shell->display);
  PicmanLayer        *new_layer;
  gboolean          has_alpha = FALSE;

  PICMAN_LOG (DND, NULL);

  if (shell->display->picman->busy)
    return;

  if (! image)
    {
      image = picman_image_new_from_pixbuf (shell->display->picman, pixbuf,
                                          _("Dropped Buffer"));
      picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0);
      g_object_unref (image);

      return;
    }

  if (gdk_pixbuf_get_n_channels (pixbuf) == 2 ||
      gdk_pixbuf_get_n_channels (pixbuf) == 4)
    {
      has_alpha = TRUE;
    }

  new_layer =
    picman_layer_new_from_pixbuf (pixbuf, image,
                                picman_image_get_layer_format (image, has_alpha),
                                _("Dropped Buffer"),
                                PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  if (new_layer)
    {
      PicmanItem *new_item = PICMAN_ITEM (new_layer);

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                                   _("Drop New Layer"));

      picman_display_shell_dnd_position_item (shell, image, new_item);

      picman_image_add_layer (image, new_layer,
                            PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_undo_group_end (image);

      picman_display_shell_dnd_flush (shell, image);
    }
}
