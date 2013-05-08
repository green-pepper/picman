/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#include <pango/pangocairo.h>

#include "text-types.h"

#include "picmantextlayout.h"
#include "picmantextlayout-render.h"


void
picman_text_layout_render (PicmanTextLayout    *layout,
                         cairo_t           *cr,
                         PicmanTextDirection  base_dir,
                         gboolean           path)
{
  PangoLayout    *pango_layout;
  cairo_matrix_t  trafo;
  gint            x, y;

  g_return_if_fail (PICMAN_IS_TEXT_LAYOUT (layout));
  g_return_if_fail (cr != NULL);

  picman_text_layout_get_offsets (layout, &x, &y);

  cairo_translate (cr, x, y);

  picman_text_layout_get_transform (layout, &trafo);
  cairo_transform (cr, &trafo);

  pango_layout = picman_text_layout_get_pango_layout (layout);

  if (path)
    pango_cairo_layout_path (cr, pango_layout);
  else
    pango_cairo_show_layout (cr, pango_layout);
}
