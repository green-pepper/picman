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

#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pangocairo.h>

#include "libpicmancolor/picmancolor.h"

#include "text-types.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer-floating-sel.h"

#include "picmantext.h"
#include "picmantext-compat.h"
#include "picmantextlayer.h"

#include "picman-intl.h"


PicmanLayer *
text_render (PicmanImage    *image,
             PicmanDrawable *drawable,
             PicmanContext  *context,
             gint          text_x,
             gint          text_y,
             const gchar  *fontname,
             const gchar  *text,
             gint          border,
             gboolean      antialias)
{
  PangoFontDescription *desc;
  PicmanText             *gtext;
  PicmanLayer            *layer;
  PicmanRGB               color;
  gchar                *font;
  gdouble               size;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (drawable == NULL || PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (drawable == NULL ||
                        picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (fontname != NULL, NULL);
  g_return_val_if_fail (text != NULL, NULL);

  if (border < 0)
    border = 0;

  desc = pango_font_description_from_string (fontname);
  size = PANGO_PIXELS (pango_font_description_get_size (desc));

  pango_font_description_unset_fields (desc, PANGO_FONT_MASK_SIZE);
  font = pango_font_description_to_string (desc);

  pango_font_description_free (desc);

  picman_context_get_foreground (context, &color);

  gtext = g_object_new (PICMAN_TYPE_TEXT,
                        "text",      text,
                        "font",      font,
                        "font-size", size,
                        "antialias", antialias,
                        "border",    border,
                        "color",     &color,
                        NULL);

  g_free (font);

  layer = picman_text_layer_new (image, gtext);

  g_object_unref (gtext);

  if (!layer)
    return NULL;

  /*  Start a group undo  */
  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TEXT,
                               _("Add Text Layer"));

  /*  Set the layer offsets  */
  picman_item_set_offset (PICMAN_ITEM (layer), text_x, text_y);

  /*  If there is a selection mask clear it--
   *  this might not always be desired, but in general,
   *  it seems like the correct behavior.
   */
  if (! picman_channel_is_empty (picman_image_get_mask (image)))
    picman_channel_clear (picman_image_get_mask (image), NULL, TRUE);

  if (drawable == NULL)
    {
      /*  If the drawable is NULL, create a new layer  */
      picman_image_add_layer (image, layer, NULL, -1, TRUE);
    }
  else
    {
      /*  Otherwise, instantiate the text as the new floating selection */
      floating_sel_attach (layer, drawable);
    }

  /*  end the group undo  */
  picman_image_undo_group_end (image);

  return layer;
}

gboolean
text_get_extents (const gchar *fontname,
                  const gchar *text,
                  gint        *width,
                  gint        *height,
                  gint        *ascent,
                  gint        *descent)
{
  PangoFontDescription *font_desc;
  PangoContext         *context;
  PangoLayout          *layout;
  PangoFontMap         *fontmap;
  PangoRectangle        rect;

  g_return_val_if_fail (fontname != NULL, FALSE);
  g_return_val_if_fail (text != NULL, FALSE);

  fontmap = pango_cairo_font_map_new_for_font_type (CAIRO_FONT_TYPE_FT);
  if (! fontmap)
    g_error ("You are using a Pango that has been built against a cairo "
             "that lacks the Freetype font backend");

  pango_cairo_font_map_set_resolution (PANGO_CAIRO_FONT_MAP (fontmap),
                                       72.0); /* FIXME: resolution */
  context = pango_font_map_create_context (fontmap);
  g_object_unref (fontmap);

  layout = pango_layout_new (context);
  g_object_unref (context);

  font_desc = pango_font_description_from_string (fontname);
  pango_layout_set_font_description (layout, font_desc);
  pango_font_description_free (font_desc);

  pango_layout_set_text (layout, text, -1);

  pango_layout_get_pixel_extents (layout, NULL, &rect);

  if (width)
    *width = rect.width;
  if (height)
    *height = rect.height;

  if (ascent || descent)
    {
      PangoLayoutIter *iter;
      PangoLayoutLine *line;

      iter = pango_layout_get_iter (layout);
      line = pango_layout_iter_get_line_readonly (iter);
      pango_layout_iter_free (iter);

      pango_layout_line_get_pixel_extents (line, NULL, &rect);

      if (ascent)
        *ascent = PANGO_ASCENT (rect);
      if (descent)
        *descent = - PANGO_DESCENT (rect);
    }

  g_object_unref (layout);

  return TRUE;
}
