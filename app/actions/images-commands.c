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

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "widgets/picmancontainerview.h"
#include "widgets/picmanimageview.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "images-commands.h"


/*  public functionss */

void
images_raise_views_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanImage           *image;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  image = picman_context_get_image (context);

  if (image && picman_container_have (container, PICMAN_OBJECT (image)))
    {
      GList *list;

      for (list = picman_get_display_iter (image->picman);
           list;
           list = g_list_next (list))
        {
          PicmanDisplay *display = list->data;

          if (picman_display_get_image (display) == image)
            picman_display_shell_present (picman_display_get_shell (display));
        }
    }
}

void
images_new_view_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanImage           *image;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  image = picman_context_get_image (context);

  if (image && picman_container_have (container, PICMAN_OBJECT (image)))
    {
      picman_create_display (image->picman, image, PICMAN_UNIT_PIXEL, 1.0);
    }
}

void
images_delete_image_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanImage           *image;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  image = picman_context_get_image (context);

  if (image && picman_container_have (container, PICMAN_OBJECT (image)))
    {
      if (picman_image_get_display_count (image) == 0)
        g_object_unref (image);
    }
}
