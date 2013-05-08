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

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"

#include "picmancontainereditor.h"
#include "picmancontainerview.h"
#include "picmancontainerview-utils.h"
#include "picmandockable.h"


/*  public functions  */

PicmanContainerView *
picman_container_view_get_by_dockable (PicmanDockable *dockable)
{
  GtkWidget *child;

  g_return_val_if_fail (PICMAN_IS_DOCKABLE (dockable), NULL);

  child = gtk_bin_get_child (GTK_BIN (dockable));

  if (child)
    {
      if (PICMAN_IS_CONTAINER_EDITOR (child))
        {
          return PICMAN_CONTAINER_EDITOR (child)->view;
        }
      else if (PICMAN_IS_CONTAINER_VIEW (child))
        {
          return PICMAN_CONTAINER_VIEW (child);
        }
    }

  return NULL;
}

void
picman_container_view_remove_active (PicmanContainerView *view)
{
  PicmanContext   *context;
  PicmanContainer *container;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));

  context   = picman_container_view_get_context (view);
  container = picman_container_view_get_container (view);

  if (context && container)
    {
      GType       children_type;
      PicmanObject *active;

      children_type = picman_container_get_children_type (container);

      active = picman_context_get_by_type (context, children_type);

      if (active)
        {
          PicmanObject *new;

          new = picman_container_get_neighbor_of (container, active);

          if (new)
            picman_context_set_by_type (context, children_type, new);

          picman_container_remove (container, active);
        }
    }
}
