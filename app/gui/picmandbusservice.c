/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanDBusService
 * Copyright (C) 2007, 2008 Sven Neumann <sven@picman.org>
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

#if HAVE_DBUS_GLIB

#include <gegl.h>
#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>

#include "gui-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"

#include "file/file-open.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmandbusservice.h"
#include "picmandbusservice-glue.h"


enum
{
  OPENED,
  LAST_SIGNAL
};

typedef struct
{
  gchar    *uri;
  gboolean  as_new;
} OpenData;


static void       picman_dbus_service_class_init (PicmanDBusServiceClass *klass);

static void       picman_dbus_service_init           (PicmanDBusService  *service);
static void       picman_dbus_service_dispose        (GObject          *object);
static void       picman_dbus_service_finalize       (GObject          *object);

static void       picman_dbus_service_picman_opened    (Picman             *picman,
						    const gchar      *uri,
						    PicmanDBusService  *service);

static gboolean   picman_dbus_service_queue_open     (PicmanDBusService  *service,
                                                    const gchar      *uri,
                                                    gboolean          as_new);

static gboolean   picman_dbus_service_open_idle      (PicmanDBusService  *service);
static OpenData * picman_dbus_service_open_data_new  (PicmanDBusService  *service,
                                                    const gchar      *uri,
                                                    gboolean          as_new);
static void       picman_dbus_service_open_data_free (OpenData         *data);


G_DEFINE_TYPE (PicmanDBusService, picman_dbus_service, G_TYPE_OBJECT)

#define parent_class picman_dbus_service_parent_class

static guint picman_dbus_service_signals[LAST_SIGNAL] = { 0 };


static void
picman_dbus_service_class_init (PicmanDBusServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  picman_dbus_service_signals[OPENED] =
    g_signal_new ("opened",
		  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDBusServiceClass, opened),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  object_class->dispose  = picman_dbus_service_dispose;
  object_class->finalize = picman_dbus_service_finalize;

  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                   &dbus_glib_picman_object_info);
}

static void
picman_dbus_service_init (PicmanDBusService *service)
{
  service->queue = g_queue_new ();
}

GObject *
picman_dbus_service_new (Picman *picman)
{
  PicmanDBusService *service;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  service = g_object_new (PICMAN_TYPE_DBUS_SERVICE, NULL);

  service->picman = picman;

  g_signal_connect_object (picman, "image-opened",
			   G_CALLBACK (picman_dbus_service_picman_opened),
			   service, 0);

  return G_OBJECT (service);
}

static void
picman_dbus_service_dispose (GObject *object)
{
  PicmanDBusService *service = PICMAN_DBUS_SERVICE (object);

  if (service->source)
    {
      g_source_remove (g_source_get_id (service->source));
      service->source = NULL;
    }

  while (! g_queue_is_empty (service->queue))
    {
      picman_dbus_service_open_data_free (g_queue_pop_head (service->queue));
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_dbus_service_finalize (GObject *object)
{
  PicmanDBusService *service = PICMAN_DBUS_SERVICE (object);

  if (service->queue)
    {
      g_queue_free (service->queue);
      service->queue = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

gboolean
picman_dbus_service_open (PicmanDBusService  *service,
                        const gchar      *uri,
                        gboolean         *success,
                        GError          **dbus_error)
{
  g_return_val_if_fail (PICMAN_IS_DBUS_SERVICE (service), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);
  g_return_val_if_fail (success != NULL, FALSE);

  *success = picman_dbus_service_queue_open (service, uri, FALSE);

  return TRUE;
}

gboolean
picman_dbus_service_open_as_new (PicmanDBusService  *service,
                               const gchar      *uri,
                               gboolean         *success,
                               GError          **dbus_error)
{
  g_return_val_if_fail (PICMAN_IS_DBUS_SERVICE (service), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);
  g_return_val_if_fail (success != NULL, FALSE);

  *success = picman_dbus_service_queue_open (service, uri, TRUE);

  return TRUE;
}

gboolean
picman_dbus_service_activate (PicmanDBusService  *service,
                            GError          **dbus_error)
{
  PicmanObject *display;

  g_return_val_if_fail (PICMAN_IS_DBUS_SERVICE (service), FALSE);

  /*  We want to be called again later in case that PICMAN is not fully
   *  started yet.
   */
  if (! picman_is_restored (service->picman))
    return TRUE;

  display = picman_container_get_first_child (service->picman->displays);

  if (display)
    picman_display_shell_present (picman_display_get_shell (PICMAN_DISPLAY (display)));

  return TRUE;
}

static void
picman_dbus_service_picman_opened (Picman            *picman,
			       const gchar     *uri,
			       PicmanDBusService *service)
{
  g_signal_emit (service, picman_dbus_service_signals[OPENED], 0, uri);
}

/*
 * Adds a request to open a file to the end of the queue and
 * starts an idle source if it is not already running.
 */
static gboolean
picman_dbus_service_queue_open (PicmanDBusService *service,
                              const gchar     *uri,
                              gboolean         as_new)
{
  g_queue_push_tail (service->queue,
                     picman_dbus_service_open_data_new (service, uri, as_new));

  if (! service->source)
    {
      service->source = g_idle_source_new ();

      g_source_set_priority (service->source, G_PRIORITY_LOW);
      g_source_set_callback (service->source,
                             (GSourceFunc) picman_dbus_service_open_idle, service,
                             NULL);
      g_source_attach (service->source, NULL);
      g_source_unref (service->source);
    }

  /*  The call always succeeds as it is handled in one way or another.
   *  Even presenting an error message is considered success ;-)
   */
  return TRUE;
}

/*
 * Idle callback that removes the first request from the queue and
 * handles it. If there are no more requests, the idle source is
 * removed.
 */
static gboolean
picman_dbus_service_open_idle (PicmanDBusService *service)
{
  OpenData *data;

  if (! service->picman->restored)
    return TRUE;

  data = g_queue_pop_tail (service->queue);

  if (data)
    {
      file_open_from_command_line (service->picman, data->uri, data->as_new);

      picman_dbus_service_open_data_free (data);

      return TRUE;
    }

  service->source = NULL;

  return FALSE;
}

static OpenData *
picman_dbus_service_open_data_new (PicmanDBusService *service,
                                 const gchar     *uri,
                                 gboolean         as_new)
{
  OpenData *data = g_slice_new (OpenData);

  data->uri    = g_strdup (uri);
  data->as_new = as_new;

  return data;
}

static void
picman_dbus_service_open_data_free (OpenData *data)
{
  g_free (data->uri);
  g_slice_free (OpenData, data);
}


#endif /* HAVE_DBUS_GLIB */
