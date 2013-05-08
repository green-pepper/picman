/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansessioninfo.c
 * Copyright (C) 2001-2008 Michael Natterer <mitch@picman.org>
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

#include "libpicmanconfig/picmanconfig.h"

#include "widgets-types.h"

#include "config/picmanguiconfig.h"

#include "widgets/picmandockcontainer.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "picmandialogfactory.h"
#include "picmandock.h"
#include "picmandockwindow.h"
#include "picmansessioninfo.h"
#include "picmansessioninfo-aux.h"
#include "picmansessioninfo-book.h"
#include "picmansessioninfo-dock.h"
#include "picmansessioninfo-private.h"
#include "picmansessionmanaged.h"

#include "picman-log.h"


enum
{
  SESSION_INFO_FACTORY_ENTRY,
  SESSION_INFO_POSITION,
  SESSION_INFO_SIZE,
  SESSION_INFO_OPEN,
  SESSION_INFO_AUX,
  SESSION_INFO_DOCK,
  SESSION_INFO_PICMAN_DOCK,
  SESSION_INFO_PICMAN_TOOLBOX
};

#define DEFAULT_SCREEN  -1


typedef struct
{
  PicmanSessionInfo   *info;
  PicmanDialogFactory *factory;
  GdkScreen         *screen;
  GtkWidget         *dialog;
} PicmanRestoreDocksData;


static void      picman_session_info_config_iface_init  (PicmanConfigInterface  *iface);
static void      picman_session_info_finalize           (GObject              *object);
static gint64    picman_session_info_get_memsize        (PicmanObject           *object,
                                                       gint64               *gui_size);
static gboolean  picman_session_info_serialize          (PicmanConfig           *config,
                                                       PicmanConfigWriter     *writer,
                                                       gpointer              data);
static gboolean  picman_session_info_deserialize        (PicmanConfig           *config,
                                                       GScanner             *scanner,
                                                       gint                  nest_level,
                                                       gpointer              data);
static gboolean  picman_session_info_is_for_dock_window (PicmanSessionInfo      *info);
static void      picman_session_info_dialog_show        (GtkWidget            *widget,
                                                       PicmanSessionInfo      *info);
static gboolean  picman_session_info_restore_docks      (PicmanRestoreDocksData *data);


G_DEFINE_TYPE_WITH_CODE (PicmanSessionInfo, picman_session_info, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_session_info_config_iface_init))

#define parent_class picman_session_info_parent_class


static void
picman_session_info_class_init (PicmanSessionInfoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->finalize         = picman_session_info_finalize;

  picman_object_class->get_memsize = picman_session_info_get_memsize;

  g_type_class_add_private (klass, sizeof (PicmanSessionInfoPrivate));
}

static void
picman_session_info_init (PicmanSessionInfo *info)
{
  info->p = G_TYPE_INSTANCE_GET_PRIVATE (info,
                                         PICMAN_TYPE_SESSION_INFO,
                                         PicmanSessionInfoPrivate);
  info->p->screen = DEFAULT_SCREEN;
}

static void
picman_session_info_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_session_info_serialize;
  iface->deserialize = picman_session_info_deserialize;
}

static void
picman_session_info_finalize (GObject *object)
{
  PicmanSessionInfo *info = PICMAN_SESSION_INFO (object);

  picman_session_info_clear_info (info);

  picman_session_info_set_widget (info, NULL);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_session_info_get_memsize (PicmanObject *object,
                               gint64     *gui_size)
{
#if 0
  PicmanSessionInfo *info    = PICMAN_SESSION_INFO (object);
#endif
  gint64           memsize = 0;

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_session_info_serialize (PicmanConfig       *config,
                             PicmanConfigWriter *writer,
                             gpointer          data)
{
  PicmanSessionInfo      *info  = PICMAN_SESSION_INFO (config);
  PicmanSessionInfoClass *klass = PICMAN_SESSION_INFO_GET_CLASS (info);
  GList                *iter  = NULL;
  gint                  x_to_write;
  gint                  y_to_write;
  gint                  w_to_write;
  gint                  h_to_write;

  if (info->p->factory_entry && info->p->factory_entry->identifier)
    {
      picman_config_writer_open (writer, "factory-entry");
      picman_config_writer_string (writer, info->p->factory_entry->identifier);
      picman_config_writer_close (writer);
    }

  x_to_write = picman_session_info_class_apply_position_accuracy (klass,
                                                                info->p->x);
  y_to_write = picman_session_info_class_apply_position_accuracy (klass,
                                                                info->p->y);
  w_to_write = picman_session_info_class_apply_position_accuracy (klass,
                                                                info->p->width);
  h_to_write = picman_session_info_class_apply_position_accuracy (klass,
                                                                info->p->height);

  picman_config_writer_open (writer, "position");
  picman_config_writer_printf (writer, "%d %d", x_to_write, y_to_write);
  picman_config_writer_close (writer);

  if (info->p->width > 0 && info->p->height > 0)
    {
      picman_config_writer_open (writer, "size");
      picman_config_writer_printf (writer, "%d %d", w_to_write, h_to_write);
      picman_config_writer_close (writer);
    }

  if (info->p->open)
    {
      picman_config_writer_open (writer, "open-on-exit");

      if (info->p->screen != DEFAULT_SCREEN)
        picman_config_writer_printf (writer, "%d", info->p->screen);

      picman_config_writer_close (writer);
    }

  if (info->p->aux_info)
    picman_session_info_aux_serialize (writer, info->p->aux_info);

  for (iter = info->p->docks; iter; iter = g_list_next (iter))
    picman_session_info_dock_serialize (writer, iter->data);

  return TRUE;
}

/*
 * This function is just like picman_scanner_parse_int(), but it is allows
 * to detect the special value '-0'. This is used as in X geometry strings.
 */
static gboolean
picman_session_info_parse_offset (GScanner *scanner,
                                gint     *dest,
                                gboolean *negative)
{
  if (g_scanner_peek_next_token (scanner) == '-')
    {
      *negative = TRUE;
      g_scanner_get_next_token (scanner);
    }
  else
    {
      *negative = FALSE;
    }

  if (g_scanner_peek_next_token (scanner) != G_TOKEN_INT)
    return FALSE;

  g_scanner_get_next_token (scanner);

  if (*negative)
    *dest = -scanner->value.v_int64;
  else
    *dest = scanner->value.v_int64;

  return TRUE;
}

static gboolean
picman_session_info_deserialize (PicmanConfig *config,
                               GScanner   *scanner,
                               gint        nest_level,
                               gpointer    data)
{
  PicmanSessionInfo *info = PICMAN_SESSION_INFO (config);
  GTokenType       token;
  guint            scope_id;
  guint            old_scope_id;

  scope_id = g_type_qname (G_TYPE_FROM_INSTANCE (config));
  old_scope_id = g_scanner_set_scope (scanner, scope_id);

  g_scanner_scope_add_symbol (scanner, scope_id, "factory-entry",
                              GINT_TO_POINTER (SESSION_INFO_FACTORY_ENTRY));
  g_scanner_scope_add_symbol (scanner, scope_id, "position",
                              GINT_TO_POINTER (SESSION_INFO_POSITION));
  g_scanner_scope_add_symbol (scanner, scope_id, "size",
                              GINT_TO_POINTER (SESSION_INFO_SIZE));
  g_scanner_scope_add_symbol (scanner, scope_id, "open-on-exit",
                              GINT_TO_POINTER (SESSION_INFO_OPEN));
  g_scanner_scope_add_symbol (scanner, scope_id, "aux-info",
                              GINT_TO_POINTER (SESSION_INFO_AUX));
  g_scanner_scope_add_symbol (scanner, scope_id, "picman-dock",
                              GINT_TO_POINTER (SESSION_INFO_PICMAN_DOCK));
  g_scanner_scope_add_symbol (scanner, scope_id, "picman-toolbox",
                              GINT_TO_POINTER (SESSION_INFO_PICMAN_TOOLBOX));

  /* For sessionrc files from version <= PICMAN 2.6 */
  g_scanner_scope_add_symbol (scanner, scope_id, "dock",
                              GINT_TO_POINTER (SESSION_INFO_DOCK));

  token = G_TOKEN_LEFT_PAREN;

  while (g_scanner_peek_next_token (scanner) == token)
    {
      token = g_scanner_get_next_token (scanner);

      switch (token)
        {
        case G_TOKEN_LEFT_PAREN:
          token = G_TOKEN_SYMBOL;
          break;

        case G_TOKEN_SYMBOL:
          switch (GPOINTER_TO_INT (scanner->value.v_symbol))
            {
            case SESSION_INFO_FACTORY_ENTRY:
              {
                gchar                  *identifier = NULL;
                PicmanDialogFactoryEntry *entry      = NULL;

                token = G_TOKEN_STRING;
                if (! picman_scanner_parse_string (scanner, &identifier))
                  goto error;

                entry = picman_dialog_factory_find_entry (picman_dialog_factory_get_singleton (),
                                                        identifier);
                if (! entry)
                  goto error;

                picman_session_info_set_factory_entry (info, entry);

                g_free (identifier);
              }
              break;

            case SESSION_INFO_POSITION:
              token = G_TOKEN_INT;
              if (! picman_session_info_parse_offset (scanner,
                                                    &info->p->x,
                                                    &info->p->right_align))
                goto error;
              if (! picman_session_info_parse_offset (scanner,
                                                    &info->p->y,
                                                    &info->p->bottom_align))
                goto error;
              break;

            case SESSION_INFO_SIZE:
              token = G_TOKEN_INT;
              if (! picman_scanner_parse_int (scanner, &info->p->width))
                goto error;
              if (! picman_scanner_parse_int (scanner, &info->p->height))
                goto error;
              break;

            case SESSION_INFO_OPEN:
              info->p->open = TRUE;

              /*  the screen number is optional  */
              if (g_scanner_peek_next_token (scanner) == G_TOKEN_RIGHT_PAREN)
                break;

              token = G_TOKEN_INT;
              if (! picman_scanner_parse_int (scanner, &info->p->screen))
                goto error;
              break;

            case SESSION_INFO_AUX:
              token = picman_session_info_aux_deserialize (scanner,
                                                         &info->p->aux_info);
              if (token != G_TOKEN_LEFT_PAREN)
                goto error;
              break;

            case SESSION_INFO_PICMAN_TOOLBOX:
            case SESSION_INFO_PICMAN_DOCK:
            case SESSION_INFO_DOCK:
              {
                PicmanSessionInfoDock *dock_info  = NULL;
                const gchar         *dock_type = NULL;

                /* Handle old sessionrc:s from versions <= PICMAN 2.6 */
                if (GPOINTER_TO_INT (scanner->value.v_symbol) == SESSION_INFO_DOCK &&
                    info->p->factory_entry &&
                    info->p->factory_entry->identifier &&
                    strcmp ("picman-toolbox-window",
                            info->p->factory_entry->identifier) == 0)
                  {
                    dock_type = "picman-toolbox";
                  }
                else
                  {
                    dock_type = ((GPOINTER_TO_INT (scanner->value.v_symbol) ==
                                  SESSION_INFO_PICMAN_TOOLBOX) ?
                                 "picman-toolbox" :
                                 "picman-dock");
                  }

                g_scanner_set_scope (scanner, scope_id + 1);
                token = picman_session_info_dock_deserialize (scanner, scope_id + 1,
                                                            &dock_info,
                                                            dock_type);

                if (token == G_TOKEN_LEFT_PAREN)
                  {
                    g_scanner_set_scope (scanner, scope_id);
                    info->p->docks = g_list_append (info->p->docks, dock_info);
                  }
                else
                  goto error;
              }
              break;

            default:
              break;
            }
          token = G_TOKEN_RIGHT_PAREN;
          break;

        case G_TOKEN_RIGHT_PAREN:
          token = G_TOKEN_LEFT_PAREN;
          break;

        default:
          break;
        }
    }

 error:

  /* If we don't have docks, assume it is a toolbox dock window from a
   * sessionrc file from PICMAN <= 2.6 and add a toolbox dock manually
   */
  if (! info->p->docks &&
      info->p->factory_entry &&
      strcmp ("picman-toolbox-window",
              info->p->factory_entry->identifier) == 0)
    {
      info->p->docks =
        g_list_append (info->p->docks,
                       picman_session_info_dock_new ("picman-toolbox"));
    }

  g_scanner_scope_remove_symbol (scanner, scope_id, "factory-entry");
  g_scanner_scope_remove_symbol (scanner, scope_id, "position");
  g_scanner_scope_remove_symbol (scanner, scope_id, "size");
  g_scanner_scope_remove_symbol (scanner, scope_id, "open-on-exit");
  g_scanner_scope_remove_symbol (scanner, scope_id, "aux-info");
  g_scanner_scope_remove_symbol (scanner, scope_id, "picman-dock");
  g_scanner_scope_remove_symbol (scanner, scope_id, "picman-toolbox");
  g_scanner_scope_remove_symbol (scanner, scope_id, "dock");

  g_scanner_set_scope (scanner, old_scope_id);

  return picman_config_deserialize_return (scanner, token, nest_level);
}

/**
 * picman_session_info_is_for_dock_window:
 * @info:
 *
 * Helper function to determine if the session info is for a dock. It
 * uses the dialog factory entry state and the associated widget state
 * if any to determine that.
 *
 * Returns: %TRUE if session info is for a dock, %FALSE otherwise.
 **/
static gboolean
picman_session_info_is_for_dock_window (PicmanSessionInfo *info)
{
  gboolean entry_state_for_dock  =  info->p->factory_entry == NULL;
  gboolean widget_state_for_dock = (info->p->widget == NULL ||
                                    PICMAN_IS_DOCK_WINDOW (info->p->widget));

  return entry_state_for_dock && widget_state_for_dock;
}

static void
picman_session_info_dialog_show (GtkWidget       *widget,
                               PicmanSessionInfo *info)
{
  gtk_window_move (GTK_WINDOW (widget),
                   info->p->x, info->p->y);
}

static gboolean
picman_session_info_restore_docks (PicmanRestoreDocksData *data)
{
  PicmanSessionInfo     *info    = data->info;
  PicmanDialogFactory   *factory = data->factory;
  GdkScreen           *screen  = data->screen;
  GtkWidget           *dialog  = data->dialog;
  GList               *iter;

  if (PICMAN_IS_DOCK_CONTAINER (dialog))
    {
      /* We expect expect there to always be docks. In sessionrc files
       * from <= 2.6 not all dock window entries had dock entries, but we
       * take care of that during sessionrc parsing
       */
      for (iter = info->p->docks; iter; iter = g_list_next (iter))
        {
          PicmanSessionInfoDock *dock_info = (PicmanSessionInfoDock *) iter->data;
          GtkWidget           *dock;

          dock =
            GTK_WIDGET (picman_session_info_dock_restore (dock_info,
                                                        factory,
                                                        screen,
                                                        PICMAN_DOCK_CONTAINER (dialog)));

          if (dock && dock_info->position != 0)
            {
              GtkWidget *parent = gtk_widget_get_parent (dock);

              if (GTK_IS_PANED (parent))
                {
                  GtkPaned *paned = GTK_PANED (parent);

                  if (dock == gtk_paned_get_child2 (paned))
                    gtk_paned_set_position (paned, dock_info->position);
                }
            }
        }
    }

  picman_session_info_clear_info (info);

  g_object_unref (dialog);
  g_object_unref (screen);
  g_object_unref (factory);
  g_object_unref (info);

  g_slice_free (PicmanRestoreDocksData, data);

  return FALSE;
}


/*  public functions  */

PicmanSessionInfo *
picman_session_info_new (void)
{
  return g_object_new (PICMAN_TYPE_SESSION_INFO, NULL);
}

void
picman_session_info_restore (PicmanSessionInfo   *info,
                           PicmanDialogFactory *factory)
{
  GtkWidget            *dialog  = NULL;
  GdkDisplay           *display = NULL;
  GdkScreen            *screen  = NULL;
  PicmanRestoreDocksData *data    = NULL;

  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));

  g_object_ref (info);

  display = gdk_display_get_default ();

  if (info->p->screen != DEFAULT_SCREEN)
    screen = gdk_display_get_screen (display, info->p->screen);

  if (! screen)
    screen = gdk_display_get_default_screen (display);

  info->p->open   = FALSE;
  info->p->screen = DEFAULT_SCREEN;

  if (info->p->factory_entry &&
      info->p->factory_entry->restore_func)
    {
      dialog = info->p->factory_entry->restore_func (factory,
                                                     screen,
                                                     info);
    }

  if (PICMAN_IS_SESSION_MANAGED (dialog) && info->p->aux_info)
    picman_session_managed_set_aux_info (PICMAN_SESSION_MANAGED (dialog),
                                       info->p->aux_info);

  /* In single-window mode, picman_session_managed_set_aux_info()
   * will set the size of the dock areas at the sides. If we don't
   * wait for those areas to get their size-allocation, we can't
   * properly restore the docks inside them, so do that in an idle
   * callback.
   */

  /* Objects are unreffed again in the callback */
  data = g_slice_new0 (PicmanRestoreDocksData);
  data->info    = g_object_ref (info);
  data->factory = g_object_ref (factory);
  data->screen  = g_object_ref (screen);
  data->dialog  = g_object_ref (dialog);

  g_idle_add ((GSourceFunc) picman_session_info_restore_docks, data);

  g_object_unref (info);
}

/* This function mostly lifted from
 * gtk+/gdk/gdkscreen.c:gdk_screen_get_monitor_at_window()
 */
static gint
picman_session_info_get_appropriate_monitor (GdkScreen *screen,
                                           gint       x,
                                           gint       y,
                                           gint       w,
                                           gint       h)
{
  GdkRectangle rect;
  gint         area    = 0;
  gint         monitor = -1;
  gint         num_monitors;
  gint         i;

  rect.x      = x;
  rect.y      = y;
  rect.width  = w;
  rect.height = h;

  num_monitors = gdk_screen_get_n_monitors (screen);

  for (i = 0; i < num_monitors; i++)
    {
      GdkRectangle geometry;

      gdk_screen_get_monitor_geometry (screen, i, &geometry);

      if (gdk_rectangle_intersect (&rect, &geometry, &geometry) &&
          geometry.width * geometry.height > area)
        {
          area = geometry.width * geometry.height;
          monitor = i;
        }
    }

  if (monitor >= 0)
    return monitor;
  else
    return gdk_screen_get_monitor_at_point (screen,
                                            rect.x + rect.width / 2,
                                            rect.y + rect.height / 2);
}

/**
 * picman_session_info_apply_geometry:
 * @info:
 *
 * Apply the geometry stored in the session info object to the
 * associated widget.
 **/
void
picman_session_info_apply_geometry (PicmanSessionInfo *info)
{
  GdkScreen   *screen;
  GdkRectangle rect;
  gchar        geom[32];
  gint         monitor;
  gboolean     use_size;

  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));
  g_return_if_fail (GTK_IS_WINDOW (info->p->widget));

  screen = gtk_widget_get_screen (info->p->widget);

  use_size = (picman_session_info_get_remember_size (info) &&
              info->p->width  > 0 &&
              info->p->height > 0);

  if (use_size)
    {
      monitor = picman_session_info_get_appropriate_monitor (screen,
                                                           info->p->x,
                                                           info->p->y,
                                                           info->p->width,
                                                           info->p->height);
    }
  else
    {
      monitor = gdk_screen_get_monitor_at_point (screen, info->p->x, info->p->y);
    }

  gdk_screen_get_monitor_geometry (screen, monitor, &rect);

  info->p->x = CLAMP (info->p->x,
                   rect.x,
                   rect.x + rect.width - (info->p->width > 0 ?
                                          info->p->width : 128));
  info->p->y = CLAMP (info->p->y,
                   rect.y,
                   rect.y + rect.height - (info->p->height > 0 ?
                                           info->p->height : 128));

  if (info->p->right_align && info->p->bottom_align)
    {
      g_strlcpy (geom, "-0-0", sizeof (geom));
    }
  else if (info->p->right_align)
    {
      g_snprintf (geom, sizeof (geom), "-0%+d", info->p->y);
    }
  else if (info->p->bottom_align)
    {
      g_snprintf (geom, sizeof (geom), "%+d-0", info->p->x);
    }
  else
    {
      g_snprintf (geom, sizeof (geom), "%+d%+d", info->p->x, info->p->y);
    }

  gtk_window_parse_geometry (GTK_WINDOW (info->p->widget), geom);

  if (use_size)
    gtk_window_set_default_size (GTK_WINDOW (info->p->widget),
                                 info->p->width, info->p->height);

  /*  Window managers and windowing systems suck. They have their own
   *  ideas about WM standards and when it's appropriate to honor
   *  user/application-set window positions and when not. Therefore,
   *  use brute force and "manually" position dialogs whenever they
   *  are shown. This is important especially for transient dialog,
   *  because window managers behave even "smarter" then...
   */
  if (GTK_IS_DIALOG (info->p->widget))
    g_signal_connect (info->p->widget, "show",
                      G_CALLBACK (picman_session_info_dialog_show),
                      info);
}

/**
 * picman_session_info_read_geometry:
 * @info:  A #PicmanSessionInfo
 * @cevent A #GdkEventConfigure. If set, use the size from here
 *         instead of from the window allocation.
 *
 * Read geometry related information from the associated widget.
 **/
void
picman_session_info_read_geometry (PicmanSessionInfo   *info,
                                 GdkEventConfigure *cevent)
{
  GdkWindow *window;

  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));
  g_return_if_fail (GTK_IS_WINDOW (info->p->widget));

  window = gtk_widget_get_window (info->p->widget);

  if (window)
    {
      gint x, y;

      gdk_window_get_root_origin (window, &x, &y);

      /* Don't write negative values to the sessionrc, they are
       * interpreted as relative to the right, respective bottom edge
       * of the screen.
       */
      info->p->x = MAX (0, x);
      info->p->y = MAX (0, y);

      if (picman_session_info_get_remember_size (info))
        {
          int width;
          int height;

          if (cevent)
            {
              width  = cevent->width;
              height = cevent->height;
            }
          else
            {
              GtkAllocation allocation;

              gtk_widget_get_allocation (info->p->widget, &allocation);

              width  = allocation.width;
              height = allocation.height;
            }

          info->p->width  = width;
          info->p->height = height;
        }
      else
        {
          info->p->width  = 0;
          info->p->height = 0;
        }
    }

  info->p->open = FALSE;

  if (picman_session_info_get_remember_if_open (info))
    {
      PicmanDialogVisibilityState visibility;

      visibility =
        GPOINTER_TO_INT (g_object_get_data (G_OBJECT (info->p->widget),
                                            PICMAN_DIALOG_VISIBILITY_KEY));

      switch (visibility)
        {
        case PICMAN_DIALOG_VISIBILITY_UNKNOWN:
          info->p->open = gtk_widget_get_visible (info->p->widget);
          break;

        case PICMAN_DIALOG_VISIBILITY_INVISIBLE:
          info->p->open = FALSE;
          break;

        case PICMAN_DIALOG_VISIBILITY_HIDDEN:
        case PICMAN_DIALOG_VISIBILITY_VISIBLE:
          /* Even if a dialog is hidden (with Windows->Hide docks) it
           * is still considered open. It will be restored the next
           * time PICMAN starts
           */
          info->p->open = TRUE;
          break;
        }
    }

  info->p->screen = DEFAULT_SCREEN;

  if (info->p->open)
    {
      GdkDisplay *display = gtk_widget_get_display (info->p->widget);
      GdkScreen  *screen  = gtk_widget_get_screen (info->p->widget);

      if (screen != gdk_display_get_default_screen (display))
        info->p->screen = gdk_screen_get_number (screen);
    }
}

void
picman_session_info_get_info (PicmanSessionInfo *info)
{
  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));
  g_return_if_fail (GTK_IS_WIDGET (info->p->widget));

  picman_session_info_read_geometry (info, NULL /*cevent*/);

  if (PICMAN_IS_SESSION_MANAGED (info->p->widget))
    info->p->aux_info =
      picman_session_managed_get_aux_info (PICMAN_SESSION_MANAGED (info->p->widget));

  if (PICMAN_IS_DOCK_CONTAINER (info->p->widget))
    {
      PicmanDockContainer *dock_container = PICMAN_DOCK_CONTAINER (info->p->widget);
      GList             *iter           = NULL;
      GList             *docks;

      docks = picman_dock_container_get_docks (dock_container);

      for (iter = docks;
           iter;
           iter = g_list_next (iter))
        {
          PicmanDock *dock = PICMAN_DOCK (iter->data);

          info->p->docks =
            g_list_append (info->p->docks,
                           picman_session_info_dock_from_widget (dock));
        }

      g_list_free (docks);
    }
}

/**
 * picman_session_info_get_info_with_widget:
 * @info:
 * @widget: #GtkWidget to use
 *
 * Temporarily sets @widget on @info and calls
 * picman_session_info_get_info(), then restores the old widget that was
 * set.
 **/
void
picman_session_info_get_info_with_widget (PicmanSessionInfo *info,
                                        GtkWidget       *widget)
{
  GtkWidget *old_widget;

  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  old_widget = picman_session_info_get_widget (info);

  picman_session_info_set_widget (info, widget);
  picman_session_info_get_info (info);
  picman_session_info_set_widget (info, old_widget);
}

void
picman_session_info_clear_info (PicmanSessionInfo *info)
{
  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));

  if (info->p->aux_info)
    {
      g_list_free_full (info->p->aux_info,
                        (GDestroyNotify) picman_session_info_aux_free);
      info->p->aux_info = NULL;
    }

  if (info->p->docks)
    {
      g_list_free_full (info->p->docks,
                        (GDestroyNotify) picman_session_info_dock_free);
      info->p->docks = NULL;
    }
}

gboolean
picman_session_info_is_singleton (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return (! picman_session_info_is_for_dock_window (info) &&
          info->p->factory_entry &&
          info->p->factory_entry->singleton);
}

gboolean
picman_session_info_is_session_managed (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return (picman_session_info_is_for_dock_window (info) ||
          (info->p->factory_entry &&
           info->p->factory_entry->session_managed));
}


gboolean
picman_session_info_get_remember_size (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return (picman_session_info_is_for_dock_window (info) ||
          (info->p->factory_entry &&
           info->p->factory_entry->remember_size));
}

gboolean
picman_session_info_get_remember_if_open (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return (picman_session_info_is_for_dock_window (info) ||
          (info->p->factory_entry &&
           info->p->factory_entry->remember_if_open));
}

GtkWidget *
picman_session_info_get_widget (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return info->p->widget;
}

void
picman_session_info_set_widget (PicmanSessionInfo *info,
                              GtkWidget       *widget)
{
  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));

  if (GTK_IS_DIALOG (info->p->widget))
    g_signal_handlers_disconnect_by_func (info->p->widget,
                                          picman_session_info_dialog_show,
                                          info);

  info->p->widget = widget;
}

PicmanDialogFactoryEntry *
picman_session_info_get_factory_entry (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return info->p->factory_entry;
}

void
picman_session_info_set_factory_entry (PicmanSessionInfo        *info,
                                     PicmanDialogFactoryEntry *entry)
{
  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));

  info->p->factory_entry = entry;
}

gboolean
picman_session_info_get_open (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), FALSE);

  return info->p->open;
}

gint
picman_session_info_get_x (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), 0);

  return info->p->x;
}

gint
picman_session_info_get_y (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), 0);

  return info->p->y;
}

gint
picman_session_info_get_width (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), 0);

  return info->p->width;
}

gint
picman_session_info_get_height (PicmanSessionInfo *info)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO (info), 0);

  return info->p->height;
}

/**
 * picman_session_info_class_set_position_accuracy:
 * @accuracy:
 *
 * When writing sessionrc, make positions and sizes a multiple of
 * @accuracy. Meant to be used by test cases that does regression
 * testing on session managed window positions and sizes, to allow for
 * some deviations from the original setup, that the window manager
 * might impose.
 **/
void
picman_session_info_class_set_position_accuracy (PicmanSessionInfoClass *klass,
                                               gint                  accuracy)
{
  g_return_if_fail (PICMAN_IS_SESSION_INFO_CLASS (klass));

  klass->position_accuracy = accuracy;
}

/**
 * picman_session_info_class_apply_position_accuracy:
 * @position:
 *
 * Rounds @position to the nearest multiple of what was set with
 * picman_session_info_class_set_position_accuracy().
 *
 * Returns: Result.
 **/
gint
picman_session_info_class_apply_position_accuracy (PicmanSessionInfoClass *klass,
                                                 gint                  position)
{
  g_return_val_if_fail (PICMAN_IS_SESSION_INFO_CLASS (klass), position);

  if (klass->position_accuracy > 0)
    {
      gint to_floor = position + klass->position_accuracy / 2;

      return to_floor - to_floor % klass->position_accuracy;
    }

  return position;
}
