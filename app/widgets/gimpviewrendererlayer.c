/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererlayer.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"

#include "text/picmantextlayer.h"

#include "picmanviewrendererlayer.h"


static void   picman_view_renderer_layer_render (PicmanViewRenderer *renderer,
                                               GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererLayer, picman_view_renderer_layer,
               PICMAN_TYPE_VIEW_RENDERER_DRAWABLE)

#define parent_class picman_view_renderer_layer_parent_class


static void
picman_view_renderer_layer_class_init (PicmanViewRendererLayerClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->render = picman_view_renderer_layer_render;
}

static void
picman_view_renderer_layer_init (PicmanViewRendererLayer *renderer)
{
}

static void
picman_view_renderer_layer_render (PicmanViewRenderer *renderer,
                                 GtkWidget        *widget)
{
  const gchar *stock_id = NULL;

  if (picman_layer_is_floating_sel (PICMAN_LAYER (renderer->viewable)))
    {
      stock_id = PICMAN_STOCK_FLOATING_SELECTION;
    }
  else if (picman_item_is_text_layer (PICMAN_ITEM (renderer->viewable)))
    {
      stock_id = picman_viewable_get_stock_id (renderer->viewable);
    }
  else
    {
      PicmanContainer *children = picman_viewable_get_children (renderer->viewable);

      if (children && picman_container_get_n_children (children) == 0)
        stock_id = GTK_STOCK_DIRECTORY;
    }

  if (stock_id)
    picman_view_renderer_render_stock (renderer, widget, stock_id);
  else
    PICMAN_VIEW_RENDERER_CLASS (parent_class)->render (renderer, widget);
}
