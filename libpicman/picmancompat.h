/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancompat.h
 * Compatibility defines to ease migration from the PICMAN-1.2 API
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_COMPAT_H__
#define __PICMAN_COMPAT_H__

G_BEGIN_DECLS

/* This file contains aliases that are kept for historical reasons,
 * because a wide code base depends on them. We suggest that you
 * only use this header temporarily while porting a plug-in to the
 * new API.
 *
 * These defines will be removed in the next development cycle.
 */

#ifndef PICMAN_DISABLE_DEPRECATED


#define PicmanRunModeType                         PicmanRunMode
#define PicmanExportReturnType                    PicmanExportReturn

#define picman_use_xshm()                         (TRUE)
#define picman_color_cube                         ((guchar *) { 6, 6, 4, 24 })

#define picman_blend                              picman_edit_blend
#define picman_bucket_fill                        picman_edit_bucket_fill
#define picman_color_picker                       picman_image_pick_color
#define picman_convert_rgb                        picman_image_convert_rgb
#define picman_convert_grayscale                  picman_image_convert_grayscale
#define picman_convert_indexed                    picman_image_convert_indexed
#define picman_crop                               picman_image_crop

#define picman_channel_get_image_id               picman_drawable_get_image
#define picman_channel_delete                     picman_drawable_delete
#define picman_channel_get_name                   picman_drawable_get_name
#define picman_channel_set_name                   picman_drawable_set_name
#define picman_channel_get_visible                picman_drawable_get_visible
#define picman_channel_set_visible                picman_drawable_set_visible
#define picman_channel_get_tattoo                 picman_drawable_get_tattoo
#define picman_channel_set_tattoo                 picman_drawable_set_tattoo

#define picman_channel_ops_offset                 picman_drawable_offset
#define picman_channel_ops_duplicate              picman_image_duplictate

#define picman_layer_get_image_id                 picman_drawable_get_image
#define picman_layer_delete                       picman_drawable_delete
#define picman_layer_get_name                     picman_drawable_get_name
#define picman_layer_set_name                     picman_drawable_set_name
#define picman_layer_get_visible                  picman_drawable_get_visible
#define picman_layer_set_visible                  picman_drawable_set_visible
#define picman_layer_get_linked                   picman_drawable_get_linked
#define picman_layer_set_linked                   picman_drawable_set_linked
#define picman_layer_get_tattoo                   picman_drawable_get_tattoo
#define picman_layer_set_tattoo                   picman_drawable_set_tattoo
#define picman_layer_is_floating_selection        picman_layer_is_floating_sel
#define picman_layer_get_preserve_transparency    picman_layer_get_preserve_trans
#define picman_layer_set_preserve_transparency    picman_layer_set_preserve_trans

#define picman_layer_mask                         picman_layer_get_mask
#define picman_layer_get_mask_id                  picman_layer_get_mask

#define picman_drawable_image                     picman_drawable_get_image
#define picman_drawable_image_id                  picman_drawable_get_image
#define picman_drawable_name                      picman_drawable_get_name
#define picman_drawable_visible                   picman_drawable_get_visible
#define picman_drawable_bytes                     picman_drawable_bpp

#define picman_image_active_drawable              picman_image_get_active_drawable
#define picman_image_floating_selection           picman_image_get_floating_sel
#define picman_image_add_layer_mask(i,l,m)        picman_layer_add_mask(l,m)
#define picman_image_remove_layer_mask(i,l,m)     picman_layer_remove_mask(l,m)

#define picman_gradients_get_active               picman_gradients_get_gradient
#define picman_gradients_set_active               picman_gradients_set_gradient

#define picman_palette_refresh                    picman_palettes_refresh

#define picman_temp_PDB_name                      picman_procedural_db_temp_name

#define picman_undo_push_group_start              picman_image_undo_group_start
#define picman_undo_push_group_end                picman_image_undo_group_end

#define picman_help_init()                        ((void) 0)
#define picman_help_free()                        ((void) 0)

#define picman_interactive_selection_brush        picman_brush_select_new
#define picman_brush_select_widget                picman_brush_select_widget_new
#define picman_brush_select_widget_set_popup      picman_brush_select_widget_set
#define picman_brush_select_widget_close_popup    picman_brush_select_widget_close

#define picman_interactive_selection_font         picman_font_select_new
#define picman_gradient_select_widget             picman_gradient_select_widget_new
#define picman_gradient_select_widget_set_popup   picman_gradient_select_widget_set
#define picman_gradient_select_widget_close_popup picman_gradient_select_widget_close

#define picman_interactive_selection_gradient     picman_gradient_select_new
#define picman_font_select_widget                 picman_font_select_widget_new
#define picman_font_select_widget_set_popup       picman_font_select_widget_set
#define picman_font_select_widget_close_popup     picman_font_select_widget_close

#define picman_interactive_selection_pattern      picman_pattern_select_new
#define picman_pattern_select_widget              picman_pattern_select_widget_new
#define picman_pattern_select_widget_set_popup    picman_pattern_select_widget_set
#define picman_pattern_select_widget_close_popup  picman_pattern_select_widget_close

#define INTENSITY(r,g,b)                        PICMAN_RGB_INTENSITY(r,g,b)
#define INTENSITY_RED                           PICMAN_RGB_INTENSITY_RED
#define INTENSITY_GREEN                         PICMAN_RGB_INTENSITY_GREEN
#define INTENSITY_BLUE                          PICMAN_RGB_INTENSITY_BLUE

#define picman_file_selection_                    picman_file_entry_
#define PicmanFileSelection                       PicmanFileEntry
#define PICMAN_TYPE_FILE_SELECTION                PICMAN_TYPE_FILE_ENTRY
#define PICMAN_FILE_SELECTION                     PICMAN_FILE_ENTRY
#define PICMAN_IS_FILE_SELECTION                  PICMAN_IS_FILE_ENTRY


enum
{
  PICMAN_WHITE_MASK         = PICMAN_ADD_WHITE_MASK,
  PICMAN_BLACK_MASK         = PICMAN_ADD_BLACK_MASK,
  PICMAN_ALPHA_MASK         = PICMAN_ADD_ALPHA_MASK,
  PICMAN_SELECTION_MASK     = PICMAN_ADD_SELECTION_MASK,
  PICMAN_COPY_MASK          = PICMAN_ADD_COPY_MASK,
};

enum
{
  PICMAN_ADD       = PICMAN_CHANNEL_OP_ADD,
  PICMAN_SUB       = PICMAN_CHANNEL_OP_SUBTRACT,
  PICMAN_REPLACE   = PICMAN_CHANNEL_OP_REPLACE,
  PICMAN_INTERSECT = PICMAN_CHANNEL_OP_INTERSECT
};

enum
{
  PICMAN_FG_BG_RGB = PICMAN_FG_BG_RGB_MODE,
  PICMAN_FG_BG_HSV = PICMAN_FG_BG_HSV_MODE,
  PICMAN_FG_TRANS  = PICMAN_FG_TRANSPARENT_MODE,
  PICMAN_CUSTOM    = PICMAN_CUSTOM_MODE
};

enum
{
  PICMAN_FG_IMAGE_FILL    = PICMAN_FOREGROUND_FILL,
  PICMAN_BG_IMAGE_FILL    = PICMAN_BACKGROUND_FILL,
  PICMAN_WHITE_IMAGE_FILL = PICMAN_WHITE_FILL,
  PICMAN_TRANS_IMAGE_FILL = PICMAN_TRANSPARENT_FILL
};

enum
{
  PICMAN_APPLY   = PICMAN_MASK_APPLY,
  PICMAN_DISCARD = PICMAN_MASK_DISCARD
};

enum
{
  PICMAN_HARD = PICMAN_BRUSH_HARD,
  PICMAN_SOFT = PICMAN_BRUSH_SOFT,
};

enum
{
  PICMAN_CONTINUOUS  = PICMAN_PAINT_CONSTANT,
  PICMAN_INCREMENTAL = PICMAN_PAINT_INCREMENTAL
};

enum
{
  PICMAN_HORIZONTAL = PICMAN_ORIENTATION_HORIZONTAL,
  PICMAN_VERTICAL   = PICMAN_ORIENTATION_VERTICAL,
  PICMAN_UNKNOWN    = PICMAN_ORIENTATION_UNKNOWN
};

enum
{
  PICMAN_LINEAR               = PICMAN_GRADIENT_LINEAR,
  PICMAN_BILNEAR              = PICMAN_GRADIENT_BILINEAR,
  PICMAN_RADIAL               = PICMAN_GRADIENT_RADIAL,
  PICMAN_SQUARE               = PICMAN_GRADIENT_SQUARE,
  PICMAN_CONICAL_SYMMETRIC    = PICMAN_GRADIENT_CONICAL_SYMMETRIC,
  PICMAN_CONICAL_ASYMMETRIC   = PICMAN_GRADIENT_CONICAL_ASYMMETRIC,
  PICMAN_SHAPEBURST_ANGULAR   = PICMAN_GRADIENT_SHAPEBURST_ANGULAR,
  PICMAN_SHAPEBURST_SPHERICAL = PICMAN_GRADIENT_SHAPEBURST_SPHERICAL,
  PICMAN_SHAPEBURST_DIMPLED   = PICMAN_GRADIENT_SHAPEBURST_DIMPLED,
  PICMAN_SPIRAL_CLOCKWISE     = PICMAN_GRADIENT_SPIRAL_CLOCKWISE,
  PICMAN_SPIRAL_ANTICLOCKWISE = PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE
};

enum
{
  PICMAN_VALUE_LUT = PICMAN_HISTOGRAM_VALUE,
  PICMAN_RED_LUT   = PICMAN_HISTOGRAM_RED,
  PICMAN_GREEN_LUT = PICMAN_HISTOGRAM_GREEN,
  PICMAN_BLUE_LUT  = PICMAN_HISTOGRAM_BLUE,
  PICMAN_ALPHA_LUT = PICMAN_HISTOGRAM_ALPHA
};


#endif  /* PICMAN_DISABLE_DEPRECATED */

G_END_DECLS

#endif  /* __PICMAN_COMPAT_H__ */
