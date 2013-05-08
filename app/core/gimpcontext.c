/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontext.c
 * Copyright (C) 1999-2010 Michael Natterer
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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmanbrush.h"
#include "picmanbuffer.h"
#include "picmancontainer.h"
#include "picmancontext.h"
#include "picmandatafactory.h"
#include "picmandynamics.h"
#include "picmanimagefile.h"
#include "picmangradient.h"
#include "picmanimage.h"
#include "picmanmarshal.h"
#include "picmanpaintinfo.h"
#include "picmanpalette.h"
#include "picmanpattern.h"
#include "picmantemplate.h"
#include "picmantoolinfo.h"
#include "picmantoolpreset.h"

#include "text/picmanfont.h"

#include "picman-intl.h"


typedef void (* PicmanContextCopyPropFunc) (PicmanContext *src,
                                          PicmanContext *dest);


#define context_find_defined(context,prop) \
  while (!(((context)->defined_props) & (1 << (prop))) && (context)->parent) \
    (context) = (context)->parent


/*  local function prototypes  */

static void    picman_context_config_iface_init (PicmanConfigInterface   *iface);

static void       picman_context_constructed    (GObject               *object);
static void       picman_context_dispose        (GObject               *object);
static void       picman_context_finalize       (GObject               *object);
static void       picman_context_set_property   (GObject               *object,
                                               guint                  property_id,
                                               const GValue          *value,
                                               GParamSpec            *pspec);
static void       picman_context_get_property   (GObject               *object,
                                               guint                  property_id,
                                               GValue                *value,
                                               GParamSpec            *pspec);
static gint64     picman_context_get_memsize    (PicmanObject            *object,
                                               gint64                *gui_size);

static gboolean   picman_context_serialize            (PicmanConfig       *config,
                                                     PicmanConfigWriter *writer,
                                                     gpointer          data);
static gboolean   picman_context_serialize_property   (PicmanConfig       *config,
                                                     guint             property_id,
                                                     const GValue     *value,
                                                     GParamSpec       *pspec,
                                                     PicmanConfigWriter *writer);
static gboolean   picman_context_deserialize_property (PicmanConfig       *config,
                                                     guint             property_id,
                                                     GValue           *value,
                                                     GParamSpec       *pspec,
                                                     GScanner         *scanner,
                                                     GTokenType       *expected);

/*  image  */
static void picman_context_image_removed       (PicmanContainer    *container,
                                              PicmanImage        *image,
                                              PicmanContext      *context);
static void picman_context_real_set_image      (PicmanContext      *context,
                                              PicmanImage        *image);

/*  display  */
static void picman_context_display_removed     (PicmanContainer    *container,
                                              gpointer          display,
                                              PicmanContext      *context);
static void picman_context_real_set_display    (PicmanContext      *context,
                                              gpointer          display);

/*  tool  */
static void picman_context_tool_dirty          (PicmanToolInfo     *tool_info,
                                              PicmanContext      *context);
static void picman_context_tool_removed        (PicmanContainer    *container,
                                              PicmanToolInfo     *tool_info,
                                              PicmanContext      *context);
static void picman_context_tool_list_thaw      (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_tool       (PicmanContext      *context,
                                              PicmanToolInfo     *tool_info);

/*  paint info  */
static void picman_context_paint_info_dirty    (PicmanPaintInfo    *paint_info,
                                              PicmanContext      *context);
static void picman_context_paint_info_removed  (PicmanContainer    *container,
                                              PicmanPaintInfo    *paint_info,
                                              PicmanContext      *context);
static void picman_context_paint_info_list_thaw(PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_paint_info (PicmanContext      *context,
                                              PicmanPaintInfo    *paint_info);

/*  foreground  */
static void picman_context_real_set_foreground (PicmanContext      *context,
                                              const PicmanRGB    *color);

/*  background  */
static void picman_context_real_set_background (PicmanContext      *context,
                                              const PicmanRGB    *color);

/*  opacity  */
static void picman_context_real_set_opacity    (PicmanContext      *context,
                                              gdouble           opacity);

/*  paint mode  */
static void picman_context_real_set_paint_mode (PicmanContext      *context,
                                              PicmanLayerModeEffects paint_mode);

/*  brush  */
static void picman_context_brush_dirty         (PicmanBrush        *brush,
                                              PicmanContext      *context);
static void picman_context_brush_removed       (PicmanContainer    *brush_list,
                                              PicmanBrush        *brush,
                                              PicmanContext      *context);
static void picman_context_brush_list_thaw     (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_brush      (PicmanContext      *context,
                                              PicmanBrush        *brush);

/*  dynamics  */

static void picman_context_dynamics_dirty      (PicmanDynamics     *dynamics,
                                              PicmanContext      *context);
static void picman_context_dynamics_removed    (PicmanContainer    *container,
                                              PicmanDynamics     *dynamics,
                                              PicmanContext      *context);
static void picman_context_dynamics_list_thaw  (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_dynamics   (PicmanContext      *context,
                                              PicmanDynamics     *dynamics);

/*  pattern  */
static void picman_context_pattern_dirty       (PicmanPattern      *pattern,
                                              PicmanContext      *context);
static void picman_context_pattern_removed     (PicmanContainer    *container,
                                              PicmanPattern      *pattern,
                                              PicmanContext      *context);
static void picman_context_pattern_list_thaw   (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_pattern    (PicmanContext      *context,
                                              PicmanPattern      *pattern);

/*  gradient  */
static void picman_context_gradient_dirty      (PicmanGradient     *gradient,
                                              PicmanContext      *context);
static void picman_context_gradient_removed    (PicmanContainer    *container,
                                              PicmanGradient     *gradient,
                                              PicmanContext      *context);
static void picman_context_gradient_list_thaw  (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_gradient   (PicmanContext      *context,
                                              PicmanGradient     *gradient);

/*  palette  */
static void picman_context_palette_dirty       (PicmanPalette      *palette,
                                              PicmanContext      *context);
static void picman_context_palette_removed     (PicmanContainer    *container,
                                              PicmanPalette      *palatte,
                                              PicmanContext      *context);
static void picman_context_palette_list_thaw   (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_palette    (PicmanContext      *context,
                                              PicmanPalette      *palatte);

/*  tool preset  */
static void picman_context_tool_preset_dirty     (PicmanToolPreset   *tool_preset,
                                                PicmanContext      *context);
static void picman_context_tool_preset_removed   (PicmanContainer    *container,
                                                PicmanToolPreset   *tool_preset,
                                                PicmanContext      *context);
static void picman_context_tool_preset_list_thaw (PicmanContainer    *container,
                                                PicmanContext      *context);
static void picman_context_real_set_tool_preset  (PicmanContext      *context,
                                                PicmanToolPreset   *tool_preset);

/*  font  */
static void picman_context_font_dirty          (PicmanFont         *font,
                                              PicmanContext      *context);
static void picman_context_font_removed        (PicmanContainer    *container,
                                              PicmanFont         *font,
                                              PicmanContext      *context);
static void picman_context_font_list_thaw      (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_font       (PicmanContext      *context,
                                              PicmanFont         *font);

/*  buffer  */
static void picman_context_buffer_dirty        (PicmanBuffer       *buffer,
                                              PicmanContext      *context);
static void picman_context_buffer_removed      (PicmanContainer    *container,
                                              PicmanBuffer       *buffer,
                                              PicmanContext      *context);
static void picman_context_buffer_list_thaw    (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_buffer     (PicmanContext      *context,
                                              PicmanBuffer       *buffer);

/*  imagefile  */
static void picman_context_imagefile_dirty     (PicmanImagefile    *imagefile,
                                              PicmanContext      *context);
static void picman_context_imagefile_removed   (PicmanContainer    *container,
                                              PicmanImagefile    *imagefile,
                                              PicmanContext      *context);
static void picman_context_imagefile_list_thaw (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_imagefile  (PicmanContext      *context,
                                              PicmanImagefile    *imagefile);

/*  template  */
static void picman_context_template_dirty      (PicmanTemplate     *template,
                                              PicmanContext      *context);
static void picman_context_template_removed    (PicmanContainer    *container,
                                              PicmanTemplate     *template,
                                              PicmanContext      *context);
static void picman_context_template_list_thaw  (PicmanContainer    *container,
                                              PicmanContext      *context);
static void picman_context_real_set_template   (PicmanContext      *context,
                                              PicmanTemplate     *template);


/*  utilities  */
static gpointer picman_context_find_object     (PicmanContext      *context,
                                              PicmanContainer    *container,
                                              const gchar      *object_name,
                                              gpointer          standard_object);


/*  properties & signals  */

enum
{
  PICMAN_CONTEXT_PROP_0,
  PICMAN_CONTEXT_PROP_PICMAN

  /*  remaining values are in core-enums.h  (PicmanContextPropType)  */
};

enum
{
  DUMMY_0,
  DUMMY_1,
  IMAGE_CHANGED,
  DISPLAY_CHANGED,
  TOOL_CHANGED,
  PAINT_INFO_CHANGED,
  FOREGROUND_CHANGED,
  BACKGROUND_CHANGED,
  OPACITY_CHANGED,
  PAINT_MODE_CHANGED,
  BRUSH_CHANGED,
  DYNAMICS_CHANGED,
  PATTERN_CHANGED,
  GRADIENT_CHANGED,
  PALETTE_CHANGED,
  TOOL_PRESET_CHANGED,
  FONT_CHANGED,
  BUFFER_CHANGED,
  IMAGEFILE_CHANGED,
  TEMPLATE_CHANGED,
  LAST_SIGNAL
};

static const gchar * const picman_context_prop_names[] =
{
  NULL, /* PROP_0 */
  "picman",
  "image",
  "display",
  "tool",
  "paint-info",
  "foreground",
  "background",
  "opacity",
  "paint-mode",
  "brush",
  "dynamics",
  "pattern",
  "gradient",
  "palette",
  "tool-preset",
  "font",
  "buffer",
  "imagefile",
  "template"
};

static GType picman_context_prop_types[] =
{
  G_TYPE_NONE, /* PROP_0    */
  G_TYPE_NONE, /* PROP_PICMAN */
  0,
  G_TYPE_NONE,
  0,
  0,
  G_TYPE_NONE,
  G_TYPE_NONE,
  G_TYPE_NONE,
  G_TYPE_NONE,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};


G_DEFINE_TYPE_WITH_CODE (PicmanContext, picman_context, PICMAN_TYPE_VIEWABLE,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_context_config_iface_init))

#define parent_class picman_context_parent_class

static guint picman_context_signals[LAST_SIGNAL] = { 0 };


static void
picman_context_class_init (PicmanContextClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanRGB          black;
  PicmanRGB          white;

  picman_rgba_set (&black, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&white, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);

  picman_context_signals[IMAGE_CHANGED] =
    g_signal_new ("image-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, image_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_IMAGE);

  picman_context_signals[DISPLAY_CHANGED] =
    g_signal_new ("display-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, display_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_OBJECT);

  picman_context_signals[TOOL_CHANGED] =
    g_signal_new ("tool-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, tool_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_TOOL_INFO);

  picman_context_signals[PAINT_INFO_CHANGED] =
    g_signal_new ("paint-info-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, paint_info_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PAINT_INFO);

  picman_context_signals[FOREGROUND_CHANGED] =
    g_signal_new ("foreground-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, foreground_changed),
                  NULL, NULL,
                  picman_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_RGB | G_SIGNAL_TYPE_STATIC_SCOPE);

  picman_context_signals[BACKGROUND_CHANGED] =
    g_signal_new ("background-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, background_changed),
                  NULL, NULL,
                  picman_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_RGB | G_SIGNAL_TYPE_STATIC_SCOPE);

  picman_context_signals[OPACITY_CHANGED] =
    g_signal_new ("opacity-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, opacity_changed),
                  NULL, NULL,
                  picman_marshal_VOID__DOUBLE,
                  G_TYPE_NONE, 1,
                  G_TYPE_DOUBLE);

  picman_context_signals[PAINT_MODE_CHANGED] =
    g_signal_new ("paint-mode-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, paint_mode_changed),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_LAYER_MODE_EFFECTS);

  picman_context_signals[BRUSH_CHANGED] =
    g_signal_new ("brush-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, brush_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_BRUSH);

  picman_context_signals[DYNAMICS_CHANGED] =
    g_signal_new ("dynamics-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, dynamics_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DYNAMICS);

  picman_context_signals[PATTERN_CHANGED] =
    g_signal_new ("pattern-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, pattern_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PATTERN);

  picman_context_signals[GRADIENT_CHANGED] =
    g_signal_new ("gradient-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, gradient_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_GRADIENT);

  picman_context_signals[PALETTE_CHANGED] =
    g_signal_new ("palette-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, palette_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PALETTE);

  picman_context_signals[TOOL_PRESET_CHANGED] =
    g_signal_new ("tool-preset-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, tool_preset_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_TOOL_PRESET);

  picman_context_signals[FONT_CHANGED] =
    g_signal_new ("font-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, font_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_FONT);

  picman_context_signals[BUFFER_CHANGED] =
    g_signal_new ("buffer-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, buffer_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_BUFFER);

  picman_context_signals[IMAGEFILE_CHANGED] =
    g_signal_new ("imagefile-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, imagefile_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_IMAGEFILE);

  picman_context_signals[TEMPLATE_CHANGED] =
    g_signal_new ("template-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContextClass, template_changed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_TEMPLATE);

  object_class->constructed      = picman_context_constructed;
  object_class->set_property     = picman_context_set_property;
  object_class->get_property     = picman_context_get_property;
  object_class->dispose          = picman_context_dispose;
  object_class->finalize         = picman_context_finalize;

  picman_object_class->get_memsize = picman_context_get_memsize;

  klass->image_changed           = NULL;
  klass->display_changed         = NULL;
  klass->tool_changed            = NULL;
  klass->paint_info_changed      = NULL;
  klass->foreground_changed      = NULL;
  klass->background_changed      = NULL;
  klass->opacity_changed         = NULL;
  klass->paint_mode_changed      = NULL;
  klass->brush_changed           = NULL;
  klass->dynamics_changed        = NULL;
  klass->pattern_changed         = NULL;
  klass->gradient_changed        = NULL;
  klass->palette_changed         = NULL;
  klass->tool_preset_changed     = NULL;
  klass->font_changed            = NULL;
  klass->buffer_changed          = NULL;
  klass->imagefile_changed       = NULL;
  klass->template_changed        = NULL;

  picman_context_prop_types[PICMAN_CONTEXT_PROP_IMAGE]       = PICMAN_TYPE_IMAGE;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_TOOL]        = PICMAN_TYPE_TOOL_INFO;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_PAINT_INFO]  = PICMAN_TYPE_PAINT_INFO;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_BRUSH]       = PICMAN_TYPE_BRUSH;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_DYNAMICS]    = PICMAN_TYPE_DYNAMICS;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_PATTERN]     = PICMAN_TYPE_PATTERN;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_GRADIENT]    = PICMAN_TYPE_GRADIENT;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_PALETTE]     = PICMAN_TYPE_PALETTE;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_TOOL_PRESET] = PICMAN_TYPE_TOOL_PRESET;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_FONT]        = PICMAN_TYPE_FONT;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_BUFFER]      = PICMAN_TYPE_BUFFER;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_IMAGEFILE]   = PICMAN_TYPE_IMAGEFILE;
  picman_context_prop_types[PICMAN_CONTEXT_PROP_TEMPLATE]    = PICMAN_TYPE_TEMPLATE;

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_IMAGE,
                                   g_param_spec_object (picman_context_prop_names[PICMAN_CONTEXT_PROP_IMAGE],
                                                        NULL, NULL,
                                                        PICMAN_TYPE_IMAGE,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_DISPLAY,
                                   g_param_spec_object (picman_context_prop_names[PICMAN_CONTEXT_PROP_DISPLAY],
                                                        NULL, NULL,
                                                        PICMAN_TYPE_OBJECT,
                                                        PICMAN_PARAM_READWRITE));

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_TOOL,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_TOOL], NULL,
                                   PICMAN_TYPE_TOOL_INFO,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_PAINT_INFO,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_PAINT_INFO], NULL,
                                   PICMAN_TYPE_PAINT_INFO,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PICMAN_CONTEXT_PROP_FOREGROUND,
                                picman_context_prop_names[PICMAN_CONTEXT_PROP_FOREGROUND],
                                NULL,
                                FALSE, &black,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PICMAN_CONTEXT_PROP_BACKGROUND,
                                picman_context_prop_names[PICMAN_CONTEXT_PROP_BACKGROUND],
                                NULL,
                                FALSE, &white,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PICMAN_CONTEXT_PROP_OPACITY,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_OPACITY],
                                   _("Opacity"),
                                   PICMAN_OPACITY_TRANSPARENT,
                                   PICMAN_OPACITY_OPAQUE,
                                   PICMAN_OPACITY_OPAQUE,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PICMAN_CONTEXT_PROP_PAINT_MODE,
                                 picman_context_prop_names[PICMAN_CONTEXT_PROP_PAINT_MODE],
                                 _("Paint Mode"),
                                 PICMAN_TYPE_LAYER_MODE_EFFECTS,
                                 PICMAN_NORMAL_MODE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_BRUSH,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_BRUSH],
                                   NULL,
                                   PICMAN_TYPE_BRUSH,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_DYNAMICS,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_DYNAMICS],
                                   NULL,
                                   PICMAN_TYPE_DYNAMICS,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_PATTERN,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_PATTERN],
                                   NULL,
                                   PICMAN_TYPE_PATTERN,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_GRADIENT,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_GRADIENT],
                                   NULL,
                                   PICMAN_TYPE_GRADIENT,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_PALETTE,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_PALETTE],
                                   NULL,
                                   PICMAN_TYPE_PALETTE,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_TOOL_PRESET,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_TOOL_PRESET],
                                   NULL,
                                   PICMAN_TYPE_TOOL_PRESET,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PICMAN_CONTEXT_PROP_FONT,
                                   picman_context_prop_names[PICMAN_CONTEXT_PROP_FONT],
                                   NULL,
                                   PICMAN_TYPE_FONT,
                                   PICMAN_PARAM_STATIC_STRINGS);

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_BUFFER,
                                   g_param_spec_object (picman_context_prop_names[PICMAN_CONTEXT_PROP_BUFFER],
                                                        NULL, NULL,
                                                        PICMAN_TYPE_BUFFER,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_IMAGEFILE,
                                   g_param_spec_object (picman_context_prop_names[PICMAN_CONTEXT_PROP_IMAGEFILE],
                                                        NULL, NULL,
                                                        PICMAN_TYPE_IMAGEFILE,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PICMAN_CONTEXT_PROP_TEMPLATE,
                                   g_param_spec_object (picman_context_prop_names[PICMAN_CONTEXT_PROP_TEMPLATE],
                                                        NULL, NULL,
                                                        PICMAN_TYPE_TEMPLATE,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_context_init (PicmanContext *context)
{
  context->picman            = NULL;

  context->parent          = NULL;

  context->defined_props   = PICMAN_CONTEXT_ALL_PROPS_MASK;
  context->serialize_props = PICMAN_CONTEXT_ALL_PROPS_MASK;

  context->image           = NULL;
  context->display         = NULL;

  context->tool_info       = NULL;
  context->tool_name       = NULL;

  context->paint_info      = NULL;
  context->paint_name      = NULL;

  context->brush           = NULL;
  context->brush_name      = NULL;

  context->dynamics        = NULL;
  context->dynamics_name   = NULL;

  context->pattern         = NULL;
  context->pattern_name    = NULL;

  context->gradient        = NULL;
  context->gradient_name   = NULL;

  context->palette         = NULL;
  context->palette_name    = NULL;

  context->tool_preset      = NULL;
  context->tool_preset_name = NULL;

  context->font            = NULL;
  context->font_name       = NULL;

  context->buffer          = NULL;
  context->buffer_name     = NULL;

  context->imagefile       = NULL;
  context->imagefile_name  = NULL;

  context->template        = NULL;
  context->template_name   = NULL;
}

static void
picman_context_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize            = picman_context_serialize;
  iface->serialize_property   = picman_context_serialize_property;
  iface->deserialize_property = picman_context_deserialize_property;
}

static void
picman_context_constructed (GObject *object)
{
  Picman          *picman;
  PicmanContainer *container;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman = PICMAN_CONTEXT (object)->picman;

  g_assert (PICMAN_IS_PICMAN (picman));

  picman->context_list = g_list_prepend (picman->context_list, object);

  g_signal_connect_object (picman->images, "remove",
                           G_CALLBACK (picman_context_image_removed),
                           object, 0);
  g_signal_connect_object (picman->displays, "remove",
                           G_CALLBACK (picman_context_display_removed),
                           object, 0);

  g_signal_connect_object (picman->tool_info_list, "remove",
                           G_CALLBACK (picman_context_tool_removed),
                           object, 0);
  g_signal_connect_object (picman->tool_info_list, "thaw",
                           G_CALLBACK (picman_context_tool_list_thaw),
                           object, 0);

  g_signal_connect_object (picman->paint_info_list, "remove",
                           G_CALLBACK (picman_context_paint_info_removed),
                           object, 0);
  g_signal_connect_object (picman->paint_info_list, "thaw",
                           G_CALLBACK (picman_context_paint_info_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->brush_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_brush_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_brush_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->dynamics_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_dynamics_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_dynamics_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->pattern_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_pattern_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_pattern_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->gradient_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_gradient_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_gradient_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->palette_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_palette_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_palette_list_thaw),
                           object, 0);

  container = picman_data_factory_get_container (picman->tool_preset_factory);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_context_tool_preset_removed),
                           object, 0);
  g_signal_connect_object (container, "thaw",
                           G_CALLBACK (picman_context_tool_preset_list_thaw),
                           object, 0);

  g_signal_connect_object (picman->fonts, "remove",
                           G_CALLBACK (picman_context_font_removed),
                           object, 0);
  g_signal_connect_object (picman->fonts, "thaw",
                           G_CALLBACK (picman_context_font_list_thaw),
                           object, 0);

  g_signal_connect_object (picman->named_buffers, "remove",
                           G_CALLBACK (picman_context_buffer_removed),
                           object, 0);
  g_signal_connect_object (picman->named_buffers, "thaw",
                           G_CALLBACK (picman_context_buffer_list_thaw),
                           object, 0);

  g_signal_connect_object (picman->documents, "remove",
                           G_CALLBACK (picman_context_imagefile_removed),
                           object, 0);
  g_signal_connect_object (picman->documents, "thaw",
                           G_CALLBACK (picman_context_imagefile_list_thaw),
                           object, 0);

  g_signal_connect_object (picman->templates, "remove",
                           G_CALLBACK (picman_context_template_removed),
                           object, 0);
  g_signal_connect_object (picman->templates, "thaw",
                           G_CALLBACK (picman_context_template_list_thaw),
                           object, 0);
}

static void
picman_context_dispose (GObject *object)
{
  PicmanContext *context = PICMAN_CONTEXT (object);

  if (context->picman)
    {
      context->picman->context_list = g_list_remove (context->picman->context_list,
                                                   context);
      context->picman = NULL;
    }

  if (context->tool_info)
    {
      g_object_unref (context->tool_info);
      context->tool_info = NULL;
    }

  if (context->paint_info)
    {
      g_object_unref (context->paint_info);
      context->paint_info = NULL;
    }

  if (context->brush)
    {
      g_object_unref (context->brush);
      context->brush = NULL;
    }

  if (context->dynamics)
    {
      g_object_unref (context->dynamics);
      context->dynamics = NULL;
    }

  if (context->pattern)
    {
      g_object_unref (context->pattern);
      context->pattern = NULL;
    }

  if (context->gradient)
    {
      g_object_unref (context->gradient);
      context->gradient = NULL;
    }

  if (context->palette)
    {
      g_object_unref (context->palette);
      context->palette = NULL;
    }

  if (context->tool_preset)
    {
      g_object_unref (context->tool_preset);
      context->tool_preset = NULL;
    }

  if (context->font)
    {
      g_object_unref (context->font);
      context->font = NULL;
    }

  if (context->buffer)
    {
      g_object_unref (context->buffer);
      context->buffer = NULL;
    }

  if (context->imagefile)
    {
      g_object_unref (context->imagefile);
      context->imagefile = NULL;
    }

  if (context->template)
    {
      g_object_unref (context->template);
      context->template = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_context_finalize (GObject *object)
{
  PicmanContext *context = PICMAN_CONTEXT (object);

  context->parent  = NULL;
  context->image   = NULL;
  context->display = NULL;

  if (context->tool_name)
    {
      g_free (context->tool_name);
      context->tool_name = NULL;
    }

  if (context->paint_name)
    {
      g_free (context->paint_name);
      context->paint_name = NULL;
    }

  if (context->brush_name)
    {
      g_free (context->brush_name);
      context->brush_name = NULL;
    }

  if (context->dynamics_name)
    {
      g_free (context->dynamics_name);
      context->dynamics_name = NULL;
    }

  if (context->pattern_name)
    {
      g_free (context->pattern_name);
      context->pattern_name = NULL;
    }

  if (context->gradient_name)
    {
      g_free (context->gradient_name);
      context->gradient_name = NULL;
    }

  if (context->palette_name)
    {
      g_free (context->palette_name);
      context->palette_name = NULL;
    }

  if (context->tool_preset_name)
    {
      g_free (context->tool_preset_name);
      context->tool_preset_name = NULL;
    }

  if (context->font_name)
    {
      g_free (context->font_name);
      context->font_name = NULL;
    }

  if (context->buffer_name)
    {
      g_free (context->buffer_name);
      context->buffer_name = NULL;
    }

  if (context->imagefile_name)
    {
      g_free (context->imagefile_name);
      context->imagefile_name = NULL;
    }

  if (context->template_name)
    {
      g_free (context->template_name);
      context->template_name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_context_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  PicmanContext *context = PICMAN_CONTEXT (object);

  switch (property_id)
    {
    case PICMAN_CONTEXT_PROP_PICMAN:
      context->picman = g_value_get_object (value);
      break;
    case PICMAN_CONTEXT_PROP_IMAGE:
      picman_context_set_image (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_DISPLAY:
      picman_context_set_display (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_TOOL:
      picman_context_set_tool (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_PAINT_INFO:
      picman_context_set_paint_info (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_FOREGROUND:
      picman_context_set_foreground (context, g_value_get_boxed (value));
      break;
    case PICMAN_CONTEXT_PROP_BACKGROUND:
      picman_context_set_background (context, g_value_get_boxed (value));
      break;
    case PICMAN_CONTEXT_PROP_OPACITY:
      picman_context_set_opacity (context, g_value_get_double (value));
      break;
    case PICMAN_CONTEXT_PROP_PAINT_MODE:
      picman_context_set_paint_mode (context, g_value_get_enum (value));
      break;
    case PICMAN_CONTEXT_PROP_BRUSH:
      picman_context_set_brush (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_DYNAMICS:
      picman_context_set_dynamics (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_PATTERN:
      picman_context_set_pattern (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_GRADIENT:
      picman_context_set_gradient (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_PALETTE:
      picman_context_set_palette (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_TOOL_PRESET:
      picman_context_set_tool_preset (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_FONT:
      picman_context_set_font (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_BUFFER:
      picman_context_set_buffer (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_IMAGEFILE:
      picman_context_set_imagefile (context, g_value_get_object (value));
      break;
    case PICMAN_CONTEXT_PROP_TEMPLATE:
      picman_context_set_template (context, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_context_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  PicmanContext *context = PICMAN_CONTEXT (object);

  switch (property_id)
    {
    case PICMAN_CONTEXT_PROP_PICMAN:
      g_value_set_object (value, context->picman);
      break;
    case PICMAN_CONTEXT_PROP_IMAGE:
      g_value_set_object (value, picman_context_get_image (context));
      break;
    case PICMAN_CONTEXT_PROP_DISPLAY:
      g_value_set_object (value, picman_context_get_display (context));
      break;
    case PICMAN_CONTEXT_PROP_TOOL:
      g_value_set_object (value, picman_context_get_tool (context));
      break;
    case PICMAN_CONTEXT_PROP_PAINT_INFO:
      g_value_set_object (value, picman_context_get_paint_info (context));
      break;
    case PICMAN_CONTEXT_PROP_FOREGROUND:
      {
        PicmanRGB color;

        picman_context_get_foreground (context, &color);
        g_value_set_boxed (value, &color);
      }
      break;
    case PICMAN_CONTEXT_PROP_BACKGROUND:
      {
        PicmanRGB color;

        picman_context_get_background (context, &color);
        g_value_set_boxed (value, &color);
      }
      break;
    case PICMAN_CONTEXT_PROP_OPACITY:
      g_value_set_double (value, picman_context_get_opacity (context));
      break;
    case PICMAN_CONTEXT_PROP_PAINT_MODE:
      g_value_set_enum (value, picman_context_get_paint_mode (context));
      break;
    case PICMAN_CONTEXT_PROP_BRUSH:
      g_value_set_object (value, picman_context_get_brush (context));
      break;
    case PICMAN_CONTEXT_PROP_DYNAMICS:
      g_value_set_object (value, picman_context_get_dynamics (context));
      break;
    case PICMAN_CONTEXT_PROP_PATTERN:
      g_value_set_object (value, picman_context_get_pattern (context));
      break;
    case PICMAN_CONTEXT_PROP_GRADIENT:
      g_value_set_object (value, picman_context_get_gradient (context));
      break;
    case PICMAN_CONTEXT_PROP_PALETTE:
      g_value_set_object (value, picman_context_get_palette (context));
      break;
    case PICMAN_CONTEXT_PROP_TOOL_PRESET:
      g_value_set_object (value, picman_context_get_tool_preset (context));
      break;
    case PICMAN_CONTEXT_PROP_FONT:
      g_value_set_object (value, picman_context_get_font (context));
      break;
    case PICMAN_CONTEXT_PROP_BUFFER:
      g_value_set_object (value, picman_context_get_buffer (context));
      break;
    case PICMAN_CONTEXT_PROP_IMAGEFILE:
      g_value_set_object (value, picman_context_get_imagefile (context));
      break;
    case PICMAN_CONTEXT_PROP_TEMPLATE:
      g_value_set_object (value, picman_context_get_template (context));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_context_get_memsize (PicmanObject *object,
                          gint64     *gui_size)
{
  PicmanContext *context = PICMAN_CONTEXT (object);
  gint64       memsize = 0;

  memsize += picman_string_get_memsize (context->tool_name);
  memsize += picman_string_get_memsize (context->paint_name);
  memsize += picman_string_get_memsize (context->brush_name);
  memsize += picman_string_get_memsize (context->dynamics_name);
  memsize += picman_string_get_memsize (context->pattern_name);
  memsize += picman_string_get_memsize (context->palette_name);
  memsize += picman_string_get_memsize (context->tool_preset_name);
  memsize += picman_string_get_memsize (context->font_name);
  memsize += picman_string_get_memsize (context->buffer_name);
  memsize += picman_string_get_memsize (context->imagefile_name);
  memsize += picman_string_get_memsize (context->template_name);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_context_serialize (PicmanConfig       *config,
                        PicmanConfigWriter *writer,
                        gpointer          data)
{
  return picman_config_serialize_changed_properties (config, writer);
}

static gboolean
picman_context_serialize_property (PicmanConfig       *config,
                                 guint             property_id,
                                 const GValue     *value,
                                 GParamSpec       *pspec,
                                 PicmanConfigWriter *writer)
{
  PicmanContext *context = PICMAN_CONTEXT (config);
  PicmanObject  *serialize_obj;

  /*  serialize nothing if the property is not in serialize_props  */
  if (! ((1 << property_id) & context->serialize_props))
    return TRUE;

  switch (property_id)
    {
    case PICMAN_CONTEXT_PROP_TOOL:
    case PICMAN_CONTEXT_PROP_PAINT_INFO:
    case PICMAN_CONTEXT_PROP_BRUSH:
    case PICMAN_CONTEXT_PROP_DYNAMICS:
    case PICMAN_CONTEXT_PROP_PATTERN:
    case PICMAN_CONTEXT_PROP_GRADIENT:
    case PICMAN_CONTEXT_PROP_PALETTE:
    case PICMAN_CONTEXT_PROP_TOOL_PRESET:
    case PICMAN_CONTEXT_PROP_FONT:
      serialize_obj = g_value_get_object (value);
      break;

    default:
      return FALSE;
    }

  picman_config_writer_open (writer, pspec->name);

  if (serialize_obj)
    picman_config_writer_string (writer, picman_object_get_name (serialize_obj));
  else
    picman_config_writer_print (writer, "NULL", 4);

  picman_config_writer_close (writer);

  return TRUE;
}

static gboolean
picman_context_deserialize_property (PicmanConfig *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec,
                                   GScanner   *scanner,
                                   GTokenType *expected)
{
  PicmanContext   *context = PICMAN_CONTEXT (object);
  PicmanContainer *container;
  PicmanObject    *current;
  gchar        **name_loc;
  gboolean       no_data = FALSE;
  gchar         *object_name;

  switch (property_id)
    {
    case PICMAN_CONTEXT_PROP_TOOL:
      container = context->picman->tool_info_list;
      current   = (PicmanObject *) context->tool_info;
      name_loc  = &context->tool_name;
      no_data   = TRUE;
      break;

    case PICMAN_CONTEXT_PROP_PAINT_INFO:
      container = context->picman->paint_info_list;
      current   = (PicmanObject *) context->paint_info;
      name_loc  = &context->paint_name;
      no_data   = TRUE;
      break;

    case PICMAN_CONTEXT_PROP_BRUSH:
      container = picman_data_factory_get_container (context->picman->brush_factory);
      current   = (PicmanObject *) context->brush;
      name_loc  = &context->brush_name;
      break;

    case PICMAN_CONTEXT_PROP_DYNAMICS:
      container = picman_data_factory_get_container (context->picman->dynamics_factory);
      current   = (PicmanObject *) context->dynamics;
      name_loc  = &context->dynamics_name;
      break;

    case PICMAN_CONTEXT_PROP_PATTERN:
      container = picman_data_factory_get_container (context->picman->pattern_factory);
      current   = (PicmanObject *) context->pattern;
      name_loc  = &context->pattern_name;
      break;

    case PICMAN_CONTEXT_PROP_GRADIENT:
      container = picman_data_factory_get_container (context->picman->gradient_factory);
      current   = (PicmanObject *) context->gradient;
      name_loc  = &context->gradient_name;
      break;

    case PICMAN_CONTEXT_PROP_PALETTE:
      container = picman_data_factory_get_container (context->picman->palette_factory);
      current   = (PicmanObject *) context->palette;
      name_loc  = &context->palette_name;
      break;

    case PICMAN_CONTEXT_PROP_TOOL_PRESET:
      container = picman_data_factory_get_container (context->picman->tool_preset_factory);
      current   = (PicmanObject *) context->tool_preset;
      name_loc  = &context->tool_preset_name;
      break;

    case PICMAN_CONTEXT_PROP_FONT:
      container = context->picman->fonts;
      current   = (PicmanObject *) context->font;
      name_loc  = &context->font_name;
      break;

    default:
      return FALSE;
    }

  if (! no_data)
    no_data = context->picman->no_data;

  if (picman_scanner_parse_identifier (scanner, "NULL"))
    {
      g_value_set_object (value, NULL);
    }
  else if (picman_scanner_parse_string (scanner, &object_name))
    {
      PicmanObject *deserialize_obj;

      if (! object_name)
        object_name = g_strdup ("");

      deserialize_obj = picman_container_get_child_by_name (container,
                                                          object_name);

      if (! deserialize_obj)
        {
          if (no_data)
            {
              g_free (*name_loc);
              *name_loc = g_strdup (object_name);
            }
          else
            {
              deserialize_obj = current;
            }
        }

      g_value_set_object (value, deserialize_obj);

      g_free (object_name);
    }
  else
    {
      *expected = G_TOKEN_STRING;
    }

  return TRUE;
}


/*****************************************************************************/
/*  public functions  ********************************************************/

PicmanContext *
picman_context_new (Picman        *picman,
                  const gchar *name,
                  PicmanContext *template)
{
  PicmanContext *context;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (! template || PICMAN_IS_CONTEXT (template), NULL);

  context = g_object_new (PICMAN_TYPE_CONTEXT,
                          "name", name,
                          "picman", picman,
                          NULL);

  if (template)
    {
      context->defined_props = template->defined_props;

      picman_context_copy_properties (template, context,
                                    PICMAN_CONTEXT_ALL_PROPS_MASK);
    }

  return context;
}

PicmanContext *
picman_context_get_parent (const PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->parent;
}

static void
picman_context_parent_notify (PicmanContext *parent,
                            GParamSpec  *pspec,
                            PicmanContext *context)
{
  /*  copy from parent if the changed property is undefined  */
  if (pspec->owner_type == PICMAN_TYPE_CONTEXT &&
      ! ((1 << pspec->param_id) & context->defined_props))
    {
      picman_context_copy_property (parent, context, pspec->param_id);
    }
}

void
picman_context_set_parent (PicmanContext *context,
                         PicmanContext *parent)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (parent == NULL || PICMAN_IS_CONTEXT (parent));
  g_return_if_fail (parent == NULL || parent->parent != context);
  g_return_if_fail (context != parent);

  if (context->parent == parent)
    return;

  if (context->parent)
    {
      g_signal_handlers_disconnect_by_func (context->parent,
                                            picman_context_parent_notify,
                                            context);
    }

  context->parent = parent;

  if (parent)
    {
      /*  copy all undefined properties from the new parent  */
      picman_context_copy_properties (parent, context,
                                    ~context->defined_props &
                                    PICMAN_CONTEXT_ALL_PROPS_MASK);

      g_signal_connect_object (parent, "notify",
                               G_CALLBACK (picman_context_parent_notify),
                               context,
                               0);
    }
}


/*  define / undefinine context properties  */

void
picman_context_define_property (PicmanContext         *context,
                              PicmanContextPropType  prop,
                              gboolean             defined)
{
  PicmanContextPropMask mask;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail ((prop >= PICMAN_CONTEXT_FIRST_PROP) &&
                    (prop <= PICMAN_CONTEXT_LAST_PROP));

  mask = (1 << prop);

  if (defined)
    {
      if (! (context->defined_props & mask))
        {
          context->defined_props |= mask;
        }
    }
  else
    {
      if (context->defined_props & mask)
        {
          context->defined_props &= ~mask;

          if (context->parent)
            picman_context_copy_property (context->parent, context, prop);
        }
    }
}

gboolean
picman_context_property_defined (PicmanContext         *context,
                               PicmanContextPropType  prop)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);

  return (context->defined_props & (1 << prop)) ? TRUE : FALSE;
}

void
picman_context_define_properties (PicmanContext         *context,
                                PicmanContextPropMask  prop_mask,
                                gboolean             defined)
{
  PicmanContextPropType prop;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  for (prop = PICMAN_CONTEXT_FIRST_PROP; prop <= PICMAN_CONTEXT_LAST_PROP; prop++)
    if ((1 << prop) & prop_mask)
      picman_context_define_property (context, prop, defined);
}


/*  specify which context properties will be serialized  */

void
picman_context_set_serialize_properties (PicmanContext         *context,
                                       PicmanContextPropMask  props_mask)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  context->serialize_props = props_mask;
}

PicmanContextPropMask
picman_context_get_serialize_properties (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), 0);

  return context->serialize_props;
}


/*  copying context properties  */

void
picman_context_copy_property (PicmanContext         *src,
                            PicmanContext         *dest,
                            PicmanContextPropType  prop)
{
  gpointer   object          = NULL;
  gpointer   standard_object = NULL;
  gchar     *src_name        = NULL;
  gchar    **dest_name_loc   = NULL;

  g_return_if_fail (PICMAN_IS_CONTEXT (src));
  g_return_if_fail (PICMAN_IS_CONTEXT (dest));
  g_return_if_fail ((prop >= PICMAN_CONTEXT_FIRST_PROP) &&
                    (prop <= PICMAN_CONTEXT_LAST_PROP));

  switch (prop)
    {
    case PICMAN_CONTEXT_PROP_IMAGE:
      picman_context_real_set_image (dest, src->image);
      break;

    case PICMAN_CONTEXT_PROP_DISPLAY:
      picman_context_real_set_display (dest, src->display);
      break;

    case PICMAN_CONTEXT_PROP_TOOL:
      picman_context_real_set_tool (dest, src->tool_info);
      object          = src->tool_info;
      standard_object = picman_tool_info_get_standard (src->picman);
      src_name        = src->tool_name;
      dest_name_loc   = &dest->tool_name;
      break;

    case PICMAN_CONTEXT_PROP_PAINT_INFO:
      picman_context_real_set_paint_info (dest, src->paint_info);
      object          = src->paint_info;
      standard_object = picman_paint_info_get_standard (src->picman);
      src_name        = src->paint_name;
      dest_name_loc   = &dest->paint_name;
      break;

    case PICMAN_CONTEXT_PROP_FOREGROUND:
      picman_context_real_set_foreground (dest, &src->foreground);
      break;

    case PICMAN_CONTEXT_PROP_BACKGROUND:
      picman_context_real_set_background (dest, &src->background);
      break;

    case PICMAN_CONTEXT_PROP_OPACITY:
      picman_context_real_set_opacity (dest, src->opacity);
      break;

    case PICMAN_CONTEXT_PROP_PAINT_MODE:
      picman_context_real_set_paint_mode (dest, src->paint_mode);
      break;

    case PICMAN_CONTEXT_PROP_BRUSH:
      picman_context_real_set_brush (dest, src->brush);
      object          = src->brush;
      standard_object = picman_brush_get_standard (src);
      src_name        = src->brush_name;
      dest_name_loc   = &dest->brush_name;
      break;

    case PICMAN_CONTEXT_PROP_DYNAMICS:
      picman_context_real_set_dynamics (dest, src->dynamics);
      object          = src->dynamics;
      standard_object = picman_dynamics_get_standard (src);
      src_name        = src->dynamics_name;
      dest_name_loc   = &dest->dynamics_name;
      break;

    case PICMAN_CONTEXT_PROP_PATTERN:
      picman_context_real_set_pattern (dest, src->pattern);
      object          = src->pattern;
      standard_object = picman_pattern_get_standard (src);
      src_name        = src->pattern_name;
      dest_name_loc   = &dest->pattern_name;
      break;

    case PICMAN_CONTEXT_PROP_GRADIENT:
      picman_context_real_set_gradient (dest, src->gradient);
      object          = src->gradient;
      standard_object = picman_gradient_get_standard (src);
      src_name        = src->gradient_name;
      dest_name_loc   = &dest->gradient_name;
      break;

    case PICMAN_CONTEXT_PROP_PALETTE:
      picman_context_real_set_palette (dest, src->palette);
      object          = src->palette;
      standard_object = picman_palette_get_standard (src);
      src_name        = src->palette_name;
      dest_name_loc   = &dest->palette_name;
      break;

    case PICMAN_CONTEXT_PROP_TOOL_PRESET:
      picman_context_real_set_tool_preset (dest, src->tool_preset);
      object          = src->tool_preset;
      src_name        = src->tool_preset_name;
      dest_name_loc   = &dest->tool_preset_name;
      break;

    case PICMAN_CONTEXT_PROP_FONT:
      picman_context_real_set_font (dest, src->font);
      object          = src->font;
      standard_object = picman_font_get_standard ();
      src_name        = src->font_name;
      dest_name_loc   = &dest->font_name;
      break;

    case PICMAN_CONTEXT_PROP_BUFFER:
      picman_context_real_set_buffer (dest, src->buffer);
      break;

    case PICMAN_CONTEXT_PROP_IMAGEFILE:
      picman_context_real_set_imagefile (dest, src->imagefile);
      break;

    case PICMAN_CONTEXT_PROP_TEMPLATE:
      picman_context_real_set_template (dest, src->template);
      break;

    default:
      break;
    }

  if (src_name && dest_name_loc)
    {
      if (! object || (standard_object && object == standard_object))
        {
          g_free (*dest_name_loc);
          *dest_name_loc = g_strdup (src_name);
        }
    }
}

void
picman_context_copy_properties (PicmanContext         *src,
                              PicmanContext         *dest,
                              PicmanContextPropMask  prop_mask)
{
  PicmanContextPropType prop;

  g_return_if_fail (PICMAN_IS_CONTEXT (src));
  g_return_if_fail (PICMAN_IS_CONTEXT (dest));

  for (prop = PICMAN_CONTEXT_FIRST_PROP; prop <= PICMAN_CONTEXT_LAST_PROP; prop++)
    if ((1 << prop) & prop_mask)
      picman_context_copy_property (src, dest, prop);
}

/*  attribute access functions  */

/*****************************************************************************/
/*  manipulate by GType  *****************************************************/

PicmanContextPropType
picman_context_type_to_property (GType type)
{
  PicmanContextPropType prop;

  for (prop = PICMAN_CONTEXT_FIRST_PROP; prop <= PICMAN_CONTEXT_LAST_PROP; prop++)
    {
      if (g_type_is_a (type, picman_context_prop_types[prop]))
        return prop;
    }

  return -1;
}

const gchar *
picman_context_type_to_prop_name (GType type)
{
  PicmanContextPropType prop;

  for (prop = PICMAN_CONTEXT_FIRST_PROP; prop <= PICMAN_CONTEXT_LAST_PROP; prop++)
    {
      if (g_type_is_a (type, picman_context_prop_types[prop]))
        return picman_context_prop_names[prop];
    }

  return NULL;
}

const gchar *
picman_context_type_to_signal_name (GType type)
{
  PicmanContextPropType prop;

  for (prop = PICMAN_CONTEXT_FIRST_PROP; prop <= PICMAN_CONTEXT_LAST_PROP; prop++)
    {
      if (g_type_is_a (type, picman_context_prop_types[prop]))
        return g_signal_name (picman_context_signals[prop]);
    }

  return NULL;
}

PicmanObject *
picman_context_get_by_type (PicmanContext *context,
                          GType        type)
{
  PicmanContextPropType  prop;
  PicmanObject          *object = NULL;

  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail ((prop = picman_context_type_to_property (type)) != -1,
                        NULL);

  g_object_get (context,
                picman_context_prop_names[prop], &object,
                NULL);

  /*  g_object_get() refs the object, this function however is a getter,
   *  which usually doesn't ref it's return value
   */
  if (object)
    g_object_unref (object);

  return object;
}

void
picman_context_set_by_type (PicmanContext *context,
                          GType        type,
                          PicmanObject  *object)
{
  PicmanContextPropType prop;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail ((prop = picman_context_type_to_property (type)) != -1);

  g_object_set (context,
                picman_context_prop_names[prop], object,
                NULL);
}

void
picman_context_changed_by_type (PicmanContext *context,
                              GType        type)
{
  PicmanContextPropType  prop;
  PicmanObject          *object;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail ((prop = picman_context_type_to_property (type)) != -1);

  object = picman_context_get_by_type (context, type);

  g_signal_emit (context,
                 picman_context_signals[prop], 0,
                 object);
}

/*****************************************************************************/
/*  image  *******************************************************************/

PicmanImage *
picman_context_get_image (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->image;
}

void
picman_context_set_image (PicmanContext *context,
                        PicmanImage   *image)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_IMAGE);

  picman_context_real_set_image (context, image);
}

void
picman_context_image_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[IMAGE_CHANGED], 0,
                 context->image);
}

/*  handle disappearing images  */
static void
picman_context_image_removed (PicmanContainer *container,
                            PicmanImage     *image,
                            PicmanContext   *context)
{
  if (context->image == image)
    picman_context_real_set_image (context, NULL);
}

static void
picman_context_real_set_image (PicmanContext *context,
                             PicmanImage   *image)
{
  if (context->image == image)
    return;

  context->image = image;

  g_object_notify (G_OBJECT (context), "image");
  picman_context_image_changed (context);
}


/*****************************************************************************/
/*  display  *****************************************************************/

gpointer
picman_context_get_display (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->display;
}

void
picman_context_set_display (PicmanContext *context,
                          gpointer     display)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_DISPLAY);

  picman_context_real_set_display (context, display);
}

void
picman_context_display_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[DISPLAY_CHANGED], 0,
                 context->display);
}

/*  handle disappearing displays  */
static void
picman_context_display_removed (PicmanContainer *container,
                              gpointer       display,
                              PicmanContext   *context)
{
  if (context->display == display)
    picman_context_real_set_display (context, NULL);
}

static void
picman_context_real_set_display (PicmanContext *context,
                               gpointer     display)
{
  PicmanObject *old_display;

  if (context->display == display)
    {
      /*  make sure that setting a display *always* sets the image
       *  to that display's image, even if the display already
       *  matches
       */
      if (display)
        {
          PicmanImage *image;

          g_object_get (display, "image", &image, NULL);

          picman_context_real_set_image (context, image);

          if (image)
            g_object_unref (image);
        }

      return;
    }

  old_display = context->display;

  context->display = display;

  if (context->display)
    {
      PicmanImage *image;

      g_object_get (display, "image", &image, NULL);

      picman_context_real_set_image (context, image);

      if (image)
        g_object_unref (image);
    }
  else if (old_display)
    {
      picman_context_real_set_image (context, NULL);
    }

  g_object_notify (G_OBJECT (context), "display");
  picman_context_display_changed (context);
}


/*****************************************************************************/
/*  tool  ********************************************************************/

PicmanToolInfo *
picman_context_get_tool (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->tool_info;
}

void
picman_context_set_tool (PicmanContext  *context,
                       PicmanToolInfo *tool_info)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (! tool_info || PICMAN_IS_TOOL_INFO (tool_info));
  context_find_defined (context, PICMAN_CONTEXT_PROP_TOOL);

  picman_context_real_set_tool (context, tool_info);
}

void
picman_context_tool_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[TOOL_CHANGED], 0,
                 context->tool_info);
}

/*  the active tool was modified  */
static void
picman_context_tool_dirty (PicmanToolInfo *tool_info,
                         PicmanContext  *context)
{
  g_free (context->tool_name);
  context->tool_name = g_strdup (picman_object_get_name (tool_info));
}

/*  the global tool list is there again after refresh  */
static void
picman_context_tool_list_thaw (PicmanContainer *container,
                             PicmanContext   *context)
{
  PicmanToolInfo *tool_info;

  if (! context->tool_name)
    context->tool_name = g_strdup ("picman-paintbrush-tool");

  tool_info = picman_context_find_object (context, container,
                                        context->tool_name,
                                        picman_tool_info_get_standard (context->picman));

  picman_context_real_set_tool (context, tool_info);
}

/*  the active tool disappeared  */
static void
picman_context_tool_removed (PicmanContainer *container,
                           PicmanToolInfo  *tool_info,
                           PicmanContext   *context)
{
  if (tool_info == context->tool_info)
    {
      context->tool_info = NULL;

      g_signal_handlers_disconnect_by_func (tool_info,
                                            picman_context_tool_dirty,
                                            context);
      g_object_unref (tool_info);

      if (! picman_container_frozen (container))
        picman_context_tool_list_thaw (container, context);
    }
}

static void
picman_context_real_set_tool (PicmanContext  *context,
                            PicmanToolInfo *tool_info)
{
  if (context->tool_info == tool_info)
    return;

  if (context->tool_name &&
      tool_info != picman_tool_info_get_standard (context->picman))
    {
      g_free (context->tool_name);
      context->tool_name = NULL;
    }

  /*  disconnect from the old tool's signals  */
  if (context->tool_info)
    {
      g_signal_handlers_disconnect_by_func (context->tool_info,
                                            picman_context_tool_dirty,
                                            context);
      g_object_unref (context->tool_info);
    }

  context->tool_info = tool_info;

  if (tool_info)
    {
      g_object_ref (tool_info);

      g_signal_connect_object (tool_info, "name-changed",
                               G_CALLBACK (picman_context_tool_dirty),
                               context,
                               0);

      if (tool_info != picman_tool_info_get_standard (context->picman))
        context->tool_name = g_strdup (picman_object_get_name (tool_info));

      if (tool_info->paint_info)
        picman_context_real_set_paint_info (context, tool_info->paint_info);
    }

  g_object_notify (G_OBJECT (context), "tool");
  picman_context_tool_changed (context);
}


/*****************************************************************************/
/*  paint info  **************************************************************/

PicmanPaintInfo *
picman_context_get_paint_info (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->paint_info;
}

void
picman_context_set_paint_info (PicmanContext   *context,
                             PicmanPaintInfo *paint_info)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (! paint_info || PICMAN_IS_PAINT_INFO (paint_info));
  context_find_defined (context, PICMAN_CONTEXT_PROP_PAINT_INFO);

  picman_context_real_set_paint_info (context, paint_info);
}

void
picman_context_paint_info_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[PAINT_INFO_CHANGED], 0,
                 context->paint_info);
}

/*  the active paint info was modified  */
static void
picman_context_paint_info_dirty (PicmanPaintInfo *paint_info,
                               PicmanContext   *context)
{
  g_free (context->paint_name);
  context->paint_name = g_strdup (picman_object_get_name (paint_info));
}

/*  the global paint info list is there again after refresh  */
static void
picman_context_paint_info_list_thaw (PicmanContainer *container,
                                   PicmanContext   *context)
{
  PicmanPaintInfo *paint_info;

  if (! context->paint_name)
    context->paint_name = g_strdup ("picman-paintbrush");

  paint_info = picman_context_find_object (context, container,
                                         context->paint_name,
                                         picman_paint_info_get_standard (context->picman));

  picman_context_real_set_paint_info (context, paint_info);
}

/*  the active paint info disappeared  */
static void
picman_context_paint_info_removed (PicmanContainer *container,
                                 PicmanPaintInfo *paint_info,
                                 PicmanContext   *context)
{
  if (paint_info == context->paint_info)
    {
      context->paint_info = NULL;

      g_signal_handlers_disconnect_by_func (paint_info,
                                            picman_context_paint_info_dirty,
                                            context);
      g_object_unref (paint_info);

      if (! picman_container_frozen (container))
        picman_context_paint_info_list_thaw (container, context);
    }
}

static void
picman_context_real_set_paint_info (PicmanContext   *context,
                                  PicmanPaintInfo *paint_info)
{
  if (context->paint_info == paint_info)
    return;

  if (context->paint_name &&
      paint_info != picman_paint_info_get_standard (context->picman))
    {
      g_free (context->paint_name);
      context->paint_name = NULL;
    }

  /*  disconnect from the old paint info's signals  */
  if (context->paint_info)
    {
      g_signal_handlers_disconnect_by_func (context->paint_info,
                                            picman_context_paint_info_dirty,
                                            context);
      g_object_unref (context->paint_info);
    }

  context->paint_info = paint_info;

  if (paint_info)
    {
      g_object_ref (paint_info);

      g_signal_connect_object (paint_info, "name-changed",
                               G_CALLBACK (picman_context_paint_info_dirty),
                               context,
                               0);

      if (paint_info != picman_paint_info_get_standard (context->picman))
        context->paint_name = g_strdup (picman_object_get_name (paint_info));
    }

  g_object_notify (G_OBJECT (context), "paint-info");
  picman_context_paint_info_changed (context);
}


/*****************************************************************************/
/*  foreground color  ********************************************************/

void
picman_context_get_foreground (PicmanContext *context,
                             PicmanRGB     *color)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (color != NULL);

  *color = context->foreground;
}

void
picman_context_get_foreground_pixel (PicmanContext *context,
                                   const Babl  *pixel_format,
                                   gpointer     pixel)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (pixel_format != NULL);
  g_return_if_fail (pixel != NULL);

  picman_rgba_get_pixel (&context->foreground, pixel_format, pixel);
}

void
picman_context_set_foreground (PicmanContext   *context,
                             const PicmanRGB *color)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (color != NULL);
  context_find_defined (context, PICMAN_CONTEXT_PROP_FOREGROUND);

  picman_context_real_set_foreground (context, color);
}

void
picman_context_set_foreground_pixel (PicmanContext   *context,
                                   const Babl    *pixel_format,
                                   gconstpointer  pixel)
{
  PicmanRGB color;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (pixel_format != NULL);
  g_return_if_fail (pixel != NULL);

  picman_rgba_set_pixel (&color, pixel_format, pixel);

  picman_context_set_foreground (context, &color);
}

void
picman_context_foreground_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[FOREGROUND_CHANGED], 0,
                 &context->foreground);
}

static void
picman_context_real_set_foreground (PicmanContext   *context,
                                  const PicmanRGB *color)
{
  if (picman_rgba_distance (&context->foreground, color) < 0.0001)
    return;

  context->foreground = *color;
  picman_rgb_set_alpha (&context->foreground, PICMAN_OPACITY_OPAQUE);

  g_object_notify (G_OBJECT (context), "foreground");
  picman_context_foreground_changed (context);
}


/*****************************************************************************/
/*  background color  ********************************************************/

void
picman_context_get_background (PicmanContext *context,
                             PicmanRGB     *color)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_return_if_fail (color != NULL);

  *color = context->background;
}

void
picman_context_get_background_pixel (PicmanContext *context,
                                   const Babl  *pixel_format,
                                   gpointer     pixel)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (pixel_format != NULL);
  g_return_if_fail (pixel != NULL);

  picman_rgba_get_pixel (&context->background, pixel_format, pixel);
}

void
picman_context_set_background (PicmanContext   *context,
                             const PicmanRGB *color)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (color != NULL);
  context_find_defined (context, PICMAN_CONTEXT_PROP_BACKGROUND);

  picman_context_real_set_background (context, color);
}

void
picman_context_set_background_pixel (PicmanContext   *context,
                                   const Babl    *pixel_format,
                                   gconstpointer  pixel)
{
  PicmanRGB color;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (pixel_format != NULL);
  g_return_if_fail (pixel != NULL);

  picman_rgba_set_pixel (&color, pixel_format, pixel);

  picman_context_set_background (context, &color);
}

void
picman_context_background_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[BACKGROUND_CHANGED], 0,
                 &context->background);
}

static void
picman_context_real_set_background (PicmanContext   *context,
                                  const PicmanRGB *color)
{
  if (picman_rgba_distance (&context->background, color) < 0.0001)
    return;

  context->background = *color;
  picman_rgb_set_alpha (&context->background, PICMAN_OPACITY_OPAQUE);

  g_object_notify (G_OBJECT (context), "background");
  picman_context_background_changed (context);
}


/*****************************************************************************/
/*  color utility functions  *************************************************/

void
picman_context_set_default_colors (PicmanContext *context)
{
  PicmanContext *bg_context;
  PicmanRGB      fg;
  PicmanRGB      bg;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  bg_context = context;

  context_find_defined (context, PICMAN_CONTEXT_PROP_FOREGROUND);
  context_find_defined (bg_context, PICMAN_CONTEXT_PROP_BACKGROUND);

  picman_rgba_set (&fg, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&bg, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);

  picman_context_real_set_foreground (context, &fg);
  picman_context_real_set_background (bg_context, &bg);
}

void
picman_context_swap_colors (PicmanContext *context)
{
  PicmanContext *bg_context;
  PicmanRGB      fg;
  PicmanRGB      bg;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  bg_context = context;

  context_find_defined (context, PICMAN_CONTEXT_PROP_FOREGROUND);
  context_find_defined (bg_context, PICMAN_CONTEXT_PROP_BACKGROUND);

  picman_context_get_foreground (context, &fg);
  picman_context_get_background (bg_context, &bg);

  picman_context_real_set_foreground (context, &bg);
  picman_context_real_set_background (bg_context, &fg);
}

/*****************************************************************************/
/*  opacity  *****************************************************************/

gdouble
picman_context_get_opacity (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), PICMAN_OPACITY_OPAQUE);

  return context->opacity;
}

void
picman_context_set_opacity (PicmanContext *context,
                          gdouble      opacity)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_OPACITY);

  picman_context_real_set_opacity (context, opacity);
}

void
picman_context_opacity_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[OPACITY_CHANGED], 0,
                 context->opacity);
}

static void
picman_context_real_set_opacity (PicmanContext *context,
                               gdouble      opacity)
{
  if (context->opacity == opacity)
    return;

  context->opacity = opacity;

  g_object_notify (G_OBJECT (context), "opacity");
  picman_context_opacity_changed (context);
}


/*****************************************************************************/
/*  paint mode  **************************************************************/

PicmanLayerModeEffects
picman_context_get_paint_mode (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), PICMAN_NORMAL_MODE);

  return context->paint_mode;
}

void
picman_context_set_paint_mode (PicmanContext          *context,
                             PicmanLayerModeEffects  paint_mode)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_PAINT_MODE);

  picman_context_real_set_paint_mode (context, paint_mode);
}

void
picman_context_paint_mode_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[PAINT_MODE_CHANGED], 0,
                 context->paint_mode);
}

static void
picman_context_real_set_paint_mode (PicmanContext          *context,
                                  PicmanLayerModeEffects  paint_mode)
{
  if (context->paint_mode == paint_mode)
    return;

  context->paint_mode = paint_mode;

  g_object_notify (G_OBJECT (context), "paint-mode");
  picman_context_paint_mode_changed (context);
}


/*****************************************************************************/
/*  brush  *******************************************************************/

PicmanBrush *
picman_context_get_brush (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->brush;
}

void
picman_context_set_brush (PicmanContext *context,
                        PicmanBrush   *brush)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (! brush || PICMAN_IS_BRUSH (brush));
  context_find_defined (context, PICMAN_CONTEXT_PROP_BRUSH);

  picman_context_real_set_brush (context, brush);
}

void
picman_context_brush_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[BRUSH_CHANGED], 0,
                 context->brush);
}

/*  the active brush was modified  */
static void
picman_context_brush_dirty (PicmanBrush   *brush,
                          PicmanContext *context)
{
  g_free (context->brush_name);
  context->brush_name = g_strdup (picman_object_get_name (brush));
}

/*  the global brush list is there again after refresh  */
static void
picman_context_brush_list_thaw (PicmanContainer *container,
                              PicmanContext   *context)
{
  PicmanBrush *brush;

  if (! context->brush_name)
    context->brush_name = g_strdup (context->picman->config->default_brush);

  brush = picman_context_find_object (context, container,
                                    context->brush_name,
                                    picman_brush_get_standard (context));

  picman_context_real_set_brush (context, brush);
}

/*  the active brush disappeared  */
static void
picman_context_brush_removed (PicmanContainer *container,
                            PicmanBrush     *brush,
                            PicmanContext   *context)
{
  if (brush == context->brush)
    {
      context->brush = NULL;

      g_signal_handlers_disconnect_by_func (brush,
                                            picman_context_brush_dirty,
                                            context);
      g_object_unref (brush);

      if (! picman_container_frozen (container))
        picman_context_brush_list_thaw (container, context);
    }
}

static void
picman_context_real_set_brush (PicmanContext *context,
                             PicmanBrush   *brush)
{
  if (context->brush == brush)
    return;

  if (context->brush_name &&
      brush != PICMAN_BRUSH (picman_brush_get_standard (context)))
    {
      g_free (context->brush_name);
      context->brush_name = NULL;
    }

  /*  disconnect from the old brush's signals  */
  if (context->brush)
    {
      g_signal_handlers_disconnect_by_func (context->brush,
                                            picman_context_brush_dirty,
                                            context);
      g_object_unref (context->brush);
    }

  context->brush = brush;

  if (brush)
    {
      g_object_ref (brush);

      g_signal_connect_object (brush, "name-changed",
                               G_CALLBACK (picman_context_brush_dirty),
                               context,
                               0);

      if (brush != PICMAN_BRUSH (picman_brush_get_standard (context)))
        context->brush_name = g_strdup (picman_object_get_name (brush));
    }

  g_object_notify (G_OBJECT (context), "brush");
  picman_context_brush_changed (context);
}


/*****************************************************************************/
/*  dynamics *****************************************************************/

PicmanDynamics *
picman_context_get_dynamics (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->dynamics;
}

void
picman_context_set_dynamics (PicmanContext  *context,
                           PicmanDynamics *dynamics)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (! dynamics || PICMAN_IS_DYNAMICS (dynamics));
  context_find_defined (context, PICMAN_CONTEXT_PROP_DYNAMICS);

  picman_context_real_set_dynamics (context, dynamics);
}

void
picman_context_dynamics_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[DYNAMICS_CHANGED], 0,
                 context->dynamics);
}

static void
picman_context_dynamics_dirty (PicmanDynamics *dynamics,
                             PicmanContext  *context)
{
  g_free (context->dynamics_name);
  context->dynamics_name = g_strdup (picman_object_get_name (dynamics));
}

static void
picman_context_dynamics_removed (PicmanContainer *container,
                               PicmanDynamics  *dynamics,
                               PicmanContext   *context)
{
  if (dynamics == context->dynamics)
    {
      context->dynamics = NULL;

      g_signal_handlers_disconnect_by_func (dynamics,
                                            picman_context_dynamics_dirty,
                                            context);
      g_object_unref (dynamics);

      if (! picman_container_frozen (container))
        picman_context_dynamics_list_thaw (container, context);
    }
}

static void
picman_context_dynamics_list_thaw (PicmanContainer *container,
                                 PicmanContext   *context)
{
  PicmanDynamics *dynamics;

  if (! context->dynamics_name)
    context->dynamics_name = g_strdup (context->picman->config->default_dynamics);

  dynamics = picman_context_find_object (context, container,
                                       context->dynamics_name,
                                       picman_dynamics_get_standard (context));

  picman_context_real_set_dynamics (context, dynamics);
}

static void
picman_context_real_set_dynamics (PicmanContext  *context,
                                PicmanDynamics *dynamics)
{
  if (context->dynamics == dynamics)
    return;

  if (context->dynamics_name &&
      dynamics != PICMAN_DYNAMICS (picman_dynamics_get_standard (context)))
    {
      g_free (context->dynamics_name);
      context->dynamics_name = NULL;
    }

  /*  disconnect from the old dynamics' signals  */
  if (context->dynamics)
    {
      g_signal_handlers_disconnect_by_func (context->dynamics,
                                            picman_context_dynamics_dirty,
                                            context);
      g_object_unref (context->dynamics);
    }

  context->dynamics = dynamics;

  if (dynamics)
    {
      g_object_ref (dynamics);

      g_signal_connect_object (dynamics, "name-changed",
                               G_CALLBACK (picman_context_dynamics_dirty),
                               context,
                               0);

      if (dynamics != PICMAN_DYNAMICS (picman_dynamics_get_standard (context)))
        context->dynamics_name = g_strdup (picman_object_get_name (dynamics));
    }

  g_object_notify (G_OBJECT (context), "dynamics");
  picman_context_dynamics_changed (context);
}


/*****************************************************************************/
/*  pattern  *****************************************************************/

PicmanPattern *
picman_context_get_pattern (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->pattern;
}

void
picman_context_set_pattern (PicmanContext *context,
                          PicmanPattern *pattern)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_PATTERN);

  picman_context_real_set_pattern (context, pattern);
}

void
picman_context_pattern_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[PATTERN_CHANGED], 0,
                 context->pattern);
}

/*  the active pattern was modified  */
static void
picman_context_pattern_dirty (PicmanPattern *pattern,
                            PicmanContext *context)
{
  g_free (context->pattern_name);
  context->pattern_name = g_strdup (picman_object_get_name (pattern));
}

/*  the global pattern list is there again after refresh  */
static void
picman_context_pattern_list_thaw (PicmanContainer *container,
                                PicmanContext   *context)
{
  PicmanPattern *pattern;

  if (! context->pattern_name)
    context->pattern_name = g_strdup (context->picman->config->default_pattern);

  pattern = picman_context_find_object (context, container,
                                      context->pattern_name,
                                      picman_pattern_get_standard (context));

  picman_context_real_set_pattern (context, pattern);
}

/*  the active pattern disappeared  */
static void
picman_context_pattern_removed (PicmanContainer *container,
                              PicmanPattern   *pattern,
                              PicmanContext   *context)
{
  if (pattern == context->pattern)
    {
      context->pattern = NULL;

      g_signal_handlers_disconnect_by_func (pattern,
                                            picman_context_pattern_dirty,
                                            context);
      g_object_unref (pattern);

      if (! picman_container_frozen (container))
        picman_context_pattern_list_thaw (container, context);
    }
}

static void
picman_context_real_set_pattern (PicmanContext *context,
                               PicmanPattern *pattern)
{
  if (context->pattern == pattern)
    return;

  if (context->pattern_name &&
      pattern != PICMAN_PATTERN (picman_pattern_get_standard (context)))
    {
      g_free (context->pattern_name);
      context->pattern_name = NULL;
    }

  /*  disconnect from the old pattern's signals  */
  if (context->pattern)
    {
      g_signal_handlers_disconnect_by_func (context->pattern,
                                            picman_context_pattern_dirty,
                                            context);
      g_object_unref (context->pattern);
    }

  context->pattern = pattern;

  if (pattern)
    {
      g_object_ref (pattern);

      g_signal_connect_object (pattern, "name-changed",
                               G_CALLBACK (picman_context_pattern_dirty),
                               context,
                               0);

      if (pattern != PICMAN_PATTERN (picman_pattern_get_standard (context)))
        context->pattern_name = g_strdup (picman_object_get_name (pattern));
    }

  g_object_notify (G_OBJECT (context), "pattern");
  picman_context_pattern_changed (context);
}


/*****************************************************************************/
/*  gradient  ****************************************************************/

PicmanGradient *
picman_context_get_gradient (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->gradient;
}

void
picman_context_set_gradient (PicmanContext  *context,
                           PicmanGradient *gradient)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_GRADIENT);

  picman_context_real_set_gradient (context, gradient);
}

void
picman_context_gradient_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[GRADIENT_CHANGED], 0,
                 context->gradient);
}

/*  the active gradient was modified  */
static void
picman_context_gradient_dirty (PicmanGradient *gradient,
                             PicmanContext  *context)
{
  g_free (context->gradient_name);
  context->gradient_name = g_strdup (picman_object_get_name (gradient));
}

/*  the global gradient list is there again after refresh  */
static void
picman_context_gradient_list_thaw (PicmanContainer *container,
                                 PicmanContext   *context)
{
  PicmanGradient *gradient;

  if (! context->gradient_name)
    context->gradient_name = g_strdup (context->picman->config->default_gradient);

  gradient = picman_context_find_object (context, container,
                                       context->gradient_name,
                                       picman_gradient_get_standard (context));

  picman_context_real_set_gradient (context, gradient);
}

/*  the active gradient disappeared  */
static void
picman_context_gradient_removed (PicmanContainer *container,
                               PicmanGradient  *gradient,
                               PicmanContext   *context)
{
  if (gradient == context->gradient)
    {
      context->gradient = NULL;

      g_signal_handlers_disconnect_by_func (gradient,
                                            picman_context_gradient_dirty,
                                            context);
      g_object_unref (gradient);

      if (! picman_container_frozen (container))
        picman_context_gradient_list_thaw (container, context);
    }
}

static void
picman_context_real_set_gradient (PicmanContext  *context,
                                PicmanGradient *gradient)
{
  if (context->gradient == gradient)
    return;

  if (context->gradient_name &&
      gradient != PICMAN_GRADIENT (picman_gradient_get_standard (context)))
    {
      g_free (context->gradient_name);
      context->gradient_name = NULL;
    }

  /*  disconnect from the old gradient's signals  */
  if (context->gradient)
    {
      g_signal_handlers_disconnect_by_func (context->gradient,
                                            picman_context_gradient_dirty,
                                            context);
      g_object_unref (context->gradient);
    }

  context->gradient = gradient;

  if (gradient)
    {
      g_object_ref (gradient);

      g_signal_connect_object (gradient, "name-changed",
                               G_CALLBACK (picman_context_gradient_dirty),
                               context,
                               0);

      if (gradient != PICMAN_GRADIENT (picman_gradient_get_standard (context)))
        context->gradient_name = g_strdup (picman_object_get_name (gradient));
    }

  g_object_notify (G_OBJECT (context), "gradient");
  picman_context_gradient_changed (context);
}


/*****************************************************************************/
/*  palette  *****************************************************************/

PicmanPalette *
picman_context_get_palette (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->palette;
}

void
picman_context_set_palette (PicmanContext *context,
                          PicmanPalette *palette)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_PALETTE);

  picman_context_real_set_palette (context, palette);
}

void
picman_context_palette_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[PALETTE_CHANGED], 0,
                 context->palette);
}

/*  the active palette was modified  */
static void
picman_context_palette_dirty (PicmanPalette *palette,
                            PicmanContext *context)
{
  g_free (context->palette_name);
  context->palette_name = g_strdup (picman_object_get_name (palette));
}

/*  the global palette list is there again after refresh  */
static void
picman_context_palette_list_thaw (PicmanContainer *container,
                                PicmanContext   *context)
{
  PicmanPalette *palette;

  if (! context->palette_name)
    context->palette_name = g_strdup (context->picman->config->default_palette);

  palette = picman_context_find_object (context, container,
                                      context->palette_name,
                                      picman_palette_get_standard (context));

  picman_context_real_set_palette (context, palette);
}

/*  the active palette disappeared  */
static void
picman_context_palette_removed (PicmanContainer *container,
                              PicmanPalette   *palette,
                              PicmanContext   *context)
{
  if (palette == context->palette)
    {
      context->palette = NULL;

      g_signal_handlers_disconnect_by_func (palette,
                                            picman_context_palette_dirty,
                                            context);
      g_object_unref (palette);

      if (! picman_container_frozen (container))
        picman_context_palette_list_thaw (container, context);
    }
}

static void
picman_context_real_set_palette (PicmanContext *context,
                               PicmanPalette *palette)
{
  if (context->palette == palette)
    return;

  if (context->palette_name &&
      palette != PICMAN_PALETTE (picman_palette_get_standard (context)))
    {
      g_free (context->palette_name);
      context->palette_name = NULL;
    }

  /*  disconnect from the old palette's signals  */
  if (context->palette)
    {
      g_signal_handlers_disconnect_by_func (context->palette,
                                            picman_context_palette_dirty,
                                            context);
      g_object_unref (context->palette);
    }

  context->palette = palette;

  if (palette)
    {
      g_object_ref (palette);

      g_signal_connect_object (palette, "name-changed",
                               G_CALLBACK (picman_context_palette_dirty),
                               context,
                               0);

      if (palette != PICMAN_PALETTE (picman_palette_get_standard (context)))
        context->palette_name = g_strdup (picman_object_get_name (palette));
    }

  g_object_notify (G_OBJECT (context), "palette");
  picman_context_palette_changed (context);
}


/********************************************************************************/
/*  tool preset *****************************************************************/

PicmanToolPreset *
picman_context_get_tool_preset (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->tool_preset;
}

void
picman_context_set_tool_preset (PicmanContext    *context,
                              PicmanToolPreset *tool_preset)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (! tool_preset || PICMAN_IS_TOOL_PRESET (tool_preset));
  context_find_defined (context, PICMAN_CONTEXT_PROP_TOOL_PRESET);

  picman_context_real_set_tool_preset (context, tool_preset);
}

void
picman_context_tool_preset_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[TOOL_PRESET_CHANGED], 0,
                 context->tool_preset);
}

static void
picman_context_tool_preset_dirty (PicmanToolPreset *tool_preset,
                                PicmanContext    *context)
{
  g_free (context->tool_preset_name);
  context->tool_preset_name = g_strdup (picman_object_get_name (tool_preset));
}

static void
picman_context_tool_preset_removed (PicmanContainer  *container,
                                  PicmanToolPreset *tool_preset,
                                  PicmanContext    *context)
{
  if (tool_preset == context->tool_preset)
    {
      context->tool_preset = NULL;

      g_signal_handlers_disconnect_by_func (tool_preset,
                                            picman_context_tool_preset_dirty,
                                            context);
      g_object_unref (tool_preset);

      if (! picman_container_frozen (container))
        picman_context_tool_preset_list_thaw (container, context);
    }
}

static void
picman_context_tool_preset_list_thaw (PicmanContainer *container,
                                    PicmanContext   *context)
{
  PicmanToolPreset *tool_preset;

  tool_preset = picman_context_find_object (context, container,
                                          context->tool_preset_name, NULL);

  picman_context_real_set_tool_preset (context, tool_preset);
}

static void
picman_context_real_set_tool_preset (PicmanContext    *context,
                                   PicmanToolPreset *tool_preset)
{
  if (context->tool_preset == tool_preset)
    return;

  if (context->tool_preset_name)
    {
      g_free (context->tool_preset_name);
      context->tool_preset_name = NULL;
    }

  /*  disconnect from the old 's signals  */
  if (context->tool_preset)
    {
      g_signal_handlers_disconnect_by_func (context->tool_preset,
                                            picman_context_tool_preset_dirty,
                                            context);
      g_object_unref (context->tool_preset);
    }

  context->tool_preset = tool_preset;

  if (tool_preset)
    {
      g_object_ref (tool_preset);

      g_signal_connect_object (tool_preset, "name-changed",
                               G_CALLBACK (picman_context_tool_preset_dirty),
                               context,
                               0);

      context->tool_preset_name = g_strdup (picman_object_get_name (tool_preset));
    }

  g_object_notify (G_OBJECT (context), "tool-preset");
  picman_context_tool_preset_changed (context);
}


/*****************************************************************************/
/*  font     *****************************************************************/

PicmanFont *
picman_context_get_font (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->font;
}

void
picman_context_set_font (PicmanContext *context,
                       PicmanFont    *font)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_FONT);

  picman_context_real_set_font (context, font);
}

const gchar *
picman_context_get_font_name (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->font_name;
}

void
picman_context_set_font_name (PicmanContext *context,
                            const gchar *name)
{
  PicmanObject *font;

  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  font = picman_container_get_child_by_name (context->picman->fonts, name);

  if (font)
    {
      picman_context_set_font (context, PICMAN_FONT (font));
    }
  else
    {
      /* No font with this name exists, use the standard font, but
       * keep the intended name around
       */
      picman_context_set_font (context, picman_font_get_standard ());

      g_free (context->font_name);
      context->font_name = g_strdup (name);
    }
}

void
picman_context_font_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[FONT_CHANGED], 0,
                 context->font);
}

/*  the active font was modified  */
static void
picman_context_font_dirty (PicmanFont    *font,
                         PicmanContext *context)
{
  g_free (context->font_name);
  context->font_name = g_strdup (picman_object_get_name (font));
}

/*  the global font list is there again after refresh  */
static void
picman_context_font_list_thaw (PicmanContainer *container,
                             PicmanContext   *context)
{
  PicmanFont *font;

  if (! context->font_name)
    context->font_name = g_strdup (context->picman->config->default_font);

  font = picman_context_find_object (context, container,
                                   context->font_name,
                                   picman_font_get_standard ());

  picman_context_real_set_font (context, font);
}

/*  the active font disappeared  */
static void
picman_context_font_removed (PicmanContainer *container,
                           PicmanFont      *font,
                           PicmanContext   *context)
{
  if (font == context->font)
    {
      context->font = NULL;

      g_signal_handlers_disconnect_by_func (font,
                                            picman_context_font_dirty,
                                            context);
      g_object_unref (font);

      if (! picman_container_frozen (container))
        picman_context_font_list_thaw (container, context);
    }
}

static void
picman_context_real_set_font (PicmanContext *context,
                            PicmanFont    *font)
{
  if (context->font == font)
    return;

  if (context->font_name &&
      font != picman_font_get_standard ())
    {
      g_free (context->font_name);
      context->font_name = NULL;
    }

  /*  disconnect from the old font's signals  */
  if (context->font)
    {
      g_signal_handlers_disconnect_by_func (context->font,
                                            picman_context_font_dirty,
                                            context);
      g_object_unref (context->font);
    }

  context->font = font;

  if (font)
    {
      g_object_ref (font);

      g_signal_connect_object (font, "name-changed",
                               G_CALLBACK (picman_context_font_dirty),
                               context,
                               0);

      if (font != picman_font_get_standard ())
        context->font_name = g_strdup (picman_object_get_name (font));
    }

  g_object_notify (G_OBJECT (context), "font");
  picman_context_font_changed (context);
}


/*****************************************************************************/
/*  buffer  ******************************************************************/

PicmanBuffer *
picman_context_get_buffer (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->buffer;
}

void
picman_context_set_buffer (PicmanContext *context,
                         PicmanBuffer *buffer)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_BUFFER);

  picman_context_real_set_buffer (context, buffer);
}

void
picman_context_buffer_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[BUFFER_CHANGED], 0,
                 context->buffer);
}

/*  the active buffer was modified  */
static void
picman_context_buffer_dirty (PicmanBuffer  *buffer,
                           PicmanContext *context)
{
  g_free (context->buffer_name);
  context->buffer_name = g_strdup (picman_object_get_name (buffer));
}

/*  the global buffer list is there again after refresh  */
static void
picman_context_buffer_list_thaw (PicmanContainer *container,
                               PicmanContext   *context)
{
  PicmanBuffer *buffer;

  buffer = picman_context_find_object (context, container,
                                     context->buffer_name,
                                     NULL);

  if (buffer)
    {
      picman_context_real_set_buffer (context, buffer);
    }
  else
    {
      g_object_notify (G_OBJECT (context), "buffer");
      picman_context_buffer_changed (context);
    }
}

/*  the active buffer disappeared  */
static void
picman_context_buffer_removed (PicmanContainer *container,
                             PicmanBuffer    *buffer,
                             PicmanContext   *context)
{
  if (buffer == context->buffer)
    {
      context->buffer = NULL;

      g_signal_handlers_disconnect_by_func (buffer,
                                            picman_context_buffer_dirty,
                                            context);
      g_object_unref (buffer);

      if (! picman_container_frozen (container))
        picman_context_buffer_list_thaw (container, context);
    }
}

static void
picman_context_real_set_buffer (PicmanContext *context,
                              PicmanBuffer  *buffer)
{
  if (context->buffer == buffer)
    return;

  if (context->buffer_name)
    {
      g_free (context->buffer_name);
      context->buffer_name = NULL;
    }

  /*  disconnect from the old buffer's signals  */
  if (context->buffer)
    {
      g_signal_handlers_disconnect_by_func (context->buffer,
                                            picman_context_buffer_dirty,
                                            context);
      g_object_unref (context->buffer);
    }

  context->buffer = buffer;

  if (buffer)
    {
      g_object_ref (buffer);

      g_signal_connect_object (buffer, "name-changed",
                               G_CALLBACK (picman_context_buffer_dirty),
                               context,
                               0);

      context->buffer_name = g_strdup (picman_object_get_name (buffer));
    }

  g_object_notify (G_OBJECT (context), "buffer");
  picman_context_buffer_changed (context);
}


/*****************************************************************************/
/*  imagefile  ***************************************************************/

PicmanImagefile *
picman_context_get_imagefile (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->imagefile;
}

void
picman_context_set_imagefile (PicmanContext   *context,
                            PicmanImagefile *imagefile)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_IMAGEFILE);

  picman_context_real_set_imagefile (context, imagefile);
}

void
picman_context_imagefile_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[IMAGEFILE_CHANGED], 0,
                 context->imagefile);
}

/*  the active imagefile was modified  */
static void
picman_context_imagefile_dirty (PicmanImagefile *imagefile,
                              PicmanContext   *context)
{
  g_free (context->imagefile_name);
  context->imagefile_name = g_strdup (picman_object_get_name (imagefile));
}

/*  the global imagefile list is there again after refresh  */
static void
picman_context_imagefile_list_thaw (PicmanContainer *container,
                                  PicmanContext   *context)
{
  PicmanImagefile *imagefile;

  imagefile = picman_context_find_object (context, container,
                                        context->imagefile_name,
                                        NULL);

  if (imagefile)
    {
      picman_context_real_set_imagefile (context, imagefile);
    }
  else
    {
      g_object_notify (G_OBJECT (context), "imagefile");
      picman_context_imagefile_changed (context);
    }
}

/*  the active imagefile disappeared  */
static void
picman_context_imagefile_removed (PicmanContainer *container,
                                PicmanImagefile *imagefile,
                                PicmanContext   *context)
{
  if (imagefile == context->imagefile)
    {
      context->imagefile = NULL;

      g_signal_handlers_disconnect_by_func (imagefile,
                                            picman_context_imagefile_dirty,
                                            context);
      g_object_unref (imagefile);

      if (! picman_container_frozen (container))
        picman_context_imagefile_list_thaw (container, context);
    }
}

static void
picman_context_real_set_imagefile (PicmanContext   *context,
                                 PicmanImagefile *imagefile)
{
  if (context->imagefile == imagefile)
    return;

  if (context->imagefile_name)
    {
      g_free (context->imagefile_name);
      context->imagefile_name = NULL;
    }

  /*  disconnect from the old imagefile's signals  */
  if (context->imagefile)
    {
      g_signal_handlers_disconnect_by_func (context->imagefile,
                                            picman_context_imagefile_dirty,
                                            context);
      g_object_unref (context->imagefile);
    }

  context->imagefile = imagefile;

  if (imagefile)
    {
      g_object_ref (imagefile);

      g_signal_connect_object (imagefile, "name-changed",
                               G_CALLBACK (picman_context_imagefile_dirty),
                               context,
                               0);

      context->imagefile_name = g_strdup (picman_object_get_name (imagefile));
    }

  g_object_notify (G_OBJECT (context), "imagefile");
  picman_context_imagefile_changed (context);
}


/*****************************************************************************/
/*  template  ***************************************************************/

PicmanTemplate *
picman_context_get_template (PicmanContext *context)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return context->template;
}

void
picman_context_set_template (PicmanContext  *context,
                           PicmanTemplate *template)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  context_find_defined (context, PICMAN_CONTEXT_PROP_TEMPLATE);

  picman_context_real_set_template (context, template);
}

void
picman_context_template_changed (PicmanContext *context)
{
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  g_signal_emit (context,
                 picman_context_signals[TEMPLATE_CHANGED], 0,
                 context->template);
}

/*  the active template was modified  */
static void
picman_context_template_dirty (PicmanTemplate *template,
                             PicmanContext  *context)
{
  g_free (context->template_name);
  context->template_name = g_strdup (picman_object_get_name (template));
}

/*  the global template list is there again after refresh  */
static void
picman_context_template_list_thaw (PicmanContainer *container,
                                 PicmanContext   *context)
{
  PicmanTemplate *template;

  template = picman_context_find_object (context, container,
                                       context->template_name,
                                       NULL);

  if (template)
    {
      picman_context_real_set_template (context, template);
    }
  else
    {
      g_object_notify (G_OBJECT (context), "template");
      picman_context_template_changed (context);
    }
}

/*  the active template disappeared  */
static void
picman_context_template_removed (PicmanContainer *container,
                               PicmanTemplate  *template,
                               PicmanContext   *context)
{
  if (template == context->template)
    {
      context->template = NULL;

      g_signal_handlers_disconnect_by_func (template,
                                            picman_context_template_dirty,
                                            context);
      g_object_unref (template);

      if (! picman_container_frozen (container))
        picman_context_template_list_thaw (container, context);
    }
}

static void
picman_context_real_set_template (PicmanContext  *context,
                                PicmanTemplate *template)
{
  if (context->template == template)
    return;

  if (context->template_name)
    {
      g_free (context->template_name);
      context->template_name = NULL;
    }

  /*  disconnect from the old template's signals  */
  if (context->template)
    {
      g_signal_handlers_disconnect_by_func (context->template,
                                            picman_context_template_dirty,
                                            context);
      g_object_unref (context->template);
    }

  context->template = template;

  if (template)
    {
      g_object_ref (template);

      g_signal_connect_object (template, "name-changed",
                               G_CALLBACK (picman_context_template_dirty),
                               context,
                               0);

      context->template_name = g_strdup (picman_object_get_name (template));
    }

  g_object_notify (G_OBJECT (context), "template");
  picman_context_template_changed (context);
}


/*****************************************************************************/
/*  utility functions  *******************************************************/

static gpointer
picman_context_find_object (PicmanContext   *context,
                          PicmanContainer *container,
                          const gchar   *object_name,
                          gpointer       standard_object)
{
  PicmanObject *object = NULL;

  if (object_name)
    object = picman_container_get_child_by_name (container, object_name);

  if (! object && ! picman_container_is_empty (container))
    object = picman_container_get_child_by_index (container, 0);

  if (! object)
    object = standard_object;

  return object;
}
