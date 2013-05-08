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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picmanimage.h"
#include "core/picmanlayermask.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "filters-actions.h"
#include "filters-commands.h"

#include "picman-intl.h"


static const PicmanStringActionEntry filters_actions[] =
{
  { "filters-c2g", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Color to Gray..."), NULL,
    NC_("filters-action", "Color to grayscale conversion"),
    "gegl:c2g",
    NULL /* FIXME PICMAN_HELP_FILTER_C2G */ },

  { "filters-cartoon", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Ca_rtoon..."), NULL,
    NC_("filters-action", "Simulate a cartoon by enhancing edges"),
    "gegl:cartoon",
    NULL /* FIXME PICMAN_HELP_FILTER_CARTOON */ },

  { "filters-checkerboard", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Checkerboard..."), NULL,
    NC_("filters-action", "Create a checkerboard pattern"),
    "gegl:checkerboard",
    NULL /* FIXME PICMAN_HELP_FILTER_CHECKERBOARD */ },

  { "filters-color-reduction", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Color _Reduction..."), NULL,
    NC_("filters-action", "Reduce the number of colors in the image, with optional dithering"),
    "gegl:color-reduction",
    NULL /* FIXME PICMAN_HELP_FILTER_COLOR_TEMPERATURE */ },

  { "filters-color-temperature", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Color T_emperature..."), NULL,
    NC_("filters-action", "Change the color temperature of the image"),
    "gegl:color-temperature",
    NULL /* FIXME PICMAN_HELP_FILTER_COLOR_TEMPERATURE */ },

  { "filters-color-to-alpha", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Color to _Alpha..."), NULL,
    NC_("filters-action", "Convert a specified color to transparency"),
    "gegl:color-to-alpha",
    NULL /* FIXME PICMAN_HELP_FILTER_COLOR_TO_ALPHA */ },

  { "filters-dot", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Dots..."), NULL,
    NC_("filters-action", "Simplify image into an array of solid-colored dots"),
    "gegl:dot",
    NULL /* FIXME PICMAN_HELP_FILTER_DOT */ },

  { "filters-difference-of-gaussians", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Difference of Gaussians..."), NULL,
    NC_("filters-action", "Edge detection with control of edge thickness"),
    "gegl:difference-of-gaussians",
    NULL /* FIXME PICMAN_HELP_FILTER_DIFFERENCE_OF_GAUSSIANS */ },

  { "filters-gaussian-blur", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Gaussian Blur..."), NULL,
    NC_("filters-action", "Apply a gaussian blur"),
    "gegl:gaussian-blur",
    NULL /* FIXME PICMAN_HELP_FILTER_GAUSSIAN_BLUR */ },

  { "filters-laplace", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Laplace"), NULL,
    NC_("filters-action", "High-resolution edge detection"),
    "gegl:edge-laplace",
    NULL /* FIXME PICMAN_HELP_FILTER_LAPLACE */ },

  { "filters-lens-distortion", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Lens Distortion..."), NULL,
    NC_("filters-action", "Corrects lens distortion"),
    "gegl:lens-distortion",
    NULL /* FIXME PICMAN_HELP_FILTER_LENS_DISTORTION */ },

  { "filters-mono-mixer", PICMAN_STOCK_GEGL,
    NC_("filters-action", "Mono Mixer..."), NULL,
    NC_("filters-action", "Monochrome channel mixer"),
    "gegl:mono-mixer",
    NULL /* FIXME PICMAN_HELP_FILTER_MONO_MIXER */ },

  { "filters-noise-cie-lch", PICMAN_STOCK_GEGL,
    NC_("filters-action", "CIE lch Noise..."), NULL,
    NC_("filters-action", "Randomize lightness, chroma and hue independently"),
    "gegl:noise-CIE_lch",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_CIE_LCH */ },

  { "filters-noise-hsv", PICMAN_STOCK_GEGL,
    NC_("filters-action", "HSV Noise..."), NULL,
    NC_("filters-action", "Scattering pixel values in HSV space"),
    "gegl:noise-hsv",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_HSV */ },

  { "filters-noise-hurl", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Hurl..."), NULL,
    NC_("filters-action", "Completely randomize a fraction of pixels"),
    "gegl:noise-hurl",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_HURL */ },

  { "filters-noise-pick", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Pick..."), NULL,
    NC_("filters-action", "Randomly interchange some pixels with neighbors"),
    "gegl:noise-pick",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_PICK */ },

  { "filters-noise-rgb", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_RGB Noise..."), NULL,
    NC_("filters-action", "Distort colors by random amounts"),
    "gegl:noise-rgb",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_RGB */ },

  { "filters-noise-slur", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Slur..."), NULL,
    NC_("filters-action", "Randomly slide some pixels downward (similar to melting)"),
    "gegl:noise-slur",
    NULL /* FIXME PICMAN_HELP_FILTER_NOISE_SLUR */ },

  { "filters-photocopy", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Photocopy..."), NULL,
    NC_("filters-action", "Simulate color distortion produced by a copy machine"),
    "gegl:photocopy",
    NULL /* FIXME PICMAN_HELP_FILTER_PHOTOCOPY */ },

  { "filters-pixelize", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Pixelize..."), NULL,
    NC_("filters-action", "Simplify image into an array of solid-colored squares"),
    "gegl:pixelize",
    NULL /* FIXME PICMAN_HELP_FILTER_PIXELIZE */ },

  { "filters-polar-coordinates", PICMAN_STOCK_GEGL,
    NC_("filters-action", "P_olar Coordinates..."), NULL,
    NC_("filters-action", "Convert image to or from polar coordinates"),
    "gegl:polar-coordinates",
    NULL /* FIXME PICMAN_HELP_FILTER_POLAR_COORDINATES */ },

  { "filters-ripple", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Ripple..."), NULL,
    NC_("filters-action", "Displace pixels in a ripple pattern"),
    "gegl:ripple",
    NULL /* FIXME PICMAN_HELP_FILTER_RIPPLE */ },

  { "filters-semi-flatten", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Semi-Flatten..."), NULL,
    NC_("filters-action", "Replace partial transparency with a color"),
    "picman:semi-flatten",
    NULL /* FIXME PICMAN_HELP_FILTER_POLAR_COORDINATES */ },

  { "filters-sobel", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Sobel..."), NULL,
    NC_("filters-action", "Specialized direction-dependent edge-detection"),
    "gegl:edge-sobel",
    NULL /* FIXME PICMAN_HELP_FILTER_SOBEL */ },

  { "filters-softglow", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Softglow..."), NULL,
    NC_("filters-action", "Simulate glow by making highlights intense and fuzzy"),
    "gegl:softglow",
    NULL /* FIXME PICMAN_HELP_FILTER_SOFTGLOW */ },

  { "filters-threshold-alpha", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Threshold Alpha..."), NULL,
    NC_("filters-action", "Make transparency all-or-nothing"),
    "picman:threshold-alpha",
    NULL /* FIXME PICMAN_HELP_FILTER_POLAR_COORDINATES */ },

  { "filters-unsharp-mask", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Unsharp Mask..."), NULL,
    NC_("filters-action", "The most widely used method for sharpening an image"),
    "gegl:unsharp-mask",
    NULL /* FIXME PICMAN_HELP_FILTER_UNSHARP_MASK */ },

  { "filters-vignette", PICMAN_STOCK_GEGL,
    NC_("filters-action", "_Vignette..."), NULL,
    NC_("filters-action", "Applies a vignette to an image"),
    "gegl:vignette",
    NULL /* FIXME PICMAN_HELP_FILTER_VIGNETTE */ },
};

void
filters_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_string_actions (group, "filters-action",
                                        filters_actions,
                                        G_N_ELEMENTS (filters_actions),
                                        G_CALLBACK (filters_filter_cmd_callback));
}

void
filters_actions_update (PicmanActionGroup *group,
                        gpointer         data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable = NULL;
  gboolean      writable = FALSE;
  gboolean      gray     = FALSE;
  gboolean      alpha    = FALSE;

  image = action_data_get_image (data);

  if (image)
    {
      drawable = picman_image_get_active_drawable (image);

      if (drawable)
        {
          PicmanItem *item;

          alpha = picman_drawable_has_alpha (drawable);
          gray  = picman_drawable_is_gray (drawable);

          if (PICMAN_IS_LAYER_MASK (drawable))
            item = PICMAN_ITEM (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));
          else
            item = PICMAN_ITEM (drawable);

          writable = ! picman_item_is_content_locked (item);

          if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
            writable = FALSE;
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("filters-c2g",                     writable && !gray);
  SET_SENSITIVE ("filters-cartoon",                 writable);
  SET_SENSITIVE ("filters-checkerboard",            writable);
  SET_SENSITIVE ("filters-color-reduction",         writable);
  SET_SENSITIVE ("filters-color-temperature",       writable && !gray);
  SET_SENSITIVE ("filters-color-to-alpha",          writable && !gray && alpha);
  SET_SENSITIVE ("filters-difference-of-gaussians", writable);
  SET_SENSITIVE ("filters-dot",                     writable);
  SET_SENSITIVE ("filters-gaussian-blur",           writable);
  SET_SENSITIVE ("filters-laplace",                 writable);
  SET_SENSITIVE ("filters-mono-mixer",              writable && !gray);
  SET_SENSITIVE ("filters-noise-cie-lch",           writable);
  SET_SENSITIVE ("filters-noise-hsv",               writable && !gray);
  SET_SENSITIVE ("filters-noise-hurl",              writable);
  SET_SENSITIVE ("filters-noise-pick",              writable);
  SET_SENSITIVE ("filters-noise-rgb",               writable);
  SET_SENSITIVE ("filters-noise-slur",              writable);
  SET_SENSITIVE ("filters-lens-distortion",         writable);
  SET_SENSITIVE ("filters-photocopy",               writable);
  SET_SENSITIVE ("filters-pixelize",                writable);
  SET_SENSITIVE ("filters-polar-coordinates",       writable);
  SET_SENSITIVE ("filters-ripple",                  writable);
  SET_SENSITIVE ("filters-sobel",                   writable);
  SET_SENSITIVE ("filters-softglow",                writable);
  SET_SENSITIVE ("filters-semi-flatten",            writable && alpha);
  SET_SENSITIVE ("filters-threshold-alpha",         writable && alpha);
  SET_SENSITIVE ("filters-unsharp-mask",            writable);

#undef SET_SENSITIVE
}
