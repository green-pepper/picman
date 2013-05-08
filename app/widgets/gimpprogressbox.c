/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprogressbox.c
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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanprogress.h"

#include "picmanprogressbox.h"

#include "picman-intl.h"


static void     picman_progress_box_progress_iface_init (PicmanProgressInterface *iface);

static void     picman_progress_box_dispose            (GObject      *object);

static PicmanProgress *
                picman_progress_box_progress_start     (PicmanProgress *progress,
                                                      const gchar  *message,
                                                      gboolean      cancelable);
static void     picman_progress_box_progress_end       (PicmanProgress *progress);
static gboolean picman_progress_box_progress_is_active (PicmanProgress *progress);
static void     picman_progress_box_progress_set_text  (PicmanProgress *progress,
                                                      const gchar  *message);
static void     picman_progress_box_progress_set_value (PicmanProgress *progress,
                                                      gdouble       percentage);
static gdouble  picman_progress_box_progress_get_value (PicmanProgress *progress);
static void     picman_progress_box_progress_pulse     (PicmanProgress *progress);


G_DEFINE_TYPE_WITH_CODE (PicmanProgressBox, picman_progress_box, GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_progress_box_progress_iface_init))

#define parent_class picman_progress_box_parent_class


static void
picman_progress_box_class_init (PicmanProgressBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_progress_box_dispose;
}

static void
picman_progress_box_init (PicmanProgressBox *box)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (box),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (box), 6);

  box->progress = gtk_progress_bar_new ();
  gtk_widget_set_size_request (box->progress, 250, 20);
  gtk_box_pack_start (GTK_BOX (box), box->progress, FALSE, FALSE, 0);
  gtk_widget_show (box->progress);

  box->label = gtk_label_new ("");
  gtk_label_set_ellipsize (GTK_LABEL (box->label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_misc_set_alignment (GTK_MISC (box->label), 0.0, 0.5);
  picman_label_set_attributes (GTK_LABEL (box->label),
                             PANGO_ATTR_STYLE, PANGO_STYLE_ITALIC,
                             -1);
  gtk_box_pack_start (GTK_BOX (box), box->label, FALSE, FALSE, 0);
  gtk_widget_show (box->label);
}

static void
picman_progress_box_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_progress_box_progress_start;
  iface->end       = picman_progress_box_progress_end;
  iface->is_active = picman_progress_box_progress_is_active;
  iface->set_text  = picman_progress_box_progress_set_text;
  iface->set_value = picman_progress_box_progress_set_value;
  iface->get_value = picman_progress_box_progress_get_value;
  iface->pulse     = picman_progress_box_progress_pulse;
}

static void
picman_progress_box_dispose (GObject *object)
{
  PicmanProgressBox *box = PICMAN_PROGRESS_BOX (object);

  G_OBJECT_CLASS (parent_class)->dispose (object);

  box->progress = NULL;
}

static PicmanProgress *
picman_progress_box_progress_start (PicmanProgress *progress,
                                  const gchar  *message,
                                  gboolean      cancelable)
{
  PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);

  if (! box->progress)
    return NULL;

  if (! box->active)
    {
      GtkProgressBar *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_label_set_text (GTK_LABEL (box->label), message);
      gtk_progress_bar_set_fraction (bar, 0.0);

      box->active     = TRUE;
      box->cancelable = cancelable;
      box->value      = 0.0;

      if (gtk_widget_is_drawable (box->progress))
        gdk_window_process_updates (gtk_widget_get_window (box->progress),
                                    TRUE);

      return progress;
    }

  return NULL;
}

static void
picman_progress_box_progress_end (PicmanProgress *progress)
{
  if (picman_progress_box_progress_is_active (progress))
    {
      PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);
      GtkProgressBar  *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_label_set_text (GTK_LABEL (box->label), "");
      gtk_progress_bar_set_fraction (bar, 0.0);

      box->active     = FALSE;
      box->cancelable = FALSE;
      box->value      = 0.0;
    }
}

static gboolean
picman_progress_box_progress_is_active (PicmanProgress *progress)
{
  PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);

  return (box->progress && box->active);
}

static void
picman_progress_box_progress_set_text (PicmanProgress *progress,
                                     const gchar  *message)
{
  if (picman_progress_box_progress_is_active (progress))
    {
      PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);

      gtk_label_set_text (GTK_LABEL (box->label), message);

      if (gtk_widget_is_drawable (box->progress))
        gdk_window_process_updates (gtk_widget_get_window (box->progress),
                                    TRUE);
    }
}

static void
picman_progress_box_progress_set_value (PicmanProgress *progress,
                                      gdouble       percentage)
{
  if (picman_progress_box_progress_is_active (progress))
    {
      PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);
      GtkProgressBar  *bar = GTK_PROGRESS_BAR (box->progress);
      GtkAllocation    allocation;

      gtk_widget_get_allocation (GTK_WIDGET (bar), &allocation);

      box->value = percentage;

      /* only update the progress bar if this causes a visible change */
      if (fabs (allocation.width *
                (percentage - gtk_progress_bar_get_fraction (bar))) > 1.0)
        {
          gtk_progress_bar_set_fraction (bar, box->value);

          if (gtk_widget_is_drawable (box->progress))
            gdk_window_process_updates (gtk_widget_get_window (box->progress),
                                        TRUE);
        }
    }
}

static gdouble
picman_progress_box_progress_get_value (PicmanProgress *progress)
{
  if (picman_progress_box_progress_is_active (progress))
    {
      return PICMAN_PROGRESS_BOX (progress)->value;
    }

  return 0.0;
}

static void
picman_progress_box_progress_pulse (PicmanProgress *progress)
{
  if (picman_progress_box_progress_is_active (progress))
    {
      PicmanProgressBox *box = PICMAN_PROGRESS_BOX (progress);
      GtkProgressBar  *bar = GTK_PROGRESS_BAR (box->progress);

      gtk_progress_bar_pulse (bar);

      if (gtk_widget_is_drawable (box->progress))
        gdk_window_process_updates (gtk_widget_get_window (box->progress),
                                    TRUE);
    }
}

GtkWidget *
picman_progress_box_new (void)
{
  return g_object_new (PICMAN_TYPE_PROGRESS_BOX, NULL);
}
