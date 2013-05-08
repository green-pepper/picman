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

#ifndef __PICMAN_PALETTE_LOAD_H__
#define __PICMAN_PALETTE_LOAD_H__


#define PICMAN_PALETTE_FILE_EXTENSION ".gpl"


typedef enum
{
  PICMAN_PALETTE_FILE_FORMAT_UNKNOWN,
  PICMAN_PALETTE_FILE_FORMAT_GPL,      /* PICMAN palette                        */
  PICMAN_PALETTE_FILE_FORMAT_RIFF_PAL, /* RIFF palette                        */
  PICMAN_PALETTE_FILE_FORMAT_ACT,      /* Photoshop binary color palette      */
  PICMAN_PALETTE_FILE_FORMAT_PSP_PAL,  /* JASC's Paint Shop Pro color palette */
  PICMAN_PALETTE_FILE_FORMAT_ACO,      /* Photoshop ACO color file            */
  PICMAN_PALETTE_FILE_FORMAT_CSS       /* Cascaded Stylesheet file (CSS)      */
} PicmanPaletteFileFormat;


GList               * picman_palette_load               (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);
GList               * picman_palette_load_act           (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);
GList               * picman_palette_load_riff          (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);
GList               * picman_palette_load_psp           (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);
GList               * picman_palette_load_aco           (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);
GList               * picman_palette_load_css           (PicmanContext  *context,
                                                       const gchar  *filename,
                                                       GError      **error);

PicmanPaletteFileFormat picman_palette_load_detect_format (const gchar  *filename);


#endif /* __PICMAN_PALETTE_H__ */
