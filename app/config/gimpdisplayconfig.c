/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanDisplayConfig class
 * Copyright (C) 2001  Sven Neumann <sven@picman.org>
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

#include "config-types.h"

#include "picmanrc-blurbs.h"
#include "picmandisplayconfig.h"
#include "picmandisplayoptions.h"

#include "picman-intl.h"


#define DEFAULT_ACTIVATE_ON_FOCUS    TRUE
#define DEFAULT_MONITOR_RESOLUTION   96.0
#define DEFAULT_MARCHING_ANTS_SPEED  200
#define DEFAULT_USE_EVENT_HISTORY    FALSE

enum
{
  PROP_0,
  PROP_TRANSPARENCY_SIZE,
  PROP_TRANSPARENCY_TYPE,
  PROP_SNAP_DISTANCE,
  PROP_MARCHING_ANTS_SPEED,
  PROP_RESIZE_WINDOWS_ON_ZOOM,
  PROP_RESIZE_WINDOWS_ON_RESIZE,
  PROP_DEFAULT_DOT_FOR_DOT,
  PROP_INITIAL_ZOOM_TO_FIT,
  PROP_PERFECT_MOUSE,
  PROP_CURSOR_MODE,
  PROP_CURSOR_UPDATING,
  PROP_SHOW_BRUSH_OUTLINE,
  PROP_SHOW_PAINT_TOOL_CURSOR,
  PROP_IMAGE_TITLE_FORMAT,
  PROP_IMAGE_STATUS_FORMAT,
  PROP_MONITOR_XRESOLUTION,
  PROP_MONITOR_YRESOLUTION,
  PROP_MONITOR_RES_FROM_GDK,
  PROP_NAV_PREVIEW_SIZE,
  PROP_DEFAULT_VIEW,
  PROP_DEFAULT_FULLSCREEN_VIEW,
  PROP_DEFAULT_SNAP_TO_GUIDES,
  PROP_DEFAULT_SNAP_TO_GRID,
  PROP_DEFAULT_SNAP_TO_CANVAS,
  PROP_DEFAULT_SNAP_TO_PATH,
  PROP_ACTIVATE_ON_FOCUS,
  PROP_SPACE_BAR_ACTION,
  PROP_ZOOM_QUALITY,
  PROP_USE_EVENT_HISTORY,

  /* ignored, only for backward compatibility: */
  PROP_CONFIRM_ON_CLOSE,
  PROP_XOR_COLOR
};


static void  picman_display_config_finalize          (GObject      *object);
static void  picman_display_config_set_property      (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void  picman_display_config_get_property      (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);

static void  picman_display_config_view_notify       (GObject      *object,
                                                    GParamSpec   *pspec,
                                                    gpointer      data);
static void  picman_display_config_fullscreen_notify (GObject      *object,
                                                    GParamSpec   *pspec,
                                                    gpointer      data);


G_DEFINE_TYPE (PicmanDisplayConfig, picman_display_config, PICMAN_TYPE_CORE_CONFIG)

#define parent_class picman_display_config_parent_class


static void
picman_display_config_class_init (PicmanDisplayConfigClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PicmanRGB       color        = { 0, 0, 0, 0 };

  object_class->finalize     = picman_display_config_finalize;
  object_class->set_property = picman_display_config_set_property;
  object_class->get_property = picman_display_config_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TRANSPARENCY_SIZE,
                                 "transparency-size", TRANSPARENCY_SIZE_BLURB,
                                 PICMAN_TYPE_CHECK_SIZE,
                                 PICMAN_CHECK_SIZE_MEDIUM_CHECKS,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TRANSPARENCY_TYPE,
                                 "transparency-type", TRANSPARENCY_TYPE_BLURB,
                                 PICMAN_TYPE_CHECK_TYPE,
                                 PICMAN_CHECK_TYPE_GRAY_CHECKS,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_SNAP_DISTANCE,
                                "snap-distance", DEFAULT_SNAP_DISTANCE_BLURB,
                                1, 255, 8,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_MARCHING_ANTS_SPEED,
                                "marching-ants-speed",
                                MARCHING_ANTS_SPEED_BLURB,
                                10, 10000, DEFAULT_MARCHING_ANTS_SPEED,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_RESIZE_WINDOWS_ON_ZOOM,
                                    "resize-windows-on-zoom",
                                    RESIZE_WINDOWS_ON_ZOOM_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_RESIZE_WINDOWS_ON_RESIZE,
                                    "resize-windows-on-resize",
                                    RESIZE_WINDOWS_ON_RESIZE_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DEFAULT_DOT_FOR_DOT,
                                    "default-dot-for-dot",
                                    DEFAULT_DOT_FOR_DOT_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_INITIAL_ZOOM_TO_FIT,
                                    "initial-zoom-to-fit",
                                    INITIAL_ZOOM_TO_FIT_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_PERFECT_MOUSE,
                                    "perfect-mouse", PERFECT_MOUSE_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CURSOR_MODE,
                                 "cursor-mode", CURSOR_MODE_BLURB,
                                 PICMAN_TYPE_CURSOR_MODE,
                                 PICMAN_CURSOR_MODE_TOOL_ICON,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CURSOR_UPDATING,
                                    "cursor-updating", CURSOR_UPDATING_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_BRUSH_OUTLINE,
                                    "show-brush-outline",
                                    SHOW_BRUSH_OUTLINE_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_SHOW_PAINT_TOOL_CURSOR,
                                    "show-paint-tool-cursor",
                                    SHOW_PAINT_TOOL_CURSOR_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_IMAGE_TITLE_FORMAT,
                                   "image-title-format",
                                   IMAGE_TITLE_FORMAT_BLURB,
                                   PICMAN_CONFIG_DEFAULT_IMAGE_TITLE_FORMAT,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_STRING (object_class, PROP_IMAGE_STATUS_FORMAT,
                                   "image-status-format",
                                   IMAGE_STATUS_FORMAT_BLURB,
                                   PICMAN_CONFIG_DEFAULT_IMAGE_STATUS_FORMAT,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RESOLUTION (object_class, PROP_MONITOR_XRESOLUTION,
                                       "monitor-xresolution",
                                       MONITOR_XRESOLUTION_BLURB,
                                       DEFAULT_MONITOR_RESOLUTION,
                                       PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_RESOLUTION (object_class, PROP_MONITOR_YRESOLUTION,
                                       "monitor-yresolution",
                                       MONITOR_YRESOLUTION_BLURB,
                                       DEFAULT_MONITOR_RESOLUTION,
                                       PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_MONITOR_RES_FROM_GDK,
                                    "monitor-resolution-from-windowing-system",
                                    MONITOR_RES_FROM_GDK_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_NAV_PREVIEW_SIZE,
                                 "navigation-preview-size",
                                 NAVIGATION_PREVIEW_SIZE_BLURB,
                                 PICMAN_TYPE_VIEW_SIZE,
                                 PICMAN_VIEW_SIZE_MEDIUM,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_DEFAULT_VIEW,
                                   "default-view",
                                   DEFAULT_VIEW_BLURB,
                                   PICMAN_TYPE_DISPLAY_OPTIONS,
                                   PICMAN_PARAM_STATIC_STRINGS |
                                   PICMAN_CONFIG_PARAM_AGGREGATE);
  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_DEFAULT_FULLSCREEN_VIEW,
                                   "default-fullscreen-view",
                                   DEFAULT_FULLSCREEN_VIEW_BLURB,
                                   PICMAN_TYPE_DISPLAY_OPTIONS,
                                   PICMAN_PARAM_STATIC_STRINGS |
                                   PICMAN_CONFIG_PARAM_AGGREGATE);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DEFAULT_SNAP_TO_GUIDES,
                                    "default-snap-to-guides",
                                    DEFAULT_SNAP_TO_GUIDES_BLURB,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DEFAULT_SNAP_TO_GRID,
                                    "default-snap-to-grid",
                                    DEFAULT_SNAP_TO_GRID_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DEFAULT_SNAP_TO_CANVAS,
                                    "default-snap-to-canvas",
                                    DEFAULT_SNAP_TO_CANVAS_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DEFAULT_SNAP_TO_PATH,
                                    "default-snap-to-path",
                                    DEFAULT_SNAP_TO_PATH_BLURB,
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_ACTIVATE_ON_FOCUS,
                                    "activate-on-focus",
                                    ACTIVATE_ON_FOCUS_BLURB,
                                    DEFAULT_ACTIVATE_ON_FOCUS,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_SPACE_BAR_ACTION,
                                 "space-bar-action",
                                 SPACE_BAR_ACTION_BLURB,
                                 PICMAN_TYPE_SPACE_BAR_ACTION,
                                 PICMAN_SPACE_BAR_ACTION_PAN,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_ZOOM_QUALITY,
                                 "zoom-quality",
                                 ZOOM_QUALITY_BLURB,
                                 PICMAN_TYPE_ZOOM_QUALITY,
                                 PICMAN_ZOOM_QUALITY_HIGH,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_EVENT_HISTORY,
                                    "use-event-history",
                                    DEFAULT_USE_EVENT_HISTORY_BLURB,
                                    DEFAULT_USE_EVENT_HISTORY,
                                    PICMAN_PARAM_STATIC_STRINGS);

  /*  only for backward compatibility:  */
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_CONFIRM_ON_CLOSE,
                                    "confirm-on-close", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS |
                                    PICMAN_CONFIG_PARAM_IGNORE);
  PICMAN_CONFIG_INSTALL_PROP_RGB (object_class, PROP_XOR_COLOR,
                                "xor-color", NULL,
                                FALSE, &color,
                                PICMAN_PARAM_STATIC_STRINGS |
                                PICMAN_CONFIG_PARAM_IGNORE);
}

static void
picman_display_config_init (PicmanDisplayConfig *config)
{
  config->default_view =
    g_object_new (PICMAN_TYPE_DISPLAY_OPTIONS, NULL);

  g_signal_connect (config->default_view, "notify",
                    G_CALLBACK (picman_display_config_view_notify),
                    config);

  config->default_fullscreen_view =
    g_object_new (PICMAN_TYPE_DISPLAY_OPTIONS, NULL);

  g_signal_connect (config->default_fullscreen_view, "notify",
                    G_CALLBACK (picman_display_config_fullscreen_notify),
                    config);
}

static void
picman_display_config_finalize (GObject *object)
{
  PicmanDisplayConfig *display_config = PICMAN_DISPLAY_CONFIG (object);

  g_free (display_config->image_title_format);
  g_free (display_config->image_status_format);

  if (display_config->default_view)
    g_object_unref (display_config->default_view);

  if (display_config->default_fullscreen_view)
    g_object_unref (display_config->default_fullscreen_view);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_display_config_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanDisplayConfig *display_config = PICMAN_DISPLAY_CONFIG (object);

  switch (property_id)
    {
    case PROP_TRANSPARENCY_SIZE:
      display_config->transparency_size = g_value_get_enum (value);
      break;
    case PROP_TRANSPARENCY_TYPE:
      display_config->transparency_type = g_value_get_enum (value);
      break;
    case PROP_SNAP_DISTANCE:
      display_config->snap_distance = g_value_get_int (value);
      break;
    case PROP_MARCHING_ANTS_SPEED:
      display_config->marching_ants_speed = g_value_get_int (value);
      break;
    case PROP_RESIZE_WINDOWS_ON_ZOOM:
      display_config->resize_windows_on_zoom = g_value_get_boolean (value);
      break;
    case PROP_RESIZE_WINDOWS_ON_RESIZE:
      display_config->resize_windows_on_resize = g_value_get_boolean (value);
      break;
    case PROP_DEFAULT_DOT_FOR_DOT:
      display_config->default_dot_for_dot = g_value_get_boolean (value);
      break;
    case PROP_INITIAL_ZOOM_TO_FIT:
      display_config->initial_zoom_to_fit = g_value_get_boolean (value);
      break;
    case PROP_PERFECT_MOUSE:
      display_config->perfect_mouse = g_value_get_boolean (value);
      break;
    case PROP_CURSOR_MODE:
      display_config->cursor_mode = g_value_get_enum (value);
      break;
    case PROP_CURSOR_UPDATING:
      display_config->cursor_updating = g_value_get_boolean (value);
      break;
    case PROP_SHOW_BRUSH_OUTLINE:
      display_config->show_brush_outline = g_value_get_boolean (value);
      break;
    case PROP_SHOW_PAINT_TOOL_CURSOR:
      display_config->show_paint_tool_cursor = g_value_get_boolean (value);
      break;
    case PROP_IMAGE_TITLE_FORMAT:
      g_free (display_config->image_title_format);
      display_config->image_title_format = g_value_dup_string (value);
      break;
    case PROP_IMAGE_STATUS_FORMAT:
      g_free (display_config->image_status_format);
      display_config->image_status_format = g_value_dup_string (value);
      break;
    case PROP_MONITOR_XRESOLUTION:
      display_config->monitor_xres = g_value_get_double (value);
      break;
    case PROP_MONITOR_YRESOLUTION:
      display_config->monitor_yres = g_value_get_double (value);
      break;
    case PROP_MONITOR_RES_FROM_GDK:
      display_config->monitor_res_from_gdk = g_value_get_boolean (value);
      break;
    case PROP_NAV_PREVIEW_SIZE:
      display_config->nav_preview_size = g_value_get_enum (value);
      break;
    case PROP_DEFAULT_VIEW:
      if (g_value_get_object (value))
        picman_config_sync (g_value_get_object (value),
                          G_OBJECT (display_config->default_view), 0);
      break;
    case PROP_DEFAULT_FULLSCREEN_VIEW:
      if (g_value_get_object (value))
        picman_config_sync (g_value_get_object (value),
                          G_OBJECT (display_config->default_fullscreen_view),
                          0);
      break;
    case PROP_DEFAULT_SNAP_TO_GUIDES:
      display_config->default_snap_to_guides = g_value_get_boolean (value);
      break;
    case PROP_DEFAULT_SNAP_TO_GRID:
      display_config->default_snap_to_grid = g_value_get_boolean (value);
      break;
    case PROP_DEFAULT_SNAP_TO_CANVAS:
      display_config->default_snap_to_canvas = g_value_get_boolean (value);
      break;
    case PROP_DEFAULT_SNAP_TO_PATH:
      display_config->default_snap_to_path = g_value_get_boolean (value);
      break;
    case PROP_ACTIVATE_ON_FOCUS:
      display_config->activate_on_focus = g_value_get_boolean (value);
      break;
    case PROP_SPACE_BAR_ACTION:
      display_config->space_bar_action = g_value_get_enum (value);
      break;
    case PROP_ZOOM_QUALITY:
      display_config->zoom_quality = g_value_get_enum (value);
      break;
    case PROP_USE_EVENT_HISTORY:
      display_config->use_event_history = g_value_get_boolean (value);
      break;

    case PROP_CONFIRM_ON_CLOSE:
    case PROP_XOR_COLOR:
      /* ignored */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_config_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanDisplayConfig *display_config = PICMAN_DISPLAY_CONFIG (object);

  switch (property_id)
    {
    case PROP_TRANSPARENCY_SIZE:
      g_value_set_enum (value, display_config->transparency_size);
      break;
    case PROP_TRANSPARENCY_TYPE:
      g_value_set_enum (value, display_config->transparency_type);
      break;
    case PROP_SNAP_DISTANCE:
      g_value_set_int (value, display_config->snap_distance);
      break;
    case PROP_MARCHING_ANTS_SPEED:
      g_value_set_int (value, display_config->marching_ants_speed);
      break;
    case PROP_RESIZE_WINDOWS_ON_ZOOM:
      g_value_set_boolean (value, display_config->resize_windows_on_zoom);
      break;
    case PROP_RESIZE_WINDOWS_ON_RESIZE:
      g_value_set_boolean (value, display_config->resize_windows_on_resize);
      break;
    case PROP_DEFAULT_DOT_FOR_DOT:
      g_value_set_boolean (value, display_config->default_dot_for_dot);
      break;
    case PROP_INITIAL_ZOOM_TO_FIT:
      g_value_set_boolean (value, display_config->initial_zoom_to_fit);
      break;
    case PROP_PERFECT_MOUSE:
      g_value_set_boolean (value, display_config->perfect_mouse);
      break;
    case PROP_CURSOR_MODE:
      g_value_set_enum (value, display_config->cursor_mode);
      break;
    case PROP_CURSOR_UPDATING:
      g_value_set_boolean (value, display_config->cursor_updating);
      break;
    case PROP_SHOW_BRUSH_OUTLINE:
      g_value_set_boolean (value, display_config->show_brush_outline);
      break;
    case PROP_SHOW_PAINT_TOOL_CURSOR:
      g_value_set_boolean (value, display_config->show_paint_tool_cursor);
      break;
    case PROP_IMAGE_TITLE_FORMAT:
      g_value_set_string (value, display_config->image_title_format);
      break;
    case PROP_IMAGE_STATUS_FORMAT:
      g_value_set_string (value, display_config->image_status_format);
      break;
    case PROP_MONITOR_XRESOLUTION:
      g_value_set_double (value, display_config->monitor_xres);
      break;
    case PROP_MONITOR_YRESOLUTION:
      g_value_set_double (value, display_config->monitor_yres);
      break;
    case PROP_MONITOR_RES_FROM_GDK:
      g_value_set_boolean (value, display_config->monitor_res_from_gdk);
      break;
    case PROP_NAV_PREVIEW_SIZE:
      g_value_set_enum (value, display_config->nav_preview_size);
      break;
    case PROP_DEFAULT_VIEW:
      g_value_set_object (value, display_config->default_view);
      break;
    case PROP_DEFAULT_FULLSCREEN_VIEW:
      g_value_set_object (value, display_config->default_fullscreen_view);
      break;
    case PROP_DEFAULT_SNAP_TO_GUIDES:
      g_value_set_boolean (value, display_config->default_snap_to_guides);
      break;
    case PROP_DEFAULT_SNAP_TO_GRID:
      g_value_set_boolean (value, display_config->default_snap_to_grid);
      break;
    case PROP_DEFAULT_SNAP_TO_CANVAS:
      g_value_set_boolean (value, display_config->default_snap_to_canvas);
      break;
    case PROP_DEFAULT_SNAP_TO_PATH:
      g_value_set_boolean (value, display_config->default_snap_to_path);
      break;
    case PROP_ACTIVATE_ON_FOCUS:
      g_value_set_boolean (value, display_config->activate_on_focus);
      break;
    case PROP_SPACE_BAR_ACTION:
      g_value_set_enum (value, display_config->space_bar_action);
      break;
    case PROP_ZOOM_QUALITY:
      g_value_set_enum (value, display_config->zoom_quality);
      break;
    case PROP_USE_EVENT_HISTORY:
      g_value_set_boolean (value, display_config->use_event_history);
      break;

    case PROP_CONFIRM_ON_CLOSE:
    case PROP_XOR_COLOR:
      /* ignored */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_config_view_notify (GObject    *object,
                                 GParamSpec *pspec,
                                 gpointer    data)
{
  g_object_notify (G_OBJECT (data), "default-view");
}

static void
picman_display_config_fullscreen_notify (GObject    *object,
                                       GParamSpec *pspec,
                                       gpointer    data)
{
  g_object_notify (G_OBJECT (data), "default-fullscreen-view");
}
