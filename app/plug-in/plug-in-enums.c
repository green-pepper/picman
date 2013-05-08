
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "plug-in-enums.h"
#include "picman-intl.h"

/* enumerations from "./plug-in-enums.h" */
GType
picman_plug_in_image_type_get_type (void)
{
  static const GFlagsValue values[] =
  {
    { PICMAN_PLUG_IN_RGB_IMAGE, "PICMAN_PLUG_IN_RGB_IMAGE", "rgb-image" },
    { PICMAN_PLUG_IN_GRAY_IMAGE, "PICMAN_PLUG_IN_GRAY_IMAGE", "gray-image" },
    { PICMAN_PLUG_IN_INDEXED_IMAGE, "PICMAN_PLUG_IN_INDEXED_IMAGE", "indexed-image" },
    { PICMAN_PLUG_IN_RGBA_IMAGE, "PICMAN_PLUG_IN_RGBA_IMAGE", "rgba-image" },
    { PICMAN_PLUG_IN_GRAYA_IMAGE, "PICMAN_PLUG_IN_GRAYA_IMAGE", "graya-image" },
    { PICMAN_PLUG_IN_INDEXEDA_IMAGE, "PICMAN_PLUG_IN_INDEXEDA_IMAGE", "indexeda-image" },
    { 0, NULL, NULL }
  };

  static const PicmanFlagsDesc descs[] =
  {
    { PICMAN_PLUG_IN_RGB_IMAGE, "PICMAN_PLUG_IN_RGB_IMAGE", NULL },
    { PICMAN_PLUG_IN_GRAY_IMAGE, "PICMAN_PLUG_IN_GRAY_IMAGE", NULL },
    { PICMAN_PLUG_IN_INDEXED_IMAGE, "PICMAN_PLUG_IN_INDEXED_IMAGE", NULL },
    { PICMAN_PLUG_IN_RGBA_IMAGE, "PICMAN_PLUG_IN_RGBA_IMAGE", NULL },
    { PICMAN_PLUG_IN_GRAYA_IMAGE, "PICMAN_PLUG_IN_GRAYA_IMAGE", NULL },
    { PICMAN_PLUG_IN_INDEXEDA_IMAGE, "PICMAN_PLUG_IN_INDEXEDA_IMAGE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_flags_register_static ("PicmanPlugInImageType", values);
      picman_type_set_translation_context (type, "plug-in-image-type");
      picman_flags_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_plug_in_call_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PLUG_IN_CALL_NONE, "PICMAN_PLUG_IN_CALL_NONE", "none" },
    { PICMAN_PLUG_IN_CALL_RUN, "PICMAN_PLUG_IN_CALL_RUN", "run" },
    { PICMAN_PLUG_IN_CALL_QUERY, "PICMAN_PLUG_IN_CALL_QUERY", "query" },
    { PICMAN_PLUG_IN_CALL_INIT, "PICMAN_PLUG_IN_CALL_INIT", "init" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PLUG_IN_CALL_NONE, "PICMAN_PLUG_IN_CALL_NONE", NULL },
    { PICMAN_PLUG_IN_CALL_RUN, "PICMAN_PLUG_IN_CALL_RUN", NULL },
    { PICMAN_PLUG_IN_CALL_QUERY, "PICMAN_PLUG_IN_CALL_QUERY", NULL },
    { PICMAN_PLUG_IN_CALL_INIT, "PICMAN_PLUG_IN_CALL_INIT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPlugInCallMode", values);
      picman_type_set_translation_context (type, "plug-in-call-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

