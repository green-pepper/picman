/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-babl.h
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

#ifndef __PICMAN_BABL_H__
#define __PICMAN_BABL_H__


void                picman_babl_init                  (void);

const gchar       * picman_babl_get_description       (const Babl        *babl);

PicmanImageBaseType   picman_babl_format_get_base_type  (const Babl        *format);
PicmanPrecision       picman_babl_format_get_precision  (const Babl        *format);
gboolean            picman_babl_format_get_linear     (const Babl        *format);


const Babl        * picman_babl_format                (PicmanImageBaseType  base_type,
                                                     PicmanPrecision      precision,
                                                     gboolean           with_alpha);
const Babl        * picman_babl_mask_format           (PicmanPrecision      precision);
const Babl        * picman_babl_component_format      (PicmanImageBaseType  base_type,
                                                     PicmanPrecision      precision,
                                                     gint               index);


#endif /* __PICMAN_BABL_H__ */
