/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGuiConfig class
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

#ifndef __PICMAN_GUI_CONFIG_H__
#define __PICMAN_GUI_CONFIG_H__

#include "config/picmandisplayconfig.h"


#define PICMAN_TYPE_GUI_CONFIG            (picman_gui_config_get_type ())
#define PICMAN_GUI_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GUI_CONFIG, PicmanGuiConfig))
#define PICMAN_GUI_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GUI_CONFIG, PicmanGuiConfigClass))
#define PICMAN_IS_GUI_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GUI_CONFIG))
#define PICMAN_IS_GUI_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GUI_CONFIG))


typedef struct _PicmanGuiConfigClass PicmanGuiConfigClass;

struct _PicmanGuiConfig
{
  PicmanDisplayConfig    parent_instance;

  gboolean             move_tool_changes_active;
  gint                 image_map_tool_max_recent;
  gboolean             trust_dirty_flag;
  gboolean             save_device_status;
  gboolean             save_session_info;
  gboolean             restore_session;
  gboolean             save_tool_options;
  gboolean             show_tooltips;
  gboolean             tearoff_menus;
  gboolean             can_change_accels;
  gboolean             save_accels;
  gboolean             restore_accels;
  gint                 last_opened_size;
  guint64              max_new_image_size;
  gboolean             toolbox_color_area;
  gboolean             toolbox_foo_area;
  gboolean             toolbox_image_area;
  gboolean             toolbox_wilber;
  gchar               *theme_path;
  gchar               *theme;
  gboolean             use_help;
  gboolean             show_help_button;
  gchar               *help_locales;
  PicmanHelpBrowserType  help_browser;
  gchar               *web_browser;
  gboolean             user_manual_online;
  gchar               *user_manual_online_uri;
  PicmanWindowHint       dock_window_hint;
  PicmanHandedness       cursor_handedness;

  /* saved in sessionrc */
  gboolean             hide_docks;
  gboolean             single_window_mode;
  gint                 last_tip_shown;
};

struct _PicmanGuiConfigClass
{
  PicmanDisplayConfigClass  parent_class;
};


GType  picman_gui_config_get_type (void) G_GNUC_CONST;


#endif /* PICMAN_GUI_CONFIG_H__ */
