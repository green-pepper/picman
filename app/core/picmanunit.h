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

#ifndef __APP_PICMAN_UNIT_H__
#define __APP_PICMAN_UNIT_H__


gint          _picman_unit_get_number_of_units          (Picman        *picman);
gint          _picman_unit_get_number_of_built_in_units (Picman        *picman) G_GNUC_CONST;

PicmanUnit      _picman_unit_new                          (Picman        *picman,
                                                       const gchar *identifier,
                                                       gdouble      factor,
                                                       gint         digits,
                                                       const gchar *symbol,
                                                       const gchar *abbreviation,
                                                       const gchar *singular,
                                                       const gchar *plural);

gboolean      _picman_unit_get_deletion_flag            (Picman        *picman,
                                                       PicmanUnit     unit);
void          _picman_unit_set_deletion_flag            (Picman        *picman,
                                                       PicmanUnit     unit,
                                                       gboolean     deletion_flag);

gdouble       _picman_unit_get_factor                   (Picman        *picman,
                                                       PicmanUnit     unit);

gint          _picman_unit_get_digits                   (Picman        *picman,
                                                       PicmanUnit     unit);

const gchar * _picman_unit_get_identifier               (Picman        *picman,
                                                       PicmanUnit     unit);

const gchar * _picman_unit_get_symbol                   (Picman        *picman,
                                                       PicmanUnit     unit);
const gchar * _picman_unit_get_abbreviation             (Picman        *picman,
                                                       PicmanUnit     unit);
const gchar * _picman_unit_get_singular                 (Picman        *picman,
                                                       PicmanUnit     unit);
const gchar * _picman_unit_get_plural                   (Picman        *picman,
                                                       PicmanUnit     unit);

void           picman_user_units_free                   (Picman        *picman);


#endif  /*  __APP_PICMAN_UNIT_H__  */
