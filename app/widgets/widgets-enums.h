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

#ifndef __WIDGETS_ENUMS_H__
#define __WIDGETS_ENUMS_H__


/*
 * enums that are registered with the type system
 */

#define PICMAN_TYPE_ACTIVE_COLOR (picman_active_color_get_type ())

GType picman_active_color_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ACTIVE_COLOR_FOREGROUND, /*< desc="Foreground" >*/
  PICMAN_ACTIVE_COLOR_BACKGROUND  /*< desc="Background" >*/
} PicmanActiveColor;


#define PICMAN_TYPE_COLOR_DIALOG_STATE (picman_color_dialog_state_get_type ())

GType picman_color_dialog_state_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_DIALOG_OK,
  PICMAN_COLOR_DIALOG_CANCEL,
  PICMAN_COLOR_DIALOG_UPDATE
} PicmanColorDialogState;


#define PICMAN_TYPE_COLOR_FRAME_MODE (picman_color_frame_mode_get_type ())

GType picman_color_frame_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_FRAME_MODE_PIXEL,  /*< desc="Pixel" >*/
  PICMAN_COLOR_FRAME_MODE_RGB,    /*< desc="RGB"   >*/
  PICMAN_COLOR_FRAME_MODE_HSV,    /*< desc="HSV"   >*/
  PICMAN_COLOR_FRAME_MODE_CMYK    /*< desc="CMYK"  >*/
} PicmanColorFrameMode;


#define PICMAN_TYPE_COLOR_PICK_MODE (picman_color_pick_mode_get_type ())

GType picman_color_pick_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_PICK_MODE_NONE,       /*< desc="Pick only"            >*/
  PICMAN_COLOR_PICK_MODE_FOREGROUND, /*< desc="Set foreground color" >*/
  PICMAN_COLOR_PICK_MODE_BACKGROUND, /*< desc="Set background color" >*/
  PICMAN_COLOR_PICK_MODE_PALETTE     /*< desc="Add to palette"       >*/
} PicmanColorPickMode;


#define PICMAN_TYPE_COLOR_PICK_STATE (picman_color_pick_state_get_type ())

GType picman_color_pick_state_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_PICK_STATE_NEW,
  PICMAN_COLOR_PICK_STATE_UPDATE
} PicmanColorPickState;


#define PICMAN_TYPE_HISTOGRAM_SCALE (picman_histogram_scale_get_type ())

GType picman_histogram_scale_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HISTOGRAM_SCALE_LINEAR,       /*< desc="Linear histogram"      >*/
  PICMAN_HISTOGRAM_SCALE_LOGARITHMIC   /*< desc="Logarithmic histogram" >*/
} PicmanHistogramScale;


#define PICMAN_TYPE_TAB_STYLE (picman_tab_style_get_type ())

GType picman_tab_style_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TAB_STYLE_ICON,          /*< desc="Icon"           >*/
  PICMAN_TAB_STYLE_PREVIEW,       /*< desc="Current status" >*/
  PICMAN_TAB_STYLE_NAME,          /*< desc="Text"           >*/
  PICMAN_TAB_STYLE_BLURB,         /*< desc="Description"    >*/
  PICMAN_TAB_STYLE_ICON_NAME,     /*< desc="Icon & text"    >*/
  PICMAN_TAB_STYLE_ICON_BLURB,    /*< desc="Icon & desc"    >*/
  PICMAN_TAB_STYLE_PREVIEW_NAME,  /*< desc="Status & text"  >*/
  PICMAN_TAB_STYLE_PREVIEW_BLURB, /*< desc="Status & desc"  >*/
  PICMAN_TAB_STYLE_UNDEFINED,     /*< desc="Undefined"      >*/
  PICMAN_TAB_STYLE_AUTOMATIC      /*< desc="Automatic"      >*/
} PicmanTabStyle;


#define PICMAN_TYPE_TAG_ENTRY_MODE       (picman_tag_entry_mode_get_type ())

GType picman_tag_entry_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TAG_ENTRY_MODE_QUERY,
  PICMAN_TAG_ENTRY_MODE_ASSIGN,
} PicmanTagEntryMode;


/*
 * non-registered enums; register them if needed
 */

typedef enum  /*< skip >*/
{
  PICMAN_VIEW_BG_CHECKS,
  PICMAN_VIEW_BG_WHITE
} PicmanViewBG;

typedef enum  /*< skip >*/
{
  PICMAN_VIEW_BORDER_BLACK,
  PICMAN_VIEW_BORDER_WHITE,
  PICMAN_VIEW_BORDER_RED,
  PICMAN_VIEW_BORDER_GREEN
} PicmanViewBorderType;

typedef enum  /*< skip >*/
{
  PICMAN_DND_TYPE_NONE         = 0,
  PICMAN_DND_TYPE_URI_LIST     = 1,
  PICMAN_DND_TYPE_TEXT_PLAIN   = 2,
  PICMAN_DND_TYPE_NETSCAPE_URL = 3,
  PICMAN_DND_TYPE_XDS          = 4,
  PICMAN_DND_TYPE_COLOR        = 5,
  PICMAN_DND_TYPE_SVG          = 6,
  PICMAN_DND_TYPE_SVG_XML      = 7,
  PICMAN_DND_TYPE_PIXBUF       = 8,
  PICMAN_DND_TYPE_IMAGE        = 9,
  PICMAN_DND_TYPE_COMPONENT    = 10,
  PICMAN_DND_TYPE_LAYER        = 11,
  PICMAN_DND_TYPE_CHANNEL      = 12,
  PICMAN_DND_TYPE_LAYER_MASK   = 13,
  PICMAN_DND_TYPE_VECTORS      = 14,
  PICMAN_DND_TYPE_BRUSH        = 15,
  PICMAN_DND_TYPE_PATTERN      = 16,
  PICMAN_DND_TYPE_GRADIENT     = 17,
  PICMAN_DND_TYPE_PALETTE      = 18,
  PICMAN_DND_TYPE_FONT         = 19,
  PICMAN_DND_TYPE_BUFFER       = 20,
  PICMAN_DND_TYPE_IMAGEFILE    = 21,
  PICMAN_DND_TYPE_TEMPLATE     = 22,
  PICMAN_DND_TYPE_TOOL_INFO    = 23,
  PICMAN_DND_TYPE_DIALOG       = 24,

  PICMAN_DND_TYPE_LAST         = PICMAN_DND_TYPE_DIALOG
} PicmanDndType;

typedef enum  /*< skip >*/
{
  PICMAN_DROP_NONE,
  PICMAN_DROP_ABOVE,
  PICMAN_DROP_BELOW
} PicmanDropType;

typedef enum  /*< skip >*/
{
  PICMAN_CURSOR_NONE = 1024,  /* (GDK_LAST_CURSOR + 2) yes, this is insane */
  PICMAN_CURSOR_MOUSE,
  PICMAN_CURSOR_CROSSHAIR,
  PICMAN_CURSOR_CROSSHAIR_SMALL,
  PICMAN_CURSOR_BAD,
  PICMAN_CURSOR_MOVE,
  PICMAN_CURSOR_ZOOM,
  PICMAN_CURSOR_COLOR_PICKER,
  PICMAN_CURSOR_CORNER_TOP,
  PICMAN_CURSOR_CORNER_TOP_RIGHT,
  PICMAN_CURSOR_CORNER_RIGHT,
  PICMAN_CURSOR_CORNER_BOTTOM_RIGHT,
  PICMAN_CURSOR_CORNER_BOTTOM,
  PICMAN_CURSOR_CORNER_BOTTOM_LEFT,
  PICMAN_CURSOR_CORNER_LEFT,
  PICMAN_CURSOR_CORNER_TOP_LEFT,
  PICMAN_CURSOR_SIDE_TOP,
  PICMAN_CURSOR_SIDE_TOP_RIGHT,
  PICMAN_CURSOR_SIDE_RIGHT,
  PICMAN_CURSOR_SIDE_BOTTOM_RIGHT,
  PICMAN_CURSOR_SIDE_BOTTOM,
  PICMAN_CURSOR_SIDE_BOTTOM_LEFT,
  PICMAN_CURSOR_SIDE_LEFT,
  PICMAN_CURSOR_SIDE_TOP_LEFT,
  PICMAN_CURSOR_LAST
} PicmanCursorType;

typedef enum  /*< skip >*/
{
  PICMAN_TOOL_CURSOR_NONE,
  PICMAN_TOOL_CURSOR_RECT_SELECT,
  PICMAN_TOOL_CURSOR_ELLIPSE_SELECT,
  PICMAN_TOOL_CURSOR_FREE_SELECT,
  PICMAN_TOOL_CURSOR_POLYGON_SELECT,
  PICMAN_TOOL_CURSOR_FUZZY_SELECT,
  PICMAN_TOOL_CURSOR_PATHS,
  PICMAN_TOOL_CURSOR_PATHS_ANCHOR,
  PICMAN_TOOL_CURSOR_PATHS_CONTROL,
  PICMAN_TOOL_CURSOR_PATHS_SEGMENT,
  PICMAN_TOOL_CURSOR_ISCISSORS,
  PICMAN_TOOL_CURSOR_MOVE,
  PICMAN_TOOL_CURSOR_ZOOM,
  PICMAN_TOOL_CURSOR_CROP,
  PICMAN_TOOL_CURSOR_RESIZE,
  PICMAN_TOOL_CURSOR_ROTATE,
  PICMAN_TOOL_CURSOR_SHEAR,
  PICMAN_TOOL_CURSOR_PERSPECTIVE,
  PICMAN_TOOL_CURSOR_FLIP_HORIZONTAL,
  PICMAN_TOOL_CURSOR_FLIP_VERTICAL,
  PICMAN_TOOL_CURSOR_TEXT,
  PICMAN_TOOL_CURSOR_COLOR_PICKER,
  PICMAN_TOOL_CURSOR_BUCKET_FILL,
  PICMAN_TOOL_CURSOR_BLEND,
  PICMAN_TOOL_CURSOR_PENCIL,
  PICMAN_TOOL_CURSOR_PAINTBRUSH,
  PICMAN_TOOL_CURSOR_AIRBRUSH,
  PICMAN_TOOL_CURSOR_INK,
  PICMAN_TOOL_CURSOR_CLONE,
  PICMAN_TOOL_CURSOR_HEAL,
  PICMAN_TOOL_CURSOR_ERASER,
  PICMAN_TOOL_CURSOR_SMUDGE,
  PICMAN_TOOL_CURSOR_BLUR,
  PICMAN_TOOL_CURSOR_DODGE,
  PICMAN_TOOL_CURSOR_BURN,
  PICMAN_TOOL_CURSOR_MEASURE,
  PICMAN_TOOL_CURSOR_HAND,
  PICMAN_TOOL_CURSOR_LAST
} PicmanToolCursorType;

typedef enum  /*< skip >*/
{
  PICMAN_CURSOR_MODIFIER_NONE,
  PICMAN_CURSOR_MODIFIER_BAD,
  PICMAN_CURSOR_MODIFIER_PLUS,
  PICMAN_CURSOR_MODIFIER_MINUS,
  PICMAN_CURSOR_MODIFIER_INTERSECT,
  PICMAN_CURSOR_MODIFIER_MOVE,
  PICMAN_CURSOR_MODIFIER_RESIZE,
  PICMAN_CURSOR_MODIFIER_CONTROL,
  PICMAN_CURSOR_MODIFIER_ANCHOR,
  PICMAN_CURSOR_MODIFIER_FOREGROUND,
  PICMAN_CURSOR_MODIFIER_BACKGROUND,
  PICMAN_CURSOR_MODIFIER_PATTERN,
  PICMAN_CURSOR_MODIFIER_JOIN,
  PICMAN_CURSOR_MODIFIER_SELECT,
  PICMAN_CURSOR_MODIFIER_LAST
} PicmanCursorModifier;

typedef enum  /*< skip >*/
{
  PICMAN_DEVICE_VALUE_MODE       = 1 << 0,
  PICMAN_DEVICE_VALUE_AXES       = 1 << 1,
  PICMAN_DEVICE_VALUE_KEYS       = 1 << 2,
  PICMAN_DEVICE_VALUE_TOOL       = 1 << 3,
  PICMAN_DEVICE_VALUE_FOREGROUND = 1 << 4,
  PICMAN_DEVICE_VALUE_BACKGROUND = 1 << 5,
  PICMAN_DEVICE_VALUE_BRUSH      = 1 << 6,
  PICMAN_DEVICE_VALUE_PATTERN    = 1 << 7,
  PICMAN_DEVICE_VALUE_GRADIENT   = 1 << 8
} PicmanDeviceValues;

typedef enum  /*< skip >*/
{
  PICMAN_FILE_CHOOSER_ACTION_OPEN,
  PICMAN_FILE_CHOOSER_ACTION_SAVE,
  PICMAN_FILE_CHOOSER_ACTION_EXPORT
} PicmanFileChooserAction;

typedef enum  /*< skip >*/
{
  PICMAN_DIALOGS_SHOWN,
  PICMAN_DIALOGS_HIDDEN_EXPLICITLY,  /* user used the Tab key to hide dialogs */
  PICMAN_DIALOGS_HIDDEN_WITH_DISPLAY /* dialogs are hidden with the display   */
} PicmanDialogsState;


#endif /* __WIDGETS_ENUMS_H__ */
