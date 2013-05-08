/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanunitcache.c
 * Copyright (C) 1999-2000 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanunitcache.h"
#include "picmanunit_pdb.h"

#include "libpicman-intl.h"

/*  internal structures  */

typedef struct
{
  gdouble      factor;
  gint         digits;
  const gchar *identifier;
  const gchar *symbol;
  const gchar *abbreviation;
  const gchar *singular;
  const gchar *plural;
} PicmanUnitDef;


static PicmanUnitDef * picman_unit_defs         = NULL;
static PicmanUnit      picman_units_initialized = 0;

/*  not a unit at all but kept here to have the strings in one place
 */
static const PicmanUnitDef picman_unit_percent =
{
  0.0, 0, "percent", "%", "%",  N_("percent"), N_("percent")
};


static void  picman_unit_def_init (PicmanUnitDef *unit_def,
                                 PicmanUnit     unit);


static gboolean
picman_unit_init (PicmanUnit unit)
{
  gint i, n;

  if (unit < picman_units_initialized)
    return TRUE;

  n = _picman_unit_get_number_of_units ();

  if (unit >= n)
    return FALSE;

  picman_unit_defs = g_renew (PicmanUnitDef, picman_unit_defs, n);

  for (i = picman_units_initialized; i < n; i++)
    {
      picman_unit_def_init (&picman_unit_defs[i], i);
    }

  picman_units_initialized = n;

  return TRUE;
}

static void
picman_unit_def_init (PicmanUnitDef *unit_def,
                    PicmanUnit     unit)
{
  unit_def->factor       = _picman_unit_get_factor (unit);
  unit_def->digits       = _picman_unit_get_digits (unit);
  unit_def->identifier   = _picman_unit_get_identifier (unit);
  unit_def->symbol       = _picman_unit_get_symbol (unit);
  unit_def->abbreviation = _picman_unit_get_abbreviation (unit);
  unit_def->singular     = _picman_unit_get_singular (unit);
  unit_def->plural       = _picman_unit_get_plural (unit);
}

gint
_picman_unit_cache_get_number_of_units (void)
{
  return _picman_unit_get_number_of_units ();
}

gint
_picman_unit_cache_get_number_of_built_in_units (void)
{
  return PICMAN_UNIT_END;
}

PicmanUnit
_picman_unit_cache_new (gchar   *identifier,
                      gdouble  factor,
                      gint     digits,
                      gchar   *symbol,
                      gchar   *abbreviation,
                      gchar   *singular,
                      gchar   *plural)
{
  return _picman_unit_new (identifier,
                         factor,
                         digits,
                         symbol,
                         abbreviation,
                         singular,
                         plural);
}

gboolean
_picman_unit_cache_get_deletion_flag (PicmanUnit unit)
{
  if (unit < PICMAN_UNIT_END)
    return FALSE;

  return _picman_unit_get_deletion_flag (unit);
}

void
_picman_unit_cache_set_deletion_flag (PicmanUnit unit,
                                    gboolean deletion_flag)
{
  if (unit < PICMAN_UNIT_END)
    return;

  _picman_unit_set_deletion_flag (unit,
                                deletion_flag);
}

gdouble
_picman_unit_cache_get_factor (PicmanUnit unit)
{
  g_return_val_if_fail (unit >= PICMAN_UNIT_INCH, 1.0);

  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.factor;

  if (!picman_unit_init (unit))
    return 1.0;

  return picman_unit_defs[unit].factor;
}

gint
_picman_unit_cache_get_digits (PicmanUnit unit)
{
  g_return_val_if_fail (unit >= PICMAN_UNIT_INCH, 0);

  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.digits;

  if (!picman_unit_init (unit))
    return 0;

  return picman_unit_defs[unit].digits;
}

const gchar *
_picman_unit_cache_get_identifier (PicmanUnit unit)
{
  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.identifier;

  if (!picman_unit_init (unit))
    return NULL;

  return picman_unit_defs[unit].identifier;
}

const gchar *
_picman_unit_cache_get_symbol (PicmanUnit unit)
{
  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.symbol;

  if (!picman_unit_init (unit))
    return NULL;

  return picman_unit_defs[unit].symbol;
}

const gchar *
_picman_unit_cache_get_abbreviation (PicmanUnit unit)
{
  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.abbreviation;

  if (!picman_unit_init (unit))
    return NULL;

  return picman_unit_defs[unit].abbreviation;
}

const gchar *
_picman_unit_cache_get_singular (PicmanUnit unit)
{
  if (unit == PICMAN_UNIT_PERCENT)
    return gettext (picman_unit_percent.singular);

  if (!picman_unit_init (unit))
    return NULL;

  return gettext (picman_unit_defs[unit].singular);
}

const gchar *
_picman_unit_cache_get_plural (PicmanUnit unit)
{
  if (unit == PICMAN_UNIT_PERCENT)
    return gettext (picman_unit_percent.plural);

  if (!picman_unit_init (unit))
    return NULL;

  return gettext (picman_unit_defs[unit].plural);
}
