/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
 *
 * picmanpluginmanager-history.c
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "plug-in-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"

#include "picmanpluginmanager.h"
#include "picmanpluginmanager-history.h"
#include "picmanpluginprocedure.h"


guint
picman_plug_in_manager_history_size (PicmanPlugInManager *manager)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), 0);

  return MAX (1, manager->picman->config->plug_in_history_size);
}

guint
picman_plug_in_manager_history_length (PicmanPlugInManager *manager)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), 0);

  return g_slist_length (manager->history);
}

PicmanPlugInProcedure *
picman_plug_in_manager_history_nth (PicmanPlugInManager *manager,
                                      guint              n)
{
  g_return_val_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager), NULL);

  return g_slist_nth_data (manager->history, n);
}

void
picman_plug_in_manager_history_add (PicmanPlugInManager   *manager,
                                  PicmanPlugInProcedure *procedure)
{
  GSList *list;
  gint    history_size;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (procedure));

  /* return early if the procedure is already at the top */
  if (manager->history && manager->history->data == procedure)
    return;

  history_size = picman_plug_in_manager_history_size (manager);

  manager->history = g_slist_remove (manager->history, procedure);
  manager->history = g_slist_prepend (manager->history, procedure);

  list = g_slist_nth (manager->history, history_size);

  if (list)
    {
      manager->history = g_slist_remove_link (manager->history, list);
      g_slist_free (list);
    }

  picman_plug_in_manager_history_changed (manager);
}

void
picman_plug_in_manager_history_remove (PicmanPlugInManager   *manager,
                                     PicmanPlugInProcedure *procedure)
{
  GSList *link;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (procedure));

  link = g_slist_find (manager->history, procedure);

  if (link)
    {
      manager->history = g_slist_delete_link (manager->history, link);

      picman_plug_in_manager_history_changed (manager);
    }
}

void
picman_plug_in_manager_history_clear (PicmanPlugInManager   *manager)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));

  if (manager->history)
    {
      g_slist_free (manager->history);
      manager->history = NULL;

      picman_plug_in_manager_history_changed (manager);
    }
}


