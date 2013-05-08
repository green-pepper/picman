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

#ifndef __PICMAN_PALETTE_IMPORT__
#define __PICMAN_PALETTE_IMPORT__


PicmanPalette * picman_palette_import_from_gradient      (PicmanGradient *gradient,
                                                      PicmanContext  *context,
                                                      gboolean      reverse,
                                                      const gchar  *palette_name,
                                                      gint          n_colors);
PicmanPalette * picman_palette_import_from_image         (PicmanImage    *image,
                                                      PicmanContext  *context,
                                                      const gchar  *palette_name,
                                                      gint          n_colors,
                                                      gint          treshold,
                                                      gboolean      selection_only);
PicmanPalette * picman_palette_import_from_indexed_image (PicmanImage    *image,
                                                      PicmanContext  *context,
                                                      const gchar  *palette_name);
PicmanPalette * picman_palette_import_from_drawable      (PicmanDrawable *drawable,
                                                      PicmanContext  *context,
                                                      const gchar  *palette_name,
                                                      gint          n_colors,
                                                      gint          threshold,
                                                      gboolean      selection_only);
PicmanPalette * picman_palette_import_from_file          (PicmanContext  *context,
                                                      const gchar  *filename,
                                                      const gchar  *palette_name,
                                                      GError      **error);

#endif  /* __PICMAN_PALETTE_IMPORT_H__ */
