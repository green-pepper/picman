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

#ifndef __CONFIG_ENUMS_H__
#define __CONFIG_ENUMS_H__


#define PICMAN_TYPE_CURSOR_MODE (picman_cursor_mode_get_type ())

GType picman_cursor_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CURSOR_MODE_TOOL_ICON,       /*< desc="Tool icon"                >*/
  PICMAN_CURSOR_MODE_TOOL_CROSSHAIR,  /*< desc="Tool icon with crosshair" >*/
  PICMAN_CURSOR_MODE_CROSSHAIR        /*< desc="Crosshair only"           >*/
} PicmanCursorMode;


#define PICMAN_TYPE_CANVAS_PADDING_MODE (picman_canvas_padding_mode_get_type ())

GType picman_canvas_padding_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CANVAS_PADDING_MODE_DEFAULT,      /*< desc="From theme"        >*/
  PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK,  /*< desc="Light check color" >*/
  PICMAN_CANVAS_PADDING_MODE_DARK_CHECK,   /*< desc="Dark check color"  >*/
  PICMAN_CANVAS_PADDING_MODE_CUSTOM,       /*< desc="Custom color"      >*/
  PICMAN_CANVAS_PADDING_MODE_RESET = -1    /*< skip >*/
} PicmanCanvasPaddingMode;


#define PICMAN_TYPE_SPACE_BAR_ACTION (picman_space_bar_action_get_type ())

GType picman_space_bar_action_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_SPACE_BAR_ACTION_NONE,  /*< desc="No action"           >*/
  PICMAN_SPACE_BAR_ACTION_PAN,   /*< desc="Pan view"            >*/
  PICMAN_SPACE_BAR_ACTION_MOVE   /*< desc="Switch to Move tool" >*/
} PicmanSpaceBarAction;


#define PICMAN_TYPE_ZOOM_QUALITY (picman_zoom_quality_get_type ())

GType picman_zoom_quality_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ZOOM_QUALITY_LOW,   /*< desc="Low"  >*/
  PICMAN_ZOOM_QUALITY_HIGH   /*< desc="High" >*/
} PicmanZoomQuality;


#define PICMAN_TYPE_HELP_BROWSER_TYPE (picman_help_browser_type_get_type ())

GType picman_help_browser_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HELP_BROWSER_PICMAN,        /*< desc="PICMAN help browser" >*/
  PICMAN_HELP_BROWSER_WEB_BROWSER  /*< desc="Web browser"       >*/
} PicmanHelpBrowserType;


#define PICMAN_TYPE_WINDOW_HINT (picman_window_hint_get_type ())

GType picman_window_hint_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_WINDOW_HINT_NORMAL,     /*< desc="Normal window"  >*/
  PICMAN_WINDOW_HINT_UTILITY,    /*< desc="Utility window" >*/
  PICMAN_WINDOW_HINT_KEEP_ABOVE  /*< desc="Keep above"     >*/
} PicmanWindowHint;


#define PICMAN_TYPE_CURSOR_FORMAT (picman_cursor_format_get_type ())

GType picman_cursor_format_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CURSOR_FORMAT_BITMAP, /*< desc="Black & white" >*/
  PICMAN_CURSOR_FORMAT_PIXBUF  /*< desc="Fancy"         >*/
} PicmanCursorFormat;


#define PICMAN_TYPE_HANDEDNESS (picman_handedness_get_type ())

GType picman_handedness_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HANDEDNESS_LEFT, /*< desc="Left-handed"  >*/
  PICMAN_HANDEDNESS_RIGHT /*< desc="Right-handed" >*/
} PicmanHandedness;


#endif /* __CONFIG_ENUMS_H__ */
