/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanCoreConfig class
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

#ifndef __PICMAN_CORE_CONFIG_H__
#define __PICMAN_CORE_CONFIG_H__

#include "core/core-enums.h"

#include "config/picmangeglconfig.h"


#define PICMAN_TYPE_CORE_CONFIG            (picman_core_config_get_type ())
#define PICMAN_CORE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CORE_CONFIG, PicmanCoreConfig))
#define PICMAN_CORE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CORE_CONFIG, PicmanCoreConfigClass))
#define PICMAN_IS_CORE_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CORE_CONFIG))
#define PICMAN_IS_CORE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CORE_CONFIG))


typedef struct _PicmanCoreConfigClass PicmanCoreConfigClass;

struct _PicmanCoreConfig
{
  PicmanGeglConfig          parent_instance;

  gchar                  *language;
  PicmanInterpolationType   interpolation_type;
  gint                    default_threshold;
  gchar                  *plug_in_path;
  gchar                  *module_path;
  gchar                  *interpreter_path;
  gchar                  *environ_path;
  gchar                  *brush_path;
  gchar                  *brush_path_writable;
  gchar                  *dynamics_path;
  gchar                  *dynamics_path_writable;
  gchar                  *pattern_path;
  gchar                  *pattern_path_writable;
  gchar                  *palette_path;
  gchar                  *palette_path_writable;
  gchar                  *gradient_path;
  gchar                  *gradient_path_writable;
  gchar                  *tool_preset_path;
  gchar                  *tool_preset_path_writable;
  gchar                  *font_path;
  gchar                  *font_path_writable;  /*  unused  */
  gchar                  *default_brush;
  gchar                  *default_dynamics;
  gchar                  *default_pattern;
  gchar                  *default_palette;
  gchar                  *default_tool_preset;
  gchar                  *default_gradient;
  gchar                  *default_font;
  gboolean                global_brush;
  gboolean                global_dynamics;
  gboolean                global_pattern;
  gboolean                global_palette;
  gboolean                global_gradient;
  gboolean                global_font;
  PicmanTemplate           *default_image;
  PicmanGrid               *default_grid;
  gint                    levels_of_undo;
  guint64                 undo_size;
  PicmanViewSize            undo_preview_size;
  gint                    plug_in_history_size;
  gchar                  *plug_in_rc_path;
  gboolean                layer_previews;
  PicmanViewSize            layer_preview_size;
  PicmanThumbnailSize       thumbnail_size;
  guint64                 thumbnail_filesize_limit;
  PicmanColorConfig        *color_management;
  PicmanColorProfilePolicy  color_profile_policy;
  gboolean                save_document_history;
  PicmanRGB                 quick_mask_color;
};

struct _PicmanCoreConfigClass
{
  PicmanGeglConfigClass  parent_class;
};


GType  picman_core_config_get_type (void) G_GNUC_CONST;


#endif /* PICMAN_CORE_CONFIG_H__ */
