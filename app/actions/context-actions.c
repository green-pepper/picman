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

#include "core/picman.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "context-actions.h"
#include "context-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static const PicmanActionEntry context_actions[] =
{
  { "context-menu",            NULL,                      NC_("context-action",
                                                              "_Context")    },
  { "context-colors-menu",     PICMAN_STOCK_DEFAULT_COLORS, NC_("context-action",
                                                              "_Colors")     },
  { "context-opacity-menu",    PICMAN_STOCK_TRANSPARENCY,   NC_("context-action",
                                                              "_Opacity")    },
  { "context-paint-mode-menu", PICMAN_STOCK_TOOL_PENCIL,    NC_("context-action",
                                                              "Paint _Mode") },
  { "context-tool-menu",       PICMAN_STOCK_TOOLS,          NC_("context-action",
                                                              "_Tool")       },
  { "context-brush-menu",      PICMAN_STOCK_BRUSH,          NC_("context-action",
                                                              "_Brush")      },
  { "context-pattern-menu",    PICMAN_STOCK_PATTERN,        NC_("context-action",
                                                              "_Pattern")    },
  { "context-palette-menu",    PICMAN_STOCK_PALETTE,        NC_("context-action",
                                                              "_Palette")    },
  { "context-gradient-menu",   PICMAN_STOCK_GRADIENT,       NC_("context-action",
                                                              "_Gradient")   },
  { "context-font-menu",       PICMAN_STOCK_FONT,           NC_("context-action",
                                                              "_Font")       },

  { "context-brush-shape-menu",    NULL,                  NC_("context-action",
                                                              "_Shape")      },
  { "context-brush-radius-menu",   NULL,                  NC_("context-action",
                                                              "_Radius")     },
  { "context-brush-spikes-menu",   NULL,                  NC_("context-action",
                                                              "S_pikes")     },
  { "context-brush-hardness-menu", NULL,                  NC_("context-action",
                                                              "_Hardness")   },
  { "context-brush-aspect-menu",   NULL,                  NC_("context-action",
                                                              "_Aspect Ratio")},
  { "context-brush-angle-menu",    NULL,                  NC_("context-action",
                                                              "A_ngle")      },

  { "context-colors-default", PICMAN_STOCK_DEFAULT_COLORS,
    NC_("context-action", "_Default Colors"), "D",
    NC_("context-action",
        "Set foreground color to black, background color to white"),
    G_CALLBACK (context_colors_default_cmd_callback),
    PICMAN_HELP_TOOLBOX_DEFAULT_COLORS },

  { "context-colors-swap", PICMAN_STOCK_SWAP_COLORS,
    NC_("context-action", "S_wap Colors"), "X",
    NC_("context-action", "Exchange foreground and background colors"),
    G_CALLBACK (context_colors_swap_cmd_callback),
    PICMAN_HELP_TOOLBOX_SWAP_COLORS }
};

static PicmanEnumActionEntry context_palette_foreground_actions[] =
{
  { "context-palette-foreground-set", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-palette-foreground-first", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-palette-foreground-last", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-palette-foreground-previous", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Previous", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-palette-foreground-next", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Next", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-palette-foreground-previous-skip", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-palette-foreground-next-skip", PICMAN_STOCK_PALETTE,
    "Foreground Palette Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static PicmanEnumActionEntry context_palette_background_actions[] =
{
  { "context-palette-background-set", PICMAN_STOCK_PALETTE,
    "Background Palette Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-palette-background-first", PICMAN_STOCK_PALETTE,
    "Background Palette Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-palette-background-last", PICMAN_STOCK_PALETTE,
    "Background Palette Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-palette-background-previous", PICMAN_STOCK_PALETTE,
    "Background Palette Color Previous", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-palette-background-next", PICMAN_STOCK_PALETTE,
    "Background Palette Color Next", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-palette-background-previous-skip", PICMAN_STOCK_PALETTE,
    "Background Palette Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-palette-background-next-skip", PICMAN_STOCK_PALETTE,
    "Background Palette Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static PicmanEnumActionEntry context_colormap_foreground_actions[] =
{
  { "context-colormap-foreground-set", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-colormap-foreground-first", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-colormap-foreground-last", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-colormap-foreground-previous", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Previous", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-colormap-foreground-next", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Next", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-colormap-foreground-previous-skip", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-colormap-foreground-next-skip", PICMAN_STOCK_COLORMAP,
    "Foreground Colormap Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static PicmanEnumActionEntry context_colormap_background_actions[] =
{
  { "context-colormap-background-set", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-colormap-background-first", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-colormap-background-last", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-colormap-background-previous", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Previous", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-colormap-background-next", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Next", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-colormap-background-previous-skip", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-colormap-background-next-skip", PICMAN_STOCK_COLORMAP,
    "Background Colormap Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static PicmanEnumActionEntry context_swatch_foreground_actions[] =
{
  { "context-swatch-foreground-set", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-swatch-foreground-first", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-swatch-foreground-last", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-swatch-foreground-previous", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Previous", "9", NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-swatch-foreground-next", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Next", "0", NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-swatch-foreground-previous-skip", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-swatch-foreground-next-skip", PICMAN_STOCK_PALETTE,
    "Foreground Swatch Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static PicmanEnumActionEntry context_swatch_background_actions[] =
{
  { "context-swatch-background-set", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, FALSE,
    NULL },
  { "context-swatch-background-first", PICMAN_STOCK_PALETTE,
    "Background Swatch Color First", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-swatch-background-last", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Last", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-swatch-background-previous", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Previous", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-swatch-background-next", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Next", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-swatch-background-previous-skip", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Skip Back", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-swatch-background-next-skip", PICMAN_STOCK_PALETTE,
    "Background Swatch Color Skip Forward", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_red_actions[] =
{
  { "context-foreground-red-set", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-red-minimum", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-red-maximum", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-red-decrease", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-red-increase", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-red-decrease-skip", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-red-increase-skip", PICMAN_STOCK_CHANNEL_RED,
    "Foreground Red Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_green_actions[] =
{
  { "context-foreground-green-set", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-green-minimum", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-green-maximum", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-green-decrease", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-green-increase", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-green-decrease-skip", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-green-increase-skip", PICMAN_STOCK_CHANNEL_GREEN,
    "Foreground Green Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_blue_actions[] =
{
  { "context-foreground-blue-set", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-blue-minimum", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-blue-maximum", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-blue-decrease", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-blue-increase", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-blue-decrease-skip", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-blue-increase-skip", PICMAN_STOCK_CHANNEL_BLUE,
    "Foreground Blue Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_red_actions[] =
{
  { "context-background-red-set", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-red-minimum", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-red-maximum", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-red-decrease", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-red-increase", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-red-decrease-skip", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-red-increase-skip", PICMAN_STOCK_CHANNEL_RED,
    "Background Red Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_green_actions[] =
{
  { "context-background-green-set", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-green-minimum", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-green-maximum", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-green-decrease", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-green-increase", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-green-decrease-skip", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-green-increase-skip", PICMAN_STOCK_CHANNEL_GREEN,
    "Background Green Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_blue_actions[] =
{
  { "context-background-blue-set", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-blue-minimum", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-blue-maximum", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-blue-decrease", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-blue-increase", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-blue-decrease-skip", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-blue-increase-skip", PICMAN_STOCK_CHANNEL_BLUE,
    "Background Blue Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_hue_actions[] =
{
  { "context-foreground-hue-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-hue-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-hue-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-hue-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-hue-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-hue-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-hue-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Hue Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_saturation_actions[] =
{
  { "context-foreground-saturation-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-saturation-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-saturation-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-saturation-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-saturation-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-saturation-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-saturation-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Saturation Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_foreground_value_actions[] =
{
  { "context-foreground-value-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-foreground-value-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-foreground-value-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-foreground-value-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-value-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-foreground-value-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-foreground-value-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Foreground Value Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_hue_actions[] =
{
  { "context-background-hue-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-hue-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-hue-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-hue-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-hue-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-hue-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-hue-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Hue Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_saturation_actions[] =
{
  { "context-background-saturation-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-saturation-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-saturation-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-saturation-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-saturation-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-saturation-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-saturation-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Saturation Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_background_value_actions[] =
{
  { "context-background-value-set", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Set", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-background-value-minimum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Minimum", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-background-value-maximum", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Maximum", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-background-value-decrease", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Decrease", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-background-value-increase", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Increase", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-background-value-decrease-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Decrease 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-background-value-increase-skip", PICMAN_STOCK_TOOL_HUE_SATURATION,
    "Background Value Increase 10%", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_opacity_actions[] =
{
  { "context-opacity-set", PICMAN_STOCK_TRANSPARENCY,
    "Set Transparency", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-opacity-transparent", PICMAN_STOCK_TRANSPARENCY,
    "Completely Transparent", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-opacity-opaque", PICMAN_STOCK_TRANSPARENCY,
    "Completely Opaque", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-opacity-decrease", PICMAN_STOCK_TRANSPARENCY,
    "More Transparent", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-opacity-increase", PICMAN_STOCK_TRANSPARENCY,
    "More Opaque", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-opacity-decrease-skip", PICMAN_STOCK_TRANSPARENCY,
    "10% More Transparent", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-opacity-increase-skip", PICMAN_STOCK_TRANSPARENCY,
    "10% More Opaque", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_paint_mode_actions[] =
{
  { "context-paint-mode-first", PICMAN_STOCK_TOOL_PENCIL,
    "First Paint Mode", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-paint-mode-last", PICMAN_STOCK_TOOL_PENCIL,
    "Last Paint Mode", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-paint-mode-previous", PICMAN_STOCK_TOOL_PENCIL,
    "Previous Paint Mode", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-paint-mode-next", PICMAN_STOCK_TOOL_PENCIL,
    "Next Paint Mode", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_tool_select_actions[] =
{
  { "context-tool-select-set", PICMAN_STOCK_TOOLS,
    "Select Tool by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-tool-select-first", PICMAN_STOCK_TOOLS,
    "First Tool", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-tool-select-last", PICMAN_STOCK_TOOLS,
    "Last Tool", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-tool-select-previous", PICMAN_STOCK_TOOLS,
    "Previous Tool", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-tool-select-next", PICMAN_STOCK_TOOLS,
    "Next Tool", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_select_actions[] =
{
  { "context-brush-select-set", PICMAN_STOCK_BRUSH,
    "Select Brush by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-select-first", PICMAN_STOCK_BRUSH,
    "First Brush", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-select-last", PICMAN_STOCK_BRUSH,
    "Last Brush", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-select-previous", PICMAN_STOCK_BRUSH,
    "Previous Brush", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-select-next", PICMAN_STOCK_BRUSH,
    "Next Brush", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_pattern_select_actions[] =
{
  { "context-pattern-select-set", PICMAN_STOCK_PATTERN,
    "Select Pattern by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-pattern-select-first", PICMAN_STOCK_PATTERN,
    "First Pattern", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-pattern-select-last", PICMAN_STOCK_PATTERN,
    "Last Pattern", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-pattern-select-previous", PICMAN_STOCK_PATTERN,
    "Previous Pattern", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-pattern-select-next", PICMAN_STOCK_PATTERN,
    "Next Pattern", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_palette_select_actions[] =
{
  { "context-palette-select-set", PICMAN_STOCK_PALETTE,
    "Select Palette by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-palette-select-first", PICMAN_STOCK_PALETTE,
    "First Palette", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-palette-select-last", PICMAN_STOCK_PALETTE,
    "Last Palette", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-palette-select-previous", PICMAN_STOCK_PALETTE,
    "Previous Palette", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-palette-select-next", PICMAN_STOCK_PALETTE,
    "Next Palette", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_gradient_select_actions[] =
{
  { "context-gradient-select-set", PICMAN_STOCK_GRADIENT,
    "Select Gradient by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-gradient-select-first", PICMAN_STOCK_GRADIENT,
    "First Gradient", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-gradient-select-last", PICMAN_STOCK_GRADIENT,
    "Last Gradient", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-gradient-select-previous", PICMAN_STOCK_GRADIENT,
    "Previous Gradient", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-gradient-select-next", PICMAN_STOCK_GRADIENT,
    "Next Gradient", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_font_select_actions[] =
{
  { "context-font-select-set", PICMAN_STOCK_FONT,
    "Select Font by Index", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-font-select-first", PICMAN_STOCK_FONT,
    "First Font", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-font-select-last", PICMAN_STOCK_FONT,
    "Last Font", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-font-select-previous", PICMAN_STOCK_FONT,
    "Previous Font", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-font-select-next", PICMAN_STOCK_FONT,
    "Next Font", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_spacing_actions[] =
{
  { "context-brush-spacing-set", PICMAN_STOCK_BRUSH,
    "Set Brush Spacing", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-spacing-minimum", PICMAN_STOCK_BRUSH,
    "Minimum Spacing", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-spacing-maximum", PICMAN_STOCK_BRUSH,
    "Maximum Spacing", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-spacing-decrease", PICMAN_STOCK_BRUSH,
    "Decrease Spacing", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-spacing-increase", PICMAN_STOCK_BRUSH,
    "Increase Spacing", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-spacing-decrease-skip", PICMAN_STOCK_BRUSH,
    "Decrease Spacing More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-spacing-increase-skip", PICMAN_STOCK_BRUSH,
    "Increase Spacing More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_shape_actions[] =
{
  { "context-brush-shape-circle", PICMAN_STOCK_BRUSH,
    "Circular Brush", NULL, NULL,
    PICMAN_BRUSH_GENERATED_CIRCLE, FALSE,
    NULL },
  { "context-brush-shape-square", PICMAN_STOCK_BRUSH,
    "Square Brush", NULL, NULL,
    PICMAN_BRUSH_GENERATED_SQUARE, FALSE,
    NULL },
  { "context-brush-shape-diamond", PICMAN_STOCK_BRUSH,
    "Diamond Brush", NULL, NULL,
    PICMAN_BRUSH_GENERATED_DIAMOND, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_radius_actions[] =
{
  { "context-brush-radius-set", PICMAN_STOCK_BRUSH,
    "Set Brush Radius", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-radius-minimum", PICMAN_STOCK_BRUSH,
    "Minimum Radius", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-radius-maximum", PICMAN_STOCK_BRUSH,
    "Maximum Radius", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-radius-decrease-less", PICMAN_STOCK_BRUSH,
    "Decrease Radius Less", NULL, NULL,
    PICMAN_ACTION_SELECT_SMALL_PREVIOUS, FALSE,
    NULL },
  { "context-brush-radius-increase-less", PICMAN_STOCK_BRUSH,
    "Increase Radius Less", NULL, NULL,
    PICMAN_ACTION_SELECT_SMALL_NEXT, FALSE,
    NULL },
  { "context-brush-radius-decrease", PICMAN_STOCK_BRUSH,
    "Decrease Radius", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-radius-increase", PICMAN_STOCK_BRUSH,
    "Increase Radius", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-radius-decrease-skip", PICMAN_STOCK_BRUSH,
    "Decrease Radius More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-radius-increase-skip", PICMAN_STOCK_BRUSH,
    "Increase Radius More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL },
  { "context-brush-radius-decrease-percent", PICMAN_STOCK_BRUSH,
    "Decrease Radius Relative", NULL, NULL,
    PICMAN_ACTION_SELECT_PERCENT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-radius-increase-percent", PICMAN_STOCK_BRUSH,
    "Increase Radius Relative", NULL, NULL,
    PICMAN_ACTION_SELECT_PERCENT_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_spikes_actions[] =
{
  { "context-brush-spikes-set", PICMAN_STOCK_BRUSH,
    "Set Brush Spikes", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-spikes-minimum", PICMAN_STOCK_BRUSH,
    "Minimum Spikes", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-spikes-maximum", PICMAN_STOCK_BRUSH,
    "Maximum Spikes", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-spikes-decrease", PICMAN_STOCK_BRUSH,
    "Decrease Spikes", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-spikes-increase", PICMAN_STOCK_BRUSH,
    "Increase Spikes", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-spikes-decrease-skip", PICMAN_STOCK_BRUSH,
    "Decrease Spikes More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-spikes-increase-skip", PICMAN_STOCK_BRUSH,
    "Increase Spikes More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_hardness_actions[] =
{
  { "context-brush-hardness-set", PICMAN_STOCK_BRUSH,
    "Set Brush Hardness", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-hardness-minimum", PICMAN_STOCK_BRUSH,
    "Minimum Hardness", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-hardness-maximum", PICMAN_STOCK_BRUSH,
    "Maximum Hardness", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-hardness-decrease", PICMAN_STOCK_BRUSH,
    "Decrease Hardness", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-hardness-increase", PICMAN_STOCK_BRUSH,
    "Increase Hardness", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-hardness-decrease-skip", PICMAN_STOCK_BRUSH,
    "Decrease Hardness More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-hardness-increase-skip", PICMAN_STOCK_BRUSH,
    "Increase Hardness More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_aspect_actions[] =
{
  { "context-brush-aspect-set", PICMAN_STOCK_BRUSH,
    "Set Brush Aspect", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-aspect-minimum", PICMAN_STOCK_BRUSH,
    "Minimum Aspect", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-aspect-maximum", PICMAN_STOCK_BRUSH,
    "Maximum Aspect", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-aspect-decrease", PICMAN_STOCK_BRUSH,
    "Decrease Aspect", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-aspect-increase", PICMAN_STOCK_BRUSH,
    "Increase Aspect", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-aspect-decrease-skip", PICMAN_STOCK_BRUSH,
    "Decrease Aspect More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-aspect-increase-skip", PICMAN_STOCK_BRUSH,
    "Increase Aspect More", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};

static const PicmanEnumActionEntry context_brush_angle_actions[] =
{
  { "context-brush-angle-set", PICMAN_STOCK_BRUSH,
    "Set Brush Angle", NULL, NULL,
    PICMAN_ACTION_SELECT_SET, TRUE,
    NULL },
  { "context-brush-angle-minimum", PICMAN_STOCK_BRUSH,
    "Horizontal", NULL, NULL,
    PICMAN_ACTION_SELECT_FIRST, FALSE,
    NULL },
  { "context-brush-angle-maximum", PICMAN_STOCK_BRUSH,
    "Vertical", NULL, NULL,
    PICMAN_ACTION_SELECT_LAST, FALSE,
    NULL },
  { "context-brush-angle-decrease", PICMAN_STOCK_BRUSH,
    "Rotate Right", NULL, NULL,
    PICMAN_ACTION_SELECT_PREVIOUS, FALSE,
    NULL },
  { "context-brush-angle-increase", PICMAN_STOCK_BRUSH,
    "Rotate Left", NULL, NULL,
    PICMAN_ACTION_SELECT_NEXT, FALSE,
    NULL },
  { "context-brush-angle-decrease-skip", PICMAN_STOCK_BRUSH,
    "Rotate Right 15°", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_PREVIOUS, FALSE,
    NULL },
  { "context-brush-angle-increase-skip", PICMAN_STOCK_BRUSH,
    "Rotate Left 15°", NULL, NULL,
    PICMAN_ACTION_SELECT_SKIP_NEXT, FALSE,
    NULL }
};


void
context_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "context-action",
                                 context_actions,
                                 G_N_ELEMENTS (context_actions));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_palette_foreground_actions,
                                      G_N_ELEMENTS (context_palette_foreground_actions),
                                      G_CALLBACK (context_palette_foreground_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_palette_background_actions,
                                      G_N_ELEMENTS (context_palette_background_actions),
                                      G_CALLBACK (context_palette_background_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_colormap_foreground_actions,
                                      G_N_ELEMENTS (context_colormap_foreground_actions),
                                      G_CALLBACK (context_colormap_foreground_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_colormap_background_actions,
                                      G_N_ELEMENTS (context_colormap_background_actions),
                                      G_CALLBACK (context_colormap_background_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_swatch_foreground_actions,
                                      G_N_ELEMENTS (context_swatch_foreground_actions),
                                      G_CALLBACK (context_swatch_foreground_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_swatch_background_actions,
                                      G_N_ELEMENTS (context_swatch_background_actions),
                                      G_CALLBACK (context_swatch_background_cmd_callback));


  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_red_actions,
                                      G_N_ELEMENTS (context_foreground_red_actions),
                                      G_CALLBACK (context_foreground_red_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_green_actions,
                                      G_N_ELEMENTS (context_foreground_green_actions),
                                      G_CALLBACK (context_foreground_green_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_blue_actions,
                                      G_N_ELEMENTS (context_foreground_blue_actions),
                                      G_CALLBACK (context_foreground_blue_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_hue_actions,
                                      G_N_ELEMENTS (context_foreground_hue_actions),
                                      G_CALLBACK (context_foreground_hue_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_saturation_actions,
                                      G_N_ELEMENTS (context_foreground_saturation_actions),
                                      G_CALLBACK (context_foreground_saturation_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_foreground_value_actions,
                                      G_N_ELEMENTS (context_foreground_value_actions),
                                      G_CALLBACK (context_foreground_value_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_red_actions,
                                      G_N_ELEMENTS (context_background_red_actions),
                                      G_CALLBACK (context_background_red_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_green_actions,
                                      G_N_ELEMENTS (context_background_green_actions),
                                      G_CALLBACK (context_background_green_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_blue_actions,
                                      G_N_ELEMENTS (context_background_blue_actions),
                                      G_CALLBACK (context_background_blue_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_hue_actions,
                                      G_N_ELEMENTS (context_background_hue_actions),
                                      G_CALLBACK (context_background_hue_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_saturation_actions,
                                      G_N_ELEMENTS (context_background_saturation_actions),
                                      G_CALLBACK (context_background_saturation_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_background_value_actions,
                                      G_N_ELEMENTS (context_background_value_actions),
                                      G_CALLBACK (context_background_value_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_opacity_actions,
                                      G_N_ELEMENTS (context_opacity_actions),
                                      G_CALLBACK (context_opacity_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_paint_mode_actions,
                                      G_N_ELEMENTS (context_paint_mode_actions),
                                      G_CALLBACK (context_paint_mode_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_tool_select_actions,
                                      G_N_ELEMENTS (context_tool_select_actions),
                                      G_CALLBACK (context_tool_select_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_select_actions,
                                      G_N_ELEMENTS (context_brush_select_actions),
                                      G_CALLBACK (context_brush_select_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_pattern_select_actions,
                                      G_N_ELEMENTS (context_pattern_select_actions),
                                      G_CALLBACK (context_pattern_select_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_palette_select_actions,
                                      G_N_ELEMENTS (context_palette_select_actions),
                                      G_CALLBACK (context_palette_select_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_gradient_select_actions,
                                      G_N_ELEMENTS (context_gradient_select_actions),
                                      G_CALLBACK (context_gradient_select_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_font_select_actions,
                                      G_N_ELEMENTS (context_font_select_actions),
                                      G_CALLBACK (context_font_select_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_spacing_actions,
                                      G_N_ELEMENTS (context_brush_spacing_actions),
                                      G_CALLBACK (context_brush_spacing_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_shape_actions,
                                      G_N_ELEMENTS (context_brush_shape_actions),
                                      G_CALLBACK (context_brush_shape_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_radius_actions,
                                      G_N_ELEMENTS (context_brush_radius_actions),
                                      G_CALLBACK (context_brush_radius_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_spikes_actions,
                                      G_N_ELEMENTS (context_brush_spikes_actions),
                                      G_CALLBACK (context_brush_spikes_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_hardness_actions,
                                      G_N_ELEMENTS (context_brush_hardness_actions),
                                      G_CALLBACK (context_brush_hardness_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_aspect_actions,
                                      G_N_ELEMENTS (context_brush_aspect_actions),
                                      G_CALLBACK (context_brush_aspect_cmd_callback));
  picman_action_group_add_enum_actions (group, NULL,
                                      context_brush_angle_actions,
                                      G_N_ELEMENTS (context_brush_angle_actions),
                                      G_CALLBACK (context_brush_angle_cmd_callback));
}

void
context_actions_update (PicmanActionGroup *group,
                        gpointer         data)
{
  PicmanContext *context   = action_data_get_context (data);
  gboolean     generated = FALSE;
  gdouble      radius    = 0.0;
  gint         spikes    = 0;
  gdouble      hardness  = 0.0;
  gdouble      aspect    = 0.0;
  gdouble      angle     = 0.0;

  if (context)
    {
      PicmanBrush *brush = picman_context_get_brush (context);

      if (PICMAN_IS_BRUSH_GENERATED (brush))
        {
          PicmanBrushGenerated *gen = PICMAN_BRUSH_GENERATED (brush);

          generated = TRUE;

          radius   = picman_brush_generated_get_radius       (gen);
          spikes   = picman_brush_generated_get_spikes       (gen);
          hardness = picman_brush_generated_get_hardness     (gen);
          aspect   = picman_brush_generated_get_aspect_ratio (gen);
          angle    = picman_brush_generated_get_angle        (gen);
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, "context-" action, (condition) != 0)

#if 0
  SET_SENSITIVE ("brush-radius-minimum",       generated && radius > 1.0);
  SET_SENSITIVE ("brush-radius-decrease",      generated && radius > 1.0);
  SET_SENSITIVE ("brush-radius-decrease-skip", generated && radius > 1.0);

  SET_SENSITIVE ("brush-radius-maximum",       generated && radius < 4000.0);
  SET_SENSITIVE ("brush-radius-increase",      generated && radius < 4000.0);
  SET_SENSITIVE ("brush-radius-increase-skip", generated && radius < 4000.0);

  SET_SENSITIVE ("brush-angle-minimum",       generated);
  SET_SENSITIVE ("brush-angle-decrease",      generated);
  SET_SENSITIVE ("brush-angle-decrease-skip", generated);

  SET_SENSITIVE ("brush-angle-maximum",       generated);
  SET_SENSITIVE ("brush-angle-increase",      generated);
  SET_SENSITIVE ("brush-angle-increase-skip", generated);
#endif

#undef SET_SENSITIVE
}
