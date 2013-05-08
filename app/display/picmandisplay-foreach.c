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
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlist.h"

#include "picmandisplay.h"
#include "picmandisplay-foreach.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-cursor.h"


gboolean
picman_displays_dirty (Picman *picman)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), FALSE);

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;
      PicmanImage   *image   = picman_display_get_image (display);

      if (image && picman_image_is_dirty (image))
        return TRUE;
    }

  return FALSE;
}

static void
picman_displays_image_dirty_callback (PicmanImage     *image,
                                    PicmanDirtyMask  dirty_mask,
                                    PicmanContainer *container)
{
  if (picman_image_is_dirty (image)              &&
      picman_image_get_display_count (image) > 0 &&
      ! picman_container_have (container, PICMAN_OBJECT (image)))
    picman_container_add (container, PICMAN_OBJECT (image));
}

static void
picman_displays_dirty_images_disconnect (PicmanContainer *dirty_container,
                                       PicmanContainer *global_container)
{
  GQuark handler;

  handler = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dirty_container),
                                                "clean-handler"));
  picman_container_remove_handler (global_container, handler);

  handler = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dirty_container),
                                                "dirty-handler"));
  picman_container_remove_handler (global_container, handler);
}

static void
picman_displays_image_clean_callback (PicmanImage     *image,
                                    PicmanDirtyMask  dirty_mask,
                                    PicmanContainer *container)
{
  if (! picman_image_is_dirty (image))
    picman_container_remove (container, PICMAN_OBJECT (image));
}

PicmanContainer *
picman_displays_get_dirty_images (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (picman_displays_dirty (picman))
    {
      PicmanContainer *container = picman_list_new_weak (PICMAN_TYPE_IMAGE, FALSE);
      GList         *list;
      GQuark         handler;

      handler =
        picman_container_add_handler (picman->images, "clean",
                                    G_CALLBACK (picman_displays_image_dirty_callback),
                                    container);
      g_object_set_data (G_OBJECT (container), "clean-handler",
                         GINT_TO_POINTER (handler));

      handler =
        picman_container_add_handler (picman->images, "dirty",
                                    G_CALLBACK (picman_displays_image_dirty_callback),
                                    container);
      g_object_set_data (G_OBJECT (container), "dirty-handler",
                         GINT_TO_POINTER (handler));

      g_signal_connect_object (container, "disconnect",
                               G_CALLBACK (picman_displays_dirty_images_disconnect),
                               G_OBJECT (picman->images), 0);

      picman_container_add_handler (container, "clean",
                                  G_CALLBACK (picman_displays_image_clean_callback),
                                  container);
      picman_container_add_handler (container, "dirty",
                                  G_CALLBACK (picman_displays_image_clean_callback),
                                  container);

      for (list = picman_get_image_iter (picman);
           list;
           list = g_list_next (list))
        {
          PicmanImage *image = list->data;

          if (picman_image_is_dirty (image) &&
              picman_image_get_display_count (image) > 0)
            picman_container_add (container, PICMAN_OBJECT (image));
        }

      return container;
    }

  return NULL;
}

/**
 * picman_displays_delete:
 * @picman:
 *
 * Calls picman_display_delete() an all displays in the display list.
 * This closes all displays, including the first one which is usually
 * kept open.
 */
void
picman_displays_delete (Picman *picman)
{
  /*  this removes the PicmanDisplay from the list, so do a while loop
   *  "around" the first element to get them all
   */
  while (! picman_container_is_empty (picman->displays))
    {
      PicmanDisplay *display = picman_get_display_iter (picman)->data;

      picman_display_delete (display);
    }
}

/**
 * picman_displays_close:
 * @picman:
 *
 * Calls picman_display_close() an all displays in the display list. The
 * first display will remain open without an image.
 */
void
picman_displays_close (Picman *picman)
{
  GList *list;
  GList *iter;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  list = g_list_copy (picman_get_display_iter (picman));

  for (iter = list; iter; iter = g_list_next (iter))
    {
      PicmanDisplay *display = iter->data;

      picman_display_close (display);
    }

  g_list_free (list);
}

void
picman_displays_reconnect (Picman      *picman,
                         PicmanImage *old,
                         PicmanImage *new)
{
  GList *contexts = NULL;
  GList *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_IMAGE (old));
  g_return_if_fail (PICMAN_IS_IMAGE (new));

  /*  check which contexts refer to old_image  */
  for (list = picman->context_list; list; list = g_list_next (list))
    {
      PicmanContext *context = list->data;

      if (picman_context_get_image (context) == old)
        contexts = g_list_prepend (contexts, list->data);
    }

  /*  set the new_image on the remembered contexts (in reverse order,
   *  since older contexts are usually the parents of newer
   *  ones). Also, update the contexts before the displays, or we
   *  might run into menu update functions that would see an
   *  inconsistent state (display = new, context = old), and thus
   *  inadvertently call actions as if the user had selected a menu
   *  item.
   */
  g_list_foreach (contexts, (GFunc) picman_context_set_image, new);
  g_list_free (contexts);

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;

      if (picman_display_get_image (display) == old)
        picman_display_set_image (display, new);
    }
}

gint
picman_displays_get_num_visible (Picman *picman)
{
  GList *list;
  gint   visible = 0;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), 0);

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay      *display = list->data;
      PicmanDisplayShell *shell   = picman_display_get_shell (display);

      if (gtk_widget_is_drawable (GTK_WIDGET (shell)))
        {
          GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

          if (GTK_IS_WINDOW (toplevel))
            {
              GdkWindow      *window = gtk_widget_get_window (toplevel);
              GdkWindowState  state  = gdk_window_get_state (window);

              if ((state & (GDK_WINDOW_STATE_WITHDRAWN |
                            GDK_WINDOW_STATE_ICONIFIED)) == 0)
                {
                  visible++;
                }
            }
        }
    }

  return visible;
}

void
picman_displays_set_busy (Picman *picman)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplayShell *shell =
        picman_display_get_shell (PICMAN_DISPLAY (list->data));

      picman_display_shell_set_override_cursor (shell, GDK_WATCH);
    }
}

void
picman_displays_unset_busy (Picman *picman)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplayShell *shell =
        picman_display_get_shell (PICMAN_DISPLAY (list->data));

      picman_display_shell_unset_override_cursor (shell);
    }
}
