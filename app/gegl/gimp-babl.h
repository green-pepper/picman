/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimp-babl.h
 * Copyright (C) 2012 Michael Natterer <mitch@gimp.org>
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

#ifndef __GIMP_BABL_H__
#define __GIMP_BABL_H__


void                gimp_babl_init                  (void);

const gchar       * gimp_babl_get_description       (const Babl        *babl);

GimpImageBaseType   gimp_babl_format_get_base_type  (const Babl        *format);
GimpPrecision       gimp_babl_format_get_precision  (const Babl        *format);
gboolean            gimp_babl_format_get_linear     (const Babl        *format);


const Babl        * gimp_babl_format                (GimpImageBaseType  base_type,
                                                     GimpPrecision      precision,
                                                     gboolean           with_alpha);
const Babl        * gimp_babl_mask_format           (GimpPrecision      precision);
const Babl        * gimp_babl_component_format      (GimpImageBaseType  base_type,
                                                     GimpPrecision      precision,
                                                     gint               index);


#endif /* __GIMP_BABL_H__ */
