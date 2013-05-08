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

#include "config.h"

#include <string.h>

#include <gegl.h>
#include <gegl-plugin.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "gegl/picman-gegl-config-proxy.h"

#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanimagemap.h"
#include "core/picmanimagemapconfig.h"
#include "core/picmanlist.h"
#include "core/picmanparamspecs-duplicate.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"

#include "display/picmandisplay.h"

#include "picmancoloroptions.h"
#include "picmanoperationtool.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void        picman_operation_tool_finalize        (GObject           *object);

static GeglNode  * picman_operation_tool_get_operation   (PicmanImageMapTool  *im_tool,
                                                        GObject          **config,
                                                        gchar            **undo_desc);
static void        picman_operation_tool_map             (PicmanImageMapTool  *im_tool);
static void        picman_operation_tool_dialog          (PicmanImageMapTool  *im_tool);
static void        picman_operation_tool_reset           (PicmanImageMapTool  *im_tool);
static GtkWidget * picman_operation_tool_get_settings_ui (PicmanImageMapTool  *image_map_tool,
                                                        PicmanContainer     *settings,
                                                        const gchar       *settings_filename,
                                                        const gchar       *import_dialog_title,
                                                        const gchar       *export_dialog_title,
                                                        const gchar       *file_dialog_help_id,
                                                        const gchar       *default_folder,
                                                        GtkWidget        **settings_box);
static void        picman_operation_tool_color_picked    (PicmanImageMapTool  *im_tool,
                                                        gpointer           identifier,
                                                        const Babl        *sample_format,
                                                        const PicmanRGB     *color);


G_DEFINE_TYPE (PicmanOperationTool, picman_operation_tool,
               PICMAN_TYPE_IMAGE_MAP_TOOL)

#define parent_class picman_operation_tool_parent_class


void
picman_operation_tool_register (PicmanToolRegisterCallback  callback,
                              gpointer                  data)
{
  (* callback) (PICMAN_TYPE_OPERATION_TOOL,
                PICMAN_TYPE_COLOR_OPTIONS,
                picman_color_options_gui,
                0,
                "picman-operation-tool",
                _("GEGL Operation"),
                _("Operation Tool: Use an arbitrary GEGL operation"),
                N_("_GEGL Operation..."), NULL,
                NULL, PICMAN_HELP_TOOL_GEGL,
                PICMAN_STOCK_GEGL,
                data);
}

static void
picman_operation_tool_class_init (PicmanOperationToolClass *klass)
{
  GObjectClass          *object_class  = G_OBJECT_CLASS (klass);
  PicmanImageMapToolClass *im_tool_class = PICMAN_IMAGE_MAP_TOOL_CLASS (klass);

  object_class->finalize         = picman_operation_tool_finalize;

  im_tool_class->dialog_desc     = _("GEGL Operation");

  im_tool_class->get_operation   = picman_operation_tool_get_operation;
  im_tool_class->map             = picman_operation_tool_map;
  im_tool_class->dialog          = picman_operation_tool_dialog;
  im_tool_class->reset           = picman_operation_tool_reset;
  im_tool_class->get_settings_ui = picman_operation_tool_get_settings_ui;
  im_tool_class->color_picked    = picman_operation_tool_color_picked;
}

static void
picman_operation_tool_init (PicmanOperationTool *tool)
{
  PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->settings_name = NULL; /* XXX hack */
}

static void
picman_operation_tool_finalize (GObject *object)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (object);

  if (tool->operation)
    {
      g_free (tool->operation);
      tool->operation = NULL;
    }

  if (tool->config)
    {
      g_object_unref (tool->config);
      tool->config = NULL;
    }

  if (tool->undo_desc)
    {
      g_free (tool->undo_desc);
      tool->undo_desc = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GeglNode *
picman_operation_tool_get_operation (PicmanImageMapTool  *im_tool,
                                   GObject          **config,
                                   gchar            **undo_desc)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (im_tool);

  if (tool->config)
    *config = g_object_ref (tool->config);

  if (tool->undo_desc)
    *undo_desc = g_strdup (tool->undo_desc);

  if (tool->operation)
    return gegl_node_new_child (NULL,
                                "operation", tool->operation,
                                NULL);

  return g_object_new (GEGL_TYPE_NODE, NULL);
}

static void
picman_operation_tool_map (PicmanImageMapTool *image_map_tool)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (image_map_tool);

  if (tool->config)
    picman_gegl_config_proxy_sync (tool->config, image_map_tool->operation);
}

static void
picman_operation_tool_dialog (PicmanImageMapTool *image_map_tool)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (image_map_tool);
  GtkWidget         *main_vbox;

  main_vbox = picman_image_map_tool_dialog_get_vbox (image_map_tool);

  /*  The options vbox  */
  tool->options_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (main_vbox), tool->options_box,
                      FALSE, FALSE, 0);
  gtk_widget_show (tool->options_box);

  if (tool->options_table)
    {
      gtk_container_add (GTK_CONTAINER (tool->options_box),
                         tool->options_table);
      gtk_widget_show (tool->options_table);
    }

  if (tool->undo_desc)
    g_object_set (PICMAN_IMAGE_MAP_TOOL (tool)->dialog,
                  "description", tool->undo_desc,
                  NULL);
}

static void
picman_operation_tool_reset (PicmanImageMapTool *image_map_tool)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (image_map_tool);

  if (tool->config)
    picman_config_reset (PICMAN_CONFIG (tool->config));
}

static GtkWidget *
picman_operation_tool_get_settings_ui (PicmanImageMapTool  *image_map_tool,
                                     PicmanContainer     *settings,
                                     const gchar       *settings_filename,
                                     const gchar       *import_dialog_title,
                                     const gchar       *export_dialog_title,
                                     const gchar       *file_dialog_help_id,
                                     const gchar       *default_folder,
                                     GtkWidget        **settings_box)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (image_map_tool);
  GType              type = G_TYPE_FROM_INSTANCE (tool->config);
  GtkWidget         *widget;
  gchar             *basename;
  gchar             *filename;
  gchar             *import_title;
  gchar             *export_title;

  settings = picman_gegl_get_config_container (type);
  if (! picman_list_get_sort_func (PICMAN_LIST (settings)))
    picman_list_set_sort_func (PICMAN_LIST (settings),
                             (GCompareFunc) picman_image_map_config_compare);

  basename = g_strconcat (G_OBJECT_TYPE_NAME (tool->config), ".settings", NULL);
  filename = g_build_filename (picman_directory (), "filters", basename, NULL);
  g_free (basename);

  import_title = g_strdup_printf (_("Import '%s' Settings"), tool->undo_desc);
  export_title = g_strdup_printf (_("Export '%s' Settings"), tool->undo_desc);

  widget =
    PICMAN_IMAGE_MAP_TOOL_CLASS (parent_class)->get_settings_ui (image_map_tool,
                                                               settings,
                                                               filename,
                                                               import_title,
                                                               export_title,
                                                               "help-foo",
                                                               g_get_home_dir (),
                                                               settings_box);

  g_free (filename);
  g_free (import_title);
  g_free (export_title);

  return widget;
}

static void
picman_operation_tool_color_picked (PicmanImageMapTool  *im_tool,
                                  gpointer           identifier,
                                  const Babl        *sample_format,
                                  const PicmanRGB     *color)
{
  PicmanOperationTool *tool = PICMAN_OPERATION_TOOL (im_tool);

  g_object_set (tool->config,
                identifier, color,
                NULL);
}

void
picman_operation_tool_set_operation (PicmanOperationTool *tool,
                                   const gchar       *operation,
                                   const gchar       *undo_desc)
{
  g_return_if_fail (PICMAN_IS_OPERATION_TOOL (tool));
  g_return_if_fail (operation != NULL);

  if (tool->operation)
    g_free (tool->operation);

  if (tool->undo_desc)
    g_free (tool->undo_desc);

  tool->operation = g_strdup (operation);
  tool->undo_desc = g_strdup (undo_desc);

  if (tool->config)
    g_object_unref (tool->config);

  tool->config = picman_gegl_get_config_proxy (tool->operation,
                                             PICMAN_TYPE_IMAGE_MAP_CONFIG);

  picman_image_map_tool_get_operation (PICMAN_IMAGE_MAP_TOOL (tool));

  if (undo_desc)
    PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->settings_name = "yes"; /* XXX hack */
  else
    PICMAN_IMAGE_MAP_TOOL_GET_CLASS (tool)->settings_name = NULL; /* XXX hack */

  if (tool->options_table)
    {
      gtk_widget_destroy (tool->options_table);
      tool->options_table = NULL;
    }

  if (tool->config)
    {
      tool->options_table =
        picman_prop_table_new (G_OBJECT (tool->config),
                             G_TYPE_FROM_INSTANCE (tool->config),
                             PICMAN_CONTEXT (PICMAN_TOOL_GET_OPTIONS (tool)),
                             (PicmanCreatePickerFunc) picman_image_map_tool_add_color_picker,
                             tool);

      if (tool->options_box)
        {
          gtk_container_add (GTK_CONTAINER (tool->options_box),
                             tool->options_table);
          gtk_widget_show (tool->options_table);
        }
    }

  if (undo_desc && PICMAN_IMAGE_MAP_TOOL (tool)->dialog)
    g_object_set (PICMAN_IMAGE_MAP_TOOL (tool)->dialog,
                  "description", undo_desc,
                  NULL);

  if (PICMAN_TOOL (tool)->drawable)
    picman_image_map_tool_preview (PICMAN_IMAGE_MAP_TOOL (tool));
}
