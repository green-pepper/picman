/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanprogress.c
 * Copyright (C) 2004  Michael Natterer <mitch@picman.org>
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

#include "core-types.h"

#include "picman.h"
#include "picmanmarshal.h"
#include "picmanprogress.h"

#include "picman-intl.h"


enum
{
  CANCEL,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void   picman_progress_iface_base_init (PicmanProgressInterface *progress_iface);


static guint progress_signals[LAST_SIGNAL] = { 0 };


GType
picman_progress_interface_get_type (void)
{
  static GType progress_iface_type = 0;

  if (! progress_iface_type)
    {
      const GTypeInfo progress_iface_info =
      {
        sizeof (PicmanProgressInterface),
        (GBaseInitFunc)     picman_progress_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      progress_iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                                    "PicmanProgressInterface",
                                                    &progress_iface_info,
                                                    0);

      g_type_interface_add_prerequisite (progress_iface_type, G_TYPE_OBJECT);
    }

  return progress_iface_type;
}

static void
picman_progress_iface_base_init (PicmanProgressInterface *progress_iface)
{
  static gboolean initialized = FALSE;

  if (! initialized)
    {
      progress_signals[CANCEL] =
        g_signal_new ("cancel",
                      G_TYPE_FROM_INTERFACE (progress_iface),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (PicmanProgressInterface, cancel),
                      NULL, NULL,
                      picman_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

      initialized = TRUE;
    }
}

PicmanProgress *
picman_progress_start (PicmanProgress *progress,
                     const gchar  *message,
                     gboolean      cancelable)
{
  PicmanProgressInterface *progress_iface;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), NULL);

  if (! message)
    message = _("Please wait");

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->start)
    return progress_iface->start (progress, message, cancelable);

  return NULL;
}

void
picman_progress_end (PicmanProgress *progress)
{
  PicmanProgressInterface *progress_iface;

  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->end)
    progress_iface->end (progress);
}

gboolean
picman_progress_is_active (PicmanProgress *progress)
{
  PicmanProgressInterface *progress_iface;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), FALSE);

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->is_active)
    return progress_iface->is_active (progress);

  return FALSE;
}

void
picman_progress_set_text (PicmanProgress *progress,
                        const gchar  *message)
{
  PicmanProgressInterface *progress_iface;

  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  if (! message || ! strlen (message))
    message = _("Please wait");

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->set_text)
    progress_iface->set_text (progress, message);
}

void
picman_progress_set_value (PicmanProgress *progress,
                         gdouble       percentage)
{
  PicmanProgressInterface *progress_iface;

  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  percentage = CLAMP (percentage, 0.0, 1.0);

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->set_value)
    progress_iface->set_value (progress, percentage);
}

gdouble
picman_progress_get_value (PicmanProgress *progress)
{
  PicmanProgressInterface *progress_iface;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), 0.0);

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->get_value)
    return progress_iface->get_value (progress);

  return 0.0;
}

void
picman_progress_pulse (PicmanProgress *progress)
{
  PicmanProgressInterface *progress_iface;

  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->pulse)
    progress_iface->pulse (progress);
}

guint32
picman_progress_get_window_id (PicmanProgress *progress)
{
  PicmanProgressInterface *progress_iface;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), 0);

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->get_window_id)
    return progress_iface->get_window_id (progress);

  return 0;
}

gboolean
picman_progress_message (PicmanProgress        *progress,
                       Picman                *picman,
                       PicmanMessageSeverity  severity,
                       const gchar         *domain,
                       const gchar         *message)
{
  PicmanProgressInterface *progress_iface;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (domain != NULL, FALSE);
  g_return_val_if_fail (message != NULL, FALSE);

  progress_iface = PICMAN_PROGRESS_GET_INTERFACE (progress);

  if (progress_iface->message)
    return progress_iface->message (progress, picman, severity, domain, message);

  return FALSE;
}

void
picman_progress_cancel (PicmanProgress *progress)
{
  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  g_signal_emit (progress, progress_signals[CANCEL], 0);
}

void
picman_progress_update_and_flush (gint     min,
                                gint     max,
                                gint     current,
                                gpointer data)
{
  picman_progress_set_value (PICMAN_PROGRESS (data),
                           (gdouble) (current - min) / (gdouble) (max - min));

  while (g_main_context_pending (NULL))
    g_main_context_iteration (NULL, TRUE);
}
