/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGrid
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picmangrid.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_STYLE,
  PROP_FGCOLOR,
  PROP_BGCOLOR,
  PROP_XSPACING,
  PROP_YSPACING,
  PROP_SPACING_UNIT,
  PROP_XOFFSET,
  PROP_YOFFSET,
  PROP_OFFSET_UNIT
};


static void   picman_grid_get_property (GObject      *object,
                                      guint         property_id,
                                      GValue       *value,
                                      GParamSpec   *pspec);
static void   picman_grid_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanGrid, picman_grid, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))


static void
picman_grid_class_init (PicmanGridClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PicmanRGB       black;
  PicmanRGB       white;

  object_class->get_property = picman_grid_get_property;
  object_class->set_property = picman_grid_set_property;

  picman_rgba_set (&black, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&white, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_STYLE,
                                 "style",
                                 N_("Line style used for the grid."),
                                 PICMAN_TYPE_GRID_STYLE,
                                 PICMAN_GRID_SOLID,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PROP_FGCOLOR,
                                "fgcolor",
                                N_("The foreground color of the grid."),
                                TRUE, &black,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PROP_BGCOLOR,
                                "bgcolor",
                                N_("The background color of the grid; "
                                   "only used in double dashed line style."),
                                TRUE, &white,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_XSPACING,
                                   "xspacing",
                                   N_("Horizontal spacing of grid lines."),
                                   1.0, PICMAN_MAX_IMAGE_SIZE, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_YSPACING,
                                   "yspacing",
                                   N_("Vertical spacing of grid lines."),
                                   1.0, PICMAN_MAX_IMAGE_SIZE, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_SPACING_UNIT,
                                 "spacing-unit", NULL,
                                 FALSE, FALSE, PICMAN_UNIT_INCH,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_XOFFSET,
                                   "xoffset",
                                   N_("Horizontal offset of the first grid "
                                      "line; this may be a negative number."),
                                   - PICMAN_MAX_IMAGE_SIZE,
                                   PICMAN_MAX_IMAGE_SIZE, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_YOFFSET,
                                   "yoffset",
                                   N_("Vertical offset of the first grid "
                                      "line; this may be a negative number."),
                                   - PICMAN_MAX_IMAGE_SIZE,
                                   PICMAN_MAX_IMAGE_SIZE, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_OFFSET_UNIT,
                                 "offset-unit", NULL,
                                 FALSE, FALSE, PICMAN_UNIT_INCH,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_grid_init (PicmanGrid *grid)
{
}

static void
picman_grid_get_property (GObject      *object,
                        guint         property_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
  PicmanGrid *grid = PICMAN_GRID (object);

  switch (property_id)
    {
    case PROP_STYLE:
      g_value_set_enum (value, grid->style);
      break;
    case PROP_FGCOLOR:
      g_value_set_boxed (value, &grid->fgcolor);
      break;
    case PROP_BGCOLOR:
      g_value_set_boxed (value, &grid->bgcolor);
      break;
    case PROP_XSPACING:
      g_value_set_double (value, grid->xspacing);
      break;
    case PROP_YSPACING:
      g_value_set_double (value, grid->yspacing);
      break;
    case PROP_SPACING_UNIT:
      g_value_set_int (value, grid->spacing_unit);
      break;
    case PROP_XOFFSET:
      g_value_set_double (value, grid->xoffset);
      break;
    case PROP_YOFFSET:
      g_value_set_double (value, grid->yoffset);
      break;
    case PROP_OFFSET_UNIT:
      g_value_set_int (value, grid->offset_unit);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_grid_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  PicmanGrid *grid = PICMAN_GRID (object);
  PicmanRGB  *color;

  switch (property_id)
    {
    case PROP_STYLE:
      grid->style = g_value_get_enum (value);
      break;
    case PROP_FGCOLOR:
      color = g_value_get_boxed (value);
      grid->fgcolor = *color;
      break;
    case PROP_BGCOLOR:
      color = g_value_get_boxed (value);
      grid->bgcolor = *color;
      break;
    case PROP_XSPACING:
      grid->xspacing = g_value_get_double (value);
      break;
    case PROP_YSPACING:
      grid->yspacing = g_value_get_double (value);
      break;
    case PROP_SPACING_UNIT:
      grid->spacing_unit = g_value_get_int (value);
      break;
    case PROP_XOFFSET:
      grid->xoffset = g_value_get_double (value);
      break;
    case PROP_YOFFSET:
      grid->yoffset = g_value_get_double (value);
      break;
    case PROP_OFFSET_UNIT:
      grid->offset_unit = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

const gchar *
picman_grid_parasite_name (void)
{
  return "picman-image-grid";
}

PicmanParasite *
picman_grid_to_parasite (const PicmanGrid *grid)
{
  PicmanParasite *parasite;
  gchar        *str;

  g_return_val_if_fail (PICMAN_IS_GRID (grid), NULL);

  str = picman_config_serialize_to_string (PICMAN_CONFIG (grid), NULL);
  g_return_val_if_fail (str != NULL, NULL);

  parasite = picman_parasite_new (picman_grid_parasite_name (),
                                PICMAN_PARASITE_PERSISTENT,
                                strlen (str) + 1, str);
  g_free (str);

  return parasite;
}

PicmanGrid *
picman_grid_from_parasite (const PicmanParasite *parasite)
{
  PicmanGrid    *grid;
  const gchar *str;
  GError      *error = NULL;

  g_return_val_if_fail (parasite != NULL, NULL);
  g_return_val_if_fail (strcmp (picman_parasite_name (parasite),
                                picman_grid_parasite_name ()) == 0, NULL);

  str = picman_parasite_data (parasite);
  g_return_val_if_fail (str != NULL, NULL);

  grid = g_object_new (PICMAN_TYPE_GRID, NULL);

  if (! picman_config_deserialize_string (PICMAN_CONFIG (grid),
                                        str,
                                        picman_parasite_data_size (parasite),
                                        NULL,
                                        &error))
    {
      g_warning ("Failed to deserialize grid parasite: %s", error->message);
      g_error_free (error);
    }

  return grid;
}
