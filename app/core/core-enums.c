
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "core-enums.h"
#include "picman-intl.h"

/* enumerations from "./core-enums.h" */
GType
picman_component_mask_get_type (void)
{
  static const GFlagsValue values[] =
  {
    { PICMAN_COMPONENT_RED, "PICMAN_COMPONENT_RED", "red" },
    { PICMAN_COMPONENT_GREEN, "PICMAN_COMPONENT_GREEN", "green" },
    { PICMAN_COMPONENT_BLUE, "PICMAN_COMPONENT_BLUE", "blue" },
    { PICMAN_COMPONENT_ALPHA, "PICMAN_COMPONENT_ALPHA", "alpha" },
    { PICMAN_COMPONENT_ALL, "PICMAN_COMPONENT_ALL", "all" },
    { 0, NULL, NULL }
  };

  static const PicmanFlagsDesc descs[] =
  {
    { PICMAN_COMPONENT_RED, "PICMAN_COMPONENT_RED", NULL },
    { PICMAN_COMPONENT_GREEN, "PICMAN_COMPONENT_GREEN", NULL },
    { PICMAN_COMPONENT_BLUE, "PICMAN_COMPONENT_BLUE", NULL },
    { PICMAN_COMPONENT_ALPHA, "PICMAN_COMPONENT_ALPHA", NULL },
    { PICMAN_COMPONENT_ALL, "PICMAN_COMPONENT_ALL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_flags_register_static ("PicmanComponentMask", values);
      picman_type_set_translation_context (type, "component-mask");
      picman_flags_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_container_policy_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CONTAINER_POLICY_STRONG, "PICMAN_CONTAINER_POLICY_STRONG", "strong" },
    { PICMAN_CONTAINER_POLICY_WEAK, "PICMAN_CONTAINER_POLICY_WEAK", "weak" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CONTAINER_POLICY_STRONG, "PICMAN_CONTAINER_POLICY_STRONG", NULL },
    { PICMAN_CONTAINER_POLICY_WEAK, "PICMAN_CONTAINER_POLICY_WEAK", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanContainerPolicy", values);
      picman_type_set_translation_context (type, "container-policy");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_convert_dither_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_NO_DITHER, "PICMAN_NO_DITHER", "no-dither" },
    { PICMAN_FS_DITHER, "PICMAN_FS_DITHER", "fs-dither" },
    { PICMAN_FSLOWBLEED_DITHER, "PICMAN_FSLOWBLEED_DITHER", "fslowbleed-dither" },
    { PICMAN_FIXED_DITHER, "PICMAN_FIXED_DITHER", "fixed-dither" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_NO_DITHER, NC_("convert-dither-type", "None"), NULL },
    { PICMAN_FS_DITHER, NC_("convert-dither-type", "Floyd-Steinberg (normal)"), NULL },
    { PICMAN_FSLOWBLEED_DITHER, NC_("convert-dither-type", "Floyd-Steinberg (reduced color bleeding)"), NULL },
    { PICMAN_FIXED_DITHER, NC_("convert-dither-type", "Positioned"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanConvertDitherType", values);
      picman_type_set_translation_context (type, "convert-dither-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_convert_palette_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_MAKE_PALETTE, "PICMAN_MAKE_PALETTE", "make-palette" },
    { PICMAN_WEB_PALETTE, "PICMAN_WEB_PALETTE", "web-palette" },
    { PICMAN_MONO_PALETTE, "PICMAN_MONO_PALETTE", "mono-palette" },
    { PICMAN_CUSTOM_PALETTE, "PICMAN_CUSTOM_PALETTE", "custom-palette" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_MAKE_PALETTE, NC_("convert-palette-type", "Generate optimum palette"), NULL },
    { PICMAN_WEB_PALETTE, NC_("convert-palette-type", "Use web-optimized palette"), NULL },
    { PICMAN_MONO_PALETTE, NC_("convert-palette-type", "Use black and white (1-bit) palette"), NULL },
    { PICMAN_CUSTOM_PALETTE, NC_("convert-palette-type", "Use custom palette"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanConvertPaletteType", values);
      picman_type_set_translation_context (type, "convert-palette-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_convolution_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_NORMAL_CONVOL, "PICMAN_NORMAL_CONVOL", "normal-convol" },
    { PICMAN_ABSOLUTE_CONVOL, "PICMAN_ABSOLUTE_CONVOL", "absolute-convol" },
    { PICMAN_NEGATIVE_CONVOL, "PICMAN_NEGATIVE_CONVOL", "negative-convol" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_NORMAL_CONVOL, "PICMAN_NORMAL_CONVOL", NULL },
    { PICMAN_ABSOLUTE_CONVOL, "PICMAN_ABSOLUTE_CONVOL", NULL },
    { PICMAN_NEGATIVE_CONVOL, "PICMAN_NEGATIVE_CONVOL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanConvolutionType", values);
      picman_type_set_translation_context (type, "convolution-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_curve_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CURVE_SMOOTH, "PICMAN_CURVE_SMOOTH", "smooth" },
    { PICMAN_CURVE_FREE, "PICMAN_CURVE_FREE", "free" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CURVE_SMOOTH, NC_("curve-type", "Smooth"), NULL },
    { PICMAN_CURVE_FREE, NC_("curve-type", "Freehand"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCurveType", values);
      picman_type_set_translation_context (type, "curve-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_gravity_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRAVITY_NONE, "PICMAN_GRAVITY_NONE", "none" },
    { PICMAN_GRAVITY_NORTH_WEST, "PICMAN_GRAVITY_NORTH_WEST", "north-west" },
    { PICMAN_GRAVITY_NORTH, "PICMAN_GRAVITY_NORTH", "north" },
    { PICMAN_GRAVITY_NORTH_EAST, "PICMAN_GRAVITY_NORTH_EAST", "north-east" },
    { PICMAN_GRAVITY_WEST, "PICMAN_GRAVITY_WEST", "west" },
    { PICMAN_GRAVITY_CENTER, "PICMAN_GRAVITY_CENTER", "center" },
    { PICMAN_GRAVITY_EAST, "PICMAN_GRAVITY_EAST", "east" },
    { PICMAN_GRAVITY_SOUTH_WEST, "PICMAN_GRAVITY_SOUTH_WEST", "south-west" },
    { PICMAN_GRAVITY_SOUTH, "PICMAN_GRAVITY_SOUTH", "south" },
    { PICMAN_GRAVITY_SOUTH_EAST, "PICMAN_GRAVITY_SOUTH_EAST", "south-east" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRAVITY_NONE, "PICMAN_GRAVITY_NONE", NULL },
    { PICMAN_GRAVITY_NORTH_WEST, "PICMAN_GRAVITY_NORTH_WEST", NULL },
    { PICMAN_GRAVITY_NORTH, "PICMAN_GRAVITY_NORTH", NULL },
    { PICMAN_GRAVITY_NORTH_EAST, "PICMAN_GRAVITY_NORTH_EAST", NULL },
    { PICMAN_GRAVITY_WEST, "PICMAN_GRAVITY_WEST", NULL },
    { PICMAN_GRAVITY_CENTER, "PICMAN_GRAVITY_CENTER", NULL },
    { PICMAN_GRAVITY_EAST, "PICMAN_GRAVITY_EAST", NULL },
    { PICMAN_GRAVITY_SOUTH_WEST, "PICMAN_GRAVITY_SOUTH_WEST", NULL },
    { PICMAN_GRAVITY_SOUTH, "PICMAN_GRAVITY_SOUTH", NULL },
    { PICMAN_GRAVITY_SOUTH_EAST, "PICMAN_GRAVITY_SOUTH_EAST", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGravityType", values);
      picman_type_set_translation_context (type, "gravity-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_histogram_channel_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HISTOGRAM_VALUE, "PICMAN_HISTOGRAM_VALUE", "value" },
    { PICMAN_HISTOGRAM_RED, "PICMAN_HISTOGRAM_RED", "red" },
    { PICMAN_HISTOGRAM_GREEN, "PICMAN_HISTOGRAM_GREEN", "green" },
    { PICMAN_HISTOGRAM_BLUE, "PICMAN_HISTOGRAM_BLUE", "blue" },
    { PICMAN_HISTOGRAM_ALPHA, "PICMAN_HISTOGRAM_ALPHA", "alpha" },
    { PICMAN_HISTOGRAM_RGB, "PICMAN_HISTOGRAM_RGB", "rgb" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HISTOGRAM_VALUE, NC_("histogram-channel", "Value"), NULL },
    { PICMAN_HISTOGRAM_RED, NC_("histogram-channel", "Red"), NULL },
    { PICMAN_HISTOGRAM_GREEN, NC_("histogram-channel", "Green"), NULL },
    { PICMAN_HISTOGRAM_BLUE, NC_("histogram-channel", "Blue"), NULL },
    { PICMAN_HISTOGRAM_ALPHA, NC_("histogram-channel", "Alpha"), NULL },
    { PICMAN_HISTOGRAM_RGB, NC_("histogram-channel", "RGB"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHistogramChannel", values);
      picman_type_set_translation_context (type, "histogram-channel");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_hue_range_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ALL_HUES, "PICMAN_ALL_HUES", "all-hues" },
    { PICMAN_RED_HUES, "PICMAN_RED_HUES", "red-hues" },
    { PICMAN_YELLOW_HUES, "PICMAN_YELLOW_HUES", "yellow-hues" },
    { PICMAN_GREEN_HUES, "PICMAN_GREEN_HUES", "green-hues" },
    { PICMAN_CYAN_HUES, "PICMAN_CYAN_HUES", "cyan-hues" },
    { PICMAN_BLUE_HUES, "PICMAN_BLUE_HUES", "blue-hues" },
    { PICMAN_MAGENTA_HUES, "PICMAN_MAGENTA_HUES", "magenta-hues" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ALL_HUES, "PICMAN_ALL_HUES", NULL },
    { PICMAN_RED_HUES, "PICMAN_RED_HUES", NULL },
    { PICMAN_YELLOW_HUES, "PICMAN_YELLOW_HUES", NULL },
    { PICMAN_GREEN_HUES, "PICMAN_GREEN_HUES", NULL },
    { PICMAN_CYAN_HUES, "PICMAN_CYAN_HUES", NULL },
    { PICMAN_BLUE_HUES, "PICMAN_BLUE_HUES", NULL },
    { PICMAN_MAGENTA_HUES, "PICMAN_MAGENTA_HUES", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHueRange", values);
      picman_type_set_translation_context (type, "hue-range");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_layer_mode_effects_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_NORMAL_MODE, "PICMAN_NORMAL_MODE", "normal-mode" },
    { PICMAN_DISSOLVE_MODE, "PICMAN_DISSOLVE_MODE", "dissolve-mode" },
    { PICMAN_BEHIND_MODE, "PICMAN_BEHIND_MODE", "behind-mode" },
    { PICMAN_MULTIPLY_MODE, "PICMAN_MULTIPLY_MODE", "multiply-mode" },
    { PICMAN_SCREEN_MODE, "PICMAN_SCREEN_MODE", "screen-mode" },
    { PICMAN_OVERLAY_MODE, "PICMAN_OVERLAY_MODE", "overlay-mode" },
    { PICMAN_DIFFERENCE_MODE, "PICMAN_DIFFERENCE_MODE", "difference-mode" },
    { PICMAN_ADDITION_MODE, "PICMAN_ADDITION_MODE", "addition-mode" },
    { PICMAN_SUBTRACT_MODE, "PICMAN_SUBTRACT_MODE", "subtract-mode" },
    { PICMAN_DARKEN_ONLY_MODE, "PICMAN_DARKEN_ONLY_MODE", "darken-only-mode" },
    { PICMAN_LIGHTEN_ONLY_MODE, "PICMAN_LIGHTEN_ONLY_MODE", "lighten-only-mode" },
    { PICMAN_HUE_MODE, "PICMAN_HUE_MODE", "hue-mode" },
    { PICMAN_SATURATION_MODE, "PICMAN_SATURATION_MODE", "saturation-mode" },
    { PICMAN_COLOR_MODE, "PICMAN_COLOR_MODE", "color-mode" },
    { PICMAN_VALUE_MODE, "PICMAN_VALUE_MODE", "value-mode" },
    { PICMAN_DIVIDE_MODE, "PICMAN_DIVIDE_MODE", "divide-mode" },
    { PICMAN_DODGE_MODE, "PICMAN_DODGE_MODE", "dodge-mode" },
    { PICMAN_BURN_MODE, "PICMAN_BURN_MODE", "burn-mode" },
    { PICMAN_HARDLIGHT_MODE, "PICMAN_HARDLIGHT_MODE", "hardlight-mode" },
    { PICMAN_SOFTLIGHT_MODE, "PICMAN_SOFTLIGHT_MODE", "softlight-mode" },
    { PICMAN_GRAIN_EXTRACT_MODE, "PICMAN_GRAIN_EXTRACT_MODE", "grain-extract-mode" },
    { PICMAN_GRAIN_MERGE_MODE, "PICMAN_GRAIN_MERGE_MODE", "grain-merge-mode" },
    { PICMAN_COLOR_ERASE_MODE, "PICMAN_COLOR_ERASE_MODE", "color-erase-mode" },
    { PICMAN_ERASE_MODE, "PICMAN_ERASE_MODE", "erase-mode" },
    { PICMAN_REPLACE_MODE, "PICMAN_REPLACE_MODE", "replace-mode" },
    { PICMAN_ANTI_ERASE_MODE, "PICMAN_ANTI_ERASE_MODE", "anti-erase-mode" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_NORMAL_MODE, NC_("layer-mode-effects", "Normal"), NULL },
    { PICMAN_DISSOLVE_MODE, NC_("layer-mode-effects", "Dissolve"), NULL },
    { PICMAN_BEHIND_MODE, NC_("layer-mode-effects", "Behind"), NULL },
    { PICMAN_MULTIPLY_MODE, NC_("layer-mode-effects", "Multiply"), NULL },
    { PICMAN_SCREEN_MODE, NC_("layer-mode-effects", "Screen"), NULL },
    { PICMAN_OVERLAY_MODE, NC_("layer-mode-effects", "Overlay"), NULL },
    { PICMAN_DIFFERENCE_MODE, NC_("layer-mode-effects", "Difference"), NULL },
    { PICMAN_ADDITION_MODE, NC_("layer-mode-effects", "Addition"), NULL },
    { PICMAN_SUBTRACT_MODE, NC_("layer-mode-effects", "Subtract"), NULL },
    { PICMAN_DARKEN_ONLY_MODE, NC_("layer-mode-effects", "Darken only"), NULL },
    { PICMAN_LIGHTEN_ONLY_MODE, NC_("layer-mode-effects", "Lighten only"), NULL },
    { PICMAN_HUE_MODE, NC_("layer-mode-effects", "Hue"), NULL },
    { PICMAN_SATURATION_MODE, NC_("layer-mode-effects", "Saturation"), NULL },
    { PICMAN_COLOR_MODE, NC_("layer-mode-effects", "Color"), NULL },
    { PICMAN_VALUE_MODE, NC_("layer-mode-effects", "Value"), NULL },
    { PICMAN_DIVIDE_MODE, NC_("layer-mode-effects", "Divide"), NULL },
    { PICMAN_DODGE_MODE, NC_("layer-mode-effects", "Dodge"), NULL },
    { PICMAN_BURN_MODE, NC_("layer-mode-effects", "Burn"), NULL },
    { PICMAN_HARDLIGHT_MODE, NC_("layer-mode-effects", "Hard light"), NULL },
    { PICMAN_SOFTLIGHT_MODE, NC_("layer-mode-effects", "Soft light"), NULL },
    { PICMAN_GRAIN_EXTRACT_MODE, NC_("layer-mode-effects", "Grain extract"), NULL },
    { PICMAN_GRAIN_MERGE_MODE, NC_("layer-mode-effects", "Grain merge"), NULL },
    { PICMAN_COLOR_ERASE_MODE, NC_("layer-mode-effects", "Color erase"), NULL },
    { PICMAN_ERASE_MODE, NC_("layer-mode-effects", "Erase"), NULL },
    { PICMAN_REPLACE_MODE, NC_("layer-mode-effects", "Replace"), NULL },
    { PICMAN_ANTI_ERASE_MODE, NC_("layer-mode-effects", "Anti erase"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanLayerModeEffects", values);
      picman_type_set_translation_context (type, "layer-mode-effects");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_alignment_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ALIGN_LEFT, "PICMAN_ALIGN_LEFT", "align-left" },
    { PICMAN_ALIGN_HCENTER, "PICMAN_ALIGN_HCENTER", "align-hcenter" },
    { PICMAN_ALIGN_RIGHT, "PICMAN_ALIGN_RIGHT", "align-right" },
    { PICMAN_ALIGN_TOP, "PICMAN_ALIGN_TOP", "align-top" },
    { PICMAN_ALIGN_VCENTER, "PICMAN_ALIGN_VCENTER", "align-vcenter" },
    { PICMAN_ALIGN_BOTTOM, "PICMAN_ALIGN_BOTTOM", "align-bottom" },
    { PICMAN_ARRANGE_LEFT, "PICMAN_ARRANGE_LEFT", "arrange-left" },
    { PICMAN_ARRANGE_HCENTER, "PICMAN_ARRANGE_HCENTER", "arrange-hcenter" },
    { PICMAN_ARRANGE_RIGHT, "PICMAN_ARRANGE_RIGHT", "arrange-right" },
    { PICMAN_ARRANGE_TOP, "PICMAN_ARRANGE_TOP", "arrange-top" },
    { PICMAN_ARRANGE_VCENTER, "PICMAN_ARRANGE_VCENTER", "arrange-vcenter" },
    { PICMAN_ARRANGE_BOTTOM, "PICMAN_ARRANGE_BOTTOM", "arrange-bottom" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ALIGN_LEFT, "PICMAN_ALIGN_LEFT", NULL },
    { PICMAN_ALIGN_HCENTER, "PICMAN_ALIGN_HCENTER", NULL },
    { PICMAN_ALIGN_RIGHT, "PICMAN_ALIGN_RIGHT", NULL },
    { PICMAN_ALIGN_TOP, "PICMAN_ALIGN_TOP", NULL },
    { PICMAN_ALIGN_VCENTER, "PICMAN_ALIGN_VCENTER", NULL },
    { PICMAN_ALIGN_BOTTOM, "PICMAN_ALIGN_BOTTOM", NULL },
    { PICMAN_ARRANGE_LEFT, "PICMAN_ARRANGE_LEFT", NULL },
    { PICMAN_ARRANGE_HCENTER, "PICMAN_ARRANGE_HCENTER", NULL },
    { PICMAN_ARRANGE_RIGHT, "PICMAN_ARRANGE_RIGHT", NULL },
    { PICMAN_ARRANGE_TOP, "PICMAN_ARRANGE_TOP", NULL },
    { PICMAN_ARRANGE_VCENTER, "PICMAN_ARRANGE_VCENTER", NULL },
    { PICMAN_ARRANGE_BOTTOM, "PICMAN_ARRANGE_BOTTOM", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanAlignmentType", values);
      picman_type_set_translation_context (type, "alignment-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_align_reference_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ALIGN_REFERENCE_FIRST, "PICMAN_ALIGN_REFERENCE_FIRST", "first" },
    { PICMAN_ALIGN_REFERENCE_IMAGE, "PICMAN_ALIGN_REFERENCE_IMAGE", "image" },
    { PICMAN_ALIGN_REFERENCE_SELECTION, "PICMAN_ALIGN_REFERENCE_SELECTION", "selection" },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_LAYER, "PICMAN_ALIGN_REFERENCE_ACTIVE_LAYER", "active-layer" },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_CHANNEL, "PICMAN_ALIGN_REFERENCE_ACTIVE_CHANNEL", "active-channel" },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_PATH, "PICMAN_ALIGN_REFERENCE_ACTIVE_PATH", "active-path" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ALIGN_REFERENCE_FIRST, NC_("align-reference-type", "First item"), NULL },
    { PICMAN_ALIGN_REFERENCE_IMAGE, NC_("align-reference-type", "Image"), NULL },
    { PICMAN_ALIGN_REFERENCE_SELECTION, NC_("align-reference-type", "Selection"), NULL },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_LAYER, NC_("align-reference-type", "Active layer"), NULL },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_CHANNEL, NC_("align-reference-type", "Active channel"), NULL },
    { PICMAN_ALIGN_REFERENCE_ACTIVE_PATH, NC_("align-reference-type", "Active path"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanAlignReferenceType", values);
      picman_type_set_translation_context (type, "align-reference-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_fill_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_FOREGROUND_FILL, "PICMAN_FOREGROUND_FILL", "foreground-fill" },
    { PICMAN_BACKGROUND_FILL, "PICMAN_BACKGROUND_FILL", "background-fill" },
    { PICMAN_WHITE_FILL, "PICMAN_WHITE_FILL", "white-fill" },
    { PICMAN_TRANSPARENT_FILL, "PICMAN_TRANSPARENT_FILL", "transparent-fill" },
    { PICMAN_PATTERN_FILL, "PICMAN_PATTERN_FILL", "pattern-fill" },
    { PICMAN_NO_FILL, "PICMAN_NO_FILL", "no-fill" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_FOREGROUND_FILL, NC_("fill-type", "Foreground color"), NULL },
    { PICMAN_BACKGROUND_FILL, NC_("fill-type", "Background color"), NULL },
    { PICMAN_WHITE_FILL, NC_("fill-type", "White"), NULL },
    { PICMAN_TRANSPARENT_FILL, NC_("fill-type", "Transparency"), NULL },
    { PICMAN_PATTERN_FILL, NC_("fill-type", "Pattern"), NULL },
    { PICMAN_NO_FILL, NC_("fill-type", "None"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanFillType", values);
      picman_type_set_translation_context (type, "fill-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_fill_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_FILL_STYLE_SOLID, "PICMAN_FILL_STYLE_SOLID", "solid" },
    { PICMAN_FILL_STYLE_PATTERN, "PICMAN_FILL_STYLE_PATTERN", "pattern" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_FILL_STYLE_SOLID, NC_("fill-style", "Solid color"), NULL },
    { PICMAN_FILL_STYLE_PATTERN, NC_("fill-style", "Pattern"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanFillStyle", values);
      picman_type_set_translation_context (type, "fill-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_stroke_method_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_STROKE_METHOD_LIBART, "PICMAN_STROKE_METHOD_LIBART", "libart" },
    { PICMAN_STROKE_METHOD_PAINT_CORE, "PICMAN_STROKE_METHOD_PAINT_CORE", "paint-core" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_STROKE_METHOD_LIBART, NC_("stroke-method", "Stroke line"), NULL },
    { PICMAN_STROKE_METHOD_PAINT_CORE, NC_("stroke-method", "Stroke with a paint tool"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanStrokeMethod", values);
      picman_type_set_translation_context (type, "stroke-method");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_join_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_JOIN_MITER, "PICMAN_JOIN_MITER", "miter" },
    { PICMAN_JOIN_ROUND, "PICMAN_JOIN_ROUND", "round" },
    { PICMAN_JOIN_BEVEL, "PICMAN_JOIN_BEVEL", "bevel" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_JOIN_MITER, NC_("join-style", "Miter"), NULL },
    { PICMAN_JOIN_ROUND, NC_("join-style", "Round"), NULL },
    { PICMAN_JOIN_BEVEL, NC_("join-style", "Bevel"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanJoinStyle", values);
      picman_type_set_translation_context (type, "join-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_cap_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CAP_BUTT, "PICMAN_CAP_BUTT", "butt" },
    { PICMAN_CAP_ROUND, "PICMAN_CAP_ROUND", "round" },
    { PICMAN_CAP_SQUARE, "PICMAN_CAP_SQUARE", "square" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CAP_BUTT, NC_("cap-style", "Butt"), NULL },
    { PICMAN_CAP_ROUND, NC_("cap-style", "Round"), NULL },
    { PICMAN_CAP_SQUARE, NC_("cap-style", "Square"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCapStyle", values);
      picman_type_set_translation_context (type, "cap-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_dash_preset_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_DASH_CUSTOM, "PICMAN_DASH_CUSTOM", "custom" },
    { PICMAN_DASH_LINE, "PICMAN_DASH_LINE", "line" },
    { PICMAN_DASH_LONG_DASH, "PICMAN_DASH_LONG_DASH", "long-dash" },
    { PICMAN_DASH_MEDIUM_DASH, "PICMAN_DASH_MEDIUM_DASH", "medium-dash" },
    { PICMAN_DASH_SHORT_DASH, "PICMAN_DASH_SHORT_DASH", "short-dash" },
    { PICMAN_DASH_SPARSE_DOTS, "PICMAN_DASH_SPARSE_DOTS", "sparse-dots" },
    { PICMAN_DASH_NORMAL_DOTS, "PICMAN_DASH_NORMAL_DOTS", "normal-dots" },
    { PICMAN_DASH_DENSE_DOTS, "PICMAN_DASH_DENSE_DOTS", "dense-dots" },
    { PICMAN_DASH_STIPPLES, "PICMAN_DASH_STIPPLES", "stipples" },
    { PICMAN_DASH_DASH_DOT, "PICMAN_DASH_DASH_DOT", "dash-dot" },
    { PICMAN_DASH_DASH_DOT_DOT, "PICMAN_DASH_DASH_DOT_DOT", "dash-dot-dot" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_DASH_CUSTOM, NC_("dash-preset", "Custom"), NULL },
    { PICMAN_DASH_LINE, NC_("dash-preset", "Line"), NULL },
    { PICMAN_DASH_LONG_DASH, NC_("dash-preset", "Long dashes"), NULL },
    { PICMAN_DASH_MEDIUM_DASH, NC_("dash-preset", "Medium dashes"), NULL },
    { PICMAN_DASH_SHORT_DASH, NC_("dash-preset", "Short dashes"), NULL },
    { PICMAN_DASH_SPARSE_DOTS, NC_("dash-preset", "Sparse dots"), NULL },
    { PICMAN_DASH_NORMAL_DOTS, NC_("dash-preset", "Normal dots"), NULL },
    { PICMAN_DASH_DENSE_DOTS, NC_("dash-preset", "Dense dots"), NULL },
    { PICMAN_DASH_STIPPLES, NC_("dash-preset", "Stipples"), NULL },
    { PICMAN_DASH_DASH_DOT, NC_("dash-preset", "Dash, dot"), NULL },
    { PICMAN_DASH_DASH_DOT_DOT, NC_("dash-preset", "Dash, dot, dot"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanDashPreset", values);
      picman_type_set_translation_context (type, "dash-preset");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_brush_generated_shape_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_BRUSH_GENERATED_CIRCLE, "PICMAN_BRUSH_GENERATED_CIRCLE", "circle" },
    { PICMAN_BRUSH_GENERATED_SQUARE, "PICMAN_BRUSH_GENERATED_SQUARE", "square" },
    { PICMAN_BRUSH_GENERATED_DIAMOND, "PICMAN_BRUSH_GENERATED_DIAMOND", "diamond" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_BRUSH_GENERATED_CIRCLE, NC_("brush-generated-shape", "Circle"), NULL },
    { PICMAN_BRUSH_GENERATED_SQUARE, NC_("brush-generated-shape", "Square"), NULL },
    { PICMAN_BRUSH_GENERATED_DIAMOND, NC_("brush-generated-shape", "Diamond"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanBrushGeneratedShape", values);
      picman_type_set_translation_context (type, "brush-generated-shape");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_orientation_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ORIENTATION_HORIZONTAL, "PICMAN_ORIENTATION_HORIZONTAL", "horizontal" },
    { PICMAN_ORIENTATION_VERTICAL, "PICMAN_ORIENTATION_VERTICAL", "vertical" },
    { PICMAN_ORIENTATION_UNKNOWN, "PICMAN_ORIENTATION_UNKNOWN", "unknown" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ORIENTATION_HORIZONTAL, NC_("orientation-type", "Horizontal"), NULL },
    { PICMAN_ORIENTATION_VERTICAL, NC_("orientation-type", "Vertical"), NULL },
    { PICMAN_ORIENTATION_UNKNOWN, NC_("orientation-type", "Unknown"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanOrientationType", values);
      picman_type_set_translation_context (type, "orientation-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_precision_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PRECISION_U8, "PICMAN_PRECISION_U8", "u8" },
    { PICMAN_PRECISION_U16, "PICMAN_PRECISION_U16", "u16" },
    { PICMAN_PRECISION_U32, "PICMAN_PRECISION_U32", "u32" },
    { PICMAN_PRECISION_HALF, "PICMAN_PRECISION_HALF", "half" },
    { PICMAN_PRECISION_FLOAT, "PICMAN_PRECISION_FLOAT", "float" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PRECISION_U8, NC_("precision", "8-bit integer"), NULL },
    { PICMAN_PRECISION_U16, NC_("precision", "16-bit integer"), NULL },
    { PICMAN_PRECISION_U32, NC_("precision", "32-bit integer"), NULL },
    { PICMAN_PRECISION_HALF, NC_("precision", "16-bit floating point"), NULL },
    { PICMAN_PRECISION_FLOAT, NC_("precision", "32-bit floating point"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPrecision", values);
      picman_type_set_translation_context (type, "precision");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_item_set_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ITEM_SET_NONE, "PICMAN_ITEM_SET_NONE", "none" },
    { PICMAN_ITEM_SET_ALL, "PICMAN_ITEM_SET_ALL", "all" },
    { PICMAN_ITEM_SET_IMAGE_SIZED, "PICMAN_ITEM_SET_IMAGE_SIZED", "image-sized" },
    { PICMAN_ITEM_SET_VISIBLE, "PICMAN_ITEM_SET_VISIBLE", "visible" },
    { PICMAN_ITEM_SET_LINKED, "PICMAN_ITEM_SET_LINKED", "linked" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ITEM_SET_NONE, NC_("item-set", "None"), NULL },
    { PICMAN_ITEM_SET_ALL, NC_("item-set", "All layers"), NULL },
    { PICMAN_ITEM_SET_IMAGE_SIZED, NC_("item-set", "Image-sized layers"), NULL },
    { PICMAN_ITEM_SET_VISIBLE, NC_("item-set", "All visible layers"), NULL },
    { PICMAN_ITEM_SET_LINKED, NC_("item-set", "All linked layers"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanItemSet", values);
      picman_type_set_translation_context (type, "item-set");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_rotation_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ROTATE_90, "PICMAN_ROTATE_90", "90" },
    { PICMAN_ROTATE_180, "PICMAN_ROTATE_180", "180" },
    { PICMAN_ROTATE_270, "PICMAN_ROTATE_270", "270" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ROTATE_90, "PICMAN_ROTATE_90", NULL },
    { PICMAN_ROTATE_180, "PICMAN_ROTATE_180", NULL },
    { PICMAN_ROTATE_270, "PICMAN_ROTATE_270", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRotationType", values);
      picman_type_set_translation_context (type, "rotation-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_view_size_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_VIEW_SIZE_TINY, "PICMAN_VIEW_SIZE_TINY", "tiny" },
    { PICMAN_VIEW_SIZE_EXTRA_SMALL, "PICMAN_VIEW_SIZE_EXTRA_SMALL", "extra-small" },
    { PICMAN_VIEW_SIZE_SMALL, "PICMAN_VIEW_SIZE_SMALL", "small" },
    { PICMAN_VIEW_SIZE_MEDIUM, "PICMAN_VIEW_SIZE_MEDIUM", "medium" },
    { PICMAN_VIEW_SIZE_LARGE, "PICMAN_VIEW_SIZE_LARGE", "large" },
    { PICMAN_VIEW_SIZE_EXTRA_LARGE, "PICMAN_VIEW_SIZE_EXTRA_LARGE", "extra-large" },
    { PICMAN_VIEW_SIZE_HUGE, "PICMAN_VIEW_SIZE_HUGE", "huge" },
    { PICMAN_VIEW_SIZE_ENORMOUS, "PICMAN_VIEW_SIZE_ENORMOUS", "enormous" },
    { PICMAN_VIEW_SIZE_GIGANTIC, "PICMAN_VIEW_SIZE_GIGANTIC", "gigantic" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_VIEW_SIZE_TINY, NC_("view-size", "Tiny"), NULL },
    { PICMAN_VIEW_SIZE_EXTRA_SMALL, NC_("view-size", "Very small"), NULL },
    { PICMAN_VIEW_SIZE_SMALL, NC_("view-size", "Small"), NULL },
    { PICMAN_VIEW_SIZE_MEDIUM, NC_("view-size", "Medium"), NULL },
    { PICMAN_VIEW_SIZE_LARGE, NC_("view-size", "Large"), NULL },
    { PICMAN_VIEW_SIZE_EXTRA_LARGE, NC_("view-size", "Very large"), NULL },
    { PICMAN_VIEW_SIZE_HUGE, NC_("view-size", "Huge"), NULL },
    { PICMAN_VIEW_SIZE_ENORMOUS, NC_("view-size", "Enormous"), NULL },
    { PICMAN_VIEW_SIZE_GIGANTIC, NC_("view-size", "Gigantic"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanViewSize", values);
      picman_type_set_translation_context (type, "view-size");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_view_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_VIEW_TYPE_LIST, "PICMAN_VIEW_TYPE_LIST", "list" },
    { PICMAN_VIEW_TYPE_GRID, "PICMAN_VIEW_TYPE_GRID", "grid" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_VIEW_TYPE_LIST, NC_("view-type", "View as list"), NULL },
    { PICMAN_VIEW_TYPE_GRID, NC_("view-type", "View as grid"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanViewType", values);
      picman_type_set_translation_context (type, "view-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_thumbnail_size_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_THUMBNAIL_SIZE_NONE, "PICMAN_THUMBNAIL_SIZE_NONE", "none" },
    { PICMAN_THUMBNAIL_SIZE_NORMAL, "PICMAN_THUMBNAIL_SIZE_NORMAL", "normal" },
    { PICMAN_THUMBNAIL_SIZE_LARGE, "PICMAN_THUMBNAIL_SIZE_LARGE", "large" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_THUMBNAIL_SIZE_NONE, NC_("thumbnail-size", "No thumbnails"), NULL },
    { PICMAN_THUMBNAIL_SIZE_NORMAL, NC_("thumbnail-size", "Normal (128x128)"), NULL },
    { PICMAN_THUMBNAIL_SIZE_LARGE, NC_("thumbnail-size", "Large (256x256)"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanThumbnailSize", values);
      picman_type_set_translation_context (type, "thumbnail-size");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_undo_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_UNDO_MODE_UNDO, "PICMAN_UNDO_MODE_UNDO", "undo" },
    { PICMAN_UNDO_MODE_REDO, "PICMAN_UNDO_MODE_REDO", "redo" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_UNDO_MODE_UNDO, "PICMAN_UNDO_MODE_UNDO", NULL },
    { PICMAN_UNDO_MODE_REDO, "PICMAN_UNDO_MODE_REDO", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanUndoMode", values);
      picman_type_set_translation_context (type, "undo-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_undo_event_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_UNDO_EVENT_UNDO_PUSHED, "PICMAN_UNDO_EVENT_UNDO_PUSHED", "undo-pushed" },
    { PICMAN_UNDO_EVENT_UNDO_EXPIRED, "PICMAN_UNDO_EVENT_UNDO_EXPIRED", "undo-expired" },
    { PICMAN_UNDO_EVENT_REDO_EXPIRED, "PICMAN_UNDO_EVENT_REDO_EXPIRED", "redo-expired" },
    { PICMAN_UNDO_EVENT_UNDO, "PICMAN_UNDO_EVENT_UNDO", "undo" },
    { PICMAN_UNDO_EVENT_REDO, "PICMAN_UNDO_EVENT_REDO", "redo" },
    { PICMAN_UNDO_EVENT_UNDO_FREE, "PICMAN_UNDO_EVENT_UNDO_FREE", "undo-free" },
    { PICMAN_UNDO_EVENT_UNDO_FREEZE, "PICMAN_UNDO_EVENT_UNDO_FREEZE", "undo-freeze" },
    { PICMAN_UNDO_EVENT_UNDO_THAW, "PICMAN_UNDO_EVENT_UNDO_THAW", "undo-thaw" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_UNDO_EVENT_UNDO_PUSHED, "PICMAN_UNDO_EVENT_UNDO_PUSHED", NULL },
    { PICMAN_UNDO_EVENT_UNDO_EXPIRED, "PICMAN_UNDO_EVENT_UNDO_EXPIRED", NULL },
    { PICMAN_UNDO_EVENT_REDO_EXPIRED, "PICMAN_UNDO_EVENT_REDO_EXPIRED", NULL },
    { PICMAN_UNDO_EVENT_UNDO, "PICMAN_UNDO_EVENT_UNDO", NULL },
    { PICMAN_UNDO_EVENT_REDO, "PICMAN_UNDO_EVENT_REDO", NULL },
    { PICMAN_UNDO_EVENT_UNDO_FREE, "PICMAN_UNDO_EVENT_UNDO_FREE", NULL },
    { PICMAN_UNDO_EVENT_UNDO_FREEZE, "PICMAN_UNDO_EVENT_UNDO_FREEZE", NULL },
    { PICMAN_UNDO_EVENT_UNDO_THAW, "PICMAN_UNDO_EVENT_UNDO_THAW", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanUndoEvent", values);
      picman_type_set_translation_context (type, "undo-event");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_undo_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_UNDO_GROUP_NONE, "PICMAN_UNDO_GROUP_NONE", "group-none" },
    { PICMAN_UNDO_GROUP_IMAGE_SCALE, "PICMAN_UNDO_GROUP_IMAGE_SCALE", "group-image-scale" },
    { PICMAN_UNDO_GROUP_IMAGE_RESIZE, "PICMAN_UNDO_GROUP_IMAGE_RESIZE", "group-image-resize" },
    { PICMAN_UNDO_GROUP_IMAGE_FLIP, "PICMAN_UNDO_GROUP_IMAGE_FLIP", "group-image-flip" },
    { PICMAN_UNDO_GROUP_IMAGE_ROTATE, "PICMAN_UNDO_GROUP_IMAGE_ROTATE", "group-image-rotate" },
    { PICMAN_UNDO_GROUP_IMAGE_CROP, "PICMAN_UNDO_GROUP_IMAGE_CROP", "group-image-crop" },
    { PICMAN_UNDO_GROUP_IMAGE_CONVERT, "PICMAN_UNDO_GROUP_IMAGE_CONVERT", "group-image-convert" },
    { PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE, "PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE", "group-image-item-remove" },
    { PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE, "PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE", "group-image-layers-merge" },
    { PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE, "PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE", "group-image-vectors-merge" },
    { PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK, "PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK", "group-image-quick-mask" },
    { PICMAN_UNDO_GROUP_IMAGE_GRID, "PICMAN_UNDO_GROUP_IMAGE_GRID", "group-image-grid" },
    { PICMAN_UNDO_GROUP_GUIDE, "PICMAN_UNDO_GROUP_GUIDE", "group-guide" },
    { PICMAN_UNDO_GROUP_SAMPLE_POINT, "PICMAN_UNDO_GROUP_SAMPLE_POINT", "group-sample-point" },
    { PICMAN_UNDO_GROUP_DRAWABLE, "PICMAN_UNDO_GROUP_DRAWABLE", "group-drawable" },
    { PICMAN_UNDO_GROUP_DRAWABLE_MOD, "PICMAN_UNDO_GROUP_DRAWABLE_MOD", "group-drawable-mod" },
    { PICMAN_UNDO_GROUP_MASK, "PICMAN_UNDO_GROUP_MASK", "group-mask" },
    { PICMAN_UNDO_GROUP_ITEM_VISIBILITY, "PICMAN_UNDO_GROUP_ITEM_VISIBILITY", "group-item-visibility" },
    { PICMAN_UNDO_GROUP_ITEM_LINKED, "PICMAN_UNDO_GROUP_ITEM_LINKED", "group-item-linked" },
    { PICMAN_UNDO_GROUP_ITEM_PROPERTIES, "PICMAN_UNDO_GROUP_ITEM_PROPERTIES", "group-item-properties" },
    { PICMAN_UNDO_GROUP_ITEM_DISPLACE, "PICMAN_UNDO_GROUP_ITEM_DISPLACE", "group-item-displace" },
    { PICMAN_UNDO_GROUP_ITEM_SCALE, "PICMAN_UNDO_GROUP_ITEM_SCALE", "group-item-scale" },
    { PICMAN_UNDO_GROUP_ITEM_RESIZE, "PICMAN_UNDO_GROUP_ITEM_RESIZE", "group-item-resize" },
    { PICMAN_UNDO_GROUP_LAYER_ADD, "PICMAN_UNDO_GROUP_LAYER_ADD", "group-layer-add" },
    { PICMAN_UNDO_GROUP_LAYER_ADD_MASK, "PICMAN_UNDO_GROUP_LAYER_ADD_MASK", "group-layer-add-mask" },
    { PICMAN_UNDO_GROUP_LAYER_APPLY_MASK, "PICMAN_UNDO_GROUP_LAYER_APPLY_MASK", "group-layer-apply-mask" },
    { PICMAN_UNDO_GROUP_FS_TO_LAYER, "PICMAN_UNDO_GROUP_FS_TO_LAYER", "group-fs-to-layer" },
    { PICMAN_UNDO_GROUP_FS_FLOAT, "PICMAN_UNDO_GROUP_FS_FLOAT", "group-fs-float" },
    { PICMAN_UNDO_GROUP_FS_ANCHOR, "PICMAN_UNDO_GROUP_FS_ANCHOR", "group-fs-anchor" },
    { PICMAN_UNDO_GROUP_EDIT_PASTE, "PICMAN_UNDO_GROUP_EDIT_PASTE", "group-edit-paste" },
    { PICMAN_UNDO_GROUP_EDIT_CUT, "PICMAN_UNDO_GROUP_EDIT_CUT", "group-edit-cut" },
    { PICMAN_UNDO_GROUP_TEXT, "PICMAN_UNDO_GROUP_TEXT", "group-text" },
    { PICMAN_UNDO_GROUP_TRANSFORM, "PICMAN_UNDO_GROUP_TRANSFORM", "group-transform" },
    { PICMAN_UNDO_GROUP_PAINT, "PICMAN_UNDO_GROUP_PAINT", "group-paint" },
    { PICMAN_UNDO_GROUP_PARASITE_ATTACH, "PICMAN_UNDO_GROUP_PARASITE_ATTACH", "group-parasite-attach" },
    { PICMAN_UNDO_GROUP_PARASITE_REMOVE, "PICMAN_UNDO_GROUP_PARASITE_REMOVE", "group-parasite-remove" },
    { PICMAN_UNDO_GROUP_VECTORS_IMPORT, "PICMAN_UNDO_GROUP_VECTORS_IMPORT", "group-vectors-import" },
    { PICMAN_UNDO_GROUP_MISC, "PICMAN_UNDO_GROUP_MISC", "group-misc" },
    { PICMAN_UNDO_IMAGE_TYPE, "PICMAN_UNDO_IMAGE_TYPE", "image-type" },
    { PICMAN_UNDO_IMAGE_PRECISION, "PICMAN_UNDO_IMAGE_PRECISION", "image-precision" },
    { PICMAN_UNDO_IMAGE_SIZE, "PICMAN_UNDO_IMAGE_SIZE", "image-size" },
    { PICMAN_UNDO_IMAGE_RESOLUTION, "PICMAN_UNDO_IMAGE_RESOLUTION", "image-resolution" },
    { PICMAN_UNDO_IMAGE_GRID, "PICMAN_UNDO_IMAGE_GRID", "image-grid" },
    { PICMAN_UNDO_IMAGE_COLORMAP, "PICMAN_UNDO_IMAGE_COLORMAP", "image-colormap" },
    { PICMAN_UNDO_GUIDE, "PICMAN_UNDO_GUIDE", "guide" },
    { PICMAN_UNDO_SAMPLE_POINT, "PICMAN_UNDO_SAMPLE_POINT", "sample-point" },
    { PICMAN_UNDO_DRAWABLE, "PICMAN_UNDO_DRAWABLE", "drawable" },
    { PICMAN_UNDO_DRAWABLE_MOD, "PICMAN_UNDO_DRAWABLE_MOD", "drawable-mod" },
    { PICMAN_UNDO_MASK, "PICMAN_UNDO_MASK", "mask" },
    { PICMAN_UNDO_ITEM_REORDER, "PICMAN_UNDO_ITEM_REORDER", "item-reorder" },
    { PICMAN_UNDO_ITEM_RENAME, "PICMAN_UNDO_ITEM_RENAME", "item-rename" },
    { PICMAN_UNDO_ITEM_DISPLACE, "PICMAN_UNDO_ITEM_DISPLACE", "item-displace" },
    { PICMAN_UNDO_ITEM_VISIBILITY, "PICMAN_UNDO_ITEM_VISIBILITY", "item-visibility" },
    { PICMAN_UNDO_ITEM_LINKED, "PICMAN_UNDO_ITEM_LINKED", "item-linked" },
    { PICMAN_UNDO_ITEM_LOCK_CONTENT, "PICMAN_UNDO_ITEM_LOCK_CONTENT", "item-lock-content" },
    { PICMAN_UNDO_ITEM_LOCK_POSITION, "PICMAN_UNDO_ITEM_LOCK_POSITION", "item-lock-position" },
    { PICMAN_UNDO_LAYER_ADD, "PICMAN_UNDO_LAYER_ADD", "layer-add" },
    { PICMAN_UNDO_LAYER_REMOVE, "PICMAN_UNDO_LAYER_REMOVE", "layer-remove" },
    { PICMAN_UNDO_LAYER_MODE, "PICMAN_UNDO_LAYER_MODE", "layer-mode" },
    { PICMAN_UNDO_LAYER_OPACITY, "PICMAN_UNDO_LAYER_OPACITY", "layer-opacity" },
    { PICMAN_UNDO_LAYER_LOCK_ALPHA, "PICMAN_UNDO_LAYER_LOCK_ALPHA", "layer-lock-alpha" },
    { PICMAN_UNDO_GROUP_LAYER_SUSPEND, "PICMAN_UNDO_GROUP_LAYER_SUSPEND", "group-layer-suspend" },
    { PICMAN_UNDO_GROUP_LAYER_RESUME, "PICMAN_UNDO_GROUP_LAYER_RESUME", "group-layer-resume" },
    { PICMAN_UNDO_GROUP_LAYER_CONVERT, "PICMAN_UNDO_GROUP_LAYER_CONVERT", "group-layer-convert" },
    { PICMAN_UNDO_TEXT_LAYER, "PICMAN_UNDO_TEXT_LAYER", "text-layer" },
    { PICMAN_UNDO_TEXT_LAYER_MODIFIED, "PICMAN_UNDO_TEXT_LAYER_MODIFIED", "text-layer-modified" },
    { PICMAN_UNDO_TEXT_LAYER_CONVERT, "PICMAN_UNDO_TEXT_LAYER_CONVERT", "text-layer-convert" },
    { PICMAN_UNDO_LAYER_MASK_ADD, "PICMAN_UNDO_LAYER_MASK_ADD", "layer-mask-add" },
    { PICMAN_UNDO_LAYER_MASK_REMOVE, "PICMAN_UNDO_LAYER_MASK_REMOVE", "layer-mask-remove" },
    { PICMAN_UNDO_LAYER_MASK_APPLY, "PICMAN_UNDO_LAYER_MASK_APPLY", "layer-mask-apply" },
    { PICMAN_UNDO_LAYER_MASK_SHOW, "PICMAN_UNDO_LAYER_MASK_SHOW", "layer-mask-show" },
    { PICMAN_UNDO_CHANNEL_ADD, "PICMAN_UNDO_CHANNEL_ADD", "channel-add" },
    { PICMAN_UNDO_CHANNEL_REMOVE, "PICMAN_UNDO_CHANNEL_REMOVE", "channel-remove" },
    { PICMAN_UNDO_CHANNEL_COLOR, "PICMAN_UNDO_CHANNEL_COLOR", "channel-color" },
    { PICMAN_UNDO_VECTORS_ADD, "PICMAN_UNDO_VECTORS_ADD", "vectors-add" },
    { PICMAN_UNDO_VECTORS_REMOVE, "PICMAN_UNDO_VECTORS_REMOVE", "vectors-remove" },
    { PICMAN_UNDO_VECTORS_MOD, "PICMAN_UNDO_VECTORS_MOD", "vectors-mod" },
    { PICMAN_UNDO_FS_TO_LAYER, "PICMAN_UNDO_FS_TO_LAYER", "fs-to-layer" },
    { PICMAN_UNDO_TRANSFORM, "PICMAN_UNDO_TRANSFORM", "transform" },
    { PICMAN_UNDO_PAINT, "PICMAN_UNDO_PAINT", "paint" },
    { PICMAN_UNDO_INK, "PICMAN_UNDO_INK", "ink" },
    { PICMAN_UNDO_FOREGROUND_SELECT, "PICMAN_UNDO_FOREGROUND_SELECT", "foreground-select" },
    { PICMAN_UNDO_PARASITE_ATTACH, "PICMAN_UNDO_PARASITE_ATTACH", "parasite-attach" },
    { PICMAN_UNDO_PARASITE_REMOVE, "PICMAN_UNDO_PARASITE_REMOVE", "parasite-remove" },
    { PICMAN_UNDO_CANT, "PICMAN_UNDO_CANT", "cant" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_UNDO_GROUP_NONE, NC_("undo-type", "<<invalid>>"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_SCALE, NC_("undo-type", "Scale image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_RESIZE, NC_("undo-type", "Resize image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_FLIP, NC_("undo-type", "Flip image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_ROTATE, NC_("undo-type", "Rotate image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_CROP, NC_("undo-type", "Crop image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_CONVERT, NC_("undo-type", "Convert image"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_ITEM_REMOVE, NC_("undo-type", "Remove item"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE, NC_("undo-type", "Merge layers"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE, NC_("undo-type", "Merge paths"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK, NC_("undo-type", "Quick Mask"), NULL },
    { PICMAN_UNDO_GROUP_IMAGE_GRID, NC_("undo-type", "Grid"), NULL },
    { PICMAN_UNDO_GROUP_GUIDE, NC_("undo-type", "Guide"), NULL },
    { PICMAN_UNDO_GROUP_SAMPLE_POINT, NC_("undo-type", "Sample Point"), NULL },
    { PICMAN_UNDO_GROUP_DRAWABLE, NC_("undo-type", "Layer/Channel"), NULL },
    { PICMAN_UNDO_GROUP_DRAWABLE_MOD, NC_("undo-type", "Layer/Channel modification"), NULL },
    { PICMAN_UNDO_GROUP_MASK, NC_("undo-type", "Selection mask"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_VISIBILITY, NC_("undo-type", "Item visibility"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_LINKED, NC_("undo-type", "Link/Unlink item"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_PROPERTIES, NC_("undo-type", "Item properties"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_DISPLACE, NC_("undo-type", "Move item"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_SCALE, NC_("undo-type", "Scale item"), NULL },
    { PICMAN_UNDO_GROUP_ITEM_RESIZE, NC_("undo-type", "Resize item"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_ADD, NC_("undo-type", "Add layer"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_ADD_MASK, NC_("undo-type", "Add layer mask"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_APPLY_MASK, NC_("undo-type", "Apply layer mask"), NULL },
    { PICMAN_UNDO_GROUP_FS_TO_LAYER, NC_("undo-type", "Floating selection to layer"), NULL },
    { PICMAN_UNDO_GROUP_FS_FLOAT, NC_("undo-type", "Float selection"), NULL },
    { PICMAN_UNDO_GROUP_FS_ANCHOR, NC_("undo-type", "Anchor floating selection"), NULL },
    { PICMAN_UNDO_GROUP_EDIT_PASTE, NC_("undo-type", "Paste"), NULL },
    { PICMAN_UNDO_GROUP_EDIT_CUT, NC_("undo-type", "Cut"), NULL },
    { PICMAN_UNDO_GROUP_TEXT, NC_("undo-type", "Text"), NULL },
    { PICMAN_UNDO_GROUP_TRANSFORM, NC_("undo-type", "Transform"), NULL },
    { PICMAN_UNDO_GROUP_PAINT, NC_("undo-type", "Paint"), NULL },
    { PICMAN_UNDO_GROUP_PARASITE_ATTACH, NC_("undo-type", "Attach parasite"), NULL },
    { PICMAN_UNDO_GROUP_PARASITE_REMOVE, NC_("undo-type", "Remove parasite"), NULL },
    { PICMAN_UNDO_GROUP_VECTORS_IMPORT, NC_("undo-type", "Import paths"), NULL },
    { PICMAN_UNDO_GROUP_MISC, NC_("undo-type", "Plug-In"), NULL },
    { PICMAN_UNDO_IMAGE_TYPE, NC_("undo-type", "Image type"), NULL },
    { PICMAN_UNDO_IMAGE_PRECISION, NC_("undo-type", "Image precision"), NULL },
    { PICMAN_UNDO_IMAGE_SIZE, NC_("undo-type", "Image size"), NULL },
    { PICMAN_UNDO_IMAGE_RESOLUTION, NC_("undo-type", "Image resolution change"), NULL },
    { PICMAN_UNDO_IMAGE_GRID, NC_("undo-type", "Grid"), NULL },
    { PICMAN_UNDO_IMAGE_COLORMAP, NC_("undo-type", "Change indexed palette"), NULL },
    { PICMAN_UNDO_GUIDE, NC_("undo-type", "Guide"), NULL },
    { PICMAN_UNDO_SAMPLE_POINT, NC_("undo-type", "Sample Point"), NULL },
    { PICMAN_UNDO_DRAWABLE, NC_("undo-type", "Layer/Channel"), NULL },
    { PICMAN_UNDO_DRAWABLE_MOD, NC_("undo-type", "Layer/Channel modification"), NULL },
    { PICMAN_UNDO_MASK, NC_("undo-type", "Selection mask"), NULL },
    { PICMAN_UNDO_ITEM_REORDER, NC_("undo-type", "Reorder item"), NULL },
    { PICMAN_UNDO_ITEM_RENAME, NC_("undo-type", "Rename item"), NULL },
    { PICMAN_UNDO_ITEM_DISPLACE, NC_("undo-type", "Move item"), NULL },
    { PICMAN_UNDO_ITEM_VISIBILITY, NC_("undo-type", "Item visibility"), NULL },
    { PICMAN_UNDO_ITEM_LINKED, NC_("undo-type", "Link/Unlink item"), NULL },
    { PICMAN_UNDO_ITEM_LOCK_CONTENT, NC_("undo-type", "Lock/Unlock content"), NULL },
    { PICMAN_UNDO_ITEM_LOCK_POSITION, NC_("undo-type", "Lock/Unlock position"), NULL },
    { PICMAN_UNDO_LAYER_ADD, NC_("undo-type", "New layer"), NULL },
    { PICMAN_UNDO_LAYER_REMOVE, NC_("undo-type", "Delete layer"), NULL },
    { PICMAN_UNDO_LAYER_MODE, NC_("undo-type", "Set layer mode"), NULL },
    { PICMAN_UNDO_LAYER_OPACITY, NC_("undo-type", "Set layer opacity"), NULL },
    { PICMAN_UNDO_LAYER_LOCK_ALPHA, NC_("undo-type", "Lock/Unlock alpha channel"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_SUSPEND, NC_("undo-type", "Suspend group layer resize"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_RESUME, NC_("undo-type", "Resume group layer resize"), NULL },
    { PICMAN_UNDO_GROUP_LAYER_CONVERT, NC_("undo-type", "Convert group layer"), NULL },
    { PICMAN_UNDO_TEXT_LAYER, NC_("undo-type", "Text layer"), NULL },
    { PICMAN_UNDO_TEXT_LAYER_MODIFIED, NC_("undo-type", "Text layer modification"), NULL },
    { PICMAN_UNDO_TEXT_LAYER_CONVERT, NC_("undo-type", "Convert text layer"), NULL },
    { PICMAN_UNDO_LAYER_MASK_ADD, NC_("undo-type", "Add layer mask"), NULL },
    { PICMAN_UNDO_LAYER_MASK_REMOVE, NC_("undo-type", "Delete layer mask"), NULL },
    { PICMAN_UNDO_LAYER_MASK_APPLY, NC_("undo-type", "Apply layer mask"), NULL },
    { PICMAN_UNDO_LAYER_MASK_SHOW, NC_("undo-type", "Show layer mask"), NULL },
    { PICMAN_UNDO_CHANNEL_ADD, NC_("undo-type", "New channel"), NULL },
    { PICMAN_UNDO_CHANNEL_REMOVE, NC_("undo-type", "Delete channel"), NULL },
    { PICMAN_UNDO_CHANNEL_COLOR, NC_("undo-type", "Channel color"), NULL },
    { PICMAN_UNDO_VECTORS_ADD, NC_("undo-type", "New path"), NULL },
    { PICMAN_UNDO_VECTORS_REMOVE, NC_("undo-type", "Delete path"), NULL },
    { PICMAN_UNDO_VECTORS_MOD, NC_("undo-type", "Path modification"), NULL },
    { PICMAN_UNDO_FS_TO_LAYER, NC_("undo-type", "Floating selection to layer"), NULL },
    { PICMAN_UNDO_TRANSFORM, NC_("undo-type", "Transform"), NULL },
    { PICMAN_UNDO_PAINT, NC_("undo-type", "Paint"), NULL },
    { PICMAN_UNDO_INK, NC_("undo-type", "Ink"), NULL },
    { PICMAN_UNDO_FOREGROUND_SELECT, NC_("undo-type", "Select foreground"), NULL },
    { PICMAN_UNDO_PARASITE_ATTACH, NC_("undo-type", "Attach parasite"), NULL },
    { PICMAN_UNDO_PARASITE_REMOVE, NC_("undo-type", "Remove parasite"), NULL },
    { PICMAN_UNDO_CANT, NC_("undo-type", "Not undoable"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanUndoType", values);
      picman_type_set_translation_context (type, "undo-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_dirty_mask_get_type (void)
{
  static const GFlagsValue values[] =
  {
    { PICMAN_DIRTY_NONE, "PICMAN_DIRTY_NONE", "none" },
    { PICMAN_DIRTY_IMAGE, "PICMAN_DIRTY_IMAGE", "image" },
    { PICMAN_DIRTY_IMAGE_SIZE, "PICMAN_DIRTY_IMAGE_SIZE", "image-size" },
    { PICMAN_DIRTY_IMAGE_META, "PICMAN_DIRTY_IMAGE_META", "image-meta" },
    { PICMAN_DIRTY_IMAGE_STRUCTURE, "PICMAN_DIRTY_IMAGE_STRUCTURE", "image-structure" },
    { PICMAN_DIRTY_ITEM, "PICMAN_DIRTY_ITEM", "item" },
    { PICMAN_DIRTY_ITEM_META, "PICMAN_DIRTY_ITEM_META", "item-meta" },
    { PICMAN_DIRTY_DRAWABLE, "PICMAN_DIRTY_DRAWABLE", "drawable" },
    { PICMAN_DIRTY_VECTORS, "PICMAN_DIRTY_VECTORS", "vectors" },
    { PICMAN_DIRTY_SELECTION, "PICMAN_DIRTY_SELECTION", "selection" },
    { PICMAN_DIRTY_ACTIVE_DRAWABLE, "PICMAN_DIRTY_ACTIVE_DRAWABLE", "active-drawable" },
    { PICMAN_DIRTY_ALL, "PICMAN_DIRTY_ALL", "all" },
    { 0, NULL, NULL }
  };

  static const PicmanFlagsDesc descs[] =
  {
    { PICMAN_DIRTY_NONE, "PICMAN_DIRTY_NONE", NULL },
    { PICMAN_DIRTY_IMAGE, "PICMAN_DIRTY_IMAGE", NULL },
    { PICMAN_DIRTY_IMAGE_SIZE, "PICMAN_DIRTY_IMAGE_SIZE", NULL },
    { PICMAN_DIRTY_IMAGE_META, "PICMAN_DIRTY_IMAGE_META", NULL },
    { PICMAN_DIRTY_IMAGE_STRUCTURE, "PICMAN_DIRTY_IMAGE_STRUCTURE", NULL },
    { PICMAN_DIRTY_ITEM, "PICMAN_DIRTY_ITEM", NULL },
    { PICMAN_DIRTY_ITEM_META, "PICMAN_DIRTY_ITEM_META", NULL },
    { PICMAN_DIRTY_DRAWABLE, "PICMAN_DIRTY_DRAWABLE", NULL },
    { PICMAN_DIRTY_VECTORS, "PICMAN_DIRTY_VECTORS", NULL },
    { PICMAN_DIRTY_SELECTION, "PICMAN_DIRTY_SELECTION", NULL },
    { PICMAN_DIRTY_ACTIVE_DRAWABLE, "PICMAN_DIRTY_ACTIVE_DRAWABLE", NULL },
    { PICMAN_DIRTY_ALL, "PICMAN_DIRTY_ALL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_flags_register_static ("PicmanDirtyMask", values);
      picman_type_set_translation_context (type, "dirty-mask");
      picman_flags_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_offset_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_OFFSET_BACKGROUND, "PICMAN_OFFSET_BACKGROUND", "background" },
    { PICMAN_OFFSET_TRANSPARENT, "PICMAN_OFFSET_TRANSPARENT", "transparent" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_OFFSET_BACKGROUND, "PICMAN_OFFSET_BACKGROUND", NULL },
    { PICMAN_OFFSET_TRANSPARENT, "PICMAN_OFFSET_TRANSPARENT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanOffsetType", values);
      picman_type_set_translation_context (type, "offset-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_gradient_color_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRADIENT_COLOR_FIXED, "PICMAN_GRADIENT_COLOR_FIXED", "fixed" },
    { PICMAN_GRADIENT_COLOR_FOREGROUND, "PICMAN_GRADIENT_COLOR_FOREGROUND", "foreground" },
    { PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT, "PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT", "foreground-transparent" },
    { PICMAN_GRADIENT_COLOR_BACKGROUND, "PICMAN_GRADIENT_COLOR_BACKGROUND", "background" },
    { PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT, "PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT", "background-transparent" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRADIENT_COLOR_FIXED, "PICMAN_GRADIENT_COLOR_FIXED", NULL },
    { PICMAN_GRADIENT_COLOR_FOREGROUND, "PICMAN_GRADIENT_COLOR_FOREGROUND", NULL },
    { PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT, "PICMAN_GRADIENT_COLOR_FOREGROUND_TRANSPARENT", NULL },
    { PICMAN_GRADIENT_COLOR_BACKGROUND, "PICMAN_GRADIENT_COLOR_BACKGROUND", NULL },
    { PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT, "PICMAN_GRADIENT_COLOR_BACKGROUND_TRANSPARENT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGradientColor", values);
      picman_type_set_translation_context (type, "gradient-color");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_gradient_segment_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRADIENT_SEGMENT_LINEAR, "PICMAN_GRADIENT_SEGMENT_LINEAR", "linear" },
    { PICMAN_GRADIENT_SEGMENT_CURVED, "PICMAN_GRADIENT_SEGMENT_CURVED", "curved" },
    { PICMAN_GRADIENT_SEGMENT_SINE, "PICMAN_GRADIENT_SEGMENT_SINE", "sine" },
    { PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING, "PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING", "sphere-increasing" },
    { PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING, "PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING", "sphere-decreasing" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRADIENT_SEGMENT_LINEAR, "PICMAN_GRADIENT_SEGMENT_LINEAR", NULL },
    { PICMAN_GRADIENT_SEGMENT_CURVED, "PICMAN_GRADIENT_SEGMENT_CURVED", NULL },
    { PICMAN_GRADIENT_SEGMENT_SINE, "PICMAN_GRADIENT_SEGMENT_SINE", NULL },
    { PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING, "PICMAN_GRADIENT_SEGMENT_SPHERE_INCREASING", NULL },
    { PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING, "PICMAN_GRADIENT_SEGMENT_SPHERE_DECREASING", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGradientSegmentType", values);
      picman_type_set_translation_context (type, "gradient-segment-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_gradient_segment_color_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRADIENT_SEGMENT_RGB, "PICMAN_GRADIENT_SEGMENT_RGB", "rgb" },
    { PICMAN_GRADIENT_SEGMENT_HSV_CCW, "PICMAN_GRADIENT_SEGMENT_HSV_CCW", "hsv-ccw" },
    { PICMAN_GRADIENT_SEGMENT_HSV_CW, "PICMAN_GRADIENT_SEGMENT_HSV_CW", "hsv-cw" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRADIENT_SEGMENT_RGB, "PICMAN_GRADIENT_SEGMENT_RGB", NULL },
    { PICMAN_GRADIENT_SEGMENT_HSV_CCW, "PICMAN_GRADIENT_SEGMENT_HSV_CCW", NULL },
    { PICMAN_GRADIENT_SEGMENT_HSV_CW, "PICMAN_GRADIENT_SEGMENT_HSV_CW", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGradientSegmentColor", values);
      picman_type_set_translation_context (type, "gradient-segment-color");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_mask_apply_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_MASK_APPLY, "PICMAN_MASK_APPLY", "apply" },
    { PICMAN_MASK_DISCARD, "PICMAN_MASK_DISCARD", "discard" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_MASK_APPLY, "PICMAN_MASK_APPLY", NULL },
    { PICMAN_MASK_DISCARD, "PICMAN_MASK_DISCARD", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanMaskApplyMode", values);
      picman_type_set_translation_context (type, "mask-apply-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_merge_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_EXPAND_AS_NECESSARY, "PICMAN_EXPAND_AS_NECESSARY", "expand-as-necessary" },
    { PICMAN_CLIP_TO_IMAGE, "PICMAN_CLIP_TO_IMAGE", "clip-to-image" },
    { PICMAN_CLIP_TO_BOTTOM_LAYER, "PICMAN_CLIP_TO_BOTTOM_LAYER", "clip-to-bottom-layer" },
    { PICMAN_FLATTEN_IMAGE, "PICMAN_FLATTEN_IMAGE", "flatten-image" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_EXPAND_AS_NECESSARY, "PICMAN_EXPAND_AS_NECESSARY", NULL },
    { PICMAN_CLIP_TO_IMAGE, "PICMAN_CLIP_TO_IMAGE", NULL },
    { PICMAN_CLIP_TO_BOTTOM_LAYER, "PICMAN_CLIP_TO_BOTTOM_LAYER", NULL },
    { PICMAN_FLATTEN_IMAGE, "PICMAN_FLATTEN_IMAGE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanMergeType", values);
      picman_type_set_translation_context (type, "merge-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_select_criterion_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_SELECT_CRITERION_COMPOSITE, "PICMAN_SELECT_CRITERION_COMPOSITE", "composite" },
    { PICMAN_SELECT_CRITERION_R, "PICMAN_SELECT_CRITERION_R", "r" },
    { PICMAN_SELECT_CRITERION_G, "PICMAN_SELECT_CRITERION_G", "g" },
    { PICMAN_SELECT_CRITERION_B, "PICMAN_SELECT_CRITERION_B", "b" },
    { PICMAN_SELECT_CRITERION_H, "PICMAN_SELECT_CRITERION_H", "h" },
    { PICMAN_SELECT_CRITERION_S, "PICMAN_SELECT_CRITERION_S", "s" },
    { PICMAN_SELECT_CRITERION_V, "PICMAN_SELECT_CRITERION_V", "v" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_SELECT_CRITERION_COMPOSITE, NC_("select-criterion", "Composite"), NULL },
    { PICMAN_SELECT_CRITERION_R, NC_("select-criterion", "Red"), NULL },
    { PICMAN_SELECT_CRITERION_G, NC_("select-criterion", "Green"), NULL },
    { PICMAN_SELECT_CRITERION_B, NC_("select-criterion", "Blue"), NULL },
    { PICMAN_SELECT_CRITERION_H, NC_("select-criterion", "Hue"), NULL },
    { PICMAN_SELECT_CRITERION_S, NC_("select-criterion", "Saturation"), NULL },
    { PICMAN_SELECT_CRITERION_V, NC_("select-criterion", "Value"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanSelectCriterion", values);
      picman_type_set_translation_context (type, "select-criterion");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_message_severity_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_MESSAGE_INFO, "PICMAN_MESSAGE_INFO", "info" },
    { PICMAN_MESSAGE_WARNING, "PICMAN_MESSAGE_WARNING", "warning" },
    { PICMAN_MESSAGE_ERROR, "PICMAN_MESSAGE_ERROR", "error" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_MESSAGE_INFO, NC_("message-severity", "Message"), NULL },
    { PICMAN_MESSAGE_WARNING, NC_("message-severity", "Warning"), NULL },
    { PICMAN_MESSAGE_ERROR, NC_("message-severity", "Error"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanMessageSeverity", values);
      picman_type_set_translation_context (type, "message-severity");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_profile_policy_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_PROFILE_POLICY_ASK, "PICMAN_COLOR_PROFILE_POLICY_ASK", "ask" },
    { PICMAN_COLOR_PROFILE_POLICY_KEEP, "PICMAN_COLOR_PROFILE_POLICY_KEEP", "keep" },
    { PICMAN_COLOR_PROFILE_POLICY_CONVERT, "PICMAN_COLOR_PROFILE_POLICY_CONVERT", "convert" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_PROFILE_POLICY_ASK, NC_("color-profile-policy", "Ask what to do"), NULL },
    { PICMAN_COLOR_PROFILE_POLICY_KEEP, NC_("color-profile-policy", "Keep embedded profile"), NULL },
    { PICMAN_COLOR_PROFILE_POLICY_CONVERT, NC_("color-profile-policy", "Convert to RGB workspace"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorProfilePolicy", values);
      picman_type_set_translation_context (type, "color-profile-policy");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_dynamics_output_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_DYNAMICS_OUTPUT_OPACITY, "PICMAN_DYNAMICS_OUTPUT_OPACITY", "opacity" },
    { PICMAN_DYNAMICS_OUTPUT_SIZE, "PICMAN_DYNAMICS_OUTPUT_SIZE", "size" },
    { PICMAN_DYNAMICS_OUTPUT_ANGLE, "PICMAN_DYNAMICS_OUTPUT_ANGLE", "angle" },
    { PICMAN_DYNAMICS_OUTPUT_COLOR, "PICMAN_DYNAMICS_OUTPUT_COLOR", "color" },
    { PICMAN_DYNAMICS_OUTPUT_HARDNESS, "PICMAN_DYNAMICS_OUTPUT_HARDNESS", "hardness" },
    { PICMAN_DYNAMICS_OUTPUT_FORCE, "PICMAN_DYNAMICS_OUTPUT_FORCE", "force" },
    { PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO, "PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO", "aspect-ratio" },
    { PICMAN_DYNAMICS_OUTPUT_SPACING, "PICMAN_DYNAMICS_OUTPUT_SPACING", "spacing" },
    { PICMAN_DYNAMICS_OUTPUT_RATE, "PICMAN_DYNAMICS_OUTPUT_RATE", "rate" },
    { PICMAN_DYNAMICS_OUTPUT_FLOW, "PICMAN_DYNAMICS_OUTPUT_FLOW", "flow" },
    { PICMAN_DYNAMICS_OUTPUT_JITTER, "PICMAN_DYNAMICS_OUTPUT_JITTER", "jitter" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_DYNAMICS_OUTPUT_OPACITY, NC_("dynamics-output-type", "Opacity"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_SIZE, NC_("dynamics-output-type", "Size"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_ANGLE, NC_("dynamics-output-type", "Angle"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_COLOR, NC_("dynamics-output-type", "Color"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_HARDNESS, NC_("dynamics-output-type", "Hardness"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_FORCE, NC_("dynamics-output-type", "Force"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO, NC_("dynamics-output-type", "Aspect ratio"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_SPACING, NC_("dynamics-output-type", "Spacing"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_RATE, NC_("dynamics-output-type", "Rate"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_FLOW, NC_("dynamics-output-type", "Flow"), NULL },
    { PICMAN_DYNAMICS_OUTPUT_JITTER, NC_("dynamics-output-type", "Jitter"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanDynamicsOutputType", values);
      picman_type_set_translation_context (type, "dynamics-output-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

