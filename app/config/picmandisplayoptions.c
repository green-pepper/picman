/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanDisplayOptions
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "config-types.h"

#include "picmanrc-blurbs.h"

#include "picmandisplayoptions.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SHOW_MENUBAR,
  PROP_SHOW_STATUSBAR,
  PROP_SHOW_RULERS,
  PROP_SHOW_SCROLLBARS,
  PROP_SHOW_SELECTION,
  PROP_SHOW_LAYER_BOUNDARY,
  PROP_SHOW_GUIDES,
  PROP_SHOW_GRID,
  PROP_SHOW_SAMPLE_POINTS,
  PROP_PADDING_MODE,
  PROP_PADDING_COLOR
};


static void   picman_display_options_set_property (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);
static void   picman_display_options_get_property (GObject      *object,
                                                 guint         property_id,
                                                 GValue       *value,
                                                 GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanDisplayOptions,
                         picman_display_options,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

typedef struct _PicmanDisplayOptions      PicmanDisplayOptionsFullscreen;
typedef struct _PicmanDisplayOptionsClass PicmanDisplayOptionsFullscreenClass;

#define picman_display_options_fullscreen_init picman_display_options_init

G_DEFINE_TYPE_WITH_CODE (PicmanDisplayOptionsFullscreen,
                         picman_display_options_fullscreen,
                         PICMAN_TYPE_DISPLAY_OPTIONS,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

typedef struct _PicmanDisplayOptions      PicmanDisplayOptionsNoImage;
typedef struct _PicmanDisplayOptionsClass PicmanDisplayOptionsNoImageClass;

#define picman_display_options_no_image_init picman_display_options_init

G_DEFINE_TYPE_WITH_CODE (PicmanDisplayOptionsNoImage,
                         picman_display_options_no_image,
                         PICMAN_TYPE_DISPLAY_OPTIONS,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))


static void
picman_display_options_class_init (PicmanDisplayOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PicmanRGB       white;

  picman_rgba_set (&white, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);

  object_class->set_property = picman_display_options_set_property;
  object_class->get_property = picman_display_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_MENUBAR,
                                    "show-menubar", SHOW_MENUBAR_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_STATUSBAR,
                                    "show-statusbar", SHOW_STATUSBAR_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_RULERS,
                                    "show-rulers", SHOW_RULERS_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SCROLLBARS,
                                    "show-scrollbars", SHOW_SCROLLBARS_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SELECTION,
                                    "show-selection", SHOW_SELECTION_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_LAYER_BOUNDARY,
                                    "show-layer-boundary", SHOW_LAYER_BOUNDARY_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GUIDES,
                                    "show-guides", SHOW_GUIDES_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GRID,
                                    "show-grid", SHOW_GRID_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SAMPLE_POINTS,
                                    "show-sample-points", SHOW_SAMPLE_POINTS_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_PADDING_MODE,
                                 "padding-mode", CANVAS_PADDING_MODE_BLURB,
                                 PICMAN_TYPE_CANVAS_PADDING_MODE,
                                 PICMAN_CANVAS_PADDING_MODE_DEFAULT,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PROP_PADDING_COLOR,
                                "padding-color", CANVAS_PADDING_COLOR_BLURB,
                                FALSE, &white,
                                PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_display_options_fullscreen_class_init (PicmanDisplayOptionsFullscreenClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PicmanRGB       black;

  picman_rgba_set (&black, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);

  object_class->set_property = picman_display_options_set_property;
  object_class->get_property = picman_display_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_MENUBAR,
                                    "show-menubar", SHOW_MENUBAR_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_STATUSBAR,
                                    "show-statusbar", SHOW_STATUSBAR_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_RULERS,
                                    "show-rulers", SHOW_RULERS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SCROLLBARS,
                                    "show-scrollbars", SHOW_SCROLLBARS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SELECTION,
                                    "show-selection", SHOW_SELECTION_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_LAYER_BOUNDARY,
                                    "show-layer-boundary", SHOW_LAYER_BOUNDARY_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GUIDES,
                                    "show-guides", SHOW_GUIDES_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GRID,
                                    "show-grid", SHOW_GRID_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SAMPLE_POINTS,
                                    "show-sample-points", SHOW_SAMPLE_POINTS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_PADDING_MODE,
                                 "padding-mode", CANVAS_PADDING_MODE_BLURB,
                                 PICMAN_TYPE_CANVAS_PADDING_MODE,
                                 PICMAN_CANVAS_PADDING_MODE_CUSTOM,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PROP_PADDING_COLOR,
                                "padding-color", CANVAS_PADDING_COLOR_BLURB,
                                FALSE, &black,
                                PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_display_options_no_image_class_init (PicmanDisplayOptionsNoImageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_display_options_set_property;
  object_class->get_property = picman_display_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_RULERS,
                                    "show-rulers", SHOW_RULERS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SCROLLBARS,
                                    "show-scrollbars", SHOW_SCROLLBARS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SELECTION,
                                    "show-selection", SHOW_SELECTION_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_LAYER_BOUNDARY,
                                    "show-layer-boundary", SHOW_LAYER_BOUNDARY_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GUIDES,
                                    "show-guides", SHOW_GUIDES_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_GRID,
                                    "show-grid", SHOW_GRID_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_SAMPLE_POINTS,
                                    "show-sample-points", SHOW_SAMPLE_POINTS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_display_options_init (PicmanDisplayOptions *options)
{
  options->padding_mode_set = FALSE;
}

static void
picman_display_options_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanDisplayOptions *options = PICMAN_DISPLAY_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SHOW_MENUBAR:
      options->show_menubar = g_value_get_boolean (value);
      break;
    case PROP_SHOW_STATUSBAR:
      options->show_statusbar = g_value_get_boolean (value);
      break;
    case PROP_SHOW_RULERS:
      options->show_rulers = g_value_get_boolean (value);
      break;
    case PROP_SHOW_SCROLLBARS:
      options->show_scrollbars = g_value_get_boolean (value);
      break;
    case PROP_SHOW_SELECTION:
      options->show_selection = g_value_get_boolean (value);
      break;
    case PROP_SHOW_LAYER_BOUNDARY:
      options->show_layer_boundary = g_value_get_boolean (value);
      break;
    case PROP_SHOW_GUIDES:
      options->show_guides = g_value_get_boolean (value);
      break;
    case PROP_SHOW_GRID:
      options->show_grid = g_value_get_boolean (value);
      break;
    case PROP_SHOW_SAMPLE_POINTS:
      options->show_sample_points = g_value_get_boolean (value);
      break;
    case PROP_PADDING_MODE:
      options->padding_mode = g_value_get_enum (value);
      break;
    case PROP_PADDING_COLOR:
      options->padding_color = *(PicmanRGB *) g_value_get_boxed (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_options_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanDisplayOptions *options = PICMAN_DISPLAY_OPTIONS (object);

  switch (property_id)
    {
    case PROP_SHOW_MENUBAR:
      g_value_set_boolean (value, options->show_menubar);
      break;
    case PROP_SHOW_STATUSBAR:
      g_value_set_boolean (value, options->show_statusbar);
      break;
    case PROP_SHOW_RULERS:
      g_value_set_boolean (value, options->show_rulers);
      break;
    case PROP_SHOW_SCROLLBARS:
      g_value_set_boolean (value, options->show_scrollbars);
      break;
    case PROP_SHOW_SELECTION:
      g_value_set_boolean (value, options->show_selection);
      break;
    case PROP_SHOW_LAYER_BOUNDARY:
      g_value_set_boolean (value, options->show_layer_boundary);
      break;
    case PROP_SHOW_GUIDES:
      g_value_set_boolean (value, options->show_guides);
      break;
    case PROP_SHOW_GRID:
      g_value_set_boolean (value, options->show_grid);
      break;
    case PROP_SHOW_SAMPLE_POINTS:
      g_value_set_boolean (value, options->show_sample_points);
      break;
    case PROP_PADDING_MODE:
      g_value_set_enum (value, options->padding_mode);
      break;
    case PROP_PADDING_COLOR:
      g_value_set_boxed (value, &options->padding_color);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
