/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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
#include "text-types.h"

#include "core/picman.h"
#include "core/picmandrawable-private.h" /* eek */
#include "core/picmanimage.h"
#include "core/picmanparasitelist.h"

#include "picmantext.h"
#include "picmantext-parasite.h"
#include "picmantextlayer.h"
#include "picmantextlayer-xcf.h"

#include "picman-intl.h"


enum
{
  TEXT_LAYER_XCF_NONE              = 0,
  TEXT_LAYER_XCF_DONT_AUTO_RENAME  = 1 << 0,
  TEXT_LAYER_XCF_MODIFIED          = 1 << 1
};


static PicmanLayer * picman_text_layer_from_layer (PicmanLayer *layer,
                                               PicmanText  *text);


gboolean
picman_text_layer_xcf_load_hack (PicmanLayer **layer)
{
  const gchar        *name;
  PicmanText           *text = NULL;
  const PicmanParasite *parasite;

  g_return_val_if_fail (layer != NULL, FALSE);
  g_return_val_if_fail (PICMAN_IS_LAYER (*layer), FALSE);

  name = picman_text_parasite_name ();
  parasite = picman_item_parasite_find (PICMAN_ITEM (*layer), name);

  if (parasite)
    {
      GError *error = NULL;

      text = picman_text_from_parasite (parasite, &error);

      if (error)
        {
          picman_message (picman_item_get_image (PICMAN_ITEM (*layer))->picman, NULL,
                        PICMAN_MESSAGE_ERROR,
                        _("Problems parsing the text parasite for layer '%s':\n"
                          "%s\n\n"
                          "Some text properties may be wrong. "
                          "Unless you want to edit the text layer, "
                          "you don't need to worry about this."),
                        picman_object_get_name (*layer),
                        error->message);
          g_clear_error (&error);
        }
    }
  else
    {
      name = picman_text_gdyntext_parasite_name ();

      parasite = picman_item_parasite_find (PICMAN_ITEM (*layer), name);

      if (parasite)
        text = picman_text_from_gdyntext_parasite (parasite);
    }

  if (text)
    {
      *layer = picman_text_layer_from_layer (*layer, text);

      /*  let the text layer knows what parasite was used to create it  */
      PICMAN_TEXT_LAYER (*layer)->text_parasite = name;
    }

  return (text != NULL);
}

void
picman_text_layer_xcf_save_prepare (PicmanTextLayer *layer)
{
  PicmanText *text;

  g_return_if_fail (PICMAN_IS_TEXT_LAYER (layer));

  /*  If the layer has a text parasite already, it wasn't changed and we
   *  can simply save the original parasite back which is still attached.
   */
  if (layer->text_parasite)
    return;

  text = picman_text_layer_get_text (layer);
  if (text)
    {
      PicmanParasite *parasite = picman_text_to_parasite (text);

      /*  Don't push an undo because the parasite only exists temporarily
       *  while the text layer is saved to XCF.
       */
      picman_item_parasite_attach (PICMAN_ITEM (layer), parasite, FALSE);

      picman_parasite_free (parasite);
    }
}

guint32
picman_text_layer_get_xcf_flags (PicmanTextLayer *text_layer)
{
  guint flags = 0;

  g_return_val_if_fail (PICMAN_IS_TEXT_LAYER (text_layer), 0);

  if (! text_layer->auto_rename)
    flags |= TEXT_LAYER_XCF_DONT_AUTO_RENAME;

  if (text_layer->modified)
    flags |= TEXT_LAYER_XCF_MODIFIED;

  return flags;
}

void
picman_text_layer_set_xcf_flags (PicmanTextLayer *text_layer,
                               guint32        flags)
{
  g_return_if_fail (PICMAN_IS_TEXT_LAYER (text_layer));

  g_object_set (text_layer,
                "auto-rename", (flags & TEXT_LAYER_XCF_DONT_AUTO_RENAME) == 0,
                "modified",    (flags & TEXT_LAYER_XCF_MODIFIED)         != 0,
                NULL);
}


/**
 * picman_text_layer_from_layer:
 * @layer: a #PicmanLayer object
 * @text: a #PicmanText object
 *
 * Converts a standard #PicmanLayer and a #PicmanText object into a
 * #PicmanTextLayer. The new text layer takes ownership of the @text and
 * @layer objects.  The @layer object is rendered unusable by this
 * function. Don't even try to use if afterwards!
 *
 * This is a gross hack that is needed in order to load text layers
 * from XCF files in a backwards-compatible way. Please don't use it
 * for anything else!
 *
 * Return value: a newly allocated #PicmanTextLayer object
 **/
static PicmanLayer *
picman_text_layer_from_layer (PicmanLayer *layer,
                            PicmanText  *text)
{
  PicmanTextLayer *text_layer;
  PicmanDrawable  *drawable;

  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT (text), NULL);

  text_layer = g_object_new (PICMAN_TYPE_TEXT_LAYER,
                             "image", picman_item_get_image (PICMAN_ITEM (layer)),
                             NULL);

  picman_item_replace_item (PICMAN_ITEM (text_layer), PICMAN_ITEM (layer));

  drawable = PICMAN_DRAWABLE (text_layer);

  drawable->private->buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer));
  PICMAN_DRAWABLE (layer)->private->buffer = NULL;

  picman_layer_set_opacity    (PICMAN_LAYER (text_layer),
                             picman_layer_get_opacity (layer), FALSE);
  picman_layer_set_mode       (PICMAN_LAYER (text_layer),
                             picman_layer_get_mode (layer), FALSE);
  picman_layer_set_lock_alpha (PICMAN_LAYER (text_layer),
                             picman_layer_get_lock_alpha (layer), FALSE);

  picman_text_layer_set_text (text_layer, text);

  g_object_unref (text);
  g_object_unref (layer);

  return PICMAN_LAYER (text_layer);
}
