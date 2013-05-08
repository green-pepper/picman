
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "core/core-enums.h"
#include "picman-gegl-enums.h"
#include "picman-intl.h"

/* enumerations from "./picman-gegl-enums.h" */
GType
picman_cage_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_CAGE_MODE_CAGE_CHANGE, "PICMAN_CAGE_MODE_CAGE_CHANGE", "cage-change" },
    { PICMAN_CAGE_MODE_DEFORM, "PICMAN_CAGE_MODE_DEFORM", "deform" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_CAGE_MODE_CAGE_CHANGE, NC_("cage-mode", "Create or adjust the cage"), NULL },
    { PICMAN_CAGE_MODE_DEFORM, NC_("cage-mode", "Deform the cage\nto deform the image"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanCageMode", values);
      picman_type_set_translation_context (type, "cage-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

