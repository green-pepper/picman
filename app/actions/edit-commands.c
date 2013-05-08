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
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picman-edit.h"
#include "core/picmanbuffer.h"
#include "core/picmancontainer.h"
#include "core/picmandrawable.h"
#include "core/picmanlayer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-new.h"
#include "core/picmanimage-undo.h"

#include "vectors/picmanvectors-import.h"

#include "widgets/picmanclipboard.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmanwindowstrategy.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-transform.h"

#include "dialogs/fade-dialog.h"

#include "actions.h"
#include "edit-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   edit_paste                         (PicmanDisplay *display,
                                                  gboolean     paste_into);
static void   cut_named_buffer_callback          (GtkWidget   *widget,
                                                  const gchar *name,
                                                  gpointer     data);
static void   copy_named_buffer_callback         (GtkWidget   *widget,
                                                  const gchar *name,
                                                  gpointer     data);
static void   copy_named_visible_buffer_callback (GtkWidget   *widget,
                                                  const gchar *name,
                                                  gpointer     data);


/*  public functions  */

void
edit_undo_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  if (picman_image_undo (image))
    picman_image_flush (image);
}

void
edit_redo_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  if (picman_image_redo (image))
    picman_image_flush (image);
}

void
edit_strong_undo_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  if (picman_image_strong_undo (image))
    picman_image_flush (image);
}

void
edit_strong_redo_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage *image;
  return_if_no_image (image, data);

  if (picman_image_strong_redo (image))
    picman_image_flush (image);
}

void
edit_undo_clear_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage     *image;
  PicmanUndoStack *undo_stack;
  PicmanUndoStack *redo_stack;
  GtkWidget     *widget;
  GtkWidget     *dialog;
  gchar         *size;
  gint64         memsize;
  gint64         guisize;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = picman_message_dialog_new (_("Clear Undo History"), PICMAN_STOCK_WARNING,
                                    widget,
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func,
                                    PICMAN_HELP_EDIT_UNDO_CLEAR,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_CLEAR,  GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect_object (gtk_widget_get_toplevel (widget), "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  g_signal_connect_object (image, "disconnect",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Really clear image's undo history?"));

  undo_stack = picman_image_get_undo_stack (image);
  redo_stack = picman_image_get_redo_stack (image);

  memsize =  picman_object_get_memsize (PICMAN_OBJECT (undo_stack), &guisize);
  memsize += guisize;
  memsize += picman_object_get_memsize (PICMAN_OBJECT (redo_stack), &guisize);
  memsize += guisize;

  size = g_format_size (memsize);

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("Clearing the undo history of this "
                               "image will gain %s of memory."), size);
  g_free (size);

  if (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      picman_image_undo_disable (image);
      picman_image_undo_enable (image);
      picman_image_flush (image);
    }

  gtk_widget_destroy (dialog);
}

void
edit_cut_cmd_callback (GtkAction *action,
                       gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  GError       *error = NULL;
  return_if_no_drawable (image, drawable, data);

  if (picman_edit_cut (image, drawable, action_data_get_context (data), &error))
    {
      PicmanDisplay *display = action_data_get_display (data);

      if (display)
        picman_message_literal (image->picman,
			      G_OBJECT (display), PICMAN_MESSAGE_INFO,
			      _("Cut pixels to the clipboard"));

      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman,
			    G_OBJECT (action_data_get_display (data)),
			    PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

void
edit_copy_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  GError       *error = NULL;
  return_if_no_drawable (image, drawable, data);

  if (picman_edit_copy (image, drawable, action_data_get_context (data), &error))
    {
      PicmanDisplay *display = action_data_get_display (data);

      if (display)
        picman_message_literal (image->picman,
			      G_OBJECT (display), PICMAN_MESSAGE_INFO,
			      _("Copied pixels to the clipboard"));

      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman,
			    G_OBJECT (action_data_get_display (data)),
			    PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

void
edit_copy_visible_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage *image;
  GError    *error = NULL;
  return_if_no_image (image, data);

  if (picman_edit_copy_visible (image, action_data_get_context (data), &error))
    {
      PicmanDisplay *display = action_data_get_display (data);

      if (display)
        picman_message_literal (image->picman,
			      G_OBJECT (display), PICMAN_MESSAGE_INFO,
			      _("Copied pixels to the clipboard"));

      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman,
			    G_OBJECT (action_data_get_display (data)),
			    PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

void
edit_paste_cmd_callback (GtkAction *action,
                         gpointer   data)
{
  PicmanDisplay *display = action_data_get_display (data);

  if (display && picman_display_get_image (display))
    edit_paste (display, FALSE);
  else
    edit_paste_as_new_cmd_callback (action, data);
}

void
edit_paste_into_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanDisplay *display;
  return_if_no_display (display, data);

  edit_paste (display, TRUE);
}

void
edit_paste_as_new_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  Picman       *picman;
  PicmanBuffer *buffer;
  return_if_no_picman (picman, data);

  buffer = picman_clipboard_get_buffer (picman);

  if (buffer)
    {
      PicmanImage *image;

      image = picman_image_new_from_buffer (picman, action_data_get_image (data),
                                          buffer);
      g_object_unref (buffer);

      picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0);
      g_object_unref (image);
    }
  else
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_WARNING,
			    _("There is no image data in the clipboard to paste."));
    }
}

void
edit_paste_as_new_layer_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  Picman       *picman;
  PicmanImage  *image;
  PicmanBuffer *buffer;
  return_if_no_picman (picman, data);
  return_if_no_image (image, data);

  buffer = picman_clipboard_get_buffer (picman);

  if (buffer)
    {
      PicmanLayer *layer;

      layer = picman_layer_new_from_buffer (picman_buffer_get_buffer (buffer),
                                          image,
                                          picman_image_get_layer_format (image,
                                                                       TRUE),
                                          _("Clipboard"),
                                          PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);
      g_object_unref (buffer);

      picman_image_add_layer (image, layer,
                            PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_WARNING,
			    _("There is no image data in the clipboard to paste."));
    }
}

void
edit_named_cut_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  GtkWidget *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = picman_query_string_box (_("Cut Named"), widget,
                                  picman_standard_help_func,
                                  PICMAN_HELP_BUFFER_CUT,
                                  _("Enter a name for this buffer"),
                                  NULL,
                                  G_OBJECT (image), "disconnect",
                                  cut_named_buffer_callback, image);
  gtk_widget_show (dialog);
}

void
edit_fade_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  GtkWidget *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = fade_dialog_new (image, widget);

  if (dialog)
    {
      g_signal_connect_object (image, "disconnect",
                               G_CALLBACK (gtk_widget_destroy),
                               dialog, G_CONNECT_SWAPPED);
      gtk_widget_show (dialog);
    }
}

void
edit_named_copy_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  GtkWidget *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = picman_query_string_box (_("Copy Named"), widget,
                                  picman_standard_help_func,
                                  PICMAN_HELP_BUFFER_COPY,
                                  _("Enter a name for this buffer"),
                                  NULL,
                                  G_OBJECT (image), "disconnect",
                                  copy_named_buffer_callback, image);
  gtk_widget_show (dialog);
}

void
edit_named_copy_visible_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  GtkWidget *dialog;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = picman_query_string_box (_("Copy Visible Named "), widget,
                                  picman_standard_help_func,
                                  PICMAN_HELP_BUFFER_COPY,
                                  _("Enter a name for this buffer"),
                                  NULL,
                                  G_OBJECT (image), "disconnect",
                                  copy_named_visible_buffer_callback, image);
  gtk_widget_show (dialog);
}

void
edit_named_paste_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  Picman      *picman;
  GtkWidget *widget;
  return_if_no_picman (picman, data);
  return_if_no_widget (widget, data);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (picman)),
                                             picman,
                                             picman_dialog_factory_get_singleton (),
                                             gtk_widget_get_screen (widget),
                                             "picman-buffer-list|picman-buffer-grid");
}

void
edit_clear_cmd_callback (GtkAction *action,
                         gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  return_if_no_drawable (image, drawable, data);

  picman_edit_clear (image, drawable, action_data_get_context (data));
  picman_image_flush (image);
}

void
edit_fill_cmd_callback (GtkAction *action,
                        gint       value,
                        gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanFillType  fill_type;
  return_if_no_drawable (image, drawable, data);

  fill_type = (PicmanFillType) value;

  picman_edit_fill (image, drawable, action_data_get_context (data),
                  fill_type, PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);
  picman_image_flush (image);
}


/*  private functions  */

static void
edit_paste (PicmanDisplay *display,
            gboolean     paste_into)
{
  PicmanImage *image = picman_display_get_image (display);
  gchar     *svg;
  gsize      svg_size;

  svg = picman_clipboard_get_svg (display->picman, &svg_size);

  if (svg)
    {
      if (picman_vectors_import_buffer (image, svg, svg_size,
                                      TRUE, FALSE,
                                      PICMAN_IMAGE_ACTIVE_PARENT, -1,
                                      NULL, NULL))
        {
          picman_image_flush (image);
        }

      g_free (svg);
    }
  else
    {
      PicmanBuffer *buffer;

      buffer = picman_clipboard_get_buffer (display->picman);

      if (buffer)
        {
          PicmanDisplayShell *shell = picman_display_get_shell (display);
          gint              x, y;
          gint              width, height;

          picman_display_shell_untransform_viewport (shell,
                                                   &x, &y, &width, &height);

          if (picman_edit_paste (image,
                               picman_image_get_active_drawable (image),
                               buffer, paste_into, x, y, width, height))
            {
              picman_image_flush (image);
            }

          g_object_unref (buffer);
        }
      else
        {
          picman_message_literal (display->picman, G_OBJECT (display),
				PICMAN_MESSAGE_WARNING,
				_("There is no image data in the clipboard to paste."));
        }
    }
}

static void
cut_named_buffer_callback (GtkWidget   *widget,
                           const gchar *name,
                           gpointer     data)
{
  PicmanImage    *image    = PICMAN_IMAGE (data);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);
  GError       *error    = NULL;

  if (! drawable)
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to cut from."));
      return;
    }

  if (! (name && strlen (name)))
    name = _("(Unnamed Buffer)");

  if (picman_edit_named_cut (image, name, drawable,
                           picman_get_user_context (image->picman), &error))
    {
      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

static void
copy_named_buffer_callback (GtkWidget   *widget,
                            const gchar *name,
                            gpointer     data)
{
  PicmanImage    *image    = PICMAN_IMAGE (data);
  PicmanDrawable *drawable = picman_image_get_active_drawable (image);
  GError       *error    = NULL;

  if (! drawable)
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to copy from."));
      return;
    }

  if (! (name && strlen (name)))
    name = _("(Unnamed Buffer)");

  if (picman_edit_named_copy (image, name, drawable,
                            picman_get_user_context (image->picman), &error))
    {
      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}

static void
copy_named_visible_buffer_callback (GtkWidget   *widget,
                                    const gchar *name,
                                    gpointer     data)
{
  PicmanImage *image = PICMAN_IMAGE (data);
  GError    *error = NULL;

  if (! (name && strlen (name)))
    name = _("(Unnamed Buffer)");

  if (picman_edit_named_copy_visible (image, name,
                                    picman_get_user_context (image->picman),
                                    &error))
    {
      picman_image_flush (image);
    }
  else
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
    }
}
