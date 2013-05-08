/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselectorpalette.c
 * Copyright (C) 2006 Michael Natterer <mitch@picman.org>
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanpalette.h"

#include "picmancolorselectorpalette.h"
#include "picmanpaletteview.h"
#include "picmanviewrendererpalette.h"

#include "picman-intl.h"


static void   picman_color_selector_palette_set_color  (PicmanColorSelector *selector,
                                                      const PicmanRGB     *rgb,
                                                      const PicmanHSV     *hsv);
static void   picman_color_selector_palette_set_config (PicmanColorSelector *selector,
                                                      PicmanColorConfig   *config);


G_DEFINE_TYPE (PicmanColorSelectorPalette, picman_color_selector_palette,
               PICMAN_TYPE_COLOR_SELECTOR)

#define parent_class picman_color_selector_palette_parent_class


static void
picman_color_selector_palette_class_init (PicmanColorSelectorPaletteClass *klass)
{
  PicmanColorSelectorClass *selector_class = PICMAN_COLOR_SELECTOR_CLASS (klass);

  selector_class->name       = _("Palette");
  selector_class->help_id    = "picman-colorselector-palette";
  selector_class->stock_id   = PICMAN_STOCK_PALETTE;
  selector_class->set_color  = picman_color_selector_palette_set_color;
  selector_class->set_config = picman_color_selector_palette_set_config;
}

static void
picman_color_selector_palette_init (PicmanColorSelectorPalette *select)
{
}

static void
picman_color_selector_palette_set_color (PicmanColorSelector *selector,
                                       const PicmanRGB     *rgb,
                                       const PicmanHSV     *hsv)
{
  PicmanColorSelectorPalette *select = PICMAN_COLOR_SELECTOR_PALETTE (selector);

  if (select->context)
    {
      PicmanPalette *palette = picman_context_get_palette (select->context);

      if (palette && picman_palette_get_n_colors (palette) > 0)
        {
          PicmanPaletteEntry *entry;

          entry = picman_palette_find_entry (palette, rgb,
                                           PICMAN_PALETTE_VIEW (select->view)->selected);

          if (entry)
            picman_palette_view_select_entry (PICMAN_PALETTE_VIEW (select->view),
                                            entry);
        }
    }
}

static void
picman_color_selector_palette_palette_changed (PicmanContext              *context,
                                             PicmanPalette              *palette,
                                             PicmanColorSelectorPalette *select)
{
  picman_view_set_viewable (PICMAN_VIEW (select->view), PICMAN_VIEWABLE (palette));
}

static void
picman_color_selector_palette_entry_clicked (PicmanPaletteView   *view,
                                           PicmanPaletteEntry  *entry,
                                           GdkModifierType    state,
                                           PicmanColorSelector *selector)
{
  selector->rgb = entry->color;
  picman_rgb_to_hsv (&selector->rgb, &selector->hsv);

  picman_color_selector_color_changed (selector);
}

static void
picman_color_selector_palette_set_config (PicmanColorSelector *selector,
                                        PicmanColorConfig   *config)
{
  PicmanColorSelectorPalette *select = PICMAN_COLOR_SELECTOR_PALETTE (selector);

  if (select->context)
    {
      g_signal_handlers_disconnect_by_func (select->context,
                                            picman_color_selector_palette_palette_changed,
                                            select);
      picman_view_renderer_set_context (PICMAN_VIEW (select->view)->renderer,
                                      NULL);

      g_object_unref (select->context);
      select->context = NULL;
    }

  if (config)
    select->context = g_object_get_data (G_OBJECT (config), "picman-context");

  if (select->context)
    {
      g_object_ref (select->context);

      if (! select->view)
        {
          GtkWidget *frame;

          frame = gtk_frame_new (NULL);
          gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
          gtk_box_pack_start (GTK_BOX (select), frame, TRUE, TRUE, 0);
          gtk_widget_show (frame);

          select->view = picman_view_new_full_by_types (select->context,
                                                      PICMAN_TYPE_PALETTE_VIEW,
                                                      PICMAN_TYPE_PALETTE,
                                                      100, 100, 0,
                                                      FALSE, TRUE, FALSE);
          picman_view_set_expand (PICMAN_VIEW (select->view), TRUE);
          picman_view_renderer_palette_set_cell_size
            (PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (select->view)->renderer),
             -1);
          picman_view_renderer_palette_set_draw_grid
            (PICMAN_VIEW_RENDERER_PALETTE (PICMAN_VIEW (select->view)->renderer),
             TRUE);
          gtk_container_add (GTK_CONTAINER (frame), select->view);
          gtk_widget_show (select->view);

          g_signal_connect (select->view, "entry-clicked",
                            G_CALLBACK (picman_color_selector_palette_entry_clicked),
                            select);
        }
      else
        {
          picman_view_renderer_set_context (PICMAN_VIEW (select->view)->renderer,
                                          select->context);
        }

      g_signal_connect_object (select->context, "palette-changed",
                               G_CALLBACK (picman_color_selector_palette_palette_changed),
                               select, 0);

      picman_color_selector_palette_palette_changed (select->context,
                                                   picman_context_get_palette (select->context),
                                                   select);
    }
}
