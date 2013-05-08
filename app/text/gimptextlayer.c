/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextLayer
 * Copyright (C) 2002-2004  Sven Neumann <sven@picman.org>
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

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pangocairo.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "text-types.h"

#include "gegl/picman-babl.h"
#include "gegl/picman-gegl-utils.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmancontext.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanitemtree.h"
#include "core/picmanparasitelist.h"

#include "picmantext.h"
#include "picmantextlayer.h"
#include "picmantextlayer-transform.h"
#include "picmantextlayout.h"
#include "picmantextlayout-render.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_TEXT,
  PROP_AUTO_RENAME,
  PROP_MODIFIED
};


static void       picman_text_layer_finalize       (GObject           *object);
static void       picman_text_layer_get_property   (GObject           *object,
                                                  guint              property_id,
                                                  GValue            *value,
                                                  GParamSpec        *pspec);
static void       picman_text_layer_set_property   (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);

static gint64     picman_text_layer_get_memsize    (PicmanObject        *object,
                                                  gint64            *gui_size);

static PicmanItem * picman_text_layer_duplicate      (PicmanItem          *item,
                                                  GType              new_type);
static gboolean   picman_text_layer_rename         (PicmanItem          *item,
                                                  const gchar       *new_name,
                                                  const gchar       *undo_desc,
                                                  GError           **error);

static void       picman_text_layer_convert_type   (PicmanDrawable      *drawable,
                                                  PicmanImage         *dest_image,
                                                  const Babl        *new_format,
                                                  PicmanImageBaseType  new_base_type,
                                                  PicmanPrecision      new_precision,
                                                  gint               layer_dither_type,
                                                  gint               mask_dither_type,
                                                  gboolean           push_undo);
static void       picman_text_layer_set_buffer     (PicmanDrawable      *drawable,
                                                  gboolean           push_undo,
                                                  const gchar       *undo_desc,
                                                  GeglBuffer        *buffer,
                                                  gint               offset_x,
                                                  gint               offset_y);
static void       picman_text_layer_push_undo      (PicmanDrawable      *drawable,
                                                  const gchar       *undo_desc,
                                                  GeglBuffer        *buffer,
                                                  gint               x,
                                                  gint               y,
                                                  gint               width,
                                                  gint               height);

static void       picman_text_layer_text_changed   (PicmanTextLayer     *layer);
static gboolean   picman_text_layer_render         (PicmanTextLayer     *layer);
static void       picman_text_layer_render_layout  (PicmanTextLayer     *layer,
                                                  PicmanTextLayout    *layout);


G_DEFINE_TYPE (PicmanTextLayer, picman_text_layer, PICMAN_TYPE_LAYER)

#define parent_class picman_text_layer_parent_class


static void
picman_text_layer_class_init (PicmanTextLayerClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class    = PICMAN_DRAWABLE_CLASS (klass);

  object_class->finalize           = picman_text_layer_finalize;
  object_class->get_property       = picman_text_layer_get_property;
  object_class->set_property       = picman_text_layer_set_property;

  picman_object_class->get_memsize   = picman_text_layer_get_memsize;

  viewable_class->default_stock_id = "picman-text-layer";

  item_class->duplicate            = picman_text_layer_duplicate;
  item_class->rename               = picman_text_layer_rename;

#if 0
  item_class->scale                = picman_text_layer_scale;
  item_class->flip                 = picman_text_layer_flip;
  item_class->rotate               = picman_text_layer_rotate;
  item_class->transform            = picman_text_layer_transform;
#endif

  item_class->default_name         = _("Text Layer");
  item_class->rename_desc          = _("Rename Text Layer");
  item_class->translate_desc       = _("Move Text Layer");
  item_class->scale_desc           = _("Scale Text Layer");
  item_class->resize_desc          = _("Resize Text Layer");
  item_class->flip_desc            = _("Flip Text Layer");
  item_class->rotate_desc          = _("Rotate Text Layer");
  item_class->transform_desc       = _("Transform Text Layer");

  drawable_class->convert_type     = picman_text_layer_convert_type;
  drawable_class->set_buffer       = picman_text_layer_set_buffer;
  drawable_class->push_undo        = picman_text_layer_push_undo;

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_TEXT,
                                   "text", NULL,
                                   PICMAN_TYPE_TEXT,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_AUTO_RENAME,
                                    "auto-rename", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_MODIFIED,
                                    "modified", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_text_layer_init (PicmanTextLayer *layer)
{
  layer->text          = NULL;
  layer->text_parasite = NULL;
}

static void
picman_text_layer_finalize (GObject *object)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (object);

  if (layer->text)
    {
      g_object_unref (layer->text);
      layer->text = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_text_layer_get_property (GObject      *object,
                              guint         property_id,
                              GValue       *value,
                              GParamSpec   *pspec)
{
  PicmanTextLayer *text_layer = PICMAN_TEXT_LAYER (object);

  switch (property_id)
    {
    case PROP_TEXT:
      g_value_set_object (value, text_layer->text);
      break;
    case PROP_AUTO_RENAME:
      g_value_set_boolean (value, text_layer->auto_rename);
      break;
    case PROP_MODIFIED:
      g_value_set_boolean (value, text_layer->modified);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_text_layer_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanTextLayer *text_layer = PICMAN_TEXT_LAYER (object);

  switch (property_id)
    {
    case PROP_TEXT:
      picman_text_layer_set_text (text_layer, g_value_get_object (value));
      break;
    case PROP_AUTO_RENAME:
      text_layer->auto_rename = g_value_get_boolean (value);
      break;
    case PROP_MODIFIED:
      text_layer->modified = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_text_layer_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanTextLayer *text_layer = PICMAN_TEXT_LAYER (object);
  gint64         memsize    = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (text_layer->text),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static PicmanItem *
picman_text_layer_duplicate (PicmanItem *item,
                           GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_TEXT_LAYER (new_item))
    {
      PicmanTextLayer *layer     = PICMAN_TEXT_LAYER (item);
      PicmanTextLayer *new_layer = PICMAN_TEXT_LAYER (new_item);

      picman_config_sync (G_OBJECT (layer), G_OBJECT (new_layer), 0);

      if (layer->text)
        {
          PicmanText *text = picman_config_duplicate (PICMAN_CONFIG (layer->text));

          picman_text_layer_set_text (new_layer, text);

          g_object_unref (text);
        }

      /*  this is just the parasite name, not a pointer to the parasite  */
      if (layer->text_parasite)
        new_layer->text_parasite = layer->text_parasite;
    }

  return new_item;
}

static gboolean
picman_text_layer_rename (PicmanItem     *item,
                        const gchar  *new_name,
                        const gchar  *undo_desc,
                        GError      **error)
{
  if (PICMAN_ITEM_CLASS (parent_class)->rename (item, new_name, undo_desc, error))
    {
      g_object_set (item, "auto-rename", FALSE, NULL);

      return TRUE;
    }

  return FALSE;
}

static void
picman_text_layer_convert_type (PicmanDrawable      *drawable,
                              PicmanImage         *dest_image,
                              const Babl        *new_format,
                              PicmanImageBaseType  new_base_type,
                              PicmanPrecision      new_precision,
                              gint               layer_dither_type,
                              gint               mask_dither_type,
                              gboolean           push_undo)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (drawable);
  PicmanImage     *image = picman_item_get_image (PICMAN_ITEM (layer));

  if (! layer->text || layer->modified || layer_dither_type != 0)
    {
      PICMAN_DRAWABLE_CLASS (parent_class)->convert_type (drawable, dest_image,
                                                        new_format,
                                                        new_base_type,
                                                        new_precision,
                                                        layer_dither_type,
                                                        mask_dither_type,
                                                        push_undo);
    }
  else
    {
      if (push_undo)
        picman_image_undo_push_text_layer_convert (image, NULL, layer);

      layer->convert_format = new_format;

      picman_text_layer_render (layer);

      layer->convert_format = NULL;
    }
}

static void
picman_text_layer_set_buffer (PicmanDrawable *drawable,
                            gboolean      push_undo,
                            const gchar  *undo_desc,
                            GeglBuffer   *buffer,
                            gint          offset_x,
                            gint          offset_y)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (drawable);
  PicmanImage     *image = picman_item_get_image (PICMAN_ITEM (layer));

  if (push_undo && ! layer->modified)
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_DRAWABLE_MOD,
                                 undo_desc);

  PICMAN_DRAWABLE_CLASS (parent_class)->set_buffer (drawable,
                                                  push_undo, undo_desc,
                                                  buffer,
                                                  offset_x, offset_y);

  if (push_undo && ! layer->modified)
    {
      picman_image_undo_push_text_layer_modified (image, NULL, layer);

      g_object_set (drawable, "modified", TRUE, NULL);

      picman_image_undo_group_end (image);
    }
}

static void
picman_text_layer_push_undo (PicmanDrawable *drawable,
                           const gchar  *undo_desc,
                           GeglBuffer   *buffer,
                           gint          x,
                           gint          y,
                           gint          width,
                           gint          height)
{
  PicmanTextLayer *layer = PICMAN_TEXT_LAYER (drawable);
  PicmanImage     *image = picman_item_get_image (PICMAN_ITEM (layer));

  if (! layer->modified)
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_DRAWABLE, undo_desc);

  PICMAN_DRAWABLE_CLASS (parent_class)->push_undo (drawable, undo_desc,
                                                 buffer,
                                                 x, y, width, height);

  if (! layer->modified)
    {
      picman_image_undo_push_text_layer_modified (image, NULL, layer);

      g_object_set (drawable, "modified", TRUE, NULL);

      picman_image_undo_group_end (image);
    }
}


/*  public functions  */

/**
 * picman_text_layer_new:
 * @image: the #PicmanImage the layer should belong to
 * @text: a #PicmanText object
 *
 * Creates a new text layer.
 *
 * Return value: a new #PicmanTextLayer or %NULL in case of a problem
 **/
PicmanLayer *
picman_text_layer_new (PicmanImage *image,
                     PicmanText  *text)
{
  PicmanTextLayer *layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT (text), NULL);

  if (! text->text && ! text->markup)
    return NULL;

  layer =
    PICMAN_TEXT_LAYER (picman_drawable_new (PICMAN_TYPE_TEXT_LAYER,
                                        image, NULL,
                                        0, 0, 1, 1,
                                        picman_image_get_layer_format (image,
                                                                     TRUE)));

  picman_text_layer_set_text (layer, text);

  if (! picman_text_layer_render (layer))
    {
      g_object_unref (layer);
      return NULL;
    }

  return PICMAN_LAYER (layer);
}

void
picman_text_layer_set_text (PicmanTextLayer *layer,
                          PicmanText      *text)
{
  g_return_if_fail (PICMAN_IS_TEXT_LAYER (layer));
  g_return_if_fail (text == NULL || PICMAN_IS_TEXT (text));

  if (layer->text == text)
    return;

  if (layer->text)
    {
      g_signal_handlers_disconnect_by_func (layer->text,
                                            G_CALLBACK (picman_text_layer_text_changed),
                                            layer);

      g_object_unref (layer->text);
      layer->text = NULL;
    }

  if (text)
    {
      layer->text = g_object_ref (text);

      g_signal_connect_object (text, "changed",
                               G_CALLBACK (picman_text_layer_text_changed),
                               layer, G_CONNECT_SWAPPED);
    }

  g_object_notify (G_OBJECT (layer), "text");
  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (layer));
}

PicmanText *
picman_text_layer_get_text (PicmanTextLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_TEXT_LAYER (layer), NULL);

  return layer->text;
}

void
picman_text_layer_set (PicmanTextLayer *layer,
                     const gchar   *undo_desc,
                     const gchar   *first_property_name,
                     ...)
{
  PicmanImage *image;
  PicmanText  *text;
  va_list    var_args;

  g_return_if_fail (picman_item_is_text_layer (PICMAN_ITEM (layer)));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)));

  text = picman_text_layer_get_text (layer);
  if (! text)
    return;

  image = picman_item_get_image (PICMAN_ITEM (layer));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TEXT, undo_desc);

  g_object_freeze_notify (G_OBJECT (layer));

  if (layer->modified)
    {
      picman_image_undo_push_text_layer_modified (image, NULL, layer);

      /*  pass copy_tiles = TRUE so we not only ref the tiles; after
       *  being a text layer again, undo doesn't care about the
       *  layer's pixels any longer because they are generated, so
       *  changing the text would happily overwrite the layer's
       *  pixels, changing the pixels on the undo stack too without
       *  any chance to ever undo again.
       */
      picman_image_undo_push_drawable_mod (image, NULL,
                                         PICMAN_DRAWABLE (layer), TRUE);
    }

  picman_image_undo_push_text_layer (image, undo_desc, layer, NULL);

  va_start (var_args, first_property_name);

  g_object_set_valist (G_OBJECT (text), first_property_name, var_args);

  va_end (var_args);

  g_object_set (layer, "modified", FALSE, NULL);

  g_object_thaw_notify (G_OBJECT (layer));

  picman_image_undo_group_end (image);
}

/**
 * picman_text_layer_discard:
 * @layer: a #PicmanTextLayer
 *
 * Discards the text information. This makes @layer behave like a
 * normal layer.
 */
void
picman_text_layer_discard (PicmanTextLayer *layer)
{
  g_return_if_fail (PICMAN_IS_TEXT_LAYER (layer));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)));

  if (! layer->text)
    return;

  picman_image_undo_push_text_layer (picman_item_get_image (PICMAN_ITEM (layer)),
                                   _("Discard Text Information"),
                                   layer, NULL);

  picman_text_layer_set_text (layer, NULL);
}

gboolean
picman_item_is_text_layer (PicmanItem *item)
{
  return (PICMAN_IS_TEXT_LAYER (item)    &&
          PICMAN_TEXT_LAYER (item)->text &&
          PICMAN_TEXT_LAYER (item)->modified == FALSE);
}


/*  private functions  */

static const Babl *
picman_text_layer_get_format (PicmanTextLayer *layer)
{
  if (layer->convert_format)
    return layer->convert_format;

  return picman_drawable_get_format (PICMAN_DRAWABLE (layer));
}

static void
picman_text_layer_text_changed (PicmanTextLayer *layer)
{
  /*  If the text layer was created from a parasite, it's time to
   *  remove that parasite now.
   */
  if (layer->text_parasite)
    {
      /*  Don't push an undo because the parasite only exists temporarily
       *  while the text layer is loaded from XCF.
       */
      picman_item_parasite_detach (PICMAN_ITEM (layer), layer->text_parasite,
                                 FALSE);
      layer->text_parasite = NULL;
    }

  picman_text_layer_render (layer);
}

static gboolean
picman_text_layer_render (PicmanTextLayer *layer)
{
  PicmanDrawable   *drawable;
  PicmanItem       *item;
  PicmanImage      *image;
  PicmanTextLayout *layout;
  gdouble         xres;
  gdouble         yres;
  gint            width;
  gint            height;

  if (! layer->text)
    return FALSE;

  drawable = PICMAN_DRAWABLE (layer);
  item     = PICMAN_ITEM (layer);
  image    = picman_item_get_image (item);

  if (picman_container_is_empty (image->picman->fonts))
    {
      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_ERROR,
			    _("Due to lack of any fonts, "
			      "text functionality is not available."));
      return FALSE;
    }

  picman_image_get_resolution (image, &xres, &yres);

  layout = picman_text_layout_new (layer->text, xres, yres);

  g_object_freeze_notify (G_OBJECT (drawable));

  if (picman_text_layout_get_size (layout, &width, &height) &&
      (width  != picman_item_get_width  (item) ||
       height != picman_item_get_height (item) ||
       picman_text_layer_get_format (layer) !=
       picman_drawable_get_format (drawable)))
    {
      GeglBuffer *new_buffer;

      new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                    picman_text_layer_get_format (layer));
      picman_drawable_set_buffer (drawable, FALSE, NULL, new_buffer);
      g_object_unref (new_buffer);

      if (picman_layer_get_mask (PICMAN_LAYER (layer)))
        {
          PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (layer));

          static PicmanContext *unused_eek = NULL;

          if (! unused_eek)
            unused_eek = picman_context_new (image->picman, "eek", NULL);

          picman_item_resize (PICMAN_ITEM (mask), unused_eek, width, height, 0, 0);
        }
    }

  if (layer->auto_rename)
    {
      PicmanItem *item = PICMAN_ITEM (layer);
      gchar    *name = NULL;

      if (layer->text->text)
        {
          name = picman_utf8_strtrim (layer->text->text, 30);
        }
      else if (layer->text->markup)
        {
          gchar *tmp = picman_markup_extract_text (layer->text->markup);
          name = picman_utf8_strtrim (tmp, 30);
          g_free (tmp);
        }

      if (! name)
        name = g_strdup (_("Empty Text Layer"));

      if (picman_item_is_attached (item))
        {
          picman_item_tree_rename_item (picman_item_get_tree (item), item,
                                      name, FALSE, NULL);
          g_free (name);
        }
      else
        {
          picman_object_take_name (PICMAN_OBJECT (layer), name);
        }
    }

  picman_text_layer_render_layout (layer, layout);

  g_object_unref (layout);

  g_object_thaw_notify (G_OBJECT (drawable));

  return (width > 0 && height > 0);
}

static void
picman_text_layer_render_layout (PicmanTextLayer  *layer,
                               PicmanTextLayout *layout)
{
  PicmanDrawable    *drawable = PICMAN_DRAWABLE (layer);
  PicmanItem        *item     = PICMAN_ITEM (layer);
  GeglBuffer      *buffer;
  cairo_t         *cr;
  cairo_surface_t *surface;
  gint             width;
  gint             height;

  g_return_if_fail (picman_drawable_has_alpha (drawable));

  width  = picman_item_get_width  (item);
  height = picman_item_get_height (item);

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

  cr = cairo_create (surface);
  picman_text_layout_render (layout, cr, layer->text->base_dir, FALSE);
  cairo_destroy (cr);

  cairo_surface_flush (surface);

  buffer = picman_cairo_surface_create_buffer (surface);

  gegl_buffer_copy (buffer, NULL,
                    picman_drawable_get_buffer (drawable), NULL);

  g_object_unref (buffer);
  cairo_surface_destroy (surface);

  picman_drawable_update (drawable, 0, 0, width, height);
}
