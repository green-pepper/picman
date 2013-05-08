
/* Generated data (by picman-mkenums) */

#include "config.h"
#include <gtk/gtk.h>
#include "libpicmanbase/picmanbase.h"
#include "widgets-enums.h"
#include "picman-intl.h"

/* enumerations from "./widgets-enums.h" */
GType
picman_active_color_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_ACTIVE_COLOR_FOREGROUND, "PICMAN_ACTIVE_COLOR_FOREGROUND", "foreground" },
    { PICMAN_ACTIVE_COLOR_BACKGROUND, "PICMAN_ACTIVE_COLOR_BACKGROUND", "background" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_ACTIVE_COLOR_FOREGROUND, NC_("active-color", "Foreground"), NULL },
    { PICMAN_ACTIVE_COLOR_BACKGROUND, NC_("active-color", "Background"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanActiveColor", values);
      picman_type_set_translation_context (type, "active-color");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_dialog_state_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_DIALOG_OK, "PICMAN_COLOR_DIALOG_OK", "ok" },
    { PICMAN_COLOR_DIALOG_CANCEL, "PICMAN_COLOR_DIALOG_CANCEL", "cancel" },
    { PICMAN_COLOR_DIALOG_UPDATE, "PICMAN_COLOR_DIALOG_UPDATE", "update" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_DIALOG_OK, "PICMAN_COLOR_DIALOG_OK", NULL },
    { PICMAN_COLOR_DIALOG_CANCEL, "PICMAN_COLOR_DIALOG_CANCEL", NULL },
    { PICMAN_COLOR_DIALOG_UPDATE, "PICMAN_COLOR_DIALOG_UPDATE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorDialogState", values);
      picman_type_set_translation_context (type, "color-dialog-state");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_frame_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_FRAME_MODE_PIXEL, "PICMAN_COLOR_FRAME_MODE_PIXEL", "pixel" },
    { PICMAN_COLOR_FRAME_MODE_RGB, "PICMAN_COLOR_FRAME_MODE_RGB", "rgb" },
    { PICMAN_COLOR_FRAME_MODE_HSV, "PICMAN_COLOR_FRAME_MODE_HSV", "hsv" },
    { PICMAN_COLOR_FRAME_MODE_CMYK, "PICMAN_COLOR_FRAME_MODE_CMYK", "cmyk" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_FRAME_MODE_PIXEL, NC_("color-frame-mode", "Pixel"), NULL },
    { PICMAN_COLOR_FRAME_MODE_RGB, NC_("color-frame-mode", "RGB"), NULL },
    { PICMAN_COLOR_FRAME_MODE_HSV, NC_("color-frame-mode", "HSV"), NULL },
    { PICMAN_COLOR_FRAME_MODE_CMYK, NC_("color-frame-mode", "CMYK"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorFrameMode", values);
      picman_type_set_translation_context (type, "color-frame-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_pick_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_PICK_MODE_NONE, "PICMAN_COLOR_PICK_MODE_NONE", "none" },
    { PICMAN_COLOR_PICK_MODE_FOREGROUND, "PICMAN_COLOR_PICK_MODE_FOREGROUND", "foreground" },
    { PICMAN_COLOR_PICK_MODE_BACKGROUND, "PICMAN_COLOR_PICK_MODE_BACKGROUND", "background" },
    { PICMAN_COLOR_PICK_MODE_PALETTE, "PICMAN_COLOR_PICK_MODE_PALETTE", "palette" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_PICK_MODE_NONE, NC_("color-pick-mode", "Pick only"), NULL },
    { PICMAN_COLOR_PICK_MODE_FOREGROUND, NC_("color-pick-mode", "Set foreground color"), NULL },
    { PICMAN_COLOR_PICK_MODE_BACKGROUND, NC_("color-pick-mode", "Set background color"), NULL },
    { PICMAN_COLOR_PICK_MODE_PALETTE, NC_("color-pick-mode", "Add to palette"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorPickMode", values);
      picman_type_set_translation_context (type, "color-pick-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_color_pick_state_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_COLOR_PICK_STATE_NEW, "PICMAN_COLOR_PICK_STATE_NEW", "new" },
    { PICMAN_COLOR_PICK_STATE_UPDATE, "PICMAN_COLOR_PICK_STATE_UPDATE", "update" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_COLOR_PICK_STATE_NEW, "PICMAN_COLOR_PICK_STATE_NEW", NULL },
    { PICMAN_COLOR_PICK_STATE_UPDATE, "PICMAN_COLOR_PICK_STATE_UPDATE", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanColorPickState", values);
      picman_type_set_translation_context (type, "color-pick-state");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_histogram_scale_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_HISTOGRAM_SCALE_LINEAR, "PICMAN_HISTOGRAM_SCALE_LINEAR", "linear" },
    { PICMAN_HISTOGRAM_SCALE_LOGARITHMIC, "PICMAN_HISTOGRAM_SCALE_LOGARITHMIC", "logarithmic" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_HISTOGRAM_SCALE_LINEAR, NC_("histogram-scale", "Linear histogram"), NULL },
    { PICMAN_HISTOGRAM_SCALE_LOGARITHMIC, NC_("histogram-scale", "Logarithmic histogram"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanHistogramScale", values);
      picman_type_set_translation_context (type, "histogram-scale");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_tab_style_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TAB_STYLE_ICON, "PICMAN_TAB_STYLE_ICON", "icon" },
    { PICMAN_TAB_STYLE_PREVIEW, "PICMAN_TAB_STYLE_PREVIEW", "preview" },
    { PICMAN_TAB_STYLE_NAME, "PICMAN_TAB_STYLE_NAME", "name" },
    { PICMAN_TAB_STYLE_BLURB, "PICMAN_TAB_STYLE_BLURB", "blurb" },
    { PICMAN_TAB_STYLE_ICON_NAME, "PICMAN_TAB_STYLE_ICON_NAME", "icon-name" },
    { PICMAN_TAB_STYLE_ICON_BLURB, "PICMAN_TAB_STYLE_ICON_BLURB", "icon-blurb" },
    { PICMAN_TAB_STYLE_PREVIEW_NAME, "PICMAN_TAB_STYLE_PREVIEW_NAME", "preview-name" },
    { PICMAN_TAB_STYLE_PREVIEW_BLURB, "PICMAN_TAB_STYLE_PREVIEW_BLURB", "preview-blurb" },
    { PICMAN_TAB_STYLE_UNDEFINED, "PICMAN_TAB_STYLE_UNDEFINED", "undefined" },
    { PICMAN_TAB_STYLE_AUTOMATIC, "PICMAN_TAB_STYLE_AUTOMATIC", "automatic" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TAB_STYLE_ICON, NC_("tab-style", "Icon"), NULL },
    { PICMAN_TAB_STYLE_PREVIEW, NC_("tab-style", "Current status"), NULL },
    { PICMAN_TAB_STYLE_NAME, NC_("tab-style", "Text"), NULL },
    { PICMAN_TAB_STYLE_BLURB, NC_("tab-style", "Description"), NULL },
    { PICMAN_TAB_STYLE_ICON_NAME, NC_("tab-style", "Icon & text"), NULL },
    { PICMAN_TAB_STYLE_ICON_BLURB, NC_("tab-style", "Icon & desc"), NULL },
    { PICMAN_TAB_STYLE_PREVIEW_NAME, NC_("tab-style", "Status & text"), NULL },
    { PICMAN_TAB_STYLE_PREVIEW_BLURB, NC_("tab-style", "Status & desc"), NULL },
    { PICMAN_TAB_STYLE_UNDEFINED, NC_("tab-style", "Undefined"), NULL },
    { PICMAN_TAB_STYLE_AUTOMATIC, NC_("tab-style", "Automatic"), NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTabStyle", values);
      picman_type_set_translation_context (type, "tab-style");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}

GType
picman_tag_entry_mode_get_type (void)
{
  static const GEnumValue values[] =
  {
    { PICMAN_TAG_ENTRY_MODE_QUERY, "PICMAN_TAG_ENTRY_MODE_QUERY", "query" },
    { PICMAN_TAG_ENTRY_MODE_ASSIGN, "PICMAN_TAG_ENTRY_MODE_ASSIGN", "assign" },
    { 0, NULL, NULL }
  };

  static const PicmanEnumDesc descs[] =
  {
    { PICMAN_TAG_ENTRY_MODE_QUERY, "PICMAN_TAG_ENTRY_MODE_QUERY", NULL },
    { PICMAN_TAG_ENTRY_MODE_ASSIGN, "PICMAN_TAG_ENTRY_MODE_ASSIGN", NULL },
    { 0, NULL, NULL }
  };

  static GType type = 0;

  if (G_UNLIKELY (! type))
    {
      type = g_enum_register_static ("PicmanTagEntryMode", values);
      picman_type_set_translation_context (type, "tag-entry-mode");
      picman_enum_set_value_descriptions (type, descs);
    }

  return type;
}


/* Generated data ends here */

