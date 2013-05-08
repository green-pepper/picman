/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "gegl/picman-babl.h"

#include "picman.h"
#include "picmanbuffer.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-new.h"
#include "picmanimage-undo.h"
#include "picmanlayer.h"
#include "picmantemplate.h"

#include "picman-intl.h"


PicmanTemplate *
picman_image_new_get_last_template (Picman      *picman,
                                  PicmanImage *image)
{
  PicmanTemplate *template;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), NULL);

  template = picman_template_new ("image new values");

  if (image)
    {
      picman_config_sync (G_OBJECT (picman->config->default_image),
                        G_OBJECT (template), 0);
      picman_template_set_from_image (template, image);
    }
  else
    {
      picman_config_sync (G_OBJECT (picman->image_new_last_template),
                        G_OBJECT (template), 0);
    }

  return template;
}

void
picman_image_new_set_last_template (Picman         *picman,
                                  PicmanTemplate *template)
{
  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (PICMAN_IS_TEMPLATE (template));

  picman_config_sync (G_OBJECT (template),
                    G_OBJECT (picman->image_new_last_template), 0);
}

PicmanImage *
picman_image_new_from_template (Picman         *picman,
                              PicmanTemplate *template,
                              PicmanContext  *context)
{
  PicmanImage   *image;
  PicmanLayer   *layer;
  gint         width, height;
  gboolean     has_alpha;
  const gchar *comment;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_TEMPLATE (template), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  image = picman_create_image (picman,
                             picman_template_get_width (template),
                             picman_template_get_height (template),
                             picman_template_get_base_type (template),
                             picman_template_get_precision (template),
                             FALSE);

  picman_image_undo_disable (image);

  comment = picman_template_get_comment (template);

  if (comment)
    {
      PicmanParasite *parasite;

      parasite = picman_parasite_new ("picman-comment",
                                    PICMAN_PARASITE_PERSISTENT,
                                    strlen (comment) + 1,
                                    comment);
      picman_image_parasite_attach (image, parasite);
      picman_parasite_free (parasite);
    }

  picman_image_set_resolution (image,
                             picman_template_get_resolution_x (template),
                             picman_template_get_resolution_y (template));
  picman_image_set_unit (image, picman_template_get_resolution_unit (template));

  width  = picman_image_get_width (image);
  height = picman_image_get_height (image);

  if (picman_template_get_fill_type (template) == PICMAN_TRANSPARENT_FILL)
    has_alpha = TRUE;
  else
    has_alpha = FALSE;

  layer = picman_layer_new (image, width, height,
                          picman_image_get_layer_format (image, has_alpha),
                          _("Background"),
                          PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_drawable_fill_by_type (PICMAN_DRAWABLE (layer),
                              context, picman_template_get_fill_type (template));

  picman_image_add_layer (image, layer, NULL, 0, FALSE);

  picman_image_undo_enable (image);
  picman_image_clean_all (image);

  picman_create_display (picman, image, picman_template_get_unit (template), 1.0);

  g_object_unref (image);

  return image;
}

PicmanImage *
picman_image_new_from_drawable (Picman         *picman,
                              PicmanDrawable *drawable)
{
  PicmanItem          *item;
  PicmanImage         *image;
  PicmanImage         *new_image;
  PicmanLayer         *new_layer;
  GType              new_type;
  gint               off_x, off_y;
  PicmanImageBaseType  type;
  gdouble            xres;
  gdouble            yres;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  item  = PICMAN_ITEM (drawable);
  image = picman_item_get_image (item);

  type = picman_drawable_get_base_type (drawable);

  new_image = picman_create_image (picman,
                                 picman_item_get_width  (item),
                                 picman_item_get_height (item),
                                 type,
                                 picman_drawable_get_precision (drawable),
                                 TRUE);
  picman_image_undo_disable (new_image);

  if (type == PICMAN_INDEXED)
    picman_image_set_colormap (new_image,
                             picman_image_get_colormap (image),
                             picman_image_get_colormap_size (image),
                             FALSE);

  picman_image_get_resolution (image, &xres, &yres);
  picman_image_set_resolution (new_image, xres, yres);
  picman_image_set_unit (new_image, picman_image_get_unit (image));

  if (PICMAN_IS_LAYER (drawable))
    new_type = G_TYPE_FROM_INSTANCE (drawable);
  else
    new_type = PICMAN_TYPE_LAYER;

  new_layer = PICMAN_LAYER (picman_item_convert (PICMAN_ITEM (drawable),
                                             new_image, new_type));

  picman_object_set_name (PICMAN_OBJECT (new_layer),
                        picman_object_get_name (drawable));

  picman_item_get_offset (PICMAN_ITEM (new_layer), &off_x, &off_y);
  picman_item_translate (PICMAN_ITEM (new_layer), -off_x, -off_y, FALSE);
  picman_item_set_visible (PICMAN_ITEM (new_layer), TRUE, FALSE);
  picman_item_set_linked (PICMAN_ITEM (new_layer), FALSE, FALSE);
  picman_layer_set_mode (new_layer, PICMAN_NORMAL_MODE, FALSE);
  picman_layer_set_opacity (new_layer, PICMAN_OPACITY_OPAQUE, FALSE);
  picman_layer_set_lock_alpha (new_layer, FALSE, FALSE);

  picman_image_add_layer (new_image, new_layer, NULL, 0, TRUE);

  picman_image_undo_enable (new_image);

  return new_image;
}

PicmanImage *
picman_image_new_from_component (Picman            *picman,
                               PicmanImage       *image,
                               PicmanChannelType  component)
{
  PicmanImage   *new_image;
  PicmanChannel *channel;
  PicmanLayer   *layer;
  const gchar *desc;
  gdouble      xres;
  gdouble      yres;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  new_image = picman_create_image (picman,
                                 picman_image_get_width  (image),
                                 picman_image_get_height (image),
                                 PICMAN_GRAY,
                                 picman_image_get_precision (image),
                                 TRUE);

  picman_image_undo_disable (new_image);

  picman_image_get_resolution (image, &xres, &yres);
  picman_image_set_resolution (new_image, xres, yres);
  picman_image_set_unit (new_image, picman_image_get_unit (image));

  channel = picman_channel_new_from_component (image, component, NULL, NULL);

  layer = PICMAN_LAYER (picman_item_convert (PICMAN_ITEM (channel),
                                         new_image, PICMAN_TYPE_LAYER));
  g_object_unref (channel);

  picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                       NULL, NULL, &desc, NULL);
  picman_object_take_name (PICMAN_OBJECT (layer),
                         g_strdup_printf (_("%s Channel Copy"), desc));

  picman_image_add_layer (new_image, layer, NULL, 0, TRUE);

  picman_image_undo_enable (new_image);

  return new_image;
}

PicmanImage *
picman_image_new_from_buffer (Picman       *picman,
                            PicmanImage  *invoke,
                            PicmanBuffer *paste)
{
  PicmanImage  *image;
  PicmanLayer  *layer;
  const Babl *format;
  gboolean    has_alpha;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (invoke == NULL || PICMAN_IS_IMAGE (invoke), NULL);
  g_return_val_if_fail (PICMAN_IS_BUFFER (paste), NULL);

  format    = picman_buffer_get_format (paste);
  has_alpha = babl_format_has_alpha (format);

  /*  create a new image  (always of type PICMAN_RGB)  */
  image = picman_create_image (picman,
                             picman_buffer_get_width  (paste),
                             picman_buffer_get_height (paste),
                             picman_babl_format_get_base_type (format),
                             picman_babl_format_get_precision (format),
                             TRUE);
  picman_image_undo_disable (image);

  if (invoke)
    {
      gdouble xres;
      gdouble yres;

      picman_image_get_resolution (invoke, &xres, &yres);
      picman_image_set_resolution (image, xres, yres);
      picman_image_set_unit (image, picman_image_get_unit (invoke));
    }

  layer = picman_layer_new_from_buffer (picman_buffer_get_buffer (paste),
                                      image,
                                      picman_image_get_layer_format (image,
                                                                   has_alpha),
                                      _("Pasted Layer"),
                                      PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_image_add_layer (image, layer, NULL, 0, TRUE);

  picman_image_undo_enable (image);

  return image;
}

PicmanImage *
picman_image_new_from_pixbuf (Picman        *picman,
                            GdkPixbuf   *pixbuf,
                            const gchar *layer_name)
{
  PicmanImage         *new_image;
  PicmanLayer         *layer;
  PicmanImageBaseType  base_type;
  gboolean           has_alpha = FALSE;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

  switch (gdk_pixbuf_get_n_channels (pixbuf))
    {
    case 2: has_alpha = TRUE;
    case 1: base_type = PICMAN_GRAY;
      break;

    case 4: has_alpha = TRUE;
    case 3: base_type = PICMAN_RGB;
      break;

    default:
      g_return_val_if_reached (NULL);
    }

  new_image = picman_create_image (picman,
                                 gdk_pixbuf_get_width  (pixbuf),
                                 gdk_pixbuf_get_height (pixbuf),
                                 base_type,
                                 PICMAN_PRECISION_U8,
                                 FALSE);

  picman_image_undo_disable (new_image);

  layer = picman_layer_new_from_pixbuf (pixbuf, new_image,
                                      picman_image_get_layer_format (new_image,
                                                                   has_alpha),
                                      layer_name,
                                      PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_image_add_layer (new_image, layer, NULL, 0, TRUE);

  picman_image_undo_enable (new_image);

  return new_image;
}
