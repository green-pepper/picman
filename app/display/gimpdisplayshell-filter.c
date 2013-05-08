/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1999 Manish Singh
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

#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmancoreconfig.h"

#include "picmandisplayshell.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-filter.h"


/*  local function prototypes  */

static void   picman_display_shell_filter_changed (PicmanColorDisplayStack *stack,
                                                 PicmanDisplayShell      *shell);


/*  public functions  */

void
picman_display_shell_filter_set (PicmanDisplayShell      *shell,
                               PicmanColorDisplayStack *stack)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (stack == NULL || PICMAN_IS_COLOR_DISPLAY_STACK (stack));

  if (stack == shell->filter_stack)
    return;

  if (shell->filter_stack)
    {
      g_signal_handlers_disconnect_by_func (shell->filter_stack,
                                            picman_display_shell_filter_changed,
                                            shell);

      g_object_unref (shell->filter_stack);
    }

  shell->filter_stack = stack;

  if (shell->filter_stack)
    {
      g_object_ref (shell->filter_stack);

      g_signal_connect (shell->filter_stack, "changed",
                        G_CALLBACK (picman_display_shell_filter_changed),
                        shell);
    }

  picman_display_shell_filter_changed (NULL, shell);
}

PicmanColorDisplayStack *
picman_display_shell_filter_new (PicmanDisplayShell *shell,
                               PicmanColorConfig  *config)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (PICMAN_IS_COLOR_CONFIG (config), NULL);

  if (config->display_module)
    {
      GType type = g_type_from_name (config->display_module);

      if (g_type_is_a (type, PICMAN_TYPE_COLOR_DISPLAY))
        {
          PicmanColorDisplay      *display;
          PicmanColorDisplayStack *stack;

          display = g_object_new (type,
                                  "color-config",  config,
                                  "color-managed", shell,
                                  NULL);

          stack = picman_color_display_stack_new ();

          picman_color_display_stack_add (stack, display);
          g_object_unref (display);

          return stack;
        }
    }

  return NULL;
}


/*  private functions  */

static gboolean
picman_display_shell_filter_changed_idle (gpointer data)
{
  PicmanDisplayShell *shell = data;

  picman_display_shell_expose_full (shell);
  shell->filter_idle_id = 0;

  return FALSE;
}

static void
picman_display_shell_filter_changed (PicmanColorDisplayStack *stack,
                                   PicmanDisplayShell      *shell)
{
  if (shell->filter_idle_id)
    g_source_remove (shell->filter_idle_id);

  shell->filter_idle_id =
    g_idle_add_full (G_PRIORITY_LOW,
                     picman_display_shell_filter_changed_idle,
                     shell, NULL);
}
