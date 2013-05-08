
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "paint-enums.h"
#include "picman-intl.h"

/* enumerations from "./paint-enums.h" */
GType
picman_brush_application_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_BRUSH_HARD, "PICMAN_BRUSH_HARD", "hard" },
    { PICMAN_BRUSH_SOFT, "PICMAN_BRUSH_SOFT", "soft" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_BRUSH_HARD, "PICMAN_BRUSH_HARD", NULL },
    { PICMAN_BRUSH_SOFT, "PICMAN_BRUSH_SOFT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanBrushApplicationMode", values);
      picman_type_set_translation_context (type, "brush-application-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_perspective_clone_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST, "PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST", "adjust" },
    { PICMAN_PERSPECTIVE_CLONE_MODE_PAINT, "PICMAN_PERSPECTIVE_CLONE_MODE_PAINT", "paint" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_PERSPECTIVE_CLONE_MODE_ADJUST, NC_("perspective-clone-mode", "Modify Perspective"), NULL },
    { PICMAN_PERSPECTIVE_CLONE_MODE_PAINT, NC_("perspective-clone-mode", "Perspective Clone"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanPerspectiveCloneMode", values);
      picman_type_set_translation_context (type, "perspective-clone-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_source_align_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_SOURCE_ALIGN_NO, "PICMAN_SOURCE_ALIGN_NO", "no" },
    { PICMAN_SOURCE_ALIGN_YES, "PICMAN_SOURCE_ALIGN_YES", "yes" },
    { PICMAN_SOURCE_ALIGN_REGISTERED, "PICMAN_SOURCE_ALIGN_REGISTERED", "registered" },
    { PICMAN_SOURCE_ALIGN_FIXED, "PICMAN_SOURCE_ALIGN_FIXED", "fixed" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_SOURCE_ALIGN_NO, NC_("source-align-mode", "None"), NULL },
    { PICMAN_SOURCE_ALIGN_YES, NC_("source-align-mode", "Aligned"), NULL },
    { PICMAN_SOURCE_ALIGN_REGISTERED, NC_("source-align-mode", "Registered"), NULL },
    { PICMAN_SOURCE_ALIGN_FIXED, NC_("source-align-mode", "Fixed"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanSourceAlignMode", values);
      picman_type_set_translation_context (type, "source-align-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_convolve_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_BLUR_CONVOLVE, "PICMAN_BLUR_CONVOLVE", "blur-convolve" },
    { PICMAN_SHARPEN_CONVOLVE, "PICMAN_SHARPEN_CONVOLVE", "sharpen-convolve" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_BLUR_CONVOLVE, NC_("convolve-type", "Blur"), NULL },
    { PICMAN_SHARPEN_CONVOLVE, NC_("convolve-type", "Sharpen"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanConvolveType", values);
      picman_type_set_translation_context (type, "convolve-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_ink_blob_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_INK_BLOB_TYPE_CIRCLE, "PICMAN_INK_BLOB_TYPE_CIRCLE", "circle" },
    { PICMAN_INK_BLOB_TYPE_SQUARE, "PICMAN_INK_BLOB_TYPE_SQUARE", "square" },
    { PICMAN_INK_BLOB_TYPE_DIAMOND, "PICMAN_INK_BLOB_TYPE_DIAMOND", "diamond" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_INK_BLOB_TYPE_CIRCLE, NC_("ink-blob-type", "Circle"), NULL },
    { PICMAN_INK_BLOB_TYPE_SQUARE, NC_("ink-blob-type", "Square"), NULL },
    { PICMAN_INK_BLOB_TYPE_DIAMOND, NC_("ink-blob-type", "Diamond"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanInkBlobType", values);
      picman_type_set_translation_context (type, "ink-blob-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

