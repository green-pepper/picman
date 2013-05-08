/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmandashpattern.h
 * Copyright (C) 2003 Simon Budig
 * Copyright (C) 2005 Sven Neumann
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

#ifndef __PICMAN_DASH_PATTERN_H__
#define __PICMAN_DASH_PATTERN_H__


#define PICMAN_TYPE_DASH_PATTERN               (picman_dash_pattern_get_type ())
#define PICMAN_VALUE_HOLDS_DASH_PATTERN(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_DASH_PATTERN))


GType            picman_dash_pattern_get_type          (void) G_GNUC_CONST;

GArray         * picman_dash_pattern_new_from_preset   (PicmanDashPreset  preset);
GArray         * picman_dash_pattern_new_from_segments (const gboolean *segments,
                                                      gint            n_segments,
                                                      gdouble         dash_length);

void             picman_dash_pattern_fill_segments     (GArray         *pattern,
                                                      gboolean       *segments,
                                                      gint            n_segments);

GArray         * picman_dash_pattern_from_value_array  (PicmanValueArray *value_array);
PicmanValueArray * picman_dash_pattern_to_value_array    (GArray         *pattern);

GArray         * picman_dash_pattern_copy              (GArray         *pattern);
void             picman_dash_pattern_free              (GArray         *pattern);


#endif  /*  __PICMAN_DASH_PATTERN_H__  */
