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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#define PICMAN_ENABLE_CONTROLLER_UNDER_CONSTRUCTION
#include "libpicmanwidgets/picmancontroller.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanlist.h"

#include "picmancontrollerinfo.h"
#include "picmancontrollers.h"
#include "picmancontrollerkeyboard.h"
#include "picmancontrollermouse.h"
#include "picmancontrollerwheel.h"
#include "picmanenumaction.h"
#include "picmanuimanager.h"

#include "picman-intl.h"


#define PICMAN_CONTROLLER_MANAGER_DATA_KEY "picman-controller-manager"


typedef struct _PicmanControllerManager PicmanControllerManager;

struct _PicmanControllerManager
{
  PicmanContainer  *controllers;
  GQuark          event_mapped_id;
  PicmanController *mouse;
  PicmanController *wheel;
  PicmanController *keyboard;
  PicmanUIManager  *ui_manager;
};


/*  local function prototypes  */

static PicmanControllerManager * picman_controller_manager_get  (Picman *picman);
static void   picman_controller_manager_free (PicmanControllerManager *manager);

static void   picman_controllers_add         (PicmanContainer         *container,
                                            PicmanControllerInfo    *info,
                                            PicmanControllerManager *manager);
static void   picman_controllers_remove      (PicmanContainer         *container,
                                            PicmanControllerInfo    *info,
                                            PicmanControllerManager *manager);

static gboolean picman_controllers_event_mapped (PicmanControllerInfo        *info,
                                               PicmanController            *controller,
                                               const PicmanControllerEvent *event,
                                               const gchar               *action_name,
                                               PicmanControllerManager     *manager);


/*  public functions  */

void
picman_controllers_init (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (picman_controller_manager_get (picman) == NULL);

  manager = g_slice_new0 (PicmanControllerManager);

  g_object_set_data_full (G_OBJECT (picman),
                          PICMAN_CONTROLLER_MANAGER_DATA_KEY, manager,
                          (GDestroyNotify) picman_controller_manager_free);

  manager->controllers = picman_list_new (PICMAN_TYPE_CONTROLLER_INFO, TRUE);

  g_signal_connect (manager->controllers, "add",
                    G_CALLBACK (picman_controllers_add),
                    manager);
  g_signal_connect (manager->controllers, "remove",
                    G_CALLBACK (picman_controllers_remove),
                    manager);

  manager->event_mapped_id =
    picman_container_add_handler (manager->controllers, "event-mapped",
                                G_CALLBACK (picman_controllers_event_mapped),
                                manager);

  g_type_class_ref (PICMAN_TYPE_CONTROLLER_MOUSE);
  g_type_class_ref (PICMAN_TYPE_CONTROLLER_WHEEL);
  g_type_class_ref (PICMAN_TYPE_CONTROLLER_KEYBOARD);
}

void
picman_controllers_exit (Picman *picman)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (picman_controller_manager_get (picman) != NULL);

  g_object_set_data (G_OBJECT (picman), PICMAN_CONTROLLER_MANAGER_DATA_KEY, NULL);

  g_type_class_unref (g_type_class_peek (PICMAN_TYPE_CONTROLLER_WHEEL));
  g_type_class_unref (g_type_class_peek (PICMAN_TYPE_CONTROLLER_KEYBOARD));
}

void
picman_controllers_restore (Picman          *picman,
                          PicmanUIManager *ui_manager)
{
  PicmanControllerManager *manager;
  gchar                 *filename;
  GError                *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_UI_MANAGER (ui_manager));

  manager = picman_controller_manager_get (picman);

  g_return_if_fail (manager != NULL);
  g_return_if_fail (manager->ui_manager == NULL);

  manager->ui_manager = g_object_ref (ui_manager);

  filename = picman_personal_rc_file ("controllerrc");

  if (picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_deserialize_file (PICMAN_CONFIG (manager->controllers),
                                      filename, NULL, &error))
    {
      if (error->code == PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        {
          g_clear_error (&error);
          g_free (filename);

          filename = g_build_filename (picman_sysconf_directory (),
                                       "controllerrc", NULL);

          if (! picman_config_deserialize_file (PICMAN_CONFIG (manager->controllers),
                                              filename, NULL, &error))
            {
              picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR,
				    error->message);
            }
        }
      else
        {
          picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
        }

      g_clear_error (&error);
    }

  picman_list_reverse (PICMAN_LIST (manager->controllers));

  g_free (filename);
}

void
picman_controllers_save (Picman *picman)
{
  const gchar *header =
    "PICMAN controllerrc\n"
    "\n"
    "This file will be entirely rewritten each time you exit.";
  const gchar *footer =
    "end of controllerrc";

  PicmanControllerManager *manager;
  gchar                 *filename;
  GError                *error = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  manager = picman_controller_manager_get (picman);

  g_return_if_fail (manager != NULL);

  filename = picman_personal_rc_file ("controllerrc");

  if (picman->be_verbose)
    g_print ("Writing '%s'\n", picman_filename_to_utf8 (filename));

  if (! picman_config_serialize_to_file (PICMAN_CONFIG (manager->controllers),
                                       filename,
                                       header, footer, NULL,
                                       &error))
    {
      picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR, error->message);
      g_error_free (error);
    }

  g_free (filename);
}

PicmanContainer *
picman_controllers_get_list (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = picman_controller_manager_get (picman);

  g_return_val_if_fail (manager != NULL, NULL);

  return manager->controllers;
}

PicmanUIManager *
picman_controllers_get_ui_manager (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = picman_controller_manager_get (picman);

  g_return_val_if_fail (manager != NULL, NULL);

  return manager->ui_manager;
}

PicmanController *
picman_controllers_get_mouse (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = picman_controller_manager_get (picman);

  g_return_val_if_fail (manager != NULL, NULL);

  return manager->mouse;
}

PicmanController *
picman_controllers_get_wheel (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = picman_controller_manager_get (picman);

  g_return_val_if_fail (manager != NULL, NULL);

  return manager->wheel;
}

PicmanController *
picman_controllers_get_keyboard (Picman *picman)
{
  PicmanControllerManager *manager;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  manager = picman_controller_manager_get (picman);

  g_return_val_if_fail (manager != NULL, NULL);

  return manager->keyboard;
}


/*  private functions  */

static PicmanControllerManager *
picman_controller_manager_get (Picman *picman)
{
  return g_object_get_data (G_OBJECT (picman), PICMAN_CONTROLLER_MANAGER_DATA_KEY);
}

static void
picman_controller_manager_free (PicmanControllerManager *manager)
{
  picman_container_remove_handler (manager->controllers,
                                 manager->event_mapped_id);

  g_object_unref (manager->controllers);
  g_object_unref (manager->ui_manager);

  g_slice_free (PicmanControllerManager, manager);
}

static void
picman_controllers_add (PicmanContainer         *container,
                      PicmanControllerInfo    *info,
                      PicmanControllerManager *manager)
{
  if (PICMAN_IS_CONTROLLER_WHEEL (info->controller))
    manager->wheel = info->controller;
  else if (PICMAN_IS_CONTROLLER_KEYBOARD (info->controller))
    manager->keyboard = info->controller;
  else if (PICMAN_IS_CONTROLLER_MOUSE (info->controller))
    manager->mouse = info->controller;
}

static void
picman_controllers_remove (PicmanContainer         *container,
                         PicmanControllerInfo    *info,
                         PicmanControllerManager *manager)
{
  if (info->controller == manager->wheel)
    manager->wheel = NULL;
  else if (info->controller == manager->keyboard)
    manager->keyboard = NULL;
}

static gboolean
picman_controllers_event_mapped (PicmanControllerInfo        *info,
                               PicmanController            *controller,
                               const PicmanControllerEvent *event,
                               const gchar               *action_name,
                               PicmanControllerManager     *manager)
{
  GtkUIManager *ui_manager = GTK_UI_MANAGER (manager->ui_manager);
  GList        *list;

  for (list = gtk_ui_manager_get_action_groups (ui_manager);
       list;
       list = g_list_next (list))
    {
      GtkActionGroup *group = list->data;
      GtkAction      *action;

      action = gtk_action_group_get_action (group, action_name);

      if (action)
        {
          switch (event->type)
            {
            case PICMAN_CONTROLLER_EVENT_VALUE:
              if (G_VALUE_HOLDS_DOUBLE (&event->value.value) &&
                  PICMAN_IS_ENUM_ACTION (action)               &&
                  PICMAN_ENUM_ACTION (action)->value_variable)
                {
                  gdouble value = g_value_get_double (&event->value.value);

                  picman_enum_action_selected (PICMAN_ENUM_ACTION (action),
                                             value * 1000);

                  break;
                }
              /* else fallthru */

            case PICMAN_CONTROLLER_EVENT_TRIGGER:
            default:
              gtk_action_activate (action);
              break;
            }

          return TRUE;
        }
    }

  return FALSE;
}
