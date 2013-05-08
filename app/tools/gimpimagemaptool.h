/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef  __PICMAN_IMAGE_MAP_TOOL_H__
#define  __PICMAN_IMAGE_MAP_TOOL_H__


#include "picmancolortool.h"


#define PICMAN_TYPE_IMAGE_MAP_TOOL            (picman_image_map_tool_get_type ())
#define PICMAN_IMAGE_MAP_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_MAP_TOOL, PicmanImageMapTool))
#define PICMAN_IMAGE_MAP_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_MAP_TOOL, PicmanImageMapToolClass))
#define PICMAN_IS_IMAGE_MAP_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_MAP_TOOL))
#define PICMAN_IS_IMAGE_MAP_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_MAP_TOOL))
#define PICMAN_IMAGE_MAP_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_MAP_TOOL, PicmanImageMapToolClass))

#define PICMAN_IMAGE_MAP_TOOL_GET_OPTIONS(t)  (PICMAN_IMAGE_MAP_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanImageMapToolClass PicmanImageMapToolClass;

struct _PicmanImageMapTool
{
  PicmanColorTool          parent_instance;

  PicmanDrawable          *drawable;

  GeglNode              *operation;
  GObject               *config;
  GObject               *default_config;
  gchar                 *undo_desc;

  PicmanImageMap          *image_map;

  /* dialog */
  gboolean               overlay;
  GtkWidget             *dialog;
  GtkWidget             *main_vbox;
  GtkWidget             *settings_box;
  GtkSizeGroup          *label_group;
  GtkWidget             *active_picker;
};

struct _PicmanImageMapToolClass
{
  PicmanColorToolClass  parent_class;

  const gchar        *dialog_desc;
  const gchar        *settings_name;
  const gchar        *import_dialog_title;
  const gchar        *export_dialog_title;

  PicmanContainer      *recent_settings;

  /* virtual functions */
  GeglNode  * (* get_operation)   (PicmanImageMapTool  *image_map_tool,
                                   GObject          **config,
                                   gchar            **undo_desc);
  void        (* map)             (PicmanImageMapTool  *image_map_tool);
  void        (* dialog)          (PicmanImageMapTool  *image_map_tool);
  void        (* reset)           (PicmanImageMapTool  *image_map_tool);

  GtkWidget * (* get_settings_ui) (PicmanImageMapTool  *image_map_tool,
                                   PicmanContainer     *settings,
                                   const gchar       *settings_filename,
                                   const gchar       *import_dialog_title,
                                   const gchar       *export_dialog_title,
                                   const gchar       *file_dialog_help_id,
                                   const gchar       *default_folder,
                                   GtkWidget        **settings_box);

  gboolean    (* settings_import) (PicmanImageMapTool  *image_map_tool,
                                   const gchar       *filename,
                                   GError           **error);
  gboolean    (* settings_export) (PicmanImageMapTool  *image_map_tool,
                                   const gchar       *filename,
                                   GError           **error);

  void        (* color_picked)    (PicmanImageMapTool  *image_map_tool,
                                   gpointer           identifier,
                                   const Babl        *sample_format,
                                   const PicmanRGB     *color);
};


GType   picman_image_map_tool_get_type      (void) G_GNUC_CONST;

void    picman_image_map_tool_preview       (PicmanImageMapTool *image_map_tool);

void    picman_image_map_tool_get_operation (PicmanImageMapTool *image_map_tool);

void    picman_image_map_tool_edit_as       (PicmanImageMapTool *image_map_tool,
                                           const gchar      *new_tool_id,
                                           PicmanConfig       *config);

/* accessors for derived classes */
GtkWidget    * picman_image_map_tool_dialog_get_vbox        (PicmanImageMapTool *tool);
GtkSizeGroup * picman_image_map_tool_dialog_get_label_group (PicmanImageMapTool *tool);

GtkWidget    * picman_image_map_tool_add_color_picker       (PicmanImageMapTool *tool,
                                                           gpointer          identifier,
                                                           const gchar      *stock_id,
                                                           const gchar      *help_id);


#endif  /*  __PICMAN_IMAGE_MAP_TOOL_H__  */
