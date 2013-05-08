
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "display-enums.h"
#include"picman-intl.h"

/* enumerations from "./display-enums.h" */
GType
picman_cursor_precision_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CURSOR_PRECISION_PIXEL_CENTER, "PICMAN_CURSOR_PRECISION_PIXEL_CENTER", "pixel-center" },
    { PICMAN_CURSOR_PRECISION_PIXEL_BORDER, "PICMAN_CURSOR_PRECISION_PIXEL_BORDER", "pixel-border" },
    { PICMAN_CURSOR_PRECISION_SUBPIXEL, "PICMAN_CURSOR_PRECISION_SUBPIXEL", "subpixel" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CURSOR_PRECISION_PIXEL_CENTER, "PICMAN_CURSOR_PRECISION_PIXEL_CENTER", NULL },
    { PICMAN_CURSOR_PRECISION_PIXEL_BORDER, "PICMAN_CURSOR_PRECISION_PIXEL_BORDER", NULL },
    { PICMAN_CURSOR_PRECISION_SUBPIXEL, "PICMAN_CURSOR_PRECISION_SUBPIXEL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCursorPrecision", values);
      picman_type_set_translation_context (type, "cursor-precision");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_guides_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_GUIDES_NONE, "PICMAN_GUIDES_NONE", "none" },
    { PICMAN_GUIDES_CENTER_LINES, "PICMAN_GUIDES_CENTER_LINES", "center-lines" },
    { PICMAN_GUIDES_THIRDS, "PICMAN_GUIDES_THIRDS", "thirds" },
    { PICMAN_GUIDES_FIFTHS, "PICMAN_GUIDES_FIFTHS", "fifths" },
    { PICMAN_GUIDES_GOLDEN, "PICMAN_GUIDES_GOLDEN", "golden" },
    { PICMAN_GUIDES_DIAGONALS, "PICMAN_GUIDES_DIAGONALS", "diagonals" },
    { PICMAN_GUIDES_N_LINES, "PICMAN_GUIDES_N_LINES", "n-lines" },
    { PICMAN_GUIDES_SPACING, "PICMAN_GUIDES_SPACING", "spacing" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_GUIDES_NONE, NC_("guides-type", "No guides"), NULL },
    { PICMAN_GUIDES_CENTER_LINES, NC_("guides-type", "Center lines"), NULL },
    { PICMAN_GUIDES_THIRDS, NC_("guides-type", "Rule of thirds"), NULL },
    { PICMAN_GUIDES_FIFTHS, NC_("guides-type", "Rule of fifths"), NULL },
    { PICMAN_GUIDES_GOLDEN, NC_("guides-type", "Golden sections"), NULL },
    { PICMAN_GUIDES_DIAGONALS, NC_("guides-type", "Diagonal lines"), NULL },
    { PICMAN_GUIDES_N_LINES, NC_("guides-type", "Number of lines"), NULL },
    { PICMAN_GUIDES_SPACING, NC_("guides-type", "Line spacing"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanGuidesType", values);
      picman_type_set_translation_context (type, "guides-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_handle_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HANDLE_SQUARE, "PICMAN_HANDLE_SQUARE", "square" },
    { PICMAN_HANDLE_FILLED_SQUARE, "PICMAN_HANDLE_FILLED_SQUARE", "filled-square" },
    { PICMAN_HANDLE_CIRCLE, "PICMAN_HANDLE_CIRCLE", "circle" },
    { PICMAN_HANDLE_FILLED_CIRCLE, "PICMAN_HANDLE_FILLED_CIRCLE", "filled-circle" },
    { PICMAN_HANDLE_DIAMOND, "PICMAN_HANDLE_DIAMOND", "diamond" },
    { PICMAN_HANDLE_FILLED_DIAMOND, "PICMAN_HANDLE_FILLED_DIAMOND", "filled-diamond" },
    { PICMAN_HANDLE_CROSS, "PICMAN_HANDLE_CROSS", "cross" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HANDLE_SQUARE, "PICMAN_HANDLE_SQUARE", NULL },
    { PICMAN_HANDLE_FILLED_SQUARE, "PICMAN_HANDLE_FILLED_SQUARE", NULL },
    { PICMAN_HANDLE_CIRCLE, "PICMAN_HANDLE_CIRCLE", NULL },
    { PICMAN_HANDLE_FILLED_CIRCLE, "PICMAN_HANDLE_FILLED_CIRCLE", NULL },
    { PICMAN_HANDLE_DIAMOND, "PICMAN_HANDLE_DIAMOND", NULL },
    { PICMAN_HANDLE_FILLED_DIAMOND, "PICMAN_HANDLE_FILLED_DIAMOND", NULL },
    { PICMAN_HANDLE_CROSS, "PICMAN_HANDLE_CROSS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHandleType", values);
      picman_type_set_translation_context (type, "handle-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_handle_anchor_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HANDLE_ANCHOR_CENTER, "PICMAN_HANDLE_ANCHOR_CENTER", "center" },
    { PICMAN_HANDLE_ANCHOR_NORTH, "PICMAN_HANDLE_ANCHOR_NORTH", "north" },
    { PICMAN_HANDLE_ANCHOR_NORTH_WEST, "PICMAN_HANDLE_ANCHOR_NORTH_WEST", "north-west" },
    { PICMAN_HANDLE_ANCHOR_NORTH_EAST, "PICMAN_HANDLE_ANCHOR_NORTH_EAST", "north-east" },
    { PICMAN_HANDLE_ANCHOR_SOUTH, "PICMAN_HANDLE_ANCHOR_SOUTH", "south" },
    { PICMAN_HANDLE_ANCHOR_SOUTH_WEST, "PICMAN_HANDLE_ANCHOR_SOUTH_WEST", "south-west" },
    { PICMAN_HANDLE_ANCHOR_SOUTH_EAST, "PICMAN_HANDLE_ANCHOR_SOUTH_EAST", "south-east" },
    { PICMAN_HANDLE_ANCHOR_WEST, "PICMAN_HANDLE_ANCHOR_WEST", "west" },
    { PICMAN_HANDLE_ANCHOR_EAST, "PICMAN_HANDLE_ANCHOR_EAST", "east" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HANDLE_ANCHOR_CENTER, "PICMAN_HANDLE_ANCHOR_CENTER", NULL },
    { PICMAN_HANDLE_ANCHOR_NORTH, "PICMAN_HANDLE_ANCHOR_NORTH", NULL },
    { PICMAN_HANDLE_ANCHOR_NORTH_WEST, "PICMAN_HANDLE_ANCHOR_NORTH_WEST", NULL },
    { PICMAN_HANDLE_ANCHOR_NORTH_EAST, "PICMAN_HANDLE_ANCHOR_NORTH_EAST", NULL },
    { PICMAN_HANDLE_ANCHOR_SOUTH, "PICMAN_HANDLE_ANCHOR_SOUTH", NULL },
    { PICMAN_HANDLE_ANCHOR_SOUTH_WEST, "PICMAN_HANDLE_ANCHOR_SOUTH_WEST", NULL },
    { PICMAN_HANDLE_ANCHOR_SOUTH_EAST, "PICMAN_HANDLE_ANCHOR_SOUTH_EAST", NULL },
    { PICMAN_HANDLE_ANCHOR_WEST, "PICMAN_HANDLE_ANCHOR_WEST", NULL },
    { PICMAN_HANDLE_ANCHOR_EAST, "PICMAN_HANDLE_ANCHOR_EAST", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHandleAnchor", values);
      picman_type_set_translation_context (type, "handle-anchor");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_path_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PATH_STYLE_DEFAULT, "PICMAN_PATH_STYLE_DEFAULT", "default" },
    { PICMAN_PATH_STYLE_VECTORS, "PICMAN_PATH_STYLE_VECTORS", "vectors" },
    { PICMAN_PATH_STYLE_OUTLINE, "PICMAN_PATH_STYLE_OUTLINE", "outline" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PATH_STYLE_DEFAULT, "PICMAN_PATH_STYLE_DEFAULT", NULL },
    { PICMAN_PATH_STYLE_VECTORS, "PICMAN_PATH_STYLE_VECTORS", NULL },
    { PICMAN_PATH_STYLE_OUTLINE, "PICMAN_PATH_STYLE_OUTLINE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPathStyle", values);
      picman_type_set_translation_context (type, "path-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_zoom_focus_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ZOOM_FOCUS_BEST_GUESS, "PICMAN_ZOOM_FOCUS_BEST_GUESS", "best-guess" },
    { PICMAN_ZOOM_FOCUS_POINTER, "PICMAN_ZOOM_FOCUS_POINTER", "pointer" },
    { PICMAN_ZOOM_FOCUS_IMAGE_CENTER, "PICMAN_ZOOM_FOCUS_IMAGE_CENTER", "image-center" },
    { PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS, "PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS", "retain-centering-else-best-guess" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ZOOM_FOCUS_BEST_GUESS, "PICMAN_ZOOM_FOCUS_BEST_GUESS", NULL },
    { PICMAN_ZOOM_FOCUS_POINTER, "PICMAN_ZOOM_FOCUS_POINTER", NULL },
    { PICMAN_ZOOM_FOCUS_IMAGE_CENTER, "PICMAN_ZOOM_FOCUS_IMAGE_CENTER", NULL },
    { PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS, "PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanZoomFocus", values);
      picman_type_set_translation_context (type, "zoom-focus");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

