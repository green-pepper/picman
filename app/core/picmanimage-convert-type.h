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

#ifndef __PICMAN_IMAGE_CONVERT_TYPE_H__
#define __PICMAN_IMAGE_CONVERT_TYPE_H__


#define MAXNUMCOLORS 256


gboolean   picman_image_convert_type      (PicmanImage               *image,
                                         PicmanImageBaseType        new_type,
                                         /* The following params used only for
                                          * new_type == PICMAN_INDEXED
                                          */
                                         gint                     num_cols,
                                         PicmanConvertDitherType    dither,
                                         gboolean                 alpha_dither,
                                         gboolean                 text_layer_dither,
                                         gboolean                 remove_dups,
                                         PicmanConvertPaletteType   palette_type,
                                         PicmanPalette             *custom_palette,
                                         PicmanProgress            *progress,
                                         GError                 **error);

void  picman_image_convert_type_set_dither_matrix (const guchar *matrix,
                                                 gint          width,
                                                 gint          height);


#endif  /*  __PICMAN_IMAGE_CONVERT_TYPE_H__  */
