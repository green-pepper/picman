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

#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#undef GSEAL_ENABLE

#include <glib/gstdio.h>
#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanbase/picmanbase.h"

#ifdef G_OS_WIN32
#include "libpicmanbase/picmanwin32-io.h"
#endif

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmandatafactory.h"
#include "core/picmangradient.h"
#include "core/picmanlist.h"
#include "core/picmanpattern.h"
#include "core/picmantoolinfo.h"

#include "picmandeviceinfo.h"
#include "picmandevicemanager.h"
#include "picmandevices.h"

#include "picman-intl.h"


#define PICMAN_DEVICE_MANAGER_DATA_KEY "picman-device-manager"


static gboolean devicerc_deleted = FALSE;


/*  public functions  */

void
picman_devices_init (Picman *picman)
{
  PicmanDeviceManager *manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  manager = g_object_get_data (G_OBJECT (picman), PICMAN_DEVICE_MANAGER_DATA_KEY);

  g_return_if_fail (manager == NULL);

  manager = picman_device_manager_new (picman);

  g_object_set_data_full (G_OBJECT (picman),
                          PICMAN_DEVICE_MANAGER_DATA_KEY, manager,
                          (GDestroyNotify) g_object_unref);
}

void
picman_devices_exit (Picman *picman)
{
  PicmanDeviceManager *manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  manager = picman_devices_get_manager (picman);

  g_return_if_fail (PICMAN_IS_DEVICE_MANAGER (manager));

  g_object_set_data (G_OBJECT (picman), PICMAN_DEVICE_MANAGER_DATA_KEY, NULL);
}

void
picman_devices_restore (Picman *picman)
{
  PicmanDeviceManager *manager;
  PicmanContext       *user_context;
  PicmanDeviceInfo    *current_device;
  GList             *list;
  gchar             *filename;
  GError            *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  manager = picman_devices_get_manager (picman);

  g_return_if_fail (PICMAN_IS_DEVICE_MANAGER (manager));

  user_context = picman_get_user_context (picman);

  for (list = PICMAN_LIST (manager)->list;
       list;
       list = g_list_next (list))
    {
      PicmanDeviceInfo *device_info = list->data;

      picman_context_copy_properties (user_context, PICMAN_CONTEXT (device_info),
                                    PICMAN_DEVICE_INFO_CONTEXT_MASK);

      picman_device_info_set_default_tool (device_info);
    }

  filename = picman_personal_rc_file ("devicerc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (manager),
                                      filename,
                                      picman,
                                      &error))
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);

      g_error_free (error);
      /* don't bail out here */
    }

  g_free (filename);

  current_device = picman_device_manager_get_current_device (manager);

  picman_context_copy_properties (PICMAN_CONTEXT (current_device), user_context,
                                PICMAN_DEVICE_INFO_CONTEXT_MASK);
  picman_context_set_parent (PICMAN_CONTEXT (current_device), user_context);
}

void
picman_devices_save (Picman     *picman,
                   gboolean  always_save)
{
  PicmanDeviceManager *manager;
  gchar             *filename;
  GError            *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  manager = picman_devices_get_manager (picman);

  g_return_if_fail (PICMAN_IS_DEVICE_MANAGER (manager));

  if (devicerc_deleted && ! always_save)
    return;

  filename = picman_personal_rc_file ("devicerc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (manager),
                                       filename,
                                       "PICMAN devicerc",
                                       "end of devicerc",
                                       NULL,
                                       &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_error_free (error);
    }

  g_free (filename);

  devicerc_deleted = FALSE;
}

gboolean
picman_devices_clear (Picman    *picman,
                    GError **error)
{
  PicmanDeviceManager *manager;
  gchar             *filename;
  gboolean           success = TRUE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  manager = picman_devices_get_manager (picman);

  g_return_val_if_fail (PICMAN_IS_DEVICE_MANAGER (manager), FALSE);

  filename = picman_personal_rc_file ("devicerc");

  if (g_unlink (filename) != 0 && errno != ENOENT)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
		   _("Deleting \"%s\" failed: %s"),
                   picman_filename_to_utf8 (filename), g_strerror (errno));
      success = FALSE;
    }
  else
    {
      devicerc_deleted = TRUE;
    }

  g_free (filename);

  return success;
}

PicmanDeviceManager *
picman_devices_get_manager (Picman *picman)
{
  PicmanDeviceManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = g_object_get_data (G_OBJECT (picman), PICMAN_DEVICE_MANAGER_DATA_KEY);

  g_return_val_if_fail (PICMAN_IS_DEVICE_MANAGER (manager), NULL);

  return manager;
}

void
picman_devices_add_widget (Picman      *picman,
                         GtkWidget *widget)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_ALL);

  g_signal_connect (widget, "motion-notify-event",
                    G_CALLBACK (picman_devices_check_callback),
                    picman);
}

gboolean
picman_devices_check_callback (GtkWidget *widget,
                             GdkEvent  *event,
                             Picman      *picman)
{
  g_return_val_if_fail (event != NULL, FALSE);
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  if (! picman->busy)
    picman_devices_check_change (picman, event);

  return FALSE;
}

gboolean
picman_devices_check_change (Picman     *picman,
                           GdkEvent *event)
{
  PicmanDeviceManager *manager;
  GdkDevice         *device;
  PicmanDeviceInfo    *device_info;
  GtkWidget         *source;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  manager = picman_devices_get_manager (picman);

  g_return_val_if_fail (PICMAN_IS_DEVICE_MANAGER (manager), FALSE);

  /* It is possible that the event was propagated from a widget that does not
     want extension events and therefore always sends core pointer events.
     This can cause a false switch to the core pointer device. */

  source = gtk_get_event_widget (event);

  if (source &&
      gtk_widget_get_extension_events (source) == GDK_EXTENSION_EVENTS_NONE)
    return FALSE;

  switch (event->type)
    {
    case GDK_MOTION_NOTIFY:
      device = ((GdkEventMotion *) event)->device;
      break;

    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      device = ((GdkEventButton *) event)->device;
      break;

    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      device = ((GdkEventProximity *) event)->device;
      break;

    case GDK_SCROLL:
      device = ((GdkEventScroll *) event)->device;
      break;

    default:
      device = picman_device_manager_get_current_device (manager)->device;
      break;
    }

  device_info = picman_device_info_get_by_device (device);

  if (device_info != picman_device_manager_get_current_device (manager))
    {
      picman_device_manager_set_current_device (manager, device_info);
      return TRUE;
    }

  return FALSE;
}
