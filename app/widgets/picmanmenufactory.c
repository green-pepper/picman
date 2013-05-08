/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmenufactory.c
 * Copyright (C) 2001-2004 Michael Natterer <mitch@picman.org>
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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"

#include "picmanactionfactory.h"
#include "picmanmenufactory.h"
#include "picmanuimanager.h"


struct _PicmanMenuFactoryPrivate
{
  Picman              *picman;
  PicmanActionFactory *action_factory;
  GList             *registered_menus;
};


static void   picman_menu_factory_finalize (GObject *object);


G_DEFINE_TYPE (PicmanMenuFactory, picman_menu_factory, PICMAN_TYPE_OBJECT)

#define parent_class picman_menu_factory_parent_class


static void
picman_menu_factory_class_init (PicmanMenuFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_menu_factory_finalize;

  g_type_class_add_private (klass, sizeof (PicmanMenuFactoryPrivate));
}

static void
picman_menu_factory_init (PicmanMenuFactory *factory)
{
  factory->p = G_TYPE_INSTANCE_GET_PRIVATE (factory,
                                            PICMAN_TYPE_MENU_FACTORY,
                                            PicmanMenuFactoryPrivate);
}

static void
picman_menu_factory_finalize (GObject *object)
{
  PicmanMenuFactory *factory = PICMAN_MENU_FACTORY (object);
  GList           *list;

  for (list = factory->p->registered_menus; list; list = g_list_next (list))
    {
      PicmanMenuFactoryEntry *entry = list->data;
      GList                *uis;

      g_free (entry->identifier);

      g_list_free_full (entry->action_groups, (GDestroyNotify) g_free);

      for (uis = entry->managed_uis; uis; uis = g_list_next (uis))
        {
          PicmanUIManagerUIEntry *ui_entry = uis->data;

          g_free (ui_entry->ui_path);
          g_free (ui_entry->basename);

          g_slice_free (PicmanUIManagerUIEntry, ui_entry);
        }

      g_list_free (entry->managed_uis);

      g_slice_free (PicmanMenuFactoryEntry, entry);
    }

  g_list_free (factory->p->registered_menus);
  factory->p->registered_menus = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanMenuFactory *
picman_menu_factory_new (Picman              *picman,
                       PicmanActionFactory *action_factory)
{
  PicmanMenuFactory *factory;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_ACTION_FACTORY (action_factory), NULL);

  factory = g_object_new (PICMAN_TYPE_MENU_FACTORY, NULL);

  factory->p->picman           = picman;
  factory->p->action_factory = action_factory;

  return factory;
}

void
picman_menu_factory_manager_register (PicmanMenuFactory *factory,
                                    const gchar     *identifier,
                                    const gchar     *first_group,
                                    ...)
{
  PicmanMenuFactoryEntry *entry;
  const gchar          *group;
  const gchar          *ui_path;
  va_list               args;

  g_return_if_fail (PICMAN_IS_MENU_FACTORY (factory));
  g_return_if_fail (identifier != NULL);
  g_return_if_fail (first_group != NULL);

  entry = g_slice_new0 (PicmanMenuFactoryEntry);

  entry->identifier = g_strdup (identifier);

  factory->p->registered_menus = g_list_prepend (factory->p->registered_menus, entry);

  va_start (args, first_group);

  for (group = first_group;
       group;
       group = va_arg (args, const gchar *))
    {
      entry->action_groups = g_list_prepend (entry->action_groups,
                                             g_strdup (group));
    }

  entry->action_groups = g_list_reverse (entry->action_groups);

  ui_path = va_arg (args, const gchar *);

  while (ui_path)
    {
      const gchar            *ui_basename;
      PicmanUIManagerSetupFunc  setup_func;
      PicmanUIManagerUIEntry   *ui_entry;

      ui_basename = va_arg (args, const gchar *);
      setup_func  = va_arg (args, PicmanUIManagerSetupFunc);

      ui_entry = g_slice_new0 (PicmanUIManagerUIEntry);

      ui_entry->ui_path    = g_strdup (ui_path);
      ui_entry->basename   = g_strdup (ui_basename);
      ui_entry->setup_func = setup_func;

      entry->managed_uis = g_list_prepend (entry->managed_uis, ui_entry);

      ui_path = va_arg (args, const gchar *);
    }

  entry->managed_uis = g_list_reverse (entry->managed_uis);

  va_end (args);
}

GList *
picman_menu_factory_get_registered_menus (PicmanMenuFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (factory), NULL);

  return factory->p->registered_menus;
}

PicmanUIManager *
picman_menu_factory_manager_new (PicmanMenuFactory *factory,
                               const gchar     *identifier,
                               gpointer         callback_data,
                               gboolean         create_tearoff)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (factory), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  for (list = factory->p->registered_menus; list; list = g_list_next (list))
    {
      PicmanMenuFactoryEntry *entry = list->data;

      if (! strcmp (entry->identifier, identifier))
        {
          PicmanUIManager *manager;
          GtkAccelGroup *accel_group;
          GList         *list;

          manager = picman_ui_manager_new (factory->p->picman, entry->identifier);
          gtk_ui_manager_set_add_tearoffs (GTK_UI_MANAGER (manager),
                                           create_tearoff);

          accel_group = gtk_ui_manager_get_accel_group (GTK_UI_MANAGER (manager));

          for (list = entry->action_groups; list; list = g_list_next (list))
            {
              PicmanActionGroup *group;
              GList           *actions;
              GList           *list2;

              group = picman_action_factory_group_new (factory->p->action_factory,
                                                     (const gchar *) list->data,
                                                     callback_data);

              actions = gtk_action_group_list_actions (GTK_ACTION_GROUP (group));

              for (list2 = actions; list2; list2 = g_list_next (list2))
                {
                  GtkAction *action = list2->data;

                  gtk_action_set_accel_group (action, accel_group);
                  gtk_action_connect_accelerator (action);
                }

              g_list_free (actions);

              gtk_ui_manager_insert_action_group (GTK_UI_MANAGER (manager),
                                                  GTK_ACTION_GROUP (group),
                                                  -1);

              g_object_unref (group);
            }

          for (list = entry->managed_uis; list; list = g_list_next (list))
            {
              PicmanUIManagerUIEntry *ui_entry = list->data;

              picman_ui_manager_ui_register (manager,
                                           ui_entry->ui_path,
                                           ui_entry->basename,
                                           ui_entry->setup_func);
            }

          return manager;
        }
    }

  g_warning ("%s: no entry registered for \"%s\"",
             G_STRFUNC, identifier);

  return NULL;
}
