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

#ifndef __CORE_ENUMS_H__
#define __CORE_ENUMS_H__


#if 0
   This file is parsed by two scripts, enumgen.pl in tools/pdbgen,
   and picman-mkenums. All enums that are not marked with
   /*< pdb-skip >*/ are exported to libpicman and the PDB. Enums that are
   not marked with /*< skip >*/ are registered with the GType system.
   If you want the enum to be skipped by both scripts, you have to use
   /*< pdb-skip, skip >*/.

   The same syntax applies to enum values.
#endif


/*
 * these enums are registered with the type system
 */


#define PICMAN_TYPE_COMPONENT_MASK (picman_component_mask_get_type ())

GType picman_component_mask_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_COMPONENT_RED   = 1 << 0,
  PICMAN_COMPONENT_GREEN = 1 << 1,
  PICMAN_COMPONENT_BLUE  = 1 << 2,
  PICMAN_COMPONENT_ALPHA = 1 << 3,

  PICMAN_COMPONENT_ALL = (PICMAN_COMPONENT_RED   |
                        PICMAN_COMPONENT_GREEN |
                        PICMAN_COMPONENT_BLUE  |
                        PICMAN_COMPONENT_ALPHA)
} PicmanComponentMask;


#define PICMAN_TYPE_CONTAINER_POLICY (picman_container_policy_get_type ())

GType picman_container_policy_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_CONTAINER_POLICY_STRONG,
  PICMAN_CONTAINER_POLICY_WEAK
} PicmanContainerPolicy;


#define PICMAN_TYPE_CONVERT_DITHER_TYPE (picman_convert_dither_type_get_type ())

GType picman_convert_dither_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_NO_DITHER,         /*< desc="None"                                     >*/
  PICMAN_FS_DITHER,         /*< desc="Floyd-Steinberg (normal)"                 >*/
  PICMAN_FSLOWBLEED_DITHER, /*< desc="Floyd-Steinberg (reduced color bleeding)" >*/
  PICMAN_FIXED_DITHER,      /*< desc="Positioned"                               >*/
  PICMAN_NODESTRUCT_DITHER  /*< pdb-skip, skip >*/
} PicmanConvertDitherType;


#define PICMAN_TYPE_CONVERT_PALETTE_TYPE (picman_convert_palette_type_get_type ())

GType picman_convert_palette_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_MAKE_PALETTE,    /*< desc="Generate optimum palette"            >*/
  PICMAN_REUSE_PALETTE,   /*< skip >*/
  PICMAN_WEB_PALETTE,     /*< desc="Use web-optimized palette"           >*/
  PICMAN_MONO_PALETTE,    /*< desc="Use black and white (1-bit) palette" >*/
  PICMAN_CUSTOM_PALETTE   /*< desc="Use custom palette"                  >*/
} PicmanConvertPaletteType;


#define PICMAN_TYPE_CONVOLUTION_TYPE (picman_convolution_type_get_type ())

GType picman_convolution_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_NORMAL_CONVOL,      /*  Negative numbers truncated  */
  PICMAN_ABSOLUTE_CONVOL,    /*  Absolute value              */
  PICMAN_NEGATIVE_CONVOL     /*  add 127 to values           */
} PicmanConvolutionType;


#define PICMAN_TYPE_CURVE_TYPE (picman_curve_type_get_type ())

GType picman_curve_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_CURVE_SMOOTH,   /*< desc="Smooth"   >*/
  PICMAN_CURVE_FREE      /*< desc="Freehand" >*/
} PicmanCurveType;


#define PICMAN_TYPE_GRAVITY_TYPE (picman_gravity_type_get_type ())

GType picman_gravity_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_GRAVITY_NONE,
  PICMAN_GRAVITY_NORTH_WEST,
  PICMAN_GRAVITY_NORTH,
  PICMAN_GRAVITY_NORTH_EAST,
  PICMAN_GRAVITY_WEST,
  PICMAN_GRAVITY_CENTER,
  PICMAN_GRAVITY_EAST,
  PICMAN_GRAVITY_SOUTH_WEST,
  PICMAN_GRAVITY_SOUTH,
  PICMAN_GRAVITY_SOUTH_EAST
} PicmanGravityType;


#define PICMAN_TYPE_HISTOGRAM_CHANNEL (picman_histogram_channel_get_type ())

GType picman_histogram_channel_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_HISTOGRAM_VALUE = 0,  /*< desc="Value" >*/
  PICMAN_HISTOGRAM_RED   = 1,  /*< desc="Red"   >*/
  PICMAN_HISTOGRAM_GREEN = 2,  /*< desc="Green" >*/
  PICMAN_HISTOGRAM_BLUE  = 3,  /*< desc="Blue"  >*/
  PICMAN_HISTOGRAM_ALPHA = 4,  /*< desc="Alpha" >*/
  PICMAN_HISTOGRAM_RGB   = 5   /*< desc="RGB", pdb-skip >*/
} PicmanHistogramChannel;


#define PICMAN_TYPE_HUE_RANGE (picman_hue_range_get_type ())

GType picman_hue_range_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ALL_HUES,
  PICMAN_RED_HUES,
  PICMAN_YELLOW_HUES,
  PICMAN_GREEN_HUES,
  PICMAN_CYAN_HUES,
  PICMAN_BLUE_HUES,
  PICMAN_MAGENTA_HUES
} PicmanHueRange;


#define PICMAN_TYPE_LAYER_MODE_EFFECTS (picman_layer_mode_effects_get_type ())

GType picman_layer_mode_effects_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_NORMAL_MODE,          /*< desc="Normal"               >*/
  PICMAN_DISSOLVE_MODE,        /*< desc="Dissolve"             >*/
  PICMAN_BEHIND_MODE,          /*< desc="Behind"               >*/
  PICMAN_MULTIPLY_MODE,        /*< desc="Multiply"             >*/
  PICMAN_SCREEN_MODE,          /*< desc="Screen"               >*/
  PICMAN_OVERLAY_MODE,         /*< desc="Overlay"              >*/
  PICMAN_DIFFERENCE_MODE,      /*< desc="Difference"           >*/
  PICMAN_ADDITION_MODE,        /*< desc="Addition"             >*/
  PICMAN_SUBTRACT_MODE,        /*< desc="Subtract"             >*/
  PICMAN_DARKEN_ONLY_MODE,     /*< desc="Darken only"          >*/
  PICMAN_LIGHTEN_ONLY_MODE,    /*< desc="Lighten only"         >*/
  PICMAN_HUE_MODE,             /*< desc="Hue"                  >*/
  PICMAN_SATURATION_MODE,      /*< desc="Saturation"           >*/
  PICMAN_COLOR_MODE,           /*< desc="Color"                >*/
  PICMAN_VALUE_MODE,           /*< desc="Value"                >*/
  PICMAN_DIVIDE_MODE,          /*< desc="Divide"               >*/
  PICMAN_DODGE_MODE,           /*< desc="Dodge"                >*/
  PICMAN_BURN_MODE,            /*< desc="Burn"                 >*/
  PICMAN_HARDLIGHT_MODE,       /*< desc="Hard light"           >*/
  PICMAN_SOFTLIGHT_MODE,       /*< desc="Soft light"           >*/
  PICMAN_GRAIN_EXTRACT_MODE,   /*< desc="Grain extract"        >*/
  PICMAN_GRAIN_MERGE_MODE,     /*< desc="Grain merge"          >*/
  PICMAN_COLOR_ERASE_MODE,     /*< desc="Color erase"          >*/
  PICMAN_ERASE_MODE,           /*< pdb-skip, desc="Erase"      >*/
  PICMAN_REPLACE_MODE,         /*< pdb-skip, desc="Replace"    >*/
  PICMAN_ANTI_ERASE_MODE       /*< pdb-skip, desc="Anti erase" >*/
} PicmanLayerModeEffects;


#define PICMAN_TYPE_ALIGNMENT_TYPE (picman_alignment_type_get_type ())

GType picman_alignment_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_ALIGN_LEFT,
  PICMAN_ALIGN_HCENTER,
  PICMAN_ALIGN_RIGHT,
  PICMAN_ALIGN_TOP,
  PICMAN_ALIGN_VCENTER,
  PICMAN_ALIGN_BOTTOM,
  PICMAN_ARRANGE_LEFT,
  PICMAN_ARRANGE_HCENTER,
  PICMAN_ARRANGE_RIGHT,
  PICMAN_ARRANGE_TOP,
  PICMAN_ARRANGE_VCENTER,
  PICMAN_ARRANGE_BOTTOM
} PicmanAlignmentType;


#define PICMAN_TYPE_ALIGN_REFERENCE_TYPE (picman_align_reference_type_get_type ())

GType picman_align_reference_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_ALIGN_REFERENCE_FIRST,          /*< desc="First item"     >*/
  PICMAN_ALIGN_REFERENCE_IMAGE,          /*< desc="Image"          >*/
  PICMAN_ALIGN_REFERENCE_SELECTION,      /*< desc="Selection"      >*/
  PICMAN_ALIGN_REFERENCE_ACTIVE_LAYER,   /*< desc="Active layer"   >*/
  PICMAN_ALIGN_REFERENCE_ACTIVE_CHANNEL, /*< desc="Active channel" >*/
  PICMAN_ALIGN_REFERENCE_ACTIVE_PATH     /*< desc="Active path"    >*/
} PicmanAlignReferenceType;


#define PICMAN_TYPE_FILL_TYPE (picman_fill_type_get_type ())

GType picman_fill_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_FOREGROUND_FILL,   /*< desc="Foreground color" >*/
  PICMAN_BACKGROUND_FILL,   /*< desc="Background color" >*/
  PICMAN_WHITE_FILL,        /*< desc="White"            >*/
  PICMAN_TRANSPARENT_FILL,  /*< desc="Transparency"     >*/
  PICMAN_PATTERN_FILL,      /*< desc="Pattern"          >*/
  PICMAN_NO_FILL            /*< desc="None",   pdb-skip >*/
} PicmanFillType;


#define PICMAN_TYPE_FILL_STYLE (picman_fill_style_get_type ())

GType picman_fill_style_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_FILL_STYLE_SOLID,  /*< desc="Solid color" >*/
  PICMAN_FILL_STYLE_PATTERN /*< desc="Pattern"     >*/
} PicmanFillStyle;


#define PICMAN_TYPE_STROKE_METHOD (picman_stroke_method_get_type ())

GType picman_stroke_method_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_STROKE_METHOD_LIBART,     /*< desc="Stroke line"              >*/
  PICMAN_STROKE_METHOD_PAINT_CORE  /*< desc="Stroke with a paint tool" >*/
} PicmanStrokeMethod;


#define PICMAN_TYPE_JOIN_STYLE (picman_join_style_get_type ())

GType picman_join_style_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_JOIN_MITER,  /*< desc="Miter" >*/
  PICMAN_JOIN_ROUND,  /*< desc="Round" >*/
  PICMAN_JOIN_BEVEL   /*< desc="Bevel" >*/
} PicmanJoinStyle;


#define PICMAN_TYPE_CAP_STYLE (picman_cap_style_get_type ())

GType picman_cap_style_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_CAP_BUTT,   /*< desc="Butt"   >*/
  PICMAN_CAP_ROUND,  /*< desc="Round"  >*/
  PICMAN_CAP_SQUARE  /*< desc="Square" >*/
} PicmanCapStyle;


#define PICMAN_TYPE_DASH_PRESET (picman_dash_preset_get_type ())

GType picman_dash_preset_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_DASH_CUSTOM,       /*< desc="Custom"         >*/
  PICMAN_DASH_LINE,         /*< desc="Line"           >*/
  PICMAN_DASH_LONG_DASH,    /*< desc="Long dashes"    >*/
  PICMAN_DASH_MEDIUM_DASH,  /*< desc="Medium dashes"  >*/
  PICMAN_DASH_SHORT_DASH,   /*< desc="Short dashes"   >*/
  PICMAN_DASH_SPARSE_DOTS,  /*< desc="Sparse dots"    >*/
  PICMAN_DASH_NORMAL_DOTS,  /*< desc="Normal dots"    >*/
  PICMAN_DASH_DENSE_DOTS,   /*< desc="Dense dots"     >*/
  PICMAN_DASH_STIPPLES,     /*< desc="Stipples"       >*/
  PICMAN_DASH_DASH_DOT,     /*< desc="Dash, dot"      >*/
  PICMAN_DASH_DASH_DOT_DOT  /*< desc="Dash, dot, dot" >*/
} PicmanDashPreset;


#define PICMAN_TYPE_BRUSH_GENERATED_SHAPE (picman_brush_generated_shape_get_type ())

GType picman_brush_generated_shape_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_BRUSH_GENERATED_CIRCLE,  /*< desc="Circle"  >*/
  PICMAN_BRUSH_GENERATED_SQUARE,  /*< desc="Square"  >*/
  PICMAN_BRUSH_GENERATED_DIAMOND  /*< desc="Diamond" >*/
} PicmanBrushGeneratedShape;


#define PICMAN_TYPE_ORIENTATION_TYPE (picman_orientation_type_get_type ())

GType picman_orientation_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ORIENTATION_HORIZONTAL, /*< desc="Horizontal" >*/
  PICMAN_ORIENTATION_VERTICAL,   /*< desc="Vertical"   >*/
  PICMAN_ORIENTATION_UNKNOWN     /*< desc="Unknown"    >*/
} PicmanOrientationType;


#define PICMAN_TYPE_PRECISION (picman_precision_get_type ())

GType picman_precision_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_PRECISION_U8,    /*< desc="8-bit integer"         >*/
  PICMAN_PRECISION_U16,   /*< desc="16-bit integer"        >*/
  PICMAN_PRECISION_U32,   /*< desc="32-bit integer"        >*/
  PICMAN_PRECISION_HALF,  /*< desc="16-bit floating point" >*/
  PICMAN_PRECISION_FLOAT  /*< desc="32-bit floating point" >*/
} PicmanPrecision;


#define PICMAN_TYPE_ITEM_SET (picman_item_set_get_type ())

GType picman_item_set_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_ITEM_SET_NONE,        /*< desc="None"               >*/
  PICMAN_ITEM_SET_ALL,         /*< desc="All layers"         >*/
  PICMAN_ITEM_SET_IMAGE_SIZED, /*< desc="Image-sized layers" >*/
  PICMAN_ITEM_SET_VISIBLE,     /*< desc="All visible layers" >*/
  PICMAN_ITEM_SET_LINKED       /*< desc="All linked layers"  >*/
} PicmanItemSet;


#define PICMAN_TYPE_ROTATION_TYPE (picman_rotation_type_get_type ())

GType picman_rotation_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_ROTATE_90,
  PICMAN_ROTATE_180,
  PICMAN_ROTATE_270
} PicmanRotationType;


#define PICMAN_TYPE_VIEW_SIZE (picman_view_size_get_type ())

GType picman_view_size_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_VIEW_SIZE_TINY        = 12,   /*< desc="Tiny"        >*/
  PICMAN_VIEW_SIZE_EXTRA_SMALL = 16,   /*< desc="Very small"  >*/
  PICMAN_VIEW_SIZE_SMALL       = 24,   /*< desc="Small"       >*/
  PICMAN_VIEW_SIZE_MEDIUM      = 32,   /*< desc="Medium"      >*/
  PICMAN_VIEW_SIZE_LARGE       = 48,   /*< desc="Large"       >*/
  PICMAN_VIEW_SIZE_EXTRA_LARGE = 64,   /*< desc="Very large"  >*/
  PICMAN_VIEW_SIZE_HUGE        = 96,   /*< desc="Huge"        >*/
  PICMAN_VIEW_SIZE_ENORMOUS    = 128,  /*< desc="Enormous"    >*/
  PICMAN_VIEW_SIZE_GIGANTIC    = 192   /*< desc="Gigantic"    >*/
} PicmanViewSize;


#define PICMAN_TYPE_VIEW_TYPE (picman_view_type_get_type ())

GType picman_view_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_VIEW_TYPE_LIST,  /*< desc="View as list" >*/
  PICMAN_VIEW_TYPE_GRID   /*< desc="View as grid" >*/
} PicmanViewType;


#define PICMAN_TYPE_THUMBNAIL_SIZE (picman_thumbnail_size_get_type ())

GType picman_thumbnail_size_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_THUMBNAIL_SIZE_NONE    = 0,    /*< desc="No thumbnails"    >*/
  PICMAN_THUMBNAIL_SIZE_NORMAL  = 128,  /*< desc="Normal (128x128)" >*/
  PICMAN_THUMBNAIL_SIZE_LARGE   = 256   /*< desc="Large (256x256)"  >*/
} PicmanThumbnailSize;


#define PICMAN_TYPE_UNDO_MODE (picman_undo_mode_get_type ())

GType picman_undo_mode_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_UNDO_MODE_UNDO,
  PICMAN_UNDO_MODE_REDO
} PicmanUndoMode;


#define PICMAN_TYPE_UNDO_EVENT (picman_undo_event_get_type ())

GType picman_undo_event_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_UNDO_EVENT_UNDO_PUSHED,  /* a new undo has been added to the undo stack */
  PICMAN_UNDO_EVENT_UNDO_EXPIRED, /* an undo has been freed from the undo stack  */
  PICMAN_UNDO_EVENT_REDO_EXPIRED, /* a redo has been freed from the redo stack   */
  PICMAN_UNDO_EVENT_UNDO,         /* an undo has been executed                   */
  PICMAN_UNDO_EVENT_REDO,         /* a redo has been executed                    */
  PICMAN_UNDO_EVENT_UNDO_FREE,    /* all undo and redo info has been cleared     */
  PICMAN_UNDO_EVENT_UNDO_FREEZE,  /* undo has been frozen                        */
  PICMAN_UNDO_EVENT_UNDO_THAW     /* undo has been thawn                         */
} PicmanUndoEvent;


#define PICMAN_TYPE_UNDO_TYPE (picman_undo_type_get_type ())

GType picman_undo_type_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  /* Type NO_UNDO_GROUP (0) is special - in the picmanimage structure it
   * means there is no undo group currently being added to.
   */
  PICMAN_UNDO_GROUP_NONE = 0,           /*< desc="<<invalid>>"                 >*/

  PICMAN_UNDO_GROUP_FIRST = PICMAN_UNDO_GROUP_NONE, /*< skip >*/

  PICMAN_UNDO_GROUP_IMAGE_SCALE,        /*< desc="Scale image"                 >*/
  PICMAN_UNDO_GROUP_IMAGE_RESIZE,       /*< desc="Resize image"                >*/
  PICMAN_UNDO_GROUP_IMAGE_FLIP,         /*< desc="Flip image"                  >*/
  PICMAN_UNDO_GROUP_IMAGE_ROTATE,       /*< desc="Rotate image"                >*/
  PICMAN_UNDO_GROUP_IMAGE_CROP,         /*< desc="Crop image"                  >*/
  PICMAN_UNDO_GROUP_IMAGE_CONVERT,      /*< desc="Convert image"               >*/
  PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE,  /*< desc="Remove item"                 >*/
  PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE, /*< desc="Merge layers"                >*/
  PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE,/*< desc="Merge paths"                 >*/
  PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK,   /*< desc="Quick Mask"                  >*/
  PICMAN_UNDO_GROUP_IMAGE_GRID,         /*< desc="Grid"                        >*/
  PICMAN_UNDO_GROUP_GUIDE,              /*< desc="Guide"                       >*/
  PICMAN_UNDO_GROUP_SAMPLE_POINT,       /*< desc="Sample Point"                >*/
  PICMAN_UNDO_GROUP_DRAWABLE,           /*< desc="Layer/Channel"               >*/
  PICMAN_UNDO_GROUP_DRAWABLE_MOD,       /*< desc="Layer/Channel modification"  >*/
  PICMAN_UNDO_GROUP_MASK,               /*< desc="Selection mask"              >*/
  PICMAN_UNDO_GROUP_ITEM_VISIBILITY,    /*< desc="Item visibility"             >*/
  PICMAN_UNDO_GROUP_ITEM_LINKED,        /*< desc="Link/Unlink item"            >*/
  PICMAN_UNDO_GROUP_ITEM_PROPERTIES,    /*< desc="Item properties"             >*/
  PICMAN_UNDO_GROUP_ITEM_DISPLACE,      /*< desc="Move item"                   >*/
  PICMAN_UNDO_GROUP_ITEM_SCALE,         /*< desc="Scale item"                  >*/
  PICMAN_UNDO_GROUP_ITEM_RESIZE,        /*< desc="Resize item"                 >*/
  PICMAN_UNDO_GROUP_LAYER_ADD,          /*< desc="Add layer"                   >*/
  PICMAN_UNDO_GROUP_LAYER_ADD_MASK,     /*< desc="Add layer mask"              >*/
  PICMAN_UNDO_GROUP_LAYER_APPLY_MASK,   /*< desc="Apply layer mask"            >*/
  PICMAN_UNDO_GROUP_FS_TO_LAYER,        /*< desc="Floating selection to layer" >*/
  PICMAN_UNDO_GROUP_FS_FLOAT,           /*< desc="Float selection"             >*/
  PICMAN_UNDO_GROUP_FS_ANCHOR,          /*< desc="Anchor floating selection"   >*/
  PICMAN_UNDO_GROUP_EDIT_PASTE,         /*< desc="Paste"                       >*/
  PICMAN_UNDO_GROUP_EDIT_CUT,           /*< desc="Cut"                         >*/
  PICMAN_UNDO_GROUP_TEXT,               /*< desc="Text"                        >*/
  PICMAN_UNDO_GROUP_TRANSFORM,          /*< desc="Transform"                   >*/
  PICMAN_UNDO_GROUP_PAINT,              /*< desc="Paint"                       >*/
  PICMAN_UNDO_GROUP_PARASITE_ATTACH,    /*< desc="Attach parasite"             >*/
  PICMAN_UNDO_GROUP_PARASITE_REMOVE,    /*< desc="Remove parasite"             >*/
  PICMAN_UNDO_GROUP_VECTORS_IMPORT,     /*< desc="Import paths"                >*/
  PICMAN_UNDO_GROUP_MISC,               /*< desc="Plug-In"                     >*/

  PICMAN_UNDO_GROUP_LAST = PICMAN_UNDO_GROUP_MISC, /*< skip >*/

  /*  Undo types which actually do something  */

  PICMAN_UNDO_IMAGE_TYPE,               /*< desc="Image type"                  >*/
  PICMAN_UNDO_IMAGE_PRECISION,          /*< desc="Image precision"             >*/
  PICMAN_UNDO_IMAGE_SIZE,               /*< desc="Image size"                  >*/
  PICMAN_UNDO_IMAGE_RESOLUTION,         /*< desc="Image resolution change"     >*/
  PICMAN_UNDO_IMAGE_GRID,               /*< desc="Grid"                        >*/
  PICMAN_UNDO_IMAGE_COLORMAP,           /*< desc="Change indexed palette"      >*/
  PICMAN_UNDO_GUIDE,                    /*< desc="Guide"                       >*/
  PICMAN_UNDO_SAMPLE_POINT,             /*< desc="Sample Point"                >*/
  PICMAN_UNDO_DRAWABLE,                 /*< desc="Layer/Channel"               >*/
  PICMAN_UNDO_DRAWABLE_MOD,             /*< desc="Layer/Channel modification"  >*/
  PICMAN_UNDO_MASK,                     /*< desc="Selection mask"              >*/
  PICMAN_UNDO_ITEM_REORDER,             /*< desc="Reorder item"                >*/
  PICMAN_UNDO_ITEM_RENAME,              /*< desc="Rename item"                 >*/
  PICMAN_UNDO_ITEM_DISPLACE,            /*< desc="Move item"                   >*/
  PICMAN_UNDO_ITEM_VISIBILITY,          /*< desc="Item visibility"             >*/
  PICMAN_UNDO_ITEM_LINKED,              /*< desc="Link/Unlink item"            >*/
  PICMAN_UNDO_ITEM_LOCK_CONTENT,        /*< desc="Lock/Unlock content"         >*/
  PICMAN_UNDO_ITEM_LOCK_POSITION,       /*< desc="Lock/Unlock position"        >*/
  PICMAN_UNDO_LAYER_ADD,                /*< desc="New layer"                   >*/
  PICMAN_UNDO_LAYER_REMOVE,             /*< desc="Delete layer"                >*/
  PICMAN_UNDO_LAYER_MODE,               /*< desc="Set layer mode"              >*/
  PICMAN_UNDO_LAYER_OPACITY,            /*< desc="Set layer opacity"           >*/
  PICMAN_UNDO_LAYER_LOCK_ALPHA,         /*< desc="Lock/Unlock alpha channel"   >*/
  PICMAN_UNDO_GROUP_LAYER_SUSPEND,      /*< desc="Suspend group layer resize"  >*/
  PICMAN_UNDO_GROUP_LAYER_RESUME,       /*< desc="Resume group layer resize"   >*/
  PICMAN_UNDO_GROUP_LAYER_CONVERT,      /*< desc="Convert group layer"         >*/
  PICMAN_UNDO_TEXT_LAYER,               /*< desc="Text layer"                  >*/
  PICMAN_UNDO_TEXT_LAYER_MODIFIED,      /*< desc="Text layer modification"     >*/
  PICMAN_UNDO_TEXT_LAYER_CONVERT,       /*< desc="Convert text layer"          >*/
  PICMAN_UNDO_LAYER_MASK_ADD,           /*< desc="Add layer mask"              >*/
  PICMAN_UNDO_LAYER_MASK_REMOVE,        /*< desc="Delete layer mask"           >*/
  PICMAN_UNDO_LAYER_MASK_APPLY,         /*< desc="Apply layer mask"            >*/
  PICMAN_UNDO_LAYER_MASK_SHOW,          /*< desc="Show layer mask"             >*/
  PICMAN_UNDO_CHANNEL_ADD,              /*< desc="New channel"                 >*/
  PICMAN_UNDO_CHANNEL_REMOVE,           /*< desc="Delete channel"              >*/
  PICMAN_UNDO_CHANNEL_COLOR,            /*< desc="Channel color"               >*/
  PICMAN_UNDO_VECTORS_ADD,              /*< desc="New path"                    >*/
  PICMAN_UNDO_VECTORS_REMOVE,           /*< desc="Delete path"                 >*/
  PICMAN_UNDO_VECTORS_MOD,              /*< desc="Path modification"           >*/
  PICMAN_UNDO_FS_TO_LAYER,              /*< desc="Floating selection to layer" >*/
  PICMAN_UNDO_TRANSFORM,                /*< desc="Transform"                   >*/
  PICMAN_UNDO_PAINT,                    /*< desc="Paint"                       >*/
  PICMAN_UNDO_INK,                      /*< desc="Ink"                         >*/
  PICMAN_UNDO_FOREGROUND_SELECT,        /*< desc="Select foreground"           >*/
  PICMAN_UNDO_PARASITE_ATTACH,          /*< desc="Attach parasite"             >*/
  PICMAN_UNDO_PARASITE_REMOVE,          /*< desc="Remove parasite"             >*/

  PICMAN_UNDO_CANT                      /*< desc="Not undoable"                >*/
} PicmanUndoType;


#define PICMAN_TYPE_DIRTY_MASK (picman_dirty_mask_get_type ())

GType picman_dirty_mask_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_DIRTY_NONE            = 0,

  PICMAN_DIRTY_IMAGE           = 1 << 0,
  PICMAN_DIRTY_IMAGE_SIZE      = 1 << 1,
  PICMAN_DIRTY_IMAGE_META      = 1 << 2,
  PICMAN_DIRTY_IMAGE_STRUCTURE = 1 << 3,
  PICMAN_DIRTY_ITEM            = 1 << 4,
  PICMAN_DIRTY_ITEM_META       = 1 << 5,
  PICMAN_DIRTY_DRAWABLE        = 1 << 6,
  PICMAN_DIRTY_VECTORS         = 1 << 7,
  PICMAN_DIRTY_SELECTION       = 1 << 8,
  PICMAN_DIRTY_ACTIVE_DRAWABLE = 1 << 9,

  PICMAN_DIRTY_ALL             = 0xffff
} PicmanDirtyMask;


#define PICMAN_TYPE_OFFSET_TYPE (picman_offset_type_get_type ())

GType picman_offset_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_OFFSET_BACKGROUND,
  PICMAN_OFFSET_TRANSPARENT
} PicmanOffsetType;


#define PICMAN_TYPE_GRADIENT_COLOR (picman_gradient_color_get_type ())

GType picman_gradient_color_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_GRADIENT_COLOR_FIXED,
  PICMAN_GRADIENT_COLOR_FOREGROUND,
  PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT,
  PICMAN_GRADIENT_COLOR_BACKGROUND,
  PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT
} PicmanGradientColor;


#define PICMAN_TYPE_GRADIENT_SEGMENT_TYPE (picman_gradient_segment_type_get_type ())

GType picman_gradient_segment_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_GRADIENT_SEGMENT_LINEAR,
  PICMAN_GRADIENT_SEGMENT_CURVED,
  PICMAN_GRADIENT_SEGMENT_SINE,
  PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING,
  PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING
} PicmanGradientSegmentType;


#define PICMAN_TYPE_GRADIENT_SEGMENT_COLOR (picman_gradient_segment_color_get_type ())

GType picman_gradient_segment_color_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_GRADIENT_SEGMENT_RGB,      /* normal RGB           */
  PICMAN_GRADIENT_SEGMENT_HSV_CCW,  /* counterclockwise hue */
  PICMAN_GRADIENT_SEGMENT_HSV_CW    /* clockwise hue        */
} PicmanGradientSegmentColor;


#define PICMAN_TYPE_MASK_APPLY_MODE (picman_mask_apply_mode_get_type ())

GType picman_mask_apply_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_MASK_APPLY,
  PICMAN_MASK_DISCARD
} PicmanMaskApplyMode;


#define PICMAN_TYPE_MERGE_TYPE (picman_merge_type_get_type ())

GType picman_merge_type_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_EXPAND_AS_NECESSARY,
  PICMAN_CLIP_TO_IMAGE,
  PICMAN_CLIP_TO_BOTTOM_LAYER,
  PICMAN_FLATTEN_IMAGE
} PicmanMergeType;


#define PICMAN_TYPE_SELECT_CRITERION (picman_select_criterion_get_type ())

GType picman_select_criterion_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_SELECT_CRITERION_COMPOSITE,  /*< desc="Composite"  >*/
  PICMAN_SELECT_CRITERION_R,          /*< desc="Red"        >*/
  PICMAN_SELECT_CRITERION_G,          /*< desc="Green"      >*/
  PICMAN_SELECT_CRITERION_B,          /*< desc="Blue"       >*/
  PICMAN_SELECT_CRITERION_H,          /*< desc="Hue"        >*/
  PICMAN_SELECT_CRITERION_S,          /*< desc="Saturation" >*/
  PICMAN_SELECT_CRITERION_V           /*< desc="Value"      >*/
} PicmanSelectCriterion;


#define PICMAN_TYPE_MESSAGE_SEVERITY (picman_message_severity_get_type ())

GType picman_message_severity_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_MESSAGE_INFO,     /*< desc="Message" >*/
  PICMAN_MESSAGE_WARNING,  /*< desc="Warning" >*/
  PICMAN_MESSAGE_ERROR     /*< desc="Error"   >*/
} PicmanMessageSeverity;


#define PICMAN_TYPE_COLOR_PROFILE_POLICY (picman_color_profile_policy_get_type ())

GType picman_color_profile_policy_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_COLOR_PROFILE_POLICY_ASK,    /*< desc="Ask what to do"           >*/
  PICMAN_COLOR_PROFILE_POLICY_KEEP,   /*< desc="Keep embedded profile"    >*/
  PICMAN_COLOR_PROFILE_POLICY_CONVERT /*< desc="Convert to RGB workspace" >*/
} PicmanColorProfilePolicy;


#define PICMAN_TYPE_DYNAMICS_OUTPUT_TYPE (picman_dynamics_output_type_get_type ())

GType picman_dynamics_output_type_get_type (void) G_GNUC_CONST;

typedef enum  /*< pdb-skip >*/
{
  PICMAN_DYNAMICS_OUTPUT_OPACITY,      /*< desc="Opacity"      >*/
  PICMAN_DYNAMICS_OUTPUT_SIZE,         /*< desc="Size"         >*/
  PICMAN_DYNAMICS_OUTPUT_ANGLE,        /*< desc="Angle"        >*/
  PICMAN_DYNAMICS_OUTPUT_COLOR,        /*< desc="Color"        >*/
  PICMAN_DYNAMICS_OUTPUT_HARDNESS,     /*< desc="Hardness"     >*/
  PICMAN_DYNAMICS_OUTPUT_FORCE,        /*< desc="Force"        >*/
  PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO, /*< desc="Aspect ratio" >*/
  PICMAN_DYNAMICS_OUTPUT_SPACING,      /*< desc="Spacing"      >*/
  PICMAN_DYNAMICS_OUTPUT_RATE,         /*< desc="Rate"         >*/
  PICMAN_DYNAMICS_OUTPUT_FLOW,         /*< desc="Flow"         >*/
  PICMAN_DYNAMICS_OUTPUT_JITTER,       /*< desc="Jitter"       >*/
} PicmanDynamicsOutputType;


/*
 * non-registered enums; register them if needed
 */


typedef enum  /*< pdb-skip, skip >*/
{
  PICMAN_CONTEXT_FIRST_PROP       =  2,

  PICMAN_CONTEXT_PROP_IMAGE       =  PICMAN_CONTEXT_FIRST_PROP,
  PICMAN_CONTEXT_PROP_DISPLAY     =  3,
  PICMAN_CONTEXT_PROP_TOOL        =  4,
  PICMAN_CONTEXT_PROP_PAINT_INFO  =  5,
  PICMAN_CONTEXT_PROP_FOREGROUND  =  6,
  PICMAN_CONTEXT_PROP_BACKGROUND  =  7,
  PICMAN_CONTEXT_PROP_OPACITY     =  8,
  PICMAN_CONTEXT_PROP_PAINT_MODE  =  9,
  PICMAN_CONTEXT_PROP_BRUSH       = 10,
  PICMAN_CONTEXT_PROP_DYNAMICS    = 11,
  PICMAN_CONTEXT_PROP_PATTERN     = 12,
  PICMAN_CONTEXT_PROP_GRADIENT    = 13,
  PICMAN_CONTEXT_PROP_PALETTE     = 14,
  PICMAN_CONTEXT_PROP_TOOL_PRESET = 15,
  PICMAN_CONTEXT_PROP_FONT        = 16,
  PICMAN_CONTEXT_PROP_BUFFER      = 17,
  PICMAN_CONTEXT_PROP_IMAGEFILE   = 18,
  PICMAN_CONTEXT_PROP_TEMPLATE    = 19,

  PICMAN_CONTEXT_LAST_PROP        = PICMAN_CONTEXT_PROP_TEMPLATE
} PicmanContextPropType;


typedef enum  /*< pdb-skip, skip >*/
{
  PICMAN_CONTEXT_IMAGE_MASK       = 1 <<  2,
  PICMAN_CONTEXT_DISPLAY_MASK     = 1 <<  3,
  PICMAN_CONTEXT_TOOL_MASK        = 1 <<  4,
  PICMAN_CONTEXT_PAINT_INFO_MASK  = 1 <<  5,
  PICMAN_CONTEXT_FOREGROUND_MASK  = 1 <<  6,
  PICMAN_CONTEXT_BACKGROUND_MASK  = 1 <<  7,
  PICMAN_CONTEXT_OPACITY_MASK     = 1 <<  8,
  PICMAN_CONTEXT_PAINT_MODE_MASK  = 1 <<  9,
  PICMAN_CONTEXT_BRUSH_MASK       = 1 << 10,
  PICMAN_CONTEXT_DYNAMICS_MASK    = 1 << 11,
  PICMAN_CONTEXT_PATTERN_MASK     = 1 << 12,
  PICMAN_CONTEXT_GRADIENT_MASK    = 1 << 13,
  PICMAN_CONTEXT_PALETTE_MASK     = 1 << 14,
  PICMAN_CONTEXT_TOOL_PRESET_MASK = 1 << 15,
  PICMAN_CONTEXT_FONT_MASK        = 1 << 16,
  PICMAN_CONTEXT_BUFFER_MASK      = 1 << 17,
  PICMAN_CONTEXT_IMAGEFILE_MASK   = 1 << 18,
  PICMAN_CONTEXT_TEMPLATE_MASK    = 1 << 19,

  /*  aliases  */
  PICMAN_CONTEXT_PAINT_PROPS_MASK = (PICMAN_CONTEXT_FOREGROUND_MASK |
                                   PICMAN_CONTEXT_BACKGROUND_MASK |
                                   PICMAN_CONTEXT_OPACITY_MASK    |
                                   PICMAN_CONTEXT_PAINT_MODE_MASK |
                                   PICMAN_CONTEXT_BRUSH_MASK      |
                                   PICMAN_CONTEXT_DYNAMICS_MASK   |
                                   PICMAN_CONTEXT_PATTERN_MASK    |
                                   PICMAN_CONTEXT_GRADIENT_MASK),
  PICMAN_CONTEXT_ALL_PROPS_MASK   = (PICMAN_CONTEXT_IMAGE_MASK      |
                                   PICMAN_CONTEXT_DISPLAY_MASK    |
                                   PICMAN_CONTEXT_TOOL_MASK       |
                                   PICMAN_CONTEXT_PAINT_INFO_MASK |
                                   PICMAN_CONTEXT_PALETTE_MASK    |
                                   PICMAN_CONTEXT_FONT_MASK       |
                                   PICMAN_CONTEXT_BUFFER_MASK     |
                                   PICMAN_CONTEXT_IMAGEFILE_MASK  |
                                   PICMAN_CONTEXT_TEMPLATE_MASK   |
                                   PICMAN_CONTEXT_PAINT_PROPS_MASK)
} PicmanContextPropMask;


typedef enum  /*< pdb-skip, skip >*/
{
  PICMAN_IMAGE_SCALE_OK,
  PICMAN_IMAGE_SCALE_TOO_SMALL,
  PICMAN_IMAGE_SCALE_TOO_BIG
} PicmanImageScaleCheckType;


typedef enum  /*< pdb-skip, skip >*/
{
  PICMAN_ITEM_TYPE_LAYERS   = 1 << 0,
  PICMAN_ITEM_TYPE_CHANNELS = 1 << 1,
  PICMAN_ITEM_TYPE_VECTORS  = 1 << 2,

  PICMAN_ITEM_TYPE_ALL      = (PICMAN_ITEM_TYPE_LAYERS   |
                             PICMAN_ITEM_TYPE_CHANNELS |
                             PICMAN_ITEM_TYPE_VECTORS)
} PicmanItemTypeMask;


#endif /* __CORE_ENUMS_H__ */
