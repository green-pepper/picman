/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanstock.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_STOCK_H__
#define __PICMAN_STOCK_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*  in button size:  */

#define PICMAN_STOCK_ANCHOR                   "picman-anchor"
#define PICMAN_STOCK_CENTER                   "picman-center"
#define PICMAN_STOCK_DUPLICATE                "picman-duplicate"
#define PICMAN_STOCK_EDIT                     "picman-edit"
#define PICMAN_STOCK_LINKED                   "picman-linked"
#define PICMAN_STOCK_PASTE_AS_NEW             "picman-paste-as-new"
#define PICMAN_STOCK_PASTE_INTO               "picman-paste-into"
#define PICMAN_STOCK_RESET                    "picman-reset"
#define PICMAN_STOCK_VISIBLE                  "picman-visible"

#define PICMAN_STOCK_GRADIENT_LINEAR               "picman-gradient-linear"
#define PICMAN_STOCK_GRADIENT_BILINEAR             "picman-gradient-bilinear"
#define PICMAN_STOCK_GRADIENT_RADIAL               "picman-gradient-radial"
#define PICMAN_STOCK_GRADIENT_SQUARE               "picman-gradient-square"
#define PICMAN_STOCK_GRADIENT_CONICAL_SYMMETRIC    "picman-gradient-conical-symmetric"
#define PICMAN_STOCK_GRADIENT_CONICAL_ASYMMETRIC   "picman-gradient-conical-asymmetric"
#define PICMAN_STOCK_GRADIENT_SHAPEBURST_ANGULAR   "picman-gradient-shapeburst-angular"
#define PICMAN_STOCK_GRADIENT_SHAPEBURST_SPHERICAL "picman-gradient-shapeburst-spherical"
#define PICMAN_STOCK_GRADIENT_SHAPEBURST_DIMPLED   "picman-gradient-shapeburst-dimpled"
#define PICMAN_STOCK_GRADIENT_SPIRAL_CLOCKWISE     "picman-gradient-spiral-clockwise"
#define PICMAN_STOCK_GRADIENT_SPIRAL_ANTICLOCKWISE "picman-gradient-spiral-anticlockwise"

#define PICMAN_STOCK_GRAVITY_EAST             "picman-gravity-east"
#define PICMAN_STOCK_GRAVITY_NORTH            "picman-gravity-north"
#define PICMAN_STOCK_GRAVITY_NORTH_EAST       "picman-gravity-north-east"
#define PICMAN_STOCK_GRAVITY_NORTH_WEST       "picman-gravity-north-west"
#define PICMAN_STOCK_GRAVITY_SOUTH            "picman-gravity-south"
#define PICMAN_STOCK_GRAVITY_SOUTH_EAST       "picman-gravity-south-east"
#define PICMAN_STOCK_GRAVITY_SOUTH_WEST       "picman-gravity-south-west"
#define PICMAN_STOCK_GRAVITY_WEST             "picman-gravity-west"

#define PICMAN_STOCK_HCENTER                  "picman-hcenter"
#define PICMAN_STOCK_VCENTER                  "picman-vcenter"

#define PICMAN_STOCK_HCHAIN                   "picman-hchain"
#define PICMAN_STOCK_HCHAIN_BROKEN            "picman-hchain-broken"
#define PICMAN_STOCK_VCHAIN                   "picman-vchain"
#define PICMAN_STOCK_VCHAIN_BROKEN            "picman-vchain-broken"

#define PICMAN_STOCK_SELECTION                "picman-selection"
#define PICMAN_STOCK_SELECTION_REPLACE        "picman-selection-replace"
#define PICMAN_STOCK_SELECTION_ADD            "picman-selection-add"
#define PICMAN_STOCK_SELECTION_SUBTRACT       "picman-selection-subtract"
#define PICMAN_STOCK_SELECTION_INTERSECT      "picman-selection-intersect"
#define PICMAN_STOCK_SELECTION_STROKE         "picman-selection-stroke"
#define PICMAN_STOCK_SELECTION_TO_CHANNEL     "picman-selection-to-channel"
#define PICMAN_STOCK_SELECTION_TO_PATH        "picman-selection-to-path"

#define PICMAN_STOCK_PATH_STROKE              "picman-path-stroke"

#define PICMAN_STOCK_CURVE_FREE               "picman-curve-free"
#define PICMAN_STOCK_CURVE_SMOOTH             "picman-curve-smooth"

#define PICMAN_STOCK_COLOR_PICKER_BLACK       "picman-color-picker-black"
#define PICMAN_STOCK_COLOR_PICKER_GRAY        "picman-color-picker-gray"
#define PICMAN_STOCK_COLOR_PICKER_WHITE       "picman-color-picker-white"
#define PICMAN_STOCK_COLOR_TRIANGLE           "picman-color-triangle"
#define PICMAN_STOCK_COLOR_PICK_FROM_SCREEN   "picman-color-pick-from-screen"

#define PICMAN_STOCK_CHAR_PICKER              "picman-char-picker"
#define PICMAN_STOCK_LETTER_SPACING           "picman-letter-spacing"
#define PICMAN_STOCK_LINE_SPACING             "picman-line-spacing"
#define PICMAN_STOCK_PRINT_RESOLUTION         "picman-print-resolution"

#define PICMAN_STOCK_TEXT_DIR_LTR             "picman-text-dir-ltr"
#define PICMAN_STOCK_TEXT_DIR_RTL             "picman-text-dir-rtl"

#define PICMAN_STOCK_TOOL_AIRBRUSH            "picman-tool-airbrush"
#define PICMAN_STOCK_TOOL_ALIGN               "picman-tool-align"
#define PICMAN_STOCK_TOOL_BLEND               "picman-tool-blend"
#define PICMAN_STOCK_TOOL_BLUR                "picman-tool-blur"
#define PICMAN_STOCK_TOOL_BRIGHTNESS_CONTRAST "picman-tool-brightness-contrast"
#define PICMAN_STOCK_TOOL_BUCKET_FILL         "picman-tool-bucket-fill"
#define PICMAN_STOCK_TOOL_BY_COLOR_SELECT     "picman-tool-by-color-select"
#define PICMAN_STOCK_TOOL_CAGE                "picman-tool-cage"
#define PICMAN_STOCK_TOOL_CLONE               "picman-tool-clone"
#define PICMAN_STOCK_TOOL_COLOR_BALANCE       "picman-tool-color-balance"
#define PICMAN_STOCK_TOOL_COLOR_PICKER        "picman-tool-color-picker"
#define PICMAN_STOCK_TOOL_COLORIZE            "picman-tool-colorize"
#define PICMAN_STOCK_TOOL_CROP                "picman-tool-crop"
#define PICMAN_STOCK_TOOL_CURVES              "picman-tool-curves"
#define PICMAN_STOCK_TOOL_DESATURATE          "picman-tool-desaturate"
#define PICMAN_STOCK_TOOL_DODGE               "picman-tool-dodge"
#define PICMAN_STOCK_TOOL_ELLIPSE_SELECT      "picman-tool-ellipse-select"
#define PICMAN_STOCK_TOOL_ERASER              "picman-tool-eraser"
#define PICMAN_STOCK_TOOL_FLIP                "picman-tool-flip"
#define PICMAN_STOCK_TOOL_FREE_SELECT         "picman-tool-free-select"
#define PICMAN_STOCK_TOOL_FOREGROUND_SELECT   "picman-tool-foreground-select"
#define PICMAN_STOCK_TOOL_FUZZY_SELECT        "picman-tool-fuzzy-select"
#define PICMAN_STOCK_TOOL_HEAL                "picman-tool-heal"
#define PICMAN_STOCK_TOOL_HUE_SATURATION      "picman-tool-hue-saturation"
#define PICMAN_STOCK_TOOL_INK                 "picman-tool-ink"
#define PICMAN_STOCK_TOOL_ISCISSORS           "picman-tool-iscissors"
#define PICMAN_STOCK_TOOL_LEVELS              "picman-tool-levels"
#define PICMAN_STOCK_TOOL_MEASURE             "picman-tool-measure"
#define PICMAN_STOCK_TOOL_MOVE                "picman-tool-move"
#define PICMAN_STOCK_TOOL_PAINTBRUSH          "picman-tool-paintbrush"
#define PICMAN_STOCK_TOOL_PATH                "picman-tool-path"
#define PICMAN_STOCK_TOOL_PENCIL              "picman-tool-pencil"
#define PICMAN_STOCK_TOOL_PERSPECTIVE         "picman-tool-perspective"
#define PICMAN_STOCK_TOOL_PERSPECTIVE_CLONE   "picman-tool-perspective-clone"
#define PICMAN_STOCK_TOOL_POSTERIZE           "picman-tool-posterize"
#define PICMAN_STOCK_TOOL_RECT_SELECT         "picman-tool-rect-select"
#define PICMAN_STOCK_TOOL_ROTATE              "picman-tool-rotate"
#define PICMAN_STOCK_TOOL_SCALE               "picman-tool-scale"
#define PICMAN_STOCK_TOOL_SHEAR               "picman-tool-shear"
#define PICMAN_STOCK_TOOL_SMUDGE              "picman-tool-smudge"
#define PICMAN_STOCK_TOOL_TEXT                "picman-tool-text"
#define PICMAN_STOCK_TOOL_THRESHOLD           "picman-tool-threshold"
#define PICMAN_STOCK_TOOL_UNIFIED_TRANSFORM   "picman-tool-unified-transform"
#define PICMAN_STOCK_TOOL_ZOOM                "picman-tool-zoom"


/*  in menu size:  */

#define PICMAN_STOCK_CONVERT_RGB              "picman-convert-rgb"
#define PICMAN_STOCK_CONVERT_GRAYSCALE        "picman-convert-grayscale"
#define PICMAN_STOCK_CONVERT_INDEXED          "picman-convert-indexed"
#define PICMAN_STOCK_INVERT                   "picman-invert"
#define PICMAN_STOCK_MERGE_DOWN               "picman-merge-down"
#define PICMAN_STOCK_LAYER_TO_IMAGESIZE       "picman-layer-to-imagesize"
#define PICMAN_STOCK_PLUGIN                   "picman-plugin"
#define PICMAN_STOCK_UNDO_HISTORY             "picman-undo-history"
#define PICMAN_STOCK_RESHOW_FILTER            "picman-reshow-filter"
#define PICMAN_STOCK_ROTATE_90                "picman-rotate-90"
#define PICMAN_STOCK_ROTATE_180               "picman-rotate-180"
#define PICMAN_STOCK_ROTATE_270               "picman-rotate-270"
#define PICMAN_STOCK_RESIZE                   "picman-resize"
#define PICMAN_STOCK_SCALE                    "picman-scale"
#define PICMAN_STOCK_FLIP_HORIZONTAL          "picman-flip-horizontal"
#define PICMAN_STOCK_FLIP_VERTICAL            "picman-flip-vertical"

#define PICMAN_STOCK_IMAGE                    "picman-image"
#define PICMAN_STOCK_LAYER                    "picman-layer"
#define PICMAN_STOCK_TEXT_LAYER               "picman-text-layer"
#define PICMAN_STOCK_FLOATING_SELECTION       "picman-floating-selection"
#define PICMAN_STOCK_CHANNEL                  "picman-channel"
#define PICMAN_STOCK_CHANNEL_RED              "picman-channel-red"
#define PICMAN_STOCK_CHANNEL_GREEN            "picman-channel-green"
#define PICMAN_STOCK_CHANNEL_BLUE             "picman-channel-blue"
#define PICMAN_STOCK_CHANNEL_GRAY             "picman-channel-gray"
#define PICMAN_STOCK_CHANNEL_INDEXED          "picman-channel-indexed"
#define PICMAN_STOCK_CHANNEL_ALPHA            "picman-channel-alpha"
#define PICMAN_STOCK_LAYER_MASK               "picman-layer-mask"
#define PICMAN_STOCK_PATH                     "picman-path"
#define PICMAN_STOCK_TEMPLATE                 "picman-template"
#define PICMAN_STOCK_TRANSPARENCY             "picman-transparency"
#define PICMAN_STOCK_COLORMAP                 "picman-colormap"

#ifndef PICMAN_DISABLE_DEPRECATED
#define PICMAN_STOCK_INDEXED_PALETTE          "picman-colormap"
#endif /* PICMAN_DISABLE_DEPRECATED */

#define PICMAN_STOCK_IMAGES                   "picman-images"
#define PICMAN_STOCK_LAYERS                   "picman-layers"
#define PICMAN_STOCK_CHANNELS                 "picman-channels"
#define PICMAN_STOCK_PATHS                    "picman-paths"

#define PICMAN_STOCK_SELECTION_ALL            "picman-selection-all"
#define PICMAN_STOCK_SELECTION_NONE           "picman-selection-none"
#define PICMAN_STOCK_SELECTION_GROW           "picman-selection-grow"
#define PICMAN_STOCK_SELECTION_SHRINK         "picman-selection-shrink"
#define PICMAN_STOCK_SELECTION_BORDER         "picman-selection-border"

#define PICMAN_STOCK_NAVIGATION               "picman-navigation"
#define PICMAN_STOCK_QUICK_MASK_OFF           "picman-quick-mask-off"
#define PICMAN_STOCK_QUICK_MASK_ON            "picman-quick-mask-on"

#ifndef PICMAN_DISABLE_DEPRECATED
#define PICMAN_STOCK_QMASK_OFF                "picman-quick-mask-off"
#define PICMAN_STOCK_QMASK_ON                 "picman-quick-mask-on"
#endif /* PICMAN_DISABLE_DEPRECATED */

#define PICMAN_STOCK_HISTOGRAM                "picman-histogram"
#define PICMAN_STOCK_HISTOGRAM_LINEAR         "picman-histogram-linear"
#define PICMAN_STOCK_HISTOGRAM_LOGARITHMIC    "picman-histogram-logarithmic"

#define PICMAN_STOCK_CLOSE                    "picman-close"
#define PICMAN_STOCK_MENU_LEFT                "picman-menu-left"
#define PICMAN_STOCK_MENU_RIGHT               "picman-menu-right"
#define PICMAN_STOCK_MOVE_TO_SCREEN           "picman-move-to-screen"
#define PICMAN_STOCK_DEFAULT_COLORS           "picman-default-colors"
#define PICMAN_STOCK_SWAP_COLORS              "picman-swap-colors"
#define PICMAN_STOCK_ZOOM_FOLLOW_WINDOW       "picman-zoom-follow-window"

#define PICMAN_STOCK_TOOLS                    "picman-tools"
#define PICMAN_STOCK_TOOL_OPTIONS             "picman-tool-options"
#define PICMAN_STOCK_DEVICE_STATUS            "picman-device-status"
#define PICMAN_STOCK_INPUT_DEVICE             "picman-input-device"
#define PICMAN_STOCK_CURSOR                   "picman-cursor"
#define PICMAN_STOCK_SAMPLE_POINT             "picman-sample-point"
#define PICMAN_STOCK_DYNAMICS                 "picman-dynamics"
#define PICMAN_STOCK_TOOL_PRESET              "picman-tool-preset"

#define PICMAN_STOCK_CONTROLLER               "picman-controller"
#define PICMAN_STOCK_CONTROLLER_KEYBOARD      "picman-controller-keyboard"
#define PICMAN_STOCK_CONTROLLER_LINUX_INPUT   "picman-controller-linux-input"
#define PICMAN_STOCK_CONTROLLER_MIDI          "picman-controller-midi"
#define PICMAN_STOCK_CONTROLLER_WHEEL         "picman-controller-wheel"

#define PICMAN_STOCK_DISPLAY_FILTER           "picman-display-filter"
#define PICMAN_STOCK_DISPLAY_FILTER_COLORBLIND "picman-display-filter-colorblind"
#define PICMAN_STOCK_DISPLAY_FILTER_CONTRAST  "picman-display-filter-contrast"
#define PICMAN_STOCK_DISPLAY_FILTER_GAMMA     "picman-display-filter-gamma"
#define PICMAN_STOCK_DISPLAY_FILTER_LCMS      "picman-display-filter-lcms"
#define PICMAN_STOCK_DISPLAY_FILTER_PROOF     "picman-display-filter-proof"

#define PICMAN_STOCK_LIST                     "picman-list"
#define PICMAN_STOCK_GRID                     "picman-grid"

#define PICMAN_STOCK_PORTRAIT                 "picman-portrait"
#define PICMAN_STOCK_LANDSCAPE                "picman-landscape"

#define PICMAN_STOCK_WEB                      "picman-web"
#define PICMAN_STOCK_VIDEO                    "picman-video"
#define PICMAN_STOCK_GEGL                     "picman-gegl"

#define PICMAN_STOCK_SHAPE_CIRCLE             "picman-shape-circle"
#define PICMAN_STOCK_SHAPE_DIAMOND            "picman-shape-diamond"
#define PICMAN_STOCK_SHAPE_SQUARE             "picman-shape-square"

#define PICMAN_STOCK_CAP_BUTT                 "picman-cap-butt"
#define PICMAN_STOCK_CAP_ROUND                "picman-cap-round"
#define PICMAN_STOCK_CAP_SQUARE               "picman-cap-square"

#define PICMAN_STOCK_JOIN_MITER               "picman-join-miter"
#define PICMAN_STOCK_JOIN_ROUND               "picman-join-round"
#define PICMAN_STOCK_JOIN_BEVEL               "picman-join-bevel"

/*  in dialog size:  */

#define PICMAN_STOCK_ERROR                    "picman-error"
#define PICMAN_STOCK_INFO                     "picman-info"
#define PICMAN_STOCK_QUESTION                 "picman-question"
#define PICMAN_STOCK_WARNING                  "picman-warning"
#define PICMAN_STOCK_WILBER                   "picman-wilber"
#define PICMAN_STOCK_WILBER_EEK               "picman-wilber-eek"
#define PICMAN_STOCK_FRAME                    "picman-frame"
#define PICMAN_STOCK_TEXTURE                  "picman-texture"
#define PICMAN_STOCK_USER_MANUAL              "picman-user-manual"


/*  missing icons:  */

#define PICMAN_STOCK_BRUSH                    PICMAN_STOCK_TOOL_PAINTBRUSH
#define PICMAN_STOCK_BUFFER                   GTK_STOCK_PASTE
#define PICMAN_STOCK_DETACH                   GTK_STOCK_CONVERT
#define PICMAN_STOCK_FONT                     GTK_STOCK_SELECT_FONT
#define PICMAN_STOCK_GRADIENT                 PICMAN_STOCK_TOOL_BLEND
#define PICMAN_STOCK_PALETTE                  GTK_STOCK_SELECT_COLOR
#define PICMAN_STOCK_PATTERN                  PICMAN_STOCK_TOOL_BUCKET_FILL
#define PICMAN_STOCK_CONTROLLER_MOUSE         PICMAN_STOCK_CURSOR
#define PICMAN_STOCK_CONVERT_PRECISION        PICMAN_STOCK_CONVERT_RGB


void   picman_stock_init (void);


G_END_DECLS

#endif /* __PICMAN_STOCK_H__ */
