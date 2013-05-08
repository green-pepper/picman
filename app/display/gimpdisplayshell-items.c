/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandisplayshell-items.c
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#include <libpicmanmath/picmanmath.h>

#include "display-types.h"

#include "picmancanvascursor.h"
#include "picmancanvasgrid.h"
#include "picmancanvaslayerboundary.h"
#include "picmancanvaspassepartout.h"
#include "picmancanvasproxygroup.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-items.h"
#include "picmandisplayshell-transform.h"


/*  local function prototypes  */

static void   picman_display_shell_item_update           (PicmanCanvasItem   *item,
                                                        cairo_region_t   *region,
                                                        PicmanDisplayShell *shell);
static void   picman_display_shell_unrotated_item_update (PicmanCanvasItem   *item,
                                                        cairo_region_t   *region,
                                                        PicmanDisplayShell *shell);


/*  public functions  */

void
picman_display_shell_items_init (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  shell->canvas_item = picman_canvas_group_new (shell);

  shell->passe_partout = picman_canvas_passe_partout_new (shell, 0, 0, 0, 0);
  picman_canvas_item_set_visible (shell->passe_partout, FALSE);
  picman_display_shell_add_item (shell, shell->passe_partout);
  g_object_unref (shell->passe_partout);

  shell->preview_items = picman_canvas_group_new (shell);
  picman_display_shell_add_item (shell, shell->preview_items);
  g_object_unref (shell->preview_items);

  shell->vectors = picman_canvas_proxy_group_new (shell);
  picman_display_shell_add_item (shell, shell->vectors);
  g_object_unref (shell->vectors);

  shell->grid = picman_canvas_grid_new (shell, NULL);
  picman_canvas_item_set_visible (shell->grid, FALSE);
  g_object_set (shell->grid, "grid-style", TRUE, NULL);
  picman_display_shell_add_item (shell, shell->grid);
  g_object_unref (shell->grid);

  shell->guides = picman_canvas_proxy_group_new (shell);
  picman_display_shell_add_item (shell, shell->guides);
  g_object_unref (shell->guides);

  shell->sample_points = picman_canvas_proxy_group_new (shell);
  picman_display_shell_add_item (shell, shell->sample_points);
  g_object_unref (shell->sample_points);

  shell->layer_boundary = picman_canvas_layer_boundary_new (shell);
  picman_canvas_item_set_visible (shell->layer_boundary, FALSE);
  picman_display_shell_add_item (shell, shell->layer_boundary);
  g_object_unref (shell->layer_boundary);

  shell->tool_items = picman_canvas_group_new (shell);
  picman_display_shell_add_item (shell, shell->tool_items);
  g_object_unref (shell->tool_items);

  g_signal_connect (shell->canvas_item, "update",
                    G_CALLBACK (picman_display_shell_item_update),
                    shell);

  shell->unrotated_item = picman_canvas_group_new (shell);

  shell->cursor = picman_canvas_cursor_new (shell);
  picman_canvas_item_set_visible (shell->cursor, FALSE);
  picman_display_shell_add_unrotated_item (shell, shell->cursor);
  g_object_unref (shell->cursor);

  g_signal_connect (shell->unrotated_item, "update",
                    G_CALLBACK (picman_display_shell_unrotated_item_update),
                    shell);
}

void
picman_display_shell_items_free (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->canvas_item)
    {
      g_signal_handlers_disconnect_by_func (shell->canvas_item,
                                            picman_display_shell_item_update,
                                            shell);

      g_object_unref (shell->canvas_item);
      shell->canvas_item = NULL;

      shell->passe_partout  = NULL;
      shell->preview_items  = NULL;
      shell->vectors        = NULL;
      shell->grid           = NULL;
      shell->guides         = NULL;
      shell->sample_points  = NULL;
      shell->layer_boundary = NULL;
      shell->tool_items     = NULL;
    }

  if (shell->unrotated_item)
    {
      g_signal_handlers_disconnect_by_func (shell->unrotated_item,
                                            picman_display_shell_unrotated_item_update,
                                            shell);

      g_object_unref (shell->unrotated_item);
      shell->unrotated_item = NULL;

      shell->cursor = NULL;
    }
}

void
picman_display_shell_add_item (PicmanDisplayShell *shell,
                             PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (shell->canvas_item), item);
}

void
picman_display_shell_remove_item (PicmanDisplayShell *shell,
                                PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (shell->canvas_item), item);
}

void
picman_display_shell_add_preview_item (PicmanDisplayShell *shell,
                                     PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (shell->preview_items), item);
}

void
picman_display_shell_remove_preview_item (PicmanDisplayShell *shell,
                                        PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (shell->preview_items), item);
}

void
picman_display_shell_add_unrotated_item (PicmanDisplayShell *shell,
                                       PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (shell->unrotated_item), item);
}

void
picman_display_shell_remove_unrotated_item (PicmanDisplayShell *shell,
                                          PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (shell->unrotated_item), item);
}

void
picman_display_shell_add_tool_item (PicmanDisplayShell *shell,
                                  PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_add_item (PICMAN_CANVAS_GROUP (shell->tool_items), item);
}

void
picman_display_shell_remove_tool_item (PicmanDisplayShell *shell,
                                     PicmanCanvasItem   *item)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_CANVAS_ITEM (item));

  picman_canvas_group_remove_item (PICMAN_CANVAS_GROUP (shell->tool_items), item);
}


/*  private functions  */

static void
picman_display_shell_item_update (PicmanCanvasItem   *item,
                                cairo_region_t   *region,
                                PicmanDisplayShell *shell)
{
  if (shell->rotate_transform)
    {
      gint n_rects;
      gint i;

      n_rects = cairo_region_num_rectangles (region);

      for (i = 0; i < n_rects; i++)
        {
          cairo_rectangle_int_t rect;
          gdouble               tx1, ty1;
          gdouble               tx2, ty2;
          gint                  x1, y1, x2, y2;

          cairo_region_get_rectangle (region, i, &rect);

          picman_display_shell_rotate_bounds (shell,
                                            rect.x, rect.y,
                                            rect.x + rect.width,
                                            rect.y + rect.height,
                                            &tx1, &ty1, &tx2, &ty2);

          x1 = floor (tx1 - 0.5);
          y1 = floor (ty1 - 0.5);
          x2 = ceil (tx2 + 0.5);
          y2 = ceil (ty2 + 0.5);

          picman_display_shell_expose_area (shell, x1, y1, x2 - x1, y2 - y1);
        }
    }
  else
    {
      picman_display_shell_expose_region (shell, region);
    }
}

static void
picman_display_shell_unrotated_item_update (PicmanCanvasItem   *item,
                                          cairo_region_t   *region,
                                          PicmanDisplayShell *shell)
{
  picman_display_shell_expose_region (shell, region);
}
