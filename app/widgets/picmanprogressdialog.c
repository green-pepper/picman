/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprogressdialog.c
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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanprogress.h"

#include "picmanprogressbox.h"
#include "picmanprogressdialog.h"

#include "picman-intl.h"


#define PROGRESS_DIALOG_WIDTH  400


static void    picman_progress_dialog_progress_iface_init (PicmanProgressInterface *iface);

static void     picman_progress_dialog_response           (GtkDialog    *dialog,
                                                         gint          response_id);

static PicmanProgress *
                picman_progress_dialog_progress_start     (PicmanProgress *progress,
                                                         const gchar  *message,
                                                         gboolean      cancelable);
static void     picman_progress_dialog_progress_end       (PicmanProgress *progress);
static gboolean picman_progress_dialog_progress_is_active (PicmanProgress *progress);
static void     picman_progress_dialog_progress_set_text  (PicmanProgress *progress,
                                                         const gchar  *message);
static void     picman_progress_dialog_progress_set_value (PicmanProgress *progress,
                                                         gdouble       percentage);
static gdouble  picman_progress_dialog_progress_get_value (PicmanProgress *progress);
static void     picman_progress_dialog_progress_pulse     (PicmanProgress *progress);


G_DEFINE_TYPE_WITH_CODE (PicmanProgressDialog, picman_progress_dialog,
                         PICMAN_TYPE_DIALOG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_progress_dialog_progress_iface_init))

#define parent_class picman_progress_dialog_parent_class


static void
picman_progress_dialog_class_init (PicmanProgressDialogClass *klass)
{
  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

  dialog_class->response = picman_progress_dialog_response;
}

static void
picman_progress_dialog_init (PicmanProgressDialog *dialog)
{
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

  dialog->box = picman_progress_box_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog->box), 12);
  gtk_box_pack_start (GTK_BOX (content_area), dialog->box, TRUE, TRUE, 0);
  gtk_widget_show (dialog->box);

  g_signal_connect (dialog->box, "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &dialog->box);

  gtk_dialog_add_button (GTK_DIALOG (dialog),
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

  gtk_widget_set_size_request (GTK_WIDGET (dialog), PROGRESS_DIALOG_WIDTH, -1);
}

static void
picman_progress_dialog_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_progress_dialog_progress_start;
  iface->end       = picman_progress_dialog_progress_end;
  iface->is_active = picman_progress_dialog_progress_is_active;
  iface->set_text  = picman_progress_dialog_progress_set_text;
  iface->set_value = picman_progress_dialog_progress_set_value;
  iface->get_value = picman_progress_dialog_progress_get_value;
  iface->pulse     = picman_progress_dialog_progress_pulse;
}

static void
picman_progress_dialog_response (GtkDialog *dialog,
                               gint       response_id)
{
  PicmanProgressDialog *progress_dialog = PICMAN_PROGRESS_DIALOG (dialog);

  if (PICMAN_PROGRESS_BOX (progress_dialog->box)->cancelable)
    picman_progress_cancel (PICMAN_PROGRESS (dialog));
}

static PicmanProgress *
picman_progress_dialog_progress_start (PicmanProgress *progress,
                                     const gchar  *message,
                                     gboolean      cancelable)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return NULL;

  if (picman_progress_start (PICMAN_PROGRESS (dialog->box), message, cancelable))
    {
      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_CANCEL, cancelable);

      gtk_window_present (GTK_WINDOW (dialog));

      return progress;
    }

  return NULL;
}

static void
picman_progress_dialog_progress_end (PicmanProgress *progress)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return;

  if (PICMAN_PROGRESS_BOX (dialog->box)->active)
    {
      picman_progress_end (PICMAN_PROGRESS (dialog->box));

      gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_CANCEL, FALSE);

      gtk_widget_hide (GTK_WIDGET (dialog));
    }
}

static gboolean
picman_progress_dialog_progress_is_active (PicmanProgress *progress)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return FALSE;

  return picman_progress_is_active (PICMAN_PROGRESS (dialog->box));
}

static void
picman_progress_dialog_progress_set_text (PicmanProgress *progress,
                                        const gchar  *message)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return;

  picman_progress_set_text (PICMAN_PROGRESS (dialog->box), message);
}

static void
picman_progress_dialog_progress_set_value (PicmanProgress *progress,
                                         gdouble       percentage)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return;

  picman_progress_set_value (PICMAN_PROGRESS (dialog->box), percentage);
}

static gdouble
picman_progress_dialog_progress_get_value (PicmanProgress *progress)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return 0.0;

  return picman_progress_get_value (PICMAN_PROGRESS (dialog->box));
}

static void
picman_progress_dialog_progress_pulse (PicmanProgress *progress)
{
  PicmanProgressDialog *dialog = PICMAN_PROGRESS_DIALOG (progress);

  if (! dialog->box)
    return;

  picman_progress_pulse (PICMAN_PROGRESS (dialog->box));
}

GtkWidget *
picman_progress_dialog_new (void)
{
  return g_object_new (PICMAN_TYPE_PROGRESS_DIALOG,
                       "title",             _("Progress"),
                       "role",              "progress",
                       "skip-taskbar-hint", TRUE,
                       "skip-pager-hint",   TRUE,
                       "resizable",         FALSE,
                       NULL);
}
