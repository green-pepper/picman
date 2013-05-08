
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "picmancolorconfig-enums.h"
#include "libpicman/libpicman-intl.h"

/* enumerations from "./picmancolorconfig-enums.h" */
GType
picman_color_management_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_MANAGEMENT_OFF, "PICMAN_COLOR_MANAGEMENT_OFF", "off" },
    { PICMAN_COLOR_MANAGEMENT_DISPLAY, "PICMAN_COLOR_MANAGEMENT_DISPLAY", "display" },
    { PICMAN_COLOR_MANAGEMENT_SOFTPROOF, "PICMAN_COLOR_MANAGEMENT_SOFTPROOF", "softproof" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_MANAGEMENT_OFF, NC_("color-management-mode", "No color management"), NULL },
    { PICMAN_COLOR_MANAGEMENT_DISPLAY, NC_("color-management-mode", "Color managed display"), NULL },
    { PICMAN_COLOR_MANAGEMENT_SOFTPROOF, NC_("color-management-mode", "Print simulation"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorManagementMode", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "color-management-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_rendering_intent_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_RENDERING_INTENT_PERCEPTUAL, "PICMAN_COLOR_RENDERING_INTENT_PERCEPTUAL", "perceptual" },
    { PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC, "PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC", "relative-colorimetric" },
    { PICMAN_COLOR_RENDERING_INTENT_SATURATION, "PICMAN_COLOR_RENDERING_INTENT_SATURATION", "saturation" },
    { PICMAN_COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC, "PICMAN_COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC", "absolute-colorimetric" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_RENDERING_INTENT_PERCEPTUAL, NC_("color-rendering-intent", "Perceptual"), NULL },
    { PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC, NC_("color-rendering-intent", "Relative colorimetric"), NULL },
    { PICMAN_COLOR_RENDERING_INTENT_SATURATION, NC_("color-rendering-intent", "Saturation"), NULL },
    { PICMAN_COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC, NC_("color-rendering-intent", "Absolute colorimetric"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorRenderingIntent", values);
      picman_type_set_translation_domain (type, GETTEXT_PACKAGE "-libpicman");
      picman_type_set_translation_context (type, "color-rendering-intent");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

