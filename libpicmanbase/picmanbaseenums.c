
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#undef PICMAN_DISABLE_DEPRECATED
#include "picmanbasetypes.h"
#include "libpicman/libpicman-intl.h"

/* enumerations from "./picmanbaseenums.h" */
GType
picman_add_mask_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ADD_WHITE_MASK, "PICMAN_ADD_WHITE_MASK", "white-mask" },
    { PICMAN_ADD_BLACK_MASK, "PICMAN_ADD_BLACK_MASK", "black-mask" },
    { PICMAN_ADD_ALPHA_MASK, "PICMAN_ADD_ALPHA_MASK", "alpha-mask" },
    { PICMAN_ADD_ALPHA_TRANSFER_MASK, "PICMAN_ADD_ALPHA_TRANSFER_MASK", "alpha-transfer-mask" },
    { PICMAN_ADD_SELECTION_MASK, "PICMAN_ADD_SELECTION_MASK", "selection-mask" },
    { PICMAN_ADD_COPY_MASK, "PICMAN_ADD_COPY_MASK", "copy-mask" },
    { PICMAN_ADD_CHANNEL_MASK, "PICMAN_ADD_CHANNEL_MASK", "channel-mask" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ADD_WHITE_MASK, NC_("add-mask-type", "_White (full opacity)"), NULL },
    { PICMAN_ADD_BLACK_MASK, NC_("add-mask-type", "_Black (full transparency)"), NULL },
    { PICMAN_ADD_ALPHA_MASK, NC_("add-mask-type", "Layer's _alpha channel"), NULL },
    { PICMAN_ADD_ALPHA_TRANSFER_MASK, NC_("add-mask-type", "_Transfer layer's alpha channel"), NULL },
    { PICMAN_ADD_SELECTION_MASK, NC_("add-mask-type", "_Selection"), NULL },
    { PICMAN_ADD_COPY_MASK, NC_("add-mask-type", "_Grayscale copy of layer"), NULL },
    { PICMAN_ADD_CHANNEL_MASK, NC_("add-mask-type", "C_hannel"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanAddMaskType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "add-mask-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_blend_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_FG_BG_RGB_MODE, "PICMAN_FG_BG_RGB_MODE", "fg-bg-rgb-mode" },
    { PICMAN_FG_BG_HSV_MODE, "PICMAN_FG_BG_HSV_MODE", "fg-bg-hsv-mode" },
    { PICMAN_FG_TRANSPARENT_MODE, "PICMAN_FG_TRANSPARENT_MODE", "fg-transparent-mode" },
    { PICMAN_CUSTOM_MODE, "PICMAN_CUSTOM_MODE", "custom-mode" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_FG_BG_RGB_MODE, NC_("blend-mode", "FG to BG (RGB)"), NULL },
    { PICMAN_FG_BG_HSV_MODE, NC_("blend-mode", "FG to BG (HSV)"), NULL },
    { PICMAN_FG_TRANSPARENT_MODE, NC_("blend-mode", "FG to transparent"), NULL },
    { PICMAN_CUSTOM_MODE, NC_("blend-mode", "Custom gradient"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanBlendMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "blend-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_bucket_fill_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_FG_BUCKET_FILL, "PICMAN_FG_BUCKET_FILL", "fg-bucket-fill" },
    { PICMAN_BG_BUCKET_FILL, "PICMAN_BG_BUCKET_FILL", "bg-bucket-fill" },
    { PICMAN_PATTERN_BUCKET_FILL, "PICMAN_PATTERN_BUCKET_FILL", "pattern-bucket-fill" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_FG_BUCKET_FILL, NC_("bucket-fill-mode", "FG color fill"), NULL },
    { PICMAN_BG_BUCKET_FILL, NC_("bucket-fill-mode", "BG color fill"), NULL },
    { PICMAN_PATTERN_BUCKET_FILL, NC_("bucket-fill-mode", "Pattern fill"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanBucketFillMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "bucket-fill-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_channel_ops_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CHANNEL_OP_ADD, "PICMAN_CHANNEL_OP_ADD", "add" },
    { PICMAN_CHANNEL_OP_SUBTRACT, "PICMAN_CHANNEL_OP_SUBTRACT", "subtract" },
    { PICMAN_CHANNEL_OP_REPLACE, "PICMAN_CHANNEL_OP_REPLACE", "replace" },
    { PICMAN_CHANNEL_OP_INTERSECT, "PICMAN_CHANNEL_OP_INTERSECT", "intersect" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CHANNEL_OP_ADD, NC_("channel-ops", "Add to the current selection"), NULL },
    { PICMAN_CHANNEL_OP_SUBTRACT, NC_("channel-ops", "Subtract from the current selection"), NULL },
    { PICMAN_CHANNEL_OP_REPLACE, NC_("channel-ops", "Replace the current selection"), NULL },
    { PICMAN_CHANNEL_OP_INTERSECT, NC_("channel-ops", "Intersect with the current selection"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanChannelOps", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "channel-ops");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_channel_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RED_CHANNEL, "PICMAN_RED_CHANNEL", "red-channel" },
    { PICMAN_GREEN_CHANNEL, "PICMAN_GREEN_CHANNEL", "green-channel" },
    { PICMAN_BLUE_CHANNEL, "PICMAN_BLUE_CHANNEL", "blue-channel" },
    { PICMAN_GRAY_CHANNEL, "PICMAN_GRAY_CHANNEL", "gray-channel" },
    { PICMAN_INDEXED_CHANNEL, "PICMAN_INDEXED_CHANNEL", "indexed-channel" },
    { PICMAN_ALPHA_CHANNEL, "PICMAN_ALPHA_CHANNEL", "alpha-channel" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RED_CHANNEL, NC_("channel-type", "Red"), NULL },
    { PICMAN_GREEN_CHANNEL, NC_("channel-type", "Green"), NULL },
    { PICMAN_BLUE_CHANNEL, NC_("channel-type", "Blue"), NULL },
    { PICMAN_GRAY_CHANNEL, NC_("channel-type", "Gray"), NULL },
    { PICMAN_INDEXED_CHANNEL, NC_("channel-type", "Indexed"), NULL },
    { PICMAN_ALPHA_CHANNEL, NC_("channel-type", "Alpha"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanChannelType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "channel-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_check_size_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CHECK_SIZE_SMALL_CHECKS, "PICMAN_CHECK_SIZE_SMALL_CHECKS", "small-checks" },
    { PICMAN_CHECK_SIZE_MEDIUM_CHECKS, "PICMAN_CHECK_SIZE_MEDIUM_CHECKS", "medium-checks" },
    { PICMAN_CHECK_SIZE_LARGE_CHECKS, "PICMAN_CHECK_SIZE_LARGE_CHECKS", "large-checks" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CHECK_SIZE_SMALL_CHECKS, NC_("check-size", "Small"), NULL },
    { PICMAN_CHECK_SIZE_MEDIUM_CHECKS, NC_("check-size", "Medium"), NULL },
    { PICMAN_CHECK_SIZE_LARGE_CHECKS, NC_("check-size", "Large"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCheckSize", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "check-size");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_check_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CHECK_TYPE_LIGHT_CHECKS, "PICMAN_CHECK_TYPE_LIGHT_CHECKS", "light-checks" },
    { PICMAN_CHECK_TYPE_GRAY_CHECKS, "PICMAN_CHECK_TYPE_GRAY_CHECKS", "gray-checks" },
    { PICMAN_CHECK_TYPE_DARK_CHECKS, "PICMAN_CHECK_TYPE_DARK_CHECKS", "dark-checks" },
    { PICMAN_CHECK_TYPE_WHITE_ONLY, "PICMAN_CHECK_TYPE_WHITE_ONLY", "white-only" },
    { PICMAN_CHECK_TYPE_GRAY_ONLY, "PICMAN_CHECK_TYPE_GRAY_ONLY", "gray-only" },
    { PICMAN_CHECK_TYPE_BLACK_ONLY, "PICMAN_CHECK_TYPE_BLACK_ONLY", "black-only" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CHECK_TYPE_LIGHT_CHECKS, NC_("check-type", "Light checks"), NULL },
    { PICMAN_CHECK_TYPE_GRAY_CHECKS, NC_("check-type", "Mid-tone checks"), NULL },
    { PICMAN_CHECK_TYPE_DARK_CHECKS, NC_("check-type", "Dark checks"), NULL },
    { PICMAN_CHECK_TYPE_WHITE_ONLY, NC_("check-type", "White only"), NULL },
    { PICMAN_CHECK_TYPE_GRAY_ONLY, NC_("check-type", "Gray only"), NULL },
    { PICMAN_CHECK_TYPE_BLACK_ONLY, NC_("check-type", "Black only"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCheckType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "check-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_clone_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_IMAGE_CLONE, "PICMAN_IMAGE_CLONE", "image-clone" },
    { PICMAN_PATTERN_CLONE, "PICMAN_PATTERN_CLONE", "pattern-clone" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_IMAGE_CLONE, NC_("clone-type", "Image"), NULL },
    { PICMAN_PATTERN_CLONE, NC_("clone-type", "Pattern"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCloneType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "clone-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_desaturate_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_DESATURATE_LIGHTNESS, "PICMAN_DESATURATE_LIGHTNESS", "lightness" },
    { PICMAN_DESATURATE_LUMINOSITY, "PICMAN_DESATURATE_LUMINOSITY", "luminosity" },
    { PICMAN_DESATURATE_AVERAGE, "PICMAN_DESATURATE_AVERAGE", "average" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_DESATURATE_LIGHTNESS, NC_("desaturate-mode", "Lightness"), NULL },
    { PICMAN_DESATURATE_LUMINOSITY, NC_("desaturate-mode", "Luminosity"), NULL },
    { PICMAN_DESATURATE_AVERAGE, NC_("desaturate-mode", "Average"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanDesaturateMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "desaturate-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_dodge_burn_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_DODGE, "PICMAN_DODGE", "dodge" },
    { PICMAN_BURN, "PICMAN_BURN", "burn" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_DODGE, NC_("dodge-burn-type", "Dodge"), NULL },
    { PICMAN_BURN, NC_("dodge-burn-type", "Burn"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanDodgeBurnType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "dodge-burn-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_foreground_extract_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_FOREGROUND_EXTRACT_SIOX, "PICMAN_FOREGROUND_EXTRACT_SIOX", "siox" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_FOREGROUND_EXTRACT_SIOX, "PICMAN_FOREGROUND_EXTRACT_SIOX", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanForegroundExtractMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "foreground-extract-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_gradient_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRADIENT_LINEAR, "PICMAN_GRADIENT_LINEAR", "linear" },
    { PICMAN_GRADIENT_BILINEAR, "PICMAN_GRADIENT_BILINEAR", "bilinear" },
    { PICMAN_GRADIENT_RADIAL, "PICMAN_GRADIENT_RADIAL", "radial" },
    { PICMAN_GRADIENT_SQUARE, "PICMAN_GRADIENT_SQUARE", "square" },
    { PICMAN_GRADIENT_CONICAL_SYMMETRIC, "PICMAN_GRADIENT_CONICAL_SYMMETRIC", "conical-symmetric" },
    { PICMAN_GRADIENT_CONICAL_ASYMMETRIC, "PICMAN_GRADIENT_CONICAL_ASYMMETRIC", "conical-asymmetric" },
    { PICMAN_GRADIENT_SHAPEBURST_ANGULAR, "PICMAN_GRADIENT_SHAPEBURST_ANGULAR", "shapeburst-angular" },
    { PICMAN_GRADIENT_SHAPEBURST_SPHERICAL, "PICMAN_GRADIENT_SHAPEBURST_SPHERICAL", "shapeburst-spherical" },
    { PICMAN_GRADIENT_SHAPEBURST_DIMPLED, "PICMAN_GRADIENT_SHAPEBURST_DIMPLED", "shapeburst-dimpled" },
    { PICMAN_GRADIENT_SPIRAL_CLOCKWISE, "PICMAN_GRADIENT_SPIRAL_CLOCKWISE", "spiral-clockwise" },
    { PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE, "PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE", "spiral-anticlockwise" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRADIENT_LINEAR, NC_("gradient-type", "Linear"), NULL },
    { PICMAN_GRADIENT_BILINEAR, NC_("gradient-type", "Bi-linear"), NULL },
    { PICMAN_GRADIENT_RADIAL, NC_("gradient-type", "Radial"), NULL },
    { PICMAN_GRADIENT_SQUARE, NC_("gradient-type", "Square"), NULL },
    { PICMAN_GRADIENT_CONICAL_SYMMETRIC, NC_("gradient-type", "Conical (sym)"), NULL },
    { PICMAN_GRADIENT_CONICAL_ASYMMETRIC, NC_("gradient-type", "Conical (asym)"), NULL },
    { PICMAN_GRADIENT_SHAPEBURST_ANGULAR, NC_("gradient-type", "Shaped (angular)"), NULL },
    { PICMAN_GRADIENT_SHAPEBURST_SPHERICAL, NC_("gradient-type", "Shaped (spherical)"), NULL },
    { PICMAN_GRADIENT_SHAPEBURST_DIMPLED, NC_("gradient-type", "Shaped (dimpled)"), NULL },
    { PICMAN_GRADIENT_SPIRAL_CLOCKWISE, NC_("gradient-type", "Spiral (cw)"), NULL },
    { PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE, NC_("gradient-type", "Spiral (ccw)"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGradientType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "gradient-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_grid_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GRID_DOTS, "PICMAN_GRID_DOTS", "dots" },
    { PICMAN_GRID_INTERSECTIONS, "PICMAN_GRID_INTERSECTIONS", "intersections" },
    { PICMAN_GRID_ON_OFF_DASH, "PICMAN_GRID_ON_OFF_DASH", "on-off-dash" },
    { PICMAN_GRID_DOUBLE_DASH, "PICMAN_GRID_DOUBLE_DASH", "double-dash" },
    { PICMAN_GRID_SOLID, "PICMAN_GRID_SOLID", "solid" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GRID_DOTS, NC_("grid-style", "Intersections (dots)"), NULL },
    { PICMAN_GRID_INTERSECTIONS, NC_("grid-style", "Intersections (crosshairs)"), NULL },
    { PICMAN_GRID_ON_OFF_DASH, NC_("grid-style", "Dashed"), NULL },
    { PICMAN_GRID_DOUBLE_DASH, NC_("grid-style", "Double dashed"), NULL },
    { PICMAN_GRID_SOLID, NC_("grid-style", "Solid"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGridStyle", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "grid-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_icon_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ICON_TYPE_STOCK_ID, "PICMAN_ICON_TYPE_STOCK_ID", "stock-id" },
    { PICMAN_ICON_TYPE_INLINE_PIXBUF, "PICMAN_ICON_TYPE_INLINE_PIXBUF", "inline-pixbuf" },
    { PICMAN_ICON_TYPE_IMAGE_FILE, "PICMAN_ICON_TYPE_IMAGE_FILE", "image-file" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ICON_TYPE_STOCK_ID, NC_("icon-type", "Stock ID"), NULL },
    { PICMAN_ICON_TYPE_INLINE_PIXBUF, NC_("icon-type", "Inline pixbuf"), NULL },
    { PICMAN_ICON_TYPE_IMAGE_FILE, NC_("icon-type", "Image file"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanIconType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "icon-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_image_base_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RGB, "PICMAN_RGB", "rgb" },
    { PICMAN_GRAY, "PICMAN_GRAY", "gray" },
    { PICMAN_INDEXED, "PICMAN_INDEXED", "indexed" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RGB, NC_("image-base-type", "RGB color"), NULL },
    { PICMAN_GRAY, NC_("image-base-type", "Grayscale"), NULL },
    { PICMAN_INDEXED, NC_("image-base-type", "Indexed color"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanImageBaseType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "image-base-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_image_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RGB_IMAGE, "PICMAN_RGB_IMAGE", "rgb-image" },
    { PICMAN_RGBA_IMAGE, "PICMAN_RGBA_IMAGE", "rgba-image" },
    { PICMAN_GRAY_IMAGE, "PICMAN_GRAY_IMAGE", "gray-image" },
    { PICMAN_GRAYA_IMAGE, "PICMAN_GRAYA_IMAGE", "graya-image" },
    { PICMAN_INDEXED_IMAGE, "PICMAN_INDEXED_IMAGE", "indexed-image" },
    { PICMAN_INDEXEDA_IMAGE, "PICMAN_INDEXEDA_IMAGE", "indexeda-image" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RGB_IMAGE, NC_("image-type", "RGB"), NULL },
    { PICMAN_RGBA_IMAGE, NC_("image-type", "RGB-alpha"), NULL },
    { PICMAN_GRAY_IMAGE, NC_("image-type", "Grayscale"), NULL },
    { PICMAN_GRAYA_IMAGE, NC_("image-type", "Grayscale-alpha"), NULL },
    { PICMAN_INDEXED_IMAGE, NC_("image-type", "Indexed"), NULL },
    { PICMAN_INDEXEDA_IMAGE, NC_("image-type", "Indexed-alpha"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanImageType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "image-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_interpolation_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_INTERPOLATION_NONE, "PICMAN_INTERPOLATION_NONE", "none" },
    { PICMAN_INTERPOLATION_LINEAR, "PICMAN_INTERPOLATION_LINEAR", "linear" },
    { PICMAN_INTERPOLATION_CUBIC, "PICMAN_INTERPOLATION_CUBIC", "cubic" },
    { PICMAN_INTERPOLATION_NOHALO, "PICMAN_INTERPOLATION_NOHALO", "nohalo" },
    { PICMAN_INTERPOLATION_LOHALO, "PICMAN_INTERPOLATION_LOHALO", "lohalo" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_INTERPOLATION_NONE, NC_("interpolation-type", "None"), NULL },
    { PICMAN_INTERPOLATION_LINEAR, NC_("interpolation-type", "Linear"), NULL },
    { PICMAN_INTERPOLATION_CUBIC, NC_("interpolation-type", "Cubic"), NULL },
    { PICMAN_INTERPOLATION_NOHALO, NC_("interpolation-type", "NoHalo"), NULL },
    { PICMAN_INTERPOLATION_LOHALO, NC_("interpolation-type", "LoHalo"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanInterpolationType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "interpolation-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_paint_application_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PAINT_CONSTANT, "PICMAN_PAINT_CONSTANT", "constant" },
    { PICMAN_PAINT_INCREMENTAL, "PICMAN_PAINT_INCREMENTAL", "incremental" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PAINT_CONSTANT, NC_("paint-application-mode", "Constant"), NULL },
    { PICMAN_PAINT_INCREMENTAL, NC_("paint-application-mode", "Incremental"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPaintApplicationMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "paint-application-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_repeat_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_REPEAT_NONE, "PICMAN_REPEAT_NONE", "none" },
    { PICMAN_REPEAT_SAWTOOTH, "PICMAN_REPEAT_SAWTOOTH", "sawtooth" },
    { PICMAN_REPEAT_TRIANGULAR, "PICMAN_REPEAT_TRIANGULAR", "triangular" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_REPEAT_NONE, NC_("repeat-mode", "None"), NULL },
    { PICMAN_REPEAT_SAWTOOTH, NC_("repeat-mode", "Sawtooth wave"), NULL },
    { PICMAN_REPEAT_TRIANGULAR, NC_("repeat-mode", "Triangular wave"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRepeatMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "repeat-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_run_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RUN_INTERACTIVE, "PICMAN_RUN_INTERACTIVE", "interactive" },
    { PICMAN_RUN_NONINTERACTIVE, "PICMAN_RUN_NONINTERACTIVE", "noninteractive" },
    { PICMAN_RUN_WITH_LAST_VALS, "PICMAN_RUN_WITH_LAST_VALS", "with-last-vals" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RUN_INTERACTIVE, NC_("run-mode", "Run interactively"), NULL },
    { PICMAN_RUN_NONINTERACTIVE, NC_("run-mode", "Run non-interactively"), NULL },
    { PICMAN_RUN_WITH_LAST_VALS, NC_("run-mode", "Run with last used values"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRunMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "run-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_size_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PIXELS, "PICMAN_PIXELS", "pixels" },
    { PICMAN_POINTS, "PICMAN_POINTS", "points" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PIXELS, NC_("size-type", "Pixels"), NULL },
    { PICMAN_POINTS, NC_("size-type", "Points"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanSizeType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "size-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_transfer_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_SHADOWS, "PICMAN_SHADOWS", "shadows" },
    { PICMAN_MIDTONES, "PICMAN_MIDTONES", "midtones" },
    { PICMAN_HIGHLIGHTS, "PICMAN_HIGHLIGHTS", "highlights" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_SHADOWS, NC_("transfer-mode", "Shadows"), NULL },
    { PICMAN_MIDTONES, NC_("transfer-mode", "Midtones"), NULL },
    { PICMAN_HIGHLIGHTS, NC_("transfer-mode", "Highlights"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTransferMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "transfer-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_transform_direction_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TRANSFORM_FORWARD, "PICMAN_TRANSFORM_FORWARD", "forward" },
    { PICMAN_TRANSFORM_BACKWARD, "PICMAN_TRANSFORM_BACKWARD", "backward" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TRANSFORM_FORWARD, NC_("transform-direction", "Normal (Forward)"), NULL },
    { PICMAN_TRANSFORM_BACKWARD, NC_("transform-direction", "Corrective (Backward)"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTransformDirection", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "transform-direction");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_transform_resize_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TRANSFORM_RESIZE_ADJUST, "PICMAN_TRANSFORM_RESIZE_ADJUST", "adjust" },
    { PICMAN_TRANSFORM_RESIZE_CLIP, "PICMAN_TRANSFORM_RESIZE_CLIP", "clip" },
    { PICMAN_TRANSFORM_RESIZE_CROP, "PICMAN_TRANSFORM_RESIZE_CROP", "crop" },
    { PICMAN_TRANSFORM_RESIZE_CROP_WITH_ASPECT, "PICMAN_TRANSFORM_RESIZE_CROP_WITH_ASPECT", "crop-with-aspect" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TRANSFORM_RESIZE_ADJUST, NC_("transform-resize", "Adjust"), NULL },
    { PICMAN_TRANSFORM_RESIZE_CLIP, NC_("transform-resize", "Clip"), NULL },
    { PICMAN_TRANSFORM_RESIZE_CROP, NC_("transform-resize", "Crop to result"), NULL },
    { PICMAN_TRANSFORM_RESIZE_CROP_WITH_ASPECT, NC_("transform-resize", "Crop with aspect"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTransformResize", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "transform-resize");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_pdb_arg_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PDB_INT32, "PICMAN_PDB_INT32", "int32" },
    { PICMAN_PDB_INT16, "PICMAN_PDB_INT16", "int16" },
    { PICMAN_PDB_INT8, "PICMAN_PDB_INT8", "int8" },
    { PICMAN_PDB_FLOAT, "PICMAN_PDB_FLOAT", "float" },
    { PICMAN_PDB_STRING, "PICMAN_PDB_STRING", "string" },
    { PICMAN_PDB_INT32ARRAY, "PICMAN_PDB_INT32ARRAY", "int32array" },
    { PICMAN_PDB_INT16ARRAY, "PICMAN_PDB_INT16ARRAY", "int16array" },
    { PICMAN_PDB_INT8ARRAY, "PICMAN_PDB_INT8ARRAY", "int8array" },
    { PICMAN_PDB_FLOATARRAY, "PICMAN_PDB_FLOATARRAY", "floatarray" },
    { PICMAN_PDB_STRINGARRAY, "PICMAN_PDB_STRINGARRAY", "stringarray" },
    { PICMAN_PDB_COLOR, "PICMAN_PDB_COLOR", "color" },
    { PICMAN_PDB_ITEM, "PICMAN_PDB_ITEM", "item" },
    { PICMAN_PDB_DISPLAY, "PICMAN_PDB_DISPLAY", "display" },
    { PICMAN_PDB_IMAGE, "PICMAN_PDB_IMAGE", "image" },
    { PICMAN_PDB_LAYER, "PICMAN_PDB_LAYER", "layer" },
    { PICMAN_PDB_CHANNEL, "PICMAN_PDB_CHANNEL", "channel" },
    { PICMAN_PDB_DRAWABLE, "PICMAN_PDB_DRAWABLE", "drawable" },
    { PICMAN_PDB_SELECTION, "PICMAN_PDB_SELECTION", "selection" },
    { PICMAN_PDB_COLORARRAY, "PICMAN_PDB_COLORARRAY", "colorarray" },
    { PICMAN_PDB_VECTORS, "PICMAN_PDB_VECTORS", "vectors" },
    { PICMAN_PDB_PARASITE, "PICMAN_PDB_PARASITE", "parasite" },
    { PICMAN_PDB_STATUS, "PICMAN_PDB_STATUS", "status" },
    { PICMAN_PDB_END, "PICMAN_PDB_END", "end" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PDB_INT32, "PICMAN_PDB_INT32", NULL },
    { PICMAN_PDB_INT16, "PICMAN_PDB_INT16", NULL },
    { PICMAN_PDB_INT8, "PICMAN_PDB_INT8", NULL },
    { PICMAN_PDB_FLOAT, "PICMAN_PDB_FLOAT", NULL },
    { PICMAN_PDB_STRING, "PICMAN_PDB_STRING", NULL },
    { PICMAN_PDB_INT32ARRAY, "PICMAN_PDB_INT32ARRAY", NULL },
    { PICMAN_PDB_INT16ARRAY, "PICMAN_PDB_INT16ARRAY", NULL },
    { PICMAN_PDB_INT8ARRAY, "PICMAN_PDB_INT8ARRAY", NULL },
    { PICMAN_PDB_FLOATARRAY, "PICMAN_PDB_FLOATARRAY", NULL },
    { PICMAN_PDB_STRINGARRAY, "PICMAN_PDB_STRINGARRAY", NULL },
    { PICMAN_PDB_COLOR, "PICMAN_PDB_COLOR", NULL },
    { PICMAN_PDB_ITEM, "PICMAN_PDB_ITEM", NULL },
    { PICMAN_PDB_DISPLAY, "PICMAN_PDB_DISPLAY", NULL },
    { PICMAN_PDB_IMAGE, "PICMAN_PDB_IMAGE", NULL },
    { PICMAN_PDB_LAYER, "PICMAN_PDB_LAYER", NULL },
    { PICMAN_PDB_CHANNEL, "PICMAN_PDB_CHANNEL", NULL },
    { PICMAN_PDB_DRAWABLE, "PICMAN_PDB_DRAWABLE", NULL },
    { PICMAN_PDB_SELECTION, "PICMAN_PDB_SELECTION", NULL },
    { PICMAN_PDB_COLORARRAY, "PICMAN_PDB_COLORARRAY", NULL },
    { PICMAN_PDB_VECTORS, "PICMAN_PDB_VECTORS", NULL },
    { PICMAN_PDB_PARASITE, "PICMAN_PDB_PARASITE", NULL },
    { PICMAN_PDB_STATUS, "PICMAN_PDB_STATUS", NULL },
    { PICMAN_PDB_END, "PICMAN_PDB_END", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPDBArgType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "pdb-arg-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_pdb_error_handler_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PDB_ERROR_HANDLER_INTERNAL, "PICMAN_PDB_ERROR_HANDLER_INTERNAL", "internal" },
    { PICMAN_PDB_ERROR_HANDLER_PLUGIN, "PICMAN_PDB_ERROR_HANDLER_PLUGIN", "plugin" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PDB_ERROR_HANDLER_INTERNAL, "PICMAN_PDB_ERROR_HANDLER_INTERNAL", NULL },
    { PICMAN_PDB_ERROR_HANDLER_PLUGIN, "PICMAN_PDB_ERROR_HANDLER_PLUGIN", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPDBErrorHandler", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "pdb-error-handler");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_pdb_proc_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_INTERNAL, "PICMAN_INTERNAL", "internal" },
    { PICMAN_PLUGIN, "PICMAN_PLUGIN", "plugin" },
    { PICMAN_EXTENSION, "PICMAN_EXTENSION", "extension" },
    { PICMAN_TEMPORARY, "PICMAN_TEMPORARY", "temporary" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_INTERNAL, NC_("pdb-proc-type", "Internal PICMAN procedure"), NULL },
    { PICMAN_PLUGIN, NC_("pdb-proc-type", "PICMAN Plug-In"), NULL },
    { PICMAN_EXTENSION, NC_("pdb-proc-type", "PICMAN Extension"), NULL },
    { PICMAN_TEMPORARY, NC_("pdb-proc-type", "Temporary Procedure"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPDBProcType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "pdb-proc-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_pdb_status_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PDB_EXECUTION_ERROR, "PICMAN_PDB_EXECUTION_ERROR", "execution-error" },
    { PICMAN_PDB_CALLING_ERROR, "PICMAN_PDB_CALLING_ERROR", "calling-error" },
    { PICMAN_PDB_PASS_THROUGH, "PICMAN_PDB_PASS_THROUGH", "pass-through" },
    { PICMAN_PDB_SUCCESS, "PICMAN_PDB_SUCCESS", "success" },
    { PICMAN_PDB_CANCEL, "PICMAN_PDB_CANCEL", "cancel" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PDB_EXECUTION_ERROR, "PICMAN_PDB_EXECUTION_ERROR", NULL },
    { PICMAN_PDB_CALLING_ERROR, "PICMAN_PDB_CALLING_ERROR", NULL },
    { PICMAN_PDB_PASS_THROUGH, "PICMAN_PDB_PASS_THROUGH", NULL },
    { PICMAN_PDB_SUCCESS, "PICMAN_PDB_SUCCESS", NULL },
    { PICMAN_PDB_CANCEL, "PICMAN_PDB_CANCEL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPDBStatusType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "pdb-status-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_message_handler_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_MESSAGE_BOX, "PICMAN_MESSAGE_BOX", "message-box" },
    { PICMAN_CONSOLE, "PICMAN_CONSOLE", "console" },
    { PICMAN_ERROR_CONSOLE, "PICMAN_ERROR_CONSOLE", "error-console" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_MESSAGE_BOX, "PICMAN_MESSAGE_BOX", NULL },
    { PICMAN_CONSOLE, "PICMAN_CONSOLE", NULL },
    { PICMAN_ERROR_CONSOLE, "PICMAN_ERROR_CONSOLE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanMessageHandlerType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "message-handler-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_stack_trace_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_STACK_TRACE_NEVER, "PICMAN_STACK_TRACE_NEVER", "never" },
    { PICMAN_STACK_TRACE_QUERY, "PICMAN_STACK_TRACE_QUERY", "query" },
    { PICMAN_STACK_TRACE_ALWAYS, "PICMAN_STACK_TRACE_ALWAYS", "always" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_STACK_TRACE_NEVER, "PICMAN_STACK_TRACE_NEVER", NULL },
    { PICMAN_STACK_TRACE_QUERY, "PICMAN_STACK_TRACE_QUERY", NULL },
    { PICMAN_STACK_TRACE_ALWAYS, "PICMAN_STACK_TRACE_ALWAYS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanStackTraceMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "stack-trace-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_progress_command_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PROGRESS_COMMAND_START, "PICMAN_PROGRESS_COMMAND_START", "start" },
    { PICMAN_PROGRESS_COMMAND_END, "PICMAN_PROGRESS_COMMAND_END", "end" },
    { PICMAN_PROGRESS_COMMAND_SET_TEXT, "PICMAN_PROGRESS_COMMAND_SET_TEXT", "set-text" },
    { PICMAN_PROGRESS_COMMAND_SET_VALUE, "PICMAN_PROGRESS_COMMAND_SET_VALUE", "set-value" },
    { PICMAN_PROGRESS_COMMAND_PULSE, "PICMAN_PROGRESS_COMMAND_PULSE", "pulse" },
    { PICMAN_PROGRESS_COMMAND_GET_WINDOW, "PICMAN_PROGRESS_COMMAND_GET_WINDOW", "get-window" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PROGRESS_COMMAND_START, "PICMAN_PROGRESS_COMMAND_START", NULL },
    { PICMAN_PROGRESS_COMMAND_END, "PICMAN_PROGRESS_COMMAND_END", NULL },
    { PICMAN_PROGRESS_COMMAND_SET_TEXT, "PICMAN_PROGRESS_COMMAND_SET_TEXT", NULL },
    { PICMAN_PROGRESS_COMMAND_SET_VALUE, "PICMAN_PROGRESS_COMMAND_SET_VALUE", NULL },
    { PICMAN_PROGRESS_COMMAND_PULSE, "PICMAN_PROGRESS_COMMAND_PULSE", NULL },
    { PICMAN_PROGRESS_COMMAND_GET_WINDOW, "PICMAN_PROGRESS_COMMAND_GET_WINDOW", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanProgressCommand", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "progress-command");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_text_direction_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TEXT_DIRECTION_LTR, "PICMAN_TEXT_DIRECTION_LTR", "ltr" },
    { PICMAN_TEXT_DIRECTION_RTL, "PICMAN_TEXT_DIRECTION_RTL", "rtl" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TEXT_DIRECTION_LTR, NC_("text-direction", "From left to right"), NULL },
    { PICMAN_TEXT_DIRECTION_RTL, NC_("text-direction", "From right to left"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTextDirection", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "text-direction");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_text_hint_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TEXT_HINT_STYLE_NONE, "PICMAN_TEXT_HINT_STYLE_NONE", "none" },
    { PICMAN_TEXT_HINT_STYLE_SLIGHT, "PICMAN_TEXT_HINT_STYLE_SLIGHT", "slight" },
    { PICMAN_TEXT_HINT_STYLE_MEDIUM, "PICMAN_TEXT_HINT_STYLE_MEDIUM", "medium" },
    { PICMAN_TEXT_HINT_STYLE_FULL, "PICMAN_TEXT_HINT_STYLE_FULL", "full" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TEXT_HINT_STYLE_NONE, NC_("text-hint-style", "None"), NULL },
    { PICMAN_TEXT_HINT_STYLE_SLIGHT, NC_("text-hint-style", "Slight"), NULL },
    { PICMAN_TEXT_HINT_STYLE_MEDIUM, NC_("text-hint-style", "Medium"), NULL },
    { PICMAN_TEXT_HINT_STYLE_FULL, NC_("text-hint-style", "Full"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTextHintStyle", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "text-hint-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_text_justification_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TEXT_JUSTIFY_LEFT, "PICMAN_TEXT_JUSTIFY_LEFT", "left" },
    { PICMAN_TEXT_JUSTIFY_RIGHT, "PICMAN_TEXT_JUSTIFY_RIGHT", "right" },
    { PICMAN_TEXT_JUSTIFY_CENTER, "PICMAN_TEXT_JUSTIFY_CENTER", "center" },
    { PICMAN_TEXT_JUSTIFY_FILL, "PICMAN_TEXT_JUSTIFY_FILL", "fill" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TEXT_JUSTIFY_LEFT, NC_("text-justification", "Left justified"), NULL },
    { PICMAN_TEXT_JUSTIFY_RIGHT, NC_("text-justification", "Right justified"), NULL },
    { PICMAN_TEXT_JUSTIFY_CENTER, NC_("text-justification", "Centered"), NULL },
    { PICMAN_TEXT_JUSTIFY_FILL, NC_("text-justification", "Filled"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTextJustification", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "text-justification");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_user_directory_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_USER_DIRECTORY_DESKTOP, "PICMAN_USER_DIRECTORY_DESKTOP", "desktop" },
    { PICMAN_USER_DIRECTORY_DOCUMENTS, "PICMAN_USER_DIRECTORY_DOCUMENTS", "documents" },
    { PICMAN_USER_DIRECTORY_DOWNLOAD, "PICMAN_USER_DIRECTORY_DOWNLOAD", "download" },
    { PICMAN_USER_DIRECTORY_MUSIC, "PICMAN_USER_DIRECTORY_MUSIC", "music" },
    { PICMAN_USER_DIRECTORY_PICTURES, "PICMAN_USER_DIRECTORY_PICTURES", "pictures" },
    { PICMAN_USER_DIRECTORY_PUBLIC_SHARE, "PICMAN_USER_DIRECTORY_PUBLIC_SHARE", "public-share" },
    { PICMAN_USER_DIRECTORY_TEMPLATES, "PICMAN_USER_DIRECTORY_TEMPLATES", "templates" },
    { PICMAN_USER_DIRECTORY_VIDEOS, "PICMAN_USER_DIRECTORY_VIDEOS", "videos" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_USER_DIRECTORY_DESKTOP, "PICMAN_USER_DIRECTORY_DESKTOP", NULL },
    { PICMAN_USER_DIRECTORY_DOCUMENTS, "PICMAN_USER_DIRECTORY_DOCUMENTS", NULL },
    { PICMAN_USER_DIRECTORY_DOWNLOAD, "PICMAN_USER_DIRECTORY_DOWNLOAD", NULL },
    { PICMAN_USER_DIRECTORY_MUSIC, "PICMAN_USER_DIRECTORY_MUSIC", NULL },
    { PICMAN_USER_DIRECTORY_PICTURES, "PICMAN_USER_DIRECTORY_PICTURES", NULL },
    { PICMAN_USER_DIRECTORY_PUBLIC_SHARE, "PICMAN_USER_DIRECTORY_PUBLIC_SHARE", NULL },
    { PICMAN_USER_DIRECTORY_TEMPLATES, "PICMAN_USER_DIRECTORY_TEMPLATES", NULL },
    { PICMAN_USER_DIRECTORY_VIDEOS, "PICMAN_USER_DIRECTORY_VIDEOS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanUserDirectory", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "user-directory");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_vectors_stroke_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_VECTORS_STROKE_TYPE_BEZIER, "PICMAN_VECTORS_STROKE_TYPE_BEZIER", "bezier" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_VECTORS_STROKE_TYPE_BEZIER, "PICMAN_VECTORS_STROKE_TYPE_BEZIER", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanVectorsStrokeType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "vectors-stroke-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

