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

#ifndef __DISPLAY_ENUMS_H__
#define __DISPLAY_ENUMS_H__


#define PICMAN_TYPE_CURSOR_PRECISION (picman_cursor_precision_get_type ())

GType picman_cursor_precision_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CURSOR_PRECISION_PIXEL_CENTER,
  PICMAN_CURSOR_PRECISION_PIXEL_BORDER,
  PICMAN_CURSOR_PRECISION_SUBPIXEL
} PicmanCursorPrecision;


#define PICMAN_TYPE_GUIDES_TYPE (picman_guides_type_get_type ())

GType picman_guides_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_GUIDES_NONE,          /*< desc="No guides"       >*/
  PICMAN_GUIDES_CENTER_LINES,  /*< desc="Center lines"    >*/
  PICMAN_GUIDES_THIRDS,        /*< desc="Rule of thirds"  >*/
  PICMAN_GUIDES_FIFTHS,        /*< desc="Rule of fifths"  >*/
  PICMAN_GUIDES_GOLDEN,        /*< desc="Golden sections" >*/
  PICMAN_GUIDES_DIAGONALS,     /*< desc="Diagonal lines"  >*/
  PICMAN_GUIDES_N_LINES,       /*< desc="Number of lines" >*/
  PICMAN_GUIDES_SPACING        /*< desc="Line spacing"    >*/
} PicmanGuidesType;


#define PICMAN_TYPE_HANDLE_TYPE (picman_handle_type_get_type ())

GType picman_handle_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HANDLE_SQUARE,
  PICMAN_HANDLE_FILLED_SQUARE,
  PICMAN_HANDLE_CIRCLE,
  PICMAN_HANDLE_FILLED_CIRCLE,
  PICMAN_HANDLE_DIAMOND,
  PICMAN_HANDLE_FILLED_DIAMOND,
  PICMAN_HANDLE_CROSS
} PicmanHandleType;


#define PICMAN_TYPE_HANDLE_ANCHOR (picman_handle_anchor_get_type ())

GType picman_handle_anchor_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HANDLE_ANCHOR_CENTER,
  PICMAN_HANDLE_ANCHOR_NORTH,
  PICMAN_HANDLE_ANCHOR_NORTH_WEST,
  PICMAN_HANDLE_ANCHOR_NORTH_EAST,
  PICMAN_HANDLE_ANCHOR_SOUTH,
  PICMAN_HANDLE_ANCHOR_SOUTH_WEST,
  PICMAN_HANDLE_ANCHOR_SOUTH_EAST,
  PICMAN_HANDLE_ANCHOR_WEST,
  PICMAN_HANDLE_ANCHOR_EAST
} PicmanHandleAnchor;


#define PICMAN_TYPE_PATH_STYLE (picman_path_style_get_type ())

GType picman_path_style_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PATH_STYLE_DEFAULT,
  PICMAN_PATH_STYLE_VECTORS,
  PICMAN_PATH_STYLE_OUTLINE
} PicmanPathStyle;


#define PICMAN_TYPE_ZOOM_FOCUS (picman_zoom_focus_get_type ())

GType picman_zoom_focus_get_type (void) G_GNUC_CONST;

typedef enum
{
  /* Make a best guess */
  PICMAN_ZOOM_FOCUS_BEST_GUESS,

  /* Use the mouse cursor (if within canvas) */
  PICMAN_ZOOM_FOCUS_POINTER,

  /* Use the image center */
  PICMAN_ZOOM_FOCUS_IMAGE_CENTER,

  /* If the image is centered, retain the centering. Else use
   * _BEST_GUESS
   */
  PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS

} PicmanZoomFocus;


#endif /* __DISPLAY_ENUMS_H__ */
