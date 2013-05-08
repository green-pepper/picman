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
#include "core/picman-edit.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-new.h"

#include "widgets/picmanbufferview.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmancontainerview-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-transform.h"

#include "buffers-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   buffers_paste (PicmanBufferView *view,
                             gboolean        paste_into);


/*  public functionss */

void
buffers_paste_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  buffers_paste (PICMAN_BUFFER_VIEW (data), FALSE);
}

void
buffers_paste_into_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  buffers_paste (PICMAN_BUFFER_VIEW (data), TRUE);
}

void
buffers_paste_as_new_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanBuffer          *buffer;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  buffer = picman_context_get_buffer (context);

  if (buffer && picman_container_have (container, PICMAN_OBJECT (buffer)))
    {
      PicmanImage *image = picman_context_get_image (context);

      if (image)
        {
          PicmanImage *new_image;

          new_image = picman_image_new_from_buffer (image->picman, image, buffer);
          picman_create_display (image->picman, new_image,
                               PICMAN_UNIT_PIXEL, 1.0);
          g_object_unref (new_image);
        }
    }
}

void
buffers_delete_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);

  picman_container_view_remove_active (editor->view);
}


/*  private functions  */

static void
buffers_paste (PicmanBufferView *view,
               gboolean        paste_into)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (view);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanBuffer          *buffer;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  buffer = picman_context_get_buffer (context);

  if (buffer && picman_container_have (container, PICMAN_OBJECT (buffer)))
    {
      PicmanDisplay *display = picman_context_get_display (context);
      PicmanImage   *image   = NULL;
      gint         x       = -1;
      gint         y       = -1;
      gint         width   = -1;
      gint         height  = -1;

      if (display)
        {
          PicmanDisplayShell *shell = picman_display_get_shell (display);

          picman_display_shell_untransform_viewport (shell,
                                                   &x, &y, &width, &height);

          image = picman_display_get_image (display);
        }
      else
        {
          image = picman_context_get_image (context);
        }

      if (image)
        {
          picman_edit_paste (image, picman_image_get_active_drawable (image),
                           buffer, paste_into, x, y, width, height);

          picman_image_flush (image);
        }
    }
}
