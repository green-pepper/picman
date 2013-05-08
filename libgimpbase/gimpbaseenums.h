/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_BASE_ENUMS_H__
#define __PICMAN_BASE_ENUMS_H__


/**
 * SECTION: picmanbaseenums
 * @title: picmanbaseenums
 * @short_description: Basic PICMAN enumeration data types.
 *
 * Basic PICMAN enumeration data types.
 **/


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_ADD_MASK_TYPE (picman_add_mask_type_get_type ())

GType picman_add_mask_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ADD_WHITE_MASK,          /*< desc="_White (full opacity)"           >*/
  PICMAN_ADD_BLACK_MASK,          /*< desc="_Black (full transparency)"      >*/
  PICMAN_ADD_ALPHA_MASK,          /*< desc="Layer's _alpha channel"          >*/
  PICMAN_ADD_ALPHA_TRANSFER_MASK, /*< desc="_Transfer layer's alpha channel" >*/
  PICMAN_ADD_SELECTION_MASK,      /*< desc="_Selection"                      >*/
  PICMAN_ADD_COPY_MASK,           /*< desc="_Grayscale copy of layer"        >*/
  PICMAN_ADD_CHANNEL_MASK         /*< desc="C_hannel"                        >*/
} PicmanAddMaskType;


#define PICMAN_TYPE_BLEND_MODE (picman_blend_mode_get_type ())

GType picman_blend_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_FG_BG_RGB_MODE,         /*< desc="FG to BG (RGB)"    >*/
  PICMAN_FG_BG_HSV_MODE,         /*< desc="FG to BG (HSV)"    >*/
  PICMAN_FG_TRANSPARENT_MODE,    /*< desc="FG to transparent" >*/
  PICMAN_CUSTOM_MODE             /*< desc="Custom gradient"   >*/
} PicmanBlendMode;


#define PICMAN_TYPE_BUCKET_FILL_MODE (picman_bucket_fill_mode_get_type ())

GType picman_bucket_fill_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_FG_BUCKET_FILL,      /*< desc="FG color fill" >*/
  PICMAN_BG_BUCKET_FILL,      /*< desc="BG color fill" >*/
  PICMAN_PATTERN_BUCKET_FILL  /*< desc="Pattern fill"  >*/
} PicmanBucketFillMode;


#define PICMAN_TYPE_CHANNEL_OPS (picman_channel_ops_get_type ())

GType picman_channel_ops_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_CHANNEL_OP_ADD,       /*< desc="Add to the current selection"         >*/
  PICMAN_CHANNEL_OP_SUBTRACT,  /*< desc="Subtract from the current selection"  >*/
  PICMAN_CHANNEL_OP_REPLACE,   /*< desc="Replace the current selection"        >*/
  PICMAN_CHANNEL_OP_INTERSECT  /*< desc="Intersect with the current selection" >*/
} PicmanChannelOps;


#define PICMAN_TYPE_CHANNEL_TYPE (picman_channel_type_get_type ())

GType picman_channel_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RED_CHANNEL,      /*< desc="Red"     >*/
  PICMAN_GREEN_CHANNEL,    /*< desc="Green"   >*/
  PICMAN_BLUE_CHANNEL,     /*< desc="Blue"    >*/
  PICMAN_GRAY_CHANNEL,     /*< desc="Gray"    >*/
  PICMAN_INDEXED_CHANNEL,  /*< desc="Indexed" >*/
  PICMAN_ALPHA_CHANNEL     /*< desc="Alpha"   >*/
} PicmanChannelType;


#define PICMAN_TYPE_CHECK_SIZE (picman_check_size_get_type ())

GType picman_check_size_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_CHECK_SIZE_SMALL_CHECKS  = 0,  /*< desc="Small"  >*/
  PICMAN_CHECK_SIZE_MEDIUM_CHECKS = 1,  /*< desc="Medium" >*/
  PICMAN_CHECK_SIZE_LARGE_CHECKS  = 2   /*< desc="Large"  >*/
} PicmanCheckSize;


#define PICMAN_TYPE_CHECK_TYPE (picman_check_type_get_type ())

GType picman_check_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_CHECK_TYPE_LIGHT_CHECKS = 0,  /*< desc="Light checks"    >*/
  PICMAN_CHECK_TYPE_GRAY_CHECKS  = 1,  /*< desc="Mid-tone checks" >*/
  PICMAN_CHECK_TYPE_DARK_CHECKS  = 2,  /*< desc="Dark checks"     >*/
  PICMAN_CHECK_TYPE_WHITE_ONLY   = 3,  /*< desc="White only"      >*/
  PICMAN_CHECK_TYPE_GRAY_ONLY    = 4,  /*< desc="Gray only"       >*/
  PICMAN_CHECK_TYPE_BLACK_ONLY   = 5   /*< desc="Black only"      >*/
} PicmanCheckType;


#define PICMAN_TYPE_CLONE_TYPE (picman_clone_type_get_type ())

GType picman_clone_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_IMAGE_CLONE,   /*< desc="Image"   >*/
  PICMAN_PATTERN_CLONE  /*< desc="Pattern" >*/
} PicmanCloneType;


#define PICMAN_TYPE_DESATURATE_MODE (picman_desaturate_mode_get_type ())

GType picman_desaturate_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_DESATURATE_LIGHTNESS,   /*< desc="Lightness"  >*/
  PICMAN_DESATURATE_LUMINOSITY,  /*< desc="Luminosity" >*/
  PICMAN_DESATURATE_AVERAGE      /*< desc="Average"    >*/
} PicmanDesaturateMode;


#define PICMAN_TYPE_DODGE_BURN_TYPE (picman_dodge_burn_type_get_type ())

GType picman_dodge_burn_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_DODGE,  /*< desc="Dodge" >*/
  PICMAN_BURN    /*< desc="Burn"  >*/
} PicmanDodgeBurnType;


#define PICMAN_TYPE_FOREGROUND_EXTRACT_MODE (picman_foreground_extract_mode_get_type ())

GType picman_foreground_extract_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_FOREGROUND_EXTRACT_SIOX
} PicmanForegroundExtractMode;


#define PICMAN_TYPE_GRADIENT_TYPE (picman_gradient_type_get_type ())

GType picman_gradient_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_GRADIENT_LINEAR,                /*< desc="Linear"            >*/
  PICMAN_GRADIENT_BILINEAR,              /*< desc="Bi-linear"         >*/
  PICMAN_GRADIENT_RADIAL,                /*< desc="Radial"            >*/
  PICMAN_GRADIENT_SQUARE,                /*< desc="Square"            >*/
  PICMAN_GRADIENT_CONICAL_SYMMETRIC,     /*< desc="Conical (sym)"     >*/
  PICMAN_GRADIENT_CONICAL_ASYMMETRIC,    /*< desc="Conical (asym)"    >*/
  PICMAN_GRADIENT_SHAPEBURST_ANGULAR,    /*< desc="Shaped (angular)"  >*/
  PICMAN_GRADIENT_SHAPEBURST_SPHERICAL,  /*< desc="Shaped (spherical)">*/
  PICMAN_GRADIENT_SHAPEBURST_DIMPLED,    /*< desc="Shaped (dimpled)"  >*/
  PICMAN_GRADIENT_SPIRAL_CLOCKWISE,      /*< desc="Spiral (cw)"       >*/
  PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE   /*< desc="Spiral (ccw)"      >*/
} PicmanGradientType;


#define PICMAN_TYPE_GRID_STYLE (picman_grid_style_get_type ())

GType picman_grid_style_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_GRID_DOTS,           /*< desc="Intersections (dots)"       >*/
  PICMAN_GRID_INTERSECTIONS,  /*< desc="Intersections (crosshairs)" >*/
  PICMAN_GRID_ON_OFF_DASH,    /*< desc="Dashed"                     >*/
  PICMAN_GRID_DOUBLE_DASH,    /*< desc="Double dashed"              >*/
  PICMAN_GRID_SOLID           /*< desc="Solid"                      >*/
} PicmanGridStyle;


#define PICMAN_TYPE_ICON_TYPE (picman_icon_type_get_type ())

GType picman_icon_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ICON_TYPE_STOCK_ID,      /*< desc="Stock ID"      >*/
  PICMAN_ICON_TYPE_INLINE_PIXBUF, /*< desc="Inline pixbuf" >*/
  PICMAN_ICON_TYPE_IMAGE_FILE     /*< desc="Image file"    >*/
} PicmanIconType;


#define PICMAN_TYPE_IMAGE_BASE_TYPE (picman_image_base_type_get_type ())

GType picman_image_base_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RGB,     /*< desc="RGB color"     >*/
  PICMAN_GRAY,    /*< desc="Grayscale"     >*/
  PICMAN_INDEXED  /*< desc="Indexed color" >*/
} PicmanImageBaseType;


#define PICMAN_TYPE_IMAGE_TYPE (picman_image_type_get_type ())

GType picman_image_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RGB_IMAGE,      /*< desc="RGB"             >*/
  PICMAN_RGBA_IMAGE,     /*< desc="RGB-alpha"       >*/
  PICMAN_GRAY_IMAGE,     /*< desc="Grayscale"       >*/
  PICMAN_GRAYA_IMAGE,    /*< desc="Grayscale-alpha" >*/
  PICMAN_INDEXED_IMAGE,  /*< desc="Indexed"         >*/
  PICMAN_INDEXEDA_IMAGE  /*< desc="Indexed-alpha"   >*/
} PicmanImageType;


#define PICMAN_TYPE_INTERPOLATION_TYPE (picman_interpolation_type_get_type ())

GType picman_interpolation_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_INTERPOLATION_NONE,   /*< desc="None"   >*/
  PICMAN_INTERPOLATION_LINEAR, /*< desc="Linear" >*/
  PICMAN_INTERPOLATION_CUBIC,  /*< desc="Cubic"  >*/
  PICMAN_INTERPOLATION_NOHALO, /*< desc="NoHalo" >*/
  PICMAN_INTERPOLATION_LOHALO, /*< desc="LoHalo" >*/
  PICMAN_INTERPOLATION_LANCZOS = PICMAN_INTERPOLATION_NOHALO /*< skip */
} PicmanInterpolationType;


#define PICMAN_TYPE_PAINT_APPLICATION_MODE (picman_paint_application_mode_get_type ())

GType picman_paint_application_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PAINT_CONSTANT,    /*< desc="Constant"    >*/
  PICMAN_PAINT_INCREMENTAL  /*< desc="Incremental" >*/
} PicmanPaintApplicationMode;


#define PICMAN_TYPE_REPEAT_MODE (picman_repeat_mode_get_type ())

GType picman_repeat_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_REPEAT_NONE,       /*< desc="None"            >*/
  PICMAN_REPEAT_SAWTOOTH,   /*< desc="Sawtooth wave"   >*/
  PICMAN_REPEAT_TRIANGULAR  /*< desc="Triangular wave" >*/
} PicmanRepeatMode;


#define PICMAN_TYPE_RUN_MODE (picman_run_mode_get_type ())

GType picman_run_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_RUN_INTERACTIVE,     /*< desc="Run interactively"         >*/
  PICMAN_RUN_NONINTERACTIVE,  /*< desc="Run non-interactively"     >*/
  PICMAN_RUN_WITH_LAST_VALS   /*< desc="Run with last used values" >*/
} PicmanRunMode;


#define PICMAN_TYPE_SIZE_TYPE (picman_size_type_get_type ())

GType picman_size_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PIXELS,  /*< desc="Pixels" >*/
  PICMAN_POINTS   /*< desc="Points" >*/
} PicmanSizeType;


#define PICMAN_TYPE_TRANSFER_MODE (picman_transfer_mode_get_type ())

GType picman_transfer_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_SHADOWS,     /*< desc="Shadows"    >*/
  PICMAN_MIDTONES,    /*< desc="Midtones"   >*/
  PICMAN_HIGHLIGHTS   /*< desc="Highlights" >*/
} PicmanTransferMode;


#define PICMAN_TYPE_TRANSFORM_DIRECTION (picman_transform_direction_get_type ())

GType picman_transform_direction_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TRANSFORM_FORWARD,   /*< desc="Normal (Forward)" >*/
  PICMAN_TRANSFORM_BACKWARD   /*< desc="Corrective (Backward)" >*/
} PicmanTransformDirection;


#define PICMAN_TYPE_TRANSFORM_RESIZE (picman_transform_resize_get_type ())

GType picman_transform_resize_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TRANSFORM_RESIZE_ADJUST           = 0, /*< desc="Adjust" >*/
  PICMAN_TRANSFORM_RESIZE_CLIP             = 1, /*< desc="Clip" >*/
  PICMAN_TRANSFORM_RESIZE_CROP,                 /*< desc="Crop to result" >*/
  PICMAN_TRANSFORM_RESIZE_CROP_WITH_ASPECT      /*< desc="Crop with aspect" >*/
} PicmanTransformResize;


typedef enum /*< skip >*/
{
  PICMAN_UNIT_PIXEL   = 0,

  PICMAN_UNIT_INCH    = 1,
  PICMAN_UNIT_MM      = 2,
  PICMAN_UNIT_POINT   = 3,
  PICMAN_UNIT_PICA    = 4,

  PICMAN_UNIT_END     = 5,

  PICMAN_UNIT_PERCENT = 65536 /*< pdb-skip >*/
} PicmanUnit;


#define PICMAN_TYPE_PDB_ARG_TYPE (picman_pdb_arg_type_get_type ())

GType picman_pdb_arg_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PDB_INT32,
  PICMAN_PDB_INT16,
  PICMAN_PDB_INT8,
  PICMAN_PDB_FLOAT,
  PICMAN_PDB_STRING,
  PICMAN_PDB_INT32ARRAY,
  PICMAN_PDB_INT16ARRAY,
  PICMAN_PDB_INT8ARRAY,
  PICMAN_PDB_FLOATARRAY,
  PICMAN_PDB_STRINGARRAY,
  PICMAN_PDB_COLOR,
  PICMAN_PDB_ITEM,
  PICMAN_PDB_DISPLAY,
  PICMAN_PDB_IMAGE,
  PICMAN_PDB_LAYER,
  PICMAN_PDB_CHANNEL,
  PICMAN_PDB_DRAWABLE,
  PICMAN_PDB_SELECTION,
  PICMAN_PDB_COLORARRAY,
  PICMAN_PDB_VECTORS,
  PICMAN_PDB_PARASITE,
  PICMAN_PDB_STATUS,
  PICMAN_PDB_END,

  /*  the following aliases are deprecated  */
  PICMAN_PDB_PATH     = PICMAN_PDB_VECTORS,     /*< skip >*/
  PICMAN_PDB_BOUNDARY = PICMAN_PDB_COLORARRAY,  /*< skip >*/
  PICMAN_PDB_REGION   = PICMAN_PDB_ITEM         /*< skip >*/
} PicmanPDBArgType;


#define PICMAN_TYPE_PDB_ERROR_HANDLER (picman_pdb_error_handler_get_type ())

GType picman_pdb_error_handler_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PDB_ERROR_HANDLER_INTERNAL,
  PICMAN_PDB_ERROR_HANDLER_PLUGIN
} PicmanPDBErrorHandler;


#define PICMAN_TYPE_PDB_PROC_TYPE (picman_pdb_proc_type_get_type ())

GType picman_pdb_proc_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_INTERNAL,   /*< desc="Internal PICMAN procedure" >*/
  PICMAN_PLUGIN,     /*< desc="PICMAN Plug-In" >*/
  PICMAN_EXTENSION,  /*< desc="PICMAN Extension" >*/
  PICMAN_TEMPORARY   /*< desc="Temporary Procedure" >*/
} PicmanPDBProcType;


#define PICMAN_TYPE_PDB_STATUS_TYPE (picman_pdb_status_type_get_type ())

GType picman_pdb_status_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PDB_EXECUTION_ERROR,
  PICMAN_PDB_CALLING_ERROR,
  PICMAN_PDB_PASS_THROUGH,
  PICMAN_PDB_SUCCESS,
  PICMAN_PDB_CANCEL
} PicmanPDBStatusType;


#define PICMAN_TYPE_MESSAGE_HANDLER_TYPE (picman_message_handler_type_get_type ())

GType picman_message_handler_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_MESSAGE_BOX,
  PICMAN_CONSOLE,
  PICMAN_ERROR_CONSOLE
} PicmanMessageHandlerType;


#define PICMAN_TYPE_STACK_TRACE_MODE (picman_stack_trace_mode_get_type ())

GType picman_stack_trace_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_STACK_TRACE_NEVER,
  PICMAN_STACK_TRACE_QUERY,
  PICMAN_STACK_TRACE_ALWAYS
} PicmanStackTraceMode;


#define PICMAN_TYPE_PROGRESS_COMMAND (picman_progress_command_get_type ())

GType picman_progress_command_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PROGRESS_COMMAND_START,
  PICMAN_PROGRESS_COMMAND_END,
  PICMAN_PROGRESS_COMMAND_SET_TEXT,
  PICMAN_PROGRESS_COMMAND_SET_VALUE,
  PICMAN_PROGRESS_COMMAND_PULSE,
  PICMAN_PROGRESS_COMMAND_GET_WINDOW
} PicmanProgressCommand;


#define PICMAN_TYPE_TEXT_DIRECTION (picman_text_direction_get_type ())

GType picman_text_direction_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TEXT_DIRECTION_LTR,   /*< desc="From left to right" >*/
  PICMAN_TEXT_DIRECTION_RTL    /*< desc="From right to left" >*/
} PicmanTextDirection;


#define PICMAN_TYPE_TEXT_HINT_STYLE (picman_text_hint_style_get_type ())

GType picman_text_hint_style_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TEXT_HINT_STYLE_NONE,     /*< desc="None"   >*/
  PICMAN_TEXT_HINT_STYLE_SLIGHT,   /*< desc="Slight" >*/
  PICMAN_TEXT_HINT_STYLE_MEDIUM,   /*< desc="Medium" >*/
  PICMAN_TEXT_HINT_STYLE_FULL      /*< desc="Full"   >*/
} PicmanTextHintStyle;


#define PICMAN_TYPE_TEXT_JUSTIFICATION (picman_text_justification_get_type ())

GType picman_text_justification_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_TEXT_JUSTIFY_LEFT,    /*< desc="Left justified"  >*/
  PICMAN_TEXT_JUSTIFY_RIGHT,   /*< desc="Right justified" >*/
  PICMAN_TEXT_JUSTIFY_CENTER,  /*< desc="Centered"        >*/
  PICMAN_TEXT_JUSTIFY_FILL     /*< desc="Filled"          >*/
} PicmanTextJustification;


#ifndef PICMAN_DISABLE_DEPRECATED
#define PICMAN_TYPE_USER_DIRECTORY (picman_user_directory_get_type ())

GType picman_user_directory_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_USER_DIRECTORY_DESKTOP,
  PICMAN_USER_DIRECTORY_DOCUMENTS,
  PICMAN_USER_DIRECTORY_DOWNLOAD,
  PICMAN_USER_DIRECTORY_MUSIC,
  PICMAN_USER_DIRECTORY_PICTURES,
  PICMAN_USER_DIRECTORY_PUBLIC_SHARE,
  PICMAN_USER_DIRECTORY_TEMPLATES,
  PICMAN_USER_DIRECTORY_VIDEOS
} PicmanUserDirectory;
#endif /* !PICMAN_DISABLE_DEPRECATED */


#define PICMAN_TYPE_VECTORS_STROKE_TYPE (picman_vectors_stroke_type_get_type ())

GType picman_vectors_stroke_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_VECTORS_STROKE_TYPE_BEZIER
} PicmanVectorsStrokeType;

G_END_DECLS

#endif  /* __PICMAN_BASE_ENUMS_H__ */
