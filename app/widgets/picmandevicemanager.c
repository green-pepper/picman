/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandevicemanager.c
 * Copyright (C) 2011 Michael Natterer
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

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanmarshal.h"

#include "picmandeviceinfo.h"
#include "picmandevicemanager.h"


enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_CURRENT_DEVICE
};


typedef struct _PicmanDeviceManagerPrivate PicmanDeviceManagerPrivate;

struct _PicmanDeviceManagerPrivate
{
  Picman           *picman;
  PicmanDeviceInfo *current_device;
};

#define GET_PRIVATE(manager) \
        G_TYPE_INSTANCE_GET_PRIVATE (manager, \
                                     PICMAN_TYPE_DEVICE_MANAGER, \
                                     PicmanDeviceManagerPrivate)


static void   picman_device_manager_constructed    (GObject           *object);
static void   picman_device_manager_dispose        (GObject           *object);
static void   picman_device_manager_finalize       (GObject           *object);
static void   picman_device_manager_set_property   (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);
static void   picman_device_manager_get_property   (GObject           *object,
                                                  guint              property_id,
                                                  GValue            *value,
                                                  GParamSpec        *pspec);

static void   picman_device_manager_display_opened (GdkDisplayManager *disp_manager,
                                                  GdkDisplay        *display,
                                                  PicmanDeviceManager *manager);
static void   picman_device_manager_display_closed (GdkDisplay        *display,
                                                  gboolean           is_error,
                                                  PicmanDeviceManager *manager);

static void   picman_device_manager_device_added   (GdkDisplay        *gdk_display,
                                                  GdkDevice         *device,
                                                  PicmanDeviceManager *manager);
static void   picman_device_manager_device_removed (GdkDisplay        *gdk_display,
                                                  GdkDevice         *device,
                                                  PicmanDeviceManager *manager);


G_DEFINE_TYPE (PicmanDeviceManager, picman_device_manager, PICMAN_TYPE_LIST)

#define parent_class picman_device_manager_parent_class


static void
picman_device_manager_class_init (PicmanDeviceManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed        = picman_device_manager_constructed;
  object_class->dispose            = picman_device_manager_dispose;
  object_class->finalize           = picman_device_manager_finalize;
  object_class->set_property       = picman_device_manager_set_property;
  object_class->get_property       = picman_device_manager_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CURRENT_DEVICE,
                                   g_param_spec_object ("current-device",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DEVICE_INFO,
                                                        PICMAN_PARAM_STATIC_STRINGS |
                                                        G_PARAM_READABLE));

  g_type_class_add_private (object_class, sizeof (PicmanDeviceManagerPrivate));
}

static void
picman_device_manager_init (PicmanDeviceManager *manager)
{
}

static void
picman_device_manager_constructed (GObject *object)
{
  PicmanDeviceManager        *manager = PICMAN_DEVICE_MANAGER (object);
  PicmanDeviceManagerPrivate *private = GET_PRIVATE (object);
  GdkDisplayManager        *disp_manager;
  GSList                   *displays;
  GSList                   *list;
  GdkDisplay               *display;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (private->picman));

  disp_manager = gdk_display_manager_get ();

  displays = gdk_display_manager_list_displays (disp_manager);

  /*  present displays in the order in which they were opened  */
  displays = g_slist_reverse (displays);

  for (list = displays; list; list = g_slist_next (list))
    {
      picman_device_manager_display_opened (disp_manager, list->data, manager);
    }

  g_slist_free (displays);

  g_signal_connect (disp_manager, "display-opened",
                    G_CALLBACK (picman_device_manager_display_opened),
                    manager);

  display = gdk_display_get_default ();

  private->current_device =
    picman_device_info_get_by_device (gdk_display_get_core_pointer (display));
}

static void
picman_device_manager_dispose (GObject *object)
{
  g_signal_handlers_disconnect_by_func (gdk_display_manager_get (),
                                        picman_device_manager_display_opened,
                                        object);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_device_manager_finalize (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_device_manager_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanDeviceManagerPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      private->picman = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_device_manager_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanDeviceManagerPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, private->picman);
      break;

    case PROP_CURRENT_DEVICE:
      g_value_set_object (value, private->current_device);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

PicmanDeviceManager *
picman_device_manager_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_DEVICE_MANAGER,
                       "picman",          picman,
                       "children-type", PICMAN_TYPE_DEVICE_INFO,
                       "policy",        PICMAN_CONTAINER_POLICY_STRONG,
                       "unique-names",  FALSE,
                       "sort-func",     picman_device_info_compare,
                       NULL);
}

PicmanDeviceInfo *
picman_device_manager_get_current_device (PicmanDeviceManager *manager)
{
  g_return_val_if_fail (PICMAN_IS_DEVICE_MANAGER (manager), NULL);

  return GET_PRIVATE (manager)->current_device;
}

void
picman_device_manager_set_current_device (PicmanDeviceManager *manager,
                                        PicmanDeviceInfo    *info)
{
  PicmanDeviceManagerPrivate *private;
  PicmanContext              *user_context;

  g_return_if_fail (PICMAN_IS_DEVICE_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_DEVICE_INFO (info));

  private = GET_PRIVATE (manager);

  picman_context_set_parent (PICMAN_CONTEXT (private->current_device), NULL);

  private->current_device = info;

  user_context = picman_get_user_context (private->picman);

  picman_context_copy_properties (PICMAN_CONTEXT (info), user_context,
                                PICMAN_DEVICE_INFO_CONTEXT_MASK);
  picman_context_set_parent (PICMAN_CONTEXT (info), user_context);

  g_object_notify (G_OBJECT (manager), "current-device");
}


/*  private functions  */


static void
picman_device_manager_display_opened (GdkDisplayManager *disp_manager,
                                    GdkDisplay        *gdk_display,
                                    PicmanDeviceManager *manager)
{
  GList *list;

  /*  create device info structures for present devices */
  for (list = gdk_display_list_devices (gdk_display); list; list = list->next)
    {
      GdkDevice *device = list->data;

      picman_device_manager_device_added (gdk_display, device, manager);
    }

  g_signal_connect (gdk_display, "closed",
                    G_CALLBACK (picman_device_manager_display_closed),
                    manager);
}

static void
picman_device_manager_display_closed (GdkDisplay        *gdk_display,
                                    gboolean           is_error,
                                    PicmanDeviceManager *manager)
{
  GList *list;

  for (list = gdk_display_list_devices (gdk_display); list; list = list->next)
    {
      GdkDevice *device = list->data;

      picman_device_manager_device_removed (gdk_display, device, manager);
    }
}

static void
picman_device_manager_device_added (GdkDisplay        *gdk_display,
                                  GdkDevice         *device,
                                  PicmanDeviceManager *manager)
{
  PicmanDeviceManagerPrivate *private = GET_PRIVATE (manager);
  PicmanDeviceInfo           *device_info;

  device_info =
    PICMAN_DEVICE_INFO (picman_container_get_child_by_name (PICMAN_CONTAINER (manager),
                                                        device->name));

  if (device_info)
    {
      picman_device_info_set_device (device_info, device, gdk_display);
    }
  else
    {
      device_info = picman_device_info_new (private->picman, device, gdk_display);

      picman_device_info_set_default_tool (device_info);

      picman_container_add (PICMAN_CONTAINER (manager), PICMAN_OBJECT (device_info));
      g_object_unref (device_info);
    }
}

static void
picman_device_manager_device_removed (GdkDisplay        *gdk_display,
                                    GdkDevice         *device,
                                    PicmanDeviceManager *manager)
{
  PicmanDeviceManagerPrivate *private = GET_PRIVATE (manager);
  PicmanDeviceInfo           *device_info;

  device_info =
    PICMAN_DEVICE_INFO (picman_container_get_child_by_name (PICMAN_CONTAINER (manager),
                                                        device->name));

  if (device_info)
    {
      picman_device_info_set_device (device_info, NULL, NULL);

      if (device_info == private->current_device)
        {
          device      = gdk_display_get_core_pointer (gdk_display);
          device_info = picman_device_info_get_by_device (device);

          picman_device_manager_set_current_device (manager, device_info);
        }
    }
}
