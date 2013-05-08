/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanunit.h
 * Copyright (C) 1999-2003 Michael Natterer <mitch@picman.org>
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

#if !defined (__PICMAN_BASE_H_INSIDE__) && !defined (PICMAN_BASE_COMPILATION)
#error "Only <libpicmanbase/picmanbase.h> can be included directly."
#endif

#ifndef __PICMAN_UNIT_H__
#define __PICMAN_UNIT_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */

/**
 * PICMAN_TYPE_UNIT:
 *
 * #PICMAN_TYPE_UNIT is a #GType derived from #G_TYPE_INT.
 **/

#define PICMAN_TYPE_UNIT               (picman_unit_get_type ())
#define PICMAN_VALUE_HOLDS_UNIT(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_UNIT))

GType        picman_unit_get_type      (void) G_GNUC_CONST;


/*
 * PICMAN_TYPE_PARAM_UNIT
 */

#define PICMAN_TYPE_PARAM_UNIT              (picman_param_unit_get_type ())
#define PICMAN_IS_PARAM_SPEC_UNIT(pspec)    (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_UNIT))

GType        picman_param_unit_get_type     (void) G_GNUC_CONST;

GParamSpec * picman_param_spec_unit         (const gchar  *name,
                                           const gchar  *nick,
                                           const gchar  *blurb,
                                           gboolean      allow_pixels,
                                           gboolean      allow_percent,
                                           PicmanUnit      default_value,
                                           GParamFlags   flags);



gint          picman_unit_get_number_of_units          (void);
gint          picman_unit_get_number_of_built_in_units (void) G_GNUC_CONST;

PicmanUnit      picman_unit_new                 (gchar       *identifier,
                                             gdouble      factor,
                                             gint         digits,
                                             gchar       *symbol,
                                             gchar       *abbreviation,
                                             gchar       *singular,
                                             gchar       *plural);

gboolean      picman_unit_get_deletion_flag   (PicmanUnit     unit);
void          picman_unit_set_deletion_flag   (PicmanUnit     unit,
                                             gboolean     deletion_flag);

gdouble       picman_unit_get_factor          (PicmanUnit     unit);

gint          picman_unit_get_digits          (PicmanUnit     unit);

const gchar * picman_unit_get_identifier      (PicmanUnit     unit);

const gchar * picman_unit_get_symbol          (PicmanUnit     unit);
const gchar * picman_unit_get_abbreviation    (PicmanUnit     unit);
const gchar * picman_unit_get_singular        (PicmanUnit     unit);
const gchar * picman_unit_get_plural          (PicmanUnit     unit);

gchar       * picman_unit_format_string       (const gchar *format,
                                             PicmanUnit     unit);

gdouble       picman_pixels_to_units          (gdouble      pixels,
                                             PicmanUnit     unit,
                                             gdouble      resolution);
gdouble       picman_units_to_pixels          (gdouble      value,
                                             PicmanUnit     unit,
                                             gdouble      resolution);
gdouble       picman_units_to_points          (gdouble      value,
                                             PicmanUnit     unit,
                                             gdouble      resolution);


G_END_DECLS

#endif /* __PICMAN_UNIT_H__ */
