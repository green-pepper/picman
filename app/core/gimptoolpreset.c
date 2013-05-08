/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picmantoolinfo.h"
#include "picmantooloptions.h"
#include "picmantoolpreset.h"
#include "picmantoolpreset-load.h"
#include "picmantoolpreset-save.h"

#include "picman-intl.h"


#define DEFAULT_USE_FG_BG    FALSE
#define DEFAULT_USE_BRUSH    TRUE
#define DEFAULT_USE_DYNAMICS TRUE
#define DEFAULT_USE_GRADIENT TRUE
#define DEFAULT_USE_PATTERN  TRUE
#define DEFAULT_USE_PALETTE  TRUE
#define DEFAULT_USE_FONT     TRUE

enum
{
  PROP_0,
  PROP_NAME,
  PROP_PICMAN,
  PROP_TOOL_OPTIONS,
  PROP_USE_FG_BG,
  PROP_USE_BRUSH,
  PROP_USE_DYNAMICS,
  PROP_USE_GRADIENT,
  PROP_USE_PATTERN,
  PROP_USE_PALETTE,
  PROP_USE_FONT
};


static void          picman_tool_preset_config_iface_init    (PicmanConfigInterface *iface);

static void          picman_tool_preset_constructed          (GObject          *object);
static void          picman_tool_preset_finalize             (GObject          *object);
static void          picman_tool_preset_set_property         (GObject          *object,
                                                            guint             property_id,
                                                            const GValue     *value,
                                                            GParamSpec       *pspec);
static void          picman_tool_preset_get_property         (GObject          *object,
                                                            guint             property_id,
                                                            GValue           *value,
                                                            GParamSpec       *pspec);
static void
             picman_tool_preset_dispatch_properties_changed  (GObject          *object,
                                                            guint             n_pspecs,
                                                            GParamSpec      **pspecs);

static const gchar * picman_tool_preset_get_extension        (PicmanData         *data);

static gboolean      picman_tool_preset_deserialize_property (PicmanConfig       *config,
                                                            guint             property_id,
                                                            GValue           *value,
                                                            GParamSpec       *pspec,
                                                            GScanner         *scanner,
                                                            GTokenType       *expected);

static void          picman_tool_preset_set_options          (PicmanToolPreset   *preset,
                                                            PicmanToolOptions  *options);
static void          picman_tool_preset_options_notify       (GObject          *tool_options,
                                                            const GParamSpec *pspec,
                                                            PicmanToolPreset   *preset);


G_DEFINE_TYPE_WITH_CODE (PicmanToolPreset, picman_tool_preset, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_tool_preset_config_iface_init))

#define parent_class picman_tool_preset_parent_class


static void
picman_tool_preset_class_init (PicmanToolPresetClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanDataClass *data_class   = PICMAN_DATA_CLASS (klass);

  object_class->constructed                 = picman_tool_preset_constructed;
  object_class->finalize                    = picman_tool_preset_finalize;
  object_class->set_property                = picman_tool_preset_set_property;
  object_class->get_property                = picman_tool_preset_get_property;
  object_class->dispatch_properties_changed = picman_tool_preset_dispatch_properties_changed;

  data_class->save                          = picman_tool_preset_save;
  data_class->get_extension                 = picman_tool_preset_get_extension;

  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_NAME,
                                   "name", NULL,
                                   "Unnamed",
                                   PICMAN_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_TOOL_OPTIONS,
                                   "tool-options", NULL,
                                   PICMAN_TYPE_TOOL_OPTIONS,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_FG_BG,
                                    "use-fg-bg", NULL,
                                    DEFAULT_USE_FG_BG,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_BRUSH,
                                    "use-brush", NULL,
                                    DEFAULT_USE_BRUSH,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_DYNAMICS,
                                    "use-dynamics", NULL,
                                    DEFAULT_USE_DYNAMICS,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_PATTERN,
                                    "use-pattern", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_PALETTE,
                                    "use-palette", NULL,
                                    DEFAULT_USE_PALETTE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_GRADIENT,
                                    "use-gradient", NULL,
                                    DEFAULT_USE_GRADIENT,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_FONT,
                                    "use-font", NULL,
                                    DEFAULT_USE_FONT,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_tool_preset_config_iface_init (PicmanConfigInterface *iface)
{
  iface->deserialize_property = picman_tool_preset_deserialize_property;
}

static void
picman_tool_preset_init (PicmanToolPreset *tool_preset)
{
  tool_preset->tool_options = NULL;
}

static void
picman_tool_preset_constructed (GObject *object)
{
  PicmanToolPreset *preset = PICMAN_TOOL_PRESET (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (preset->picman));
}

static void
picman_tool_preset_finalize (GObject *object)
{
  PicmanToolPreset *tool_preset = PICMAN_TOOL_PRESET (object);

  picman_tool_preset_set_options (tool_preset, NULL);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_tool_preset_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanToolPreset *tool_preset = PICMAN_TOOL_PRESET (object);

  switch (property_id)
    {
    case PROP_NAME:
      picman_object_set_name (PICMAN_OBJECT (tool_preset),
                            g_value_get_string (value));
      break;

    case PROP_PICMAN:
      tool_preset->picman = g_value_get_object (value); /* don't ref */
      break;

    case PROP_TOOL_OPTIONS:
      picman_tool_preset_set_options (tool_preset,
                                    PICMAN_TOOL_OPTIONS (g_value_get_object (value)));
      break;

    case PROP_USE_FG_BG:
      tool_preset->use_fg_bg = g_value_get_boolean (value);
      break;
    case PROP_USE_BRUSH:
      tool_preset->use_brush = g_value_get_boolean (value);
      break;
    case PROP_USE_DYNAMICS:
      tool_preset->use_dynamics = g_value_get_boolean (value);
      break;
    case PROP_USE_PATTERN:
      tool_preset->use_pattern = g_value_get_boolean (value);
      break;
    case PROP_USE_PALETTE:
      tool_preset->use_palette = g_value_get_boolean (value);
      break;
    case PROP_USE_GRADIENT:
      tool_preset->use_gradient = g_value_get_boolean (value);
      break;
    case PROP_USE_FONT:
      tool_preset->use_font = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_preset_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanToolPreset *tool_preset = PICMAN_TOOL_PRESET (object);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string (value, picman_object_get_name (tool_preset));
      break;

    case PROP_PICMAN:
      g_value_set_object (value, tool_preset->picman);
      break;

    case PROP_TOOL_OPTIONS:
      g_value_set_object (value, tool_preset->tool_options);
      break;

    case PROP_USE_FG_BG:
      g_value_set_boolean (value, tool_preset->use_fg_bg);
      break;
    case PROP_USE_BRUSH:
      g_value_set_boolean (value, tool_preset->use_brush);
      break;
    case PROP_USE_DYNAMICS:
      g_value_set_boolean (value, tool_preset->use_dynamics);
      break;
    case PROP_USE_PATTERN:
      g_value_set_boolean (value, tool_preset->use_pattern);
      break;
    case PROP_USE_PALETTE:
      g_value_set_boolean (value, tool_preset->use_palette);
      break;
    case PROP_USE_GRADIENT:
      g_value_set_boolean (value, tool_preset->use_gradient);
      break;
    case PROP_USE_FONT:
      g_value_set_boolean (value, tool_preset->use_font);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tool_preset_dispatch_properties_changed (GObject     *object,
                                              guint        n_pspecs,
                                              GParamSpec **pspecs)
{
  gint i;

  G_OBJECT_CLASS (parent_class)->dispatch_properties_changed (object,
                                                              n_pspecs, pspecs);

  for (i = 0; i < n_pspecs; i++)
    {
      if (pspecs[i]->flags & PICMAN_CONFIG_PARAM_SERIALIZE)
        {
          picman_data_dirty (PICMAN_DATA (object));
          break;
        }
    }
}

static const gchar *
picman_tool_preset_get_extension (PicmanData *data)
{
  return PICMAN_TOOL_PRESET_FILE_EXTENSION;
}

static gboolean
picman_tool_preset_deserialize_property (PicmanConfig *config,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec,
                                       GScanner   *scanner,
                                       GTokenType *expected)
{
  PicmanToolPreset *tool_preset = PICMAN_TOOL_PRESET (config);

  switch (property_id)
    {
    case PROP_TOOL_OPTIONS:
      {
        GObject             *options;
        gchar               *type_name;
        GType                type;
        PicmanContextPropMask  serialize_props;

        if (! picman_scanner_parse_string (scanner, &type_name))
          {
            *expected = G_TOKEN_STRING;
            break;
          }

        type = g_type_from_name (type_name);

        if (! type)
          {
            g_scanner_error (scanner,
                             "unable to determine type of '%s'",
                             type_name);
            *expected = G_TOKEN_STRING;
            g_free (type_name);
            break;
          }

        if (! g_type_is_a (type, PICMAN_TYPE_TOOL_OPTIONS))
          {
            g_scanner_error (scanner,
                             "'%s' is not a subclass of PicmanToolOptions",
                             type_name);
            *expected = G_TOKEN_STRING;
            g_free (type_name);
            break;
          }

        g_free (type_name);

        options = g_object_new (type,
                                "picman", tool_preset->picman,
                                NULL);

        if (! PICMAN_CONFIG_GET_INTERFACE (options)->deserialize (PICMAN_CONFIG (options),
                                                                scanner, 1,
                                                                NULL))
          {
            g_object_unref (options);
            break;
          }

        /* we need both tool and tool-info on the options */
        if (picman_context_get_tool (PICMAN_CONTEXT (options)))
          {
            g_object_set (options,
                          "tool-info",
                          picman_context_get_tool (PICMAN_CONTEXT (options)),
                          NULL);
          }
        else if (PICMAN_TOOL_OPTIONS (options)->tool_info)
          {
            g_object_set (options,
                          "tool", PICMAN_TOOL_OPTIONS (options)->tool_info,
                          NULL);
          }
        else
          {
            /* if we have none, the options set_property() logic will
             * replace the NULL with its best guess
             */
            g_object_set (options,
                          "tool",      NULL,
                          "tool-info", NULL,
                          NULL);
          }

        serialize_props =
          picman_context_get_serialize_properties (PICMAN_CONTEXT (options));

        picman_context_set_serialize_properties (PICMAN_CONTEXT (options),
                                               serialize_props |
                                               PICMAN_CONTEXT_TOOL_MASK);

        g_value_take_object (value, options);
      }
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

static void
picman_tool_preset_set_options (PicmanToolPreset  *preset,
                              PicmanToolOptions *options)
{
  if (preset->tool_options)
    {
      g_signal_handlers_disconnect_by_func (preset->tool_options,
                                            picman_tool_preset_options_notify,
                                            preset);

      g_object_unref (preset->tool_options);
      preset->tool_options = NULL;
    }

  if (options)
    {
      PicmanContextPropMask serialize_props;

      preset->tool_options =
        PICMAN_TOOL_OPTIONS (picman_config_duplicate (PICMAN_CONFIG (options)));

      serialize_props =
        picman_context_get_serialize_properties (PICMAN_CONTEXT (preset->tool_options));

      picman_context_set_serialize_properties (PICMAN_CONTEXT (preset->tool_options),
                                             serialize_props |
                                             PICMAN_CONTEXT_TOOL_MASK);

      if (! (serialize_props & PICMAN_CONTEXT_FOREGROUND_MASK))
        g_object_set (preset, "use-fg-bg", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_BRUSH_MASK))
        g_object_set (preset, "use-brush", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_DYNAMICS_MASK))
        g_object_set (preset, "use-dynamics", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_GRADIENT_MASK))
        g_object_set (preset, "use-gradient", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_PATTERN_MASK))
        g_object_set (preset, "use-pattern", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_PALETTE_MASK))
        g_object_set (preset, "use-palette", FALSE, NULL);

      if (! (serialize_props & PICMAN_CONTEXT_FONT_MASK))
        g_object_set (preset, "use-font", FALSE, NULL);

      g_signal_connect (preset->tool_options, "notify",
                        G_CALLBACK (picman_tool_preset_options_notify),
                        preset);
    }

  g_object_notify (G_OBJECT (preset), "tool-options");
}

static void
picman_tool_preset_options_notify (GObject          *tool_options,
                                 const GParamSpec *pspec,
                                 PicmanToolPreset   *preset)
{
  g_object_notify (G_OBJECT (preset), "tool-options");
}


/*  public functions  */

PicmanData *
picman_tool_preset_new (PicmanContext *context,
                      const gchar *unused)
{
  PicmanToolInfo *tool_info;
  const gchar  *stock_id;

  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  tool_info = picman_context_get_tool (context);

  g_return_val_if_fail (tool_info != NULL, NULL);

  stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info));

  return g_object_new (PICMAN_TYPE_TOOL_PRESET,
                       "name",         tool_info->blurb,
                       "stock-id",     stock_id,
                       "picman",         context->picman,
                       "tool-options", tool_info->tool_options,
                       NULL);
}

PicmanContextPropMask
picman_tool_preset_get_prop_mask (PicmanToolPreset *preset)
{
  PicmanContextPropMask serialize_props;
  PicmanContextPropMask use_props = 0;

  g_return_val_if_fail (PICMAN_IS_TOOL_PRESET (preset), 0);

  serialize_props =
    picman_context_get_serialize_properties (PICMAN_CONTEXT (preset->tool_options));

  if (preset->use_fg_bg)
    {
      use_props |= (PICMAN_CONTEXT_FOREGROUND_MASK & serialize_props);
      use_props |= (PICMAN_CONTEXT_BACKGROUND_MASK & serialize_props);
    }

  if (preset->use_brush)
    use_props |= (PICMAN_CONTEXT_BRUSH_MASK & serialize_props);

  if (preset->use_dynamics)
    use_props |= (PICMAN_CONTEXT_DYNAMICS_MASK & serialize_props);

  if (preset->use_pattern)
    use_props |= (PICMAN_CONTEXT_PATTERN_MASK & serialize_props);

  if (preset->use_palette)
    use_props |= (PICMAN_CONTEXT_PALETTE_MASK & serialize_props);

  if (preset->use_gradient)
    use_props |= (PICMAN_CONTEXT_GRADIENT_MASK & serialize_props);

  if (preset->use_font)
    use_props |= (PICMAN_CONTEXT_FONT_MASK & serialize_props);

  return use_props;
}
