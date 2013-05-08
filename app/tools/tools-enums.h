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

#ifndef __TOOLS_ENUMS_H__
#define __TOOLS_ENUMS_H__

/*
 * these enums are registered with the type system
 */

#define PICMAN_TYPE_BUTTON_PRESS_TYPE (picman_button_press_type_get_type ())

GType picman_button_press_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_BUTTON_PRESS_NORMAL,
  PICMAN_BUTTON_PRESS_DOUBLE,
  PICMAN_BUTTON_PRESS_TRIPLE
} PicmanButtonPressType;


#define PICMAN_TYPE_BUTTON_RELEASE_TYPE (picman_button_release_type_get_type ())

GType picman_button_release_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_BUTTON_RELEASE_NORMAL,
  PICMAN_BUTTON_RELEASE_CANCEL,
  PICMAN_BUTTON_RELEASE_CLICK,
  PICMAN_BUTTON_RELEASE_NO_MOTION
} PicmanButtonReleaseType;


#define PICMAN_TYPE_RECTANGLE_CONSTRAINT (picman_rectangle_constraint_get_type ())

GType picman_rectangle_constraint_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RECTANGLE_CONSTRAIN_NONE,
  PICMAN_RECTANGLE_CONSTRAIN_IMAGE,
  PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE
} PicmanRectangleConstraint;


#define PICMAN_TYPE_RECTANGLE_PRECISION (picman_rectangle_precision_get_type ())

GType picman_rectangle_precision_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RECTANGLE_PRECISION_INT,
  PICMAN_RECTANGLE_PRECISION_DOUBLE,
} PicmanRectanglePrecision;


#define PICMAN_TYPE_RECTANGLE_TOOL_FIXED_RULE (picman_rectangle_tool_fixed_rule_get_type ())

GType picman_rectangle_tool_fixed_rule_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RECTANGLE_TOOL_FIXED_ASPECT, /*< desc="Aspect ratio" >*/
  PICMAN_RECTANGLE_TOOL_FIXED_WIDTH,  /*< desc="Width"        >*/
  PICMAN_RECTANGLE_TOOL_FIXED_HEIGHT, /*< desc="Height"       >*/
  PICMAN_RECTANGLE_TOOL_FIXED_SIZE,   /*< desc="Size"         >*/
} PicmanRectangleToolFixedRule;


#define PICMAN_TYPE_RECT_SELECT_MODE (picman_rect_select_mode_get_type ())

GType picman_rect_select_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RECT_SELECT_MODE_FREE,        /*< desc="Free select"        >*/
  PICMAN_RECT_SELECT_MODE_FIXED_SIZE,  /*< desc="Fixed size"         >*/
  PICMAN_RECT_SELECT_MODE_FIXED_RATIO  /*< desc="Fixed aspect ratio" >*/
} PicmanRectSelectMode;


#define PICMAN_TYPE_TRANSFORM_TYPE (picman_transform_type_get_type ())

GType picman_transform_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TRANSFORM_TYPE_LAYER,     /*< desc="Layer"     >*/
  PICMAN_TRANSFORM_TYPE_SELECTION, /*< desc="Selection" >*/
  PICMAN_TRANSFORM_TYPE_PATH       /*< desc="Path"      >*/
} PicmanTransformType;


#define PICMAN_TYPE_VECTOR_MODE (picman_vector_mode_get_type ())

GType picman_vector_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_VECTOR_MODE_DESIGN,      /*< desc="Design" >*/
  PICMAN_VECTOR_MODE_EDIT,        /*< desc="Edit"   >*/
  PICMAN_VECTOR_MODE_MOVE         /*< desc="Move"   >*/
} PicmanVectorMode;


#define PICMAN_TYPE_TOOL_ACTION (picman_tool_action_get_type ())

GType picman_tool_action_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TOOL_ACTION_PAUSE,
  PICMAN_TOOL_ACTION_RESUME,
  PICMAN_TOOL_ACTION_HALT
} PicmanToolAction;


/*
 * non-registered enums; register them if needed
 */

typedef enum /*< skip >*/
{
  SELECTION_SELECT,
  SELECTION_MOVE_MASK,
  SELECTION_MOVE,
  SELECTION_MOVE_COPY,
  SELECTION_ANCHOR
} SelectFunction;

/*  Modes of PicmanEditSelectionTool  */
typedef enum /*< skip >*/
{
  PICMAN_TRANSLATE_MODE_VECTORS,
  PICMAN_TRANSLATE_MODE_CHANNEL,
  PICMAN_TRANSLATE_MODE_LAYER_MASK,
  PICMAN_TRANSLATE_MODE_MASK,
  PICMAN_TRANSLATE_MODE_MASK_TO_LAYER,
  PICMAN_TRANSLATE_MODE_MASK_COPY_TO_LAYER,
  PICMAN_TRANSLATE_MODE_LAYER,
  PICMAN_TRANSLATE_MODE_FLOATING_SEL
} PicmanTranslateMode;

/*  Motion event report modes  */
typedef enum /*< skip >*/
{
  PICMAN_MOTION_MODE_EXACT,
  PICMAN_MOTION_MODE_COMPRESS
} PicmanMotionMode;


#endif /* __TOOLS_ENUMS_H__ */
