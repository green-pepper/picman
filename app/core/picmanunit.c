/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanunit.c
 * Copyright (C) 1999-2000 Michael Natterer <mitch@picman.org>
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

/* This file contains the definition of the size unit objects. The
 * factor of the units is relative to inches (which have a factor of 1).
 */

#include "config.h"

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman.h"
#include "picmanunit.h"

#include "picman-intl.h"


/* internal structures */

typedef struct
{
  gboolean  delete_on_exit;
  gdouble   factor;
  gint      digits;
  gchar    *identifier;
  gchar    *symbol;
  gchar    *abbreviation;
  gchar    *singular;
  gchar    *plural;
} PicmanUnitDef;


/*  these are the built-in units
 */
static const PicmanUnitDef picman_unit_defs[PICMAN_UNIT_END] =
{
  /* pseudo unit */
  { FALSE,  0.0, 0, "pixels",      "px", "px",
    NC_("unit-singular", "pixel"),      NC_("unit-plural", "pixels")      },

  /* standard units */
  { FALSE,  1.0, 2, "inches",      "''", "in",
    NC_("unit-singular", "inch"),       NC_("unit-plural", "inches")      },

  { FALSE, 25.4, 1, "millimeters", "mm", "mm",
    NC_("unit-singular", "millimeter"), NC_("unit-plural", "millimeters") },

  /* professional units */
  { FALSE, 72.0, 0, "points",      "pt", "pt",
    NC_("unit-singular", "point"),      NC_("unit-plural", "points")      },

  { FALSE,  6.0, 1, "picas",       "pc", "pc",
    NC_("unit-singular", "pica"),       NC_("unit-plural", "picas")       }
};

/*  not a unit at all but kept here to have the strings in one place
 */
static const PicmanUnitDef picman_unit_percent =
{
    FALSE,  0.0, 0, "percent",     "%",  "%", 
    NC_("singular", "percent"),    NC_("plural", "percent")
};


/* private functions */

static PicmanUnitDef *
_picman_unit_get_user_unit (Picman     *picman,
                          PicmanUnit  unit)
{
  return g_list_nth_data (picman->user_units, unit - PICMAN_UNIT_END);
}


/* public functions */

gint
_picman_unit_get_number_of_units (Picman *picman)
{
  return PICMAN_UNIT_END + picman->n_user_units;
}

gint
_picman_unit_get_number_of_built_in_units (Picman *picman)
{
  return PICMAN_UNIT_END;
}

PicmanUnit
_picman_unit_new (Picman        *picman,
                const gchar *identifier,
                gdouble      factor,
                gint         digits,
                const gchar *symbol,
                const gchar *abbreviation,
                const gchar *singular,
                const gchar *plural)
{
  PicmanUnitDef *user_unit = g_slice_new0 (PicmanUnitDef);

  user_unit->delete_on_exit = TRUE;
  user_unit->factor         = factor;
  user_unit->digits         = digits;
  user_unit->identifier     = g_strdup (identifier);
  user_unit->symbol         = g_strdup (symbol);
  user_unit->abbreviation   = g_strdup (abbreviation);
  user_unit->singular       = g_strdup (singular);
  user_unit->plural         = g_strdup (plural);

  picman->user_units = g_list_append (picman->user_units, user_unit);
  picman->n_user_units++;

  return PICMAN_UNIT_END + picman->n_user_units - 1;
}

gboolean
_picman_unit_get_deletion_flag (Picman     *picman,
                              PicmanUnit  unit)
{
  g_return_val_if_fail (unit < (PICMAN_UNIT_END + picman->n_user_units), FALSE);

  if (unit < PICMAN_UNIT_END)
    return FALSE;

  return _picman_unit_get_user_unit (picman, unit)->delete_on_exit;
}

void
_picman_unit_set_deletion_flag (Picman     *picman,
                              PicmanUnit  unit,
                              gboolean  deletion_flag)
{
  g_return_if_fail ((unit >= PICMAN_UNIT_END) &&
                    (unit < (PICMAN_UNIT_END + picman->n_user_units)));

  _picman_unit_get_user_unit (picman, unit)->delete_on_exit =
    deletion_flag ? TRUE : FALSE;
}

gdouble
_picman_unit_get_factor (Picman     *picman,
                       PicmanUnit  unit)
{
  g_return_val_if_fail (unit < (PICMAN_UNIT_END + picman->n_user_units),
                        picman_unit_defs[PICMAN_UNIT_INCH].factor);

  if (unit < PICMAN_UNIT_END)
    return picman_unit_defs[unit].factor;

  return _picman_unit_get_user_unit (picman, unit)->factor;
}

gint
_picman_unit_get_digits (Picman     *picman,
                       PicmanUnit  unit)
{
  g_return_val_if_fail (unit < (PICMAN_UNIT_END + picman->n_user_units),
                        picman_unit_defs[PICMAN_UNIT_INCH].digits);

  if (unit < PICMAN_UNIT_END)
    return picman_unit_defs[unit].digits;

  return _picman_unit_get_user_unit (picman, unit)->digits;
}

const gchar *
_picman_unit_get_identifier (Picman     *picman,
                           PicmanUnit  unit)
{
  g_return_val_if_fail ((unit < (PICMAN_UNIT_END + picman->n_user_units)) ||
                        (unit == PICMAN_UNIT_PERCENT),
                        picman_unit_defs[PICMAN_UNIT_INCH].identifier);

  if (unit < PICMAN_UNIT_END)
    return picman_unit_defs[unit].identifier;

  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.identifier;

  return _picman_unit_get_user_unit (picman, unit)->identifier;
}

const gchar *
_picman_unit_get_symbol (Picman     *picman,
                       PicmanUnit  unit)
{
  g_return_val_if_fail ((unit < (PICMAN_UNIT_END + picman->n_user_units)) ||
                        (unit == PICMAN_UNIT_PERCENT),
                        picman_unit_defs[PICMAN_UNIT_INCH].symbol);

  if (unit < PICMAN_UNIT_END)
    return picman_unit_defs[unit].symbol;

  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.symbol;

  return _picman_unit_get_user_unit (picman, unit)->symbol;
}

const gchar *
_picman_unit_get_abbreviation (Picman     *picman,
                             PicmanUnit  unit)
{
  g_return_val_if_fail ((unit < (PICMAN_UNIT_END + picman->n_user_units)) ||
                        (unit == PICMAN_UNIT_PERCENT),
                        picman_unit_defs[PICMAN_UNIT_INCH].abbreviation);

  if (unit < PICMAN_UNIT_END)
    return picman_unit_defs[unit].abbreviation;

  if (unit == PICMAN_UNIT_PERCENT)
    return picman_unit_percent.abbreviation;

  return _picman_unit_get_user_unit (picman, unit)->abbreviation;
}

const gchar *
_picman_unit_get_singular (Picman     *picman,
                         PicmanUnit  unit)
{
  g_return_val_if_fail ((unit < (PICMAN_UNIT_END + picman->n_user_units)) ||
                        (unit == PICMAN_UNIT_PERCENT),
                        picman_unit_defs[PICMAN_UNIT_INCH].singular);

  if (unit < PICMAN_UNIT_END)
    return g_dpgettext2 (NULL, "unit-singular", picman_unit_defs[unit].singular);

  if (unit == PICMAN_UNIT_PERCENT)
    return g_dpgettext2 (NULL, "unit-singular", picman_unit_percent.singular);

  return _picman_unit_get_user_unit (picman, unit)->singular;
}

const gchar *
_picman_unit_get_plural (Picman     *picman,
                       PicmanUnit  unit)
{
  g_return_val_if_fail ((unit < (PICMAN_UNIT_END + picman->n_user_units)) ||
                        (unit == PICMAN_UNIT_PERCENT),
                        picman_unit_defs[PICMAN_UNIT_INCH].plural);

  if (unit < PICMAN_UNIT_END)
    return g_dpgettext2 (NULL, "unit-plural", picman_unit_defs[unit].plural);

  if (unit == PICMAN_UNIT_PERCENT)
    return g_dpgettext2 (NULL, "unit-plural", picman_unit_percent.plural);

  return _picman_unit_get_user_unit (picman, unit)->plural;
}


/* The sole purpose of this function is to release the allocated
 * memory. It must only be used from picman_units_exit().
 */
void
picman_user_units_free (Picman *picman)
{
  GList *list;

  for (list = picman->user_units; list; list = g_list_next (list))
    {
      PicmanUnitDef *user_unit = list->data;

      g_free (user_unit->identifier);
      g_free (user_unit->symbol);
      g_free (user_unit->abbreviation);
      g_free (user_unit->singular);
      g_free (user_unit->plural);

      g_slice_free (PicmanUnitDef, user_unit);
    }

  g_list_free (picman->user_units);
  picman->user_units   = NULL;
  picman->n_user_units = 0;
}
