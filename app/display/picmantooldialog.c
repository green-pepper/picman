/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantooldialog.c
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#include "display-types.h"

#include "core/picmanobject.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmandialogfactory.h"

#include "picmandisplayshell.h"
#include "picmantooldialog.h"


typedef struct _PicmanToolDialogPrivate PicmanToolDialogPrivate;

struct _PicmanToolDialogPrivate
{
  PicmanDisplayShell *shell;
};

#define GET_PRIVATE(dialog) G_TYPE_INSTANCE_GET_PRIVATE (dialog, \
                                                         PICMAN_TYPE_TOOL_DIALOG, \
                                                         PicmanToolDialogPrivate)


static void   picman_tool_dialog_dispose     (GObject          *object);

static void   picman_tool_dialog_shell_unmap (PicmanDisplayShell *shell,
                                            PicmanToolDialog   *dialog);


G_DEFINE_TYPE (PicmanToolDialog, picman_tool_dialog, PICMAN_TYPE_VIEWABLE_DIALOG)

static void
picman_tool_dialog_class_init (PicmanToolDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_tool_dialog_dispose;

  g_type_class_add_private (klass, sizeof (PicmanToolDialogPrivate));
}

static void
picman_tool_dialog_init (PicmanToolDialog *dialog)
{
}

static void
picman_tool_dialog_dispose (GObject *object)
{
  PicmanToolDialogPrivate *private = GET_PRIVATE (object);

  if (private->shell)
    {
      g_object_remove_weak_pointer (G_OBJECT (private->shell),
                                    (gpointer) &private->shell);
      private->shell = NULL;
    }

  G_OBJECT_CLASS (picman_tool_dialog_parent_class)->dispose (object);
}


/**
 * picman_tool_dialog_new:
 * @tool_info: a #PicmanToolInfo
 * @shell:     the parent display shell this dialog
 * @desc:      a string to use in the dialog header or %NULL to use the help
 *             field from #PicmanToolInfo
 * @...:       a %NULL-terminated valist of button parameters as described in
 *             gtk_dialog_new_with_buttons().
 *
 * This function conveniently creates a #PicmanViewableDialog using the
 * information stored in @tool_info. It also registers the tool with
 * the "toplevel" dialog factory.
 *
 * Return value: a new #PicmanViewableDialog
 **/
GtkWidget *
picman_tool_dialog_new (PicmanToolInfo     *tool_info,
                      PicmanDisplayShell *shell,
                      const gchar      *desc,
                      ...)
{
  GtkWidget   *dialog;
  const gchar *stock_id;
  gchar       *identifier;
  va_list      args;

  g_return_val_if_fail (PICMAN_IS_TOOL_INFO (tool_info), NULL);
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info));

  dialog = g_object_new (PICMAN_TYPE_TOOL_DIALOG,
                         "title",        tool_info->blurb,
                         "role",         picman_object_get_name (tool_info),
                         "help-func",    picman_standard_help_func,
                         "help-id",      tool_info->help_id,
                         "stock-id",     stock_id,
                         "description",  desc ? desc : tool_info->help,
                         NULL);

  picman_tool_dialog_set_shell (PICMAN_TOOL_DIALOG (dialog), shell);

  va_start (args, desc);
  picman_dialog_add_buttons_valist (PICMAN_DIALOG (dialog), args);
  va_end (args);

  identifier = g_strconcat (picman_object_get_name (tool_info), "-dialog", NULL);

  picman_dialog_factory_add_foreign (picman_dialog_factory_get_singleton (),
                                   identifier,
                                   dialog);

  g_free (identifier);

  return dialog;
}

void
picman_tool_dialog_set_shell (PicmanToolDialog   *tool_dialog,
                            PicmanDisplayShell *shell)
{
  PicmanToolDialogPrivate *private = GET_PRIVATE (tool_dialog);

  g_return_if_fail (PICMAN_IS_TOOL_DIALOG (tool_dialog));
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell == private->shell)
    return;

  if (private->shell)
    {
      g_object_remove_weak_pointer (G_OBJECT (private->shell),
                                    (gpointer) &private->shell);
      g_signal_handlers_disconnect_by_func (private->shell,
                                            picman_tool_dialog_shell_unmap,
                                            tool_dialog);
      private->shell = NULL;
    }

  private->shell = shell;

  if (private->shell)
    {
      GtkWidget *toplevel;

      g_signal_connect_object (private->shell, "unmap",
                               G_CALLBACK (picman_tool_dialog_shell_unmap),
                               tool_dialog, 0);

      g_object_add_weak_pointer (G_OBJECT (private->shell),
                                 (gpointer) &private->shell);
      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

      gtk_window_set_transient_for (GTK_WINDOW (tool_dialog),
                                    GTK_WINDOW (toplevel));
    }
}


/*  private functions  */

static void
picman_tool_dialog_shell_unmap (PicmanDisplayShell *shell,
                              PicmanToolDialog   *dialog)
{
  /*  the dialog being mapped while the shell is being unmapped
   *  happens when switching images in PicmanImageWindow
   */
  if (gtk_widget_get_mapped (GTK_WIDGET (dialog)))
    g_signal_emit_by_name (dialog, "close");
}
