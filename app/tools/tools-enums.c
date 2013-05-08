
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <glib-object.h>
#include "libpicmanbase/picmanbase.h"
#include "core/core-enums.h"
#include "tools-enums.h"
#include "picman-intl.h"

/* enumerations from "./tools-enums.h" */
GType
picman_button_press_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_BUTTON_PRESS_NORMAL, "PICMAN_BUTTON_PRESS_NORMAL", "normal" },
    { PICMAN_BUTTON_PRESS_DOUBLE, "PICMAN_BUTTON_PRESS_DOUBLE", "double" },
    { PICMAN_BUTTON_PRESS_TRIPLE, "PICMAN_BUTTON_PRESS_TRIPLE", "triple" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_BUTTON_PRESS_NORMAL, "PICMAN_BUTTON_PRESS_NORMAL", NULL },
    { PICMAN_BUTTON_PRESS_DOUBLE, "PICMAN_BUTTON_PRESS_DOUBLE", NULL },
    { PICMAN_BUTTON_PRESS_TRIPLE, "PICMAN_BUTTON_PRESS_TRIPLE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanButtonPressType", values);
      picman_type_set_translation_context (type, "button-press-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_button_release_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_BUTTON_RELEASE_NORMAL, "PICMAN_BUTTON_RELEASE_NORMAL", "normal" },
    { PICMAN_BUTTON_RELEASE_CANCEL, "PICMAN_BUTTON_RELEASE_CANCEL", "cancel" },
    { PICMAN_BUTTON_RELEASE_CLICK, "PICMAN_BUTTON_RELEASE_CLICK", "click" },
    { PICMAN_BUTTON_RELEASE_NO_MOTION, "PICMAN_BUTTON_RELEASE_NO_MOTION", "no-motion" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_BUTTON_RELEASE_NORMAL, "PICMAN_BUTTON_RELEASE_NORMAL", NULL },
    { PICMAN_BUTTON_RELEASE_CANCEL, "PICMAN_BUTTON_RELEASE_CANCEL", NULL },
    { PICMAN_BUTTON_RELEASE_CLICK, "PICMAN_BUTTON_RELEASE_CLICK", NULL },
    { PICMAN_BUTTON_RELEASE_NO_MOTION, "PICMAN_BUTTON_RELEASE_NO_MOTION", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanButtonReleaseType", values);
      picman_type_set_translation_context (type, "button-release-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_rectangle_constraint_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RECTANGLE_CONSTRAIN_NONE, "PICMAN_RECTANGLE_CONSTRAIN_NONE", "none" },
    { PICMAN_RECTANGLE_CONSTRAIN_IMAGE, "PICMAN_RECTANGLE_CONSTRAIN_IMAGE", "image" },
    { PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE, "PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE", "drawable" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RECTANGLE_CONSTRAIN_NONE, "PICMAN_RECTANGLE_CONSTRAIN_NONE", NULL },
    { PICMAN_RECTANGLE_CONSTRAIN_IMAGE, "PICMAN_RECTANGLE_CONSTRAIN_IMAGE", NULL },
    { PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE, "PICMAN_RECTANGLE_CONSTRAIN_DRAWABLE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRectangleConstraint", values);
      picman_type_set_translation_context (type, "rectangle-constraint");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_rectangle_precision_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RECTANGLE_PRECISION_INT, "PICMAN_RECTANGLE_PRECISION_INT", "int" },
    { PICMAN_RECTANGLE_PRECISION_DOUBLE, "PICMAN_RECTANGLE_PRECISION_DOUBLE", "double" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RECTANGLE_PRECISION_INT, "PICMAN_RECTANGLE_PRECISION_INT", NULL },
    { PICMAN_RECTANGLE_PRECISION_DOUBLE, "PICMAN_RECTANGLE_PRECISION_DOUBLE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRectanglePrecision", values);
      picman_type_set_translation_context (type, "rectangle-precision");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_rectangle_tool_fixed_rule_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RECTANGLE_TOOL_FIXED_ASPECT, "PICMAN_RECTANGLE_TOOL_FIXED_ASPECT", "aspect" },
    { PICMAN_RECTANGLE_TOOL_FIXED_WIDTH, "PICMAN_RECTANGLE_TOOL_FIXED_WIDTH", "width" },
    { PICMAN_RECTANGLE_TOOL_FIXED_HEIGHT, "PICMAN_RECTANGLE_TOOL_FIXED_HEIGHT", "height" },
    { PICMAN_RECTANGLE_TOOL_FIXED_SIZE, "PICMAN_RECTANGLE_TOOL_FIXED_SIZE", "size" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RECTANGLE_TOOL_FIXED_ASPECT, NC_("rectangle-tool-fixed-rule", "Aspect ratio"), NULL },
    { PICMAN_RECTANGLE_TOOL_FIXED_WIDTH, NC_("rectangle-tool-fixed-rule", "Width"), NULL },
    { PICMAN_RECTANGLE_TOOL_FIXED_HEIGHT, NC_("rectangle-tool-fixed-rule", "Height"), NULL },
    { PICMAN_RECTANGLE_TOOL_FIXED_SIZE, NC_("rectangle-tool-fixed-rule", "Size"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRectangleToolFixedRule", values);
      picman_type_set_translation_context (type, "rectangle-tool-fixed-rule");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_rect_select_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_RECT_SELECT_MODE_FREE, "PICMAN_RECT_SELECT_MODE_FREE", "free" },
    { PICMAN_RECT_SELECT_MODE_FIXED_SIZE, "PICMAN_RECT_SELECT_MODE_FIXED_SIZE", "fixed-size" },
    { PICMAN_RECT_SELECT_MODE_FIXED_RATIO, "PICMAN_RECT_SELECT_MODE_FIXED_RATIO", "fixed-ratio" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_RECT_SELECT_MODE_FREE, NC_("rect-select-mode", "Free select"), NULL },
    { PICMAN_RECT_SELECT_MODE_FIXED_SIZE, NC_("rect-select-mode", "Fixed size"), NULL },
    { PICMAN_RECT_SELECT_MODE_FIXED_RATIO, NC_("rect-select-mode", "Fixed aspect ratio"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanRectSelectMode", values);
      picman_type_set_translation_context (type, "rect-select-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_transform_type_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TRANSFORM_TYPE_LAYER, "PICMAN_TRANSFORM_TYPE_LAYER", "layer" },
    { PICMAN_TRANSFORM_TYPE_SELECTION, "PICMAN_TRANSFORM_TYPE_SELECTION", "selection" },
    { PICMAN_TRANSFORM_TYPE_PATH, "PICMAN_TRANSFORM_TYPE_PATH", "path" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TRANSFORM_TYPE_LAYER, NC_("transform-type", "Layer"), NULL },
    { PICMAN_TRANSFORM_TYPE_SELECTION, NC_("transform-type", "Selection"), NULL },
    { PICMAN_TRANSFORM_TYPE_PATH, NC_("transform-type", "Path"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTransformType", values);
      picman_type_set_translation_context (type, "transform-type");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_vector_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_VECTOR_MODE_DESIGN, "PICMAN_VECTOR_MODE_DESIGN", "design" },
    { PICMAN_VECTOR_MODE_EDIT, "PICMAN_VECTOR_MODE_EDIT", "edit" },
    { PICMAN_VECTOR_MODE_MOVE, "PICMAN_VECTOR_MODE_MOVE", "move" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_VECTOR_MODE_DESIGN, NC_("vector-mode", "Design"), NULL },
    { PICMAN_VECTOR_MODE_EDIT, NC_("vector-mode", "Edit"), NULL },
    { PICMAN_VECTOR_MODE_MOVE, NC_("vector-mode", "Move"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanVectorMode", values);
      picman_type_set_translation_context (type, "vector-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_tool_action_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TOOL_ACTION_PAUSE, "PICMAN_TOOL_ACTION_PAUSE", "pause" },
    { PICMAN_TOOL_ACTION_RESUME, "PICMAN_TOOL_ACTION_RESUME", "resume" },
    { PICMAN_TOOL_ACTION_HALT, "PICMAN_TOOL_ACTION_HALT", "halt" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TOOL_ACTION_PAUSE, "PICMAN_TOOL_ACTION_PAUSE", NULL },
    { PICMAN_TOOL_ACTION_RESUME, "PICMAN_TOOL_ACTION_RESUME", NULL },
    { PICMAN_TOOL_ACTION_HALT, "PICMAN_TOOL_ACTION_HALT", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanToolAction", values);
      picman_type_set_translation_context (type, "tool-action");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

