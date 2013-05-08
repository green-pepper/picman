/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanstrokeoptions.c
 * Copyright (C) 2003 Simon Budig
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picmanbrush.h"
#include "picmancontext.h"
#include "picmandashpattern.h"
#include "picmanmarshal.h"
#include "picmanpaintinfo.h"
#include "picmanparamspecs.h"
#include "picmanstrokeoptions.h"

#include "paint/picmanpaintoptions.h"

#include "picman-intl.h"


enum
{
  PROP_0,

  PROP_METHOD,

  PROP_STYLE,
  PROP_WIDTH,
  PROP_UNIT,
  PROP_CAP_STYLE,
  PROP_JOIN_STYLE,
  PROP_MITER_LIMIT,
  PROP_ANTIALIAS,
  PROP_DASH_UNIT,
  PROP_DASH_OFFSET,
  PROP_DASH_INFO,

  PROP_PAINT_OPTIONS,
  PROP_EMULATE_DYNAMICS
};

enum
{
  DASH_INFO_CHANGED,
  LAST_SIGNAL
};


typedef struct _PicmanStrokeOptionsPrivate PicmanStrokeOptionsPrivate;

struct _PicmanStrokeOptionsPrivate
{
  PicmanStrokeMethod  method;

  /*  options for medhod == LIBART  */
  gdouble           width;
  PicmanUnit          unit;

  PicmanCapStyle      cap_style;
  PicmanJoinStyle     join_style;

  gdouble           miter_limit;

  gdouble           dash_offset;
  GArray           *dash_info;

  /*  options for method == PAINT_TOOL  */
  PicmanPaintOptions *paint_options;
  gboolean          emulate_dynamics;
};

#define GET_PRIVATE(options) \
        G_TYPE_INSTANCE_GET_PRIVATE (options, \
                                     PICMAN_TYPE_STROKE_OPTIONS, \
                                     PicmanStrokeOptionsPrivate)


static void   picman_stroke_options_config_iface_init (gpointer      iface,
                                                     gpointer      iface_data);

static void   picman_stroke_options_finalize          (GObject      *object);
static void   picman_stroke_options_set_property      (GObject      *object,
                                                     guint         property_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec);
static void   picman_stroke_options_get_property      (GObject      *object,
                                                     guint         property_id,
                                                     GValue       *value,
                                                     GParamSpec   *pspec);

static PicmanConfig * picman_stroke_options_duplicate   (PicmanConfig   *config);


G_DEFINE_TYPE_WITH_CODE (PicmanStrokeOptions, picman_stroke_options,
                         PICMAN_TYPE_FILL_OPTIONS,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_stroke_options_config_iface_init))

#define parent_class picman_stroke_options_parent_class

static PicmanConfigInterface *parent_config_iface = NULL;

static guint stroke_options_signals[LAST_SIGNAL] = { 0 };


static void
picman_stroke_options_class_init (PicmanStrokeOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec   *array_spec;

  object_class->finalize     = picman_stroke_options_finalize;
  object_class->set_property = picman_stroke_options_set_property;
  object_class->get_property = picman_stroke_options_get_property;

  klass->dash_info_changed = NULL;

  stroke_options_signals[DASH_INFO_CHANGED] =
    g_signal_new ("dash-info-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanStrokeOptionsClass, dash_info_changed),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DASH_PRESET);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_METHOD,
                                 "method", NULL,
                                 PICMAN_TYPE_STROKE_METHOD,
                                 PICMAN_STROKE_METHOD_LIBART,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_WIDTH,
                                   "width", NULL,
                                   0.0, 2000.0, 6.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_UNIT,
                                 "unit", NULL,
                                 TRUE, FALSE, PICMAN_UNIT_PIXEL,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CAP_STYLE,
                                 "cap-style", NULL,
                                 PICMAN_TYPE_CAP_STYLE, PICMAN_CAP_BUTT,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_JOIN_STYLE,
                                 "join-style", NULL,
                                 PICMAN_TYPE_JOIN_STYLE, PICMAN_JOIN_MITER,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_MITER_LIMIT,
                                   "miter-limit",
                                   _("Convert a mitered join to a bevelled "
                                     "join if the miter would extend to a "
                                     "distance of more than miter-limit * "
                                     "line-width from the actual join point."),
                                   0.0, 100.0, 10.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_DASH_OFFSET,
                                   "dash-offset", NULL,
                                   0.0, 2000.0, 0.0,
                                   PICMAN_PARAM_STATIC_STRINGS);

  array_spec = g_param_spec_double ("dash-length", NULL, NULL,
                                    0.0, 2000.0, 1.0, PICMAN_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_DASH_INFO,
                                   picman_param_spec_value_array ("dash-info",
                                                                NULL, NULL,
                                                                array_spec,
                                                                PICMAN_PARAM_STATIC_STRINGS |
                                                                PICMAN_CONFIG_PARAM_FLAGS));

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_PAINT_OPTIONS,
                                   "paint-options", NULL,
                                   PICMAN_TYPE_PAINT_OPTIONS,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_EMULATE_DYNAMICS,
                                    "emulate-brush-dynamics", NULL,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  g_type_class_add_private (klass, sizeof (PicmanStrokeOptionsPrivate));
}

static void
picman_stroke_options_config_iface_init (gpointer  iface,
                                       gpointer  iface_data)
{
  PicmanConfigInterface *config_iface = (PicmanConfigInterface *) iface;

  parent_config_iface = g_type_interface_peek_parent (config_iface);

  if (! parent_config_iface)
    parent_config_iface = g_type_default_interface_peek (PICMAN_TYPE_CONFIG);

  config_iface->duplicate = picman_stroke_options_duplicate;
}

static void
picman_stroke_options_init (PicmanStrokeOptions *options)
{
}

static void
picman_stroke_options_finalize (GObject *object)
{
  PicmanStrokeOptionsPrivate *private = GET_PRIVATE (object);

  if (private->dash_info)
    {
      picman_dash_pattern_free (private->dash_info);
      private->dash_info = NULL;
    }

  if (private->paint_options)
    {
      g_object_unref (private->paint_options);
      private->paint_options = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_stroke_options_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanStrokeOptions        *options = PICMAN_STROKE_OPTIONS (object);
  PicmanStrokeOptionsPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_METHOD:
      private->method = g_value_get_enum (value);
      break;

    case PROP_WIDTH:
      private->width = g_value_get_double (value);
      break;
    case PROP_UNIT:
      private->unit = g_value_get_int (value);
      break;
    case PROP_CAP_STYLE:
      private->cap_style = g_value_get_enum (value);
      break;
    case PROP_JOIN_STYLE:
      private->join_style = g_value_get_enum (value);
      break;
    case PROP_MITER_LIMIT:
      private->miter_limit = g_value_get_double (value);
      break;
    case PROP_DASH_OFFSET:
      private->dash_offset = g_value_get_double (value);
      break;
    case PROP_DASH_INFO:
      {
        PicmanValueArray *value_array = g_value_get_boxed (value);
        GArray         *pattern;

        pattern = picman_dash_pattern_from_value_array (value_array);
        picman_stroke_options_take_dash_pattern (options, PICMAN_DASH_CUSTOM,
                                               pattern);
      }
      break;

    case PROP_PAINT_OPTIONS:
      if (private->paint_options)
        g_object_unref (private->paint_options);
      private->paint_options = g_value_dup_object (value);
      break;
    case PROP_EMULATE_DYNAMICS:
      private->emulate_dynamics = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_stroke_options_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanStrokeOptionsPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_METHOD:
      g_value_set_enum (value, private->method);
      break;

    case PROP_WIDTH:
      g_value_set_double (value, private->width);
      break;
    case PROP_UNIT:
      g_value_set_int (value, private->unit);
      break;
    case PROP_CAP_STYLE:
      g_value_set_enum (value, private->cap_style);
      break;
    case PROP_JOIN_STYLE:
      g_value_set_enum (value, private->join_style);
      break;
    case PROP_MITER_LIMIT:
      g_value_set_double (value, private->miter_limit);
      break;
    case PROP_DASH_OFFSET:
      g_value_set_double (value, private->dash_offset);
      break;
    case PROP_DASH_INFO:
      {
        PicmanValueArray *value_array;

        value_array = picman_dash_pattern_to_value_array (private->dash_info);
        g_value_take_boxed (value, value_array);
      }
      break;

    case PROP_PAINT_OPTIONS:
      g_value_set_object (value, private->paint_options);
      break;
    case PROP_EMULATE_DYNAMICS:
      g_value_set_boolean (value, private->emulate_dynamics);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static PicmanConfig *
picman_stroke_options_duplicate (PicmanConfig *config)
{
  PicmanStrokeOptions        *options = PICMAN_STROKE_OPTIONS (config);
  PicmanStrokeOptionsPrivate *private = GET_PRIVATE (options);
  PicmanStrokeOptions        *new_options;

  new_options = PICMAN_STROKE_OPTIONS (parent_config_iface->duplicate (config));

  if (private->paint_options)
    {
      GObject *paint_options;

      paint_options = picman_config_duplicate (PICMAN_CONFIG (private->paint_options));
      g_object_set (new_options, "paint-options", paint_options, NULL);
      g_object_unref (paint_options);
    }

  return PICMAN_CONFIG (new_options);
}


/*  public functions  */

PicmanStrokeOptions *
picman_stroke_options_new (Picman        *picman,
                         PicmanContext *context,
                         gboolean     use_context_color)
{
  PicmanPaintInfo     *paint_info = NULL;
  PicmanStrokeOptions *options;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (use_context_color == FALSE || context != NULL, NULL);

  if (context)
    paint_info = picman_context_get_paint_info (context);

  if (! paint_info)
    paint_info = picman_paint_info_get_standard (picman);

  options = g_object_new (PICMAN_TYPE_STROKE_OPTIONS,
                          "picman",       picman,
                          "paint-info", paint_info,
                          NULL);

  if (use_context_color)
    {
      picman_context_define_properties (PICMAN_CONTEXT (options),
                                      PICMAN_CONTEXT_FOREGROUND_MASK |
                                      PICMAN_CONTEXT_PATTERN_MASK,
                                      FALSE);

      picman_context_set_parent (PICMAN_CONTEXT (options), context);
    }

  return options;
}

PicmanStrokeMethod
picman_stroke_options_get_method (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options),
                        PICMAN_STROKE_METHOD_LIBART);

  return GET_PRIVATE (options)->method;
}

gdouble
picman_stroke_options_get_width (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), 1.0);

  return GET_PRIVATE (options)->width;
}

PicmanUnit
picman_stroke_options_get_unit (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), PICMAN_UNIT_PIXEL);

  return GET_PRIVATE (options)->unit;
}

PicmanCapStyle
picman_stroke_options_get_cap_style (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), PICMAN_CAP_BUTT);

  return GET_PRIVATE (options)->cap_style;
}

PicmanJoinStyle
picman_stroke_options_get_join_style (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), PICMAN_JOIN_MITER);

  return GET_PRIVATE (options)->join_style;
}

gdouble
picman_stroke_options_get_miter_limit (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), 1.0);

  return GET_PRIVATE (options)->miter_limit;
}

gdouble
picman_stroke_options_get_dash_offset (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), 0.0);

  return GET_PRIVATE (options)->dash_offset;
}

GArray *
picman_stroke_options_get_dash_info (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), NULL);

  return GET_PRIVATE (options)->dash_info;
}

PicmanPaintOptions *
picman_stroke_options_get_paint_options (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), NULL);

  return GET_PRIVATE (options)->paint_options;
}

gboolean
picman_stroke_options_get_emulate_dynamics (PicmanStrokeOptions *options)
{
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), FALSE);

  return GET_PRIVATE (options)->emulate_dynamics;
}

/**
 * picman_stroke_options_take_dash_pattern:
 * @options: a #PicmanStrokeOptions object
 * @preset: a value out of the #PicmanDashPreset enum
 * @pattern: a #GArray or %NULL if @preset is not %PICMAN_DASH_CUSTOM
 *
 * Sets the dash pattern. Either a @preset is passed and @pattern is
 * %NULL or @preset is %PICMAN_DASH_CUSTOM and @pattern is the #GArray
 * to use as the dash pattern. Note that this function takes ownership
 * of the passed pattern.
 */
void
picman_stroke_options_take_dash_pattern (PicmanStrokeOptions *options,
                                       PicmanDashPreset     preset,
                                       GArray            *pattern)
{
  PicmanStrokeOptionsPrivate *private;

  g_return_if_fail (PICMAN_IS_STROKE_OPTIONS (options));
  g_return_if_fail (preset == PICMAN_DASH_CUSTOM || pattern == NULL);

  private = GET_PRIVATE (options);

  if (preset != PICMAN_DASH_CUSTOM)
    pattern = picman_dash_pattern_new_from_preset (preset);

  if (private->dash_info)
    picman_dash_pattern_free (private->dash_info);

  private->dash_info = pattern;

  g_object_notify (G_OBJECT (options), "dash-info");

  g_signal_emit (options, stroke_options_signals [DASH_INFO_CHANGED], 0,
                 preset);
}

void
picman_stroke_options_prepare (PicmanStrokeOptions *options,
                             PicmanContext       *context,
                             gboolean           use_default_values)
{
  PicmanStrokeOptionsPrivate *private;

  g_return_if_fail (PICMAN_IS_STROKE_OPTIONS (options));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  private = GET_PRIVATE (options);

  switch (private->method)
    {
    case PICMAN_STROKE_METHOD_LIBART:
      break;

    case PICMAN_STROKE_METHOD_PAINT_CORE:
      {
        PicmanPaintInfo    *paint_info = PICMAN_CONTEXT (options)->paint_info;
        PicmanPaintOptions *paint_options;

        if (use_default_values)
          {
            PicmanBrush *brush;
            gdouble    brush_size;
            gint       height;
            gint       width;

            paint_options = picman_paint_options_new (paint_info);

            brush = picman_context_get_brush (context);

            if (PICMAN_IS_BRUSH (brush))
              {
                picman_brush_transform_size (brush, 1.0, 1.0, 0.0, &height, &width);
                brush_size = MAX (height, width);

                g_object_set (paint_options,
                              "brush-size", brush_size,
                              NULL);
              }

            /*  undefine the paint-relevant context properties and get them
             *  from the passed context
             */
            picman_context_define_properties (PICMAN_CONTEXT (paint_options),
                                            PICMAN_CONTEXT_PAINT_PROPS_MASK,
                                            FALSE);
            picman_context_set_parent (PICMAN_CONTEXT (paint_options), context);
          }
        else
          {
            PicmanCoreConfig      *config       = context->picman->config;
            PicmanContextPropMask  global_props = 0;

            paint_options =
              picman_config_duplicate (PICMAN_CONFIG (paint_info->paint_options));

            /*  FG and BG are always shared between all tools  */
            global_props |= PICMAN_CONTEXT_FOREGROUND_MASK;
            global_props |= PICMAN_CONTEXT_BACKGROUND_MASK;

            if (config->global_brush)
              global_props |= PICMAN_CONTEXT_BRUSH_MASK;
            if (config->global_dynamics)
              global_props |= PICMAN_CONTEXT_DYNAMICS_MASK;
            if (config->global_pattern)
              global_props |= PICMAN_CONTEXT_PATTERN_MASK;
            if (config->global_palette)
              global_props |= PICMAN_CONTEXT_PALETTE_MASK;
            if (config->global_gradient)
              global_props |= PICMAN_CONTEXT_GRADIENT_MASK;
            if (config->global_font)
              global_props |= PICMAN_CONTEXT_FONT_MASK;

            picman_context_copy_properties (context,
                                          PICMAN_CONTEXT (paint_options),
                                          global_props);
          }

        g_object_set (options, "paint-options", paint_options, NULL);
        g_object_unref (paint_options);
      }
      break;

    default:
      g_return_if_reached ();
    }
}

void
picman_stroke_options_finish (PicmanStrokeOptions *options)
{
  g_return_if_fail (PICMAN_IS_STROKE_OPTIONS (options));

  g_object_set (options, "paint-options", NULL, NULL);
}
