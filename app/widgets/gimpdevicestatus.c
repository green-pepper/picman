/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmandevicestatus.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#undef GSEAL_ENABLE

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmandatafactory.h"
#include "core/picmangradient.h"
#include "core/picmanlist.h"
#include "core/picmanpattern.h"
#include "core/picmantoolinfo.h"

#include "picmandnd.h"
#include "picmandeviceinfo.h"
#include "picmandevicemanager.h"
#include "picmandevices.h"
#include "picmandevicestatus.h"
#include "picmandialogfactory.h"
#include "picmanpropwidgets.h"
#include "picmanview.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


#define CELL_SIZE 20 /* The size of the view cells */


enum
{
  PROP_0,
  PROP_PICMAN
};


struct _PicmanDeviceStatusEntry
{
  PicmanDeviceInfo *device_info;

  GtkWidget      *ebox;
  GtkWidget      *tool;
  GtkWidget      *foreground;
  GtkWidget      *background;
  GtkWidget      *brush;
  GtkWidget      *pattern;
  GtkWidget      *gradient;
};


static void picman_device_status_constructed     (GObject               *object);
static void picman_device_status_dispose         (GObject               *object);
static void picman_device_status_set_property    (GObject               *object,
                                                guint                  property_id,
                                                const GValue          *value,
                                                GParamSpec            *pspec);

static void picman_device_status_device_add      (PicmanContainer         *devices,
                                                PicmanDeviceInfo        *device_info,
                                                PicmanDeviceStatus      *status);
static void picman_device_status_device_remove   (PicmanContainer         *devices,
                                                PicmanDeviceInfo        *device_info,
                                                PicmanDeviceStatus      *status);

static void picman_device_status_notify_device   (PicmanDeviceManager     *manager,
                                                const GParamSpec      *pspec,
                                                PicmanDeviceStatus      *status);
static void picman_device_status_update_entry    (PicmanDeviceInfo        *device_info,
                                                PicmanDeviceStatusEntry *entry);
static void picman_device_status_save_clicked    (GtkWidget             *button,
                                                PicmanDeviceStatus      *status);
static void picman_device_status_view_clicked    (GtkWidget             *widget,
                                                GdkModifierType        state,
                                                const gchar           *identifier);


G_DEFINE_TYPE (PicmanDeviceStatus, picman_device_status, PICMAN_TYPE_EDITOR)

#define parent_class picman_device_status_parent_class


static void
picman_device_status_class_init (PicmanDeviceStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_device_status_constructed;
  object_class->dispose      = picman_device_status_dispose;
  object_class->set_property = picman_device_status_set_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_device_status_init (PicmanDeviceStatus *status)
{
  status->picman           = NULL;
  status->current_device = NULL;

  status->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (status->vbox), 2);
  gtk_box_pack_start (GTK_BOX (status), status->vbox, TRUE, TRUE, 0);
  gtk_widget_show (status->vbox);

  status->save_button =
    picman_editor_add_button (PICMAN_EDITOR (status), GTK_STOCK_SAVE,
                            _("Save device status"), NULL,
                            G_CALLBACK (picman_device_status_save_clicked),
                            NULL,
                            status);
}

static void
picman_device_status_constructed (GObject *object)
{
  PicmanDeviceStatus *status = PICMAN_DEVICE_STATUS (object);
  PicmanContainer    *devices;
  GList            *list;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (status->picman));

  devices = PICMAN_CONTAINER (picman_devices_get_manager (status->picman));

  for (list = PICMAN_LIST (devices)->list; list; list = list->next)
    picman_device_status_device_add (devices, list->data, status);

  g_signal_connect_object (devices, "add",
                           G_CALLBACK (picman_device_status_device_add),
                           status, 0);
  g_signal_connect_object (devices, "remove",
                           G_CALLBACK (picman_device_status_device_remove),
                           status, 0);

  g_signal_connect (devices, "notify::current-device",
                    G_CALLBACK (picman_device_status_notify_device),
                    status);

  picman_device_status_notify_device (PICMAN_DEVICE_MANAGER (devices), NULL, status);
}

static void
picman_device_status_dispose (GObject *object)
{
  PicmanDeviceStatus *status = PICMAN_DEVICE_STATUS (object);

  if (status->devices)
    {
      GList *list;

      for (list = status->devices; list; list = list->next)
        {
          PicmanDeviceStatusEntry *entry = list->data;

          g_signal_handlers_disconnect_by_func (entry->device_info,
                                                picman_device_status_update_entry,
                                                entry);

          g_slice_free (PicmanDeviceStatusEntry, entry);
        }

      g_list_free (status->devices);
      status->devices = NULL;

      g_signal_handlers_disconnect_by_func (picman_devices_get_manager (status->picman),
                                            picman_device_status_notify_device,
                                            status);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_device_status_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanDeviceStatus *status = PICMAN_DEVICE_STATUS (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      status->picman = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_status_device_add (PicmanContainer    *devices,
                               PicmanDeviceInfo   *device_info,
                               PicmanDeviceStatus *status)
{
  PicmanContext           *context = PICMAN_CONTEXT (device_info);
  PicmanDeviceStatusEntry *entry;
  GClosure              *closure;
  GtkWidget             *vbox;
  GtkWidget             *hbox;
  GtkWidget             *label;
  gchar                 *name;

  entry = g_slice_new0 (PicmanDeviceStatusEntry);

  status->devices = g_list_prepend (status->devices, entry);

  entry->device_info = device_info;

  closure = g_cclosure_new (G_CALLBACK (picman_device_status_update_entry),
                            entry, NULL);
  g_object_watch_closure (G_OBJECT (status), closure);
  g_signal_connect_closure (device_info, "changed", closure, FALSE);

  entry->ebox = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX (status->vbox), entry->ebox,
                      FALSE, FALSE, 0);
  gtk_widget_show (entry->ebox);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
  gtk_container_add (GTK_CONTAINER (entry->ebox), vbox);
  gtk_widget_show (vbox);

  /*  the device name  */

  if (device_info->display == NULL ||
      device_info->display == gdk_display_get_default ())
    name = g_strdup (picman_object_get_name (device_info));
  else
    name = g_strdup_printf ("%s (%s)",
                            picman_object_get_name (device_info),
                            gdk_display_get_name (device_info->display));

  label = gtk_label_new (name);
  g_free (name);

  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  picman_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  /*  the row of properties  */

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  the tool  */

  entry->tool = picman_prop_view_new (G_OBJECT (context), "tool",
                                    context, CELL_SIZE);
  gtk_box_pack_start (GTK_BOX (hbox), entry->tool, FALSE, FALSE, 0);
  gtk_widget_show (entry->tool);

  /*  the foreground color  */

  entry->foreground = picman_prop_color_area_new (G_OBJECT (context),
                                                "foreground",
                                                CELL_SIZE, CELL_SIZE,
                                                PICMAN_COLOR_AREA_FLAT);
  gtk_widget_add_events (entry->foreground,
                         GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_box_pack_start (GTK_BOX (hbox), entry->foreground, FALSE, FALSE, 0);
  gtk_widget_show (entry->foreground);

  /*  the background color  */

  entry->background = picman_prop_color_area_new (G_OBJECT (context),
                                                "background",
                                                CELL_SIZE, CELL_SIZE,
                                                PICMAN_COLOR_AREA_FLAT);
  gtk_widget_add_events (entry->background,
                         GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
  gtk_box_pack_start (GTK_BOX (hbox), entry->background, FALSE, FALSE, 0);
  gtk_widget_show (entry->background);

  /*  the brush  */

  entry->brush = picman_prop_view_new (G_OBJECT (context), "brush",
                                     context, CELL_SIZE);
  PICMAN_VIEW (entry->brush)->clickable  = TRUE;
  PICMAN_VIEW (entry->brush)->show_popup = TRUE;
  gtk_box_pack_start (GTK_BOX (hbox), entry->brush, FALSE, FALSE, 0);
  gtk_widget_show (entry->brush);

  g_signal_connect (entry->brush, "clicked",
                    G_CALLBACK (picman_device_status_view_clicked),
                    "picman-brush-grid|picman-brush-list");

  /*  the pattern  */

  entry->pattern = picman_prop_view_new (G_OBJECT (context), "pattern",
                                       context, CELL_SIZE);
  PICMAN_VIEW (entry->pattern)->clickable  = TRUE;
  PICMAN_VIEW (entry->pattern)->show_popup = TRUE;
  gtk_box_pack_start (GTK_BOX (hbox), entry->pattern, FALSE, FALSE, 0);
  gtk_widget_show (entry->pattern);

  g_signal_connect (entry->pattern, "clicked",
                    G_CALLBACK (picman_device_status_view_clicked),
                    "picman-pattern-grid|picman-pattern-list");

  /*  the gradient  */

  entry->gradient = picman_prop_view_new (G_OBJECT (context), "gradient",
                                        context, 2 * CELL_SIZE);
  PICMAN_VIEW (entry->gradient)->clickable  = TRUE;
  PICMAN_VIEW (entry->gradient)->show_popup = TRUE;
  gtk_box_pack_start (GTK_BOX (hbox), entry->gradient, FALSE, FALSE, 0);
  gtk_widget_show (entry->gradient);

  g_signal_connect (entry->gradient, "clicked",
                    G_CALLBACK (picman_device_status_view_clicked),
                    "picman-gradient-list|picman-gradient-grid");

  picman_device_status_update_entry (device_info, entry);
}

static void
picman_device_status_device_remove (PicmanContainer    *devices,
                                  PicmanDeviceInfo   *device_info,
                                  PicmanDeviceStatus *status)
{
  GList *list;

  for (list = status->devices; list; list = list->next)
    {
      PicmanDeviceStatusEntry *entry = list->data;

      if (entry->device_info == device_info)
        {
          status->devices = g_list_remove (status->devices, entry);

          g_signal_handlers_disconnect_by_func (entry->device_info,
                                                picman_device_status_update_entry,
                                                entry);

          g_slice_free (PicmanDeviceStatusEntry, entry);

          return;
        }
    }
}

GtkWidget *
picman_device_status_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_DEVICE_STATUS,
                       "picman", picman,
                       NULL);
}


/*  private functions  */

static void
picman_device_status_notify_device (PicmanDeviceManager *manager,
                                  const GParamSpec  *pspec,
                                  PicmanDeviceStatus  *status)
{
  GList *list;

  status->current_device = picman_device_manager_get_current_device (manager);

  for (list = status->devices; list; list = list->next)
    {
      PicmanDeviceStatusEntry *entry = list->data;

      gtk_widget_set_state (entry->ebox,
                            entry->device_info == status->current_device ?
                            GTK_STATE_SELECTED : GTK_STATE_NORMAL);
    }
}

static void
picman_device_status_update_entry (PicmanDeviceInfo        *device_info,
                                 PicmanDeviceStatusEntry *entry)
{
  if (! picman_device_info_get_device (device_info, NULL) ||
      picman_device_info_get_mode (device_info) == GDK_MODE_DISABLED)
    {
      gtk_widget_hide (entry->ebox);
    }
  else
    {
      PicmanContext *context = PICMAN_CONTEXT (device_info);
      PicmanRGB      color;
      guchar       r, g, b;
      gchar        buf[64];

      picman_context_get_foreground (context, &color);
      picman_rgb_get_uchar (&color, &r, &g, &b);
      g_snprintf (buf, sizeof (buf), _("Foreground: %d, %d, %d"), r, g, b);
      picman_help_set_help_data (entry->foreground, buf, NULL);

      picman_context_get_background (context, &color);
      picman_rgb_get_uchar (&color, &r, &g, &b);
      g_snprintf (buf, sizeof (buf), _("Background: %d, %d, %d"), r, g, b);
      picman_help_set_help_data (entry->background, buf, NULL);

      gtk_widget_show (entry->ebox);
    }
}

static void
picman_device_status_save_clicked (GtkWidget        *button,
                                 PicmanDeviceStatus *status)
{
  picman_devices_save (status->picman, TRUE);
}

static void
picman_device_status_view_clicked (GtkWidget       *widget,
                                 GdkModifierType  state,
                                 const gchar     *identifier)
{
  PicmanDeviceStatus  *status;
  PicmanDialogFactory *dialog_factory;

  status = PICMAN_DEVICE_STATUS (gtk_widget_get_ancestor (widget,
                                                        PICMAN_TYPE_DEVICE_STATUS));
  dialog_factory = picman_dialog_factory_get_singleton ();

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (status->picman)),
                                             status->picman,
                                             dialog_factory,
                                             gtk_widget_get_screen (widget),
                                             identifier);
}
