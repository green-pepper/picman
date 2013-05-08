/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * The PICMAN Help plug-in
 * Copyright (C) 1999-2008 Sven Neumann <sven@picman.org>
 *                         Michael Natterer <mitch@picman.org>
 *                         Henrik Brix Andersen <brix@picman.org>
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

/*  This code is written so that it can also be compiled standalone.
 *  It shouldn't depend on libpicman.
 */

#include "config.h"

#include <string.h>

#include <gio/gio.h>

#include "picmanhelptypes.h"
#include "picmanhelpprogress.h"
#include "picmanhelpprogress-private.h"


struct _PicmanHelpProgress
{
  PicmanHelpProgressVTable  vtable;
  gpointer                user_data;

  GCancellable           *cancellable;
};


PicmanHelpProgress *
picman_help_progress_new (const PicmanHelpProgressVTable *vtable,
                        gpointer                      user_data)
{
  PicmanHelpProgress *progress;

  g_return_val_if_fail (vtable != NULL, NULL);

  progress = g_slice_new0 (PicmanHelpProgress);

  progress->vtable.start     = vtable->start;
  progress->vtable.end       = vtable->end;
  progress->vtable.set_value = vtable->set_value;

  progress->user_data        = user_data;

  return progress;
}

void
picman_help_progress_free (PicmanHelpProgress *progress)
{
  g_return_if_fail (progress != NULL);

  if (progress->cancellable)
    {
      g_object_unref (progress->cancellable);
      progress->cancellable = NULL;
    }

  g_slice_free (PicmanHelpProgress, progress);
}

void
picman_help_progress_cancel (PicmanHelpProgress *progress)
{
  g_return_if_fail (progress != NULL);

  if (progress->cancellable)
    g_cancellable_cancel (progress->cancellable);
}


void
_picman_help_progress_start (PicmanHelpProgress *progress,
                           GCancellable     *cancellable,
                           const gchar      *format,
                           ...)
{
  gchar   *message;
  va_list  args;

  g_return_if_fail (progress != NULL);

  if (cancellable)
    g_object_ref (cancellable);

  if (progress->cancellable)
    g_object_unref (progress->cancellable);

  progress->cancellable = cancellable;

  va_start (args, format);
  message = g_strdup_vprintf (format, args);
  va_end (args);

  if (progress->vtable.start)
    progress->vtable.start (message, cancellable != NULL, progress->user_data);

  g_free (message);
}

void
_picman_help_progress_update (PicmanHelpProgress *progress,
                            gdouble           percentage)
{
  g_return_if_fail (progress != NULL);

  if (progress->vtable.set_value)
    progress->vtable.set_value (percentage, progress->user_data);
}

void
_picman_help_progress_pulse (PicmanHelpProgress *progress)
{
  g_return_if_fail (progress != NULL);

  _picman_help_progress_update (progress, -1.0);
}

void
_picman_help_progress_finish (PicmanHelpProgress *progress)
{
  g_return_if_fail (progress != NULL);

  if (progress->vtable.end)
    progress->vtable.end (progress->user_data);

  if (progress->cancellable)
    {
      g_object_unref (progress->cancellable);
      progress->cancellable = NULL;
    }
}
