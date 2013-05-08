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

#include "config.h"

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanbase/picmanbase-private.h"

#include "core/core-types.h"

#include "core/picman.h"
#include "core/picmanunit.h"

#include "units.h"


static Picman *the_unit_picman = NULL;


static gint
units_get_number_of_units (void)
{
  return _picman_unit_get_number_of_units (the_unit_picman);
}

static gint
units_get_number_of_built_in_units (void)
{
  return PICMAN_UNIT_END;
}

static PicmanUnit
units_unit_new (gchar   *identifier,
                gdouble  factor,
                gint     digits,
                gchar   *symbol,
                gchar   *abbreviation,
                gchar   *singular,
                gchar   *plural)
{
  return _picman_unit_new (the_unit_picman,
                         identifier,
                         factor,
                         digits,
                         symbol,
                         abbreviation,
                         singular,
                         plural);
}

static gboolean
units_unit_get_deletion_flag (PicmanUnit unit)
{
  return _picman_unit_get_deletion_flag (the_unit_picman, unit);
}

static void
units_unit_set_deletion_flag (PicmanUnit unit,
                              gboolean deletion_flag)
{
  _picman_unit_set_deletion_flag (the_unit_picman, unit, deletion_flag);
}

static gdouble
units_unit_get_factor (PicmanUnit unit)
{
  return _picman_unit_get_factor (the_unit_picman, unit);
}

static gint
units_unit_get_digits (PicmanUnit unit)
{
  return _picman_unit_get_digits (the_unit_picman, unit);
}

static const gchar *
units_unit_get_identifier (PicmanUnit unit)
{
  return _picman_unit_get_identifier (the_unit_picman, unit);
}

static const gchar *
units_unit_get_symbol (PicmanUnit unit)
{
  return _picman_unit_get_symbol (the_unit_picman, unit);
}

static const gchar *
units_unit_get_abbreviation (PicmanUnit unit)
{
  return _picman_unit_get_abbreviation (the_unit_picman, unit);
}

static const gchar *
units_unit_get_singular (PicmanUnit unit)
{
  return _picman_unit_get_singular (the_unit_picman, unit);
}

static const gchar *
units_unit_get_plural (PicmanUnit unit)
{
  return _picman_unit_get_plural (the_unit_picman, unit);
}

void
units_init (Picman *picman)
{
  PicmanUnitVtable vtable;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));
  g_return_if_fail (the_unit_picman == NULL);

  the_unit_picman = picman;

  vtable.unit_get_number_of_units          = units_get_number_of_units;
  vtable.unit_get_number_of_built_in_units = units_get_number_of_built_in_units;
  vtable.unit_new               = units_unit_new;
  vtable.unit_get_deletion_flag = units_unit_get_deletion_flag;
  vtable.unit_set_deletion_flag = units_unit_set_deletion_flag;
  vtable.unit_get_factor        = units_unit_get_factor;
  vtable.unit_get_digits        = units_unit_get_digits;
  vtable.unit_get_identifier    = units_unit_get_identifier;
  vtable.unit_get_symbol        = units_unit_get_symbol;
  vtable.unit_get_abbreviation  = units_unit_get_abbreviation;
  vtable.unit_get_singular      = units_unit_get_singular;
  vtable.unit_get_plural        = units_unit_get_plural;

  picman_base_init (&vtable);
}
