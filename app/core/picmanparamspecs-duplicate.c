/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanparamspecs-duplicate.c
 * Copyright (C) 2008-2009 Michael Natterer <mitch@picman.org>
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

#include <cairo.h>
#include <gegl.h>
#include <gegl-paramspecs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picmanparamspecs.h"
#include "picmanparamspecs-duplicate.h"


/* FIXME: this code is not yet general as it should be (gegl tool only atm) */

GParamSpec *
picman_param_spec_duplicate (GParamSpec *pspec)
{
  g_return_val_if_fail (pspec != NULL, NULL);

  if (G_IS_PARAM_SPEC_STRING (pspec))
    {
      GParamSpecString *spec = G_PARAM_SPEC_STRING (pspec);

      if (GEGL_IS_PARAM_SPEC_FILE_PATH (pspec))
        {
          return picman_param_spec_config_path (pspec->name,
                                              g_param_spec_get_nick (pspec),
                                              g_param_spec_get_blurb (pspec),
                                              PICMAN_CONFIG_PATH_FILE,
                                              spec->default_value,
                                              pspec->flags |
                                              PICMAN_CONFIG_PARAM_SERIALIZE);
        }
      else
        {
          static GQuark  multiline_quark = 0;
          GParamSpec    *new;

          if (! multiline_quark)
            multiline_quark = g_quark_from_static_string ("multiline");

          new = g_param_spec_string (pspec->name,
                                     g_param_spec_get_nick (pspec),
                                     g_param_spec_get_blurb (pspec),
                                     spec->default_value,
                                     pspec->flags |
                                     PICMAN_CONFIG_PARAM_SERIALIZE);

          if (GEGL_IS_PARAM_SPEC_MULTILINE (pspec))
            {
              g_param_spec_set_qdata (new, multiline_quark,
                                      GINT_TO_POINTER (TRUE));
            }

          return new;
        }
    }
  else if (G_IS_PARAM_SPEC_BOOLEAN (pspec))
    {
      GParamSpecBoolean *spec = G_PARAM_SPEC_BOOLEAN (pspec);

      return g_param_spec_boolean (pspec->name,
                                   g_param_spec_get_nick (pspec),
                                   g_param_spec_get_blurb (pspec),
                                   spec->default_value,
                                   pspec->flags |
                                   PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_ENUM (pspec))
    {
      GParamSpecEnum *spec = G_PARAM_SPEC_ENUM (pspec);

      return g_param_spec_enum (pspec->name,
                                g_param_spec_get_nick (pspec),
                                g_param_spec_get_blurb (pspec),
                                G_TYPE_FROM_CLASS (spec->enum_class),
                                spec->default_value,
                                pspec->flags |
                                PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (GEGL_IS_PARAM_SPEC_DOUBLE (pspec))
    {
      GeglParamSpecDouble *gspec = GEGL_PARAM_SPEC_DOUBLE (pspec);
      GParamSpecDouble    *spec = G_PARAM_SPEC_DOUBLE (pspec);

      return gegl_param_spec_double (pspec->name,
                                     g_param_spec_get_nick (pspec),
                                     g_param_spec_get_blurb (pspec),
                                     spec->minimum,
                                     spec->maximum,
                                     spec->default_value,
                                     gspec->ui_minimum,
                                     gspec->ui_maximum,
                                     gspec->ui_gamma,
                                     pspec->flags |
                                     PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_DOUBLE (pspec))
    {
      GParamSpecDouble *spec = G_PARAM_SPEC_DOUBLE (pspec);

      return g_param_spec_double (pspec->name,
                                  g_param_spec_get_nick (pspec),
                                  g_param_spec_get_blurb (pspec),
                                  spec->minimum,
                                  spec->maximum,
                                  spec->default_value,
                                  pspec->flags |
                                  PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_FLOAT (pspec))
    {
      GParamSpecFloat *spec = G_PARAM_SPEC_FLOAT (pspec);

      return g_param_spec_float (pspec->name,
                                 g_param_spec_get_nick (pspec),
                                 g_param_spec_get_blurb (pspec),
                                 spec->minimum,
                                 spec->maximum,
                                 spec->default_value,
                                 pspec->flags |
                                 PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (GEGL_IS_PARAM_SPEC_INT (pspec))
    {
      GeglParamSpecInt *gspec = GEGL_PARAM_SPEC_INT (pspec);
      GParamSpecInt *spec = G_PARAM_SPEC_INT (pspec);

      return gegl_param_spec_int (pspec->name,
                                  g_param_spec_get_nick (pspec),
                                  g_param_spec_get_blurb (pspec),
                                  spec->minimum,
                                  spec->maximum,
                                  spec->default_value,
                                  gspec->ui_minimum,
                                  gspec->ui_maximum,
                                  gspec->ui_gamma,
                                  pspec->flags |
                                  PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (GEGL_IS_PARAM_SPEC_SEED (pspec))
    {
      return gegl_param_spec_seed (pspec->name,
                                   g_param_spec_get_nick (pspec),
                                   g_param_spec_get_blurb (pspec),
                                   pspec->flags |
                                   PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_INT (pspec))
    {
      GParamSpecInt *spec = G_PARAM_SPEC_INT (pspec);

      return g_param_spec_int (pspec->name,
                               g_param_spec_get_nick (pspec),
                               g_param_spec_get_blurb (pspec),
                               spec->minimum,
                               spec->maximum,
                               spec->default_value,
                               pspec->flags |
                               PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_UINT (pspec))
    {
      GParamSpecUInt *spec = G_PARAM_SPEC_UINT (pspec);

      return g_param_spec_uint (pspec->name,
                                g_param_spec_get_nick (pspec),
                                g_param_spec_get_blurb (pspec),
                                spec->minimum,
                                spec->maximum,
                                spec->default_value,
                                pspec->flags |
                                PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (PICMAN_IS_PARAM_SPEC_RGB (pspec))
    {
      GValue  value = G_VALUE_INIT;
      PicmanRGB color;

      g_value_init (&value, PICMAN_TYPE_RGB);
      g_param_value_set_default (pspec, &value);
      picman_value_get_rgb (&value, &color);
      g_value_unset (&value);

      return picman_param_spec_rgb (pspec->name,
                                  g_param_spec_get_nick (pspec),
                                  g_param_spec_get_blurb (pspec),
                                  picman_param_spec_rgb_has_alpha (pspec),
                                  &color,
                                  pspec->flags |
                                  PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (GEGL_IS_PARAM_SPEC_COLOR (pspec))
    {
      GeglColor *gegl_color;
      PicmanRGB    picman_color;
      gdouble    r = 0.0;
      gdouble    g = 0.0;
      gdouble    b = 0.0;
      gdouble    a = 1.0;
      GValue     value = { 0, };

      g_value_init (&value, GEGL_TYPE_COLOR);
      g_param_value_set_default (pspec, &value);

      gegl_color = g_value_get_object (&value);
      if (gegl_color)
        gegl_color_get_rgba (gegl_color, &r, &g, &b, &a);

      picman_rgba_set (&picman_color, r, g, b, a);

      g_value_unset (&value);

      return picman_param_spec_rgb (pspec->name,
                                  g_param_spec_get_nick (pspec),
                                  g_param_spec_get_blurb (pspec),
                                  TRUE,
                                  &picman_color,
                                  pspec->flags |
                                  PICMAN_CONFIG_PARAM_SERIALIZE);
    }
  else if (G_IS_PARAM_SPEC_OBJECT (pspec) ||
           G_IS_PARAM_SPEC_POINTER (pspec))
    {
      /*  ignore object properties  */
      return NULL;
    }

  g_warning ("%s: not supported: %s (%s)\n", G_STRFUNC,
             g_type_name (G_TYPE_FROM_INSTANCE (pspec)), pspec->name);

  return NULL;
}
