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

#include <stdlib.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picman-edit.h"
#include "picmanbuffer.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmandrawableundo.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"
#include "picmanlist.h"
#include "picmanpattern.h"
#include "picmanpickable.h"
#include "picmanselection.h"
#include "picmantempbuf.h"

#include "picman-intl.h"


/*  local function protypes  */

static PicmanBuffer * picman_edit_extract (PicmanImage     *image,
                                       PicmanPickable  *pickable,
                                       PicmanContext   *context,
                                       gboolean       cut_pixels,
                                       GError       **error);


/*  public functions  */

const PicmanBuffer *
picman_edit_cut (PicmanImage     *image,
               PicmanDrawable  *drawable,
               PicmanContext   *context,
               GError       **error)
{
  PicmanBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (drawable),
                              context, TRUE, error);

  if (buffer)
    {
      picman_set_global_buffer (image->picman, buffer);
      g_object_unref (buffer);

      return image->picman->global_buffer;
    }

  return NULL;
}

const PicmanBuffer *
picman_edit_copy (PicmanImage     *image,
                PicmanDrawable  *drawable,
                PicmanContext   *context,
                GError       **error)
{
  PicmanBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (drawable),
                              context, FALSE, error);

  if (buffer)
    {
      picman_set_global_buffer (image->picman, buffer);
      g_object_unref (buffer);

      return image->picman->global_buffer;
    }

  return NULL;
}

const PicmanBuffer *
picman_edit_copy_visible (PicmanImage    *image,
                        PicmanContext  *context,
                        GError      **error)
{
  PicmanProjection *projection;
  PicmanBuffer     *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  projection = picman_image_get_projection (image);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (projection),
                              context, FALSE, error);

  if (buffer)
    {
      picman_set_global_buffer (image->picman, buffer);
      g_object_unref (buffer);

      return image->picman->global_buffer;
    }

  return NULL;
}

PicmanLayer *
picman_edit_paste (PicmanImage    *image,
                 PicmanDrawable *drawable,
                 PicmanBuffer   *paste,
                 gboolean      paste_into,
                 gint          viewport_x,
                 gint          viewport_y,
                 gint          viewport_width,
                 gint          viewport_height)
{
  PicmanLayer  *layer;
  const Babl *format;
  gint        image_width;
  gint        image_height;
  gint        width;
  gint        height;
  gint        offset_x;
  gint        offset_y;
  gboolean    clamp_to_image = TRUE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (drawable == NULL || PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (drawable == NULL ||
                        picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_BUFFER (paste), NULL);

  /*  Make a new layer: if drawable == NULL,
   *  user is pasting into an empty image.
   */

  if (drawable)
    format = picman_drawable_get_format_with_alpha (drawable);
  else
    format = picman_image_get_layer_format (image, TRUE);

  layer = picman_layer_new_from_buffer (picman_buffer_get_buffer (paste),
                                      image,
                                      format,
                                      _("Pasted Layer"),
                                      PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  if (! layer)
    return NULL;

  image_width  = picman_image_get_width  (image);
  image_height = picman_image_get_height (image);

  width  = picman_item_get_width  (PICMAN_ITEM (layer));
  height = picman_item_get_height (PICMAN_ITEM (layer));

  if (viewport_width  == image_width &&
      viewport_height == image_height)
    {
      /* if the whole image is visible, act as if there was no viewport */

      viewport_x      = 0;
      viewport_y      = 0;
      viewport_width  = 0;
      viewport_height = 0;
    }

  if (drawable)
    {
      /*  if pasting to a drawable  */

      gint     off_x, off_y;
      gint     x1, y1, x2, y2;
      gint     paste_x, paste_y;
      gint     paste_width, paste_height;
      gboolean have_mask;

      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);
      have_mask = picman_item_mask_bounds (PICMAN_ITEM (drawable),
                                         &x1, &y1, &x2, &y2);

      if (! have_mask         && /* if we have no mask */
          viewport_width  > 0 && /* and we have a viewport */
          viewport_height > 0 &&
          (width  < (x2 - x1) || /* and the paste is smaller than the target */
           height < (y2 - y1)) &&

          /* and the viewport intersects with the target */
          picman_rectangle_intersect (viewport_x, viewport_y,
                                    viewport_width, viewport_height,
                                    off_x, off_y,
                                    x2 - x1, y2 - y1,
                                    &paste_x, &paste_y,
                                    &paste_width, &paste_height))
        {
          /*  center on the viewport  */

          offset_x = paste_x + (paste_width - width)  / 2;
          offset_y = paste_y + (paste_height- height) / 2;
        }
      else
        {
          /*  otherwise center on the target  */

          offset_x = off_x + ((x1 + x2) - width)  / 2;
          offset_y = off_y + ((y1 + y2) - height) / 2;

          /*  and keep it that way  */
          clamp_to_image = FALSE;
        }
    }
  else if (viewport_width  > 0 &&  /* if we have a viewport */
           viewport_height > 0 &&
           (width  < image_width || /* and the paste is       */
            height < image_height)) /* smaller than the image */
    {
      /*  center on the viewport  */

      offset_x = viewport_x + (viewport_width  - width)  / 2;
      offset_y = viewport_y + (viewport_height - height) / 2;
    }
  else
    {
      /*  otherwise center on the image  */

      offset_x = (image_width  - width)  / 2;
      offset_y = (image_height - height) / 2;

      /*  and keep it that way  */
      clamp_to_image = FALSE;
    }

  if (clamp_to_image)
    {
      /*  Ensure that the pasted layer is always within the image, if it
       *  fits and aligned at top left if it doesn't. (See bug #142944).
       */
      offset_x = MIN (offset_x, image_width  - width);
      offset_y = MIN (offset_y, image_height - height);
      offset_x = MAX (offset_x, 0);
      offset_y = MAX (offset_y, 0);
    }

  picman_item_set_offset (PICMAN_ITEM (layer), offset_x, offset_y);

  /*  Start a group undo  */
  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                               C_("undo-type", "Paste"));

  /*  If there is a selection mask clear it--
   *  this might not always be desired, but in general,
   *  it seems like the correct behavior.
   */
  if (! picman_channel_is_empty (picman_image_get_mask (image)) && ! paste_into)
    picman_channel_clear (picman_image_get_mask (image), NULL, TRUE);

  /*  if there's a drawable, add a new floating selection  */
  if (drawable)
    floating_sel_attach (layer, drawable);
  else
    picman_image_add_layer (image, layer, NULL, 0, TRUE);

  /*  end the group undo  */
  picman_image_undo_group_end (image);

  return layer;
}

const gchar *
picman_edit_named_cut (PicmanImage     *image,
                     const gchar   *name,
                     PicmanDrawable  *drawable,
                     PicmanContext   *context,
                     GError       **error)
{
  PicmanBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (drawable),
                              context, TRUE, error);

  if (buffer)
    {
      picman_object_set_name (PICMAN_OBJECT (buffer), name);
      picman_container_add (image->picman->named_buffers, PICMAN_OBJECT (buffer));
      g_object_unref (buffer);

      return picman_object_get_name (buffer);
    }

  return NULL;
}

const gchar *
picman_edit_named_copy (PicmanImage     *image,
                      const gchar   *name,
                      PicmanDrawable  *drawable,
                      PicmanContext   *context,
                      GError       **error)
{
  PicmanBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (drawable),
                              context, FALSE, error);

  if (buffer)
    {
      picman_object_set_name (PICMAN_OBJECT (buffer), name);
      picman_container_add (image->picman->named_buffers, PICMAN_OBJECT (buffer));
      g_object_unref (buffer);

      return picman_object_get_name (buffer);
    }

  return NULL;
}

const gchar *
picman_edit_named_copy_visible (PicmanImage    *image,
                              const gchar  *name,
                              PicmanContext  *context,
                              GError      **error)
{
  PicmanProjection *projection;
  PicmanBuffer     *buffer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  projection = picman_image_get_projection (image);

  buffer = picman_edit_extract (image, PICMAN_PICKABLE (projection),
                              context, FALSE, error);

  if (buffer)
    {
      picman_object_set_name (PICMAN_OBJECT (buffer), name);
      picman_container_add (image->picman->named_buffers, PICMAN_OBJECT (buffer));
      g_object_unref (buffer);

      return picman_object_get_name (buffer);
    }

  return NULL;
}

gboolean
picman_edit_clear (PicmanImage    *image,
                 PicmanDrawable *drawable,
                 PicmanContext  *context)
{
  PicmanRGB              background;
  PicmanLayerModeEffects paint_mode;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);

  picman_context_get_background (context, &background);

  if (picman_drawable_has_alpha (drawable))
    paint_mode = PICMAN_ERASE_MODE;
  else
    paint_mode = PICMAN_NORMAL_MODE;

  return picman_edit_fill_full (image, drawable,
                              &background, NULL,
                              PICMAN_OPACITY_OPAQUE, paint_mode,
                              C_("undo-type", "Clear"));
}

gboolean
picman_edit_fill (PicmanImage            *image,
                PicmanDrawable         *drawable,
                PicmanContext          *context,
                PicmanFillType          fill_type,
                gdouble               opacity,
                PicmanLayerModeEffects  paint_mode)
{
  PicmanRGB      color;
  PicmanPattern *pattern = NULL;
  const gchar *undo_desc;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);

  switch (fill_type)
    {
    case PICMAN_FOREGROUND_FILL:
      picman_context_get_foreground (context, &color);
      undo_desc = C_("undo-type", "Fill with Foreground Color");
      break;

    case PICMAN_BACKGROUND_FILL:
      picman_context_get_background (context, &color);
      undo_desc = C_("undo-type", "Fill with Background Color");
      break;

    case PICMAN_WHITE_FILL:
      picman_rgba_set (&color, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);
      undo_desc = C_("undo-type", "Fill with White");
      break;

    case PICMAN_TRANSPARENT_FILL:
      picman_context_get_background (context, &color);
      undo_desc = C_("undo-type", "Fill with Transparency");
      break;

    case PICMAN_PATTERN_FILL:
      pattern = picman_context_get_pattern (context);
      undo_desc = C_("undo-type", "Fill with Pattern");
      break;

    case PICMAN_NO_FILL:
      return TRUE;  /*  nothing to do, but the fill succeeded  */

    default:
      g_warning ("%s: unknown fill type", G_STRFUNC);
      return picman_edit_fill (image, drawable,
                             context, PICMAN_BACKGROUND_FILL,
                             PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);
    }

  return picman_edit_fill_full (image, drawable,
                              &color, pattern,
                              opacity, paint_mode,
                              undo_desc);
}

gboolean
picman_edit_fill_full (PicmanImage            *image,
                     PicmanDrawable         *drawable,
                     const PicmanRGB        *color,
                     PicmanPattern          *pattern,
                     gdouble               opacity,
                     PicmanLayerModeEffects  paint_mode,
                     const gchar          *undo_desc)
{
  GeglBuffer *dest_buffer;
  const Babl *format;
  gint        x, y, width, height;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (color != NULL || pattern != NULL, FALSE);

  if (! picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &width, &height))
    return TRUE;  /*  nothing to do, but the fill succeeded  */

  if (pattern &&
      babl_format_has_alpha (picman_temp_buf_get_format (pattern->mask)) &&
      ! picman_drawable_has_alpha (drawable))
    {
      format = picman_drawable_get_format_with_alpha (drawable);
    }
  else
    {
      format = picman_drawable_get_format (drawable);
    }

  dest_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                 format);

  if (pattern)
    {
      GeglBuffer *src_buffer = picman_pattern_create_buffer (pattern);

      gegl_buffer_set_pattern (dest_buffer, NULL, src_buffer, 0, 0);
      g_object_unref (src_buffer);
    }
  else
    {
      GeglColor *gegl_color = picman_gegl_color_new (color);

      gegl_buffer_set_color (dest_buffer, NULL, gegl_color);
      g_object_unref (gegl_color);
    }

  picman_drawable_apply_buffer (drawable, dest_buffer,
                              GEGL_RECTANGLE (0, 0, width, height),
                              TRUE, undo_desc,
                              opacity, paint_mode,
                              NULL, x, y);

  g_object_unref (dest_buffer);

  picman_drawable_update (drawable, x, y, width, height);

  return TRUE;
}

gboolean
picman_edit_fade (PicmanImage   *image,
                PicmanContext *context)
{
  PicmanDrawableUndo *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);

  undo = PICMAN_DRAWABLE_UNDO (picman_image_undo_get_fadeable (image));

  if (undo && undo->applied_buffer)
    {
      PicmanDrawable *drawable;
      GeglBuffer   *buffer;

      drawable = PICMAN_DRAWABLE (PICMAN_ITEM_UNDO (undo)->item);

      g_object_ref (undo);
      buffer = g_object_ref (undo->applied_buffer);

      picman_image_undo (image);

      picman_drawable_apply_buffer (drawable, buffer,
                                  GEGL_RECTANGLE (0, 0,
                                                  gegl_buffer_get_width (undo->buffer),
                                                  gegl_buffer_get_height (undo->buffer)),
                                  TRUE,
                                  picman_object_get_name (undo),
                                  picman_context_get_opacity (context),
                                  picman_context_get_paint_mode (context),
                                  NULL, undo->x, undo->y);

      g_object_unref (buffer);
      g_object_unref (undo);

      return TRUE;
    }

  return FALSE;
}


/*  private functions  */

static PicmanBuffer *
picman_edit_extract (PicmanImage     *image,
                   PicmanPickable  *pickable,
                   PicmanContext   *context,
                   gboolean       cut_pixels,
                   GError       **error)
{
  GeglBuffer *buffer;
  gint        offset_x;
  gint        offset_y;

  if (cut_pixels)
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_CUT, C_("undo-type", "Cut"));

  /*  Cut/copy the mask portion from the image  */
  buffer = picman_selection_extract (PICMAN_SELECTION (picman_image_get_mask (image)),
                                   pickable, context,
                                   cut_pixels, FALSE, FALSE,
                                   &offset_x, &offset_y, error);

  if (cut_pixels)
    picman_image_undo_group_end (image);

  if (buffer)
    {
      PicmanBuffer *picman_buffer = picman_buffer_new (buffer, _("Global Buffer"),
                                                 offset_x, offset_y, FALSE);
      g_object_unref (buffer);

      return picman_buffer;
    }

  return NULL;
}
