/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanactionfactory.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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
#include "picmanactiongroup.h"


static void   picman_action_factory_finalize (GObject *object);


G_DEFINE_TYPE (PicmanActionFactory, picman_action_factory, PICMAN_TYPE_OBJECT)

#define parent_class picman_action_factory_parent_class


static void
picman_action_factory_class_init (PicmanActionFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_action_factory_finalize;
}

static void
picman_action_factory_init (PicmanActionFactory *factory)
{
  factory->picman              = NULL;
  factory->registered_groups = NULL;
}

static void
picman_action_factory_finalize (GObject *object)
{
  PicmanActionFactory *factory = PICMAN_ACTION_FACTORY (object);
  GList             *list;

  for (list = factory->registered_groups; list; list = g_list_next (list))
    {
      PicmanActionFactoryEntry *entry = list->data;

      g_free (entry->identifier);
      g_free (entry->label);
      g_free (entry->stock_id);

      g_slice_free (PicmanActionFactoryEntry, entry);
    }

  g_list_free (factory->registered_groups);
  factory->registered_groups = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanActionFactory *
picman_action_factory_new (Picman *picman)
{
  PicmanActionFactory *factory;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  factory = g_object_new (PICMAN_TYPE_ACTION_FACTORY, NULL);

  factory->picman = picman;

  return factory;
}

void
picman_action_factory_group_register (PicmanActionFactory         *factory,
                                    const gchar               *identifier,
                                    const gchar               *label,
                                    const gchar               *stock_id,
                                    PicmanActionGroupSetupFunc   setup_func,
                                    PicmanActionGroupUpdateFunc  update_func)
{
  PicmanActionFactoryEntry *entry;

  g_return_if_fail (PICMAN_IS_ACTION_FACTORY (factory));
  g_return_if_fail (identifier != NULL);
  g_return_if_fail (label != NULL);
  g_return_if_fail (setup_func != NULL);
  g_return_if_fail (update_func != NULL);

  entry = g_slice_new0 (PicmanActionFactoryEntry);

  entry->identifier  = g_strdup (identifier);
  entry->label       = g_strdup (label);
  entry->stock_id    = g_strdup (stock_id);
  entry->setup_func  = setup_func;
  entry->update_func = update_func;

  factory->registered_groups = g_list_prepend (factory->registered_groups,
                                               entry);
}

PicmanActionGroup *
picman_action_factory_group_new (PicmanActionFactory *factory,
                               const gchar       *identifier,
                               gpointer           user_data)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_ACTION_FACTORY (factory), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  for (list = factory->registered_groups; list; list = g_list_next (list))
    {
      PicmanActionFactoryEntry *entry = list->data;

      if (! strcmp (entry->identifier, identifier))
        {
          PicmanActionGroup *group;

          group = picman_action_group_new (factory->picman,
                                         entry->identifier,
                                         entry->label,
                                         entry->stock_id,
                                         user_data,
                                         entry->update_func);

          if (entry->setup_func)
            entry->setup_func (group);

          return group;
        }
    }

  g_warning ("%s: no entry registered for \"%s\"",
             G_STRFUNC, identifier);

  return NULL;
}
