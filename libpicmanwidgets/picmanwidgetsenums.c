
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "picmanwidgetsenums.h"
#include "libpicman/libpicman-intl.h"

/* enumerations from "./picmanwidgetsenums.h" */
GType
picman_aspect_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ASPECT_SQUARE, "PICMAN_ASPECT_SQUARE", "square" },
    { PICMAN_ASPECT_PORTRAIT, "PICMAN_ASPECT_PORTRAIT", "portrait" },
    { PICMAN_ASPECT_LANDSCAPE, "PICMAN_ASPECT_LANDSCAPE", "landscape" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ASPECT_SQUARE, NC_("aspect-type", "Square"), NULL },
    { PICMAN_ASPECT_PORTRAIT, NC_("aspect-type", "Portrait"), NULL },
    { PICMAN_ASPECT_LANDSCAPE, NC_("aspect-type", "Landscape"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanAspectType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "aspect-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_chain_position_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CHAIN_TOP, "PICMAN_CHAIN_TOP", "top" },
    { PICMAN_CHAIN_LEFT, "PICMAN_CHAIN_LEFT", "left" },
    { PICMAN_CHAIN_BOTTOM, "PICMAN_CHAIN_BOTTOM", "bottom" },
    { PICMAN_CHAIN_RIGHT, "PICMAN_CHAIN_RIGHT", "right" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CHAIN_TOP, "PICMAN_CHAIN_TOP", NULL },
    { PICMAN_CHAIN_LEFT, "PICMAN_CHAIN_LEFT", NULL },
    { PICMAN_CHAIN_BOTTOM, "PICMAN_CHAIN_BOTTOM", NULL },
    { PICMAN_CHAIN_RIGHT, "PICMAN_CHAIN_RIGHT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanChainPosition", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "chain-position");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_area_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_AREA_FLAT, "PICMAN_COLOR_AREA_FLAT", "flat" },
    { PICMAN_COLOR_AREA_SMALL_CHECKS, "PICMAN_COLOR_AREA_SMALL_CHECKS", "small-checks" },
    { PICMAN_COLOR_AREA_LARGE_CHECKS, "PICMAN_COLOR_AREA_LARGE_CHECKS", "large-checks" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_AREA_FLAT, "PICMAN_COLOR_AREA_FLAT", NULL },
    { PICMAN_COLOR_AREA_SMALL_CHECKS, "PICMAN_COLOR_AREA_SMALL_CHECKS", NULL },
    { PICMAN_COLOR_AREA_LARGE_CHECKS, "PICMAN_COLOR_AREA_LARGE_CHECKS", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorAreaType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "color-area-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_selector_channel_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_SELECTOR_HUE, "PICMAN_COLOR_SELECTOR_HUE", "hue" },
    { PICMAN_COLOR_SELECTOR_SATURATION, "PICMAN_COLOR_SELECTOR_SATURATION", "saturation" },
    { PICMAN_COLOR_SELECTOR_VALUE, "PICMAN_COLOR_SELECTOR_VALUE", "value" },
    { PICMAN_COLOR_SELECTOR_RED, "PICMAN_COLOR_SELECTOR_RED", "red" },
    { PICMAN_COLOR_SELECTOR_GREEN, "PICMAN_COLOR_SELECTOR_GREEN", "green" },
    { PICMAN_COLOR_SELECTOR_BLUE, "PICMAN_COLOR_SELECTOR_BLUE", "blue" },
    { PICMAN_COLOR_SELECTOR_ALPHA, "PICMAN_COLOR_SELECTOR_ALPHA", "alpha" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_SELECTOR_HUE, NC_("color-selector-channel", "_H"), N_("Hue") },
    { PICMAN_COLOR_SELECTOR_SATURATION, NC_("color-selector-channel", "_S"), N_("Saturation") },
    { PICMAN_COLOR_SELECTOR_VALUE, NC_("color-selector-channel", "_V"), N_("Value") },
    { PICMAN_COLOR_SELECTOR_RED, NC_("color-selector-channel", "_R"), N_("Red") },
    { PICMAN_COLOR_SELECTOR_GREEN, NC_("color-selector-channel", "_G"), N_("Green") },
    { PICMAN_COLOR_SELECTOR_BLUE, NC_("color-selector-channel", "_B"), N_("Blue") },
    { PICMAN_COLOR_SELECTOR_ALPHA, NC_("color-selector-channel", "_A"), N_("Alpha") },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorSelectorChannel", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "color-selector-channel");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_page_selector_target_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PAGE_SELECTOR_TARGET_LAYERS, "PICMAN_PAGE_SELECTOR_TARGET_LAYERS", "layers" },
    { PICMAN_PAGE_SELECTOR_TARGET_IMAGES, "PICMAN_PAGE_SELECTOR_TARGET_IMAGES", "images" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PAGE_SELECTOR_TARGET_LAYERS, NC_("page-selector-target", "Layers"), NULL },
    { PICMAN_PAGE_SELECTOR_TARGET_IMAGES, NC_("page-selector-target", "Images"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPageSelectorTarget", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "page-selector-target");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_size_entry_update_policy_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_SIZE_ENTRY_UPDATE_NONE, "PICMAN_SIZE_ENTRY_UPDATE_NONE", "none" },
    { PICMAN_SIZE_ENTRY_UPDATE_SIZE, "PICMAN_SIZE_ENTRY_UPDATE_SIZE", "size" },
    { PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION, "PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION", "resolution" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_SIZE_ENTRY_UPDATE_NONE, "PICMAN_SIZE_ENTRY_UPDATE_NONE", NULL },
    { PICMAN_SIZE_ENTRY_UPDATE_SIZE, "PICMAN_SIZE_ENTRY_UPDATE_SIZE", NULL },
    { PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION, "PICMAN_SIZE_ENTRY_UPDATE_RESOLUTION", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanSizeEntryUpdatePolicy", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "size-entry-update-policy");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_zoom_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ZOOM_IN, "PICMAN_ZOOM_IN", "in" },
    { PICMAN_ZOOM_OUT, "PICMAN_ZOOM_OUT", "out" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ZOOM_IN, NC_("zoom-type", "Zoom in"), NULL },
    { PICMAN_ZOOM_OUT, NC_("zoom-type", "Zoom out"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanZoomType", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "zoom-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

