/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrenderer-utils.c
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

#include "widgets-types.h"

#include "core/picmanbrush.h"
#include "core/picmanbuffer.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmanimagefile.h"
#include "core/picmanlayer.h"
#include "core/picmanpalette.h"

#include "vectors/picmanvectors.h"

#include "picmanviewrenderer-utils.h"
#include "picmanviewrendererbrush.h"
#include "picmanviewrendererbuffer.h"
#include "picmanviewrendererlayer.h"
#include "picmanviewrenderergradient.h"
#include "picmanviewrendererimage.h"
#include "picmanviewrendererimagefile.h"
#include "picmanviewrendererpalette.h"
#include "picmanviewrenderervectors.h"


GType
picman_view_renderer_type_from_viewable_type (GType viewable_type)
{
  GType type = PICMAN_TYPE_VIEW_RENDERER;

  g_return_val_if_fail (g_type_is_a (viewable_type, PICMAN_TYPE_VIEWABLE),
                        G_TYPE_NONE);

  if (g_type_is_a (viewable_type, PICMAN_TYPE_BRUSH))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_BRUSH;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_BUFFER))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_BUFFER;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_IMAGE))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_IMAGE;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_LAYER))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_LAYER;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_DRAWABLE))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_DRAWABLE;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_GRADIENT))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_GRADIENT;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_VECTORS))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_VECTORS;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_IMAGEFILE))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_IMAGEFILE;
    }
  else if (g_type_is_a (viewable_type, PICMAN_TYPE_PALETTE))
    {
      type = PICMAN_TYPE_VIEW_RENDERER_PALETTE;
    }

  return type;
}
