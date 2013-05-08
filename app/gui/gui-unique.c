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

#include <gegl.h>
#include <gtk/gtk.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <Carbon/Carbon.h>
#include <sys/param.h>
#endif

#if HAVE_DBUS_GLIB
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#endif

#include "gui/gui-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"

#include "display/picmandisplay.h"

#include "file/file-open.h"

#include "picmandbusservice.h"
#include "gui-unique.h"


#if HAVE_DBUS_GLIB
static void  gui_dbus_service_init (Picman *picman);
static void  gui_dbus_service_exit (void);

static DBusGConnection *dbus_connection  = NULL;
#endif

#ifdef G_OS_WIN32
static void  gui_unique_win32_init (Picman *picman);
static void  gui_unique_win32_exit (void);

static Picman            *unique_picman      = NULL;
static HWND             proxy_window     = NULL;
#endif

#ifdef GDK_WINDOWING_QUARTZ
static void  gui_unique_mac_init (Picman *picman);
static void  gui_unique_mac_exit (void);

static Picman            *unique_picman      = NULL;
AEEventHandlerUPP       open_document_callback_proc;
#endif


void
gui_unique_init (Picman *picman)
{
#ifdef G_OS_WIN32
  gui_unique_win32_init (picman);
#elif HAVE_DBUS_GLIB
  gui_dbus_service_init (picman);
#endif

#ifdef GDK_WINDOWING_QUARTZ
  gui_unique_mac_init (picman);
#endif
}

void
gui_unique_exit (void)
{
#ifdef G_OS_WIN32
  gui_unique_win32_exit ();
#elif HAVE_DBUS_GLIB
  gui_dbus_service_exit ();
#endif

#ifdef GDK_WINDOWING_QUARTZ
  gui_unique_mac_exit ();
#endif
}


#if HAVE_DBUS_GLIB

static void
gui_dbus_service_init (Picman *picman)
{
  GError  *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (dbus_connection == NULL);

  dbus_connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (dbus_connection)
    {
      GObject *service = picman_dbus_service_new (picman);

      dbus_bus_request_name (dbus_g_connection_get_connection (dbus_connection),
                             PICMAN_DBUS_SERVICE_NAME, 0, NULL);

      dbus_g_connection_register_g_object (dbus_connection,
                                           PICMAN_DBUS_SERVICE_PATH, service);
    }
  else
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);
    }
}

static void
gui_dbus_service_exit (void)
{
  if (dbus_connection)
    {
      dbus_g_connection_unref (dbus_connection);
      dbus_connection = NULL;
    }
}

#endif  /* HAVE_DBUS_GLIB */


#ifdef G_OS_WIN32

typedef struct
{
  gchar    *name;
  gboolean  as_new;
} IdleOpenData;


static IdleOpenData *
idle_open_data_new (const gchar *name,
                    gint         len,
                    gboolean     as_new)
{
  IdleOpenData *data = g_slice_new0 (IdleOpenData);

  if (len > 0)
    {
      data->name   = g_strdup (name);
      data->as_new = as_new;
    }

  return data;
}

static void
idle_open_data_free (IdleOpenData *data)
{
  g_free (data->name);
  g_slice_free (IdleOpenData, data);
}

static gboolean
gui_unique_win32_idle_open (IdleOpenData *data)
{
  /*  We want to be called again later in case that PICMAN is not fully
   *  started yet.
   */
  if (! picman_is_restored (unique_picman))
    return TRUE;

  if (data->name)
    {
      file_open_from_command_line (unique_picman, data->name, data->as_new);
    }
  else
    {
      /*  raise the first display  */
      PicmanObject *display;

      display = picman_container_get_first_child (unique_picman->displays);

      picman_display_shell_present (picman_display_get_shell (PICMAN_DISPLAY (display)));
    }

  return FALSE;
}

static LRESULT CALLBACK
gui_unique_win32_message_handler (HWND   hWnd,
                                  UINT   uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
  switch (uMsg)
    {
    case WM_COPYDATA:
      if (unique_picman)
        {
          COPYDATASTRUCT *copydata = (COPYDATASTRUCT *) lParam;
          GSource        *source;
          GClosure       *closure;
          IdleOpenData   *data;

          data = idle_open_data_new (copydata->lpData,
                                     copydata->cbData,
                                     copydata->dwData != 0);

          closure = g_cclosure_new (G_CALLBACK (gui_unique_win32_idle_open),
                                    data,
                                    (GClosureNotify) idle_open_data_free);

          g_object_watch_closure (unique_picman, closure);

          source = g_idle_source_new ();
          g_source_set_priority (source, G_PRIORITY_LOW);
          g_source_set_closure (source, closure);
          g_source_attach (source, NULL);
          g_source_unref (source);
        }
      return TRUE;

    default:
      return DefWindowProcW (hWnd, uMsg, wParam, lParam);
    }
}

static void
gui_unique_win32_init (Picman *picman)
{
  WNDCLASSW wc;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (unique_picman == NULL);

  unique_picman = picman;

  /* register window class for proxy window */
  memset (&wc, 0, sizeof (wc));

  wc.hInstance     = GetModuleHandle (NULL);
  wc.lpfnWndProc   = gui_unique_win32_message_handler;
  wc.lpszClassName = PICMAN_UNIQUE_WIN32_WINDOW_CLASS;

  RegisterClassW (&wc);

  proxy_window = CreateWindowExW (0,
                                  PICMAN_UNIQUE_WIN32_WINDOW_CLASS,
                                  PICMAN_UNIQUE_WIN32_WINDOW_NAME,
                                  WS_POPUP, 0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);
}

static void
gui_unique_win32_exit (void)
{
  g_return_if_fail (PICMAN_IS_PICMAN (unique_picman));

  unique_picman = NULL;

  DestroyWindow (proxy_window);
}

#endif  /* G_OS_WIN32 */


#ifdef GDK_WINDOWING_QUARTZ

static gboolean
gui_unique_mac_idle_open (gchar *data)
{
  /*  We want to be called again later in case that PICMAN is not fully
   *  started yet.
   */
  if (! picman_is_restored (unique_picman))
    return TRUE;

  if (data)
    {
      file_open_from_command_line (unique_picman, data, FALSE);
    }

  return FALSE;
}

/* Handle the kAEOpenDocuments Apple events. This will register
 * an idle source callback for each filename in the event.
 */
static pascal OSErr
gui_unique_mac_open_documents (const AppleEvent *inAppleEvent,
                               AppleEvent       *outAppleEvent,
                               long              handlerRefcon)
{
  OSStatus    status;
  AEDescList  documents;
  gchar       path[MAXPATHLEN];

  status = AEGetParamDesc (inAppleEvent,
                           keyDirectObject, typeAEList,
                           &documents);
  if (status == noErr)
    {
      long count = 0;
      int  i;

      AECountItems (&documents, &count);

      for (i = 0; i < count; i++)
        {
          FSRef    ref;
          gchar    *callback_path;
          GSource  *source;
          GClosure *closure;

          status = AEGetNthPtr (&documents, i + 1, typeFSRef,
                                0, 0, &ref, sizeof (ref),
                                0);
          if (status != noErr)
            continue;

          FSRefMakePath (&ref, (UInt8 *) path, MAXPATHLEN);

          callback_path = g_strdup (path);

          closure = g_cclosure_new (G_CALLBACK (gui_unique_mac_idle_open),
                                    (gpointer) callback_path,
                                    (GClosureNotify) g_free);

          g_object_watch_closure (G_OBJECT (unique_picman), closure);

          source = g_idle_source_new ();
          g_source_set_priority (source, G_PRIORITY_LOW);
          g_source_set_closure (source, closure);
          g_source_attach (source, NULL);
          g_source_unref (source);
        }
    }

    return status;
}

static void
gui_unique_mac_init (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (unique_picman == NULL);

  unique_picman = picman;

  open_document_callback_proc = NewAEEventHandlerUPP(gui_unique_mac_open_documents);

  AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
                         open_document_callback_proc,
                         0L, TRUE);
}

static void
gui_unique_mac_exit (void)
{
  unique_picman = NULL;

  AERemoveEventHandler (kCoreEventClass, kAEOpenDocuments,
                        open_document_callback_proc, TRUE);

  DisposeAEEventHandlerUPP(open_document_callback_proc);
}

#endif /* GDK_WINDOWING_QUARTZ */
