
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "text-enums.h"
#include "picman-intl.h"

/* enumerations from "./text-enums.h" */
GType
picman_text_box_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TEXT_BOX_DYNAMIC, "PICMAN_TEXT_BOX_DYNAMIC", "dynamic" },
    { PICMAN_TEXT_BOX_FIXED, "PICMAN_TEXT_BOX_FIXED", "fixed" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TEXT_BOX_DYNAMIC, NC_("text-box-mode", "Dynamic"), NULL },
    { PICMAN_TEXT_BOX_FIXED, NC_("text-box-mode", "Fixed"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTextBoxMode", values);
      picman_type_set_translation_context (type, "text-box-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_text_outline_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TEXT_OUTLINE_NONE, "PICMAN_TEXT_OUTLINE_NONE", "none" },
    { PICMAN_TEXT_OUTLINE_STROKE_ONLY, "PICMAN_TEXT_OUTLINE_STROKE_ONLY", "stroke-only" },
    { PICMAN_TEXT_OUTLINE_STROKE_FILL, "PICMAN_TEXT_OUTLINE_STROKE_FILL", "stroke-fill" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TEXT_OUTLINE_NONE, "PICMAN_TEXT_OUTLINE_NONE", NULL },
    { PICMAN_TEXT_OUTLINE_STROKE_ONLY, "PICMAN_TEXT_OUTLINE_STROKE_ONLY", NULL },
    { PICMAN_TEXT_OUTLINE_STROKE_FILL, "PICMAN_TEXT_OUTLINE_STROKE_FILL", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTextOutline", values);
      picman_type_set_translation_context (type, "text-outline");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

