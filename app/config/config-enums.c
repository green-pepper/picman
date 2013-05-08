
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "config-enums.h"
#include"picman-intl.h"

/* enumerations from "./config-enums.h" */
GType
picman_cursor_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CURSOR_MODE_TOOL_ICON, "PICMAN_CURSOR_MODE_TOOL_ICON", "tool-icon" },
    { PICMAN_CURSOR_MODE_TOOL_CROSSHAIR, "PICMAN_CURSOR_MODE_TOOL_CROSSHAIR", "tool-crosshair" },
    { PICMAN_CURSOR_MODE_CROSSHAIR, "PICMAN_CURSOR_MODE_CROSSHAIR", "crosshair" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CURSOR_MODE_TOOL_ICON, NC_("cursor-mode", "Tool icon"), NULL },
    { PICMAN_CURSOR_MODE_TOOL_CROSSHAIR, NC_("cursor-mode", "Tool icon with crosshair"), NULL },
    { PICMAN_CURSOR_MODE_CROSSHAIR, NC_("cursor-mode", "Crosshair only"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCursorMode", values);
      picman_type_set_translation_context (type, "cursor-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_canvas_padding_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CANVAS_PADDING_MODE_DEFAULT, "PICMAN_CANVAS_PADDING_MODE_DEFAULT", "default" },
    { PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK, "PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK", "light-check" },
    { PICMAN_CANVAS_PADDING_MODE_DARK_CHECK, "PICMAN_CANVAS_PADDING_MODE_DARK_CHECK", "dark-check" },
    { PICMAN_CANVAS_PADDING_MODE_CUSTOM, "PICMAN_CANVAS_PADDING_MODE_CUSTOM", "custom" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CANVAS_PADDING_MODE_DEFAULT, NC_("canvas-padding-mode", "From theme"), NULL },
    { PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK, NC_("canvas-padding-mode", "Light check color"), NULL },
    { PICMAN_CANVAS_PADDING_MODE_DARK_CHECK, NC_("canvas-padding-mode", "Dark check color"), NULL },
    { PICMAN_CANVAS_PADDING_MODE_CUSTOM, NC_("canvas-padding-mode", "Custom color"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCanvasPaddingMode", values);
      picman_type_set_translation_context (type, "canvas-padding-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_space_bar_action_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_SPACE_BAR_ACTION_NONE, "PICMAN_SPACE_BAR_ACTION_NONE", "none" },
    { PICMAN_SPACE_BAR_ACTION_PAN, "PICMAN_SPACE_BAR_ACTION_PAN", "pan" },
    { PICMAN_SPACE_BAR_ACTION_MOVE, "PICMAN_SPACE_BAR_ACTION_MOVE", "move" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_SPACE_BAR_ACTION_NONE, NC_("space-bar-action", "No action"), NULL },
    { PICMAN_SPACE_BAR_ACTION_PAN, NC_("space-bar-action", "Pan view"), NULL },
    { PICMAN_SPACE_BAR_ACTION_MOVE, NC_("space-bar-action", "Switch to Move tool"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanSpaceBarAction", values);
      picman_type_set_translation_context (type, "space-bar-action");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_zoom_quality_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ZOOM_QUALITY_LOW, "PICMAN_ZOOM_QUALITY_LOW", "low" },
    { PICMAN_ZOOM_QUALITY_HIGH, "PICMAN_ZOOM_QUALITY_HIGH", "high" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ZOOM_QUALITY_LOW, NC_("zoom-quality", "Low"), NULL },
    { PICMAN_ZOOM_QUALITY_HIGH, NC_("zoom-quality", "High"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanZoomQuality", values);
      picman_type_set_translation_context (type, "zoom-quality");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_help_browser_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HELP_BROWSER_PICMAN, "PICMAN_HELP_BROWSER_PICMAN", "picman" },
    { PICMAN_HELP_BROWSER_WEB_BROWSER, "PICMAN_HELP_BROWSER_WEB_BROWSER", "web-browser" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HELP_BROWSER_PICMAN, NC_("help-browser-type", "PICMAN help browser"), NULL },
    { PICMAN_HELP_BROWSER_WEB_BROWSER, NC_("help-browser-type", "Web browser"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHelpBrowserType", values);
      picman_type_set_translation_context (type, "help-browser-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_window_hint_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_WINDOW_HINT_NORMAL, "PICMAN_WINDOW_HINT_NORMAL", "normal" },
    { PICMAN_WINDOW_HINT_UTILITY, "PICMAN_WINDOW_HINT_UTILITY", "utility" },
    { PICMAN_WINDOW_HINT_KEEP_ABOVE, "PICMAN_WINDOW_HINT_KEEP_ABOVE", "keep-above" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_WINDOW_HINT_NORMAL, NC_("window-hint", "Normal window"), NULL },
    { PICMAN_WINDOW_HINT_UTILITY, NC_("window-hint", "Utility window"), NULL },
    { PICMAN_WINDOW_HINT_KEEP_ABOVE, NC_("window-hint", "Keep above"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanWindowHint", values);
      picman_type_set_translation_context (type, "window-hint");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_cursor_format_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CURSOR_FORMAT_BITMAP, "PICMAN_CURSOR_FORMAT_BITMAP", "bitmap" },
    { PICMAN_CURSOR_FORMAT_PIXBUF, "PICMAN_CURSOR_FORMAT_PIXBUF", "pixbuf" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CURSOR_FORMAT_BITMAP, NC_("cursor-format", "Black & white"), NULL },
    { PICMAN_CURSOR_FORMAT_PIXBUF, NC_("cursor-format", "Fancy"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCursorFormat", values);
      picman_type_set_translation_context (type, "cursor-format");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_handedness_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HANDEDNESS_LEFT, "PICMAN_HANDEDNESS_LEFT", "left" },
    { PICMAN_HANDEDNESS_RIGHT, "PICMAN_HANDEDNESS_RIGHT", "right" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HANDEDNESS_LEFT, NC_("handedness", "Left-handed"), NULL },
    { PICMAN_HANDEDNESS_RIGHT, NC_("handedness", "Right-handed"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHandedness", values);
      picman_type_set_translation_context (type, "handedness");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

