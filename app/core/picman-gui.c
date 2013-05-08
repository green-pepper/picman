/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman.h"
#include "picman-gui.h"
#include "picmancontainer.h"
#include "picmancontext.h"
#include "picmanimage.h"
#include "picmanprogress.h"

#include "about.h"

#include "picman-intl.h"


void
picman_gui_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  picman->gui.ungrab                = NULL;
  picman->gui.threads_enter         = NULL;
  picman->gui.threads_leave         = NULL;
  picman->gui.set_busy              = NULL;
  picman->gui.unset_busy            = NULL;
  picman->gui.show_message          = NULL;
  picman->gui.help                  = NULL;
  picman->gui.get_program_class     = NULL;
  picman->gui.get_display_name      = NULL;
  picman->gui.get_user_time         = NULL;
  picman->gui.get_theme_dir         = NULL;
  picman->gui.display_get_by_id     = NULL;
  picman->gui.display_get_id        = NULL;
  picman->gui.display_get_window_id = NULL;
  picman->gui.display_create        = NULL;
  picman->gui.display_delete        = NULL;
  picman->gui.displays_reconnect    = NULL;
  picman->gui.progress_new          = NULL;
  picman->gui.progress_free         = NULL;
  picman->gui.pdb_dialog_set        = NULL;
  picman->gui.pdb_dialog_close      = NULL;
  picman->gui.recent_list_add_uri   = NULL;
  picman->gui.recent_list_load      = NULL;
}

void
picman_gui_ungrab (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->gui.ungrab)
    picman->gui.ungrab (picman);
}

void
picman_threads_enter (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->gui.threads_enter)
    picman->gui.threads_enter (picman);
}

void
picman_threads_leave (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->gui.threads_leave)
    picman->gui.threads_leave (picman);
}

void
picman_set_busy (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  /* FIXME: picman_busy HACK */
  picman->busy++;

  if (picman->busy == 1)
    {
      if (picman->gui.set_busy)
        picman->gui.set_busy (picman);
    }
}

static gboolean
picman_idle_unset_busy (gpointer data)
{
  Picman *picman = data;

  picman_unset_busy (picman);

  picman->busy_idle_id = 0;

  return FALSE;
}

void
picman_set_busy_until_idle (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (! picman->busy_idle_id)
    {
      picman_set_busy (picman);

      picman->busy_idle_id = g_idle_add_full (G_PRIORITY_HIGH,
                                            picman_idle_unset_busy, picman,
                                            NULL);
    }
}

void
picman_unset_busy (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (picman->busy > 0);

  /* FIXME: picman_busy HACK */
  picman->busy--;

  if (picman->busy == 0)
    {
      if (picman->gui.unset_busy)
        picman->gui.unset_busy (picman);
    }
}

void
picman_show_message (Picman                *picman,
                   GObject             *handler,
                   PicmanMessageSeverity  severity,
                   const gchar         *domain,
                   const gchar         *message)
{
  const gchar *desc = "Message";

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (handler == NULL || G_IS_OBJECT (handler));
  g_return_if_fail (message != NULL);

  if (! domain)
    domain = PICMAN_ACRONYM;

  if (! picman->console_messages)
    {
      if (picman->gui.show_message)
        {
          picman->gui.show_message (picman, handler,
                                  severity, domain, message);
          return;
        }
      else if (PICMAN_IS_PROGRESS (handler) &&
               picman_progress_message (PICMAN_PROGRESS (handler), picman,
                                      severity, domain, message))
        {
          /* message has been handled by PicmanProgress */
          return;
        }
    }

  picman_enum_get_value (PICMAN_TYPE_MESSAGE_SEVERITY, severity,
                       NULL, NULL, &desc, NULL);
  g_printerr ("%s-%s: %s\n\n", domain, desc, message);
}

void
picman_help (Picman         *picman,
           PicmanProgress *progress,
           const gchar  *help_domain,
           const gchar  *help_id)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (picman->gui.help)
    picman->gui.help (picman, progress, help_domain, help_id);
}

const gchar *
picman_get_program_class (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->gui.get_program_class)
    return picman->gui.get_program_class (picman);

  return NULL;
}

gchar *
picman_get_display_name (Picman *picman,
                       gint  display_ID,
                       gint *monitor_number)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (monitor_number != NULL, NULL);

  if (picman->gui.get_display_name)
    return picman->gui.get_display_name (picman, display_ID, monitor_number);

  *monitor_number = 0;

  return NULL;
}

/**
 * picman_get_user_time:
 * @picman:
 *
 * Returns the timestamp of the last user interaction. The timestamp is
 * taken from events caused by user interaction such as key presses or
 * pointer movements. See gdk_x11_display_get_user_time().
 *
 * Return value: the timestamp of the last user interaction
 */
guint32
picman_get_user_time (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), 0);

  if (picman->gui.get_user_time)
    return picman->gui.get_user_time (picman);

  return 0;
}

const gchar *
picman_get_theme_dir (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->gui.get_theme_dir)
    return picman->gui.get_theme_dir (picman);

  return NULL;
}

PicmanObject *
picman_get_window_strategy (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->gui.get_window_strategy)
    return picman->gui.get_window_strategy (picman);

  return NULL;
}

PicmanObject *
picman_get_empty_display (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->gui.get_empty_display)
    return picman->gui.get_empty_display (picman);

  return NULL;
}

PicmanObject *
picman_get_display_by_ID (Picman *picman,
                        gint  ID)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman->gui.display_get_by_id)
    return picman->gui.display_get_by_id (picman, ID);

  return NULL;
}

gint
picman_get_display_ID (Picman       *picman,
                     PicmanObject *display)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), -1);
  g_return_val_if_fail (PICMAN_IS_OBJECT (display), -1);

  if (picman->gui.display_get_id)
    return picman->gui.display_get_id (display);

  return -1;
}

guint32
picman_get_display_window_id (Picman       *picman,
                            PicmanObject *display)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), -1);
  g_return_val_if_fail (PICMAN_IS_OBJECT (display), -1);

  if (picman->gui.display_get_window_id)
    return picman->gui.display_get_window_id (display);

  return -1;
}

PicmanObject *
picman_create_display (Picman      *picman,
                     PicmanImage *image,
                     PicmanUnit   unit,
                     gdouble    scale)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), NULL);

  if (picman->gui.display_create)
    return picman->gui.display_create (picman, image, unit, scale);

  return NULL;
}

void
picman_delete_display (Picman       *picman,
                     PicmanObject *display)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_OBJECT (display));

  if (picman->gui.display_delete)
    picman->gui.display_delete (display);
}

void
picman_reconnect_displays (Picman      *picman,
                         PicmanImage *old_image,
                         PicmanImage *new_image)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_IMAGE (old_image));
  g_return_if_fail (PICMAN_IS_IMAGE (new_image));

  if (picman->gui.displays_reconnect)
    picman->gui.displays_reconnect (picman, old_image, new_image);
}

PicmanProgress *
picman_new_progress (Picman       *picman,
                   PicmanObject *display)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (display == NULL || PICMAN_IS_OBJECT (display), NULL);

  if (picman->gui.progress_new)
    return picman->gui.progress_new (picman, display);

  return NULL;
}

void
picman_free_progress (Picman         *picman,
                    PicmanProgress *progress)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_PROGRESS (progress));

  if (picman->gui.progress_free)
    picman->gui.progress_free (picman, progress);
}

gboolean
picman_pdb_dialog_new (Picman          *picman,
                     PicmanContext   *context,
                     PicmanProgress  *progress,
                     PicmanContainer *container,
                     const gchar   *title,
                     const gchar   *callback_name,
                     const gchar   *object_name,
                     ...)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (title != NULL, FALSE);
  g_return_val_if_fail (callback_name != NULL, FALSE);

  if (picman->gui.pdb_dialog_new)
    {
      va_list args;

      va_start (args, object_name);

      retval = picman->gui.pdb_dialog_new (picman, context, progress,
                                         container, title,
                                         callback_name, object_name,
                                         args);

      va_end (args);
    }

  return retval;
}

gboolean
picman_pdb_dialog_set (Picman          *picman,
                     PicmanContainer *container,
                     const gchar   *callback_name,
                     const gchar   *object_name,
                     ...)
{
  gboolean retval = FALSE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (callback_name != NULL, FALSE);
  g_return_val_if_fail (object_name != NULL, FALSE);

  if (picman->gui.pdb_dialog_set)
    {
      va_list args;

      va_start (args, object_name);

      retval = picman->gui.pdb_dialog_set (picman, container, callback_name,
                                         object_name, args);

      va_end (args);
    }

  return retval;
}

gboolean
picman_pdb_dialog_close (Picman          *picman,
                       PicmanContainer *container,
                       const gchar   *callback_name)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), FALSE);
  g_return_val_if_fail (callback_name != NULL, FALSE);

  if (picman->gui.pdb_dialog_close)
    return picman->gui.pdb_dialog_close (picman, container, callback_name);

  return FALSE;
}

gboolean
picman_recent_list_add_uri (Picman        *picman,
                          const gchar *uri,
                          const gchar *mime_type)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  if (picman->gui.recent_list_add_uri)
    return picman->gui.recent_list_add_uri (picman, uri, mime_type);

  return FALSE;
}

void
picman_recent_list_load (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->gui.recent_list_load)
    picman->gui.recent_list_load (picman);
}
