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

#include "display-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-icon.h"


#define PICMAN_DISPLAY_UPDATE_ICON_TIMEOUT  1000

static gboolean   picman_display_shell_icon_update_idle (gpointer data);


/*  public functions  */

void
picman_display_shell_icon_update (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_icon_update_stop (shell);

  if (picman_display_get_image (shell->display))
    shell->icon_idle_id = g_timeout_add_full (G_PRIORITY_LOW,
                                              PICMAN_DISPLAY_UPDATE_ICON_TIMEOUT,
                                              picman_display_shell_icon_update_idle,
                                              shell,
                                              NULL);
  else
    picman_display_shell_icon_update_idle (shell);
}

void
picman_display_shell_icon_update_stop (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->icon_idle_id)
    {
      g_source_remove (shell->icon_idle_id);
      shell->icon_idle_id = 0;
    }
}


/*  private functions  */

static gboolean
picman_display_shell_icon_update_idle (gpointer data)
{
  PicmanDisplayShell *shell  = PICMAN_DISPLAY_SHELL (data);
  PicmanImage        *image  = picman_display_get_image (shell->display);
  GdkPixbuf        *pixbuf = NULL;

  shell->icon_idle_id = 0;

  if (image)
    {
      Picman    *picman   = picman_display_get_picman (shell->display);
      gint     width;
      gint     height;
      gdouble  factor = ((gdouble) picman_image_get_height (image) /
                         (gdouble) picman_image_get_width  (image));

      if (factor >= 1)
        {
          height = MAX (shell->icon_size, 1);
          width  = MAX (((gdouble) shell->icon_size) / factor, 1);
        }
      else
        {
          height = MAX (((gdouble) shell->icon_size) * factor, 1);
          width  = MAX (shell->icon_size, 1);
        }

      pixbuf = picman_viewable_get_pixbuf (PICMAN_VIEWABLE (image),
                                         picman_get_user_context (picman),
                                         width, height);
    }

  g_object_set (shell, "icon", pixbuf, NULL);

  return FALSE;
}
