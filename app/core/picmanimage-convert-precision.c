/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimage-convert-precision.c
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#include "core-types.h"

#include "gegl/picman-gegl-utils.h"

#include "picmandrawable.h"
#include "picmanimage.h"
#include "picmanimage-convert-precision.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanprogress.h"

#include "text/picmantextlayer.h"

#include "picman-intl.h"


void
picman_image_convert_precision (PicmanImage     *image,
                              PicmanPrecision  precision,
                              gint           layer_dither_type,
                              gint           text_layer_dither_type,
                              gint           mask_dither_type,
                              PicmanProgress  *progress)
{
  GList       *all_drawables;
  GList       *list;
  const gchar *undo_desc = NULL;
  gint         nth_drawable, n_drawables;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (precision != picman_image_get_precision (image));
  g_return_if_fail (precision == PICMAN_PRECISION_U8 ||
                    picman_image_get_base_type (image) != PICMAN_INDEXED);
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  all_drawables = g_list_concat (picman_image_get_layer_list (image),
                                 picman_image_get_channel_list (image));

  n_drawables = g_list_length (all_drawables) + 1 /* + selection */;

  switch (precision)
    {
    case PICMAN_PRECISION_U8:
      undo_desc = C_("undo-type", "Convert Image to 8 bit integer");
      break;

    case PICMAN_PRECISION_U16:
      undo_desc = C_("undo-type", "Convert Image to 16 bit integer");
      break;

    case PICMAN_PRECISION_U32:
      undo_desc = C_("undo-type", "Convert Image to 32 bit integer");
      break;

    case PICMAN_PRECISION_HALF:
      undo_desc = C_("undo-type", "Convert Image to 16 bit floating point");
      break;

    case PICMAN_PRECISION_FLOAT:
      undo_desc = C_("undo-type", "Convert Image to 32 bit floating point");
      break;
    }

  if (progress)
    picman_progress_start (progress, undo_desc, FALSE);

  g_object_freeze_notify (G_OBJECT (image));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_CONVERT,
                               undo_desc);

  /*  Push the image precision to the stack  */
  picman_image_undo_push_image_precision (image, NULL);

  /*  Set the new precision  */
  g_object_set (image, "precision", precision, NULL);

  for (list = all_drawables, nth_drawable = 0;
       list;
       list = g_list_next (list), nth_drawable++)
    {
      PicmanDrawable *drawable = list->data;
      gint          dither_type;

      if (picman_item_is_text_layer (PICMAN_ITEM (drawable)))
        dither_type = text_layer_dither_type;
      else
        dither_type = layer_dither_type;

      picman_drawable_convert_type (drawable, image,
                                  picman_drawable_get_base_type (drawable),
                                  precision,
                                  dither_type,
                                  mask_dither_type,
                                  TRUE);

      if (progress)
        picman_progress_set_value (progress,
                                 (gdouble) nth_drawable / (gdouble) n_drawables);
    }
  g_list_free (all_drawables);

  /*  convert the selection mask  */
  {
    PicmanChannel *mask = picman_image_get_mask (image);
    GeglBuffer  *buffer;

    picman_image_undo_push_mask_precision (image, NULL, mask);

    buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                              picman_image_get_width  (image),
                                              picman_image_get_height (image)),
                              picman_image_get_mask_format (image));

    gegl_buffer_copy (picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)), NULL,
                      buffer, NULL);

    picman_drawable_set_buffer (PICMAN_DRAWABLE (mask), FALSE, NULL, buffer);
    g_object_unref (buffer);

    nth_drawable++;

    if (progress)
      picman_progress_set_value (progress,
                               (gdouble) nth_drawable / (gdouble) n_drawables);
  }

  picman_image_undo_group_end (image);

  picman_image_precision_changed (image);
  g_object_thaw_notify (G_OBJECT (image));

  if (progress)
    picman_progress_end (progress);
}
