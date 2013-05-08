/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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

#include <gegl.h>

#include "pdb-types.h"

#include "core/picman.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmancontainer.h"
#include "core/picmandatafactory.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanitem.h"

#include "text/picmantextlayer.h"

#include "vectors/picmanvectors.h"

#include "picmanpdb-utils.h"
#include "picmanpdberror.h"

#include "picman-intl.h"


static PicmanObject *
picman_pdb_get_data_factory_item (PicmanDataFactory *data_factory,
                                const gchar     *name)
{
  PicmanObject *picman_object;

  picman_object = picman_container_get_child_by_name (picman_data_factory_get_container (data_factory), name);

  if (! picman_object)
    picman_object = picman_container_get_child_by_name (picman_data_factory_get_container_obsolete (data_factory), name);

  return picman_object;
}


PicmanBrush *
picman_pdb_get_brush (Picman         *picman,
                    const gchar  *name,
                    gboolean      writable,
                    GError      **error)
{
  PicmanBrush *brush;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty brush name"));
      return NULL;
    }

  brush = (PicmanBrush *) picman_pdb_get_data_factory_item (picman->brush_factory, name);

  if (! brush)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Brush '%s' not found"), name);
    }
  else if (writable && ! picman_data_is_writable (PICMAN_DATA (brush)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Brush '%s' is not editable"), name);
      return NULL;
    }

  return brush;
}

PicmanBrush *
picman_pdb_get_generated_brush (Picman         *picman,
                              const gchar  *name,
                              gboolean      writable,
                              GError      **error)
{
  PicmanBrush *brush;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  brush = picman_pdb_get_brush (picman, name, writable, error);

  if (! brush)
    return NULL;

  if (! PICMAN_IS_BRUSH_GENERATED (brush))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Brush '%s' is not a generated brush"), name);
      return NULL;
    }

  return brush;
}

PicmanDynamics *
picman_pdb_get_dynamics (Picman         *picman,
                       const gchar  *name,
                       gboolean      writable,
                       GError      **error)
{
  PicmanDynamics *dynamics;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty paint dynamics name"));
      return NULL;
    }

  dynamics = (PicmanDynamics *) picman_pdb_get_data_factory_item (picman->dynamics_factory, name);

  if (! dynamics)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Paint dynamics '%s' not found"), name);
    }
  else if (writable && ! picman_data_is_writable (PICMAN_DATA (dynamics)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Paint dynamics '%s' is not editable"), name);
      return NULL;
    }

  return dynamics;
}

PicmanPattern *
picman_pdb_get_pattern (Picman         *picman,
                      const gchar  *name,
                      GError      **error)
{
  PicmanPattern *pattern;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty pattern name"));
      return NULL;
    }

  pattern = (PicmanPattern *) picman_pdb_get_data_factory_item (picman->pattern_factory, name);

  if (! pattern)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Pattern '%s' not found"), name);
    }

  return pattern;
}

PicmanGradient *
picman_pdb_get_gradient (Picman         *picman,
                       const gchar  *name,
                       gboolean      writable,
                       GError      **error)
{
  PicmanGradient *gradient;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty gradient name"));
      return NULL;
    }

  gradient = (PicmanGradient *) picman_pdb_get_data_factory_item (picman->gradient_factory, name);

  if (! gradient)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Gradient '%s' not found"), name);
    }
  else if (writable && ! picman_data_is_writable (PICMAN_DATA (gradient)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Gradient '%s' is not editable"), name);
      return NULL;
    }

  return gradient;
}

PicmanPalette *
picman_pdb_get_palette (Picman         *picman,
                      const gchar  *name,
                      gboolean      writable,
                      GError      **error)
{
  PicmanPalette *palette;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty palette name"));
      return NULL;
    }

  palette = (PicmanPalette *) picman_pdb_get_data_factory_item (picman->palette_factory, name);

  if (! palette)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Palette '%s' not found"), name);
    }
  else if (writable && ! picman_data_is_writable (PICMAN_DATA (palette)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Palette '%s' is not editable"), name);
      return NULL;
    }

  return palette;
}

PicmanFont *
picman_pdb_get_font (Picman         *picman,
                   const gchar  *name,
                   GError      **error)
{
  PicmanFont *font;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty font name"));
      return NULL;
    }

  font = (PicmanFont *)
    picman_container_get_child_by_name (picman->fonts, name);

  if (! font)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Font '%s' not found"), name);
    }

  return font;
}

PicmanBuffer *
picman_pdb_get_buffer (Picman         *picman,
                     const gchar  *name,
                     GError      **error)
{
  PicmanBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty buffer name"));
      return NULL;
    }

  buffer = (PicmanBuffer *)
    picman_container_get_child_by_name (picman->named_buffers, name);

  if (! buffer)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Named buffer '%s' not found"), name);
    }

  return buffer;
}

PicmanPaintInfo *
picman_pdb_get_paint_info (Picman         *picman,
                         const gchar  *name,
                         GError      **error)
{
  PicmanPaintInfo *paint_info;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! name || ! strlen (name))
    {
      g_set_error_literal (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
			   _("Invalid empty paint method name"));
      return NULL;
    }

  paint_info = (PicmanPaintInfo *)
    picman_container_get_child_by_name (picman->paint_info_list, name);

  if (! paint_info)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Paint method '%s' does not exist"), name);
    }

  return paint_info;
}

gboolean
picman_pdb_item_is_attached (PicmanItem           *item,
                           PicmanImage          *image,
                           PicmanPDBItemModify   modify,
                           GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_item_is_attached (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be used because it has not "
                     "been added to an image"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  if (image && image != picman_item_get_image (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be used because it is "
                     "attached to another image"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return picman_pdb_item_is_modifyable (item, modify, error);
}

gboolean
picman_pdb_item_is_in_tree (PicmanItem           *item,
                          PicmanImage          *image,
                          PicmanPDBItemModify   modify,
                          GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_pdb_item_is_attached (item, image, modify, error))
    return FALSE;

  if (! picman_item_get_tree (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be used because it is not "
                     "a direct child of an item tree"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_in_same_tree (PicmanItem   *item,
                               PicmanItem   *item2,
                               PicmanImage  *image,
                               GError    **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (item2), FALSE);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_pdb_item_is_in_tree (item, image, FALSE, error) ||
      ! picman_pdb_item_is_in_tree (item2, image, FALSE, error))
    return FALSE;

  if (picman_item_get_tree (item) != picman_item_get_tree (item2))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Items '%s' (%d) and '%s' (%d) cannot be used "
                     "because they are not part of the same item tree"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item),
                   picman_object_get_name (item2),
                   picman_item_get_ID (item2));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_not_ancestor (PicmanItem  *item,
                               PicmanItem  *not_descendant,
                               GError   **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (PICMAN_IS_ITEM (not_descendant), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_viewable_is_ancestor (PICMAN_VIEWABLE (item),
                                 PICMAN_VIEWABLE (not_descendant)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) must not be an ancestor of "
                     "'%s' (%d)"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item),
                   picman_object_get_name (not_descendant),
                   picman_item_get_ID (not_descendant));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_floating (PicmanItem  *item,
                           PicmanImage *dest_image,
                           GError   **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (dest_image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! g_object_is_floating (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) has already been added to an image"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }
  else if (picman_item_get_image (item) != dest_image)
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Trying to add item '%s' (%d) to wrong image"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_modifyable (PicmanItem           *item,
                             PicmanPDBItemModify   modify,
                             GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if ((modify & PICMAN_PDB_ITEM_CONTENT) && picman_item_is_content_locked (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be modified because its "
                     "contents are locked"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  if ((modify & PICMAN_PDB_ITEM_POSITION) && picman_item_is_position_locked (item))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be modified because its "
                     "position and size are locked"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_group (PicmanItem  *item,
                        GError   **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_viewable_get_children (PICMAN_VIEWABLE (item)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be used because it is "
                     "not a group item"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_item_is_not_group (PicmanItem  *item,
                            GError   **error)
{
  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Item '%s' (%d) cannot be modified because it "
                     "is a group item"),
                   picman_object_get_name (item),
                   picman_item_get_ID (item));
      return FALSE;
    }

  return TRUE;
}

gboolean
picman_pdb_layer_is_text_layer (PicmanLayer          *layer,
                              PicmanPDBItemModify   modify,
                              GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_item_is_text_layer (PICMAN_ITEM (layer)))
    {
      g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                   _("Layer '%s' (%d) cannot be used because it is not "
                     "a text layer"),
                   picman_object_get_name (layer),
                   picman_item_get_ID (PICMAN_ITEM (layer)));

      return FALSE;
    }

  return picman_pdb_item_is_attached (PICMAN_ITEM (layer), NULL, modify, error);
}

static const gchar *
picman_pdb_enum_value_get_nick (GType enum_type,
                              gint  value)
{
  GEnumClass  *enum_class;
  GEnumValue  *enum_value;
  const gchar *nick;

  enum_class = g_type_class_ref (enum_type);
  enum_value = g_enum_get_value (enum_class, value);

  nick = enum_value->value_nick;

  g_type_class_unref (enum_class);

  return nick;
}

gboolean
picman_pdb_image_is_base_type (PicmanImage          *image,
                             PicmanImageBaseType   type,
                             GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_image_get_base_type (image) == type)
    return TRUE;

  g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
               _("Image '%s' (%d) is of type '%s', "
                 "but an image of type '%s' is expected"),
               picman_image_get_display_name (image),
               picman_image_get_ID (image),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_IMAGE_BASE_TYPE,
                                             picman_image_get_base_type (image)),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_IMAGE_BASE_TYPE, type));

  return FALSE;
}

gboolean
picman_pdb_image_is_not_base_type (PicmanImage          *image,
                                 PicmanImageBaseType   type,
                                 GError            **error)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_image_get_base_type (image) != type)
    return TRUE;

  g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
               _("Image '%s' (%d) must not be of type '%s'"),
               picman_image_get_display_name (image),
               picman_image_get_ID (image),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_IMAGE_BASE_TYPE, type));

  return FALSE;
}

gboolean
picman_pdb_image_is_precision (PicmanImage      *image,
                             PicmanPrecision   precision,
                             GError        **error)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_image_get_precision (image) == precision)
    return TRUE;

  g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
               _("Image '%s' (%d) has precision '%s', "
                 "but an image of precision '%s' is expected"),
               picman_image_get_display_name (image),
               picman_image_get_ID (image),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_PRECISION,
                                             picman_image_get_precision (image)),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_PRECISION, precision));

  return FALSE;
}

gboolean
picman_pdb_image_is_not_precision (PicmanImage      *image,
                                 PicmanPrecision   precision,
                                 GError        **error)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_image_get_precision (image) != precision)
    return TRUE;

  g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
               _("Image '%s' (%d) must not be of precision '%s'"),
               picman_image_get_display_name (image),
               picman_image_get_ID (image),
               picman_pdb_enum_value_get_nick (PICMAN_TYPE_PRECISION, precision));

  return FALSE;
}

PicmanStroke *
picman_pdb_get_vectors_stroke (PicmanVectors        *vectors,
                             gint                stroke_ID,
                             PicmanPDBItemModify   modify,
                             GError            **error)
{
  PicmanStroke *stroke = NULL;

  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! picman_pdb_item_is_not_group (PICMAN_ITEM (vectors), error))
    return NULL;

  if (! modify || picman_pdb_item_is_modifyable (PICMAN_ITEM (vectors),
                                               modify, error))
    {
      stroke = picman_vectors_stroke_get_by_ID (vectors, stroke_ID);

      if (! stroke)
        g_set_error (error, PICMAN_PDB_ERROR, PICMAN_PDB_ERROR_INVALID_ARGUMENT,
                     _("Vectors object %d does not contain stroke with ID %d"),
                     picman_item_get_ID (PICMAN_ITEM (vectors)), stroke_ID);
    }

  return stroke;
}
