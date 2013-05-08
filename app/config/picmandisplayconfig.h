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

#ifndef __PICMAN_DISPLAY_CONFIG_H__
#define __PICMAN_DISPLAY_CONFIG_H__

#include "config/picmancoreconfig.h"


#define PICMAN_CONFIG_DEFAULT_IMAGE_TITLE_FORMAT  "%D*%f-%p.%i (%t, %L) %wx%h"
#define PICMAN_CONFIG_DEFAULT_IMAGE_STATUS_FORMAT "%n (%m)"


#define PICMAN_TYPE_DISPLAY_CONFIG            (picman_display_config_get_type ())
#define PICMAN_DISPLAY_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DISPLAY_CONFIG, PicmanDisplayConfig))
#define PICMAN_DISPLAY_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DISPLAY_CONFIG, PicmanDisplayConfigClass))
#define PICMAN_IS_DISPLAY_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DISPLAY_CONFIG))
#define PICMAN_IS_DISPLAY_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DISPLAY_CONFIG))


typedef struct _PicmanDisplayConfigClass PicmanDisplayConfigClass;

struct _PicmanDisplayConfig
{
  PicmanCoreConfig      parent_instance;

  PicmanCheckSize       transparency_size;
  PicmanCheckType       transparency_type;
  gint                snap_distance;
  gint                marching_ants_speed;
  gboolean            resize_windows_on_zoom;
  gboolean            resize_windows_on_resize;
  gboolean            default_dot_for_dot;
  gboolean            initial_zoom_to_fit;
  gboolean            perfect_mouse;
  PicmanCursorMode      cursor_mode;
  gboolean            cursor_updating;
  gboolean            show_brush_outline;
  gboolean            show_paint_tool_cursor;
  gchar              *image_title_format;
  gchar              *image_status_format;
  gdouble             monitor_xres;
  gdouble             monitor_yres;
  gboolean            monitor_res_from_gdk;
  PicmanViewSize        nav_preview_size;
  PicmanDisplayOptions *default_view;
  PicmanDisplayOptions *default_fullscreen_view;
  gboolean            default_snap_to_guides;
  gboolean            default_snap_to_grid;
  gboolean            default_snap_to_canvas;
  gboolean            default_snap_to_path;
  gboolean            activate_on_focus;
  PicmanSpaceBarAction  space_bar_action;
  PicmanZoomQuality     zoom_quality;
  gboolean            use_event_history;
};

struct _PicmanDisplayConfigClass
{
  PicmanCoreConfigClass  parent_class;
};


GType  picman_display_config_get_type (void) G_GNUC_CONST;


#endif /* PICMAN_DISPLAY_CONFIG_H__ */
