/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-babl.c
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

#include "config.h"

#include <gegl.h>

#include "picman-gegl-types.h"

#include "picman-babl.h"

#include "picman-intl.h"


void
picman_babl_init (void)
{
  babl_format_new ("name", "R' u8",
                   babl_model ("R'G'B'A"),
                   babl_type ("u8"),
                   babl_component ("R'"),
                   NULL);
  babl_format_new ("name", "G' u8",
                   babl_model ("R'G'B'A"),
                   babl_type ("u8"),
                   babl_component ("G'"),
                   NULL);
  babl_format_new ("name", "B' u8",
                   babl_model ("R'G'B'A"),
                   babl_type ("u8"),
                   babl_component ("B'"),
                   NULL);
  babl_format_new ("name", "A u8",
                   babl_model ("R'G'B'A"),
                   babl_type ("u8"),
                   babl_component ("A"),
                   NULL);

  babl_format_new ("name", "R' u16",
                   babl_model ("R'G'B'A"),
                   babl_type ("u16"),
                   babl_component ("R'"),
                   NULL);
  babl_format_new ("name", "G' u16",
                   babl_model ("R'G'B'A"),
                   babl_type ("u16"),
                   babl_component ("G'"),
                   NULL);
  babl_format_new ("name", "B' u16",
                   babl_model ("R'G'B'A"),
                   babl_type ("u16"),
                   babl_component ("B'"),
                   NULL);
  babl_format_new ("name", "A u16",
                   babl_model ("R'G'B'A"),
                   babl_type ("u16"),
                   babl_component ("A"),
                   NULL);

  babl_format_new ("name", "R u32",
                   babl_model ("RGBA"),
                   babl_type ("u32"),
                   babl_component ("R"),
                   NULL);
  babl_format_new ("name", "G u32",
                   babl_model ("RGBA"),
                   babl_type ("u32"),
                   babl_component ("G"),
                   NULL);
  babl_format_new ("name", "B u32",
                   babl_model ("RGBA"),
                   babl_type ("u32"),
                   babl_component ("B"),
                   NULL);
  babl_format_new ("name", "A u32",
                   babl_model ("RGBA"),
                   babl_type ("u32"),
                   babl_component ("A"),
                   NULL);

  babl_format_new ("name", "R half",
                   babl_model ("RGBA"),
                   babl_type ("half"),
                   babl_component ("R"),
                   NULL);
  babl_format_new ("name", "G half",
                   babl_model ("RGBA"),
                   babl_type ("half"),
                   babl_component ("G"),
                   NULL);
  babl_format_new ("name", "B half",
                   babl_model ("RGBA"),
                   babl_type ("half"),
                   babl_component ("B"),
                   NULL);
  babl_format_new ("name", "A half",
                   babl_model ("RGBA"),
                   babl_type ("half"),
                   babl_component ("A"),
                   NULL);

  babl_format_new ("name", "R float",
                   babl_model ("RGBA"),
                   babl_type ("float"),
                   babl_component ("R"),
                   NULL);
  babl_format_new ("name", "G float",
                   babl_model ("RGBA"),
                   babl_type ("float"),
                   babl_component ("G"),
                   NULL);
  babl_format_new ("name", "B float",
                   babl_model ("RGBA"),
                   babl_type ("float"),
                   babl_component ("B"),
                   NULL);
  babl_format_new ("name", "A float",
                   babl_model ("RGBA"),
                   babl_type ("float"),
                   babl_component ("A"),
                   NULL);

  babl_format_new ("name", "A double",
                   babl_model ("RGBA"),
                   babl_type ("double"),
                   babl_component ("A"),
                   NULL);
}

static const struct
{
  const gchar *name;
  const gchar *description;
}
babl_descriptions[] =
{
  { "R'G'B' u8",  N_("RGB") },
  { "R'G'B' u16", N_("RGB") },
  { "RGB u32",    N_("RGB") },
  { "RGB half",   N_("RGB") },
  { "RGB float",  N_("RGB") },

  { "R'G'B'A u8", N_("RGB-alpha") },
  { "R'G'B'A u16",N_("RGB-alpha") },
  { "RGBA u32",   N_("RGB-alpha") },
  { "RGBA half",  N_("RGB-alpha") },
  { "RGBA float", N_("RGB-alpha") },

  { "Y' u8",      N_("Grayscale") },
  { "Y u8",       N_("Grayscale") },
  { "Y' u16",     N_("Grayscale") },
  { "Y u16",      N_("Grayscale") },
  { "Y u32",      N_("Grayscale") },
  { "Y half",     N_("Grayscale") },
  { "Y float",    N_("Grayscale") },

  { "Y'A u8",     N_("Grayscale-alpha") },
  { "Y'A u16",    N_("Grayscale-alpha") },
  { "YA u32",     N_("Grayscale-alpha") },
  { "YA half",    N_("Grayscale-alpha") },
  { "YA float",   N_("Grayscale-alpha") },

  { "R' u8",      N_("Red component") },
  { "R' u16",     N_("Red component") },
  { "R u32",      N_("Red component") },
  { "R half",     N_("Red component") },
  { "R float",    N_("Red component") },

  { "G' u8",      N_("Green component") },
  { "G' u16",     N_("Green component") },
  { "G u32",      N_("Green component") },
  { "G half",     N_("Green component") },
  { "G float",    N_("Green component") },

  { "B' u8",      N_("Blue component") },
  { "B' u16",     N_("Blue component") },
  { "B u32",      N_("Blue component") },
  { "B half",     N_("Blue component") },
  { "B float",    N_("Blue component") },

  { "A u8",       N_("Alpha component") },
  { "A u16",      N_("Alpha component") },
  { "A u32",      N_("Alpha component") },
  { "A half",     N_("Alpha component") },
  { "A float",    N_("Alpha component") },
  { "A double",   N_("Alpha component") }
};

static GHashTable *babl_description_hash = NULL;

const gchar *
picman_babl_get_description (const Babl *babl)
{
  const gchar *description;

  g_return_val_if_fail (babl != NULL, NULL);

  if (G_UNLIKELY (! babl_description_hash))
    {
      gint i;

      babl_description_hash = g_hash_table_new (g_str_hash,
                                                g_str_equal);

      for (i = 0; i < G_N_ELEMENTS (babl_descriptions); i++)
        g_hash_table_insert (babl_description_hash,
                             (gpointer) babl_descriptions[i].name,
                             gettext (babl_descriptions[i].description));
    }

  if (babl_format_is_palette (babl))
    {
      if (babl_format_has_alpha (babl))
        return _("Indexed-alpha");
      else
        return _("Indexed");
    }

  description = g_hash_table_lookup (babl_description_hash,
                                     babl_get_name (babl));

  if (description)
    return description;

  return g_strconcat ("ERROR: unknown Babl format ",
                      babl_get_name (babl), NULL);
}

PicmanImageBaseType
picman_babl_format_get_base_type (const Babl *format)
{
  const Babl *model;

  g_return_val_if_fail (format != NULL, -1);

  model = babl_format_get_model (format);

  if (model == babl_model ("Y")  ||
      model == babl_model ("Y'") ||
      model == babl_model ("YA") ||
      model == babl_model ("Y'A"))
    {
      return PICMAN_GRAY;
    }
  else if (model == babl_model ("RGB")    ||
           model == babl_model ("R'G'B'") ||
           model == babl_model ("RGBA")   ||
           model == babl_model ("R'G'B'A"))
    {
      return PICMAN_RGB;
    }
  else if (babl_format_is_palette (format))
    {
      return PICMAN_INDEXED;
    }

  g_return_val_if_reached (-1);
}

PicmanPrecision
picman_babl_format_get_precision (const Babl *format)
{
  const Babl *type;

  g_return_val_if_fail (format != NULL, -1);

  type = babl_format_get_type (format, 0);

  if (type == babl_type ("u8"))
    return PICMAN_PRECISION_U8;
  else if (type == babl_type ("u16"))
    return PICMAN_PRECISION_U16;
  else if (type == babl_type ("u32"))
    return PICMAN_PRECISION_U32;
  else if (type == babl_type ("half"))
    return PICMAN_PRECISION_HALF;
  else if (type == babl_type ("float"))
    return PICMAN_PRECISION_FLOAT;

  g_return_val_if_reached (-1);
}

gboolean
picman_babl_format_get_linear (const Babl *format)
{
  const Babl *model;

  g_return_val_if_fail (format != NULL, FALSE);

  model = babl_format_get_model (format);

  if (model == babl_model ("Y")   ||
      model == babl_model ("YA")  ||
      model == babl_model ("RGB") ||
      model == babl_model ("RGBA"))
    {
      return TRUE;
    }
  else if (model == babl_model ("Y'")     ||
           model == babl_model ("Y'A")    ||
           model == babl_model ("R'G'B'") ||
           model == babl_model ("R'G'B'A"))
    {
      return FALSE;
    }
  else if (babl_format_is_palette (format))
    {
      return FALSE;
    }

  g_return_val_if_reached (FALSE);
}

const Babl *
picman_babl_format (PicmanImageBaseType  base_type,
                  PicmanPrecision      precision,
                  gboolean           with_alpha)
{
  switch (base_type)
    {
    case PICMAN_RGB:
      switch (precision)
        {
        case PICMAN_PRECISION_U8:
          if (with_alpha)
            return babl_format ("R'G'B'A u8");
          else
            return babl_format ("R'G'B' u8");

        case PICMAN_PRECISION_U16:
          if (with_alpha)
            return babl_format ("R'G'B'A u16");
          else
            return babl_format ("R'G'B' u16");

        case PICMAN_PRECISION_U32:
          if (with_alpha)
            return babl_format ("RGBA u32");
          else
            return babl_format ("RGB u32");

        case PICMAN_PRECISION_HALF:
          if (with_alpha)
            return babl_format ("RGBA half");
          else
            return babl_format ("RGB half");

        case PICMAN_PRECISION_FLOAT:
          if (with_alpha)
            return babl_format ("RGBA float");
          else
            return babl_format ("RGB float");

        default:
          break;
        }
      break;

    case PICMAN_GRAY:
      switch (precision)
        {
        case PICMAN_PRECISION_U8:
          if (with_alpha)
            return babl_format ("Y'A u8");
          else
            return babl_format ("Y' u8");

        case PICMAN_PRECISION_U16:
          if (with_alpha)
            return babl_format ("Y'A u16");
          else
            return babl_format ("Y' u16");

        case PICMAN_PRECISION_U32:
          if (with_alpha)
            return babl_format ("YA u32");
          else
            return babl_format ("Y u32");

        case PICMAN_PRECISION_HALF:
          if (with_alpha)
            return babl_format ("YA half");
          else
            return babl_format ("Y half");

        case PICMAN_PRECISION_FLOAT:
          if (with_alpha)
            return babl_format ("YA float");
          else
            return babl_format ("Y float");

        default:
          break;
        }
      break;

    case PICMAN_INDEXED:
      /* need to use the image's api for this */
      break;
    }

  g_return_val_if_reached (NULL);
}

const Babl *
picman_babl_mask_format (PicmanPrecision precision)
{
  switch (precision)
    {
    case PICMAN_PRECISION_U8:    return babl_format ("Y u8");
    case PICMAN_PRECISION_U16:   return babl_format ("Y u16");
    case PICMAN_PRECISION_U32:   return babl_format ("Y u32");
    case PICMAN_PRECISION_HALF:  return babl_format ("Y half");
    case PICMAN_PRECISION_FLOAT: return babl_format ("Y float");
    }

  g_return_val_if_reached (NULL);
}

const Babl *
picman_babl_component_format (PicmanImageBaseType base_type,
                            PicmanPrecision     precision,
                            gint              index)
{
  switch (base_type)
    {
    case PICMAN_RGB:
      switch (precision)
        {
        case PICMAN_PRECISION_U8:
          switch (index)
            {
            case 0: return babl_format ("R' u8");
            case 1: return babl_format ("G' u8");
            case 2: return babl_format ("B' u8");
            case 3: return babl_format ("A u8");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_U16:
          switch (index)
            {
            case 0: return babl_format ("R' u16");
            case 1: return babl_format ("G' u16");
            case 2: return babl_format ("B' u16");
            case 3: return babl_format ("A u16");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_U32:
          switch (index)
            {
            case 0: return babl_format ("R u32");
            case 1: return babl_format ("G u32");
            case 2: return babl_format ("B u32");
            case 3: return babl_format ("A u32");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_HALF:
          switch (index)
            {
            case 0: return babl_format ("R half");
            case 1: return babl_format ("G half");
            case 2: return babl_format ("B half");
            case 3: return babl_format ("A half");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_FLOAT:
          switch (index)
            {
            case 0: return babl_format ("R float");
            case 1: return babl_format ("G float");
            case 2: return babl_format ("B float");
            case 3: return babl_format ("A float");
            default:
              break;
            }
          break;

        default:
          break;
        }
      break;

    case PICMAN_GRAY:
      switch (precision)
        {
        case PICMAN_PRECISION_U8:
          switch (index)
            {
            case 0: return babl_format ("Y' u8");
            case 1: return babl_format ("A u8");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_U16:
          switch (index)
            {
            case 0: return babl_format ("Y' u16");
            case 1: return babl_format ("A u16");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_U32:
          switch (index)
            {
            case 0: return babl_format ("Y u32");
            case 1: return babl_format ("A u32");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_HALF:
          switch (index)
            {
            case 0: return babl_format ("Y half");
            case 1: return babl_format ("A half");
            default:
              break;
            }
          break;

        case PICMAN_PRECISION_FLOAT:
          switch (index)
            {
            case 0: return babl_format ("Y float");
            case 1: return babl_format ("A float");
            default:
              break;
            }
          break;

        default:
          break;
        }
      break;

    case PICMAN_INDEXED:
      /* need to use the image's api for this */
      break;
    }

  g_return_val_if_reached (NULL);
}
